#include <stdint.h>
#include "NGMSIS3302RawReader.h"
#include "NGMSystemConfiguration.h"
#include "NGMConfigurationTable.h"
#include "NGMConfigurationParameter.h"
#include "NGMBufferedPacket.h"
#include "NGMModule.h"
#include "NGMSystem.h"
#include "NGMLogger.h"
#include "sis3302card.h"
#include "TThread.h"
#include "TFile.h"
#include "TObjString.h"
#include "TString.h"
#include "TCanvas.h"
#include "TROOT.h"
#include <cmath>
#include <queue>
#include <list>
#include "NGMTimingCal.h"

ClassImp(NGMSIS3302RawReader)

class sis3302evtInfo
{
public:
    sis3302evtInfo(unsigned long long trawclock,
            unsigned short ticard,
            unsigned short tichan,
            size_t tevtposition);
    static int ParseEventData(sis3302card* vcard, unsigned short card, unsigned short chan,std::queue<sis3302evtInfo> &evtList);
    static void AddToList(sis3302evtInfo evt,std::list<sis3302evtInfo> &evtList);
    unsigned long long rawclock;
    unsigned short icard;
    unsigned short ichan;
    size_t evtposition;
};
sis3302evtInfo::sis3302evtInfo(unsigned long long trawclock,
        unsigned short ticard,
        unsigned short tichan,
        size_t tevtposition)
:rawclock(trawclock), icard(ticard), ichan(tichan),evtposition(tevtposition)
{}

int sis3302evtInfo::ParseEventData(sis3302card* vcard, unsigned short card, unsigned short chan,std::queue<sis3302evtInfo> &evtList)
{
    int block = chan/SIS3302_CHANNELS_PER_ADCGROUP;
    int dataformat = vcard->dataformat_block[block];
    int nomEventSize = 10;
    if(dataformat&(0x2<<(8*(chan%SIS3302_CHANNELS_PER_ADCGROUP)))) nomEventSize+=2;
    int nevents = 0;
    unsigned int numberOfSamples = 0;
    unsigned int indexOfNextEvent = 0;
    while(indexOfNextEvent+nomEventSize<vcard->databufferread[chan])
    {
        numberOfSamples=vcard->databuffer[chan][indexOfNextEvent+nomEventSize-1]&0x0000FFFF;
        ULong64_t rawclock = vcard->databuffer[chan][indexOfNextEvent+1];
        rawclock=rawclock | ((((ULong64_t)(vcard->databuffer[chan][indexOfNextEvent])) & 0xFFFF0000)<<16);
        sis3302evtInfo tevt(rawclock,card,chan,indexOfNextEvent);
        //printf("Nevents %d %lld %d %d %ld\n ",nevents,tevt.rawclock,tevt.icard,tevt.ichan,tevt.evtposition);
        evtList.push(tevt);
        
        // next event
        indexOfNextEvent+=nomEventSize+numberOfSamples/2;
        nevents++;
    }
    //printf("Nevents (%d,%d) %d\n ",card,chan,nevents);
    return nevents;
}

void sis3302evtInfo::AddToList(sis3302evtInfo evt,std::list<sis3302evtInfo> &evtList)
{
    for(std::list<sis3302evtInfo>::iterator itr = evtList.begin(); itr!=evtList.end(); itr++)
    {
        if(evt.rawclock<itr->rawclock)
        {
            evtList.insert(itr,evt);
            return;
        }
    }
    evtList.push_back(evt);
}

NGMSIS3302RawReader::NGMSIS3302RawReader()
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

NGMSIS3302RawReader::NGMSIS3302RawReader(NGMModule* parent)
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
    _hitPool = new NGMHitPoolv6();

}

NGMSIS3302RawReader::~NGMSIS3302RawReader()
{
    CloseInputFile();
    delete _hitPool;
}

