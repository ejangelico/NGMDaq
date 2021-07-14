//
//  NGMPIXIE16RawReaderv2.cpp
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/15/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMPIXIE16RawReaderv2.h"
#include "NGMLogger.h"
#include "NGMHit.h"
#include "NGMBufferedPacket.h"
#include "NGMSystemConfiguration.h"
#include "NGMConfigurationTable.h"
#include "TThread.h"
#include "TFile.h"
#include "TSystem.h"
#include "NGMModule.h"
#include "TSystem.h"
#include "NGMBufferedPacket.h"
#include <stdlib.h>

////// NGMPIXIE16RawReaderv2 Implementation
ClassImp(NGMPIXIE16RawReaderv2)

NGMPIXIE16RawReaderv2::NGMPIXIE16RawReaderv2()
{
  _rawbuffer = 0;
  _outBuffer = 0; // The packet buffer filled when parsing raw file
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
  
}

NGMPIXIE16RawReaderv2::NGMPIXIE16RawReaderv2(NGMModule* parent)
: NGMReaderBase(parent)
{
  _rawbuffer = new unsigned int[2000000];
  _outBuffer = new NGMBufferedPacketv2(0,0,TTimeStamp((time_t)0,0),6);
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
  _filecounter = 0;
  
  
}

NGMPIXIE16RawReaderv2::~NGMPIXIE16RawReaderv2()
{
  if(_inputfile)
  {
    if(_inputfile->is_open())
      _inputfile->close();
    delete _inputfile;
  }
  if(_rawbuffer)
    delete [] _rawbuffer;
  if(_outBuffer)
    delete _outBuffer; // The packet buffer filled when parsing raw file  
}

void NGMPIXIE16RawReaderv2::SetConfiguration(const NGMSystemConfiguration* sysConf)
{
  // Let Base keep a copy of configuration
  NGMReaderBase::SetConfiguration(sysConf);
  
  // Extract Clock Settings for each slot
  NGMConfigurationTable* slotPars = GetConfiguration()->GetSlotParameters();
  
  _NanoSecondsPerClock.Set(slotPars->GetEntries());
  for(int islot = 0; islot < slotPars->GetEntries(); islot++)
  {
    _NanoSecondsPerClock[islot] = 10.0; //Hard coded for now
  }
  
  // Initialize other beginning of run variables
  _runBegin = _config->GetTimeStamp();
  _firstTimeOfRun = TTimeStamp(0,0);
  _lastPlotIndex = -1;
  _runduration = 0;
  _livetime = 0;
  _totalEventsThisRun = 0;
  
}
//
//NGMSystemConfiguration* NGMPIXIE16RawReaderv2::CreateConfigurationFromPIXIE()
//{
//  
//}


Long64_t NGMPIXIE16RawReaderv2::OpenInputFile(const char* filename)
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

Long64_t NGMPIXIE16RawReaderv2::OpenRawBinaryFile(const char* pathname)
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
  Long64_t result = readBytesWaiting(bytesInRunHeader);
  if(result!=bytesInRunHeader)
  {
    LOG<<" Full Header not found in buffer "<<pathname<<ENDM_INFO;
    return 1;
  }
  
  LOG<<"Found SIS Run Header of version "<<_rawbuffer[0] << ENDM_INFO;

  
  return 0;
}

Long64_t NGMPIXIE16RawReaderv2::ReadAll()
{
  int nwordsinbuffer = 0;
  int i = 0;
  
  //Read Header
  
  
  while( ! _abortread )
  {
    
    nwordsinbuffer = ReadNextBufferFromFile();
    if(nwordsinbuffer == 0) break;
    
    i++;
    if(i%1 == 0) gSystem->ProcessEvents();
  }
  // Flush any last events
  if(_outBuffer->getPulseCount()>0)
  {
    _outBuffer->sortHits();
    AnaStats(_outBuffer);
    GetParent()->push(*((TObject*)_outBuffer));
    _outBuffer->Clear();
  }
  
  return 0;
  
}

Long64_t NGMPIXIE16RawReaderv2::CloseInputFile()
{
  if(_inputfile)
  {
    if(_inputfile->is_open())
      _inputfile->close();
    delete _inputfile;
    _inputfile = 0;
  }
  if(_outBuffer)
  {
    delete _outBuffer; // The packet buffer filled when parsing raw file  
    _outBuffer = 0;
  }
  return 0;
}

Long64_t NGMPIXIE16RawReaderv2::ReadNextBufferFromFile()
{
  return ReadNextBufferFromFilev2();
}

