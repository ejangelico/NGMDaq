#include "NGMPacketBufferIO.h"

#include "TFile.h"
#include "TTree.h"
#include "NGMLogger.h"
#include "NGMBufferedPacket.h"
#include "NGMSystemConfiguration.h"
#include "NGMHit.h"
#include <iostream>
#include <sstream>
#include "TROOT.h"
#include "TThread.h"
#include "TBranch.h"
#include "TServerSocket.h"
#include "TSocket.h"
#include "TMessage.h"
#include "TObjString.h"
#include "TThread.h"
#include "TSystem.h"

ClassImp(NGMPacketBufferIO)

NGMPacketBufferIO::NGMPacketBufferIO(){
  _inputFile = 0;
  _inputBuffer = 0;
  _localBuffer = 0;
  _compressionlevel = 3;
  _maxfilesize = 1900000000LL;
  _basepath = "";
  _basepathvar = "";
  _numberOfSlots = 16;
  _numberOfHWChannels = 16*8;
  _channelsperSlot = _numberOfHWChannels/_numberOfSlots;
  _prevBufferIndex = -1;
  _spillCounter = 0;
  _sysConf = 0;
  _verbosity = 0;
}

NGMPacketBufferIO::NGMPacketBufferIO(const char* name, const char* title)
: NGMModule(name, title)
{
  _inputFile = 0;
  _inputBuffer = 0;
  _localBuffer = 0;
  _compressionlevel = 3;
  _maxfilesize = 1900000000LL;
  _basepath = "";
  _basepathvar = "";
  _numberOfSlots = 16;
  _numberOfHWChannels = 16*8;
  _channelsperSlot = _numberOfHWChannels/_numberOfSlots;
  _prevBufferIndex = -1;
  _spillCounter = 0;
  _sysConf = 0;
  _verbosity = 0;
}

NGMPacketBufferIO::~NGMPacketBufferIO(){
  //Clean up our own heap allocated objects
  if(_localBuffer){
    delete _localBuffer;
    _localBuffer = 0;
  }
  //When closing the input file
  // the associated input buffer is automatically deleted
  if(_inputFile){
    _inputFile->Close();
    _inputFile = 0;
    _localBuffer = 0;
  }
  if(_sysConf) delete _sysConf;
}

bool NGMPacketBufferIO::process(const TObject& tData){
  push(tData);
  return true;
}

int NGMPacketBufferIO::pushPacket(NGMBufferedPacket* packetBuffer){
  if(!packetBuffer) return -1;  
  return 0;
}

int NGMPacketBufferIO::openInputFile(const char* inputfilepath, bool readConfig){
  if(_inputFile){
    //Should Print Error message
    LOG<<" File already open "<<ENDM_WARN;
    return -1;
  }
   
   LOG<<"Attempting to open "<<inputfilepath<<ENDM_INFO;
   
  //TODO: Add lots of error checking here
  TThread::Lock();
  _inputFile = TFile::Open(inputfilepath);
  if(!_inputFile){
    LOG<<" Input file not found "<<inputfilepath<<ENDM_WARN;
    return -1;
  }
  _inputBuffer = (TTree*) (_inputFile->Get("pulse"));
  delete _localBuffer;
  _localBuffer = 0;
  if(!_localBuffer){
    if(TString(_inputBuffer->GetBranch("pulsebuffer")->GetClassName()) == "NGMBufferedPacketv5")
      _localBuffer = new NGMBufferedPacketv5(0,10000,TTimeStamp((time_t)0,0));
    else
      _localBuffer = new NGMBufferedPacketv2(0,10000,TTimeStamp((time_t)0,0));
  }
  if(_inputBuffer){
    _inputBuffer->SetBranchAddress("pulsebuffer",&_localBuffer);
  }else{
    LOG<<"Input Buffer not found"<<ENDM_WARN;
  }
  TThread::UnLock();
  
  NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*) (_inputFile->Get("NGMSystemConfiguration"));
  if(sysConf){
    LOG<<" Found Configuration "<<sysConf->GetName()<<"("<<sysConf->GetTitle()<<")"<<ENDM_INFO;

    _numberOfSlots = sysConf->GetSlotParameters()->GetEntries();
    _numberOfHWChannels = sysConf->GetChannelParameters()->GetEntries();
    _channelsperSlot = _numberOfHWChannels/_numberOfSlots;
    _prevBufferIndex = -1;
	
    if(readConfig) process(*((const TObject*) sysConf));
  }else{
    LOG<<" Configuration Object Not found "<<ENDM_WARN; 
  }
  _currEventNumber = 0;
  return 0;
}

