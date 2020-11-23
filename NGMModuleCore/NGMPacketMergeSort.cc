#include "NGMPacketMergeSort.h"
#include "NGMBufferedPacket.h"
#include "NGMSystemConfiguration.h"
#include "NGMConfigurationTable.h"
#include "NGMConfigurationParameter.h"
#include "NGMLogger.h"
#include "TObjArray.h"
#include "TList.h"
#include "NGMHit.h"
#include "TClass.h"
#include <Riostream.h>
#include <limits>
#include "TObjString.h"
#include "TThread.h"
#include "TROOT.h"
#include "NGMSystem.h"
#include "TFolder.h"
#include "NGMSimpleParticleIdent.h"
#include "TArrayI.h"
#include "sis3316card.h"

ClassImp(NGMPacketMergeSort)

NGMPacketMergeSort::NGMPacketMergeSort()
{
  InitCommon();
}

NGMPacketMergeSort::NGMPacketMergeSort(const char* name, const char* title)
: NGMPacketBufferIO(name,title)
{
  InitCommon();
  initialize();
}

void NGMPacketMergeSort::InitCommon()
{
  _requiredSlots = 2;
  _packetArray = 0;
  _hitArray = 0;
  _bufferFlushWatermark = 10;
  _numpackets = 0;
  _numhits = 0;
  _maxactivechannels = 36;
  _channelsPerSlot = 8;
  _channelsPerBuffer = 1;
  _mergemode = WaitOnPackets;
  _prevchanID = -1;
  _hTimeStampErrors = 0;
  _hTotalEvents = 0;
  _avgcomps = 0.0;
  _ncompares = 0;
  _maxLiveTime = 1E20;
  _maxLiveTimeReached = false;
  _plotfrequency = 10.0;
  _livetimeOfLastPlot = 0.0;
  _saveOnPlotUpdate = false;
  _resetOnPlotUpdate = false;
  _hitSortedArray = 0;
  _maxTimingCorrection = 10.0;
}

void NGMPacketMergeSort::setMergeMode(int newVal)
{
	_mergemode = (MergeMode)newVal;
}

void NGMPacketMergeSort::setRequiredBuffersPerSlot(int newVal){
  _bufferFlushWatermark=newVal;
}


int NGMPacketMergeSort::getMergeSlot(const NGMBufferedPacket* packet) const
{
  // This code maps hardware buffers to pipelined data virtual slots
  
  //For PIXIE16 System all channels on a card are readout together
  if(_channelsPerBuffer == 16) return packet->getSlotId();
  
  //For SIS3302, SIS3320 all channels are readout independently
  return getChannelIndex(packet->getHit(0));
}

int NGMPacketMergeSort::getChannelIndex(const NGMHit* tHit) const
{
  if(!tHit) return -1;
  int slotid = tHit->GetSlot();

  //  slotid = tHit->GetSlot() + tHit->GetChannel();      
  // Special case for when SIS slotids were recorded as vme memory address
  if(slotid>=0x4000)
    slotid = (tHit->GetSlot()- 0x4000)/0x800;
  

  if(_channelsPerBuffer == 1)
  {
    return slotid *_channelsPerSlot + tHit->GetChannel();      
  }
  
  return slotid;
}

bool NGMPacketMergeSort::init()
{
  initialize();
  
  if(!_hTotalEvents){
    TThread::Lock();
    // create histogram for this channel
    char hname[1024];
    sprintf(hname,"%s_%s",GetName(),"TotalEvents");
    //LOG<<"Creating histogram "<<hname<<ENDM_INFO;
    _hTotalEvents = new TH1F(hname,hname,maxchannels,0.0,maxchannels);
    _hTotalEvents->SetDirectory(0);
	  _hTotalEvents->SetYTitle("Number of Events");
	  _hTotalEvents->SetXTitle("Channel Number");
    GetParentFolder()->Add(_hTotalEvents);
    TThread::UnLock();
  }else{
    _hTotalEvents->Reset();
  }

  if(!_hTimeStampErrors){
    TThread::Lock();
    // create histogram for this channel
    char hname[1024];
    sprintf(hname,"%s_%s",GetName(),"TimeStampErrors");
    //LOG<<"Creating histogram "<<hname<<ENDM_INFO;
    _hTimeStampErrors = new TH1F(hname,hname,maxchannels,0.0,maxchannels);
    _hTimeStampErrors->SetDirectory(0);
	  _hTimeStampErrors->SetYTitle("Number of TimeStampErrors");
	  _hTimeStampErrors->SetXTitle("Channel Number");
    GetParentFolder()->Add(_hTimeStampErrors);
    TThread::UnLock();
  }else{
    _hTimeStampErrors->Reset();
  }
  
  return true;
}