Long64_t NGMPIXIE16RawReaderv2::ReadNextBufferFromFilev2()
{
  const UInt_t EVENT_LENGTH_SHIFT = 			17;
  const UInt_t EVENT_LENGTH_MASK = 			0x7FFE0000;
  
  //We begin by reading the header for a packet
  const int nbytesPacketHeader = 10*4;
  // This is a little different from v1 in that we expect a sequence of 1 summed header
  // and 4 individual channel computations.
  
  // A 10 word packet header contains module id and packet length
  
  int bytesRead = readBytesWaiting(nbytesPacketHeader,false);
  if(nbytesPacketHeader!=bytesRead)
  { 
    if(bytesRead == 0){
      return 0;
    }else{
      LOG<<"ERROR reading packet header"<<ENDM_FATAL;
      return 0;
    }
  }
  
  int moduleid = _rawbuffer[0];
  int packetLengthInWords = _rawbuffer[1];
  
  if (packetLengthInWords > 50000000) {
    LOG<<" Suspiciously large packet size: "<<packetLengthInWords<<""<<ENDM_FATAL;
    exit(-1);
  }
  
  
  gSystem->ProcessEvents();
  
  if(readBytesWaiting(packetLengthInWords*4,false)>0)
  {
    Long64_t status = writePacket(packetLengthInWords, _rawbuffer, _outBuffer);
    if(status) return 0;
    if(_outBuffer->getPulseCount()>1000000)
    {
      _outBuffer->sortHits();
      AnaStats(_outBuffer);
      GetParent()->push(*((TObject*)_outBuffer));
      _outBuffer->Clear();
    }
  }
  else 
    return 0;
  return 1;
}


int NGMPIXIE16RawReaderv2::AnaStats(NGMBufferedPacket* packet)
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

Long64_t NGMPIXIE16RawReaderv2::readBytesWaiting(Long64_t nbytesToRead, bool waitOnData, size_t localOffsetInBytes)
{
  // If the end of file we will return 0
  // If expected bytes not found return -1
  // Otherwise return number of bytes read...
  
  if(!_inputfile->is_open())
  {
    LOG<<" No file appears to be open for input "<<ENDM_WARN;
    return 0;
  }
  
  if(!_rawbuffer)
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
      _inputfile->read((char*)_rawbuffer,(std::streamsize)endOfFileBytes);
      unsigned int eoftest = _rawbuffer[0];
      if(eoftest == tailrecord)
      {
        return 0;
        LOG<<"Found of end of file record"<<ENDM_INFO;
      }else{
        _inputfile->seekg(_nextbufferPosition);
      }
    }
    
    // We have a problem ...
    //   either this buffer is still being written
    //   or this buffer is corrupted.
    //LOG<<" Requested buffer beyond end of file"<<ENDM_FATAL;
    
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
  _inputfile->read(((char*)_rawbuffer+localOffsetInBytes),(std::streamsize)nbytesToRead);
  Long64_t bytesRead = _inputfile->gcount();
  
  //Save the pointer for the next read
  _nextbufferPosition = _inputfile->tellg();
  
  if(nbytesToRead != bytesRead)
  {
    LOG<<" Not enough bytes read "<<ENDM_FATAL;
  }
  
  return bytesRead;
  
}



