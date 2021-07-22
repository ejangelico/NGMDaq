#include <stdint.h>
#include "NGMSIS3316RawReader.h"
#include "NGMSystemConfiguration.h"
#include "NGMConfigurationTable.h"
#include "NGMConfigurationParameter.h"
#include "NGMBufferedPacket.h"
#include "NGMModule.h"
#include "NGMSystem.h"
#include "NGMLogger.h"
#include "sis3316card.h"
#include "TThread.h"
#include "TFile.h"
#include "TSystem.h"
#include "TObjString.h"
#include "TString.h"
#include "TCanvas.h"
#include "TROOT.h"
#include <cmath>
#include <queue>
#include <list>
#include "NGMTimingCal.h"
#include "TH1.h"
#include "TCanvas.h"
#include "NGMSimpleParticleIdent.h"

ClassImp(NGMSIS3316RawReader)

class sis3316evtInfo
{
public:
    sis3316evtInfo(unsigned long long trawclock,
                   unsigned short ticard,
                   unsigned short tichan,
                   size_t tevtposition);
    static int ParseEventData(sis3316card* vcard, unsigned short card, unsigned short chan,std::queue<sis3316evtInfo> &evtList);
    static void AddToList(sis3316evtInfo evt,std::list<sis3316evtInfo> &evtList);
    unsigned long long rawclock;
    unsigned short icard;
    unsigned short ichan;
    size_t evtposition;
};
sis3316evtInfo::sis3316evtInfo(unsigned long long trawclock,
                               unsigned short ticard,
                               unsigned short tichan,
                               size_t tevtposition)
:rawclock(trawclock), icard(ticard), ichan(tichan),evtposition(tevtposition)
{}



int sis3316evtInfo::ParseEventData(sis3316card* vcard, unsigned short card, unsigned short chan,std::queue<sis3316evtInfo> &evtList)
{
    int block = chan/SIS3316_CHANNELS_PER_ADCGROUP;
    int dataformat = vcard->dataformat_block[block];
    int nomEventSize = 3;
    if(dataformat&0x1) nomEventSize+=7;
    if(dataformat&0x2) nomEventSize+=2;
    if(dataformat&0x4) nomEventSize+=3;
    int nevents = 0;
    unsigned int numberOfSamples = 0;
    unsigned int indexOfNextEvent = 0;
    while(indexOfNextEvent+nomEventSize<vcard->databufferread[chan])
    {
        numberOfSamples=vcard->databuffer[chan][indexOfNextEvent+nomEventSize-1]&0x03FFFFFF;
        ULong64_t rawclock = vcard->databuffer[chan][indexOfNextEvent+1];
        rawclock=rawclock | ((((ULong64_t)(vcard->databuffer[chan][indexOfNextEvent])) & 0xFFFF0000)<<16);
        sis3316evtInfo tevt(rawclock,card,chan,indexOfNextEvent);
        //printf("Nevents %d %lld %d %d %ld\n ",nevents,tevt.rawclock,tevt.icard,tevt.ichan,tevt.evtposition);
        evtList.push(tevt);
        
        // next event
        indexOfNextEvent+=nomEventSize+numberOfSamples;
        nevents++;
    }
    //printf("Nevents (%d,%d) %d\n ",card,chan,nevents);
    return nevents;
}

void sis3316evtInfo::AddToList(sis3316evtInfo evt,std::list<sis3316evtInfo> &evtList)
{
    for(std::list<sis3316evtInfo>::iterator itr = evtList.begin(); itr!=evtList.end(); itr++)
    {
        if(evt.rawclock<itr->rawclock)
        {
            evtList.insert(itr,evt);
            return;
        }
    }
    evtList.push_back(evt);
}


NGMSIS3316RawReader::NGMSIS3316RawReader()
{
    _verbosity = 0;
	_runBegin = TTimeStamp(0,0);
    _nextbufferPosition = 0;
    _runduration = 0;
    _inputfile = 0;
    _inputfilesize = 0;
    _psdScheme = 0;
        _hitPool = 0;
}

NGMSIS3316RawReader::NGMSIS3316RawReader(NGMModule* parent)
: NGMReaderBase(parent)
{
	_runBegin = TTimeStamp(0,0);
    _nextbufferPosition = 0;
    _runduration = 0;
    _inputfile = 0;
    _inputfilesize = 0;
    _filecounter = 0;
    _verbosity = 0;
    _psdScheme = 0;
    _hitPool = new NGMHitPoolv6;
}

NGMSIS3316RawReader::~NGMSIS3316RawReader()
{
    CloseInputFile();
    delete     _hitPool;
}

UInt_t NGMSIS3316RawReader::sumarray(int alength, unsigned short * arrptr)
{
    uint32_t sum = 0;
    for(int i = 0; i<alength; i++) sum+=arrptr[i];
    return sum;
}

void NGMSIS3316RawReader::GetStatus(TString &sStatus)
{
    sStatus="";
    return;
}

void NGMSIS3316RawReader::SetConfiguration(const NGMSystemConfiguration* sysConf)
{
    // Let Base keep a copy of configuration
    NGMReaderBase::SetConfiguration(sysConf);
    
    // Extract Clock Settings for each slot
    NGMConfigurationTable* cslot = GetConfiguration()->GetSlotParameters();
      
    NGMConfigurationParameter* cards = cslot->GetColumn("card");
    _NanoSecondsPerClock.Set(cslot->GetEntries());
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
        if(! sysConf->GetSlotParameters()->GetParValueI("ModEnable",icard)) continue;
        sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
        _NanoSecondsPerClock[icard]=4.0;
        if(card->clock_source_choice==0)
            _NanoSecondsPerClock[icard]=4.0;
        card->AllocateBuffers(0);
        card->ResetRunScalars();
    }

    if(_tcal){
        if(!_tcal->_partId) _tcal->_partId=new NGMSimpleParticleIdent();
        _tcal->_partId->Init(sysConf);
        _tcal->SetNChannels(GetConfiguration()->GetChannelParameters()->GetEntries());
	if(GetConfiguration()->GetDetectorParameters()->GetParIndex("CalTimingOffset")>=0){
	  for(int ichan = 0; ichan< GetConfiguration()->GetChannelParameters()->GetEntries();ichan++){
	    
	    TString detName( GetConfiguration()->GetChannelParameters()->GetParValueS("DetectorName",ichan));
	    
	    int detRow =  GetConfiguration()->GetDetectorParameters()->FindFirstRowMatching("DetectorName",detName.Data());
	    if(detRow>=0){
	      LOG<<"Setting timing offset for detector "<<detName.Data()<<" to "<< GetConfiguration()->GetDetectorParameters()->GetParValueD("CalTimingOffset",detRow)<<ENDM_INFO;
	      _tcal->SetOffset(ichan,  GetConfiguration()->GetDetectorParameters()->GetParValueD("CalTimingOffset",detRow));
	    }
	  }
	}
    }

    // Initialize other beginning of run variables
    _runBegin = _config->GetTimeStamp();
    _firstTimeOfRun = TTimeStamp(0,0);
    _lastPlotIndex = -1;
    _runduration = 0;
    _livetime = 0;
    _totalEventsThisRun = 0;
    
}

