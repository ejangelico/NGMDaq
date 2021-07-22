/*
 *  NGMReaderBase.cpp
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/3/08.
 *  Copyright 2008 LLNL. All rights reserved.
 *
 */
#include <stdint.h>
#include "NGMReaderBase.h"
#include "Riostream.h"
#include "TSystem.h"
#include "TThread.h"
#include <fstream>
#include "NGMLogger.h"
#include "NGMBufferedPacket.h"
#include "NGMHit.h"
#include "NGMSystemConfiguration.h"
#include "NGMConfigurationTable.h"
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "NGMModule.h"
#include "TObjString.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "TROOT.h"
#include "NGMSystem.h"
#include <cstdlib>
#include "TDirectory.h"
#include "NGMPIXIE16RawReaderv2.h"
#include "PIXIE16RawReaderv3.h"
#include "PIXIE16RawReaderv4.h"
#include "NGMSIS3316RawReader.h"
#include "NGMSIS3302RawReader.h"

#define MAXALLOWEDSAMPLES 10000
#define MAXBUFFERLENGTH 20000000

////// NGMReaderBase Implementation

ClassImp(NGMReaderBase)

NGMReaderBase::NGMReaderBase()
{ 
  _parent = 0;
  _config = 0;
  _pause = false;
  _abortread = false;
  _seriesCount = 0;
  _bytesRead = 0;
  _maxruntime = -1.0;
  _tcal = 0;

}

NGMReaderBase::NGMReaderBase(NGMModule* parent)
{
  _parent = parent;
  _config = 0;
  _pause = false;
  _abortread = false;
  _seriesCount = 0;
  _bytesRead = 0;
  _maxruntime = -1.0;
  _tcal = 0;
}

NGMReaderBase::~NGMReaderBase()
{
 if(_config)
   delete _config;
}

void NGMReaderBase::SetConfiguration(const NGMSystemConfiguration* sysConf)
{
    delete _config;
    _config = sysConf->DuplicateConfiguration();
  
    if(_parent){
        _parent->SetRunNumber(_config->getRunNumber());
      
      // If non-default passname is set, lets add the passname to the SystemConfiguration Object
      if(GetParent()->InheritsFrom(gROOT->GetClass("NGMSystem")))
      {
        NGMSystem* pSystem = dynamic_cast<NGMSystem*>(GetParent());
        if(pSystem->GetPassName()!="")
        {
          if(_config->GetSystemParameters()->GetParIndex("PassName")<0)
          {
            _config->GetSystemParameters()->AddParameterS("PassName", "");
          }
          _config->GetSystemParameters()->SetParameterS("PassName", 0, pSystem->GetPassName().Data());
        }
      }
  
    }
  _bytesRead = 0;

  _abortread = false;
}

void NGMReaderBase::SendEndRun()
{
  // Send end of analysis signals to all modules.
  TObjString endRunFlush("EndRunFlush");
  TObjString endRunSave("EndRunSave");
  GetParent()->push(*((const TObject*)&endRunFlush));
  GetParent()->push(*((const TObject*)&endRunSave));
  _seriesCount = 0;
}

void NGMReaderBase::SendPlotUpdate()
{
  // Send plot request signal to all modules.
  TObjString plotUpdate("PlotUpdate");
  GetParent()->push(*((const TObject*)&plotUpdate));
}

NGMReaderBase* NGMReaderBase::Factory(const char* readerType, NGMModule* parent)
{
  TString rstr(readerType);
  if(rstr=="SISRaw")
  {
    return new NGMSISRawReader(parent);
  }else if(rstr=="SIS3305"){
    return new NGMSIS3305RawReader(parent);
  }else if(rstr=="GAGERaw"){
    return new NGMGageRawReader(parent);
  }else if(rstr=="PIXIE16ORNL"){
    return new NGMPIXIE16ORNLRawReader(parent);
  }else if(rstr=="PIXIE16Rawv4"){
    return new PIXIE16RawReaderv4(parent);
  }else if(rstr=="SIS3316Raw"){
    return new NGMSIS3316RawReader(parent);
  }else if(rstr=="SIS3302Raw"){
    return new NGMSIS3302RawReader(parent);
  }
  
  
  // Default to Root Reader
  return new NGMRootReader(parent);
}


void NGMReaderBase::ReadRunSeries(const char* inputrunlist)
{
  
}

void NGMReaderBase::ReadNextInSeries(const char* inputfilepath)
{
  OpenInputFile(inputfilepath);
  if(_seriesCount==0) _parent->push(*((const TObject*) _config));
  ReadAll();
  _seriesCount++;
}

////// NGMRootReader Implementation
ClassImp(NGMRootReader)

NGMRootReader::NGMRootReader()
{
  _inputFile = 0;
  _inputBuffer = 0;
  _localBuffer = 0;
  _localHit = 0;
  _numberOfSlots = 16;
  _numberOfHWChannels = 16*8;
  _channelsperSlot = _numberOfHWChannels/_numberOfSlots;
  _prevBufferIndex = -1;
  _spillCounter = 0;
  _readingHits = false;
  _readUnsortedPackets = true;
}  

NGMRootReader::NGMRootReader(NGMModule* parent)
: NGMReaderBase(parent)
{
  _inputFile = 0;
  _inputBuffer = 0;
  _localBuffer = 0;
  _localHit = 0;
  _numberOfSlots = 16;
  _numberOfHWChannels = 16*8;
  _channelsperSlot = _numberOfHWChannels/_numberOfSlots;
  _prevBufferIndex = -1;
  _spillCounter = 0;
  _readingHits = false;
  _readUnsortedPackets = true;

}

NGMRootReader::~NGMRootReader()
{
  // //Clean up our own heap allocated objects
  // if(_localBuffer){
  //   delete _localBuffer;
  //   _localBuffer = 0;
  // }
  // //When closing the input file
  // // the associated input buffer is automatically deleted
  // if(_inputFile){
  //   _inputFile->Close();
  //   _inputFile = 0;
  //   _localBuffer = 0;
  // }
  
}

Long64_t NGMRootReader::OpenInputFile(const char* inputfilepath){

  // Close a previously open file;
  CloseInputFile();
  
  LOG<<"Attempting to open "<<inputfilepath<<ENDM_INFO;
  
  _inputFile = TFile::Open(inputfilepath);
  if(!_inputFile){
    LOG<<" Input file not found "<<inputfilepath<<ENDM_WARN;
    return -1;
  }
  
  if(_inputFile->IsZombie())
  {
    delete _inputFile;
    TThread::Sleep(1);
    _inputFile = TFile::Open(inputfilepath);
  }
  ;
  TString oldFileName = inputfilepath;
  // Find and strip "underscore filecount" from previous name
  int stripLoc = oldFileName.Index("_",1,oldFileName.Length()-10,TString::kExact);
  int fileCounter = 0;
  // If no _ in last 10 chars of filename this must be the first file segment
  if(stripLoc < 0)
  {
    stripLoc = oldFileName.Length() - 5;
    fileCounter = 0;
  }else{
    fileCounter = TString(oldFileName(stripLoc+1,oldFileName.Length()-5-(stripLoc+1))).Atoi();
  }

  
  NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*) (_inputFile->Get("NGMSystemConfiguration"));
  if(!sysConf&&fileCounter==0)
  {
    LOG<<"No configuration object available. Try once more in 10 seconds."<<ENDM_WARN;
    TThread::Sleep(10);
    _inputFile->ReadKeys();
    sysConf = (NGMSystemConfiguration*) (_inputFile->Get("NGMSystemConfiguration"));
    if(!sysConf)
    {
      LOG<<"Still No configuration object available... bailing."<<ENDM_WARN;
      return -1;
    }
  }
  
  // _localBuffer is for packet data only
  delete _localBuffer;
  _localBuffer = 0;

  // _localHit is for hit data only
  delete _localHit;
  _localHit = 0;
  _inputBuffer = 0;
  while(_inputBuffer==0)
  {
    // If we are reading packet buffers...
    if( (_inputBuffer = (TTree*) (_inputFile->Get("pulse")))){
      _inputBuffer->SetBranchAddress("pulsebuffer",&_localBuffer);
      _readingHits = false;
      
      // Are we reading raw packet data (slotid>=0) or buffered merge sorted hits (slotid ==-1)
      _inputBuffer->GetEntry(0);
      if(_localBuffer->getSlotId()<0)
      {
        // We are reading buffered hits ...
        _readUnsortedPackets = false;
      }else{
        _readUnsortedPackets = true;
      }
    }else if((_inputBuffer = (TTree*) (_inputFile->Get("HitTree")))){    
      _inputBuffer->SetBranchAddress("HitTree",&_localHit);
      _readingHits = true;
    }else{
      LOG<<"Waiting for tree to be written "<<ENDM_INFO;
      TThread::Sleep(10000);
      _inputFile->ReadKeys();
    }
  }
  
  if(sysConf){
    
    //Lets look for a file of the same name except ".root" -> "-cal.root"
    TString dirName(gSystem->DirName(inputfilepath));
    TString fname(gSystem->BaseName(inputfilepath));
    fname.ReplaceAll(".root","-cal.root");
    TString caldir(gSystem->Getenv("NGMCALDIR"));
    if(caldir=="") caldir =dirName;
    TString calName = caldir+"/"+fname;
    if(!gSystem->AccessPathName(calName))
    {
      LOG<<"Using configuration object from "<<calName.Data()<<ENDM_INFO;
      TFile* calfin = TFile::Open(calName);
      sysConf = (NGMSystemConfiguration*) (calfin->Get("NGMSystemConfiguration"));
    }
    
    // Correct the case where NGMSystemConfiguration Names were written as
    // NGMSystemConfiguration
    TString confName(sysConf->GetName());
    if(confName == "NGMSystemConfiguration")
    {
      sysConf->SetName("SIS3320");
      sysConf->SetTitle("SIS3320");
    }
    if(TString(sysConf->GetTitle())=="SIS3150")
    {
      sysConf->SetName("SIS3320");
      sysConf->SetTitle("SIS3320");
    }else{
      sysConf->SetTitle(sysConf->GetName());
    }
    
    LOG<<" Found Configuration "<<sysConf->GetName()<<"("<<sysConf->GetTitle()<<")"<<ENDM_INFO;
    
    SetConfiguration(sysConf);
  }else{
    LOG<<" Configuration Object Not found "<<ENDM_WARN; 
  }
  _currEventNumber = 0;

  return 0;
}

