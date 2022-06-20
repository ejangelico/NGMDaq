#include "SIS3316SystemMT.h"
#include "NGMSystemConfiguration.h"
#include <limits>
#include "sis3316card.h"
#include "vme_interface_class.h"
#include <stdlib.h>
#include "NGMBufferedPacket.h"
#include "TFile.h"
#include "NGMLogger.h"
#include "TObjString.h"
#include "TSystem.h"
#include "tbb/concurrent_queue.h"
#include "TThread.h"
#include <regex>
#include <iostream>
using namespace std;

class SIS3316Buffer_st{
public:
    SIS3316Buffer_st() : slot(-1),chan(-1),status(normal),skippedRead(false){}
    SIS3316Buffer_st(int tslot, int tchan): slot(tslot),chan(tchan), status(normal), skippedRead(false){}
    enum { normal,firstOfSpill,spillComplete,endOfRun} status;
    int slot;
    int chan;
    bool skippedRead;
};


class SIS3316SystemMTDaqRun
{
public:
	SIS3316SystemMTDaqRun(){};
	virtual ~SIS3316SystemMTDaqRun(){}
	tbb::concurrent_bounded_queue<SIS3316Buffer_st> freePacketBuffers;
	tbb::concurrent_bounded_queue<SIS3316Buffer_st> filledPacketBuffers;
    std::vector<sis3316card*> vcards;
};

SIS3316SystemMT::SIS3316SystemMT()
{
    vmei = 0;
    _rawFilePointer = 0;
    _isRunning = false;
    _broadcastbase = 0x30000000;
    _runBegin = 0;
    _numberOfSlots = 5;
    _maxbytestoraw = 1700000000LL;
    _prun = 0;
    _sigintStop = false;
    
}

SIS3316SystemMT::~SIS3316SystemMT()
{
    delete vmei;
}