int NGMPacketBufferIO::closeInputFile(){
  TThread::Lock();
  if(!_inputFile){
    //Should Print Error message
    LOG<<" No input file to close "<<ENDM_WARN;
    TThread::UnLock();
    return -1;
  }
  _inputFile->Close();
  delete _inputFile;
  _inputFile = 0;
  TThread::UnLock();
  return 0;
}  

bool NGMPacketBufferIO::FindNextFile()
{
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
      closeInputFile();
      openInputFile(newFileName.Data(),false);
      return true;
   }
   return false;
}

int NGMPacketBufferIO::readBuffer(Long64_t eventToRead)
{
  // verify the buffer
  if(!_inputBuffer) return 0;
  // attemp read from input buffer
  Long64_t bytesRead = _inputBuffer->GetEvent(eventToRead);
  setBytesRead(getBytesRead()+bytesRead);
  //check that the event was found and read
  if(bytesRead == 0) return 0;
  _currEventNumber = eventToRead +1;
  // pass the data to daughter tasks
  process(*((const TObject*) _localBuffer));
  return bytesRead;
}

int NGMPacketBufferIO::readNextBuffer()
{
  // verify the buffer
  if(!_inputBuffer) return 0;
  // attemp read from input buffer
  Long64_t bytesRead = _inputBuffer->GetEvent(_currEventNumber++);
  setBytesRead(getBytesRead()+bytesRead);
  //check that the event was found and read
  if(bytesRead == 0) return 0;
  
  // pass the data to daughter tasks
  Long64_t bufferIndex	= 0 ;
  if(_localBuffer->getChannelId() >=0 && _localBuffer->getSlotId() >=0){
	bufferIndex	= _localBuffer->getSlotId()*_channelsperSlot + _localBuffer->getChannelId() ;
  }else if(_localBuffer->getPulseCount() && _localBuffer->getSlotId()>=0){
	bufferIndex = _localBuffer->getHit(0)->GetSlot()*_channelsperSlot + _localBuffer->getHit(0)->GetChannel();
  }
  
  if(bufferIndex <= _prevBufferIndex && _prevBufferIndex >=0)
  {
	_spillCounter++;
	TObjString endSpillFlush("EndSpillFlush");
	process(*((const TObject*)&endSpillFlush));
  }
  
  _prevBufferIndex = bufferIndex;
  process(*((const TObject*) _localBuffer));
  return bytesRead;
}