void NGMRootReader::SetConfiguration(const NGMSystemConfiguration* sysConf)
{
  // Let Base keep a copy of configuration
  NGMReaderBase::SetConfiguration(sysConf);
  
  _numberOfSlots = GetConfiguration()->GetSlotParameters()->GetEntries();
  _numberOfHWChannels = GetConfiguration()->GetChannelParameters()->GetEntries();
  _channelsperSlot = _numberOfHWChannels/_numberOfSlots;
  _prevBufferIndex = -1;

}

bool NGMRootReader::FindNextFile()
{
  // In case root automatically created subsequent file lets check for it.
  
  if(!_inputBuffer) return false;
  
  TFile* inputFile = _inputBuffer->GetCurrentFile();
  TString oldFileName = inputFile->GetName();
  TString newFileName;
  
  // Find and strip "underscore filecount" from previous name
  int stripLoc = oldFileName.Index("_",1,oldFileName.Length()-10,TString::kExact);
  int fileCounter = 0;
  // If no _ in last 10 chars of filename this must be the first file segment
  if(stripLoc < 0)
  {
    stripLoc = oldFileName.Length() - 5;
    fileCounter = 0;
  }else{
    fileCounter = TString(oldFileName(stripLoc+1,oldFileName.Length()-5-(stripLoc+1))).Atoi();
  }
  fileCounter++;
  newFileName= oldFileName(0,stripLoc);
  
  newFileName+="_";
  newFileName+=fileCounter;
  newFileName+=".root";
  
  // Lets see if the file exists
  if(gSystem->AccessPathName(newFileName.Data())==0) // Bizarre convention
  {
    OpenInputFile(newFileName.Data());
    return true;
  }
  return false;
}

Long64_t NGMRootReader::ReadAll()
{
  if(_readingHits)
  {
    return ReadAllHits();
  }else{
    return ReadAllBufferedPackets();
  }
  
}

Long64_t NGMRootReader::ReadAllHits()
{
  if(!_inputBuffer) return 0;
 
  Long64_t bytesRead = 0;
  do{
    bytesRead = _inputBuffer->GetEvent(_currEventNumber++);
    
    //Lets check for a subsequent file
    if(bytesRead == 0)
      if(FindNextFile())
      {
        bytesRead= _inputBuffer->GetEvent(_currEventNumber++);
      }
    //check that the event was found and read
    if(bytesRead == 0) return _bytesRead;
    
    _bytesRead+=bytesRead;
    
    GetParent()->push(*((const TObject*)_localHit));
  }while(!_abortread);

  return _bytesRead;

}

Long64_t NGMRootReader::ReadAllBufferedPackets()
{
  // verify the buffer
  if(!_inputBuffer) return 0;
  // attemp read from input buffer
  Long64_t bufferIndex = 0;
  Long64_t bytesRead = 0;
  TStopwatch spillTimer;
  Long64_t eventsThisSpill = 0;
  TStopwatch srvStopWatch;
  srvStopWatch.Start(true);
  _timeSinceLastProcessSrv = 0.0;
  spillTimer.Start();
  do {

    Long64_t thisBytesRead= 0;
    //_inputBuffer->GetEvent(_currEventNumber++);
    
    if( _currEventNumber>=_inputBuffer->GetEntries()){
      
      //Lets see if it might be an open file
      _inputBuffer->GetCurrentFile()->ReadKeys();
      if(!_inputBuffer->GetCurrentFile()->FindObjectAny("FileClosed"))
      {
        int count = 0;
        do{
          // This means we should wait for an update to the file...
          TThread::Sleep(5,0);
          LOG<<"Waiting for more data to be written to open file attempt "<<count+1
	     <<" tree entries "<<_inputBuffer->GetEntries()
	     <<" current index "<<_currEventNumber-1<<ENDM_INFO;
          _inputBuffer->GetCurrentFile()->ReadKeys();
          _inputBuffer->Refresh();
          if(_currEventNumber<=_inputBuffer->GetEntries())
          {
            thisBytesRead= _inputBuffer->GetEvent(_currEventNumber++);            
            if(thisBytesRead)
	      break;
          }
	  if(_inputBuffer->GetCurrentFile()->FindObjectAny("FileClosed"))
	    break;
        }while(count++<20);
      }
      
      LOG<<"Read first event after waiting..."<<ENDM_INFO;

      //Lets check for a subsequent file
      if(thisBytesRead == 0 && FindNextFile())
      {
        thisBytesRead= _inputBuffer->GetEvent(_currEventNumber++);
      }
    }else{
      thisBytesRead= _inputBuffer->GetEvent(_currEventNumber++);
    }

    //check that the event was found and read
    if(thisBytesRead == 0) return bytesRead;
    
    bytesRead+=thisBytesRead;
    
    // pass the data to daughter tasks
    if(_localBuffer->getChannelId() >=0 ){
      bufferIndex	= _localBuffer->getSlotId()*_channelsperSlot + _localBuffer->getChannelId() ;
    }else if(_localBuffer->getPulseCount()){
      bufferIndex = _localBuffer->getHit(0)->GetSlot()*_channelsperSlot + _localBuffer->getHit(0)->GetChannel();
    }
  
    if(!_readUnsortedPackets && _abortread) break;
    
    if(_readUnsortedPackets && bufferIndex <= _prevBufferIndex && _prevBufferIndex >=0)
    {
      _spillCounter++;
	    _prevBufferIndex = -1;
      
      //LOG<<"Spill Counter:"<<_spillCounter<<ENDM_INFO;
      
      TObjString endSpillFlush("EndSpillFlush");
      GetParent()->push(*((const TObject*)&endSpillFlush));
      spillTimer.Stop();
      if(0)
      {
        LOG<<"Events: "<<eventsThisSpill<<" Bytes: "<<bytesRead
        <<" CPUTime: "<<spillTimer.CpuTime()<<" RealTime: "<<spillTimer.RealTime()
        <<" ("<<bytesRead/spillTimer.RealTime()<<") "
        <<ENDM_INFO;
        
      }
      eventsThisSpill = 0;
      if(_abortread) break;
      spillTimer.Start(true);
    }
    
    eventsThisSpill+=_localBuffer->getPulseCount();
    _bytesRead+=thisBytesRead;
    
    
    // pass the data to daughter tasks
    GetParent()->push(*((const TObject*) _localBuffer));
    double tmpSeconds = srvStopWatch.RealTime();
    if( tmpSeconds > 1)
    {
      //LOG<<" Checking network service "<<ENDM_INFO;
      if(gDirectory!=gROOT)  gROOT->cd();
      ((NGMSystem*)GetParent())->ProcessSpyServ();
      srvStopWatch.Start(true);
    }else{
      srvStopWatch.Start(false);
    }
    _prevBufferIndex = bufferIndex;
  }while(1);
  
  return _bytesRead;
}

/// \brief Close Input File Stream
Long64_t NGMRootReader::CloseInputFile()
{
  // If previous file open delete it!
  if(_inputFile){
    _inputFile->ls();
    delete _inputFile;
    _inputFile = 0;
    // The corresponding tree is no longer available so lets zero the dangling pointer
    _inputBuffer = 0;
  }
  
  // _localBuffer is for packet data only
  delete _localBuffer;
  _localBuffer = 0;
  
  // _localHit is for hit data only
  delete _localHit;
  _localHit = 0;
  
  return 0;
}


////// NGMSISRawReader Implementation
ClassImp(NGMSISRawReader)

NGMSISRawReader::NGMSISRawReader()
{
  _rawbuffer = 0;
  _outBuffer = 0; // The packet buffer filled when parsing raw file
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
}

NGMSISRawReader::NGMSISRawReader(NGMModule* parent)
: NGMReaderBase(parent)
{
  _rawbuffer = new unsigned int[MAXBUFFERLENGTH];
  _outBuffer = new NGMBufferedPacketv2(0,0,TTimeStamp((time_t)0,0),6);
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
  _filecounter = 0;

}

NGMSISRawReader::~NGMSISRawReader()
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

