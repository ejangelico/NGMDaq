#include "SIS3302System.h"
#include "NGMSystemConfiguration.h"
#include <limits>
#include "sis3302card.h"
#include "sis3820card.h"
#include "vme_interface_class.h"
#include <stdlib.h>
#include "NGMBufferedPacket.h"
#include "TFile.h"
#include "NGMLogger.h"
#include "TObjString.h"
#include "TSystem.h"

SIS3302System::SIS3302System()
{
    vmei = 0;
    _rawFilePointer = 0;
    _isRunning = false;
    _broadcastbase = 0x30000000;
    _runBegin = 0;
    _c3820 = 0;
    _numberOfSlots = 3;

}

SIS3302System::~SIS3302System()
{
    delete _c3820;
}

int SIS3302System::CreateDefaultConfig(char const *configname)
{

    
    if(_config)
        delete _config;
    _config = new NGMSystemConfigurationv1("SIS3302",_numberOfSlots,_numberOfSlots*SIS3302_CHANNELS_PER_CARD);
    
    NGMConfigurationTable* csys = _config->GetSystemParameters();
    NGMConfigurationTable* cslot = _config->GetSlotParameters();
    NGMConfigurationTable* cchan = _config->GetChannelParameters();
    NGMConfigurationTable* cdet = _config->GetDetectorParameters();
    NGMConfigurationTable* chv = _config->GetHVParameters();
    
    csys->AddParameterI("RunCheckStopTimeFlag", 0);
    csys->AddParameterI("RunCheckStopEventsFlag", 0);
    csys->AddParameterI("RunMaxEventCounter", 0);
    csys->AddParameterD("MaxDuration", 0,60);
    csys->AddParameterI("DataEvent_FileCounter", 0);
    csys->AddParameterI("dataFormat", 0);
    csys->AddParameterI("NoOfModulesRun", 0);
    csys->AddParameterS("RawOutputPath", "./");
    csys->AddParameterI("PSD_SCHEME",10,0,2);
    csys->SetParameterI("PSD_SCHEME",0,2);
    csys->SetParameterToDefault("RawOutputPath");
    csys->SetParameterToDefault("MaxDuration");

    // Required Configuration Parameters
    csys->AddParameterI("RunReadoutMode",1,1,3);

    cslot->AddParameterI("ModEnable");
    cslot->AddParameterS("ModAddr", "NA");
    cslot->AddParameterI("ClockMode",0);
    cslot->AddParameterD("NanoSecondsPerSample",10.0);
    cslot->AddParameterI("modid");
    cslot->AddParameterS("cardtype","sis3302card");
    cslot->AddParameterO("card");
    
     
    NGMConfigurationParameter* cards = cslot->GetColumn("card");
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
        TString addrString;
	addrString.Form("0x%08x",0x40000000+icard*0x08000000);
	cslot->SetParameterO("card",icard,new sis3302card());
	cslot->SetParameterS("ModAddr",icard,addrString.Data());
	cslot->SetParameterS("cardtype",icard,"sis3302card");
        sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
        card->baseaddress = strtoul(cslot->GetParValueS("ModAddr",icard),0,0);
        printf("Slot %d BaseAddress 0x%08x\n",icard,card->baseaddress);
        cslot->SetParameterI("ModEnable",icard,1);
        cslot->SetParameterD("NanoSecondsPerSample",icard,10.0);
    }
    
    cdet->AddParameterS("DetectorName","NA");
    cdet->SetParameterToDefault("DetectorName");
    
    chv->SetNRows(6);
    chv->AddParameterS("DetectorName","NA");
    chv->AddParameterS("HVController","caenHV6533card");
    chv->AddParameterD("Voltage",0.0,0.0,1300);
    chv->AddParameterI("ChannelID",0);
    chv->AddParameterI("HVChanID",0);
    chv->SetParameterToDefault("Voltage");
    chv->SetParameterToDefault("ChannelID");
    chv->SetParameterToDefault("DetectorID");
    chv->SetParameterToDefault("HVController");
    
    for(int ichan = 0; ichan< chv->GetEntries(); ichan++)
    {
        chv->SetParameterI("HVChanID",ichan,ichan);
    }
    
    return 0;
}

