//
//  PIXIE16RawReaderv4.cpp
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/15/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "PIXIE16RawReaderv4.h"
#include "NGMLogger.h"
#include "NGMHit.h"
#include "NGMBufferedPacket.h"
#include "NGMSystemConfiguration.h"
#include "NGMConfigurationTable.h"
#include "TThread.h"
#include "TFile.h"
#include "NGMModule.h"
#include "NGMBufferedPacket.h"
#include <stdlib.h>
#include "NGMTimingCal.h"
#include "NGMSimpleParticleIdent.h"

////// PIXIE16RawReaderv4 Implementation
ClassImp(PIXIE16RawReaderv4)

PIXIE16RawReaderv4::PIXIE16RawReaderv4()
{
  _rawhdrbuffer = 0;
  _rawbuffer = 0;
  _nrawbuffers = 0;
  _rawbufferstart = 0;
  _runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
  _hitPool = 0;
  _hitStream = 0;
  _minBufferDuration_ns = 1000.0;
}

PIXIE16RawReaderv4::PIXIE16RawReaderv4(NGMModule* parent)
: NGMReaderBase(parent)
{
  _rawhdrbuffer = new unsigned int[2000000];
  _rawbuffer = 0; //new unsigned int[2000000];
  _nrawbuffers = 0;
  _rawbufferstart = 0;
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
  _filecounter = 0;
  _hitPool = 0;
  _hitStream = 0;
  _minBufferDuration_ns = 1000.0;
  
}

PIXIE16RawReaderv4::~PIXIE16RawReaderv4()
{
  if(_inputfile)
  {
    if(_inputfile->is_open())
      _inputfile->close();
    delete _inputfile;
  }
  if(_rawhdrbuffer)
    delete [] _rawhdrbuffer;
  deleteRawBuffers();
}

void PIXIE16RawReaderv4::SetConfiguration(const NGMSystemConfiguration* sysConf)
{
  // Let Base keep a copy of configuration
  NGMReaderBase::SetConfiguration(sysConf);
  
  // Extract Clock Settings for each slot
  NGMConfigurationTable* slotPars = GetConfiguration()->GetSlotParameters();
  
  _NanoSecondsPerClock.Set(slotPars->GetEntries());
  for(int islot = 0; islot < slotPars->GetEntries(); islot++)
  {
    _NanoSecondsPerClock[islot] = 10.0; //Hard coded for now
    if(GetConfiguration()->GetSlotParameters()->GetParIndex("NanoSecondsPerSample")>=0){
      _NanoSecondsPerClock[islot] = GetConfiguration()->GetSlotParameters()->GetParValueD("NanoSecondsPerSample",islot); //Hard coded for now
    }
    //LOG<<"For DEBUGGING forcing nanoseconds per sample to 10.0 "<<ENDM_WARN;
    //_NanoSecondsPerClock[islot] = 10.0; //Hard coded for now
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
  if(!_hitPool) _hitPool = new NGMHitPoolv6();
  if(!_hitStream){
    _hitStream = new NGMTimingCal();
    _hitStream->_maxtimediff_ns = -1.0;
  }
  _activeSlot.resize(GetConfiguration()->GetSlotParameters()->GetEntries());
  if(GetConfiguration()->GetSlotParameters()->
     GetParIndex("ACTIVE")){
      for(int islot = 0; islot<_activeSlot.size(); islot++){
          _activeSlot[islot]=GetConfiguration()->GetSlotParameters()->
          GetParValueI("ACTIVE", islot);
      }
  }
  // Lets delete and reallocate NGMTimingCal containters.
  // These containers own their objects
  // and should delete their objects
  for(int ibuf = 0; ibuf < _hitBuffers.size(); ibuf++){
    delete _hitBuffers[ibuf];
  }
  _hitBuffers.resize(_nrawbuffers);
  for(int ibuf = 0; ibuf < _hitBuffers.size(); ibuf++){
    if(_tcal){
      _hitBuffers[ibuf] = new NGMTimingCal(*_tcal);
      if(_tcal->_maxtimediff_ns<_minBufferDuration_ns){
        _hitBuffers[ibuf]->_maxtimediff_ns = _minBufferDuration_ns;
      }
    }else{
      _hitBuffers[ibuf] = new NGMTimingCal();
      _hitBuffers[ibuf]->_maxtimediff_ns = _minBufferDuration_ns;
    }
    
  }

}

Long64_t PIXIE16RawReaderv4::OpenInputFile(const char* filename)
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
  
  deleteRawBuffers();
  createRawBuffers(confBuffer->GetSlotParameters()->GetEntries());
    
  // Save information we need from configuration
  SetConfiguration(confBuffer);
  
  OpenRawBinaryFile(rawFileName.Data());
  TThread::UnLock();
  return 0;

}