void NGMSISRawReader::SetConfiguration(const NGMSystemConfiguration* sysConf)
{
  // Let Base keep a copy of configuration
  NGMReaderBase::SetConfiguration(sysConf);
  
  // Extract Clock Settings for each slot
  NGMConfigurationTable* slotPars = GetConfiguration()->GetSlotParameters();
  
  _NanoSecondsPerClock.Set(slotPars->GetEntries());
  _CardModel.Set(slotPars->GetEntries());
  _GateMode.Set(slotPars->GetEntries());
  for(int islot = 0; islot < slotPars->GetEntries(); islot++)
  {
    _NanoSecondsPerClock[islot] = slotPars->GetParValueD("NanoSecondsPerSample",islot);
    _CardModel[islot] = atoi(slotPars->GetParValueS("ModId",islot))/10000;
    _GateMode[islot] = slotPars->GetParValueI("3302GateMode",islot);
  }
  
  // Initialize other beginning of run variables
  _runBegin = _config->GetTimeStamp();
  _firstTimeOfRun = TTimeStamp(0,0);
  _lastPlotIndex = -1;
  _runduration = 0;
  _livetime = 0;
  _totalEventsThisRun = 0;
  
}


Long64_t NGMSISRawReader::OpenInputFile(const char* filename)
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

Long64_t NGMSISRawReader::OpenRawBinaryFile(const char* pathname)
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

Long64_t NGMSISRawReader::ReadAll()
{
  int nwordsinbuffer = 0;
  int i = 0;
  
  while( ! _abortread )
  {
    
    nwordsinbuffer = ReadNextBufferFromFile();
    if(!nwordsinbuffer)
    {
			_filecounter++;
      //Check once for subsequent file
      TString rawFileName = _inputfilename;
      rawFileName+="_";
      rawFileName+=_filecounter;
      rawFileName+=".bin";
      
      if(OpenRawBinaryFile(rawFileName)==0)
      {
        nwordsinbuffer = ReadNextBufferFromFile();
      }
      if(!nwordsinbuffer) break;
    }
    
    i++;
    if(i%1 == 0) gSystem->ProcessEvents();
  }
  
  return 0;
  
}

Long64_t NGMSISRawReader::CloseInputFile()
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

Long64_t NGMSISRawReader::ReadNextBufferFromFile()
{
  
  //We begin by reading the header for a packet
  const int nbytesPacketHeader = 10*4;
  int bytesRead = readBytesWaiting(nbytesPacketHeader,true);
  if(nbytesPacketHeader!=bytesRead)
  { 
    if(bytesRead == 0){
      return 0;
    }else{
      LOG<<"ERROR reading packet header"<<ENDM_FATAL;
      return -1;
    }
  }
  
  int packetLengthInWords = _rawbuffer[1];
  
  gSystem->ProcessEvents();
  
  if(readBytesWaiting(packetLengthInWords*4,true)>0)
    writePacket(packetLengthInWords, _rawbuffer);
  else 
    return 0;
  return 1;
}


Long64_t NGMSISRawReader::readBytesWaiting(Long64_t nbytesToRead, bool waitOnData)
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
  _inputfile->read((char*)_rawbuffer,(std::streamsize)nbytesToRead);
  Long64_t bytesRead = _inputfile->gcount();
  
  //Save the pointer for the next read
  _nextbufferPosition = _inputfile->tellg();
  
  if(nbytesToRead != bytesRead)
  {
    LOG<<" Not enough bytes read "<<ENDM_FATAL;
  }
  
  return bytesRead;
  
}


Long64_t NGMSISRawReader::writePacket(int packetlength, unsigned int *data)
{
  
	// Nominal event size without raw data
	unsigned int nomNumberOfEventWords = 10;
	const unsigned int nanoSecInSec = 1000000000;
	unsigned int actNumberOfEventWords = 10;
	unsigned long long rawtimestamp;
	unsigned long long prevrawtimestamp = 0;
  //unused:	unsigned short eventheader;
	unsigned short moduleid;
	unsigned short channelid;
	unsigned short peakheight;
	unsigned short peakindex;
	unsigned short pileup;
	unsigned int gate[8];
	unsigned short numberrawsamples;
	unsigned int beginNanoSec;
	unsigned int beginSec;
	unsigned int prevSec = 0;
	unsigned int prevNanoSec = 0;
  //unused:	unsigned short prevTS1 = 0;
  //unused:	unsigned short prevTS2 = 0;
  //unused:	unsigned short prevTS3 = 0;
  char cbuff[1024];
  sprintf(cbuff,"%x",data[0]&0xFFFF);
  
  
	//LOG<<"Begin packet write... "<<packetlength<<" Address "<<cbuff<<ENDM_INFO;
  
  
  if(!_outBuffer)
	{
		_outBuffer = new NGMBufferedPacketv2(0,0,TTimeStamp((time_t)0,0),6);
	}
  
  TTimeStamp* runBegin = &_runBegin;
  NGMBufferedPacket* packetBuffer = _outBuffer;
  
  
	beginNanoSec = runBegin->GetNanoSec();
	beginSec = runBegin->GetSec();
	if(data == 0 || (unsigned int)packetlength < nomNumberOfEventWords) return -1;
  
	packetBuffer->Clear();
	//LOG<<"Previous packet cleared... "<<packetlength<<" : "<<data[0]<<ENDM_INFO;
  
	unsigned int *nextBuffer = data;
	// Loop over the data event by event,
	// check that next event data is within buffer length
	//LOG<<"Loop over events."<<ENDM_INFO;
	int pulsecounter = 0;
  
	while(( (unsigned int)packetlength - (nextBuffer - data ) >= nomNumberOfEventWords  ))
	{
		pulsecounter++;
		moduleid = nextBuffer[0] & 0xFFF8;
		// Module ID for SIS is the Slot Memory Base Address
		// Following our convention for starting with 0x4000 and increasing
		// with 0x800 increments
		moduleid = (moduleid - 0x4000)/0x800;
    double nanoSecsPerClock = _NanoSecondsPerClock[moduleid];
    if(_GateMode[moduleid]==1 && _CardModel[moduleid]==3302)
    {
      nomNumberOfEventWords = 12;
    }
		channelid = nextBuffer[0] & 0x7;
    
    if(pulsecounter==1){
			packetBuffer->setSlotId(moduleid);
      packetBuffer->setChannelId(channelid);
    }
    
		rawtimestamp = (nextBuffer[0] & 0xFFFF0000);
		rawtimestamp = rawtimestamp<<16;
		rawtimestamp = rawtimestamp | nextBuffer[1];
		
		//std::cout<<rawtimestamp<<std::endl;
    
		peakindex = (nextBuffer[2] & 0xFFFF0000)>>16;
		pileup = (nextBuffer[3]>>24);
    if(_CardModel[moduleid]==3302 && _GateMode[moduleid]==0)
    {
      peakheight = (nextBuffer[2] & 0xFFFF);
      gate[0] = nextBuffer[3] & 0xFFFFFF;
      gate[1] = nextBuffer[4] & 0x0FFFFFFF;
      gate[2] = nextBuffer[5] & 0x0FFFFFFF;
      gate[3] = nextBuffer[6] & 0x0FFFFFFF;
      gate[4] = nextBuffer[7] & 0x0FFFFFFF;
      gate[5] = nextBuffer[8] & 0x0FFFFFFF;
      gate[6] = 0;
      gate[7] = 0;

    }else if(_CardModel[moduleid]==3302 && _GateMode[moduleid]==1){
      peakheight = (nextBuffer[2] & 0xFFFF);
      gate[0] = nextBuffer[3] & 0xFFFFFF;
      gate[1] = nextBuffer[4] & 0x0FFFFFFF;
      gate[2] = nextBuffer[5] & 0x0FFFFFFF;
      gate[3] = nextBuffer[6] & 0x0FFFFFFF;
      gate[4] = nextBuffer[7] & 0x0FFFFFFF;
      gate[5] = nextBuffer[8] & 0x0FFFFFFF;
      gate[6] = nextBuffer[9] & 0x0FFFFFFF;
      gate[7] = nextBuffer[10] & 0x0FFFFFFF;

    }else{
      peakheight = (nextBuffer[2] & 0x0FFF);
      gate[0] = nextBuffer[3] & 0x1FFFFF;
      gate[1] = nextBuffer[4] & 0x1FFFFF;
      gate[2] = nextBuffer[5] & 0x1FFFFF;
      gate[3] = nextBuffer[6] & 0x1FFFFF;
      gate[4] = nextBuffer[7] & 0xFFFF;
      gate[5] = (nextBuffer[7] & 0xFFFF0000)>>16;
      gate[6] = nextBuffer[8] & 0xFFFF;
      gate[7] = (nextBuffer[8] & 0xFFFF0000)>>16;
      
    }
    
    
		numberrawsamples = nextBuffer[nomNumberOfEventWords-1] & 0xFFFF;
		actNumberOfEventWords = nomNumberOfEventWords + (numberrawsamples/2);
		if(pulsecounter == 2)
      //LOG<<moduleid<<"\t"
      //	<<channelid<<"\t"
      //	//<<rawtimestamp<<"\t"
      //	<<actNumberOfEventWords<<"\t"
      //	<<peakindex<<"\t"
      //	<<ENDM_INFO;
		  if(actNumberOfEventWords + (nextBuffer-data) > (unsigned int)packetlength){
        // We have a problem
        // print Error Message
        //	LOG<<"Actual number of words does not match expected."<<ENDM_FATAL;
        
        break;
      }
		NGMHit* tHit = packetBuffer->addHit(0,8);
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
		for(int gateid =0 ; gateid < 8; gateid++)
			tHit->SetGate(gateid,gate[gateid]);
		tHit->SetPileUpCounter(pileup);
    tHit->SetRawClock(rawtimestamp);
    if(numberrawsamples > 0){
			// Copy the waveform
			tHit->SetNSamples(numberrawsamples);
			for(int isample =0; isample < numberrawsamples; isample++){
				tHit->SetSample(isample, ((isample%2)?
                                  (nextBuffer[nomNumberOfEventWords+isample/2])>>16
                                  :(nextBuffer[nomNumberOfEventWords+isample/2] & 0xFFFF)));
			}
		}else{
			tHit->SetNSamples(0);
		}
    
		prevSec = tHit->GetTimeStamp().GetSec();
		prevNanoSec = tHit->GetTimeStamp().GetNanoSec();
    if(prevrawtimestamp>rawtimestamp)
    {
      LOG<<"TimeStamps out of order: Slot("<<(moduleid*8+channelid)
      <<") "<<(long long)prevrawtimestamp<<" : "<< (long long)rawtimestamp <<ENDM_WARN;
    }
    
		prevrawtimestamp = rawtimestamp;
    
		// Debugging rjn
    //prevTS1 = nextBuffer[1] & 0xFFFF;
		//prevTS2 = (nextBuffer[1] & 0xFFFF0000) >> 16;
		//prevTS3 = (nextBuffer[0] & 0xFFFF0000) >> 16;
		
    //Increment nextBuffer
		nextBuffer+=actNumberOfEventWords;
	}
  // Basic analysis of packet
  NGMBufferedPacket* packet = packetBuffer;
  int nHits = packet->getPulseCount();
  
  if(nHits>=1)
  {
    const NGMHit* tHit = packet->getHit(0);
    int plotIndex = tHit->GetSlot()*8 + tHit->GetChannel();
    
    
	  double hitRate = 0.0;
    if(nHits>=2) hitRate = nHits/(packet->getHit(nHits-1)->TimeDiffNanoSec(packet->getHit(0)->GetTimeStamp())*1E-9);
    // Assuming we read in plot index order lets assume we have completely
    // readout a spill if the previous plotIndex > current plotIndex
    
    if(_lastPlotIndex > -1 && _lastPlotIndex >= plotIndex)
    {
      //If _runBegin is still 0 then this is the first spill for this run
      if(_firstTimeOfRun.GetSec() == 0)
      {
        _firstTimeOfRun = _earliestTimeInSpill;
      }
      //We update livetime
      _runduration = _latestTimeInSpill - _firstTimeOfRun;
      _livetime += (double)(_latestTimeInSpill - _earliestTimeInSpill);
      LOG<<"LiveTime( "<<_livetime
      <<" RunDuration( "<<_runduration;
      if(_runduration > 0)
        LOG<<" ) LiveTime( "<<_livetime/_runduration*100.0<<"% ";
      LOG<<" ) AverageHitRate ( "<<_totalEventsThisRun/_livetime<<" ) "<<ENDM_INFO;
    }
    
    if(_lastPlotIndex == -1 || _lastPlotIndex >= plotIndex)
    {
      _earliestTimeInSpill = packet->getHit(0)->GetTimeStamp();
      _latestTimeInSpill = packet->getHit(nHits-1)->GetTimeStamp();
    }else{
      if((double)(packet->getHit(0)->GetTimeStamp() - _earliestTimeInSpill) < 0.0)
        _earliestTimeInSpill = packet->getHit(0)->GetTimeStamp();
      if((double)(packet->getHit(nHits-1)->GetTimeStamp() - _latestTimeInSpill) > 0.0)
        _latestTimeInSpill = packet->getHit(nHits-1)->GetTimeStamp();
    }
    
    _totalEventsThisRun+=nHits;
    _lastPlotIndex = plotIndex;
  }
  //LOG<<"Sending Packet Slot("<<packetBuffer->getHit(0)->GetSlot()
  //   <<") Channel("<<packetBuffer->getHit(0)->GetChannel()
  //   <<")"<<ENDM_INFO;
  GetParent()->push(*((TObject*)packetBuffer));
	
  return 0;
}