Long64_t NGMPacketBufferIO::readNextSpill()
{
  // verify the buffer
  if(!_inputBuffer) return 0;
  // attemp read from input buffer
  Long64_t bufferIndex = 0;
  Long64_t bytesRead = 0;
  TStopwatch spillTimer;
  Long64_t eventsThisSpill = 0;
  spillTimer.Start();
  do {
	Long64_t thisBytesRead= _inputBuffer->GetEvent(_currEventNumber++);
	
   //Lets check for a susequent file
   if(thisBytesRead == 0)
      if(FindNextFile())
      {
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
   
	if(getDebug()) LOG<<"Read Buffer( "<<_currEventNumber-1<<" ) Channel( "<<bufferIndex<<" )"<<ENDM_INFO;
   
	if(bufferIndex <= _prevBufferIndex && _prevBufferIndex >=0)
	{
		_spillCounter++;
		_currEventNumber--;
	    _prevBufferIndex = -1;
		
		LOG<<"Spill Counter:"<<_spillCounter<<ENDM_INFO;

		TObjString endSpillFlush("EndSpillFlush");
		process(*((const TObject*)&endSpillFlush));
		break;
	}
	eventsThisSpill+=_localBuffer->getPulseCount();
	setBytesRead(getBytesRead()+thisBytesRead);

   
	// pass the data to daughter tasks
	process(*((const TObject*) _localBuffer));
    _prevBufferIndex = bufferIndex;
  }while(1);
  
  spillTimer.Stop();
  //if(getDebug())
  {
	LOG<<"Events: "<<eventsThisSpill<<" Bytes: "<<bytesRead
		<<" CPUTime: "<<spillTimer.CpuTime()<<" RealTime: "<<spillTimer.RealTime()
      <<" ("<<bytesRead/spillTimer.RealTime()<<") "
      <<ENDM_INFO;
  }
  return getBytesRead();
}


int NGMPacketBufferIO::run()
{
  Long64_t nevents = 0;
  while(readNextBuffer()){nevents++;}
  LOG<<"Read "<<getBytesRead()<< " in "<<nevents<<" packets"<<ENDM_INFO;
  return nevents;
}

ClassImp(NGMPacketOutputFile)

NGMPacketOutputFile::NGMPacketOutputFile(){
  _outputFile = 0;
  _outputBuffer = 0;
  _localBranch = 0;
  _passThrough = false;
  _spillsThisOutputFile = 0;
  _spillsPerFile = 0;
  _newFileNextSpill = false;
  _fileCounter = 0;
  _splitRuns = false;
  _timeOfLastSelfSave = 0.0;
  _selfSavePeriod = 1e9;
}

NGMPacketOutputFile::NGMPacketOutputFile(const char* name, const char* title)
: NGMPacketBufferIO(name, title)
{
  _outputFile = 0;
  _outputBuffer = 0;
  _localBranch = 0;
  _passThrough = false;
  _spillsThisOutputFile = 0;
  _spillsPerFile = 0;
  _newFileNextSpill = false;
  _fileCounter = 0;
  _splitRuns = false;
  _timeOfLastSelfSave = 0.0;
  _selfSavePeriod = 1e9;}

NGMPacketOutputFile::~NGMPacketOutputFile(){
  //When closing the output file
  // the associated output buffer is automatically deleted
  closeOutputFile();
}

int NGMPacketOutputFile::openOutputFile(const char* outputfilepath){

  if(_outputFile){
    LOG<<"Closing previous file "<<_outputFile->GetName()<<" and opening new file "<<outputfilepath<<ENDM_INFO;
	  closeOutputFile();
  }

  TThread::Lock();
  //TODO: Add lots of error checking here
  _outputFile = TFile::Open(outputfilepath,"RECREATE");
  _outputFile->SetCompressionLevel(getCompressionLevel());
  if(!_outputBuffer){
    _outputBuffer = new TTree("pulse","pulse");
    _outputBuffer->SetMaxTreeSize(2.0*getMaxFileSize());
    _outputBuffer->SetAutoSave(1000000000ULL);
  }
//  if(!_localBuffer){
//    // Would like to generate this by inspecting the data type
//    // of the output buffer...
//    _localBuffer = new NGMBufferedPacketv2(0,10000,TTimeStamp((time_t)0,0));
//  }
//  _outputBuffer->Branch("pulsebuffer","NGMBufferedPacketv2",&_localBuffer,1024*1024);
  setBytesWritten(0);
  TThread::UnLock();
  _newFileNextSpill = false;
  return 0;
  
}

int NGMPacketOutputFile::closeOutputFile(){
	//Check that output file has not changed like when a TTree opens a new file
  TThread::Lock();
  if(_outputBuffer)
	  _outputFile = _outputBuffer->GetCurrentFile();
  if(_outputFile){
    //LOG<<"Closing output file "<<_outputFile->GetName()<<ENDM_INFO;
	TObjString* sFileClosedKey = new TObjString("FileClosed");
  _outputBuffer->FlushBaskets();
	_outputFile->Write();
	_outputFile->WriteTObject(sFileClosedKey,"FileClosed");
	_outputFile->Write();
    _outputFile->Close();
    delete sFileClosedKey;
    _outputFile = 0;
    _outputBuffer = 0;
  }
  delete _localBuffer;
  _localBuffer = 0;

  // Do not delete or zero out _localBuffer
  TThread::UnLock();
  return 0;
}

bool NGMPacketOutputFile::process(const TObject & tData)
{
  
  static TClass* tNGMBufferedPacketType = gROOT->GetClass("NGMBufferedPacket");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  static TClass* tObjStringType = gROOT->GetClass("TObjString");

  //Check data type
  if(tData.InheritsFrom(tNGMBufferedPacketType)){
  
    const NGMBufferedPacket* packetBuffer = (const NGMBufferedPacket*)(&tData);
  
	if(!_localBuffer){
		// Would like to generate this by inspecting the data type
		// of the output buffer...
		if(TString(packetBuffer->IsA()->GetName()) == "NGMBufferedPacketv5")
			_localBuffer = new NGMBufferedPacketv5(0,10000,TTimeStamp((time_t)0,0));
		else
			_localBuffer = new NGMBufferedPacketv2(0,10000,TTimeStamp((time_t)0,0));

		_outputBuffer->Branch("pulsebuffer",_localBuffer->IsA()->GetName(),&_localBuffer,320000,0);
	}
    
	// Copy to our local buffer to be written to the output tree
	//_localBranch->SetAddress(&tData);
	if(_localBuffer && _localBuffer != packetBuffer){
      _localBuffer->CopyPacket(packetBuffer);
    }
	

    //Now write to the outputfile
    if(_outputBuffer){
			  
	  // Check if we want a new file before the next buffer
	  if(_newFileNextSpill)
	  {
      if(_splitRuns)
      {
        if(packetBuffer->getPulseCount()<=0)
        {
          LOG<<"Zero Length buffer at end of fill"<<ENDM_FATAL;
          throw 1;
        }
        _sysConf->SetTimeStamp(packetBuffer->getHit(0)->GetNGMTime());
        long long trunNumber = _sysConf->GetTimeStamp().GetDate();
        trunNumber*=1000000L;
        trunNumber+=_sysConf->GetTimeStamp().GetTime();
        _sysConf->setRunNumber(trunNumber);
      }
      
      
	    // This method was protected until root version 5.17, for now we do it ourselves.
		// _outputFile  = _outputBuffer->ChangeFile(_outputBuffer->GetCurrentFile());
		_fileCounter++;
		_outputFile = _outputBuffer->GetCurrentFile();
		TString oldFileName = _outputFile->GetName();
		TString newFileName;
    if(!_splitRuns){
		// Find and strip "underscore filecount" from previous name
        if(_fileCounter>1)
        {
          int stripLoc = oldFileName.Index("_",1,oldFileName.Length()-10,TString::kExact);
          newFileName= oldFileName(0,stripLoc);
        }else{
          int stripLoc = oldFileName.Length() - 5;
          newFileName=oldFileName(0,stripLoc);
        }
        newFileName+="_";
        newFileName+=_fileCounter;
        newFileName+=".root";
      }else{
        newFileName = _basepath;
        if(_basepathvar!="")
          if(_sysConf->GetSystemParameters()->GetParIndex(_basepathvar)>=0)
            newFileName = _sysConf->GetSystemParameters()->GetParValueS(_basepathvar,0);
        newFileName+=GetName();
        newFileName+= _sysConf->getRunNumber();
        newFileName+= "-ngm.root";
        
      }
		openOutputFile(newFileName.Data());
      
    // Need to reallocate the _localbuffer based on the packetBuffer Type
    if(!_localBuffer){
        // Would like to generate this by inspecting the data type
        // of the output buffer...
        if(TString(packetBuffer->IsA()->GetName()) == "NGMBufferedPacketv5")
          _localBuffer = new NGMBufferedPacketv5(0,10000,TTimeStamp((time_t)0,0));
        else
          _localBuffer = new NGMBufferedPacketv2(0,10000,TTimeStamp((time_t)0,0));
        
        _outputBuffer->Branch("pulsebuffer",_localBuffer->IsA()->GetName(),&_localBuffer,320000,0);

      if(_localBuffer && _localBuffer != packetBuffer){
        _localBuffer->CopyPacket(packetBuffer);
      }
      
    }
      
    // Save configuration header  
    if(_outputFile && _sysConf)
    {
      _outputFile->WriteTObject(_sysConf,"NGMSystemConfiguration");
      _outputFile->Write();
    }
      
		if(!_outputFile){
			LOG<<"Unable to open new outputfile on file change request"<<ENDM_FATAL;
		}
		_newFileNextSpill = false;
		_spillsThisOutputFile = 0;
	  }
      
	  _outputBuffer->Fill();
	  // This may be necessary for small packets

	  //_outputBuffer->FlushBaskets();
    _localBuffer->Clear();
    setBytesWritten(_outputBuffer->GetTotBytes());
    //Check if we need to save the tree for another process that might
    // be reading the open file
    TTimeStamp ts;
    if((ts.AsDouble()-_timeOfLastSelfSave)>_selfSavePeriod)
    {
      if(_outputFile)
        LOG<<"AutoSaving Output stream "<<_outputBuffer->GetCurrentFile()->GetName()<<ENDM_INFO;
      if(_outputBuffer) _outputBuffer->AutoSave("SaveSelf");
      _timeOfLastSelfSave=ts.AsDouble();
    }
    }
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
    if(_sysConf) delete _sysConf;
    _sysConf = new NGMSystemConfigurationv1(confBuffer->GetName(),1,1);
    _sysConf->CopyConfiguration(confBuffer);
    _timeOfLastSelfSave = 0.0;
    if(_splitRuns)
    {
      // Add 1 second to runtime so that runnumbers are unique
      _sysConf->setRunNumber(_sysConf->getRunNumber()+1);
    }
    TString outputPath = _basepath;
    if(_basepathvar!="")
      if(_sysConf->GetSystemParameters()->GetParIndex(_basepathvar)>=0)
        outputPath = _sysConf->GetSystemParameters()->GetParValueS(_basepathvar,0);
    TString filename = outputPath;
    filename+=GetName();
    filename+= _sysConf->getRunNumber();
    filename+= "-ngm.root";
    openOutputFile(filename);    
    if(_outputFile){
      _outputFile->WriteTObject(_sysConf,"NGMSystemConfiguration");
    }
    _spillCounter = 0;
    _spillsThisOutputFile = 0;
    _fileCounter = 0;
    _newFileNextSpill = false;	
    
    if(!_localBuffer){
      // Would like to generate this by inspecting the data type
      // of the output buffer...
      LOG<<"Assuming NGMBufferedPacketv2 NGMHitv6 "<<ENDM_WARN;
      _localBuffer = new NGMBufferedPacketv2(0,10000,TTimeStamp((time_t)0,0));
      _outputBuffer->Branch("pulsebuffer",_localBuffer->IsA()->GetName(),&_localBuffer,320000,0);
      
    }
    //Update the file with the keys created above
    if(_outputFile) _outputFile->Write();
    
  }else if(tData.IsA() == tObjStringType){
    const TObjString* controlMessage = (const TObjString*)(&tData);
    if(controlMessage->GetString() == "PlotUpdate")
    {
      if(_outputBuffer && _outputBuffer->GetCurrentFile())
        LOG<<"AutoSaving Output stream "<<_outputBuffer->GetCurrentFile()->GetName()<<ENDM_INFO;
      if(_outputBuffer) _outputBuffer->AutoSave("SaveSelf");
    }
    if(controlMessage->GetString() == "EndRunSave")
    {
      closeOutputFile();
    }
    if(controlMessage->GetString() == "EndSpillFlush")
    {
	  // This is the end of this spill
	  // increment spill counter...
	  _spillCounter++;
    _spillsThisOutputFile++;
	  // Check if we want to create a new file for next spill
	  if(_spillsThisOutputFile>=_spillsPerFile && _spillsPerFile > 0)
	  {
      _newFileNextSpill = true;
	  }
	
	  if(getDebug()) LOG<<"Spill Counter:"<<_spillCounter<<" NewFileNextSpill:"<<_newFileNextSpill<<ENDM_INFO;

    }

  }

  push(tData);

  return true;
}


ClassImp(NGMPacketFilter)

NGMPacketFilter::NGMPacketFilter()
{

  _numskips = 0;
  
  for(int iskip =0; iskip < maxskips; iskip++)
  {
    _slotsToSkip[iskip] = 0;
    _channelsToSkip[iskip] = 0;
  }

}

NGMPacketFilter::NGMPacketFilter(const char* name, const char* title)
:  NGMModule(name,title)
{

  _numskips = 0;
  
  for(int iskip =0; iskip < maxskips; iskip++)
  {
    _slotsToSkip[iskip] = 0;
    _channelsToSkip[iskip] = 0;
  }

}

NGMPacketFilter::~NGMPacketFilter()
{
}

bool NGMPacketFilter::addSkipChannel(int slotid, int channelid)
{

  if(! (_numskips < maxskips ) ) return false;
  
  //If channelid == -1 then we'll skips all channels with slotid

  _slotsToSkip[_numskips] = slotid;
  _channelsToSkip[_numskips] = channelid;
  _numskips++;

  return true;
}

bool NGMPacketFilter::process(const TObject & tData)
{
  
  static TClass* tNGMBufferedPacketType = gROOT->GetClass("NGMBufferedPacket");
  //unused:  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  
  //Check data type
  if(tData.InheritsFrom(tNGMBufferedPacketType)){
  
    const NGMBufferedPacket* packetBuffer = (const NGMBufferedPacket*)(&tData);
    // Skip if first hit is in the skip list
    bool writeData = true;
    if(packetBuffer->getPulseCount()>0)
    {
      int tSlot = packetBuffer->getHit(0)->GetSlot();
      int tChan = packetBuffer->getHit(0)->GetChannel();
      for(int iskip = 0; iskip < _numskips; iskip++)
      {
	if(tSlot==_slotsToSkip[iskip]
	   && ( tChan == _channelsToSkip[iskip]
		|| _channelsToSkip[iskip] ==  -1)
	   )
	{
	  if(getDebug()) LOG<<"Skipping Slot("<< tSlot << ") Chan(" << tChan <<") "<<ENDM_INFO;
	  writeData = false;
	  break;
	}
      }
	}else{
		writeData = false;
	}

    if(writeData)
    {
      push(tData);
    }
    return true;
  }

  push(tData);
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// NGMPacketSocket Input /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

ClassImp(NGMPacketSocketInput)

NGMPacketSocketInput::NGMPacketSocketInput()
{
  serverSocket = 0;
  signalServerSocket = 0;
  socket = 0;
  signalSocket = 0;
  _exitLoop = false;
  _receiveThread = 0;
}

NGMPacketSocketInput::NGMPacketSocketInput(const char* name, const char* title)
:NGMPacketBufferIO(name,title)
{
  serverSocket = 0;
  signalServerSocket = 0;
  socket = 0;
  signalSocket = 0;
  _exitLoop =false;
  _receiveThread = 0;
}

NGMPacketSocketInput::~NGMPacketSocketInput()
{
  
}

bool NGMPacketSocketInput::init()
{
  _exitLoop =false;
  return true;
}

bool NGMPacketSocketInput::process(const TObject &tData)
{
  static TClass* tNGMBufferedPacketType = gROOT->GetClass("NGMBufferedPacket");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  //unused:  static TClass* tObjStringType = gROOT->GetClass("TObjString");

  if(!socket) return false;
  if(!socket->IsValid()) return false;

  push(tData);
  
  //Check data type
  if(tData.InheritsFrom(tNGMBufferedPacketType)){
    
    const NGMBufferedPacket* packetBuffer = (const NGMBufferedPacket*)(&tData);
    if(getDebug()) LOG<<" Data received "<<packetBuffer->getPulseCount()<<ENDM_INFO;
    //Now write to the outputfile
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    
  //unused:    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
    
    return true;
  }
  
  return true;
}

bool NGMPacketSocketInput::finish()
{
  return true;
}

void NGMPacketSocketInput::setExitLoop(bool newVal)
{
  _exitLoop = newVal;
}

bool NGMPacketSocketInput::ReceiveLoop()
{
  int timeout = 6000000; //Inital timeout of 10s
  bool nodata = false;

  while(socket->Select(TSocket::kRead,timeout)==1 && ! nodata &&  !_exitLoop)
  {
    
    TMessage* message;
    socket->Recv(message);
    // if message is empty and we had TSocket::Select returned 1
    // it appears the socket has been closed on the server side...
    if(message){
      const TObject* rBuffer = (const TObject*) message->ReadObject(message->GetClass());
      delete message;
      if(rBuffer)
      {
        process(*rBuffer);
        delete rBuffer;
      }
    }else{
      nodata = true;
    }    
    //gSystem->ProcessEvents();

  }
  return true;
}

void* NGMPacketSocketInput::ReceiveLoopThread(void* arg)
{
  NGMPacketSocketInput* tPacketSocketInput = (NGMPacketSocketInput*) arg;
  tPacketSocketInput->ReceiveLoop();
  return 0;
}

void NGMPacketSocketInput::LaunchReceiveLoopThread()
{
  if(!_receiveThread)
    _receiveThread = new TThread("memberfunction",(void(*)(void *)) &ReceiveLoopThread, (void*) this);
  _receiveThread->Run();
  return;
}

int NGMPacketSocketInput::openSocket()
{

  const int socketOffset = 9090;
  const int socketSignalOffset = 7070;
  //First lets create a server socket connection
  // Needs Error and time out checks
  if(serverSocket)
  {
    delete serverSocket;
    if(socket)
      delete socket;
  }
  if(signalServerSocket)
  {
    delete signalServerSocket;
    if(signalSocket)
      delete signalSocket;
  }
  serverSocket = new TServerSocket(socketOffset, kTRUE);
  std::cout<<"Waiting on Data Socket Accept\n";
  socket = serverSocket->Accept();
  signalServerSocket = new TServerSocket(socketSignalOffset, kTRUE);
  std::cout<<"Waiting on Signal Socket Accept\n";  
  signalSocket = signalServerSocket->Accept();
  std::cout<<"All Sockets Accepted\n";  
  
//  //Check for signal to start
//  std::cout<<"Check for start signal timeout (20s)\n";
//  if(signalSocket->Select(TSocket::kRead,20000)){
//    std::cout<<"Received start signal.\n";        
//    TMessage* smessage;
//    signalSocket->Recv(smessage);
//    if(smessage){
//      if(smessage->GetClass()){
//        TString sObjString("TObjString");
//        TString schkStop("Start");
//        std::cout<<smessage->GetClass()->GetName()<<std::endl;
//        if(sObjString == smessage->GetClass()->GetName()){
//          TObjString* signalOString = (TObjString*)(smessage->ReadObject(smessage->GetClass()));
//          std::cout<<signalOString->GetString().Data()<<std::endl;
//        }
//      }
//    }          
//  }else{
//    std::cout<<"No signal detected - aborting...\n";
//    serverSocket->Close();
//    signalServerSocket->Close();
//    return -1;
//  }
//  std::cout<<"Finished Check for start signal\n";
  

  
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// NGMPacketSocket Output /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

ClassImp(NGMPacketSocketOutput)

NGMPacketSocketOutput::NGMPacketSocketOutput()
{
  socket = 0;
  signalSocket = 0;  
}

NGMPacketSocketOutput::NGMPacketSocketOutput(const char* name, const char* title)
:NGMPacketBufferIO(name,title)
{
  socket = 0;
  signalSocket = 0;  
}

NGMPacketSocketOutput::~NGMPacketSocketOutput()
{
  
}

bool NGMPacketSocketOutput::init()
{
  return true;
}

bool NGMPacketSocketOutput::process(const TObject &tData)
{

  static TClass* tObjStringType = gROOT->GetClass("TObjString");

  if(!socket) return false;
  if(!socket->IsValid()) return false;

  //if(TString(tData.GetName())=="NGMBufferedPacketv2")
  //{
  //  const NGMBufferedPacket* packet = (const NGMBufferedPacket*)(&tData);
  //  LOG<<"Sending "<<packet->getHit(0)->GetSlot()<<" "<<packet->getHit(0)->GetChannel()<<ENDM_INFO;
  //}
  
  // Do not compress the text message objects.
  if(tObjStringType == tData.IsA()){
    TMessage dmessage(kMESS_OBJECT);
    dmessage.WriteObject(&tData);
    socket->Send(dmessage);
  }else{
    TMessage dmessage(kMESS_OBJECT/*|kMESS_ZIP*/);
    dmessage.WriteObject(&tData);
    socket->Send(dmessage);
  }
  return true;
}

bool NGMPacketSocketOutput::finish()
{
  return true;
}

int NGMPacketSocketOutput::openSocket(const char* hostname)
{
  
  const int socketOffset = 9090;
  const int socketSignalOffset = 7070;
  const int maxConnectRetries = 5;
  int connectRetries = 0;
  socket = 0;
  while(!socket){
    
    if(maxConnectRetries <= connectRetries){
      std::cout<<"Data socket create failed after "<<connectRetries<<" attempts\n";
      return -1;
    }
    
    gSystem->Sleep(1000);
    
    socket = new TSocket(hostname,socketOffset);
    //socket->SetCompressionLevel(1);
    connectRetries++;
    if(! socket->IsValid()){
      delete socket;
      socket=0;
    }
  }
  
  std::cout<<"Data Socket created\n";
  
  signalSocket = 0;
  connectRetries = 0;
  
  while( !signalSocket){
    if(maxConnectRetries <= connectRetries){
      std::cout<<"Signal socket create failed after "<<connectRetries<<" attempts\n";
      return -2;
    }
    
    gSystem->Sleep(1000);
    
    signalSocket = new TSocket(hostname,socketSignalOffset); 
    connectRetries++;
    
    if( ! signalSocket->IsValid() ){
      delete signalSocket;
      signalSocket = 0;
    }
  }
  std::cout<<"Signal Socket created\n";
  
  gSystem->Sleep(1000);
  
  return 0;
}