Long64_t NGMSIS3316RawReader::OpenInputFile(const char* filename)
{
    TThread::Lock();
    CloseInputFile();
    _filecounter = 1;
    
    _inputfilename = filename;
    // Check if path has been specified with or without -conf.root or with _1.bin
    if(_inputfilename.EndsWith("-conf.root"))
    {
        _inputfilename.ReplaceAll("-conf.root","");
    }
    // Check if path has been specified with or without -conf.root or with _1.bin
    if(_inputfilename.EndsWith("_1.bin"))
    {
        _inputfilename.ReplaceAll("_1.bin","");
    }
    
    TString confFileName = _inputfilename+"-conf.root";
    TString rawFileName = _inputfilename;
    rawFileName+="_";
    rawFileName+=_filecounter;
    rawFileName+=".bin";
    
    //Lets look for a file of the same name except ".root" -> "-cal.root"
    TString dirName(gSystem->DirName(confFileName));
    TString fname(gSystem->BaseName(confFileName));
    fname.ReplaceAll(".root","-cal.root");
    TString caldir(gSystem->Getenv("NGMCALDIR"));
    if(caldir=="") caldir =dirName;
    TString calName = caldir+"/"+fname;
    if(!gSystem->AccessPathName(calName))
    {
        LOG<<"Using configuration object from "<<calName.Data()<<ENDM_INFO;
        confFileName=calName;
    }else{
        LOG<<"Unable to find "<<calName.Data()<<ENDM_WARN;
    }

    
    TFile* confFile = TFile::Open(confFileName.Data());
    if(!confFile)
    {
        LOG<<"File not found : "<<confFileName.Data()<<ENDM_WARN;
        return 1;
    }
    
    
    NGMSystemConfiguration* confBuffer = (NGMSystemConfiguration*)(confFile->Get("NGMSystemConfiguration"));
    
    // Save information we need from configuration
    SetConfiguration(confBuffer);
    
    OpenRawBinaryFile(rawFileName.Data());
    TThread::UnLock();
    return 0;
}

Long64_t NGMSIS3316RawReader::OpenRawBinaryFile(const char* pathname)
{
    FileStat_t pathinfo;
    gSystem->GetPathInfo(pathname,pathinfo);
    if(_inputfile) fclose(_inputfile);
    _inputfile = fopen(pathname,"rb");

    _nextbufferPosition = 0;
    
    if(!_inputfile)
    {
        LOG<<" Unable to open file "<<pathname<<ENDM_WARN;
        return 2;
    }

    // Get file size
    fseek(_inputfile,0,SEEK_END);
    _inputfilesize = ftell(_inputfile);
    rewind(_inputfile);
    
    // Read header
    const int wordsInRunHeader = 100;
    unsigned int runhdr[100];
    Long64_t result = sis3316card::readFileWaiting(runhdr, 0x4, 100, _inputfile);
    if(result!=wordsInRunHeader)
    {
        LOG<<" Full Header not found in buffer "<<pathname<<ENDM_INFO;
        return 1;
    }
    if((runhdr[0]&0xFFFF0000)!=0x33160000)
    {
        LOG<<"File does not appear to be a SIS3316 raw file"<<ENDM_FATAL;
        return 3;
    }
    
    LOG<<"Found SIS Run Header of version "<<runhdr[0] << ENDM_INFO;
    
    return 0;
}


Long64_t NGMSIS3316RawReader::CloseInputFile()
{
    
    if(_inputfile) fclose(_inputfile);
    _inputfile = 0;
    return 0;
}

Long64_t NGMSIS3316RawReader::ReadNextSpillFromFile()
{
    size_t bytesread;
    int spillhdr[10];
    
    bytesread = sis3316card::readFileWaiting(spillhdr,0x4,10,_inputfile)*0x4;

    //Spill header is the same length as endoffile record.
    // Check if spillheader is 0x0E0F0E0F;//ENDOFFILE
    if(spillhdr[0] == 0x0E0F0E0F)
    {
        LOG<<"Found End of File Record\n"<<ENDM_INFO;
        //Check for subsequent file
        TString rawFileName = _inputfilename;
        rawFileName+="_";
        rawFileName+= ++_filecounter;
        rawFileName+=".bin";

        if( OpenRawBinaryFile(rawFileName.Data()) != 0)  return -1;
        //Looks like we found a subsequent file, lets check the spillhdr
        bytesread = sis3316card::readFileWaiting(spillhdr,0x4,10,_inputfile)*0x4;
        // Sanity Check if spillheader is 0xABBAABBA;
        if(spillhdr[0] != 0xABBAABBA)
        {
            LOG<<"Inconsistent Spill Header "<<LOG_HEX<<spillhdr[0]<<LOG_DEC<<ENDM_FATAL;
            return -2;
        }

    }
    
    // Sanity Check if spillheader is 0xABBAABBA;
    if(spillhdr[0] != 0xABBAABBA)
    {
        LOG<<"Inconsistent Spill Header "<<LOG_HEX<<spillhdr[0]<<LOG_DEC<<ENDM_FATAL;
        return -2;
    }

    NGMConfigurationTable* cslot = GetConfiguration()->GetSlotParameters();
    NGMConfigurationParameter* cards = cslot->GetColumn("card");
    NGMConfigurationParameter* modenable = cslot->GetColumn("ModEnable");
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
        if(! modenable->GetValueI(icard)) continue;
        sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
        bytesread+=card->ReadSpillFromFile(_inputfile);
    }
    long curbytes = ftell(_inputfile);
    if (curbytes/(_inputfilesize/10) != (curbytes-bytesread)/(_inputfilesize/10)) {
        std::cout << "Read " << bytesread << " bytes; currently " << 100*ftell(_inputfile)/_inputfilesize << "% done with current file("<<_filecounter<<")." << std::endl;
        // Recheck file size in case we're taking data
        fpos_t curpos;
        fgetpos(_inputfile, &curpos);
        fseek(_inputfile,0,SEEK_END);
        _inputfilesize = ftell(_inputfile);
        fsetpos(_inputfile,&curpos);
    }
    if(0){ //Plot Scalars
        static TH1* hScalars = 0;
        static TH1* hScalars2 = 0;
        static TH1* hScalars3 = 0;
        static TH1* hScalars4 = 0;
        int nchannels = cards->GetEntries()*SIS3316_CHANNELS_PER_CARD;
        if(!hScalars)
        {
            hScalars = new TH1D("hScalars","Scalars",nchannels, 0, nchannels);
            hScalars->SetDirectory(0);
        }
        if(!hScalars2){
            hScalars2 = new TH1D("hScalars2","Scalars",nchannels, 0, nchannels);
            hScalars2->SetLineColor(kRed);
            hScalars2->SetDirectory(0);
        }
        if(!hScalars3){
            hScalars3 = new TH1D("hScalars3","Scalars",nchannels, 0, nchannels);
            hScalars3->SetLineColor(kGreen);
            hScalars3->SetDirectory(0);
        }
        if(!hScalars4){
            hScalars4 = new TH1D("hScalars4","Scalars",nchannels, 0, nchannels);
            hScalars4->SetLineColor(kCyan);
            hScalars4->SetDirectory(0);
        }        std::vector<unsigned int> s1,s2,s3,s4;
        for(int icard = 0; icard < cards->GetEntries(); icard++)
        {
            if(! modenable->GetValueI(icard)) continue;
            sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
            card->GetScalars(s1,s2,s3,s4);
            for(int ichan = 0; ichan<SIS3316_CHANNELS_PER_CARD;ichan++){
                hScalars->SetBinContent(icard*SIS3316_CHANNELS_PER_CARD+ichan+1,s1[ichan]);
                hScalars2->SetBinContent(icard*SIS3316_CHANNELS_PER_CARD+ichan+1,s2[ichan]);
                hScalars3->SetBinContent(icard*SIS3316_CHANNELS_PER_CARD+ichan+1,s3[ichan]);
                hScalars4->SetBinContent(icard*SIS3316_CHANNELS_PER_CARD+ichan+1,s4[ichan]);
            }
        }
        hScalars->DrawCopy();
        hScalars2->DrawCopy("same");
        hScalars3->DrawCopy("same");
        hScalars4->DrawCopy("same");
        gSystem->ProcessEvents();
        gPad->Modified();
        gPad->Update();
        gSystem->ProcessEvents();
    }
    return bytesread;
}