////// NGMSIS3305RawReader Implementation
ClassImp(NGMSIS3305RawReader)

NGMSIS3305RawReader::NGMSIS3305RawReader()
{
  _rawbuffer = 0;
  _outBuffer = 0; // The packet buffer filled when parsing raw file
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
}

NGMSIS3305RawReader::NGMSIS3305RawReader(NGMModule* parent)
: NGMReaderBase(parent)
{
  _rawbuffer = new unsigned int[MAXBUFFERLENGTH];
  _outBuffer = new NGMBufferedPacketv2(0,0,TTimeStamp((time_t)0,0),6);
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
  _filecounter = 0;
  
}

NGMSIS3305RawReader::~NGMSIS3305RawReader()
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

void NGMSIS3305RawReader::SetConfiguration(const NGMSystemConfiguration* sysConf)
{
  // Let Base keep a copy of configuration
  NGMReaderBase::SetConfiguration(sysConf);
  
  // Extract Clock Settings for each slot
  NGMConfigurationTable* slotPars = GetConfiguration()->GetSlotParameters();
  
  _NanoSecondsPerClock.Set(slotPars->GetEntries());
  for(int islot = 0; islot < slotPars->GetEntries(); islot++)
  {
    _NanoSecondsPerClock[islot] = slotPars->GetParValueD("NanoSecondsPerSample",islot);
  }
  
  // Initialize other beginning of run variables
  _runBegin = _config->GetTimeStamp();
  _firstTimeOfRun = TTimeStamp(0,0);
  _lastPlotIndex = -1;
  _runduration = 0;
  _livetime = 0;
  _totalEventsThisRun = 0;
  
}


Long64_t NGMSIS3305RawReader::OpenInputFile(const char* filename)
{
  
  CloseInputFile();
  _filecounter = 1;
  
  _inputfilename = filename;
  
  
  NGMSystemConfiguration* confBuffer = new NGMSystemConfigurationv1("SIS3305v1",4,64);
  
  TString fileBaseName = gSystem->BaseName(filename);
  TString teststring = fileBaseName(fileBaseName.Length()-25,21);
  tm_t tdate;
  strptime(teststring.Data(), "D%m-%d-%Y-T%H-%M-%S", &tdate);
  TTimeStamp ts(TTimeStamp::MktimeFromUTC(&tdate));

  if(teststring.BeginsWith("20"))
  {
    long long trunnumber = 0;
    trunnumber = ts.GetDate()*1000000 + ts.GetTime();
    confBuffer->setRunNumber(trunnumber);
    TTimeStamp tmpBeginRunTime;
    tmpBeginRunTime.Set(trunnumber/1000000, trunnumber%1000000, 0, true, 0);
  }
  confBuffer->SetTimeStamp(ts);
  // Save information we need from configuration
  SetConfiguration(confBuffer);
  
  OpenRawBinaryFile(filename);
  return 0;
}

Long64_t NGMSIS3305RawReader::OpenRawBinaryFile(const char* pathname)
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
  const int bytesInRunHeader = 4*4;
  Long64_t result = readBytesWaiting(bytesInRunHeader);
  if(result!=bytesInRunHeader)
  {
    LOG<<" Full Header not found in buffer "<<pathname<<ENDM_INFO;
    return 1;
  }
  
  LOG<<"Found SIS Run Header of version "<<_rawbuffer[1] << ENDM_INFO;
  return 0;
}

Long64_t NGMSIS3305RawReader::ReadAll()
{
  int nwordsinbuffer = 0;
  int i = 0;
  
  while( ! _abortread )
  {
    
    nwordsinbuffer = ReadNextBufferFromFile();
    if(!nwordsinbuffer)
    {
			_filecounter++;
      //Check once for subsequent file
      TString rawFileName = _inputfilename;
      rawFileName+="_";
      rawFileName+=_filecounter;
      rawFileName+=".bin";
      
      if(OpenRawBinaryFile(rawFileName)==0)
      {
        nwordsinbuffer = ReadNextBufferFromFile();
      }
      if(!nwordsinbuffer) break;
    }
    
    i++;
    if(i%1 == 0) gSystem->ProcessEvents();
  }
  
  return 0;
  
}

Long64_t NGMSIS3305RawReader::CloseInputFile()
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

Long64_t NGMSIS3305RawReader::ReadNextBufferFromFile()
{
  
  //We begin by reading the header for a packet
  const int nbytesPacketHeader = 4*4;
  int bytesRead = readBytesWaiting(nbytesPacketHeader,true);
  if(nbytesPacketHeader!=bytesRead)
  { 
    if(bytesRead == 0){
      return 0;
    }else{
      LOG<<"ERROR reading packet header"<<ENDM_FATAL;
      return -1;
    }
  }
  
  if(_rawbuffer[0]!=0xABBAABBA)
  {
    LOG<<"ERROR did not find expected Event Buffer Header"<<ENDM_FATAL;
    exit(1);
  }
  
  int packetLengthInWords = _rawbuffer[3];
  int _curchannel = _rawbuffer[2];
  
  LOG<<"Reading Evt("<<_rawbuffer[1]<<")Channel("<<_curchannel<<") Words("<<_rawbuffer[3]<<")"<<ENDM_INFO;
  
  gSystem->ProcessEvents();
  
  if(readBytesWaiting(packetLengthInWords*4,true)>0)
  {
    char sheaderbuff[255];
    sprintf(sheaderbuff,"%x %x %x",(_rawbuffer[0]&0xF000000)>>28, (_rawbuffer[0]&0x0F00000)>>24,(_rawbuffer[0]&0x00FF000)>>16);
    LOG<<"Reading packet of "<<packetLengthInWords<<" "<<sheaderbuff<<ENDM_INFO;
    
    writePacket(packetLengthInWords, _rawbuffer);    
  }
  else 
    return 0;
  return 1;
}