bool NGMPacketMergeSort::process(const TObject &tData)
{
  // Timer needs false argument to prevent reset
  _timer.Start(kFALSE);
    
  static TClass* tNGMBufferedPacketType = gROOT->GetClass("NGMBufferedPacket");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  static TClass* tNGMHitType = gROOT->GetClass("NGMHit");
  static TClass* tObjStringType = gROOT->GetClass("TObjString");

  //Check data type
  if(tData.InheritsFrom(tNGMBufferedPacketType)){
    
    const NGMBufferedPacket* packetBuffer = (const NGMBufferedPacket*)(&tData);

    if(packetBuffer->getSlotId() < 0)
    {
      // Well assume this is an already sorted and merged buffer
      // So we will serialize it... should check for an option to do this
      int hitcount = packetBuffer->getPulseCount();
      for(int id = 0; id < hitcount; id++)
      {
        const NGMHit* tHit = packetBuffer->getHit(id);
        static bool badhitdetected = false;
        if(partID->getPlotIndex(tHit)>=partID->getNumberOfChannels())
        {
          if(!badhitdetected)
          {
            LOG<<"There is a rogue channel index "<<partID->getPlotIndex(tHit)<<" potentially very serious error.!"<<ENDM_WARN;
            badhitdetected = true;
          }
          continue;
        }
        
        pushHit(packetBuffer->getHit(id));
      }
      return true;
    }

      // This is a packet well need to merge and sort
    NGMBufferedPacket* packetToAnalyze = packetBuffer->DuplicatePacket();
    if(packetToAnalyze->getPulseCount() < 1)
    {
        if(getVerbosity()>10)
          LOG<<"Empty Buffer for slot("<<packetToAnalyze->getSlotId()
            <<") chan("<<packetToAnalyze->getChannelId()<<")"<<ENDM_WARN;
    }else{
      const NGMHit* tHit = packetToAnalyze->getHit(0);
      packetToAnalyze->setTimeStamp(TTimeStamp(tHit->GetNGMTime().GetSec(),tHit->GetNGMTime().GetNanoSec()));
      pushPacket(packetToAnalyze);
    }
  }else if(tData.InheritsFrom(tNGMHitType)){
    const NGMHit* tHit = dynamic_cast<const NGMHit*>(&tData);
    pushHit(tHit);
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
	  // The right way to determine the number of channels is to query the configuration
	  // table for the number of active channels...  Until that is availible we'll
	  // guess...
	  int numberOfSlots = confBuffer->GetSlotParameters()->GetEntries();
    setRequiredSlots(numberOfSlots);
    if(!partID) partID = new NGMSimpleParticleIdent;
      partID->Init(confBuffer);
    _channelsPerSlot = confBuffer->GetChannelParameters()->GetEntries()/numberOfSlots;
    if(TString(confBuffer->GetName()).BeginsWith("PIXIE16"))
    {
      _channelsPerBuffer = 16;
      // Important that the configuration object accurately reflect how many
      // channels slots are present
      setRequiredSlots(confBuffer->GetSlotParameters()->GetEntries());
        setMergeMode(WaitOnPackets);
        setRequiredBuffersPerSlot(20);
      //Lets count active slots via ACTIVE flag in the SlotParametersTable
      int activeslots=0;
      if(confBuffer->GetSlotParameters()->GetParIndex("ACTIVE")>=0)
      {
	int islot = -1;
	while(++islot<confBuffer->GetSlotParameters()->GetEntries())
	  if(confBuffer->GetSlotParameters()->GetParValueI("ACTIVE",islot))
	    activeslots++;
	setRequiredSlots(activeslots);
      }
    }
    if(confBuffer->GetSystemParameters()->GetParIndex("CHANNELS_PER_BUFFER")>=0)
    {
      _channelsPerBuffer = confBuffer->GetSystemParameters()->GetParValueI("CHANNELS_PER_BUFFER",0);
    }
  if(_channelsPerBuffer == 1)
  {
    setRequiredSlots(confBuffer->GetChannelParameters()->GetEntries());
  }
      if(TString(confBuffer->GetName()).BeginsWith("SIS3316"))
      {
          _channelsPerBuffer = 1;
          setMergeMode(NoWaitOnPackets);
          // Important that the configuration object accurately reflect how many
          // channels slots are present
          // Extract Clock Settings for each slot
          const NGMConfigurationTable* cslot = confBuffer->GetSlotParameters();
          const NGMConfigurationParameter* cards = cslot->GetColumn("card");
          int numberOfDataBuffers = 0;
          for(int icard = 0; icard < cards->GetEntries(); icard++)
          {
              if(! confBuffer->GetSlotParameters()->GetParValueI("ModEnable",icard) ) continue;
              const sis3316card* card = dynamic_cast<const sis3316card*>(cards->GetValueO(icard));
              for(int iadc=0; iadc<SIS3316_ADCGROUP_PER_CARD; iadc++)
              {
                  if(card->IsBlockReadout(iadc))
                      numberOfDataBuffers+=1;
                  else
                      numberOfDataBuffers+=4;
              }
          }

          setRequiredSlots(numberOfDataBuffers);
       }
      
	  flush();
    _gapFinder.Reset();
    _livetimeOfLastPlot=0.0;
    _maxLiveTimeReached = false;
	  _prevchanID = -1;
    // Reset Histograms
    _hTimeStampErrors->Reset();
    _hTotalEvents->Reset();
    LOG<<"Active Slots for  "<<_requiredSlots<<ENDM_INFO;
    push(tData);
  }else if(tData.IsA() == tObjStringType){
    const TObjString* controlMessage = (const TObjString*)(&tData);
    if(controlMessage->GetString() == "EndRunFlush")
    {
      flush();
    }else if(controlMessage->GetString() == "EndSpillFlush" &&  _mergemode == NoWaitOnPackets)
	{
      // Flush a single spills worth of data
	  flush();
	  _prevchanID = -1;
	  if(getVerbosity()>0)
      {
        double cputime = _timer.CpuTime();
        if(cputime <= 0.0) cputime = -1.0;
        LOG<<"NGMPacketMergeSort("<<GetTitle()<<") Packets("<<_numpackets
          <<") NumHits("<<_numhits<<") PacketCPURate("<<((double)_numpackets)/cputime
          <<") HitCPURate("<<((double)_numhits)/cputime
          <<") AvgCompares("<<_avgcomps<<")"<<ENDM_INFO;

      }
	  
    }
    push(tData);
  }else{
    // We will pass through hits assuming they are already sorted.
    push(tData);
  }
  _timer.Stop();
  return true;
}
bool NGMPacketMergeSort::finish()
{
  flush();

  double cputime = _timer.CpuTime();
  if(cputime <= 0.0) cputime = -1.0;
  LOG<<"NGMPacketMergeSort("<<GetTitle()<<") Packets("<<_numpackets
    <<") NumHits("<<_numhits<<") PacketCPURate("<<((double)_numpackets)/cputime
    <<") HitCPURate("<<((double)_numhits)/cputime<<ENDM_INFO;

  return true;
}