Long64_t NGMSIS3316RawReader::SortSpill()
{
    NGMConfigurationTable* cslot = GetConfiguration()->GetSlotParameters();
    NGMConfigurationParameter* cards = cslot->GetColumn("card");
    NGMConfigurationParameter* modenable = cslot->GetColumn("ModEnable");
    TObjString endSpillFlush("EndSpillFlush");
    if(!_hitPool) _hitPool=new NGMHitPoolv6();
    std::vector<sis3316card*> lcard;
    for(int icard=0; icard<cards->GetEntries(); icard++)
    {
        sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
        lcard.push_back(card);
    }
    std::vector< std::queue<sis3316evtInfo> > eventList;
    //printf("Begin Parsing Event Headers Buffers(%ld)\n",eventList.size());
    
    for(int icard=0; icard<lcard.size(); icard++)
    {
        for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
        {
            int chanseq = icard*SIS3316_CHANNELS_PER_CARD + ichan;
            std::queue<sis3316evtInfo> tList;
            eventList.push_back(tList);
            //printf("Parsed Event Headers Slot(%d) Chan(%d) ChanSeq(%d)\n",icard, ichan,chanseq);
            if(!lcard[icard]) continue;
            sis3316evtInfo::ParseEventData(lcard[icard],icard,ichan,
                                           eventList[chanseq]);
            //printf("Parsed Event Headers Slot(%d) Chan(%d) ChanSeq(%d) Nevents(%ld)\n",
            //       icard, ichan,chanseq, eventList[chanseq].size());
        }
    }
    
    //Lets loop through all queues and pop the head into a sorted map keyed on clock
    std::list<sis3316evtInfo> headSort;
    std::vector<sis3316evtInfo> sortedList;
    
    for(int icard=0; icard<lcard.size(); icard++)
    {
        for(int iadc = 0; iadc < SIS3316_ADCGROUP_PER_CARD; iadc++)
        {
            if(lcard[icard]->IsBlockReadout(iadc)){
                // Lets perform a one time check on the data to verify that the
                // Block Triggered Data Is Synchronized among the four channels
                // in the block
                //TODO: Implement check
                int ichan = iadc*SIS3316_CHANNELS_PER_ADCGROUP;
                int chanseq = icard*SIS3316_CHANNELS_PER_CARD + ichan;
                if(eventList[chanseq].size()>0){
                    sis3316evtInfo::AddToList( eventList[chanseq].front(),headSort);
                    eventList[chanseq].pop();
                }
            }else{
                for(int ic = 0; ic<SIS3316_CHANNELS_PER_ADCGROUP; ic++){
                    int ichan = iadc*SIS3316_CHANNELS_PER_ADCGROUP+ic;
                    int chanseq = icard*SIS3316_CHANNELS_PER_CARD + ichan;
                    if(eventList[chanseq].size()>0){
                        sis3316evtInfo::AddToList( eventList[chanseq].front(),headSort);
                        eventList[chanseq].pop();
                    }
                }
            }
        }
    }
    // now we cycle through
    while(!headSort.empty())
    {
        sis3316evtInfo nextEvt = headSort.front();
        headSort.pop_front();
        int chanseq = nextEvt.icard*SIS3316_CHANNELS_PER_CARD + nextEvt.ichan;
        NGMHit* hit = _hitPool->GetHit();
        if( lcard[nextEvt.icard]->IsBlockReadout(nextEvt.ichan/SIS3316_CHANNELS_PER_ADCGROUP)){
            ParseBlockEvent(lcard[nextEvt.icard],nextEvt.ichan,nextEvt.evtposition,hit);
        }else{
            ParseEvent(lcard[nextEvt.icard],nextEvt.ichan,nextEvt.evtposition,hit);
        }

        if(_maxruntime > 0.0 && _maxruntime < hit->TimeDiffNanoSec(_runBegin)*1e-9)
        {
            _abortread = true;
            break;
        }
        
        if(_tcal&&GetParent())
        {
            _tcal->pushHit(hit);
            NGMHit* nexthit = 0;
            while( (nexthit =_tcal->nextHit()) )
            {
                GetParent()->push(*((const TObject*)(nexthit)));
                _hitPool->ReturnHit( nexthit);
            }
        }else if(GetParent()){
            GetParent()->push(*((const TObject*)(hit)));
            _hitPool->ReturnHit(hit);
        }

        if(!eventList[chanseq].empty())
        {
            sis3316evtInfo::AddToList( eventList[chanseq].front(),headSort);
            eventList[chanseq].pop();
        }
    }
    if(GetParent()){
        gSystem->ProcessEvents();
        GetParent()->push(*((const TObject*)&endSpillFlush));
        ((NGMSystem*)GetParent())->ProcessSpyServ();
    }
    
    return 0;
}