int SIS3316SystemMT::CreateDefaultConfig(char const *configname)
{
    
    if(_config)
        delete _config;
    _config = new NGMSystemConfigurationv1("SIS3316",_numberOfSlots,_numberOfSlots*SIS3316_CHANNELS_PER_CARD);
    
    NGMConfigurationTable* csys = _config->GetSystemParameters();
    NGMConfigurationTable* cslot = _config->GetSlotParameters();
    NGMConfigurationTable* cchan = _config->GetChannelParameters();
    NGMConfigurationTable* cdet = _config->GetDetectorParameters();
    NGMConfigurationTable* chv = _config->GetHVParameters();
    
    csys->AddParameterI("RunCheckStopTimeFlag", 0);
    csys->AddParameterI("RunCheckStopEventsFlag", 0);
    csys->AddParameterI("RunMaxEventCounter", 0);
    csys->AddParameterD("MaxDuration", 0,60);
    csys->AddParameterD("SpillDuration", 0,2);
    csys->AddParameterD("FullFraction", 0,0.9);
    csys->AddParameterI("DataEvent_FileCounter", 0);
	csys->AddParameterI("dataFormat", 0);
	csys->AddParameterI("NoOfModulesRun", 0);
	csys->AddParameterS("RawOutputPath", "./");
	csys->AddParameterS("OutputFileSuffix", "");
	csys->AddParameterS("Comment", "");
    csys->AddParameterI("PSD_SCHEME",10,0,2);
    csys->SetParameterI("PSD_SCHEME",0,2);
    csys->AddParameterI("MaxFileCount",1000,0,10000);
    csys->SetParameterI("MaxFileCount",0,1000);
    
    csys->SetParameterToDefault("RawOutputPath");
    csys->SetParameterToDefault("OutputFileSuffix");
    csys->SetParameterToDefault("MaxDuration");
    csys->SetParameterToDefault("SpillDuration");
    csys->SetParameterToDefault("FullFraction");

    // Required Configuration Parameters
    csys->AddParameterI("RunReadoutMode",1,1,3);

    cslot->AddParameterI("ModEnable");
	cslot->AddParameterS("ModAddr", "NA");
    cslot->AddParameterI("ClockMode",0);
    cslot->AddParameterD("NanoSecondsPerSample",4.0);
    cslot->AddParameterI("modid");
    cslot->AddParameterS("cardtype","sis3316card");
    cslot->AddParameterO("card");
    
        
    NGMConfigurationParameter* cards = cslot->GetColumn("card");
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
        TString addrString;
        addrString.Form("0x%08x",0x31000000+icard*0x01000000);
        cslot->SetParameterO("card",icard,new sis3316card());
        cslot->SetParameterS("ModAddr",icard,addrString.Data());
        cslot->SetParameterS("cardtype",icard,"sis3316card");

        sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
        card->baseaddress = strtoul(cslot->GetParValueS("ModAddr",icard),0,0);
        printf("Slot %d BaseAddress 0x%08x\n",icard,card->baseaddress);
        cslot->SetParameterI("ModEnable",icard,1);
        cslot->SetParameterD("NanoSecondsPerSample",icard,4.0);
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

void SIS3316SystemMT::SetInterfaceType(const char* vmeinterfacestr)
{
    _vmeinterfacetype = vmeinterfacestr;
}

int SIS3316SystemMT::InitializeSystem()
{
    _sisreadlog.open("sisreadthread.log",std::ios::app);
    _writelog.open("siswritethread.log",std::ios::app);

    if(_vmeinterfacetype=="") _vmeinterfacetype="SISUSB";

    if(_config==0) {
      CreateDefaultConfig("SIS3316");
    }
    
    NGMConfigurationTable* cslot = _config->GetSlotParameters();

  std::cout << "SIS3316SystemMT::InitializeSystem():"
    << _numberOfSlots << " slots" << std::endl;

  std::cout << "_vmeinterfacetype: " << _vmeinterfacetype  << std::endl;

  TString sInterface(_vmeinterfacetype.c_str());

  if(_numberOfSlots==1){
    if(sInterface.Contains("eth"))
    {
      daqifmode=ethsingle;
      std::cout << "1: ethsingle" << endl;
    }else{
      daqifmode=masterbridge;
      std::cout << "1: masterbridge" << endl;
    }
  }else{
    if(sInterface.Contains("eth"))
    {
      daqifmode=ethmulti;
      std::cout << "ethmulti" << endl;
    }else{
      daqifmode=masterbridge;
      std::cout << "masterbridge" << endl;
    }
    std::cout << "daqifmode: " << daqifmode << endl;
  }
  
  if(daqifmode==masterbridge) {
    vmei = vme_interface_class::Factory(_vmeinterfacetype.c_str());
    vmei->vmeopen();
    if(vmei==0)
    {
      LOG<<"Unable to Find vme interface "<<ENDM_FATAL;
      return -1;
    }
  }
  else{
    vmei = 0;
    _broadcastbase = 0;
  }
  // std::vector<std::string> vEthType = {"sis3316_ethb","sis3316_ethb"}; // From Jason Oct-25 2017
  NGMConfigurationParameter* cards = cslot->GetColumn("card");
  for(int icard = 0; icard < cards->GetEntries(); icard++)
  {
    std::cout << "SIS3316SystemMT::InitializeSystem(): card " << icard << std::endl;
    if(cslot->GetParValueI("ModEnable",icard)==0) {
      std::cout << "\t disabled!" << std::endl;
      continue;
    }
    sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
    if(daqifmode==ethmulti || daqifmode == ethsingle) {
      vme_interface_class * vmei_eth = vme_interface_class::Factory(_vmeinterfacetype.c_str());
      TString IPaddr = cslot->GetParValueS("IPaddr",icard);
      std::cout << "card " << icard << ": _vmeinterfacetype: " << _vmeinterfacetype.c_str() << std::endl;
      std::cout<<"Setting IPAddress for card"<< icard <<" to " << IPaddr.Data()<<std::endl;
      vmei_eth->setDigitizerIPaddress(const_cast<char*>(IPaddr.Data()));
      //card->baseaddress = 0;
      vmei_eth->vmeopen();
      if(vmei_eth==0)
      {
        LOG<<"Unable to Find vme interface "<< IPaddr << ENDM_FATAL;
        return -1;
      }
      card->vmei=vmei_eth;
      if(icard==0){
        vmei = vmei_eth;
      }
    }
    else card->vmei=vmei;
    card->initcard(); //sets struck to its power up state. 
    card->AllocateBuffers(); //by default allocates 64 MB, the max size of hardware memory per channel. 
    if(_numberOfSlots>1)
    {
      if(icard==0)
      {
        std::cout << "Making card #" << icard << " Master " << std::endl;
        card->SetClockChoice(0,2);  //default is (0,2); changed into (1,2) since card 0 with SN 205 could only sample at 125 MS/s
      // card->SetClockChoice(0,2);  
      }
      else
      {
         std::cout << "Making card #" << icard << " Slave " << std::endl;
         card->SetClockChoice(0,1);  //default is (0,1); changed into (1,1) so that card 0 and 1 could sample at the same frequency
      // card->SetClockChoice(0,1); 
      } 
    }
    else
    {
     // card->SetClockChoice(1,0);
      card->SetClockChoice(0,0);   // default is (0,0);
    }
    card->SetBroadcastAddress(_broadcastbase,true,icard==0/*first card is broadcast master*/);
    Default3316Card(card);
    card->SetCardHeader(icard);
  }


  return 0;
}

void SIS3316SystemMT::Default3316Card(sis3316card* vslot)
{
    /*************Event Configuration Registers*******************/
    for(int iadc=0; iadc < SIS3316_ADCGROUP_PER_CARD;iadc++ ){
        vslot->gate_window_length_block[iadc]=200;
        vslot->pretriggerdelay_block[iadc]=50;
        vslot->sample_length_block[iadc]=200;
        vslot->sample_start_block[iadc]=0;
        vslot->pretriggerdelaypg_block[iadc]=0;
        for(int igate=0;igate<SIS3316_QDC_PER_CHANNEL;igate++){
            vslot->qdcstart[iadc][igate]=80;
            vslot->qdclength[iadc][igate]=10;
        }
        vslot->qdcstart[iadc][0]=0;
        vslot->qdclength[iadc][0]=60;
        vslot->qdcstart[iadc][1]=80;
        vslot->qdclength[iadc][1]=10;
        vslot->addressthreshold[iadc]=0x100000;
    }
    
    for (int iadc=0;iadc<SIS3316_ADCGROUP_PER_CARD;iadc++)
    {
        vslot->dataformat_block[iadc] = 0x05050505; //
    }
    vslot->nimtriginput=0x0; //Disable:0x0 Enable:0x1 Enable+Invert:0x3
    vslot->nimtrigoutput_to=0x0;
    vslot->nimtrigoutput_uo=0x0;

    for (int ichan=0;ichan<SIS3316_CHANNELS_PER_CARD;ichan++) {
        if (ichan<4)
        {
            vslot->trigconf[ichan] = 0x2 | 0x8;
        }else{
            vslot->trigconf[ichan] = 0x2 | 0x8;
        }
    }
    vslot->ConfigureEventRegisters();
    /************Configure Analog Registers*******************/
    for(int ic = 0; ic<SIS3316_CHANNELS_PER_CARD;ic++)
    {
        vslot->gain[ic]=1; //0:5VRange 1:2VRange
        vslot->termination[ic]=1;// 1:50Ohm 0:1kOhm
    }
    vslot->ConfigureAnalogRegisters();
    /*********Configure Triggering*****************************************/
    for (int ichan=0;ichan<SIS3316_CHANNELS_PER_CARD;ichan++) {
        vslot->risetime[ichan]=2;
        vslot->gaptime[ichan]=0;
        vslot->firenable[ichan]=0;
        vslot->highenergysuppress[ichan]=0;
        vslot->fircfd[ichan]=0x3;
        vslot->firthresh[ichan]=400;
        vslot->highenergythresh[ichan]=0;
    }
    for (int iadc=0;iadc<SIS3316_ADCGROUP_PER_CARD;iadc++) {
        vslot->risetime_block[iadc]=8;
        vslot->gaptime_block[iadc]=0;
        vslot->firenable_block[iadc]=1;
        vslot->highenergysuppress_block[iadc]=0;
        vslot->fircfd_block[iadc]=0x3;
        vslot->firthresh_block[iadc]=800;
        vslot->highenergythresh_block[iadc]=0;
    }
    vslot->ConfigureFIR();
    /***************************************************/
    vslot->EnableThresholdInterrupt();
}

int SIS3316SystemMT::ConfigureSystem()
{
    NGMConfigurationParameter* cards = _config->GetSlotParameters()->GetColumn("card");
    NGMConfigurationParameter* active = _config->GetSlotParameters()->GetColumn("ModEnable");
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
        cout << "SIS3316SystemMT::ConfigureSystem() card " << icard << endl;
	if(! active->GetValueI(icard)) continue;
        
        sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
        //card->SetBroadcastAddress(_broadcastbase,true,icard==0/*first card is broadcast master*/);
        card->ConfigureEventRegisters();
        card->ConfigureAnalogRegisters();
        card->ConfigureFIR();
        card->EnableThresholdInterrupt();
        if(card->coincidenceEnable)
            card->ConfigureCoincidenceTable();
        printf("Modid %x Firmware 0x%08x\n",card->modid,card->adcfirmware[0]);
        if(getDebug()) card->PrintRegisters();

    }
   return 0;
}