void NGMSIS3302RawReader::GetStatus(TString& sStatus)
{
    size_t bytesRead = ftell(_inputfile);
    sStatus=Form("Reading %ld bytes from run %lld\n Currently %02ld%% done with current file.",bytesRead,GetConfiguration()->getRunNumber(),100*bytesRead/_inputfilesize);
                 
}

UInt_t NGMSIS3302RawReader::sumarray(int alength, unsigned short * arrptr)
{
    uint32_t sum = 0;
    for(int i = 0; i<alength; i++) sum+=arrptr[i];
    return sum;
}

void NGMSIS3302RawReader::SetConfiguration(const NGMSystemConfiguration* sysConf)
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
        sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
        _NanoSecondsPerClock[icard]=10.0;
        if(card->clock_source_choice==6)
            _NanoSecondsPerClock[icard]=10.0;
        card->AllocateBuffers(0);
        card->ResetRunScalars();
    }

    // Initialize other beginning of run variables
    _runBegin = _config->GetTimeStamp();
    _firstTimeOfRun = TTimeStamp(0,0);
    _lastPlotIndex = -1;
    _runduration = 0;
    _livetime = 0;
    _totalEventsThisRun = 0;
    
}

Long64_t NGMSIS3302RawReader::OpenInputFile(const char* filename)
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

Long64_t NGMSIS3302RawReader::OpenRawBinaryFile(const char* pathname)
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
    Long64_t result = sis3302card::readFileWaiting(runhdr, 0x4, 100, _inputfile);
    if(result!=wordsInRunHeader)
    {
        LOG<<" Full Header not found in buffer "<<pathname<<ENDM_INFO;
        return 1;
    }
    if((runhdr[0]&0xFFFF0000)!=0x33020000)
    {
        LOG<<"File does not appear to be a SIS3302 raw file"<<ENDM_FATAL;
        return 3;
    }
    
    LOG<<"Found SIS Run Header of version "<<runhdr[0] << ENDM_INFO;
    
    return 0;
}


Long64_t NGMSIS3302RawReader::CloseInputFile()
{
    
    if(_inputfile) fclose(_inputfile);
    _inputfile = 0;
    return 0;
}

Long64_t NGMSIS3302RawReader::ReadNextSpillFromFile()
{
    size_t bytesread;
    int spillhdr[10];
    
    bytesread = sis3302card::readFileWaiting(spillhdr,0x4,10,_inputfile)*0x4;
    // Sanity Check if spillheader is 0xABBAABBA;
    //printf("Found spill header: 0x%08x\n",spillhdr[0]);
    if(spillhdr[0] != 0xABBAABBA)
    {
        return -2;
    }


    //Spill header is the same length as endoffile record.
    // Check if spillheader is 0x0E0F0E0F;//ENDOFFILE
    if(spillhdr[0] == 0x0E0F0E0F)
    {
        LOG<<"Found End of File Record\n"<<ENDM_INFO;
        return -1;
    }
    
    NGMConfigurationTable* cslot = GetConfiguration()->GetSlotParameters();
    NGMConfigurationParameter* cards = cslot->GetColumn("card");
    NGMConfigurationParameter* modenable = cslot->GetColumn("ModEnable");
    for(int icard = 0; icard < cards->GetEntries(); icard++)
    {
        if(! modenable->GetValueI(icard)) continue;
        sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
        bytesread+=card->ReadSpillFromFile(_inputfile);
    }
    long curbytes = ftell(_inputfile);
    if (curbytes/(_inputfilesize/10) != (curbytes-bytesread)/(_inputfilesize/10)) {
        std::cout << "Read " << bytesread << " bytes; currently " << 100*ftell(_inputfile)/_inputfilesize << "% done with current file." << std::endl;
        // Recheck file size in case we're taking data
        fpos_t curpos;
        fgetpos(_inputfile, &curpos);
        fseek(_inputfile,0,SEEK_END);
        _inputfilesize = ftell(_inputfile);
        fsetpos(_inputfile,&curpos);
    }
    return bytesread;
}