Long64_t NGMSIS3316RawReader::ReadAll()
{
    
    NGMConfigurationTable* cslot = GetConfiguration()->GetSlotParameters();
    NGMConfigurationParameter* cards = cslot->GetColumn("card");
    NGMConfigurationParameter* modenable = cslot->GetColumn("ModEnable");
    NGMBufferedPacket* packet = new NGMBufferedPacketv2(0,0,TTimeStamp((time_t)0,0),6);
    TObjString endSpillFlush("EndSpillFlush");

    while(ReadNextSpillFromFile()>0&&!_abortread)
    {
        // Early data has a bug that can result in a mixing of old and new buffers
        // Lets do some inefficienct sanity checks for now...
        bool pushPackets = false;
        if(pushPackets){
            bool goodData = true;
            for(int icard=0; icard<cards->GetEntries(); icard++)
            {
                if(modenable->GetValueI(icard))
                {
                    sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
                    for(int iadc = 0; iadc <SIS3316_ADCGROUP_PER_CARD; iadc++)
                    {
                        if(card->IsBlockReadout(iadc))
                        {
                            std::vector<unsigned long long> evtPointer[SIS3316_CHANNELS_PER_ADCGROUP];
                            std::map<unsigned long long,int> evtid[SIS3316_CHANNELS_PER_ADCGROUP];
                            int parsedEvents[SIS3316_CHANNELS_PER_ADCGROUP];
                            //Sanity Checks
                            for(int ic=0; ic<SIS3316_CHANNELS_PER_ADCGROUP; ic++)
                            {
                                parsedEvents[ic] = ParseEventDataSingle(card,
                                                                        iadc*SIS3316_CHANNELS_PER_ADCGROUP+ic,
                                                                        evtPointer[ic],
                                                                        evtid[ic]);
                                if(ic>0 && parsedEvents[ic]!=parsedEvents[ic-1]) goodData = false;
                            }
                            
                        }else{
                            for(int ic = 0; ic<SIS3316_CHANNELS_PER_ADCGROUP; ic++)
                            {
                                
                                int ichan = iadc*SIS3316_CHANNELS_PER_ADCGROUP + ic;
                                std::vector<unsigned long long> evtPointer;
                                std::map<unsigned long long,int> evtid;
                                int parsedEvents = ParseEventDataSingle(card,ichan,evtPointer,evtid);
                                if(parsedEvents == 0) continue;
                                if( ParseClock(&(card->databuffer[ichan][evtPointer[0]])) > ParseClock(&(card->databuffer[ichan][evtPointer[parsedEvents-1]]))) goodData=false;
                            }
                        }
                    }
                }
            }
            if(!goodData)
            {
                LOG<<"Skipping spill with inconsistent data"<<ENDM_WARN;
                continue;
            }
            for(int icard=0; icard<cards->GetEntries(); icard++)
            {
                if(modenable->GetValueI(icard))
                {
                    sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(icard));
                    for(int iadc = 0; iadc <SIS3316_ADCGROUP_PER_CARD; iadc++)
                    {
                        if(card->IsBlockReadout(iadc))
                        {
                            ParseADCBlockData(card, iadc, packet);
                            if(GetParent())
                            {
                                GetParent()->push(*((const TObject*)(packet)));
                            }
                            packet->Clear();
                        }else{
                            for(int ic = 0; ic<SIS3316_CHANNELS_PER_ADCGROUP; ic++)
                            {
                                ParseSingleChannelData(card, iadc*SIS3316_CHANNELS_PER_ADCGROUP+ic, packet);
                                GetParent()->push(*((const TObject*)(packet)));
                                packet->Clear();
                            }
                        }
                    }
                }
            }
            if(GetParent()){
                GetParent()->push(*((const TObject*)&endSpillFlush));
                ((NGMSystem*)GetParent())->ProcessSpyServ();
            }
        }else{
            SortSpill();
        }


    }
    
    return 0;
}

Long64_t NGMSIS3316RawReader::parsePacket(int slot, int chan, NGMBufferedPacket* packet)
{
    NGMConfigurationTable* cslot = GetConfiguration()->GetSlotParameters();
    NGMConfigurationParameter* cards = cslot->GetColumn("card");

    sis3316card* card = dynamic_cast<sis3316card*>(cards->GetValueO(slot));
    int iadc=chan/SIS3316_CHANNELS_PER_ADCGROUP;
    if(card->IsBlockReadout(iadc))
    {
        ParseADCBlockData(card,iadc,packet);
    }else{
        ParseSingleChannelData(card,chan,packet);
    }
    
    
    return 0;
}

int NGMSIS3316RawReader::ParseEventDataSingle(sis3316card* vcard, int ichan, std::vector<unsigned long long> &evtPointer, std::map<unsigned long long,int> &evtid )
{
    evtPointer.clear();
    evtid.clear();
    
    int block = ichan/SIS3316_CHANNELS_PER_ADCGROUP;
    int dataformat = vcard->dataformat_block[block];
    int nomEventSize = 3;
    if(dataformat&0x1) nomEventSize+=7;
    if(dataformat&0x2) nomEventSize+=2;
    if(dataformat&0x4) nomEventSize+=3;
    int nevents = 0;
    unsigned int numberOfSamples = 0;
    unsigned int indexOfNextEvent = 0;
    while(indexOfNextEvent+nomEventSize<vcard->databufferread[ichan])
    {
        numberOfSamples=vcard->databuffer[ichan][indexOfNextEvent+nomEventSize-1]&0x03FFFFFF;
        ULong64_t rawclock = vcard->databuffer[ichan][indexOfNextEvent+1];
        rawclock=rawclock | ((((ULong64_t)(vcard->databuffer[ichan][indexOfNextEvent])) & 0xFFFF0000)<<16);
        evtPointer.push_back(indexOfNextEvent);
        evtid[rawclock]=nevents;

        // next event
        indexOfNextEvent+=nomEventSize+numberOfSamples;
        nevents++;
    }
    return nevents;
}