void PIXIE16RawReaderv4::deleteRawBuffers(){
    for(int imod = 0; imod < _nrawbuffers;imod++)
    {
        delete [] _rawbuffer[imod];
    }
    delete [] _rawbuffer;
    delete [] _rawbufferstart;
    for(int ibuf = 0; ibuf < _hitBuffers.size(); ibuf++){
        delete _hitBuffers[ibuf];
    }
}

void PIXIE16RawReaderv4::createRawBuffers(int nmodules)
{
    _nrawbuffers = nmodules;
    _rawbuffer = new unsigned int*[nmodules];
    _rawbufferstart = new unsigned int[nmodules];
    for(int imod = 0; imod < _nrawbuffers;imod++)
    {
        _rawbuffer[imod] =  new unsigned int[2000000];
        _rawbufferstart[imod] = 0;
    }
}

bool PIXIE16RawReaderv4::pushHit(NGMHit* hit)
{
  if(_tcal&&GetParent())
  {
    _tcal->pushHit(hit);
    //std::cout<<"Slot: "<<hit->GetSlot()<<" "<<hit->GetSlot()<<" "<<hit->GetRawClock()<<std::endl;
    
    NGMHit* nexthit = 0;
    while( (nexthit =_tcal->nextHit()) )
    {
      GetParent()->push(*((const TObject*)(nexthit)));
      //      std::cout<<"Slot("<<nexthit->GetSlot();
      //      std::cout<<") Channel("<<nexthit->GetChannel();
      //      std::cout<<") RawClock("<<nexthit->GetRawClock();
      //      std::cout<<") CFD("<<nexthit->GetCFD();
      //      std::cout<<") time("<<nexthit->TimeDiffNanoSec(NGMTimeStamp(_runBegin)) ;
      //      std::cout<<")"<<std::endl;
      _hitPool->ReturnHit( nexthit);
    }
  }else if(GetParent()){
    GetParent()->push(*((const TObject*)(hit)));
    _hitPool->ReturnHit(hit);
  }
  return true;
}

Long64_t PIXIE16RawReaderv4::OpenRawBinaryFile(const char* pathname)
{
  FileStat_t pathinfo;
  gSystem->GetPathInfo(pathname,pathinfo);
  
  if(! _inputfile)   _inputfile = new std::ifstream;
  
	if(_inputfile->is_open()) _inputfile->close();
	
  _inputfile->open(pathname,std::ios::in|std::ios::binary);
  _nextbufferPosition = 0;
  
  if(!_inputfile->is_open())
  {
    LOG<<" Unable to open file "<<pathname<<ENDM_WARN;
    return 2;
  }

  // Read header
  const int bytesInRunHeader = 400;
  Long64_t result = readBytesWaiting(bytesInRunHeader,_rawhdrbuffer,true);
  if(result!=bytesInRunHeader)
  {
    LOG<<" Full Header not found in buffer "<<pathname<<ENDM_INFO;
    return 1;
  }
  
  LOG<<"Found XIA Run Header of version "<<_rawhdrbuffer[0] << " in file "<< pathname << ENDM_INFO;
  _fileversion = _rawhdrbuffer[0];
  //  LOG<<"For DEBUGGING Forcing fileversion to 0x1002 in file "<< pathname << ENDM_INFO;
  //  _fileversion = 0x1002;
  
  return 0;
}

bool PIXIE16RawReaderv4::MinDataPresentForMerge(){
    for(int islot = 0; islot<_activeSlot.size();islot++){
      if(_activeSlot[islot]&&_hitBuffers[islot]->empty()) return false;
    }
    return true;
}