void NGMPacketMergeSort::initialize(){
  _requiredSlots = 2;
  _slotCount = 0;
  _numpackets = 0;
  _numhits = 0;
// Arrays are object owners
  if(!_packetArray) _packetArray = new TObjArray(maxchannels);
  _packetArray->SetOwner();
  if(!_hitArray) _hitArray = new TObjArray(maxchannels);
  _hitArray->SetOwner();
  if(!_hitSortedArray) _hitSortedArray = new TObjArray(maxchannels);
  _timer.Stop();
  _timer.Reset();
}

NGMPacketMergeSort::~NGMPacketMergeSort()
{
  // loop over the arrays and delete containers
  // then delete arrays
  if(_packetArray){
    _packetArray->Delete();
    delete _packetArray;
//    int lastid = _packetArray->GetLast();
//    for(int id = 0; id <=lastid;id++){
//      TList* tmp_packetArray->At(id);
//    }
  }
  if(_hitArray){
    _hitArray->Delete();
    delete _hitArray;
  }
  if(_hitSortedArray){
    _hitSortedArray->Delete();
    delete _hitSortedArray;
  }
}

int NGMPacketMergeSort::setRequiredSlots(int newVal){
  _requiredSlots = newVal;
  _maxactivechannels = newVal;
  return 0;
}