int NGMSIS3316RawReader::ParseSingleChannelData(sis3316card* vcard, int ichan,NGMBufferedPacket* packet)
{
    int block = ichan/SIS3316_CHANNELS_PER_ADCGROUP;
    int dataformat = vcard->dataformat_block[block];
    int nomEventSize = 3;
    if(dataformat&0x1) nomEventSize+=7;
    if(dataformat&0x2) nomEventSize+=2;
    if(dataformat&0x4) nomEventSize+=3;
    
    packet->Clear();
    packet->setSlotId((vcard->adcheaderid[block])>>24);
    packet->setChannelId(ichan);
    
    std::vector<unsigned long long> evtPointer;
    std::map<unsigned long long,int> evtid;
    int parsedEvents = ParseEventDataSingle(vcard,ichan,evtPointer,evtid);
    if(parsedEvents == 0) return 0;
    bool goodData = true;
    if( ParseClock(&(vcard->databuffer[ichan][evtPointer[0]])) > ParseClock(&(vcard->databuffer[ichan][evtPointer[parsedEvents-1]]))) goodData=false;

    if(!goodData || _verbosity>10)
    {
        LOG<<"Chan: "<<ichan;
        LOG<<" "<<parsedEvents;
        //        std::map<unsigned long long,int>::iterator evtiter = evtid[ic].begin();
        //        for(int ievt=0; ievt<min(10,parsedEvents[ic]);ievt++){
        //            LOG<< " ("<<evtiter->second<<") "<<evtiter->first;
        //            evtiter++;
        //        }
        for(int ievt=0; ievt<fmin(10,parsedEvents);ievt++){
            LOG<<" "<<ParseClock(&(vcard->databuffer[ichan][evtPointer[ievt]]));
        }
        LOG<<" ... ";
        for(int ievt=parsedEvents-10; ievt<parsedEvents;ievt++){
            if(ievt<0) continue;
            LOG<<" "<<ParseClock(&(vcard->databuffer[ichan][evtPointer[ievt]]));
        }
        LOG<<ENDM_INFO;
    }
    
    if(!goodData) return -1;
   
    int nevents = 0;
    unsigned int numberOfSamples = 0;
    unsigned int indexOfNextEvent = 0;
    int maw1, maw2, mawmax;
    double nsperclock=4.0;
    if(vcard->clock_source_choice==0)
        nsperclock=4.0;

    while(indexOfNextEvent+nomEventSize<vcard->databufferread[ichan])
    {
        numberOfSamples=vcard->databuffer[ichan][indexOfNextEvent+nomEventSize-1]&0x03FFFFFF;
        NGMHit* hit = packet->addHit();
        hit->SetSlot(packet->getSlotId());
        hit->SetChannel(ichan);
        //For all data formats we have timestamp
        ULong64_t rawclock = vcard->databuffer[ichan][indexOfNextEvent+1];
        rawclock=rawclock | ((((ULong64_t)(vcard->databuffer[ichan][indexOfNextEvent])) & 0xFFFF0000)<<16);
        hit->SetRawClock(rawclock);
        NGMTimeStamp rawTime(_runBegin);
        rawTime.IncrementNs(rawclock*nsperclock);
        hit->SetTimeStamp(rawTime);
        hit->SetPileUpCounter( ((vcard->databuffer[ichan][indexOfNextEvent+3])>>24) & 0xFF);
        if(dataformat&0x1)
        {
            double sum = 0.0;
            double psdsum = 0.0;
            int igate = 1;
            int psdgate = 2;
            if(dataformat&0x4)
            {
                int mawoffset=9;
                if(dataformat&0x2)
                    mawoffset+=2;
                mawmax = vcard->databuffer[ichan][indexOfNextEvent+mawoffset]-0x8000000;
                maw1 =  vcard->databuffer[ichan][indexOfNextEvent+mawoffset+1]-0x8000000;
                maw2 = vcard->databuffer[ichan][indexOfNextEvent+mawoffset+2]-0x8000000;
                float cfd = 1.0+(mawmax*0.5-maw2)/float(maw2-maw1);
                if(cfd>1.0) cfd=1.0;
                if(cfd<0.0) cfd=0.0;
                hit->SetCFD(cfd);
                rawTime.IncrementNs(cfd*nsperclock);
                hit->SetRawTime(rawTime);
            }

            double baseline = (vcard->databuffer[ichan][indexOfNextEvent+nomEventSize] & 0xFFFFFF)/(double)(vcard->qdclength[block][0]);
            sum = (vcard->databuffer[ichan][indexOfNextEvent+igate+nomEventSize]& 0xFFFFFFF) - baseline*(vcard->qdclength[block][igate]);
            psdsum= (vcard->databuffer[ichan][indexOfNextEvent+psdgate+nomEventSize]& 0xFFFFFFF) - baseline*(vcard->qdclength[block][psdgate]);


            //hEnergy->Fill(sum);
            //hPSDEnergy->Fill(sum,psdsum/sum);
            hit->SetPulseHeight(sum);
            hit->SetPSD(psdsum);
            hit->SetPileUpCounter((vcard->databuffer[ichan][indexOfNextEvent+3])>>24);
        }
        
        unsigned short* wf=(unsigned short*)(&(vcard->databuffer[ichan][indexOfNextEvent+nomEventSize]));

        hit->SetNSamples(numberOfSamples*2);
        for(int isample=0; isample<numberOfSamples*2;isample++)
        {
            hit->SetSample(isample,
                           wf[isample]);
        }
        
        indexOfNextEvent+=nomEventSize+numberOfSamples;
        nevents++;
    }
    return 0;
}

ULong64_t NGMSIS3316RawReader::ParseClock(unsigned int* data)
{
    return  ((ULong64_t)(data[1])) | (((ULong64_t)(data[0] & 0xFFFF0000))<<16);
}