Long64_t NGMSIS3305RawReader::readBytesWaiting(Long64_t nbytesToRead, bool waitOnData)
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
  _inputfile->read((char*)_rawbuffer,(std::streamsize)nbytesToRead);
  Long64_t bytesRead = _inputfile->gcount();
  
  //Save the pointer for the next read
  _nextbufferPosition = _inputfile->tellg();
  
  if(nbytesToRead != bytesRead)
  {
    LOG<<" Not enough bytes read "<<ENDM_FATAL;
  }
  
  return bytesRead;
  
}


Long64_t NGMSIS3305RawReader::writePacket(int packetlength, unsigned int *data)
{
  
	// Nominal event size without raw data
	const unsigned int nomNumberOfEventWords = 4;
	const unsigned int nanoSecInSec = 1000000000;
	unsigned int actNumberOfEventWords = 10;
	unsigned long long rawtimestamp;
	unsigned long long prevrawtimestamp = 0;
  //unused:	unsigned short eventheader;
	unsigned short moduleid;
	unsigned short channelid;
	unsigned short peakheight;
	unsigned short peakindex;
	unsigned short pileup;
	unsigned int gate[8];
	unsigned short numberrawsamples;
	unsigned int beginNanoSec;
	unsigned int beginSec;
	unsigned int prevSec = 0;
	unsigned int prevNanoSec = 0;
  //unused:	unsigned short prevTS1 = 0;
  //unused:	unsigned short prevTS2 = 0;
  //unused:	unsigned short prevTS3 = 0;
  char cbuff[1024];
  sprintf(cbuff,"%x",data[0]&0xFFFF);
  
  
	//LOG<<"Begin packet write... "<<packetlength<<" Address "<<cbuff<<ENDM_INFO;
  
  
  if(!_outBuffer)
	{
		_outBuffer = new NGMBufferedPacketv2(0,0,TTimeStamp((time_t)0,0),6);
	}
  
  TTimeStamp* runBegin = &_runBegin;
  NGMBufferedPacket* packetBuffer = _outBuffer;
  
  
	beginNanoSec = runBegin->GetNanoSec();
	beginSec = runBegin->GetSec();
	if(data == 0 || (unsigned int)packetlength < nomNumberOfEventWords) return -1;
  
	packetBuffer->Clear();
	//LOG<<"Previous packet cleared... "<<packetlength<<" : "<<data[0]<<ENDM_INFO;
  
	unsigned int *nextBuffer = data;
	// Loop over the data event by event,
	// check that next event data is within buffer length
	//LOG<<"Loop over events."<<ENDM_INFO;
	int pulsecounter = 0;
  
	while(( (unsigned int)packetlength - (nextBuffer - data ) >= nomNumberOfEventWords  ))
	{
		pulsecounter++;
		// Module ID for SIS is the Slot Memory Base Address
		// Following our convention for starting with 0x4000 and increasing
		// with 0x800 increments
		moduleid = _curchannel/2;
    
    unsigned int eventID = (nextBuffer[0]&0xF0000000)>>28;
    unsigned int evtHeaderInfo = (nextBuffer[0]&0x0F000000)>>28;
    unsigned int evtHeaderID = (nextBuffer[0]&0x00FF0000)>>28;
    unsigned int nblocks = nextBuffer[3]&0xFFFF;

    if (eventID<0x4) {
      unsigned int blocksize = 4;
      actNumberOfEventWords = nomNumberOfEventWords + (nblocks)*blocksize;
      LOG<<"Parsing 1.25GS/s Event: "<<eventID<<" "<<evtHeaderInfo<<" "<<evtHeaderID<<" "<<actNumberOfEventWords<<ENDM_INFO;

      // 1.25GS/s event
      double nanoSecsPerClock = 1.0/1.25;
      NGMHit* tHit = packetBuffer->addHit();
      rawtimestamp = (unsigned long long)nextBuffer[1] + ((unsigned long long)(nextBuffer[0]&0xFFFF))<<32;
      unsigned long long timeInPicoSeconds = (rawtimestamp*int(nanoSecsPerClock*1000));
      unsigned long long timeInNanoSeconds = timeInPicoSeconds/1000;
      unsigned long long timeInSeconds = timeInNanoSeconds/nanoSecInSec;
      tHit->SetRawTime(
                       NGMTimeStamp( (time_t)(beginSec + timeInSeconds),
                                    beginNanoSec + (timeInNanoSeconds - timeInSeconds*nanoSecInSec)
                                    ,timeInPicoSeconds - timeInNanoSeconds*1000)
                       );
      tHit->SetChannel(eventID + 4*(_curchannel%2)); // assuming channel is 4group chip number
      tHit->SetSlot(moduleid);
      unsigned int nsamples = nblocks*blocksize*3;
      unsigned int isample = 0;
      LOG<<"Parsing "<<nsamples<<" samples."<<ENDM_INFO;
      tHit->SetNSamples(nsamples);
      for (int iblock=0; iblock < nblocks; iblock++) {
        for (int iw = 0; iw < blocksize; iw++) {
          unsigned int sampleTriplet = nextBuffer[nomNumberOfEventWords+iblock*blocksize+iw];
          tHit->SetSample(isample++, 0x3FF-(sampleTriplet>>20)&0x3FF);
          tHit->SetSample(isample++, 0x3FF-(sampleTriplet>>10)&0x3FF);
          tHit->SetSample(isample++, 0x3FF-(sampleTriplet)&0x3FF);
        }
      }
      tHit->Print();
    }else if (eventID < 0x6) {
      unsigned int blocksize = 8;
      actNumberOfEventWords = nomNumberOfEventWords + (nblocks)*blocksize;
      LOG<<"Parsing 2.5GS/s Event: "<<eventID<<" "<<evtHeaderInfo<<" "<<evtHeaderID<<" "<<actNumberOfEventWords<<ENDM_INFO;
      
      // 2.5GS/s event
      double nanoSecsPerClock = 1.0/2.5;
      NGMHit* tHit = packetBuffer->addHit();
      rawtimestamp = (unsigned long long)nextBuffer[1] + ((unsigned long long)(nextBuffer[0]&0xFFFF))<<32;
      unsigned long long timeInPicoSeconds = (rawtimestamp*int(nanoSecsPerClock*1000));
      unsigned long long timeInNanoSeconds = timeInPicoSeconds/1000;
      unsigned long long timeInSeconds = timeInNanoSeconds/nanoSecInSec;
      tHit->SetRawTime(
                       NGMTimeStamp( (time_t)(beginSec + timeInSeconds),
                                    beginNanoSec + (timeInNanoSeconds - timeInSeconds*nanoSecInSec)
                                    ,timeInPicoSeconds - timeInNanoSeconds*1000)
                       );
      tHit->SetChannel(eventID-0x4 + 4*(_curchannel%2)); // assuming channel is 4group chip number
      tHit->SetSlot(moduleid);
      unsigned int nsamples = nblocks*blocksize*3;
      LOG<<"Parsing "<<nsamples<<" samples."<<ENDM_INFO;
      tHit->SetNSamples(nsamples);
      for (int iblock=0; iblock < nblocks; iblock++) {
        int isamplebase = iblock*blocksize*3;
        for (int iw = 0; iw < blocksize; iw++) {
          unsigned int sampleTriplet = nextBuffer[nomNumberOfEventWords+iblock*blocksize+iw];
          tHit->SetSample(isamplebase+iw/4 + (iw%4)*6, 0x3FF-(sampleTriplet>>20)&0x3FF);
          tHit->SetSample(isamplebase+iw/4 + (iw%4)*6+2, 0x3FF-(sampleTriplet>>10)&0x3FF);
          tHit->SetSample(isamplebase+iw/4 + (iw%4)*6+4, 0x3FF-(sampleTriplet)&0x3FF);
        }
      }
      tHit->Print();

    }else if(eventID < 0x8){
      // 5GS/s Event
      unsigned int blocksize = 16;
      actNumberOfEventWords = nomNumberOfEventWords + nblocks*blocksize;
      LOG<<"Parsing 5GS/s Event: "<<eventID<<" "<<evtHeaderInfo<<" "<<evtHeaderID<<" "<<actNumberOfEventWords<<ENDM_INFO;
      LOG<<"Parsing 2.5GS/s Event: "<<eventID<<" "<<evtHeaderInfo<<" "<<evtHeaderID<<" "<<actNumberOfEventWords<<ENDM_INFO;
      
      // 5GS/s event
      double nanoSecsPerClock = 1.0/5.0;
      NGMHit* tHit = packetBuffer->addHit();
      rawtimestamp = (unsigned long long)nextBuffer[1] + ((unsigned long long)(nextBuffer[0]&0xFFFF))<<32;
      unsigned long long timeInPicoSeconds = (rawtimestamp*int(nanoSecsPerClock*1000));
      unsigned long long timeInNanoSeconds = timeInPicoSeconds/1000;
      unsigned long long timeInSeconds = timeInNanoSeconds/nanoSecInSec;
      tHit->SetRawTime(
                       NGMTimeStamp( (time_t)(beginSec + timeInSeconds),
                                    beginNanoSec + (timeInNanoSeconds - timeInSeconds*nanoSecInSec)
                                    ,timeInPicoSeconds - timeInNanoSeconds*1000)
                       );
      tHit->SetChannel(4*(_curchannel%2)); // assuming channel is 4group chip number
      tHit->SetSlot(moduleid);
      unsigned int nsamples = nblocks*blocksize*3;
      unsigned int isample = 0;
      LOG<<"Parsing "<<nsamples<<" samples."<<ENDM_INFO;
      tHit->SetNSamples(nsamples);
      for (int iblock=0; iblock < nblocks; iblock++) {
        int isamplebase = iblock*blocksize*3;
        for (int iw = 0; iw < blocksize; iw++) {
          unsigned int sampleTriplet = nextBuffer[nomNumberOfEventWords+iblock*blocksize+iw];
          // isamplebase = beginning of datablock
          // iw/8 = odd or even
          // ((iw%8)/4)*2 = top or bottom of odd and even blocks see page 60 of SIS3305 manual
          // (iw%4)*12 + {0,4,8} determines where each 10 bits goes.
          tHit->SetSample(isamplebase + iw/8 + ((iw%8)/4)*2 + (iw%4)*12, 0x3FF-(sampleTriplet>>20)&0x3FF);
          tHit->SetSample(isamplebase + iw/8 + ((iw%8)/4)*2 + (iw%4)*12+4, 0x3FF-(sampleTriplet>>10)&0x3FF);
          tHit->SetSample(isamplebase + iw/8 + ((iw%8)/4)*2 + (iw%4)*12+8, 0x3FF-(sampleTriplet)&0x3FF);
        }
      }
      tHit->Print();
      
    }else if(eventID < 0x9){
      // TDC Event
      actNumberOfEventWords = nomNumberOfEventWords;
      LOG<<"Parsing TDC Event: "<<eventID<<" "<<evtHeaderInfo<<" "<<evtHeaderID<<" "<<actNumberOfEventWords<<ENDM_INFO;

    }else{
      // End of record marker
      actNumberOfEventWords = nomNumberOfEventWords + nblocks*16;;
      LOG<<"Parsing NULL Event: "<<eventID<<" "<<evtHeaderInfo<<" "<<evtHeaderID<<" "<<actNumberOfEventWords<<ENDM_INFO;
      break;
    }
    
    if(actNumberOfEventWords + (nextBuffer-data) >= (unsigned int)packetlength){
      // We have a problem
      // print Error Message
      LOG<<"Next expected packet position ("<<actNumberOfEventWords + (nextBuffer-data)<<") longer than packet length "<<packetlength<<ENDM_FATAL;
      break;
    }
    
    //Increment nextBuffer
		nextBuffer+=actNumberOfEventWords;
	}
  
  // Basic analysis of packet
  NGMBufferedPacket* packet = packetBuffer;
  int nHits = packet->getPulseCount();
  
  if(nHits>=1)
  {
    _totalEventsThisRun+=nHits;
    _lastPlotIndex = 0;
  }
  if(nHits>=1)
  {
    LOG<<"Sending Packet Slot("<<packetBuffer->getHit(0)->GetSlot();
    LOG  <<") Channel("<<packetBuffer->getHit(0)->GetChannel();
    LOG   <<") Count("<<packet->getPulseCount()<<")"<<ENDM_INFO;    
  }
  GetParent()->push(*((TObject*)packetBuffer));
	
  return 0;
}