int NGMPacketMergeSort::pushPacket(NGMBufferedPacket* packet){
  int slotid = 0;
  if(packet->getPulseCount()>0)
  {
    slotid = getMergeSlot(packet);
  }else{
    return 0;
  }
  if(getVerbosity()>10) LOG<<"Analyzing packet "<<slotid<<" MinBuffersInQueue() "<<minBuffersInQueue()<<ENDM_INFO;
  if(!_packetArray) initialize();
  _numpackets++;

  TList* tList = 0;
  // This is a quick fix for excluding unused channels 
  if(_idMap.count(slotid) < 1 && _idMap.size()==(unsigned int)_maxactivechannels)
  {
    if(getVerbosity()>10)
      LOG<<"skipping data for slot "<<slotid<<ENDM_WARN;
    delete packet;
    return 0;
  }
  // Check if we've already seen this slot
  // if not create buffers for this slot
  if(_idMap.count(slotid) < 1 ){
    _idMap[slotid] = _slotCount;
    tList = new TList;
    (*_packetArray)[_slotCount] = tList;
    (*_hitArray)[_slotCount] = new TList;
    _slotCount++;
    if(getVerbosity()>10)
      LOG<<"First packet of slot "<<slotid<<". Creating list "<<_slotCount-1<<ENDM_INFO;    

  }else{
    tList = (TList*) (*_packetArray)[_idMap[slotid]];
  }
  
  // Check for the mode of merge in which we assume all time sorted data is in a single buffer
  // If the slotid is greater than the previous slotid, then well go ahead and merge the data so far.
  if(_mergemode == NoWaitOnPackets)
  {
	  if(_prevchanID >= slotid && _prevchanID > -1)
	  {
		  flush();
      if(getVerbosity()>0)
      {
        double cputime = _timer.CpuTime();
        if(cputime <= 0.0) cputime = -1.0;
        LOG<<"NGMPacketMergeSort("<<GetTitle()<<") Packets("<<_numpackets
          <<") NumHits("<<_numhits<<") PacketCPURate("<<((double)_numpackets)/cputime
          <<") HitCPURate("<<((double)_numhits)/cputime
          <<") AvgCompares("<<_avgcomps<<")"<<ENDM_INFO;
      }

	  }
  }
  _prevchanID = slotid;
  
  
  
  
  // Sort packet, time calibrations should
  // have been applied prior
  if(_channelsPerBuffer != 1) packet->sortHits();
  
  // Now add slot to the appropriate list
  tList->AddLast(packet);

  // If we are waiting on packets we'll merge using the conditions below
  // 
  if(_mergemode == WaitOnPackets)
  if(_slotCount >= _requiredSlots)
    if(minBuffersInQueue() >= _bufferFlushWatermark){
      //Lets flush a packet's worth of hits into the Hit Lists
      for(int slotid = 0; slotid < _slotCount; slotid++){
        if(getVerbosity()>10) std::cout<<"Flushing buffer Slot("<<slotid<<")\n";
        while(((TList*)(_packetArray->At(slotid)))->GetSize()>=_bufferFlushWatermark)
        {
          flushBufferToList(slotid);
        }
        // We must sort because it is possible that a
        // hit in the later buffer has timing that is 
        // slightly earlier than a hit in the earlier
        // buffer.  We could probably be more clever
        // with this and this may not be the correct place
        // to do it.
        
        //((TList*)((*_hitArray)[slotid]))->Sort();
      }
      // Now that there is more data in the lists
      // lets calculate the latest time for which 
      // it is safe to assume all the hits are in
      // the lists
      NGMTimeStamp earliestBufferTime = earliestTimeInNextBuffers();
      earliestBufferTime.IncrementNs(-_maxTimingCorrection);
      if(getVerbosity()>10)
      {
        std::cout<<" calling merge hits "<<earliestBufferTime.GetSec()<< " "<<earliestBufferTime.GetNanoSec()<<std::endl;
        for(int id = 0; id < _slotCount; id++){
          if( ((TList*)(_hitArray->At(id))) > 0){
            std::cout<<id<<"     ("<< ((TList*)(_hitArray->At(id)))->LastIndex()+1<<") ";
            NGMHit* last = dynamic_cast<NGMHit*>(((TList*)(_hitArray->At(id)))->Last());
            NGMHit* first = dynamic_cast<NGMHit*>(((TList*)(_hitArray->At(id)))->First());
            std::cout<<" B("<<first->GetNGMTime().GetSec()<<" "<<first->GetNGMTime().GetNanoSec()<<") ";
            std::cout<<" E("<<last->GetNGMTime().GetSec()<<" "<<last->GetNGMTime().GetNanoSec()<<") ";
            std::cout<<" D("<<last->TimeDiffNanoSec(first->GetNGMTime())<<")";
            std::cout<<" Size("<<((TList*)(_hitArray->At(id)))->GetSize()<<")";
            std::cout<<std::endl;
          }
       }
      }
      mergeHitsToLatestTime(earliestBufferTime);
      if(getVerbosity()>11)
      {
        std::cout<<" called merge hits "<<earliestBufferTime.GetSec()<< " "<<earliestBufferTime.GetNanoSec()<<std::endl;
        for(int id = 0; id < _slotCount; id++){
          if( ((TList*)(_hitArray->At(id))) > 0){
            std::cout<<id<<"     ("<< ((TList*)(_hitArray->At(id)))->LastIndex()+1<<") ";
            NGMHit* last = dynamic_cast<NGMHit*>(((TList*)(_hitArray->At(id)))->Last());
            NGMHit* first = dynamic_cast<NGMHit*>(((TList*)(_hitArray->At(id)))->First());
            std::cout<<" B("<<first->GetNGMTime().GetSec()<<" "<<first->GetNGMTime().GetNanoSec()<<") ";
            std::cout<<" E("<<last->GetNGMTime().GetSec()<<" "<<last->GetNGMTime().GetNanoSec()<<") ";
            std::cout<<" D("<<last->TimeDiffNanoSec(first->GetNGMTime())<<")";
            std::cout<<" Size("<<((TList*)(_hitArray->At(id)))->GetSize()<<")";
            std::cout<<std::endl;
          }
        }
      }
      
    }

    return 0;
}