int SIS3316SystemMT::StartAcquisition()
{
    if(_isRunning) return -1;
    _maxfilecount = _config->GetSystemParameters()->GetParValueI("MaxFileCount",0);

    SetStopDaqOnCtlC();
    _isRunning = 1;
    _requestStop=false;
    _sigintStop=false;
    _totalEventsThisRun = 0;
    _firstTimeOfRun = TTimeStamp(0,0);
    _earliestTimeInSpill = TTimeStamp(0,0);
    _latestTimeInSpill = TTimeStamp(0,0);
    _lastPlotIndex = -1;
    _livetime = 0.0;
    _runduration = 0.0;
    _totalBytesWritten=0;
    _rawFileCount = 1;

    //This is a short delay to ensure the  cleartimestamp is issued near the half second of the system clock
    TTimeStamp ts1;
    ts1.Print();
    TTimeStamp ts2(ts1);
    if(ts1.GetNanoSec()*1e-9 > 0.25){
        ts2.SetSec(ts2.GetSec()+1);
    }
    ts2.SetNanoSec(500000000);
    while(ts1.AsDouble()<ts2.AsDouble()) ts1.Set();

    _config->SetTimeStampNow();
    delete _runBegin;
    _runBegin = new TTimeStamp(_config->GetTimeStamp());
    std::cout<<"SIS3316SystemMT - Config Timestamp: "<<_config->GetTimeStamp().AsString()<<std::endl;
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

    return (int)_sigintStop;
}