int SIS3302System::InitializeSystem()
{
    CreateDefaultConfig("SIS3302");
    
    NGMConfigurationTable* cslot = _config->GetSlotParameters();

    vmei = vme_interface_class::Factory("SISUSB");
    vmei->vmeopen();
    if(vmei==0)
    {
        LOG<<"Unable to Find vme interface "<<ENDM_FATAL;
        return -1;
    }
    // Assume if we have more than one card
    // we have a 3820 to sync them
    if(_numberOfSlots>1)
    {
        _c3820 = new sis3820card(vmei,0x38000000);
        _c3820->SetClockChoice(0);
    }
    
    NGMConfigurationParameter* cards = cslot->GetColumn("card");
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
	if(cslot->GetParValueI("ModEnable",icard)==0) continue;
        sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
        card->vmei=vmei;
        card->initcard();
        card->AllocateBuffers();
        if(_numberOfSlots>1)
        {
            card->SetClockChoice(6,0); //6 is external lemo clock
        }else{
            card->SetClockChoice(2,0); //1 is internal 50MHz
        }
        card->SetBroadcastAddress(_broadcastbase,true,icard==0/*first card is broadcast master*/);
        Default3302Card(card);
        card->SetCardHeader(icard);
    }
    return 0;
}

void SIS3302System::Default3302Card(sis3302card* vslot)
{
    /*************Event Configuration Registers*******************/
    for(int iadc=0; iadc < SIS3302_ADCGROUP_PER_CARD;iadc++ ){
        vslot->gate_window_length_block[iadc]=1000;
        vslot->pretriggerdelay_block[iadc]=300;
        vslot->sample_length_block[iadc]=vslot->gate_window_length_block[iadc];
        vslot->sample_start_block[iadc]=0;
        for(int igate=0;igate<SIS3302_QDC_PER_CHANNEL;igate++){
            vslot->qdcstart[iadc][igate]=80;
            vslot->qdclength[iadc][igate]=10;
        }
        vslot->qdcstart[iadc][0]=0;
        vslot->qdclength[iadc][0]=60;
        vslot->qdcstart[iadc][1]=80;
        vslot->qdclength[iadc][1]=10;
        vslot->addressthreshold[iadc]=0x100000;
    }
    
    for (int iadc=0;iadc<SIS3302_ADCGROUP_PER_CARD;iadc++)
    {
        vslot->dataformat_block[iadc] = 0x1F1F; //
    }
    for (int ichan=0;ichan<SIS3302_CHANNELS_PER_CARD;ichan++) {
        vslot->trigconf[ichan] = 0x5;
    }
    vslot->ConfigureEventRegisters();
    vslot->ConfigureAnalogRegisters();
    /*********Configure Triggering*****************************************/
    for (int ichan=0;ichan<SIS3302_CHANNELS_PER_CARD;ichan++) {
        vslot->risetime[ichan]=4;
        vslot->gaptime[ichan]=0;
        vslot->firenable[ichan]=1;
        vslot->fircfd[ichan]=0x3;
        vslot->firthresh[ichan]=100;
    }
    vslot->ConfigureFIR();
    /***************************************************/
    vslot->EnableThresholdInterrupt();
    printf("Modid %x \n",vslot->modid);
    vslot->PrintRegisters();

}

int SIS3302System::ReadActualADCValues()
{
    NGMConfigurationParameter* cards = _config->GetSlotParameters()->GetColumn("card");
    NGMConfigurationParameter* active = _config->GetSlotParameters()->GetColumn("ModEnable");
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
	if(! active->GetValueI(icard)) continue;
        sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
	card->ReadActualSampleValues();
    }
}