Long64_t NGMSIS3302RawReader::SortSpill()
{
    NGMConfigurationTable* cslot = GetConfiguration()->GetSlotParameters();
    NGMConfigurationParameter* cards = cslot->GetColumn("card");
    NGMConfigurationParameter* modenable = cslot->GetColumn("ModEnable");
    TObjString endSpillFlush("EndSpillFlush");

    std::vector<sis3302card*> lcard;
    for(int icard=0; icard<cards->GetEntries(); icard++)
    {
        sis3302card* card = 0;
        if(modenable->GetValueI(icard))
        {
            card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
        }
        lcard.push_back(card);
    }
    std::vector< std::queue<sis3302evtInfo> > eventList;
    eventList.reserve(lcard.size()*SIS3302_CHANNELS_PER_CARD);
    //printf("Begin Parsing Event Headers Buffers(%ld)\n",eventList.size());
    
    for(int icard=0; icard<lcard.size(); icard++)
    {
        if(!lcard[icard]) continue;
        for(int ichan = 0; ichan < SIS3302_CHANNELS_PER_CARD; ichan++)
        {
            int chanseq = icard*SIS3302_CHANNELS_PER_CARD + ichan;
            std::queue<sis3302evtInfo> tList;
            eventList.push_back(tList);
            //printf("Parsed Event Headers Slot(%d) Chan(%d) ChanSeq(%d)\n",icard, ichan,chanseq);
            sis3302evtInfo::ParseEventData(lcard[icard],icard,ichan,
                                 eventList[chanseq]);
            //printf("Parsed Event Headers Slot(%d) Chan(%d) ChanSeq(%d) Nevents(%ld)\n",
            //       icard, ichan,chanseq, eventList[chanseq].size());
        }
    }
    
    //Lets loop through all queues and pop the head into a sorted map keyed on clock
    std::list<sis3302evtInfo> headSort;
    std::vector<sis3302evtInfo> sortedList;
    for(int chanseq = 0; chanseq<eventList.size(); chanseq++)
    {
        if(!eventList[chanseq].empty())
        {
            sis3302evtInfo::AddToList( eventList[chanseq].front(),headSort);
            eventList[chanseq].pop();
        }
    }
    // now we cycle through
    while(!headSort.empty())
    {
        sis3302evtInfo nextEvt = headSort.front();
        headSort.pop_front();
        int chanseq = nextEvt.icard*SIS3302_CHANNELS_PER_CARD + nextEvt.ichan;
        if(!eventList[chanseq].empty())
        {
            sis3302evtInfo::AddToList( eventList[chanseq].front(),headSort);
            eventList[chanseq].pop();
        }
        NGMHit* hit = _hitPool->GetHit();
        ParseEvent(lcard[nextEvt.icard],nextEvt.ichan,nextEvt.evtposition,hit);
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
        }else if(GetParent())
        {
            GetParent()->push(*((const TObject*)(hit)));
            _hitPool->ReturnHit(hit);
        }
        //sortedList.push_back(nextEvt);
    }
    
    if(GetParent()){
        GetParent()->push(*((const TObject*)&endSpillFlush));
        //((NGMSystem*)GetParent())->ProcessSpyServ();
    }
    
    return 0;
}