void* SIS3316SystemMT::StartRunPipelinedAcquisitionWriteThread(void * arg){
    SIS3316SystemMT* tSystem = reinterpret_cast<SIS3316SystemMT*>(arg);
    tSystem->RunPipelinedAcquisitionWriteThread();
    return 0;
}

void* SIS3316SystemMT::StartRunPipelinedAcquisitionReadThread(void * arg){
    SIS3316SystemMT* tSystem = reinterpret_cast<SIS3316SystemMT*>(arg);
    tSystem->RunPipelinedAcquisitionReadThread();
    return 0;
}

int SIS3316SystemMT::RunPipelinedAcquisition()
{
    SIS3316Buffer_st spillComplete;
    spillComplete.status = SIS3316Buffer_st::spillComplete;
    if(_prun){
        delete _prun;
    }
    _prun = new SIS3316SystemMTDaqRun();
    
    _prun->freePacketBuffers.push(spillComplete);
    
    //Start Write Thread
	TThread* wThread = new TThread("P16WriteThread",(TThread::VoidRtnFunc_t) &StartRunPipelinedAcquisitionWriteThread,this);
	//Start Read Thread
	TThread* rThread = new TThread("P16ReadThread",(TThread::VoidRtnFunc_t) &StartRunPipelinedAcquisitionReadThread,this);
	wThread->Run();
	rThread->Run();
	rThread->Join();
    LOG<<"Read thread complete"<<ENDM_INFO;
	wThread->Join();
    LOG<<"Write thread complete"<<ENDM_INFO;
    delete wThread;
    wThread = 0;
    delete rThread;
    rThread=0;
    
    return 0;
}