int NGMSIS3316RawReader::ParseADCBlockData(sis3316card* vcard, int iblock,NGMBufferedPacket* packet)
{
    int dataformat = vcard->dataformat_block[iblock];
    int nomEventSize = 3;
    if(dataformat&0x1) nomEventSize+=7;
    if(dataformat&0x2) nomEventSize+=2;
    if(dataformat&0x4) nomEventSize+=3;
    int qdcoffset= 3;
    
    std::vector<unsigned long long> evtPointer[SIS3316_CHANNELS_PER_ADCGROUP];
    std::map<unsigned long long,int> evtid[SIS3316_CHANNELS_PER_ADCGROUP];
    int parsedEvents[SIS3316_CHANNELS_PER_ADCGROUP];
    //Sanity Checks
    bool goodData=true;
    for(int ic=0; ic<SIS3316_CHANNELS_PER_ADCGROUP; ic++)
    {
        parsedEvents[ic] = ParseEventDataSingle(vcard,
                                                iblock*SIS3316_CHANNELS_PER_ADCGROUP+ic,
                                                evtPointer[ic],
                                                evtid[ic]);
        if(ic>0 && parsedEvents[ic]!=parsedEvents[ic-1]) goodData = false;
    }
    if(!goodData)
    {
        LOG<<"Datawords for block "<<iblock<<" ";
        for(int ipmt=0; ipmt<4; ipmt++)
        {
            LOG<<vcard->databufferread[iblock*4+ipmt]<<" ";
        }
        LOG<<ENDM_WARN;
        for(int ipmt=0; ipmt<4; ipmt++)
        {
            for(int istat = 0; istat<SIS3316_TRIGGER_STATS_PER_CHANNEL; istat++)
                LOG<<vcard->triggerstatspill[iblock*4+ipmt][istat]<<"\t";
            LOG<<ENDM_WARN;
        }
     }
    if(_verbosity>10||!goodData)
    {
        for(int ic=0; ic<SIS3316_CHANNELS_PER_ADCGROUP; ic++)
        {
            LOG<<"Chan: "<<iblock*SIS3316_CHANNELS_PER_ADCGROUP+ic;
            LOG<<" "<<parsedEvents[ic];
            for(int ievt=0; ievt<fmin(10,parsedEvents[ic]);ievt++){
                LOG<<" "<<ParseClock(&(vcard->databuffer[iblock*SIS3316_CHANNELS_PER_ADCGROUP+ic][evtPointer[ic][ievt]]));
            }
            LOG<<" ... ";
            for(int ievt=parsedEvents[ic]-10; ievt<parsedEvents[ic];ievt++){
                if(ievt<0) continue;
                LOG<<" "<<ParseClock(&(vcard->databuffer[iblock*SIS3316_CHANNELS_PER_ADCGROUP+ic][evtPointer[ic][ievt]]));
            }
            LOG<<ENDM_INFO;
        }
    }
    
    

    int minWords = vcard->databufferread[iblock*4];
    for(int ipmt=0; ipmt<SIS3316_CHANNELS_PER_ADCGROUP; ipmt++)
    {
        if(vcard->databufferread[iblock*4] != vcard->databufferread[iblock*4+ipmt])
            goodData= false;
        if(minWords>vcard->databufferread[iblock*4+ipmt])
            minWords=vcard->databufferread[iblock*4+ipmt];
    }
    if(!goodData) return 0;

    int nevents = 0;
    unsigned int numberOfSampleWords = 0;
    int maw1, maw2, mawmax;
    double nsperclock=4.0;
    if(vcard->clock_source_choice==0)
        nsperclock=4.0;
    ULong64_t rawclockC[SIS3316_CHANNELS_PER_ADCGROUP];

    packet->Clear();
    packet->setSlotId((vcard->adcheaderid[iblock])>>24);
    packet->setChannelId(iblock*SIS3316_CHANNELS_PER_ADCGROUP);
    

    if(parsedEvents[0]==0) return 0;
    
    for(int ievent =0; ievent<parsedEvents[0]; ievent++)
    {
        numberOfSampleWords=vcard->databuffer[iblock*4][evtPointer[0][ievent]+nomEventSize-1]&0x03FFFFFF;
        //For all data formats we have timestamp
        ULong64_t rawclock = 0;
        bool matchingClocks = true;
        
        for(int ic = 0; ic<SIS3316_CHANNELS_PER_ADCGROUP;ic++)
        {
            rawclockC[ic] = ParseClock(&(vcard->databuffer[iblock*4][evtPointer[ic][ievent]]));
            if(ic>0&&rawclockC[ic]!=rawclockC[ic-1])
                matchingClocks=false;
        }

        if(!matchingClocks)
        {
            LOG<<"Mismatched clocks 2 for block "<<iblock;
            LOG<<" event "<<nevents;
            for(int ic = 0; ic<SIS3316_CHANNELS_PER_ADCGROUP;ic++)
                LOG<<" "<<(Long64_t)rawclockC[ic];
            LOG<<ENDM_WARN;
            break;
        }
        
        rawclock=rawclockC[0];
        
        NGMHit* hit = packet->addHit();
        hit->SetRawClock(rawclock);
        NGMTimeStamp rawTime(_runBegin);
        rawTime.IncrementNs(rawclock*nsperclock);
        hit->SetTimeStamp(rawTime);
        hit->SetSlot(packet->getSlotId());
        hit->SetChannel(iblock*SIS3316_CHANNELS_PER_ADCGROUP);
        
        unsigned short* wf[4];
        for(int ipmt=0; ipmt<4; ipmt++)
        {
            wf[ipmt]=(unsigned short*)(&(vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+nomEventSize]));
        }
        bool saveAllWaveforms = true;
        if(saveAllWaveforms){
            hit->SetNSamples(numberOfSampleWords*2*4);
            for(int ipmt = 0; ipmt<SIS3316_CHANNELS_PER_ADCGROUP; ipmt++)
            {
                for(int isample=0; isample<numberOfSampleWords*2;isample++)
                {
                    hit->SetSample(isample+ipmt*numberOfSampleWords*2,
                                   wf[ipmt][isample]);
                }
            }
        }else{
            hit->SetNSamples(numberOfSampleWords*2);
            for(int isample=0; isample<numberOfSampleWords*2;isample++)
            {
                hit->SetSample(isample,
                               wf[0][isample]+wf[1][isample]
                               + wf[2][isample]+wf[3][isample]);
            }
        }
        if(dataformat&0x1)
        {
            double pmt[4];
            double sum = 0.0;
            double psdsum = 0.0;
            int igate = 1;
            int psdgate = 2;
            
            bool adcSaturation= false;
            unsigned short evtInfo = 0;
            for(int ipmt=0; ipmt<4; ipmt++)
            {
                unsigned int peakHighValue = (vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+2] & 0xFFFF);
                if(peakHighValue==0x3FFF)
                {
                    evtInfo=evtInfo|0x80;
                    adcSaturation=true;
                }
                evtInfo=evtInfo|((vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+2] & 0xFF000000)>>24);
            }
            hit->SetPileUpCounter(evtInfo);
            
            if(dataformat&0x4)
            {
                int mawoffset=9;
                if(dataformat&0x2)
                    mawoffset+=2;
                mawmax = 0;
                maw1 =  0;
                maw2 = 0;
                for(int ipmt=0; ipmt<4; ipmt++)
                {
                    mawmax += vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+mawoffset]-0x8000000;
                    maw1 +=  vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+mawoffset+1]-0x8000000;
                    maw2 += vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+mawoffset+2]-0x8000000;
                }
                float cfd = 1.0+(mawmax*0.5-maw2)/float(maw2-maw1);
                if(cfd>1.0) cfd=1.0;
                if(cfd<0.0) cfd=0.0;
                hit->SetCFD(cfd);
                rawTime.IncrementNs(cfd*nsperclock);
                hit->SetRawTime(rawTime);
            }
            for(int ipmt=0; ipmt<4; ipmt++)
            {
                double baseline = (vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+3] & 0xFFFFFF)/(double)(vcard->qdclength[iblock][0]);
                pmt[ipmt] = (vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+igate+3]& 0xFFFFFFF) - baseline*(vcard->qdclength[iblock][igate]);
                sum+=pmt[ipmt];
                psdsum+= (vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+psdgate+3]& 0xFFFFFFF) - baseline*(vcard->qdclength[iblock][psdgate]);

                
            }
//            // For now populate from waveform
//            sum=0.0;
//            psdsum = 0.0;
//            hit->SetGateSize(20);
//            for(int ipmt=0; ipmt<4; ipmt++)
//            {
//                unsigned int basegate = 0; for(int i =0; i<20; i++) basegate+=wf[ipmt][i];
//                double baseline = basegate/20.0;
//                hit->SetGate(5*ipmt+0,basegate);
//                hit->SetGate(5*ipmt+1,wf[ipmt][38]);
//                hit->SetGate(5*ipmt+2,wf[ipmt][39]);
//                hit->SetGate(5*ipmt+3,wf[ipmt][190]);
//                hit->SetGate(5*ipmt+4,wf[ipmt][191]);
//                pmt[ipmt] = wf[ipmt][190]*(1.0-hit->GetCFD()) + wf[ipmt][191]*hit->GetCFD() - baseline;
//                sum+=pmt[ipmt];
//                psdsum+= (wf[ipmt][38]*(1.0-hit->GetCFD()) + wf[ipmt][39]*hit->GetCFD() - baseline);
//            }
            
            // For now populate from waveform
            sum=0.0;
            psdsum = 0.0;
            hit->SetGateSize(20);
            for(int ipmt=0; ipmt<4; ipmt++)
            {
                hit->SetGate(5*ipmt+0,(vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+qdcoffset] & 0xFFFFFF));
                hit->SetGate(5*ipmt+1,(vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+qdcoffset+1] & 0xFFFFFFF));
                hit->SetGate(5*ipmt+2,(vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+qdcoffset+2] & 0xFFFFFFF));
                hit->SetGate(5*ipmt+3,(vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+qdcoffset+3] & 0xFFFFFFF));
                hit->SetGate(5*ipmt+4,(vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+qdcoffset+4] & 0xFFFFFFF));
                pmt[ipmt] = hit->GetGate(5*ipmt+3)*(1.0-hit->GetCFD()) + hit->GetGate(5*ipmt+4)*hit->GetCFD() - hit->GetGate(5*ipmt+0)/(double)vcard->qdclength[iblock][0];
                sum+=pmt[ipmt];
                psdsum+= (hit->GetGate(5*ipmt+1)*(1.0-hit->GetCFD()) + hit->GetGate(5*ipmt+2)*hit->GetCFD() - hit->GetGate(5*ipmt+0)/(double)vcard->qdclength[iblock][0]);
            }

            
            double x = (pmt[2]+pmt[3])/sum;
            double y = (pmt[0]+pmt[2])/sum;
            //OOPS Swapped cables for runs less than 20130228220142
            //double x = (pmt[3]+pmt[0])/sum;
            //double y = (pmt[1]+pmt[3])/sum;
            hit->SetBlockX(x);
            hit->SetBlockY(y);
            hit->SetPulseHeight(sum);
            hit->SetEnergy(sum);
            hit->SetPSD(psdsum/sum);
        }
        
        if(_verbosity>10 && ievent==parsedEvents[0]-1)
        {
            LOG<<"Last event of Buffer event "<<nevents;
            for(int ic = 0; ic<SIS3316_CHANNELS_PER_ADCGROUP;ic++)
            {
                LOG<<" "<<(Long64_t)rawclockC[ic];
                LOG<<" ("<<evtPointer[ic][ievent]-vcard->databufferread[iblock*4+ic]<<")";
            }
            LOG<<ENDM_WARN;

        }
        nevents++;
    }
    
    
    return 0;
}