int NGMPacketMergeSort::minBuffersInQueue()
{
  int mincount = 1000;
  TArrayI slotCnt(_slotCount);
  for(int id = 0; id < _slotCount; id++){
    slotCnt[id]=((TList*)(_packetArray->At(id)))->GetSize();
    if( ((TList*)(_packetArray->At(id)))->GetSize() < mincount)
      mincount = ((TList*)(_packetArray->At(id)))->GetSize();
  }
  if(getVerbosity()>10){
    std::cout<<"Min buffers in "<< _slotCount<<" queues "<<mincount<<" (";
    for(int id = 0; id < _slotCount; id++)
    {
      std::cout<<" "<<slotCnt[id];
    }
    std::cout<<" )";
    std::cout<<std::endl;
  }
  return mincount;
}

NGMTimeStamp NGMPacketMergeSort::earliestTimeInNextBuffers() const
{
  //Set Time To Far Future
  NGMTimeStamp minTime(std::numeric_limits<int>::max()/*2147483647*/, 0,0);
  for(int id = 0; id < _slotCount; id++){
    if( ((TList*)(_packetArray->At(id)))->GetSize() > 0){
      NGMBufferedPacket* tPacket = dynamic_cast<NGMBufferedPacket*>(((TList*)(_packetArray->At(id)))->First());
      NGMTimeStamp bufferTime;
      if(tPacket->getPulseCount() > 0)
        bufferTime = tPacket->getHit(0)->GetNGMTime();
      else  //For an empty buffer we use the buffer timestamp
            // This likely needs some tuning
        bufferTime = NGMTimeStamp(tPacket->getTimeStamp());
      if(minTime>bufferTime) minTime = bufferTime;
    }
  }
  return minTime;
}