int SIS3316SystemMT::RunPipelinedAcquisitionReadThread()
{
    _sisreadlog<<"Begin RunPipelinedAcquisitionReadThread"<<std::endl;
    char messages[4096];
    NGMConfigurationParameter* cards = _config->GetSlotParameters()->GetColumn("card");

    // Reset Trigger Statistics Counters
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
        if(_config->GetSlotParameters()->GetParValueI("ModEnable",icard)==0) continue;

        sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
        _prun->vcards.push_back(card);
        card->ResetRunScalars();
    }
    sis3316card* firstcard = _prun->vcards[0];
    
    int return_code;
    
    // Clear Timestamp  */
    TTimeStamp ts1;
    _sisreadlog<<"Before Timestamp clear: "<< ts1.AsString()<<std::endl;
	return_code = firstcard->ClearTimeStamp();  //
    ts1.Set();
    _sisreadlog<<"After Timestamp clear: "<< ts1.AsString()<<std::endl;
    _sisreadlog<<"Diff from Config "<<ts1.AsDouble()-_runBegin->AsDouble()<<" seconds." <<std::endl;
    // Start Readout Loop  */
    return_code = firstcard->DisarmAndArmBank();  //primes the SIS3316 memory registers for data logging
    
    double maxLiveTime = _config->GetSystemParameters()->GetParValueD("MaxDuration", 0); //this is the 60 seconds or so specified in takeData
    double timeOfLastTemp = 0.0;
    const double timeToReadTemp = 10.0;
    const double maxTemp = 58.0;
    unsigned int triggerCount;
    unsigned int data_rd = 0;
    int poll_counter = 0;
    bool doForcedTriggers = false;
    bool singleCardAtThreshold = false;
    double maxTimeBetweenSpills = _config->GetSystemParameters()->GetParValueD("SpillDuration", 0);
    double timeOfLastSpillRead = 0.0;
    TTimeStamp curTime;
    cout << "Starting run with maxLiveTime " << maxLiveTime << endl;
    while(!_requestStop)
    {
        cout << "Listening for events in " << maxTimeBetweenSpills << " second interval..." << endl;
        triggerCount = 0;
        poll_counter = 0 ;
        singleCardAtThreshold = false;

        //iterate for a duration of "maxTimeBetweenSpills" seconds, unless some other
        //conditions arise
        while ((data_rd & 0xFE) == 0x00000 && !_requestStop && !singleCardAtThreshold
           && (_runduration-timeOfLastSpillRead < maxTimeBetweenSpills))
        {
            poll_counter++;
            //vmei->vme_IRQ_Status_read( &data_rd);
            //if(poll_counter>1) usleep(1000);

            for(int icard = 0; icard < _prun->vcards.size(); icard++){
                if(_prun->vcards[icard]->DataThresholdReached()){
                    curTime.Set();
                    _runduration = curTime.AsDouble()-_runBegin->AsDouble();

                    _sisreadlog<<"Data Treshold reached for card "<<icard<<" at "<<_runduration<<" seconds."<<std::endl;
                    singleCardAtThreshold = true;
                    break;
                }
            }

            //doForcedTrigger not implemented for us, but is like a software trigger
            if(vmei!=0&&doForcedTriggers&&poll_counter%1==0)
            {
                triggerCount++;
                return_code = vmei->vme_A32D32_write ( _broadcastbase + SIS3316_KEY_TRIGGER, 0x0);
                usleep(10);
                uint32_t data;
                for(int ichan = 0; ichan<1; ichan++)
                {
                    unsigned int prevBankEndingRegister = firstcard->baseaddress + SIS3316_ADC_CH1_ACTUAL_SAMPLE_ADDRESS_REG + 0x1000*(ichan/4)+ (ichan%4)*0x4;
                    return_code = vmei->vme_A32D32_read (prevBankEndingRegister,&data);
                    sprintf(messages,"Trigger Count %d Address 0x%08x %d",triggerCount,data,ichan);
                    _sisreadlog<<messages<<std::endl;
                }
            }


            curTime.Set();
            _runduration = curTime.AsDouble()-_runBegin->AsDouble();
            //if the run duration reaches the 50 seconds or so full run duration, end. 
            if(_runduration>maxLiveTime)
            {
                sprintf(messages,"Maximum RunTime Exceeded %f(%f)",_runduration,maxLiveTime);
                _sisreadlog<<messages<<std::endl;
                _requestStop = true;
            }
            //take temperatures at regular intervals
            if((_runduration-timeOfLastTemp)>timeToReadTemp)
            {
                timeOfLastTemp = _runduration;
                std::vector<double> vtemp;
                double tmaxtemp = 0.0;
                _sisreadlog<<"SIS3316 Temperature: ";
                for(int icard=0; icard<_prun->vcards.size();icard++){
                  vtemp.push_back(_prun->vcards[icard]->ReadTemp());
                  _sisreadlog<<vtemp[icard]<<"\t";
                  if(vtemp[icard]>tmaxtemp) tmaxtemp = vtemp[icard];
                }
                _sisreadlog<<std::endl;
                if(tmaxtemp>maxTemp)
                {
                _sisreadlog<<"SIS3316 Temperature Exceeding maximum of 52C < "<<tmaxtemp<<ENDM_FATAL;
                _requestStop = true;
                }
            }
            

            curTime.Set();
            _runduration = curTime.AsDouble()-_runBegin->AsDouble();
     
        } 
    

        curTime.Set();
        _runduration = curTime.AsDouble()-_runBegin->AsDouble();
        timeOfLastSpillRead = _runduration;
        sprintf(messages,"Run Duration %f seconds: Exiting Event Loop: 0x%08x %d %d",_runduration,data_rd,_requestStop,poll_counter);
        _sisreadlog<<messages<<std::endl;
    
        // This blocking pop tells us that the Writing thread from a previous spill
        // has completed.
        SIS3316Buffer_st isFillComplete;
        _sisreadlog<<"Waiting on previous buffer to be written"<<std::endl;
        _prun->freePacketBuffers.pop(isFillComplete);
        _sisreadlog<<"... Complete"<<std::endl;
        // Print status and current address
        if(getDebug())
        {
            int icard =0;
            _sisreadlog<<"Prior to bank switch ... "<<std::endl;
            for(auto tcard: _prun->vcards)
            {
              _sisreadlog<<icard
              <<"\t"<<std::hex<<tcard->GetAcquisitionControl()
              <<"\t"<<std::hex<<tcard->GetActualSampleAddress()
              <<std::dec
              <<std::endl;
              icard++;
            }
            _sisreadlog<<"... End Prior to bank switch. "<<std::endl;
        }


        firstcard->DisarmAndArmBank();

        // Print status and current address
        if(getDebug()){
            usleep(100);
            int icard =0;
            _sisreadlog<<"Post bank switch ... "<<std::endl;
            for(auto tcard: _prun->vcards)
            {
                _sisreadlog<<icard
                <<"\t"<<std::hex<<tcard->GetAcquisitionControl()
                <<"\t"<<std::hex<<tcard->GetActualSampleAddress()
                <<std::dec
                <<std::endl;
                icard++;
            }
            _sisreadlog<<"... End Post bank switch. "<<std::endl;
        }
        FetchData();

    }

    firstcard->Disarm();
    SIS3316Buffer_st endOfRun;
    endOfRun.status=SIS3316Buffer_st::endOfRun;
    _prun->filledPacketBuffers.push(endOfRun);
    _sisreadlog<<"End of Read Thread"<<std::endl;

    return 0;

}