int SIS3302System::ConfigureSystem()
{
    NGMConfigurationParameter* cards = _config->GetSlotParameters()->GetColumn("card");
    NGMConfigurationParameter* active = _config->GetSlotParameters()->GetColumn("ModEnable");
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
	if(! active->GetValueI(icard)) continue;
        
        sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
        //card->SetClockChoice(0,1);
        //card->SetBroadcastAddress(_broadcastbase,true,icard==0/*first card is broadcast master*/);
        card->ConfigureEventRegisters();
        card->ConfigureAnalogRegisters();
        card->ConfigureFIR();
        card->EnableThresholdInterrupt();
    }
   return 0;
}

int SIS3302System::StartAcquisition()
{
    if(_isRunning) return -1;
    SetStopDaqOnCtlC();
    _isRunning = 1;
    _requestStop=false;
    _totalEventsThisRun = 0;
    _firstTimeOfRun = TTimeStamp(0,0);
    _earliestTimeInSpill = TTimeStamp(0,0);
    _latestTimeInSpill = TTimeStamp(0,0);
    _lastPlotIndex = -1;
    _livetime = 0.0;
    _runduration = 0.0;
    _totalBytesWritten=0;
    _rawFileCount = 1;

    _config->SetTimeStampNow();
    delete _runBegin;
    _runBegin = new TTimeStamp(_config->GetTimeStamp());
    openRawOutputFile();
    
    StartListModeGeneral();
    
    // Lets send two signals to all attached modules
    // TObjString("EndRunFlush")
    // TObjString("EndRunSave")
    
    closeRawOutputFile();
    
    TObjString endRunFlush("EndRunFlush");
    TObjString endRunSave("EndRunSave");
    LOG<<"Sending message "<<endRunFlush.GetString().Data()<<ENDM_INFO;
    push(*((const TObject*)&endRunFlush));
    push(*((const TObject*)&endRunSave));
    
    _isRunning = 0;
    ResetCtlC();
    
    return 0;
}