void NGMSIS3316RawReader::PlotFirstWaveform(sis3316card* vcard, int ichan)
{
    TString cName;
    cName.Form("cWaveSlot%d_%d",vcard->modid,ichan);
    
    TCanvas* c1 = dynamic_cast<TCanvas*>(gROOT->FindObjectAny(cName.Data()));
    if(!c1) c1 = new TCanvas(cName.Data(),cName.Data());
    c1->cd();
    
    int iblock = ichan/SIS3316_CHANNELS_PER_ADCGROUP;
    int dataformat = vcard->dataformat_block[iblock];
    int nomEventSize = 3;
    if(dataformat&0x1) nomEventSize+=7;
    if(dataformat&0x2) nomEventSize+=2;
    if(dataformat&0x4) nomEventSize+=3;
    
    if(vcard->databufferread[ichan] < nomEventSize ) return;
    
    int numberOfSampleWords=vcard->databuffer[ichan][nomEventSize-1]&0x03FFFFFF;

    if(vcard->databufferread[ichan] < nomEventSize +numberOfSampleWords ) return;
    
    unsigned short* wf =(unsigned short*)(&(vcard->databuffer[ichan][nomEventSize]));
    
    return;
}

int NGMSIS3316RawReader::ParseEvent(sis3316card* vcard, int ichan, size_t indexOfNextEvent, NGMHit* hit)
{
    int block = ichan/SIS3316_CHANNELS_PER_ADCGROUP;
    int dataformat = vcard->dataformat_block[block];
    int nomEventSize = 3;
    if(dataformat&0x1) nomEventSize+=7;
    if(dataformat&0x2) nomEventSize+=2;
    if(dataformat&0x4) nomEventSize+=3;
    int qdcoffset= 3;

    unsigned int numberOfSamplesWords = 0;
    int maw1, maw2, mawmax;
    double nsperclock=4.0;
    if(vcard->clock_source_choice==0)
        nsperclock=4.0;
    
    if(indexOfNextEvent+nomEventSize<vcard->databufferread[ichan])
    {
        numberOfSamplesWords=vcard->databuffer[ichan][indexOfNextEvent+nomEventSize-1]&0x03FFFFFF;
        hit->SetSlot((vcard->adcheaderid[block])>>24);
        hit->SetChannel(ichan);
        //For all data formats we have timestamp
        ULong64_t rawclock = ParseClock(&(vcard->databuffer[ichan][indexOfNextEvent]));
        hit->SetRawClock(rawclock);
        NGMTimeStamp rawTime(_runBegin);
        rawTime.IncrementNs(rawclock*nsperclock);
        hit->SetTimeStamp(rawTime);
        hit->SetPileUpCounter( ((vcard->databuffer[ichan][indexOfNextEvent+3])>>24) & 0xFF);
        if(dataformat&0x1) // We have at least five qdc values
        {
            hit->SetGateSize(5);
            for(int iqdc = 0; iqdc<5; iqdc++)
                hit->SetGate(iqdc,(vcard->databuffer[ichan][indexOfNextEvent+qdcoffset+iqdc] & 0xFFFFFF));
        }
        if(dataformat&0x1)
        {
            double sum = 0.0;
            double psdsum = 0.0;
            int igate = 2;
            int psdgate = 1;
            if(dataformat&0x4)
            {
                int mawoffset=9;
                if(dataformat&0x2)
                    mawoffset+=2;
                mawmax = vcard->databuffer[ichan][indexOfNextEvent+mawoffset]-0x8000000;
                maw1 =  vcard->databuffer[ichan][indexOfNextEvent+mawoffset+1]-0x8000000;
                maw2 = vcard->databuffer[ichan][indexOfNextEvent+mawoffset+2]-0x8000000;
                float cfd = 1.0+(mawmax*0.5-maw2)/float(maw2-maw1);
                if(cfd>1.0) cfd=1.0;
                if(cfd<0.0) cfd=0.0;
                hit->SetCFD(cfd);
                rawTime.IncrementNs(cfd*nsperclock);
                hit->SetRawTime(rawTime);
            }
            
            double baseline = (vcard->databuffer[ichan][indexOfNextEvent+nomEventSize] & 0xFFFFFF)/(double)(vcard->qdclength[block][0]);
            sum = (vcard->databuffer[ichan][indexOfNextEvent+igate+nomEventSize]& 0xFFFFFFF) - baseline*(vcard->qdclength[block][igate]);
            psdsum= (vcard->databuffer[ichan][indexOfNextEvent+psdgate+nomEventSize]& 0xFFFFFFF) - baseline*(vcard->qdclength[block][psdgate]);
            
            
            //hEnergy->Fill(sum);
            //hPSDEnergy->Fill(sum,psdsum/sum);
            hit->SetPulseHeight(sum);
            hit->SetEnergy(sum);
            hit->SetPSD(psdsum);
            hit->SetPileUpCounter((vcard->databuffer[ichan][indexOfNextEvent+3])>>24);
        }
        
        unsigned short* wf=(unsigned short*)(&(vcard->databuffer[ichan][indexOfNextEvent+nomEventSize]));
        if(_skipWaveforms){
            hit->SetNSamples(0);
        }else{
            hit->SetNSamples(numberOfSamplesWords*2);
            for(int isample=0; isample<numberOfSamplesWords*2;isample++)
            {
                hit->SetSample(isample,
                               wf[isample]);
            }
        }
        indexOfNextEvent+=nomEventSize+numberOfSamplesWords;
    }
    return 0;
}