Long64_t NGMSIS3302RawReader::ReadAll()
{
    
    NGMConfigurationTable* cslot = GetConfiguration()->GetSlotParameters();
    NGMConfigurationParameter* cards = cslot->GetColumn("card");
    NGMConfigurationParameter* modenable = cslot->GetColumn("ModEnable");
    NGMBufferedPacket* packet = new NGMBufferedPacketv2(0,0,TTimeStamp((time_t)0,0),6);
    TObjString endSpillFlush("EndSpillFlush");

    // This sequence attempts to sort dat a for a spill and send hits to daughter modules
    while(!_abortread&&ReadNextSpillFromFile()>0)
    {
        SortSpill();
        gSystem->ProcessEvents();
    }
    return 0;
    
    
    // Original Reading dumps packets to daughter modules
    while(!_abortread&&ReadNextSpillFromFile()>0)
    {
        // Early data has a bug that can result in a mixing of old and new buffers
        // Lets do some inefficienct sanity checks for now...
        bool goodData = true;
        for(int icard=0; icard<0/*cards->GetEntries()*/; icard++)
        {
            if(modenable->GetValueI(icard))
            {
                sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
                for(int iadc = 0; iadc <SIS3302_ADCGROUP_PER_CARD; iadc++)
                {
                    if(card->IsBlockReadout(iadc))
                    {
                        std::vector<unsigned long long> evtPointer[SIS3302_CHANNELS_PER_ADCGROUP];
                        std::map<unsigned long long,int> evtid[SIS3302_CHANNELS_PER_ADCGROUP];
                        int parsedEvents[SIS3302_CHANNELS_PER_ADCGROUP];
                        //Sanity Checks
                        for(int ic=0; ic<SIS3302_CHANNELS_PER_ADCGROUP; ic++)
                        {
                            parsedEvents[ic] = ParseEventDataSingle(card,
                                                                    iadc*SIS3302_CHANNELS_PER_ADCGROUP+ic,
                                                                    evtPointer[ic],
                                                                    evtid[ic]);
                            if(ic>0 && parsedEvents[ic]!=parsedEvents[ic-1]) goodData = false;
                        }

                    }else{
                        for(int ic = 0; ic<SIS3302_CHANNELS_PER_ADCGROUP; ic++)
                        {
                            
                            int ichan = iadc*SIS3302_CHANNELS_PER_ADCGROUP + ic;
                            std::vector<unsigned long long> evtPointer;
                            std::map<unsigned long long,int> evtid;
                            int parsedEvents = ParseEventDataSingle(card,ichan,evtPointer,evtid);
                            if(parsedEvents == 0) continue;
                            if( ParseClock(&(card->databuffer[ichan][evtPointer[0]])) > ParseClock(&(card->databuffer[ichan][evtPointer[parsedEvents-1]]))) goodData=false;
                            if( ParseClock(&(card->databuffer[ichan][evtPointer[0]])) > ParseClock(&(card->databuffer[ichan][evtPointer[parsedEvents-1]])))
                            {
                                printf("Bad clock sort %d %d %lld %lld \n",icard,ic,ParseClock(&(card->databuffer[ichan][evtPointer[0]])),ParseClock(&(card->databuffer[ichan][evtPointer[parsedEvents-1]])));
                            }
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
                sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(icard));
                for(int iadc = 0; iadc <SIS3302_ADCGROUP_PER_CARD; iadc++)
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
                        for(int ic = 0; ic<SIS3302_CHANNELS_PER_ADCGROUP; ic++)
                        {
                            ParseSingleChannelData(card, iadc*SIS3302_CHANNELS_PER_ADCGROUP+ic, packet);
                            GetParent()->push(*((const TObject*)(packet)));
                            packet->Clear();
                        }
                    }
                }
            }
        }
        if(GetParent()){
            GetParent()->push(*((const TObject*)&endSpillFlush));
            //((NGMSystem*)GetParent())->ProcessSpyServ();
        }

    }
    
    return 0;
}

Long64_t NGMSIS3302RawReader::parsePacket(int slot, int chan, NGMBufferedPacket* packet)
{
    NGMConfigurationTable* cslot = GetConfiguration()->GetSlotParameters();
    NGMConfigurationParameter* cards = cslot->GetColumn("card");

    sis3302card* card = dynamic_cast<sis3302card*>(cards->GetValueO(slot));
    int iadc=chan/SIS3302_CHANNELS_PER_ADCGROUP;
    if(card->IsBlockReadout(iadc))
    {
        ParseADCBlockData(card,iadc,packet);
    }else{
        ParseSingleChannelData(card,chan,packet);
    }
    
    
    return 0;
}