int SIS3302System::StartListModeGeneral()
{
    NGMConfigurationParameter* cards = _config->GetSlotParameters()->GetColumn("card");
    sis3302card* firstcard = dynamic_cast<sis3302card*>(cards->GetValueO(0));
    
    int return_code;
    unsigned int data;
    // Clear Timestamp  */
    if(_numberOfSlots==1)
    {
        firstcard->ClearTimeStamp();
    }else{
        return_code = _c3820->ClearTimeStamp();  //
    }
    double maxLiveTime = _config->GetSystemParameters()->GetParValueD("MaxDuration", 0);
    double timeOfLastTemp = 0.0;
    const double timeToReadTemp = 10.0;
    const double maxTemp = 52.0;
    unsigned int triggerCount = 0;
    unsigned int data_rd = 0;
    int poll_counter = 0;

    // Start Readout Loop  */
    if(1){
      unsigned int actualAddress = 0;
      unsigned int ctlstatus = 0;
      unsigned int irqstatus = 0;
      unsigned int irqconfig = 0;
      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_IRQ_CONFIG,&irqconfig);
      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC1,&actualAddress);
      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_ACQUISITION_CONTROL,&ctlstatus);
      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_IRQ_CONTROL,&irqstatus);
      printf("Trigger Count %d  Sample(0x%08x) Ctl(0x%08x) IRQ(0x%08x 0x%08x) USB(0x%08x)\n",triggerCount,actualAddress,ctlstatus,irqstatus,irqconfig,data_rd);
    }
    return_code = firstcard->ArmBank();  //
    
    if(1){
      unsigned int actualAddress = 0;
      unsigned int ctlstatus = 0;
      unsigned int irqstatus = 0;
      unsigned int irqconfig = 0;
      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_IRQ_CONFIG,&irqconfig);
      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC1,&actualAddress);
      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_ACQUISITION_CONTROL,&ctlstatus);
      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_IRQ_CONTROL,&irqstatus);
      printf("Trigger Count %d  Sample(0x%08x) Ctl(0x%08x) IRQ(0x%08x 0x%08x) USB(0x%08x)\n",triggerCount,actualAddress,ctlstatus,irqstatus,irqconfig,data_rd);
    }


    do {
        triggerCount = 0;
	poll_counter = 0 ;
        do {
	    poll_counter++;
	    if(1){
	      unsigned int actualAddress = 0;
	      unsigned int ctlstatus = 0;
	      unsigned int irqstatus = 0;
	      unsigned int irqconfig = 0;
	      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_IRQ_CONFIG,&irqconfig);
	      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC1,&actualAddress);
	      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_ACQUISITION_CONTROL,&ctlstatus);
	      return_code = vmei->vme_A32D32_read(firstcard->baseaddress + SIS3302_IRQ_CONTROL,&irqstatus);
	      printf("Trigger Count %d  Sample(0x%08x) Ctl(0x%08x) IRQ(0x%08x 0x%08x) USB(0x%08x)\n",triggerCount,actualAddress,ctlstatus,irqstatus,irqconfig,data_rd);
	    }

            usleep(100000);
	    vmei->vme_IRQ_Status_read( &data_rd);
        bool doForcedTriggers = false;
        if(doForcedTriggers)
        if(poll_counter%10==0){
            triggerCount++;
            return_code = vmei->vme_A32D32_write ( _broadcastbase + SIS3302_KEY_TRIGGER, 0x0);
            usleep(100);
        }

            if(poll_counter%1==0)
            {
                TTimeStamp curTime;
                curTime.Set();
                _runduration = curTime.AsDouble()-_runBegin->AsDouble();
                if(_runduration>maxLiveTime)
                {
                    printf("Maximum RunTime Exceeded %f(%f)\n",_runduration,maxLiveTime);
                    _requestStop = true;
                }
                if((_runduration-timeOfLastTemp)>timeToReadTemp)
                {
                    timeOfLastTemp = _runduration;
                    float thisTemp = firstcard->ReadTemp();
                    //LOG<<"SIS3302 Temperature:"<<thisTemp<<ENDM_INFO;
                    if(thisTemp>maxTemp)
                    {
                        LOG<<"SIS3302 Temperature Exceeding maximum of 52C < "<<thisTemp<<ENDM_FATAL;
                        _requestStop = true;
                    }
                }
            }
            //This should allow the NGMSpyServ to process requests on 
            gSystem->ProcessEvents();
        } while ((data_rd & 0xFE) == 0x00000 && !_requestStop && poll_counter<100);
        printf("Exiting Event Loop: 0x%08x %d %d\n",data_rd,_requestStop,poll_counter);
        firstcard->DisarmAndArmBank();
        usleep(10000);
        FetchData();
        WriteSpillToFile();
        gSystem->ProcessEvents();
    }while(!_requestStop);
    firstcard->Disarm();
    return 0;
}

int SIS3302System::FetchData()
{
    //Loop over each active card and determine if we need to read each adc block out as separate channels or blocks
    //Perhaps these string addressed items should be cached at the beginning of each acquisition
    NGMConfigurationParameter* cards = _config->GetSlotParameters()->GetColumn("card");
    NGMConfigurationParameter* active = _config->GetSlotParameters()->GetColumn("ModEnable");
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
        //Skip readout of disabled cards
        if(! active->GetValueI(icard) ) continue;
        sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
        card->FetchScalars();
        card->FetchAllData();
        gSystem->ProcessEvents();
    }
    
    return 0;
}

int SIS3302System::ParseSingleChannelData(sis3302card* vcard, int ichan)
{
    //These methods are implemented in NGMSIS3302RawReader
    return 0;
}

int SIS3302System::StopAcquisition()
{
    _requestStop = true;
    return 0;
}
int SIS3302System::RequestAcquisitionStop()
{
    _requestStop = true;
    return 0;
}
void SIS3302System::PlotAdcDistribution(int slot, int channel)
{
    return;
}
int SIS3302System::writePacket(int packetlength, unsigned int *data)
{
    return 0;
}
int SIS3302System::WriteGlobalToTable()
{
    return 0;
}
int SIS3302System::WriteTableToGlobal()
{
    return 0;
}