Long64_t PIXIE16RawReaderv4::ReadAll()
{
  int nwordsinbuffer = 0;
  int i = 0;
  
  // Lets read all the data until we have enough to begin merging
  while( ! _abortread && nwordsinbuffer>=0 && !MinDataPresentForMerge() )
  {
    nwordsinbuffer = ReadNextBufferFromFile();
    if(nwordsinbuffer < 0) break;
    
    i++;
    if(i%100 == 0)
    {
      gSystem->ProcessEvents();
    }
  }
  if(!MinDataPresentForMerge()) return 0;
  
  for(int islot = 0; islot<_activeSlot.size();islot++){
    if(_activeSlot[islot]){
      //This is safe since we just passed the MinDataPresentForMerge test
      _hitStream->pushHit(_hitBuffers[islot]->nextHit());
    }
  }

  // Now lets write the steady state merging
  while( ! _abortread )
  {
    int slot = 0;
    NGMHit* hit = 0;
    do{
      hit = _hitStream->nextHit();
      slot = hit->GetSlot();
      pushHit(hit);
      hit = _hitBuffers[slot]->nextHit();
      if(!hit) break;
      _hitStream->pushHit(hit);
    }while(true);
    //Looks like we need more data
    while( ! _abortread && !MinDataPresentForMerge() )
    {
      nwordsinbuffer = ReadNextBufferFromFile();
      if(nwordsinbuffer < 0) break;
      i++;
      if(i%100 == 0)
      {
        gSystem->ProcessEvents();
      }
    }
    if(!MinDataPresentForMerge()) break;
    // Now that we have the minimum again we should be able to continue
    // and we can add the hit from the previous
    hit = _hitBuffers[slot]->nextHit();
    if(!hit) break;
    _hitStream->pushHit(hit);
  }
  
  // Flush any last events
  do{
    NGMHit* hit = _hitStream->nextHit(-1.0); // -1.0 forces it to give a hit if its there
    if(!hit) break;
    int slot = hit->GetSlot();
    pushHit(hit);
    hit = _hitBuffers[slot]->nextHit();
    if(hit) _hitStream->pushHit(hit);
  }while(true);
  
  if(_tcal&&GetParent()){
    while(!_tcal->_hitbuffer.empty()){
      NGMHit* hit = _tcal->nextHit(-1.0); // -1.0 forces it to give a hit if its there
      if(!hit) break;
      GetParent()->push(*((const TObject*)(hit)));
      _hitPool->ReturnHit(hit);
    }
  }
  return 0;
  
}

Long64_t PIXIE16RawReaderv4::CloseInputFile()
{
  if(_inputfile)
  {
    if(_inputfile->is_open())
      _inputfile->close();
    delete _inputfile;
    _inputfile = 0;
  }
  return 0;
}

Long64_t PIXIE16RawReaderv4::ReadNextBufferFromFile()
{
  return ReadNextBufferFromFilev2();
}

Long64_t PIXIE16RawReaderv4::ReadNextBufferFromFilev2()
{
  const UInt_t EVENT_LENGTH_SHIFT = 			17;
  const UInt_t EVENT_LENGTH_MASK = 			0x7FFE0000;
  
  //We begin by reading the header for a packet
  const int nbytesPacketHeader = 10*4;
  // This is a little different from v1 in that we expect a sequence of 1 summed header
  // and 4 individual channel computations.
  
  // A 10 word packet header contains module id and packet length
  
  int bytesRead = readBytesWaiting(nbytesPacketHeader,_rawhdrbuffer,true);
  if(nbytesPacketHeader!=bytesRead)
  { 
    if(bytesRead == 0){
      return -1;
    }else{
      LOG<<"ERROR reading packet header"<<ENDM_FATAL;
      return 0;
    }
  }
  
  int moduleid = _rawhdrbuffer[0];
  int packetLengthInWords = _rawhdrbuffer[1];
  
  if (packetLengthInWords > 50000000) {
    LOG<<" Suspiciously large packet size: "<<packetLengthInWords<<" for module "<<moduleid<<ENDM_FATAL;
    exit(-1);
  }

  if(packetLengthInWords==0)
  {
     return 0; 
  }
    
  bytesRead = readBytesWaiting(packetLengthInWords*4,_rawbuffer[moduleid],true,_rawbufferstart[moduleid]*4);
  if(bytesRead>0)
  {
    Long64_t status = writePacket(packetLengthInWords+_rawbufferstart[moduleid], _rawbuffer[moduleid], _hitBuffers[moduleid], moduleid);
    if(status) return 0;
  }else if(bytesRead==0){
      return 0;
  }else
      return -1;
  return packetLengthInWords;
}