Long64_t NGMPIXIE16RawReaderv2::writePacket(int packetlength, unsigned int *data, NGMBufferedPacket* &_outBuffer)
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
  const UInt_t EVENT_TIME_HIGH_MASK = 			0xFFFF;
  const UInt_t PULSE_SHAPE_TYPE_MASK = 			0xC0000000;
  const UInt_t TRACE_LENGTH_MASK = 			0x3FFF0000;
  const UInt_t EVENT_ENERGY_MASK = 			0x7FFF;
  const UInt_t EVENT_TIME_LOW_MASK = 			0xFFFFFFFF;
  const UInt_t PULSE_SHAPE_MASK = 			0xFFFFFFFF;
  const UInt_t ADC_DATA1_MASK = 				0xFFF;
  const UInt_t ADC_DATA2_MASK = 				0xFFF0000;
  
	// Nominal event size without raw data
  const unsigned int sumheaderlength = 4;
  const unsigned int nomNumberOfEventWords = 5;
  const unsigned int nanoSecInSec = 1000000000;
  unsigned int actNumberOfEventWords = 4;
  unsigned long long rawtimestamp;
  unsigned long long prevrawtimestamp = 0;
  unsigned short moduleid;
  unsigned short channelid;
  unsigned short peakheight;
  unsigned short peakindex;
  unsigned short pileup;
  unsigned short cfd;
  unsigned int gate[8];
  unsigned short numberrawsamples;
  unsigned short headerlength;
  unsigned short eventlength;
  unsigned int beginNanoSec;
  unsigned int beginSec;
  unsigned int prevSec = 0;
  unsigned int prevNanoSec = 0;
  char cbuff[1024];
  
  if(!_outBuffer)
  {
    _outBuffer = new NGMBufferedPacketv2(0,0,TTimeStamp((time_t)0,0),6);
  }
  
  TTimeStamp* runBegin = &_runBegin;
  NGMBufferedPacket* packetBuffer = _outBuffer;
  
  packetBuffer->Clear();
  
  beginNanoSec = runBegin->GetNanoSec();
  beginSec = runBegin->GetSec();
  if(data == 0 || (unsigned int)packetlength < nomNumberOfEventWords) return -1;
  
  
  unsigned int *nextBuffer = data;
  // Loop over the data event by event,
  // check that next event data is within buffer length
  //LOG<<"Loop over events."<<ENDM_INFO;
  int pulsecounter = 0;
  
  while((nextBuffer-data)<packetlength)
  {
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
    int partialDataMissing = (((nextBuffer+actNumberOfEventWords)-data) - packetlength);
    int dataToSave = actNumberOfEventWords - partialDataMissing;
    

    // sprintf(sbuf," %x %x %d %d %d ",
    // 	    nextBuffer[0],
    // 	    nextBuffer[4],
    // 	    pulsecounter,
    // 	    partialDataMissing,
    // 	    actNumberOfEventWords
    // 	    );
    //LOG<<sbuf<<ENDM_INFO;
    
    moduleid = ( nextBuffer[0] & SLOT_ID_MASK ) >> SLOT_ID_SHIFT;
    // encoded in the data is slot number which typically starts at two
    // Really I should use a reverse lookup of the PXISlotMap to map 
    // 2 -> 0, 3 -> 1, etc...
    moduleid = moduleid - 2;
    
    if(moduleid>=_NanoSecondsPerClock.GetSize()){
      
      LOG<<"Module Id is "<<moduleid<<" and data appears corrupt after "<<pulsecounter <<" events: "<<sbuf<<ENDM_FATAL;
      return -1;
    }
    double nanoSecsPerClock = _NanoSecondsPerClock[moduleid];
    channelid = ( nextBuffer[0] & CHANNEL_NUMBER_MASK)  >> CHANNEL_NUMBER_SHIFT;
    
    pulsecounter++;
    
    rawtimestamp = ( nextBuffer[1] & EVENT_TIME_LOW_MASK ) >> EVENT_TIME_LOW_SHIFT;
    rawtimestamp = rawtimestamp | (((unsigned long long)(nextBuffer[2] & EVENT_TIME_HIGH_MASK) >> EVENT_TIME_HIGH_SHIFT)<<32);
    cfd = (nextBuffer[2] & CFD_FRACTIONAL_TIME_MASK) >> CFD_FRACTIONAL_TIME_SHIFT;
    peakheight = (nextBuffer[3] & EVENT_ENERGY_MASK) >> EVENT_ENERGY_SHIFT;
    pileup =  ( nextBuffer[0] & FINISH_CODE_MASK ) >> FINISH_CODE_SHIFT;
    
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
    
    NGMHit* tHit = packetBuffer->addHit();
    unsigned long long timeInPicoSeconds = (rawtimestamp*int(nanoSecsPerClock*1000));
    unsigned long long timeInNanoSeconds = timeInPicoSeconds/1000;
    unsigned long long timeInSeconds = timeInNanoSeconds/nanoSecInSec;
    tHit->SetRawTime(
                     NGMTimeStamp( (time_t)(beginSec + timeInSeconds),
                                  beginNanoSec + (timeInNanoSeconds - timeInSeconds*nanoSecInSec)
                                  ,timeInPicoSeconds - timeInNanoSeconds*1000)
                     );
    
    if(pulsecounter==1){
      packetBuffer->setSlotId(moduleid);
      packetBuffer->setChannelId(channelid);
      packetBuffer->setTimeStamp(tHit->GetRawTime());
    }
    
    tHit->SetSlot(moduleid);
    tHit->SetChannel(channelid);
    tHit->SetPulseHeight(peakheight);
    tHit->SetPileUpCounter(pileup);
    tHit->SetRawClock(rawtimestamp);
    tHit->SetCFD(cfd/65536.0);
    
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
    if(numberrawsamples > 0){
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
    
    //Increment nextBuffer
    nextBuffer+=actNumberOfEventWords;
  }
  
  _totalEventsThisRun+=pulsecounter;
  
  if(pulsecounter>2700)
  {
    LOG<<"Module "<<moduleid<<" Buffer exceeds 2700 "<<ENDM_WARN;
  }
  
  GetParent()->push(*((TObject*)packetBuffer));
  
  return 0;
}