////// NGMPIXIE16ORNLRawReader Implementation
ClassImp(NGMPIXIE16ORNLRawReader)

NGMPIXIE16ORNLRawReader::NGMPIXIE16ORNLRawReader()
{
  _rawbuffer = 0;
  _outBuffer = 0; // The packet buffer filled when parsing raw file
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;

}

NGMPIXIE16ORNLRawReader::NGMPIXIE16ORNLRawReader(NGMModule* parent)
: NGMReaderBase(parent)
{
  _rawbuffer = new unsigned int[MAXBUFFERLENGTH];
  _outBuffer = new NGMBufferedPacketv2(0,0,TTimeStamp((time_t)0,0),6);
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
  _filecounter = 0;

  
}

NGMPIXIE16ORNLRawReader::~NGMPIXIE16ORNLRawReader()
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

void NGMPIXIE16ORNLRawReader::SetConfiguration(const NGMSystemConfiguration* sysConf)
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
//NGMSystemConfiguration* NGMPIXIE16ORNLRawReader::CreateConfigurationFromPIXIE()
//{
//  
//}


Long64_t NGMPIXIE16ORNLRawReader::OpenInputFile(const char* filename)
{
  
  CloseInputFile();
  _filecounter = 1;
  
  _inputfilename = filename;


  NGMSystemConfiguration* confBuffer = new NGMSystemConfigurationv1("PIXIE16ORNLv2",4,64);
  
  TString fileBaseName = gSystem->BaseName(filename);
  TString teststring = fileBaseName(fileBaseName.Length()-18,14);
  if(teststring.BeginsWith("20"))
  {
    long long trunnumber = 0;
    trunnumber = teststring.Atoll();
    confBuffer->setRunNumber(trunnumber);
    TTimeStamp tmpBeginRunTime;
    tmpBeginRunTime.Set(trunnumber/1000000, trunnumber%1000000, 0, true, 0);
  }
  
  // Save information we need from configuration
  SetConfiguration(confBuffer);
  
  OpenRawBinaryFile(filename);
  
  return 0;
}

Long64_t NGMPIXIE16ORNLRawReader::OpenRawBinaryFile(const char* pathname)
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
//  Long64_t result = readBytesWaiting(bytesInRunHeader);
//  if(result!=bytesInRunHeader)
//  {
//    LOG<<" Full Header not found in buffer "<<pathname<<ENDM_INFO;
//    return 1;
//  }
//  
//  LOG<<"Found SIS Run Header of version "<<_rawbuffer[0] << ENDM_INFO;
  return 0;
}