int PIXIE16RawReaderv4::AnaStats(NGMBufferedPacket* packet)
{
  // Basic analysis of packet
  int nHits = packet->getPulseCount();
  
  if(nHits>=1)
  {
    const NGMHit* tHit = packet->getHit(0);
    int plotIndex = tHit->GetSlot()*8 + tHit->GetChannel();
    
    
	  double hitRate = 0.0;
    if(nHits>=2) hitRate = nHits/(packet->getHit(nHits-1)->TimeDiffNanoSec(packet->getHit(0)->GetTimeStamp())*1E-9);
    // Assuming we read in plot index order lets assume we have completely
    // readout a spill if the previous plotIndex > current plotIndex
    
    _earliestTimeInSpill = packet->getHit(0)->GetTimeStamp();
    _latestTimeInSpill = packet->getHit(nHits-1)->GetTimeStamp();
    
    _totalEventsThisRun+=nHits;
    _lastPlotIndex = plotIndex;
    
    
    //If _runBegin is still 0 then this is the first spill for this run
    if(_firstTimeOfRun.GetSec() == 0)
    {
      _firstTimeOfRun = _earliestTimeInSpill;
    }
    //We update livetime
    _runduration = _latestTimeInSpill - _firstTimeOfRun;
    _livetime += (double)(_latestTimeInSpill - _earliestTimeInSpill);
    if(nHits>=2){
      LOG<<"LiveTime( "<<_livetime
      <<" RunDuration( "<<_runduration;
      if(_runduration > 0)
        LOG<<" ) LiveTime( "<<_livetime/_runduration*100.0<<"% ";
      LOG<<" ) AverageHitRate ( "<<_totalEventsThisRun/_livetime<<" ) "<<ENDM_INFO;          
    }
    
    
  }
  //LOG<<"Sending Packet Slot("<<packetBuffer->getHit(0)->GetSlot()
  //   <<") Channel("<<packetBuffer->getHit(0)->GetChannel()
  //   <<")"<<ENDM_INFO;
    return 0;
}