int NGMSIS3302RawReader::ParseEventDataSingle(sis3302card* vcard, int ichan, std::vector<unsigned long long> &evtPointer, std::map<unsigned long long,int> &evtid )
{
    evtPointer.clear();
    evtid.clear();
    
    int block = ichan/SIS3302_CHANNELS_PER_ADCGROUP;
    int dataformat = vcard->dataformat_block[block];
    int nomEventSize = 10;
    if(dataformat&(0x2<<(8*(ichan%SIS3302_CHANNELS_PER_ADCGROUP)))) nomEventSize+=2;
    int nevents = 0;
    unsigned int numberOfSamples = 0;
    unsigned int indexOfNextEvent = 0;
    while(indexOfNextEvent+nomEventSize<vcard->databufferread[ichan])
    {
        numberOfSamples=vcard->databuffer[ichan][indexOfNextEvent+nomEventSize-1]&0x0000FFFF;
        ULong64_t rawclock = vcard->databuffer[ichan][indexOfNextEvent+1];
        rawclock=rawclock | ((((ULong64_t)(vcard->databuffer[ichan][indexOfNextEvent])) & 0xFFFF0000)<<16);
        evtPointer.push_back(indexOfNextEvent);
        evtid[rawclock]=nevents;
        
        if(_verbosity>0&&indexOfNextEvent==0)
        {
            printf("First Event Header 0x%04x %f \n",vcard->databuffer[ichan][indexOfNextEvent]&0xFFFF,vcard->databufferread[ichan]/512.0);
        }
        if(_verbosity>0&&(indexOfNextEvent==0 || (indexOfNextEvent+nomEventSize+numberOfSamples/2)==vcard->databufferread[ichan]))
        {
            printf("Raw Clock: %d %lld 0x%08x 0x%08x %f %d\n",nevents,rawclock,vcard->databuffer[ichan][indexOfNextEvent+1], vcard->databuffer[ichan][indexOfNextEvent],
               rawclock*1e-8,numberOfSamples);
        }
        // next event
        indexOfNextEvent+=nomEventSize+numberOfSamples/2;
        nevents++;
    }
    //printf("Nevents %d\n ",nevents);
    return nevents;
}

int NGMSIS3302RawReader::ParseEvent(sis3302card* vcard, int ichan, size_t indexOfNextEvent, NGMHit* hit)
{
    int block = ichan/SIS3302_CHANNELS_PER_ADCGROUP;
    int dataformat = vcard->dataformat_block[block];
    int nomEventSize = 10;
    int ngates = 6;
    if(dataformat&(0x2<<(8*(ichan%SIS3302_CHANNELS_PER_ADCGROUP))))
    {
        ngates = 8;
        nomEventSize+=2;
    }
    
    hit->SetSlot((vcard->adcheaderid[block]));
    
    unsigned int numberOfSamples = 0;
    int maw1, maw2, mawmax;
    double nsperclock=10.0;
    if(vcard->clock_source_choice==0)
        nsperclock=10.0;
    
    if(indexOfNextEvent+nomEventSize<vcard->databufferread[ichan])
    {
        numberOfSamples=vcard->databuffer[ichan][indexOfNextEvent+nomEventSize-1]&0x0000FFFF;
        hit->SetChannel(ichan);
        hit->SetPixel(hit->GetSlot()*SIS3302_CHANNELS_PER_CARD + hit->GetChannel());
        //For all data formats we have timestamp
        ULong64_t rawclock = vcard->databuffer[ichan][indexOfNextEvent+1];
        rawclock=rawclock | ((((ULong64_t)(vcard->databuffer[ichan][indexOfNextEvent])) & 0xFFFF0000)<<16);
        hit->SetRawClock(rawclock);
        NGMTimeStamp rawTime(_runBegin);
        rawTime.IncrementNs(rawclock*nsperclock);
        hit->SetTimeStamp(rawTime);
        unsigned int puc = (((vcard->databuffer[ichan][indexOfNextEvent+3])>>24) & 0xFF)<<16;
        puc = puc | ((vcard->databuffer[ichan][indexOfNextEvent+2]) & 0xFFFF);
        hit->SetPileUpCounter(puc);
        hit->SetGateSize(ngates);
        hit->SetGate(0,(vcard->databuffer[ichan][indexOfNextEvent+3])&0xFFFFFF);
        for(int igate = 1; igate<ngates; igate++)
        {
            hit->SetGate(igate,(vcard->databuffer[ichan][indexOfNextEvent+3+igate])&0xFFFFFFF );
        }
        
        if(0)//dataformat&0x1)
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
        
        hit->SetNSamples(numberOfSamples);
        for(int isample=0; isample<numberOfSamples;isample++)
        {
            hit->SetSample(isample,
                           wf[isample]);
        }
        return 1;
    }
    return 0;
}