int SIS3316SystemMT::RunPipelinedAcquisitionWriteThread()
{
	// Check for binary write
    _writelog<<"Begin write thread"<<std::endl;
    size_t written;
    int errorcode = 0;
    int spillhdr[10];
    spillhdr[0]=0xABBAABBA;//gl_uint_ModAddrRun[module_index];
    spillhdr[1]=0xABBAABBA;//dma_got_no_of_words;
    for(int ihdr = 2; ihdr < 10; ihdr++)
    {
        spillhdr[ihdr] = 0xABBAABBA;
    }
    spillhdr[9]=0xABBAABBA;//dma_got_no_of_words;
    SIS3316Buffer_st spillWritingDone;
    spillWritingDone.status=SIS3316Buffer_st::spillComplete;
    
	SIS3316Buffer_st nextBuf;
	while(1)
	{
		_prun->filledPacketBuffers.pop(nextBuf);
        
		if(nextBuf.status==SIS3316Buffer_st::endOfRun){
		  _writelog<<"Found End Of Run Signal"<<std::endl;
			break;
		}
        
        if(nextBuf.status==SIS3316Buffer_st::firstOfSpill){
            checkForIncrementingRawFile();
            if(nextBuf.skippedRead)
                spillhdr[1] = 0;
            else
                spillhdr[1] = 0xABBAABBA;
            
            written = fwrite(spillhdr,0x4,10,_rawFilePointer)*0x4;
            _totalBytesWritten+=written;
            _writelog<<"Begin writing next spill Total Previous Bytes("<<_totalBytesWritten<<") ";
            if(nextBuf.skippedRead) _writelog<<" Writing Skipped ";
            _writelog<<std::endl;
        }
        
        if(nextBuf.status==SIS3316Buffer_st::normal
           || nextBuf.status==SIS3316Buffer_st::firstOfSpill){
            if(nextBuf.chan==0)
            {
                unsigned int phdrid[2];
                phdrid[0] = _prun->vcards[nextBuf.slot]->adcheaderid[0]&0xFF000000;
                phdrid[1] = 0;
                if(nextBuf.skippedRead) phdrid[1] = 0x1;
                written+= fwrite(phdrid,0x4,2,_rawFilePointer)*0x4;
                errorcode = ferror(_rawFilePointer);
                if(errorcode)
                {
                    _writelog<<"sis3316card::WriteSpillToFile: Error writing spill header to binary file "<<errorcode<<std::endl;
                }
            }   
            written = _prun->vcards[nextBuf.slot]->WriteChannelToFile(nextBuf.chan, _rawFilePointer);
            _totalBytesWritten+=written;

        }
        if(nextBuf.status==SIS3316Buffer_st::spillComplete){

            _prun->freePacketBuffers.push(spillWritingDone);
            _writelog<<"Spill writing complete Total Bytes Written ("<<_totalBytesWritten<<")"<<std::endl;
        }
        
	}
    _writelog<<"Exiting Write Thread"<<std::endl;
	return 0;

}