int SIS3302System::WriteSpillToFile()
{
    if(!_rawFilePointer ) return 0;
    
    size_t written;
    int spillhdr[10];
    spillhdr[0]=0xABBAABBA;//gl_uint_ModAddrRun[module_index];
    spillhdr[1]=0xABBAABBA;//dma_got_no_of_words;
    for(int ihdr = 2; ihdr < 10; ihdr++)
    {
        spillhdr[ihdr] = 0xABBAABBA;
    }
    spillhdr[9]=0xABBAABBA;//dma_got_no_of_words;

    written = fwrite(spillhdr,0x4,10,_rawFilePointer)*0x4;
    _totalBytesWritten+=written;
    
    NGMConfigurationParameter* cards = _config->GetSlotParameters()->GetColumn("card");
    NGMConfigurationParameter* active = _config->GetSlotParameters()->GetColumn("ModEnable");
    
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
        if(! active->GetValueI(icard) ) continue;
        sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
        written = card->WriteSpillToFile(_rawFilePointer);
        _totalBytesWritten+=written;
    }


    return 0;
}

void SIS3302System::closeRawOutputFile()
{
    //Write Raw Trailer
    size_t written;
    if(_rawFilePointer)
    {
        // Write end of file trailer
        // Make it same as Spill header So that we
        // Can check for ENDOFFILE patter
        int spillhdr[10];
        for(int ihdr = 0; ihdr < 10; ihdr++)
        {
            spillhdr[ihdr] = 0x0E0F0E0F;//ENDOFFILE
        }
        written = fwrite(spillhdr,0x4,10,_rawFilePointer)*0x4;
        fclose(_rawFilePointer);
        _totalBytesWritten+=written;
        _rawFilePointer = 0;
    }

}

void SIS3302System::openRawOutputFile()
{
    // Check that a previously opened file is
    // properly closed with a our end of file
    // word
    size_t written;
    closeRawOutputFile();
    
    // Open file and write run header
    unsigned int runhdr[100];
    for(int ihdr = 0; ihdr < 100; ihdr++) runhdr[ihdr]=0;
    runhdr[0]=0x33020001; // File Version
    runhdr[1] = (_config->getRunNumber()&0xFFFF0000 ) >> 32;
    runhdr[2] = (_config->getRunNumber()&0xFFFF);
    
    TString outFileName;
    // Check for output directory
    if(_config->GetSystemParameters()->GetParIndex("RawOutputPath")>=0)
    {
        outFileName+=_config->GetSystemParameters()->GetParValueS("RawOutputPath",0);
        if(! outFileName.EndsWith("/"))
        {
            outFileName+="/";
        }
    }
    // Use run number for file name key
    outFileName+="SIS3302Raw_";
    outFileName+=_config->getRunNumber();
    
    if(_rawFileCount == 1)
    {
        TFile* cnfg = TFile::Open(outFileName+"-conf.root","RECREATE");
        cnfg->WriteTObject(_config,"NGMSystemConfiguration");
        delete cnfg;
    }
    
    outFileName+="_";
    outFileName+=_rawFileCount;
    outFileName+=".bin";
    
    LOG<<" Open raw binary file "<<outFileName.Data()<<ENDM_INFO;
    _rawFilePointer = fopen(outFileName.Data(),"wb");
    if(!_rawFilePointer){
        LOG<<"Unable to open file "<<outFileName.Data()<<ENDM_FATAL;
        return;
    }
    // Write header using 32 bit words
    written = fwrite(runhdr,0x4,100,_rawFilePointer)*0x4;
    _totalBytesWritten+=written;
    if(written != 400)
    {
        LOG<<" Error writing header to "<<outFileName.Data()<<ENDM_FATAL;
        _rawFilePointer = 0;
        return;
    }
    
}

void SIS3302System::GetStatus(TString& status)
{
    status="SIS3302System ";
    if(_isRunning){
        status+=" Active Run ";
        status+=_config->getRunNumber();
    }else{
        status+=" Previous Run ";
        status+=_config->getRunNumber();
    }
    status+=" Duration ";
    status+=_runduration;
    status+="\n";
}