int NGMSIS3302RawReader::ParseSingleChannelData(sis3302card* vcard, int ichan,NGMBufferedPacket* packet)
{
    int block = ichan/SIS3302_CHANNELS_PER_ADCGROUP;
    int dataformat = vcard->dataformat_block[block];
    int nomEventSize = 10;
    if(dataformat&(0x2<<(8*(ichan%SIS3302_CHANNELS_PER_ADCGROUP)))) nomEventSize+=2;
    
    packet->Clear();
    packet->setSlotId((vcard->adcheaderid[block]));
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
    double nsperclock=10.0;
    if(vcard->clock_source_choice==0)
        nsperclock=10.0;

    while(indexOfNextEvent+nomEventSize<vcard->databufferread[ichan])
    {
        numberOfSamples=vcard->databuffer[ichan][indexOfNextEvent+nomEventSize-1]&0x0000FFFF;
        NGMHit* hit = packet->addHit();
        hit->SetSlot(packet->getSlotId());
        hit->SetChannel(ichan);
        //For all data formats we have timestamp
        ULong64_t rawclock = vcard->databuffer[ichan][indexOfNextEvent+1];
        rawclock=rawclock | ((((ULong64_t)(vcard->databuffer[ichan][indexOfNextEvent])) & 0xFFFF0000)<<16);
        hit->SetRawClock(rawclock);
        //printf("%d %d %lld\n",hit->GetSlot(),hit->GetChannel(),rawclock);
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

        hit->SetNSamples(numberOfSamples);
        for(int isample=0; isample<numberOfSamples;isample++)
        {
            hit->SetSample(isample,
                           wf[isample]);
        }
        
        indexOfNextEvent+=nomEventSize+numberOfSamples/2;
        nevents++;
    }
    return 0;
}

ULong64_t NGMSIS3302RawReader::ParseClock(unsigned int* data)
{
    return  ((ULong64_t)(data[1])) | (((ULong64_t)(data[0] & 0xFFFF0000))<<16);
}