int SIS3316SystemMT::StartListModeGeneral()
{
    return RunPipelinedAcquisition();
}

int SIS3316SystemMT::FetchData()
{
    bool firstBufferOfSpill = true;
    SIS3316Buffer_st sisbuffer;
    sisbuffer.status = SIS3316Buffer_st::firstOfSpill;
    double fullFraction = _config->GetSystemParameters()->GetParValueD("FullFraction", 0); //fraction of data buffer considered "full"
    bool isAnyChannelTooFull = false;
    for(int icard = 0; icard < _prun->vcards.size(); icard++)
    {
        sis3316card* card =_prun->vcards[icard];
        card->FetchScalars();
        for(int ichan = 0; ichan<SIS3316_CHANNELS_PER_CARD; ichan++){

            //this function will return 0x900 = 2304 if it fails to
            //"Verify that the previous bank address is valid", which is a bit vague. 
            double fractionOfBuffer = card->FetchDataSizeForChannel(ichan);
            
            if(fractionOfBuffer>fullFraction) isAnyChannelTooFull = true; //not always, sometimes its 0x900, but the outcome is also a skipped read. 
            if (isAnyChannelTooFull) std::cout << " Full channel found "<< ichan << " with fraction of buffer " << fractionOfBuffer << std::endl;
        }
    }
    sisbuffer.skippedRead = isAnyChannelTooFull;
    cout << "sisbuffer.skippedRead: " << isAnyChannelTooFull << endl;
    int read_retval = 0;
    for(int icard = 0; icard < _prun->vcards.size(); icard++)
    {
        sis3316card* card =_prun->vcards[icard];
        for(int ichan = 0; ichan<SIS3316_CHANNELS_PER_CARD; ichan++){
            if(!sisbuffer.skippedRead){
                read_retval = card->FetchDataOnlyForChannel(ichan);
            }
            sisbuffer.slot = icard;
            sisbuffer.chan = ichan;
            _prun->filledPacketBuffers.push(sisbuffer);

            if(firstBufferOfSpill){
                sisbuffer.status = SIS3316Buffer_st::normal;
                firstBufferOfSpill = false;
            }
        }
        card->LogScalars(_sisreadlog);
    }

    
    sisbuffer.status =SIS3316Buffer_st::spillComplete;
    _prun->filledPacketBuffers.push(sisbuffer);
    return 0;
}