NGMTimeStamp NGMPacketMergeSort::latestTimeInHitLists() const
{
  //Set Time To Minimum
  NGMTimeStamp maxTime(0,0,0);
  // Find the latest time in the Lists of hits
  for(int id = 0; id < _slotCount; id++){
    if( ((TList*)(_hitArray->At(id)))->GetSize() > 0){
      NGMHit* tHit = dynamic_cast<NGMHit*>(((TList*)(_hitArray->At(id)))->Last());
      NGMTimeStamp hitTime;
      if(tHit)
        if(maxTime < tHit->GetNGMTime())
          maxTime = tHit->GetNGMTime();
    }
  }
  return maxTime;
}


int NGMPacketMergeSort::flushBufferToList(int index)
{
  int numadded = 0;
  // bounds check the index
  if(index < 0 || index >= _slotCount) return -1;
  
  // Get the packet list and hit list for this index
  NGMBufferedPacket* tPacket = dynamic_cast<NGMBufferedPacket*>(((TList*) (_packetArray->At(index)))->First());
  TList* tHitList = (TList*) (_hitArray->At(index));

  if(tPacket){
    // Copy contents to List
    int hitcount = tPacket->getPulseCount();
    for(int id = 0; id < hitcount; id++){
      // Test if previous hit is later than current
      int plotIndex = getChannelIndex(tPacket->getHit(id));
      _hTotalEvents->Fill(plotIndex);
      if(id > 0 && tHitList->Last() > 0)
      {
        if(tPacket->getHit(id)->GetNGMTime() < ((NGMHit*)(tHitList->Last()))->GetNGMTime() && _channelsPerBuffer!=1)
        {
          _hTimeStampErrors->Fill(plotIndex);
          continue;
        }
      }
      tHitList->AddLast(tPacket->getHit(id)->DuplicateHit());
      numadded++;
    }
    tHitList->Sort();
    ((TList*)(_packetArray->At(index)))->Remove(tPacket);
    //RJN Do not use delete packet. ROOT May need me to call tPacket->Delete()
    delete tPacket;
  }
  return numadded;
}