int NGMSIS3302RawReader::ParseADCBlockData(sis3302card* vcard, int iblock,NGMBufferedPacket* packet)
{
    int dataformat = vcard->dataformat_block[iblock];
    int nomEventSize = 3;
    if(dataformat&0x1) nomEventSize+=7;
    if(dataformat&0x2) nomEventSize+=2;
    if(dataformat&0x4) nomEventSize+=3;
    int qdcoffset= 3;
    
    std::vector<unsigned long long> evtPointer[SIS3302_CHANNELS_PER_ADCGROUP];
    std::map<unsigned long long,int> evtid[SIS3302_CHANNELS_PER_ADCGROUP];
    int parsedEvents[SIS3302_CHANNELS_PER_ADCGROUP];
    //Sanity Checks
    bool goodData=true;
    for(int ic=0; ic<SIS3302_CHANNELS_PER_ADCGROUP; ic++)
    {
        parsedEvents[ic] = ParseEventDataSingle(vcard,
                                                iblock*SIS3302_CHANNELS_PER_ADCGROUP+ic,
                                                evtPointer[ic],
                                                evtid[ic]);
        if(ic>0 && parsedEvents[ic]!=parsedEvents[ic-1]) goodData = false;
    }
    if(_verbosity>10||!goodData)
    {
        for(int ic=0; ic<SIS3302_CHANNELS_PER_ADCGROUP; ic++)
        {
            LOG<<"Chan: "<<iblock*SIS3302_CHANNELS_PER_ADCGROUP+ic;
            LOG<<" "<<parsedEvents[ic];
            for(int ievt=0; ievt<fmin(10,parsedEvents[ic]);ievt++){
                LOG<<" "<<ParseClock(&(vcard->databuffer[iblock*SIS3302_CHANNELS_PER_ADCGROUP+ic][evtPointer[ic][ievt]]));
            }
            LOG<<" ... ";
            for(int ievt=parsedEvents[ic]-10; ievt<parsedEvents[ic];ievt++){
                if(ievt<0) continue;
                LOG<<" "<<ParseClock(&(vcard->databuffer[iblock*SIS3302_CHANNELS_PER_ADCGROUP+ic][evtPointer[ic][ievt]]));
            }
            LOG<<ENDM_INFO;
        }
    }
    
    

    int minWords = vcard->databufferread[iblock*4];
    for(int ipmt=0; ipmt<SIS3302_CHANNELS_PER_ADCGROUP; ipmt++)
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
    ULong64_t rawclockC[SIS3302_CHANNELS_PER_ADCGROUP];

    packet->Clear();
    packet->setSlotId((vcard->adcheaderid[iblock])>>24);
    packet->setChannelId(iblock*SIS3302_CHANNELS_PER_ADCGROUP);
    

    if(parsedEvents[0]==0) return 0;
    
    for(int ievent =0; ievent<parsedEvents[0]; ievent++)
    {
        numberOfSampleWords=vcard->databuffer[iblock*4][evtPointer[0][ievent]+nomEventSize-1]&0x03FFFFFF;
        //For all data formats we have timestamp
        ULong64_t rawclock = 0;
        bool matchingClocks = true;
        
        for(int ic = 0; ic<SIS3302_CHANNELS_PER_ADCGROUP;ic++)
        {
            rawclockC[ic] = ParseClock(&(vcard->databuffer[iblock*4][evtPointer[ic][ievent]]));
            if(ic>0&&rawclockC[ic]!=rawclockC[ic-1])
                matchingClocks=false;
        }

        if(!matchingClocks)
        {
            LOG<<"Mismatched clocks 2 for block "<<iblock;
            LOG<<" event "<<nevents;
            for(int ic = 0; ic<SIS3302_CHANNELS_PER_ADCGROUP;ic++)
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
        hit->SetChannel(iblock*SIS3302_CHANNELS_PER_ADCGROUP);
        
        unsigned short* wf[4];
        for(int ipmt=0; ipmt<4; ipmt++)
        {
            wf[ipmt]=(unsigned short*)(&(vcard->databuffer[iblock*4+ipmt][evtPointer[ipmt][ievent]+nomEventSize]));
        }
        bool saveAllWaveforms = true;
        if(saveAllWaveforms){
            hit->SetNSamples(numberOfSampleWords*2*4);
            for(int ipmt = 0; ipmt<SIS3302_CHANNELS_PER_ADCGROUP; ipmt++)
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
            for(int ic = 0; ic<SIS3302_CHANNELS_PER_ADCGROUP;ic++)
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

void NGMSIS3302RawReader::PlotFirstWaveform(sis3302card* vcard, int ichan)
{
    TString cName;
    cName.Form("cWaveSlot%d_%d",vcard->modid,ichan);
    
    TCanvas* c1 = dynamic_cast<TCanvas*>(gROOT->FindObjectAny(cName.Data()));
    if(!c1) c1 = new TCanvas(cName.Data(),cName.Data());
    c1->cd();
    
    int iblock = ichan/SIS3302_CHANNELS_PER_ADCGROUP;
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