int SIS3316SystemMT::ParseADCBlockData(sis3316card* vcard, int iblock)
{
    //These methods are implemented in NGMSIS3316RawReader
    return 0;
}

int SIS3316SystemMT::ParseSingleChannelData(sis3316card* vcard, int ichan)
{
    //These methods are implemented in NGMSIS3316RawReader
    return 0;
}

int SIS3316SystemMT::StopAcquisition()
{
    return 0;
}
int SIS3316SystemMT::RequestAcquisitionStop()
{
    printf("In SIS33316SystemMT.cc, request acquisition stop called");
    _requestStop = true;
    _sigintStop = true; //this is only called when sigint triggers, see NGMSystem class of which this derives
    return 0;
}
void SIS3316SystemMT::PlotAdcDistribution(int slot, int channel)
{
    return;
}
int SIS3316SystemMT::writePacket(int packetlength, unsigned int *data)
{
    return 0;
}
int SIS3316SystemMT::WriteGlobalToTable()
{
    return 0;
}
int SIS3316SystemMT::WriteTableToGlobal()
{
    return 0;
}

int SIS3316SystemMT::WriteSpillToFile()
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
    
    // Reset Trigger Statistics Counters
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
        if(! active->GetValueI(icard) ) continue;
        sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
        written = card->WriteSpillToFile(_rawFilePointer);
        _totalBytesWritten+=written;
    }


    return 0;
}

void SIS3316SystemMT::closeRawOutputFile()
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

void SIS3316SystemMT::openRawOutputFile()
{
    // Check that a previously opened file is
    // properly closed with a our end of file
    // word
    size_t written;
    closeRawOutputFile();
    
    // Open file and write run header
    unsigned int runhdr[100];
    for(int ihdr = 0; ihdr < 100; ihdr++) runhdr[ihdr]=0;
    runhdr[0]=0x33160001; // File Version
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
    outFileName+="SIS3316Raw_";
    outFileName+=_config->getRunNumber();

    if(_config->GetSystemParameters()->GetParIndex("OutputFileSuffix")>=0) 
    {
        outFileName+=_config->GetSystemParameters()->GetParValueS("OutputFileSuffix", 0);
    }
    
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

int SIS3316SystemMT::checkForIncrementingRawFile()
{
	// Lets check if we are approaching the 2G limit for the case of a raw binary file
	if(_rawFilePointer )
	{
		// Find the size of the file
		if(ftell(_rawFilePointer) > _maxbytestoraw)
		{
			//RequestAcquisitionStop();
			if(_rawFileCount<_maxfilecount)
			{
				_rawFileCount++;
				openRawOutputFile();
			}else{
				//We have collected all the requested data
				_writelog<<"Max filecount of "<<_maxfilecount<<" exceeded. Ending Run..."<<std::endl;
				_requestStop = true;
			}
		}
	}
    
	return 0;
}