Long64_t PIXIE16RawReaderv4::readBytesWaiting(Long64_t nbytesToRead, unsigned int* rawbuffer, bool waitOnData, size_t localOffsetInBytes)
{
  // If the end of file we will return 0
  // If expected bytes not found return -1
  // Otherwise return number of bytes read...
  
  if(!_inputfile->is_open())
  {
    LOG<<" No file appears to be open for input "<<ENDM_WARN;
    return 0;
  }
  
  if(!rawbuffer)
  {
    LOG<<" No raw buffer was allocated "<<ENDM_WARN;
    return 0;
  }
  
  size_t filesize;
  
  //  Lets try to read the run header
  _inputfile->seekg(0,std::ios::end);
  filesize = _inputfile->tellg();
  _inputfile->seekg(_nextbufferPosition);
  
  int ntriestoread=0;
  
  while((filesize-_nextbufferPosition)<(unsigned int)nbytesToRead)
  {
    
    // Check for end of file record
    const unsigned int tailrecord = 0x0E0F0E0F;
    const int endOfFileBytes = 4;
    
    if((filesize-_nextbufferPosition)==(unsigned int)endOfFileBytes)
    {
      unsigned int eofbuf[1];
      _inputfile->read((char*)eofbuf,(std::streamsize)endOfFileBytes);
      unsigned int eoftest = eofbuf[0];
      if(eoftest == tailrecord)
      {
        LOG<<"Found of end of file record"<<ENDM_INFO;
        //Check for next file
        TString rawFileName = _inputfilename;
        rawFileName+="_";
        rawFileName+=_filecounter+1;
        rawFileName+=".bin";
        if(!OpenRawBinaryFile(rawFileName.Data()))
        {
            //Update filesize
            _filecounter++;
            _inputfile->seekg(0,std::ios::end);
            filesize = _inputfile->tellg();
            _inputfile->seekg(_nextbufferPosition);
            continue;
        }
        
        return 0;
      }else{
        _inputfile->seekg(_nextbufferPosition);
      }
    }
    
    // We have a problem ...
    //   either this buffer is still being written
    //   or this buffer is corrupted.
    //LOG<<" Requested buffer beyond end of file"<<ENDM_FATAL;
    
    //Perhaps we have a subsequent file to open
    
      
    if(!waitOnData)    return -1;
    
    // We could add some code here to wait for either the request
    // to stop data taking or the availibility of more data.
    
    // Lets let the thread sleep for a 1 sec
    // and try again
    
    for(int i = 0; i < 10; i++)
    {
      TThread::Sleep(0,100000000L);
      gSystem->ProcessEvents();
    }
    
    // Recalculate the size of the file
    _inputfile->seekg(0,std::ios::end);
    filesize = _inputfile->tellg();
    _inputfile->seekg(_nextbufferPosition);
    
    if(ntriestoread > 30) return -1;
    
  }// while file size too small for request
  
  // Since number of words seems reasonable lets read this buffer
  _inputfile->seekg(_nextbufferPosition);
  _inputfile->read(((char*)rawbuffer+localOffsetInBytes),(std::streamsize)nbytesToRead);
  Long64_t bytesRead = _inputfile->gcount();
  
  //Save the pointer for the next read
  _nextbufferPosition = _inputfile->tellg();
  
  if(nbytesToRead != bytesRead)
  {
    LOG<<" Not enough bytes read "<<ENDM_FATAL;
  }
  
  return bytesRead;
  
}