Long64_t NGMPIXIE16ORNLRawReader::ReadAll()
{
  int nwordsinbuffer = 0;
  int i = 0;
  
  while( ! _abortread )
  {
    
    nwordsinbuffer = ReadNextBufferFromFile();
    if(nwordsinbuffer == 0) break;
//    if(!nwordsinbuffer)
//    {
//			_filecounter++;
//      //Check once for subsequent file
//      TString rawFileName = _inputfilename;
//      rawFileName+="_";
//      rawFileName+=_filecounter;
//      rawFileName+=".bin";
//      
//      if(OpenRawBinaryFile(rawFileName)==0)
//      {
//        nwordsinbuffer = ReadNextBufferFromFile();
//      }
//      if(!nwordsinbuffer) break;
//    }
    
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

Long64_t NGMPIXIE16ORNLRawReader::CloseInputFile()
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

Long64_t NGMPIXIE16ORNLRawReader::ReadNextBufferFromFile()
{
  return ReadNextBufferFromFilev2();
}

Long64_t NGMPIXIE16ORNLRawReader::ReadNextBufferFromFilev1()
{
  const UInt_t EVENT_LENGTH_SHIFT = 			17;
  const UInt_t EVENT_LENGTH_MASK = 			0x7FFE0000;

  //We begin by reading the header for a packet
  const int nbytesPacketHeader = 4*4;
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
  
  int packetLengthInWords = ( _rawbuffer[0] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT;
  
  if (packetLengthInWords > 10000) {
    LOG<<" Suspiciously large packet size: "<<packetLengthInWords<<""<<ENDM_FATAL;
    exit(-1);
  }
  
    
  gSystem->ProcessEvents();
  
  if(readBytesWaiting(packetLengthInWords*4 - nbytesPacketHeader,false,nbytesPacketHeader)>0)
  {
    Long64_t status = writePacket(packetLengthInWords, _rawbuffer);
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

Long64_t NGMPIXIE16ORNLRawReader::ReadNextBufferFromFilev2()
{
  const UInt_t EVENT_LENGTH_SHIFT = 			17;
  const UInt_t EVENT_LENGTH_MASK = 			0x7FFE0000;
  
  //We begin by reading the header for a packet
  const int nbytesPacketHeader = 5*4;
  // This is a little different from v1 in that we expect a sequence of 1 summed header
  // and 4 individual channel computations.
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
  
  int packetLengthInWordsSum = ( _rawbuffer[0] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT;
  int packetLengthInWordsSingle = ( _rawbuffer[4] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT;
  int packetLengthInWords = packetLengthInWordsSum + 4*packetLengthInWordsSingle;
  
  //LOG<<"PL: "<<packetLengthInWordsSum<<" "<<packetLengthInWordsSingle<<ENDM_INFO;
  
  if (packetLengthInWords > 10000) {
    LOG<<" Suspiciously large packet size: "<<packetLengthInWords<<""<<ENDM_FATAL;
    exit(-1);
  }
  
  
  gSystem->ProcessEvents();
  
  if(readBytesWaiting(packetLengthInWords*4 - nbytesPacketHeader,false,nbytesPacketHeader)>0)
  {
    Long64_t status = writePacketv2(packetLengthInWords, _rawbuffer);
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


int NGMPIXIE16ORNLRawReader::AnaStats(NGMBufferedPacket* packet)
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

Long64_t NGMPIXIE16ORNLRawReader::readBytesWaiting(Long64_t nbytesToRead, bool waitOnData, size_t localOffsetInBytes)
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


Long64_t NGMPIXIE16ORNLRawReader::writePacket(int packetlength, unsigned int *data)
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
  const UInt_t EVENT_ENERGY_MASK = 			0xFFFF;
  const UInt_t EVENT_TIME_LOW_MASK = 			0xFFFFFFFF;
  const UInt_t PULSE_SHAPE_MASK = 			0xFFFFFFFF;
  const UInt_t ADC_DATA1_MASK = 				0xFFF;
  const UInt_t ADC_DATA2_MASK = 				0xFFF0000;
  
	// Nominal event size without raw data
	const unsigned int nomNumberOfEventWords = 4;
	const unsigned int nanoSecInSec = 1000000000;
	unsigned int actNumberOfEventWords = 4;
	unsigned long long rawtimestamp;
	unsigned long long prevrawtimestamp = 0;
	unsigned short moduleid;
	unsigned short channelid;
	unsigned short peakheight;
	unsigned short peakindex;
	unsigned short pileup;
  unsigned int cfd;
	unsigned int gate[8];
	unsigned short numberrawsamples;
  unsigned short headerlength;
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
  
  
	beginNanoSec = runBegin->GetNanoSec();
	beginSec = runBegin->GetSec();
	if(data == 0 || (unsigned int)packetlength < nomNumberOfEventWords) return -1;
  
 
	unsigned int *nextBuffer = data;
	// Loop over the data event by event,
	// check that next event data is within buffer length
	//LOG<<"Loop over events."<<ENDM_INFO;
	int pulsecounter = 0;
  
	while(( (unsigned int)packetlength - (nextBuffer - data ) >= nomNumberOfEventWords  ))
	{
		pulsecounter++;
		moduleid = ( nextBuffer[0] & SLOT_ID_MASK ) >> SLOT_ID_SHIFT;
    if(moduleid>=_NanoSecondsPerClock.GetSize()){
      LOG<<"Module Id is "<<moduleid<<" and data appears corrupt"<<ENDM_FATAL;
      return -1;
    }
    double nanoSecsPerClock = _NanoSecondsPerClock[moduleid];
    
		channelid = ( nextBuffer[0] & CHANNEL_NUMBER_MASK)  >> CHANNEL_NUMBER_SHIFT;
    
    if(pulsecounter==1){
			packetBuffer->setSlotId(moduleid);
      packetBuffer->setChannelId(channelid);
    }
    actNumberOfEventWords = ( nextBuffer[0] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT;
    headerlength = ( nextBuffer[0] & HEADER_LENGTH_MASK ) >> HEADER_LENGTH_SHIFT;
    
		rawtimestamp = ( nextBuffer[1] & EVENT_TIME_LOW_MASK ) >> EVENT_TIME_LOW_SHIFT;
                 
		rawtimestamp = rawtimestamp | (((((unsigned long long)(nextBuffer[2])) & EVENT_TIME_HIGH_MASK) >> EVENT_TIME_HIGH_SHIFT)<<32);
    cfd = (nextBuffer[2] & CFD_FRACTIONAL_TIME_MASK) >> CFD_FRACTIONAL_TIME_SHIFT;
    peakheight = (nextBuffer[3] & EVENT_ENERGY_MASK) >> EVENT_ENERGY_SHIFT;
    numberrawsamples = ( nextBuffer[3] & TRACE_LENGTH_MASK ) >> TRACE_LENGTH_SHIFT;
    pileup =  ( nextBuffer[0] & FINISH_CODE_MASK ) >> FINISH_CODE_SHIFT;
    
    if(numberrawsamples > 10000)
    {
      
      
    }
    
		if(pulsecounter == 2)
      //LOG<<moduleid<<"\t"
      //	<<channelid<<"\t"
      //	//<<rawtimestamp<<"\t"
      //	<<actNumberOfEventWords<<"\t"
      //	<<peakindex<<"\t"
      //	<<ENDM_INFO;
		  if(actNumberOfEventWords + (nextBuffer-data) > (unsigned int)packetlength){
        // We have a problem
        // print Error Message
        //	LOG<<"Actual number of words does not match expected."<<ENDM_FATAL;
        
        break;
      }
                 
		NGMHit* tHit = packetBuffer->addHit();
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
		for(int gateid =0 ; gateid < 8; gateid++)
			tHit->SetGate(gateid,gate[gateid]);
		tHit->SetPileUpCounter(pileup);
    tHit->SetRawClock(rawtimestamp);
    if(numberrawsamples > 0){
			// Copy the waveform
			tHit->SetNSamples(numberrawsamples);
			for(int isample =0; isample < numberrawsamples; isample++){
        unsigned short tmpsample = ((isample%2)?
                                    (nextBuffer[headerlength+isample/2])>>16
                                    :(nextBuffer[headerlength+isample/2] & 0xFFFF));
				tHit->SetSample(isample, tmpsample);
			}
		}else{
      LOG<<"No Waveform "<<ENDM_FATAL;
      exit(-1);
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
    
		// Debugging rjn
    //prevTS1 = nextBuffer[1] & 0xFFFF;
		//prevTS2 = (nextBuffer[1] & 0xFFFF0000) >> 16;
		//prevTS3 = (nextBuffer[0] & 0xFFFF0000) >> 16;
		
    //Increment nextBuffer
		nextBuffer+=actNumberOfEventWords;
	}
	
  return 0;
}

Long64_t NGMPIXIE16ORNLRawReader::writePacketv2(int packetlength, unsigned int *data)
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
  
  
	beginNanoSec = runBegin->GetNanoSec();
	beginSec = runBegin->GetSec();
	if(data == 0 || (unsigned int)packetlength < nomNumberOfEventWords) return -1;
  
  
	unsigned int *nextBuffer = data;
	// Loop over the data event by event,
	// check that next event data is within buffer length
	//LOG<<"Loop over events."<<ENDM_INFO;
	int pulsecounter = 0;
  headerlength = ( nextBuffer[4] & HEADER_LENGTH_MASK ) >> HEADER_LENGTH_SHIFT;
  eventlength = ( nextBuffer[4] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT;
  numberrawsamples = (eventlength-headerlength)*2;  
  actNumberOfEventWords = ((( nextBuffer[0] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT)
                           + 4*(( nextBuffer[4] & EVENT_LENGTH_MASK ) >> EVENT_LENGTH_SHIFT));
  
  moduleid = ( nextBuffer[0] & SLOT_ID_MASK ) >> SLOT_ID_SHIFT;
  if(moduleid>=_NanoSecondsPerClock.GetSize()){
    LOG<<"Module Id is "<<moduleid<<" and data appears corrupt"<<ENDM_FATAL;
    return -1;
  }
  double nanoSecsPerClock = _NanoSecondsPerClock[moduleid];
  channelid = ( nextBuffer[0] & CHANNEL_NUMBER_MASK)  >> CHANNEL_NUMBER_SHIFT;
  
  pulsecounter++;
  
  
  if(pulsecounter==1){
    packetBuffer->setSlotId(moduleid);
    packetBuffer->setChannelId(channelid);
  }
  
  rawtimestamp = ( nextBuffer[1] & EVENT_TIME_LOW_MASK ) >> EVENT_TIME_LOW_SHIFT;
  rawtimestamp = rawtimestamp | (uint64_t(((nextBuffer[2] & EVENT_TIME_HIGH_MASK) >> EVENT_TIME_HIGH_SHIFT))<<32);
  cfd = (nextBuffer[2] & CFD_FRACTIONAL_TIME_MASK) >> CFD_FRACTIONAL_TIME_SHIFT;
  peakheight = (nextBuffer[3] & EVENT_ENERGY_MASK) >> EVENT_ENERGY_SHIFT;
  pileup =  ( nextBuffer[0] & FINISH_CODE_MASK ) >> FINISH_CODE_SHIFT;
  
  if(numberrawsamples > 10000)
  {
  }
  
  
  
  NGMHit* tHit = packetBuffer->addHit();
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
  tHit->SetCFD(cfd);
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
	tHit->SetGate(4*ichan + igate, nextBuffer[offset]);
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
    LOG<<"No Waveform "<<ENDM_FATAL;
    exit(-1);
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
	
  return 0;
}

////// NGMGageRawReader Implementation
ClassImp(NGMGageRawReader)

NGMGageRawReader::NGMGageRawReader()
{
  _rawbuffer = 0;
  _outBuffer = 0; // The packet buffer filled when parsing raw file
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
}

NGMGageRawReader::NGMGageRawReader(NGMModule* parent)
: NGMReaderBase(parent)
{
  _rawbuffer = new unsigned int[MAXBUFFERLENGTH];
  _outBuffer = new NGMBufferedPacketv2(0,0,TTimeStamp((time_t)0,0));
	_runBegin = TTimeStamp(0,0);
  _nextbufferPosition = 0;
  _runduration = 0;
  _inputfile = 0;
}

NGMGageRawReader::~NGMGageRawReader()
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

void NGMGageRawReader::SetConfiguration(const NGMSystemConfiguration* sysConf)
{
  // Let Base keep a copy of configuration
  NGMReaderBase::SetConfiguration(sysConf);
    
  // Initialize other beginning of run variables
  _runBegin = _config->GetTimeStamp();
  _firstTimeOfRun = TTimeStamp(0,0);
  _lastPlotIndex = -1;
  _runduration = 0;
  _livetime = 0;
  _totalEventsThisRun = 0;
  
}


Long64_t NGMGageRawReader::OpenInputFile(const char* filename)
{
  char cbuf[4096];
  CloseInputFile();
  
  if(! _inputfile)   _inputfile = new std::ifstream;
  FileStat_t pathinfo;
  gSystem->GetPathInfo(filename,pathinfo);
  
  _inputfile->open(filename,std::ios::in|std::ios::binary);
  _nextbufferPosition = 0;
  
  if(!_inputfile->is_open())
  {
    LOG<<" Unable to open file "<<filename<<ENDM_WARN;
    return 0;
  }

  //Populate this based on the new format.
  
  // Read header
  const int minBytesInRunHeader = 20;
  Long64_t result = readBytesWaiting(minBytesInRunHeader);
  if(result!=minBytesInRunHeader)
  {
    LOG<<" Full Header not found in buffer "<<ENDM_INFO;
    return 1;
  }
  
  /////////////////////
  // Header FORMAT
  /////////////////////
  // Header Type 200 for GAGE in int32
  // Header Version 1 int32
  // Run Number uint64
  // Comment lenth uint32
  // Comment char[length]  and is not null terminated
  
  Int_t headerType;
  Int_t headerVersion;
  ULong64_t runnumber;
  UInt_t commentLength;
  headerType = *((Int_t*)(&(_rawbuffer[0])));
  headerVersion = *((Int_t*)(&(_rawbuffer[1])));
  runnumber = *((ULong64_t*)(&(_rawbuffer[2])));
  
  //Weird behavior of Labview is to use default endian on strings even though little-endian reqested
  commentLength = ((_rawbuffer[4]&0xFF)<<24)|((_rawbuffer[4]&0xFF00)<<8)|((_rawbuffer[4]&0xFF0000)>>8)|((_rawbuffer[4]&0xFF000000)>>24);
  printf("%x\n",_rawbuffer[4]);
  TString srunnumber;
  srunnumber+=runnumber;
  
  if(headerType==200)
  {
    LOG<<"Found GAGE Run Header of version "<<headerVersion << ENDM_INFO;
    LOG<<"GAGE runnumber "<<srunnumber.Data() << ENDM_INFO;
    LOG<<"Comment Length "<<commentLength<<ENDM_INFO;
  }else{
    LOG<<"Header does not match GAGE raw type "<<ENDM_FATAL;
    exit(-1);
  }
  

  result = readBytesWaiting(commentLength);
  if(commentLength!=result)
  {
    LOG<<"Unable to parse header"<<ENDM_FATAL;
    exit(1);
  }
  strncpy(cbuf,(char*)(_rawbuffer),commentLength);
  cbuf[commentLength] = '\0';
  
  // Read number of channels in configuration
  result = readBytesWaiting(4);
  unsigned int nchannels = *((UInt_t*)(&(_rawbuffer[0])));
  
  // Save information we need from configuration
  NGMSystemConfiguration* confBuffer = new NGMSystemConfigurationv1("GAGE",nchannels,nchannels);
  confBuffer->setRunNumber(runnumber);
  NGMConfigurationTable* sysConf = confBuffer->GetSystemParameters();
  NGMConfigurationTable* chanConf = confBuffer->GetChannelParameters();
  sysConf->AddParameterS("Comment", "Comment");
  sysConf->SetParameterS("Comment", 0, cbuf);
  sysConf->PrintRow(0);

  chanConf->AddParameterS("DetectorName", "");
  chanConf->AddParameterD("DynamicRange", 2000.0, 0.0, 10000.0);
  chanConf->AddParameterD("VOffset", 2000.0, -10000.0, 10000.0);
  chanConf->AddParameterD("NanoSecondsPerSample",5.0,0.0,10000.0);
  
  // Read the number of strings in the detector name array
  // Should be the same as nchannels
  result = readBytesWaiting(4);
  if(nchannels!=_rawbuffer[0])
  {
    LOG<<"Error parsing header"<<ENDM_FATAL;
    exit(0);
  }
  
  for(unsigned int ichan =0; ichan < nchannels; ichan++)
  {
    result = readBytesWaiting(4);
    unsigned int nameLength= _rawbuffer[0];
    LOG<<"Name length "<<nameLength<<ENDM_INFO;
    result = readBytesWaiting(nameLength);
    strncpy(cbuf,(char*)(_rawbuffer),nameLength);
    cbuf[nameLength] = '\0';
    LOG<<"Name "<<cbuf<<ENDM_INFO;
    chanConf->SetParameterS("DetectorName", ichan, cbuf);
  }
  // Read the number of doubles in the dynamicrange array
  // Should be the same as nchannels
  result = readBytesWaiting(4);
  if(nchannels!=_rawbuffer[0])
  {
    LOG<<"Error parsing header"<<ENDM_FATAL;
    exit(0);
  }
  
  for(unsigned int ichan =0; ichan < nchannels; ichan++)
  {
    result = readBytesWaiting(4);
    double dynamicrange = *((float*)(&(_rawbuffer[0])));
    chanConf->SetParameterD("DynamicRange", ichan, dynamicrange);
  }
  // Read the number of doubles in the voltageoffset array
  // Should be the same as nchannels
  result = readBytesWaiting(4);
  if(nchannels!=_rawbuffer[0])
  {
    LOG<<"Error parsing header"<<ENDM_FATAL;
    exit(0);
  }
  
  for(unsigned int ichan =0; ichan < nchannels; ichan++)
  {
    result = readBytesWaiting(4);
    double voltageoffset = *((float*)(&(_rawbuffer[0])));
    chanConf->SetParameterD("VOffset", ichan, voltageoffset);
  }
  // Read the number of doubles in the clockspeed array
  // Should be the same as nchannels
  result = readBytesWaiting(4);
  if(nchannels!=_rawbuffer[0])
  {
    LOG<<"Error parsing header"<<ENDM_FATAL;
    exit(0);
  }
  
  for(unsigned int ichan =0; ichan < nchannels; ichan++)
  {
    result = readBytesWaiting(4);
    double nspersample = *((float*)(&(_rawbuffer[0])));
    nspersample=1.0/nspersample*1E9;
    chanConf->SetParameterD("NanoSecondsPerSample", ichan, nspersample);
  }
  chanConf->PrintRow(0);
  
  // Now read the GAGEHeaderTrailer
  result = readBytesWaiting(4);
  if(_rawbuffer[0]!=0xEDEDBFAF)
  {
    LOG<<"Error parsing header"<<ENDM_FATAL;
    exit(0);
  }
  
  SetConfiguration(confBuffer);
  
  
  return 0;
}

Long64_t NGMGageRawReader::ReadAll()
{
  int nwordsinbuffer = 0;
  int i = 0;
  
  while( (nwordsinbuffer = ReadNextBufferFromFile()) && ! _abortread )
  {
    i++;
    if(i%1 == 0) gSystem->ProcessEvents();
  }
  
  return 0;
  
}

Long64_t NGMGageRawReader::CloseInputFile()
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

Long64_t NGMGageRawReader::ReadNextBufferFromFile()
{
  
  //We begin by reading the header for a packet
  const int nbytesPacketHeader = 12;
  int bytesRead = readBytesWaiting(nbytesPacketHeader,true);
  if(nbytesPacketHeader!=bytesRead)
  { 
    if(bytesRead == 0){
      return 0;
    }else{
      LOG<<"ERROR reading packet header"<<ENDM_FATAL;
      return -1;
    }
  }
  double timeSinceBeginOfRun =  *((double*)(&(_rawbuffer[0])));
  short unsigned chanid =  (_rawbuffer[2]&0xFFFF);
  short unsigned nsamples =  (_rawbuffer[2]>>16);
  
  gSystem->ProcessEvents();
  
  if(readBytesWaiting((nsamples/2+nsamples%2)*4,true)>0)
    writePacket(timeSinceBeginOfRun, chanid, nsamples);
  else 
    return 0;
  return 1;
}


Long64_t NGMGageRawReader::readBytesWaiting(Long64_t nbytesToRead, bool waitOnData)
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
  _nextbufferPosition =_inputfile->tellg();
  _inputfile->seekg(0,std::ios::end);
  filesize = _inputfile->tellg();
  _inputfile->seekg(_nextbufferPosition);
  
  int ntriestoread=0;
  
  while((filesize-_nextbufferPosition)<(unsigned int)nbytesToRead)
  {
    
    // Check for end of file record
    const unsigned int tailrecord = 0xDEADBEEF;
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
  _inputfile->read((char*)_rawbuffer,(std::streamsize)nbytesToRead);
  Long64_t bytesRead = _inputfile->gcount();
  
  //Save the pointer for the next read
  _nextbufferPosition = _inputfile->tellg();
  
  if(nbytesToRead != bytesRead)
  {
    LOG<<" Not enough bytes read "<<ENDM_FATAL;
  }
  
  return bytesRead;
  
}


Long64_t NGMGageRawReader::writePacket(double timens, int chanid, int nsamples)
{
  
  if(!_outBuffer)
	{
		_outBuffer = new NGMBufferedPacketv2;
	}
  
  NGMBufferedPacket* packetBuffer = _outBuffer;
  
	packetBuffer->Clear();
  // For now just one hit per packet
  // we'll aggregate later
  
		NGMHit* tHit = packetBuffer->addHit();
		tHit->SetRawTime(
                     NGMTimeStamp( 0,
                                  timens
                                  ,((int)(timens*1000.0))%1000)
                     );
    
		tHit->SetSlot(0);
		tHit->SetChannel(chanid);
    if(nsamples > 0){
			// Copy the waveform
			tHit->SetNSamples(nsamples);
			for(int isample =0; isample < nsamples; isample++){
				tHit->SetSample(isample, ((isample%2)?
                                  (_rawbuffer[isample/2])>>16
                                  :(_rawbuffer[isample/2] & 0xFFFF)));
			}
		}else{
			tHit->SetNSamples(0);
		}
        
    GetParent()->push(*((TObject*)packetBuffer));
	
  return 0;
}