int NGMSIS3316RawReader::ParseBlockEvent(sis3316card* vcard, int ichan, size_t indexOfNextEvent, NGMHit* hit)
{
    int iblock = ichan/SIS3316_CHANNELS_PER_ADCGROUP;
    int dataformat = vcard->dataformat_block[iblock];
    int nomEventSize = 3;
    if(dataformat&0x1) nomEventSize+=7;
    if(dataformat&0x2) nomEventSize+=2;
    if(dataformat&0x4) nomEventSize+=3;
    int qdcoffset= 3;
    

    unsigned int numberOfSampleWords = 0;
    int maw1, maw2, mawmax;
    double nsperclock=4.0;
    if(vcard->clock_source_choice==0)
        nsperclock=4.0;
    ULong64_t rawclockC[SIS3316_CHANNELS_PER_ADCGROUP];
    
    hit->SetSlot((vcard->adcheaderid[iblock])>>24);
    hit->SetChannel(ichan);
    
    
    
    numberOfSampleWords=vcard->databuffer[iblock*4][indexOfNextEvent+nomEventSize-1]&0x03FFFFFF;
    //For all data formats we have timestamp
    ULong64_t rawclock = 0;
    bool matchingClocks = true;
    
    for(int ic = 0; ic<SIS3316_CHANNELS_PER_ADCGROUP;ic++)
    {
        rawclockC[ic] = ParseClock(&(vcard->databuffer[iblock*4][indexOfNextEvent]));
        if(ic>0&&rawclockC[ic]!=rawclockC[ic-1])
            matchingClocks=false;
    }
    
    if(!matchingClocks)
    {
        LOG<<"Mismatched clocks 2 for block "<<iblock;
        for(int ic = 0; ic<SIS3316_CHANNELS_PER_ADCGROUP;ic++)
            LOG<<" "<<(Long64_t)rawclockC[ic];
        LOG<<ENDM_WARN;
    }
    
    rawclock=rawclockC[0];

    hit->SetRawClock(rawclock);
    NGMTimeStamp rawTime(_runBegin);
    rawTime.IncrementNs(rawclock*nsperclock);
    hit->SetTimeStamp(rawTime);
    hit->SetChannel(iblock*SIS3316_CHANNELS_PER_ADCGROUP);
    
    unsigned short* wf[4];
    for(int ipmt=0; ipmt<4; ipmt++)
    {
        wf[ipmt]=(unsigned short*)(&(vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+nomEventSize]));
    }
    bool saveAllWaveforms = true;
    if(_skipWaveforms){
        hit->SetNSamples(0);
    }else{
        if(saveAllWaveforms){
            hit->SetNSamples(numberOfSampleWords*2*4);
            for(int ipmt = 0; ipmt<SIS3316_CHANNELS_PER_ADCGROUP; ipmt++)
            {
                for(int isample=0; isample<numberOfSampleWords*2;isample++)
                {
                    hit->SetSample(isample+ipmt*numberOfSampleWords*2,
                                   wf[ipmt][isample]);
                }
            }
        }else{
            hit->SetNSamples(numberOfSampleWords*2);
            for(int isample=0; isample<numberOfSampleWords*2;isample++)
            {
                hit->SetSample(isample,
                               wf[0][isample]+wf[1][isample]
                               + wf[2][isample]+wf[3][isample]);
            }
        }
    }
    if(dataformat&0x1)
    {
        double pmt[4];
        double sum = 0.0;
        double psdsum = 0.0;
        int igate = 1;
        int psdgate = 2;
        
        bool adcSaturation= false;
        unsigned short evtInfo = 0;
        for(int ipmt=0; ipmt<4; ipmt++)
        {
            unsigned int peakHighValue = (vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+2] & 0xFFFF);
            if(peakHighValue==0x3FFF)
            {
                evtInfo=evtInfo|0x80;
                adcSaturation=true;
            }
            evtInfo=evtInfo|((vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+2] & 0xFF000000)>>24);
        }
        hit->SetPileUpCounter(evtInfo);
        
        if(dataformat&0x4)
        {
            int mawoffset=9;
            if(dataformat&0x2)
                mawoffset+=2;
            mawmax = 0;
            maw1 =  0;
            maw2 = 0;
            for(int ipmt=0; ipmt<4; ipmt++)
            {
                mawmax += vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+mawoffset]-0x8000000;
                maw1 +=  vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+mawoffset+1]-0x8000000;
                maw2 += vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+mawoffset+2]-0x8000000;
            }
            float cfd = 1.0+(mawmax*0.5-maw2)/float(maw2-maw1);
            if(cfd>1.0) cfd=1.0;
            if(cfd<0.0) cfd=0.0;
            if(cfd!=cfd) cfd=0.0;
            hit->SetCFD(cfd);
            rawTime.IncrementNs(cfd*nsperclock);
            hit->SetRawTime(rawTime);
        }
        for(int ipmt=0; ipmt<4; ipmt++)
        {
            double baseline = (vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+3] & 0xFFFFFF)/(double)(vcard->qdclength[iblock][0]);
            pmt[ipmt] = (vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+igate+3]& 0xFFFFFFF) - baseline*(vcard->qdclength[iblock][igate]);
            sum+=pmt[ipmt];
            psdsum+= (vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+psdgate+3]& 0xFFFFFFF) - baseline*(vcard->qdclength[iblock][psdgate]);
            
            
        }
        
        sum=0.0;
        psdsum = 0.0;
        hit->SetGateSize(20);
        for(int ipmt=0; ipmt<4; ipmt++)
        {
            hit->SetGate(5*ipmt+0,(vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+qdcoffset] & 0xFFFFFF));
            hit->SetGate(5*ipmt+1,(vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+qdcoffset+1] & 0xFFFFFFF));
            hit->SetGate(5*ipmt+2,(vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+qdcoffset+2] & 0xFFFFFFF));
            hit->SetGate(5*ipmt+3,(vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+qdcoffset+3] & 0xFFFFFFF));
            hit->SetGate(5*ipmt+4,(vcard->databuffer[iblock*4+ipmt][indexOfNextEvent+qdcoffset+4] & 0xFFFFFFF));
            pmt[ipmt] = hit->GetGate(5*ipmt+3)*(1.0-hit->GetCFD()) + hit->GetGate(5*ipmt+4)*hit->GetCFD() - hit->GetGate(5*ipmt+0)/(double)vcard->qdclength[iblock][0];
            sum+=pmt[ipmt];
            psdsum+= (hit->GetGate(5*ipmt+1)*(1.0-hit->GetCFD()) + hit->GetGate(5*ipmt+2)*hit->GetCFD() - hit->GetGate(5*ipmt+0)/(double)vcard->qdclength[iblock][0]);
        }
        
        
        double x = (pmt[2]+pmt[3])/sum;
        double y = (pmt[0]+pmt[2])/sum;
        //OOPS Swapped cables for runs less than 20130228220142
        //double x = (pmt[3]+pmt[0])/sum;
        //double y = (pmt[1]+pmt[3])/sum;
        hit->SetBlockX(x);
        hit->SetBlockY(y);
        hit->SetPulseHeight(sum);
        hit->SetEnergy(sum);
        hit->SetPSD(psdsum/sum);
    }
    return 0;
}