int NGMPacketMergeSort::mergeHitsToLatestTime(NGMTimeStamp latestTime){

  // Sort earliest hit of each buffer into array
  // to find the earliest hit of all then flush
  // hit from sorted array and hitList to daughter modules
  // replacing with hit from the same buffer to be resorted and repeated
  TList hitSArray;
  for(int slotid = 0; slotid < _slotCount; slotid++){
    NGMHit* tHit = dynamic_cast<NGMHit*>(((TList*)(_hitArray->At(slotid)))->First());
    if(tHit) hitSArray.AddLast(tHit);
  }
  //Now Sort our little short list of leading candidates
  // for the next hit in time
  bool firstSort = false;
  while(true){
    // We should only need sort the first time
    // for subsequent loops we just insert in the
	// correct place.
	if(!firstSort)
	{
		hitSArray.Sort();
		firstSort = true;
	}
    NGMHit* nextHit = dynamic_cast<NGMHit*>(hitSArray.First());
    if(nextHit){
      // Lets remove the hit from both lists
      // Then add the next hit from the hitList of the same slot
      // and forward old hit to daughter modules

      //List for this slot

      TList* hitList = (TList*)(_hitArray->At(_idMap[getChannelIndex(nextHit)]));
      
      hitSArray.Remove(nextHit);
      hitList->Remove(nextHit);

      pushHit(nextHit);
      delete nextHit;

      // Lets check the next candidate
      NGMHit* nextCandidate = dynamic_cast<NGMHit*>(hitList->First());
      if(nextCandidate) {
        if(nextCandidate->GetNGMTime()<latestTime){
 		  // The smart thing is to insert in the
		  // correct place and then no need to sort again
		TIter itr(&hitSArray);
		bool hitIsPlaced = false;
		NGMHit* tHit = 0;
      int ncomps = 0;
      while( (tHit = (NGMHit*)(itr.Next())) )
		{
         ncomps++;
			if(tHit->Compare(nextCandidate) >= 0)
			{
				hitSArray.AddBefore(tHit,nextCandidate);
				hitIsPlaced = true;
				break;
			}
		}
      _avgcomps = (_avgcomps * _ncompares + ncomps)/(double)(_ncompares+1);
      _ncompares++;
		// Apparently it belongs at the end
		if(!hitIsPlaced)
			hitSArray.AddLast(nextCandidate);

	}else{
          return 2;
        }
      }
    }else{
      return 1;
    }
    
  }
  return 0;
}

int NGMPacketMergeSort::pushHit(const NGMHit* tHit){
  if(_maxLiveTimeReached) return 0;
  
  if(!tHit) return -1;
//  TIter next(GetListOfTasks());    // GetTaskList Member method of TTask                       
//  while (NGMPacketBufferIO *obj = dynamic_cast<NGMPacketBufferIO*>(next())){                                
//    obj->pushPacket(packetBuffer);
//  }
  _numhits++;

  if(getVerbosity()>10000) std::cout<<"Next Hit: Slot("<< tHit->GetSlot() 
           << ")  "<<tHit->GetNGMTime().GetSec()
           << "\t" <<tHit->GetNGMTime().GetNanoSec()
           << std::endl;
  
  // Track Live Time for this run
  if(_gapFinder.nextTimeIsGap(tHit->GetNGMTime()))
  {
    TObjString gapDetected("GapDetected");
    push(*((const TObject*)&gapDetected));
  }
  
  // Check for PlotUpdate
  if(_plotfrequency>0.0)
    if ( ((_gapFinder.GetLiveTime()*1E-9)-_livetimeOfLastPlot) > _plotfrequency )
    {
      _livetimeOfLastPlot = (_gapFinder.GetLiveTime()*1E-9);
      // Send plot request signal to all modules.
      TObjString plotUpdate("PlotUpdate");
      push(*((const TObject*)&plotUpdate));
      // Check if we should also request save the analysis tree
      if(_saveOnPlotUpdate)
      {
        if(NGMSystem::getSystem())
        {
          NGMSystem::getSystem()->SaveAnaTree();
        }
      }
      if(_resetOnPlotUpdate)
      {
        TObjString plotReset("PlotReset");
        push(*((const TObject*)&plotReset));
      }
    }
  
  //Pass to all daughters
  if( _gapFinder.GetLiveTime()< (1E9*_maxLiveTime ))
  {
    push(*((const TObject*) tHit));
  }else{
    if(NGMSystem::getSystem())
    {
      _maxLiveTimeReached = true;
      NGMSystem::getSystem()->RequestAcquisitionStop();
      LOG<<"Request Acquisition Stop after "<<_gapFinder.GetLiveTime()*1E-9<<" seconds livetime "
         <<_gapFinder.GetRunDuration()*1E-9<<" seconds total running time."<<ENDM_INFO;
    }
  }
  return 0;
}

int NGMPacketMergeSort::flush(){
  // Timer start needs false argument to prevent reset
  _timer.Start(kFALSE);
  for(int id = 0; id < _slotCount; id++){
    while(flushBufferToList(id)){ }
  }
  NGMTimeStamp maxTime(std::numeric_limits<int>::max(),0);
  mergeHitsToLatestTime(maxTime);
  _timer.Stop();
//  _timer.Print();

  return 0;
}