Long64_t PIXIE16RawReaderv4::writePacket(int packetlength, unsigned int *data, NGMTimingCal* _outlist, int moduleid)
{
  // Parsing Info
  const UInt_t FINISH_CODE_SHIFT = 			31;
  const UInt_t EVENT_LENGTH_SHIFT = 			17;
  const UInt_t HEADER_LENGTH_SHIFT = 			12;
  const UInt_t CRATE_ID_SHIFT = 				8;
  const UInt_t SLOT_ID_SHIFT = 				4;
  const UInt_t CHANNEL_NUMBER_SHIFT = 			0;
  const UInt_t CFD_FRACTIONAL_TIME_SHIFT = 		16;
  const UInt_t EVENT_TIME_HIGH_SHIFT = 			0;
  const UInt_t PULSE_SHAPE_TYPE_SHIFT = 			30;
  const UInt_t TRACE_LENGTH_SHIFT = 			16;
  const UInt_t EVENT_ENERGY_SHIFT = 			0;
  const UInt_t EVENT_TIME_LOW_SHIFT = 			0;
  const UInt_t PULSE_SHAPE_SHIFT = 			0;
  const UInt_t ADC_DATA1_SHIFT = 				0;
  const UInt_t ADC_DATA2_SHIFT = 				16;
  
  const UInt_t FINISH_CODE_MASK = 			0x80000000;
  const UInt_t EVENT_LENGTH_MASK = 			0x7FFE0000;
  const UInt_t HEADER_LENGTH_MASK = 			0x1F000;
  const UInt_t CRATE_ID_MASK = 				0xF00;
  const UInt_t SLOT_ID_MASK = 				0xF0;
  const UInt_t CHANNEL_NUMBER_MASK = 			0xF;
  const UInt_t CFD_FRACTIONAL_TIME_MASK = 		0xFFFF0000;
  const UInt_t CFD_FRACTIONAL_TIME_MASKv2 = 		0x7FFF0000;
  const UInt_t CFD_SOURCE_MASKv2 =                      0x80000000;
  const UInt_t EVENT_TIME_HIGH_MASK = 			0xFFFF;
  const UInt_t PULSE_SHAPE_TYPE_MASK = 			0xC0000000;
  const UInt_t TRACE_LENGTH_MASK = 			0x3FFF0000;
  const UInt_t EVENT_ENERGY_MASK = 			0x7FFF;
  const UInt_t EVENT_TIME_LOW_MASK = 			0xFFFFFFFF;
  const UInt_t PULSE_SHAPE_MASK = 			0xFFFFFFFF;
  const UInt_t ADC_DATA1_MASK = 			0xFFF;
  const UInt_t ADC_DATA2_MASK = 			0xFFF0000;

	// Nominal event size without raw data
  const unsigned int sumheaderlength = 4;
  const unsigned int nomNumberOfEventWords = 5;
  const unsigned int nanoSecInSec = 1000000000;
  unsigned int actNumberOfEventWords = 4;
  unsigned long long rawtimestamp;
  unsigned long long prevrawtimestamp = 0;
  unsigned short tmoduleid;
  unsigned short channelid;
  unsigned short peakheight;
  unsigned short peakindex;
  unsigned short pileup;
  unsigned short cfd;
  unsigned int gate[8];
  unsigned short numberrawsamples;
  unsigned short headerlength;
  unsigned short eventlength;
  unsigned short blockheaderlength;
  unsigned short blockeventlength;
  unsigned int beginNanoSec;
  unsigned int beginSec;
  unsigned int prevSec = 0;
  unsigned int prevNanoSec = 0;
  char cbuff[1024];
  
  if(!_outlist)
  {
      LOG<<"output stream not allocated for module "<<moduleid<<ENDM_FATAL;
      exit(1);
  }
    
  TTimeStamp* runBegin = &_runBegin;
  
  beginNanoSec = runBegin->GetNanoSec();
  beginSec = runBegin->GetSec();
  if(data == 0 || (unsigned int)packetlength < nomNumberOfEventWords) return -1;
  
  
  unsigned int *nextBuffer = data;
  // Loop over the data event by event,
  // check that next event data is within buffer length
  //LOG<<"Loop over events."<<ENDM_INFO;
  int pulsecounter = 0;
  const int minHeaderLength = 5;
  int dataToSave = 0;
  while((nextBuffer-data)<packetlength)
  {
    
    //Check for incomplete Header
    if( (nextBuffer-data) + minHeaderLength > packetlength)
    {
        dataToSave = packetlength - (nextBuffer-data);
        break;
    }
    
    blockheaderlength = ( nextBuffer[0] & HEADER_LENGTH_MASK ) >> HEADER_LENGTH_SHIFT;
    blockeventlength = ( nextBuffer[0] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT;
    headerlength = ( nextBuffer[4] & HEADER_LENGTH_MASK ) >> HEADER_LENGTH_SHIFT;
    eventlength = ( nextBuffer[4] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT;
    numberrawsamples = (eventlength-headerlength)*2;  
    actNumberOfEventWords = (
                             (( nextBuffer[0] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT)
                             + (4*(( nextBuffer[4] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT))
                             );
    
    if(actNumberOfEventWords>5000)
    {
      LOG<<"Suspiciouly large event size "<<actNumberOfEventWords<<" words"<<ENDM_FATAL;
      return 0;
    }
    char sbuf[1024];
    int partialDataMissing = (nextBuffer-data) + actNumberOfEventWords - packetlength;
    //Check for an incomplete event
    if(partialDataMissing>0){
        dataToSave = actNumberOfEventWords - partialDataMissing;
        break;
    }

    
     

    // sprintf(sbuf," %x %x %d %d %d ",
    // 	    nextBuffer[0],
    // 	    nextBuffer[4],
    // 	    pulsecounter,
    // 	    partialDataMissing,
    // 	    actNumberOfEventWords
    // 	    );
    //LOG<<sbuf<<ENDM_INFO;
    
    tmoduleid = ( nextBuffer[0] & SLOT_ID_MASK ) >> SLOT_ID_SHIFT;
    // encoded in the data is slot number which typically starts at two
    // Really I should use a reverse lookup of the PXISlotMap to map 
    // 2 -> 0, 3 -> 1, etc...
    tmoduleid = tmoduleid - 2;
    
    if(tmoduleid!=moduleid){
        LOG<<"Mismatch in Module Id is Packet Header is "<<moduleid<<" and packet contains "<<tmoduleid <<ENDM_FATAL;
        return -1;
    }

    if(moduleid>=_NanoSecondsPerClock.GetSize()){
      
      LOG<<"Module Id is "<<moduleid<<" and data appears corrupt after "<<pulsecounter <<" events: "<<sbuf<<ENDM_FATAL;
      return -1;
    }
      
    double nanoSecsPerClock = _NanoSecondsPerClock[moduleid];
    channelid = ( nextBuffer[0] & CHANNEL_NUMBER_MASK)  >> CHANNEL_NUMBER_SHIFT;
    
    pulsecounter++;
    
    rawtimestamp = ( nextBuffer[1] & EVENT_TIME_LOW_MASK ) >> EVENT_TIME_LOW_SHIFT;
    rawtimestamp = rawtimestamp | (((unsigned long long)(nextBuffer[2] & EVENT_TIME_HIGH_MASK) >> EVENT_TIME_HIGH_SHIFT)<<32);
    bool goodCFD = true;
    if(_fileversion==0x1001){
      cfd = (nextBuffer[2] & CFD_FRACTIONAL_TIME_MASK) >> CFD_FRACTIONAL_TIME_SHIFT;
    }else{
      cfd = (nextBuffer[2] & CFD_FRACTIONAL_TIME_MASKv2) >> CFD_FRACTIONAL_TIME_SHIFT;
      if( nextBuffer[2] & CFD_SOURCE_MASKv2 ) goodCFD = false;
    }
    peakheight = (nextBuffer[3] & EVENT_ENERGY_MASK) >> EVENT_ENERGY_SHIFT;
    pileup =  ( nextBuffer[0] & FINISH_CODE_MASK ) >> FINISH_CODE_SHIFT;
    if(!goodCFD){
      pileup=pileup|0x2;
    }
    if(nanoSecsPerClock==4.0) rawtimestamp=rawtimestamp<<1;
    
    if(numberrawsamples > 10000)
    {
      LOG<<"Unusually large number of samples "<<numberrawsamples
      <<" after "<< pulsecounter<<" pulses. Bailing."<<ENDM_FATAL;
      return 0;
    }
    prevrawtimestamp = rawtimestamp;
    // Uncomment code below to skip
    // filling root structures
    // //Increment nextBuffer
    // nextBuffer+=actNumberOfEventWords;
    // continue;
    
    NGMHit* tHit = _hitPool->GetHit();
    unsigned long long timeInPicoSeconds = (rawtimestamp*int(nanoSecsPerClock*1000));
    unsigned long long timeInNanoSeconds = timeInPicoSeconds/1000;
    unsigned long long timeInSeconds = timeInNanoSeconds/nanoSecInSec;
    tHit->SetRawTime(
                     NGMTimeStamp( (time_t)(beginSec + timeInSeconds),
                                  beginNanoSec + (timeInNanoSeconds - timeInSeconds*nanoSecInSec)
                                  ,timeInPicoSeconds - timeInNanoSeconds*1000)
                     );
    
    tHit->SetSlot(moduleid);
    tHit->SetChannel(channelid);
    tHit->SetPulseHeight(peakheight);
    tHit->SetPileUpCounter(pileup);
    tHit->SetRawClock(rawtimestamp);
    if(_fileversion==0x1001){
      tHit->SetCFD(cfd/65536.0);
    }else{
      tHit->SetCFD(cfd/32768.0);
    }
    tHit->GetNGMTime().IncrementNs(tHit->GetCFD()*nanoSecsPerClock);
    
    //LOG<<"Header Length "<<headerlength<<" Event Length "<<eventlength<<" Raw Samples "<<numberrawsamples<<ENDM_INFO;
    if(headerlength==11)
    {
      tHit->SetGate(4*8-1,0);
      for(int ichan = 0; ichan<4; ichan++)
      {
        for(int igate = 0; igate<8;igate++)
        {
          int offset = sumheaderlength + eventlength*ichan + 3 + igate;
          //LOG<<"ichan("<<ichan<<") igate("<<igate<<") offset("<<offset<<") gate("<<nextBuffer[offset]<<ENDM_INFO;
          tHit->SetGate(8*ichan + igate, nextBuffer[offset]);
        }
      }
    }else{
      //tHit->SetNGates(0);
    }
    //  LOG<<tHit->IsA()->GetName()<<" Gate(0) "<<tHit->GetGate(0)<<" Gate(8): "<<tHit->GetGate(8)
    //     <<"Gate(16) "<<tHit->GetGate(16)<<" Gate(24): "<<tHit->GetGate(24)<<ENDM_INFO;
    bool sumPMTWaveforms = false;
    if(numberrawsamples > 0 && sumPMTWaveforms){
      // Copy the waveform
      tHit->SetNSamples(numberrawsamples);
      int sample1 = sumheaderlength + headerlength;
      int sample2 = sumheaderlength + headerlength + eventlength;
      int sample3 = sumheaderlength + headerlength + 2*eventlength;
      int sample4 = sumheaderlength + headerlength + 3*eventlength;
      
      for(int isample =0; isample < numberrawsamples; isample++){
        unsigned int tmpsample = ((isample%2)?
                                  (nextBuffer[sample1+isample/2])>>16
                                  :(nextBuffer[sample1+isample/2] & 0xFFFF))
        + ((isample%2)?
           (nextBuffer[sample2+isample/2])>>16
           :(nextBuffer[sample2+isample/2] & 0xFFFF))
        + ((isample%2)?
           (nextBuffer[sample3+isample/2])>>16
           :(nextBuffer[sample3+isample/2] & 0xFFFF))
        + ((isample%2)?
           (nextBuffer[sample4+isample/2])>>16
           :(nextBuffer[sample4+isample/2] & 0xFFFF));
        tHit->SetSample(isample, tmpsample);
      }
    }else if(numberrawsamples > 0 && !sumPMTWaveforms){
        // Copy the waveform
        tHit->SetNSamples(numberrawsamples*4);
        int sample1 = sumheaderlength + headerlength;
        int sample2 = sumheaderlength + headerlength + eventlength;
        int sample3 = sumheaderlength + headerlength + 2*eventlength;
        int sample4 = sumheaderlength + headerlength + 3*eventlength;
        
        for(int isample =0; isample < numberrawsamples; isample++){

            tHit->SetSample(isample,
                            ((isample%2)?
                             (nextBuffer[sample1+isample/2])>>16
                             :(nextBuffer[sample1+isample/2] & 0xFFFF)));
            
            tHit->SetSample(isample+numberrawsamples,
                            ((isample%2)?
                             (nextBuffer[sample2+isample/2])>>16
                             :(nextBuffer[sample2+isample/2] & 0xFFFF)));
            
            tHit->SetSample(isample+2*numberrawsamples,
                            ((isample%2)?
                             (nextBuffer[sample3+isample/2])>>16
                             :(nextBuffer[sample3+isample/2] & 0xFFFF)));
            tHit->SetSample(isample+3*numberrawsamples,
                            ((isample%2)?
                             (nextBuffer[sample4+isample/2])>>16
                             :(nextBuffer[sample4+isample/2] & 0xFFFF)));
            
         }
    }else{
      //    LOG<<"No Waveform "<<ENDM_FATAL;
      //exit(-1);
      tHit->SetNSamples(0);
    }
    
    prevSec = tHit->GetTimeStamp().GetSec();
    prevNanoSec = tHit->GetTimeStamp().GetNanoSec();
    if(prevrawtimestamp>rawtimestamp)
    {
      LOG<<"TimeStamps out of order: Slot("<<(moduleid*16+channelid)
      <<") "<<(long long)prevrawtimestamp<<" : "<< (long long)rawtimestamp <<ENDM_WARN;
    }
    
    prevrawtimestamp = rawtimestamp;
    
    _outlist->pushHit(tHit);
      
    //Increment nextBuffer
    nextBuffer+=actNumberOfEventWords;
  }
  _rawbufferstart[moduleid]=dataToSave;
  if(dataToSave>0)
  {
      //LOG<<" Data remaining for module "<<moduleid<< " is "<<dataToSave<<" bytes."<<ENDM_INFO;
      memcpy(_rawbuffer[moduleid],nextBuffer,dataToSave*4);
  }
  _totalEventsThisRun+=pulsecounter;
  
  if(pulsecounter>2800)
  {
    LOG<<"Module "<<moduleid<<" Buffer exceeds 2800 "<<pulsecounter<<ENDM_WARN;
  }
  
  return 0;
}
