#include "NGMHitIO.h"

#include "TFile.h"
#include "TTree.h"
#include "NGMLogger.h"
#include "NGMHit.h"
#include "NGMSystemConfiguration.h"
#include "NGMBufferedPacket.h"
#include <iostream>
#include "TROOT.h"
#include "TThread.h"
#include "TList.h"
#include <cmath>
#include "TObjString.h"
#include "NGMParticleIdent.h"
#include "NGMSimpleParticleIdent.h"
#include "TMath.h"
#include "TObjArray.h"
//#include "NGMBlockDetectorCalibrator.h"
#include "NGMBaseline.h"
#include "sis3316card.h"
#include <limits>

////////////////////////////////////////////////////////////
//////////// NGMHitIO Implementation         ///////////////
////////////////////////////////////////////////////////////

ClassImp(NGMHitIO)

NGMHitIO::NGMHitIO(){
  _inputFile = 0;
  _inputBuffer = 0;
  _localBuffer = 0;
  _verbosity = 0;
}

NGMHitIO::NGMHitIO(const char* name, const char* title)
: NGMModule(name, title)
{
  _inputFile = 0;
  _inputBuffer = 0;
  _localBuffer = 0;
  _verbosity = 0;
}

NGMHitIO::~NGMHitIO(){
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
}

bool NGMHitIO::process(const TObject& tData){
  push(tData);
  return true;
}

int NGMHitIO::pushHit(NGMHit* hitBuffer){
  if(!hitBuffer) return -1;  
  return 0;
}

int NGMHitIO::openInputFile(const char* inputfilepath){
  if(_inputFile){
    //Should Print Error message
    LOG<<" File already open "<<ENDM_WARN;
    return -1;
  }
  //TODO: Add lots of error checking here
  _inputFile = TFile::Open(inputfilepath);
  if(!_inputFile){
    LOG<<" Input file not found "<<inputfilepath<<ENDM_WARN;
    return -1;
  }
  _inputBuffer = (TTree*) (_inputFile->Get("HitTree"));
  if(!_localBuffer){
    TString hitClassVerion(_inputBuffer->GetBranch("HitTree")->GetClassName());
      if(hitClassVerion == "NGMHitv4")
      _localBuffer = new NGMHitv4;
   else if(hitClassVerion == "NGMHitv5")
     _localBuffer = new NGMHitv5;
   else if(hitClassVerion == "NGMHitv6")
     _localBuffer = new NGMHitv6;
   else 
      _localBuffer = new NGMHitv8;

  }


  if(_inputBuffer){
    _inputBuffer->SetBranchAddress("HitTree",&_localBuffer);
  }else{
    LOG<<"Input Buffer not found"<<ENDM_WARN;
  }
  
  NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*) (_inputFile->Get("NGMSystemConfiguration"));
  if(sysConf){
    LOG<<" Found Configuration "<<sysConf->GetName()<<"("<<sysConf->GetTitle()<<")"<<ENDM_INFO;
    process(*((const TObject*) sysConf));
  }else{
    LOG<<" Configuration Object Not found "<<ENDM_WARN; 
  }
  _currEventNumber = 0;
  return 0;
}

int NGMHitIO::closeInputFile(){
  if(!_inputFile){
    //Should Print Error message
    LOG<<" No input file to close "<<ENDM_WARN;
    return -1;
  }
  _inputFile->Close();
  delete _inputFile;
  _inputFile = 0;
  delete _localBuffer;
  _localBuffer = 0;
  return 0;
}  

int NGMHitIO::readBuffer(Long64_t eventToRead)
{
  // verify the buffer
  if(!_inputBuffer) return 0;
  // attemp read from input buffer
  int bytesRead = _inputBuffer->GetEvent(eventToRead);
  //check that the event was found and read
  if(bytesRead == 0) return 0;
  _currEventNumber = eventToRead +1;
  // pass the data to daughter tasks
  process(*((const TObject*) _localBuffer));
  return bytesRead;
}

int NGMHitIO::readNextBuffer()
{
  // verify the buffer
  if(!_inputBuffer) return 0;
  // attemp read from input buffer
  int bytesRead = _inputBuffer->GetEvent(_currEventNumber++);
  //check that the event was found and read
  if(bytesRead == 0) return 0;
  // pass the data to daughter tasks
  process(*((const TObject*) _localBuffer));
  return bytesRead;
}

int NGMHitIO::readNextSpill()
{
  return readNextBuffer();
}

int NGMHitIO::run()
{
  Long64_t nevents = 0;
  Long64_t ntotal = _inputBuffer->GetEntriesFast();
  while(readNextBuffer()){
    nevents++;
    if((nevents%(ntotal/10))==0)
    {
      LOG<<"Read "<<nevents<<" out of "<< ntotal<<ENDM_INFO;
    }
  }
  return nevents;
}

////////////////////////////////////////////////////////////
//////////// NGMHitOutputFile Implementation ///////////////
////////////////////////////////////////////////////////////

ClassImp(NGMHitOutputFile)

NGMHitOutputFile::NGMHitOutputFile(){
  _outputFile = 0;
  _outputBuffer = 0;
  _basepath = "";
  _basepathvar = "";//"RawOutputPath";
}

NGMHitOutputFile::NGMHitOutputFile(const char* name, const char* title)
: NGMHitIO(name, title)
{
  _outputFile = 0;
  _outputBuffer = 0;
  _basepath = "";
  _basepathvar = "";//"RawOutputPath";
  partID = new NGMSimpleParticleIdent;

}

NGMHitOutputFile::~NGMHitOutputFile(){
  //When closing the output file
  // the associated output buffer is automatically deleted
  if(_outputFile){
    _outputFile->Close();
    delete _outputFile;
    _outputFile = 0;
  }
  _outputBuffer = 0;
}

void NGMHitOutputFile::setBasePath(const char* basepath)
{
  _basepath = basepath;
}

void NGMHitOutputFile::setBasePathVariable(const char* basepathvar)
{
  _basepathvar = basepathvar;
  
}


int NGMHitOutputFile::openOutputFile(const char* outputfilepath){
  if(_outputFile){
    LOG<<"Closing previous file "<<_outputFile->GetName()<<" and opening new file "<<outputfilepath<<ENDM_INFO;
    closeOutputFile();
  }
  
  //TODO: Add lots of error checking here
  TThread::Lock();
  _outputFile = TFile::Open(outputfilepath,"RECREATE");
  if(!_localBuffer){
    // Would like to generate this by inspecting the data type
    // of the output buffer...
    _localBuffer = new NGMHitv8;
  }
  if(!_outputBuffer){
    _outputBuffer = new TTree("HitTree","HitTree");
    _outputBuffer->SetMaxTreeSize(40000000000LL);
  }
  //_outputBuffer->Branch("HitTree","NGMHitv6",&_localBuffer,3200000,99);
  _outputBuffer->Branch("HitTree",&_localBuffer);
  TThread::UnLock();
  return 0;
  
}

int NGMHitOutputFile::closeOutputFile(){
  TThread::Lock();
  if(_outputBuffer)
  {
    _outputFile = _outputBuffer->GetCurrentFile();
    if(_outputFile)
    {
      LOG<<"Closing output file "<<_outputFile->GetName()<<ENDM_INFO;
      _outputFile->Write();
      _outputFile->Close();
      _outputFile = 0;
      _outputBuffer = 0;
    }
  }
  TThread::UnLock();
  // Do not delete or zero out _localBuffer
  return 0;
}

bool NGMHitOutputFile::process(const TObject & tData)
{
  
  static TClass* tNGMHitType = gROOT->GetClass("NGMHit");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  static TClass* tObjStringType = gROOT->GetClass("TObjString");
  
  //Check data type
  if(tData.InheritsFrom(tNGMHitType)){
  
    const NGMHit* hitBuffer = (const NGMHit*)(&tData);
    if(partID->IsSelected(hitBuffer))
    {
      // Copy to our local buffer to be written to the output tree
      if(_localBuffer){
        _localBuffer->CopyHit(hitBuffer);
      }
      //Now wrtie to the outputfile
      if(_outputBuffer){
        _outputBuffer->Fill();
      }
    }
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
    // Initialize particle identifier
    partID->Init(confBuffer);

    TString outputPath = _basepath;
    if(_basepathvar!="")
      if(confBuffer->GetSystemParameters()->GetParIndex(_basepathvar)>=0)
        outputPath = confBuffer->GetSystemParameters()->GetParValueS(_basepathvar,0);
    TString filename = outputPath;
    filename+=GetName();
    //filename+= confBuffer->getRunNumber();
    filename+= "-ngm.root";
    openOutputFile(filename);
    if(_outputFile){
      _outputFile->WriteTObject(confBuffer,"NGMSystemConfiguration");
    }
  }else if(tData.IsA() == tObjStringType){
    const TObjString* controlMessage = (const TObjString*)(&tData);
    
    if(getVerbosity()>10) LOG<<"Sending Message "<<controlMessage->GetString().Data()<<ENDM_INFO;

    if(controlMessage->GetString() == "EndRunSave")
    {
      closeOutputFile();
    }
  }

    
  push(tData);

  return true;
}

////////////////////////////////////////////////////////////
//////////// NGMHit Serialize Implementation ///////////////
////////////////////////////////////////////////////////////

ClassImp(NGMHitSerialize)

NGMHitSerialize::NGMHitSerialize(){
  _verbosity = 0;
}

NGMHitSerialize::NGMHitSerialize(const char* name, const char* title)
: NGMModule(name,title)
{
  _verbosity = 0;
}

NGMHitSerialize::~NGMHitSerialize(){
}

bool NGMHitSerialize::process(const TObject & tData)
{
  // Timer needs false argument to prevent reset
  _timer.Start(kFALSE);
  
  static TClass* tNGMBufferedPacketType = gROOT->GetClass("NGMBufferedPacket");  
  //unused:  static TClass* tNGMHitType = gROOT->GetClass("NGMHit");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  
  //Check data type
  if(tData.InheritsFrom(tNGMBufferedPacketType)){
    
    const NGMBufferedPacket* packetBuffer = (const NGMBufferedPacket*)(&tData);
    int pulseCount = packetBuffer->getPulseCount();
    for(int ihit = 0; ihit < pulseCount; ihit++){
      const NGMHit* tHit = packetBuffer->getHit(ihit);
      if(tHit)
        push(*tHit);
    }
    // Dont pass a packet to daughter modules
    return true;
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
  
  }
  // Pass everything but packets to daughter modules.
  push(tData);
  return true;
}

////////////////////////////////////////////////////////////
//////////// NGMHit RandomIO Implementation ///////////////
////////////////////////////////////////////////////////////

ClassImp(NGMHitRandomIO)

NGMHitRandomIO::NGMHitRandomIO(){
  _inputFile = 0;
  _inputBuffer = 0;
  _localBuffer = 0;
  _hitList = 0;
  _countwidth = 200;
  _currEventNumber = 0;
}

NGMHitRandomIO::NGMHitRandomIO(const char* name, const char* title)
: NGMModule(name, title)
{
  _inputFile = 0;
  _inputBuffer = 0;
  _localBuffer = 0;
  _hitList = 0;
  _countwidth = 200;
  _currEventNumber = 0;
}

NGMHitRandomIO::~NGMHitRandomIO()
{
}

Long64_t NGMHitRandomIO::getMaxEvents() const
{
  if(_inputBuffer) return _inputBuffer->GetEntries();
  return 0;
}

bool NGMHitRandomIO::process(const TObject& tData){
  push(tData);
  return true;
}

int NGMHitRandomIO::openInputFile(const char* inputfilepath){
  if(_inputFile){
    //Should Print Error message
    LOG<<" File already open "<<ENDM_WARN;
    return -1;
  }
  //TODO: Add lots of error checking here
  _inputFile = TFile::Open(inputfilepath);
  if(!_inputFile){
    LOG<<" Input file not found "<<inputfilepath<<ENDM_WARN;
    return -1;
  }
  _inputBuffer = (TTree*) (_inputFile->Get("HitTree"));
  
  TString hitClassVersion(_inputBuffer->GetBranch("HitTree")->GetClassName());
  delete _localBuffer;
  _localBuffer = 0;

  if(!_localBuffer){
    if(hitClassVersion == "NGMHitv5")
      _localBuffer = new NGMHitv5;
    else if(hitClassVersion == "NGMHitv6")
      _localBuffer = new NGMHitv6;
  }
  if(_inputBuffer){
    _inputBuffer->SetBranchAddress("HitTree",&_localBuffer);
  }else{
    LOG<<"Input Buffer not found"<<ENDM_WARN;
  }
  
  if(!_hitList)
  {
    _hitList = new TList;
  }
  
  NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*) (_inputFile->Get("NGMSystemConfiguration"));
  if(sysConf){
    LOG<<" Found Configuration "<<sysConf->GetName()<<"("<<sysConf->GetTitle()<<")"<<ENDM_INFO;
    process(*((const TObject*) sysConf));
  }else{
    LOG<<" Configuration Object Not found "<<ENDM_WARN; 
  }
  _currEventNumber = 0;
  return 0;
}

int NGMHitRandomIO::closeInputFile(){
  if(!_inputFile){
    //Should Print Error message
    LOG<<" No input file to close "<<ENDM_WARN;
    return -1;
  }
  _inputFile->Close();
  delete _inputFile;
  _inputFile = 0;
  return 0;
}  

int NGMHitRandomIO::readBuffer(Long64_t eventToRead, Long64_t eventlength)
{
  // verify the buffer
  if(!_inputBuffer) return 0;
  _currEventNumber = eventToRead;
  Long64_t totalEvents = _inputBuffer->GetEntries();
  if(eventToRead >= totalEvents) return 0;
  if(totalEvents - eventToRead < eventlength)
    eventlength = totalEvents - eventToRead;
  
  // Fill the local hit list
  _hitList->Delete();
  int bytesRead = 0;
  int bytesThisHit = 0;
  for(Long64_t ihit = eventToRead; ihit < eventToRead + eventlength; ihit++)
  {
    // attemp read from input buffer
    bytesThisHit = _inputBuffer->GetEvent(ihit);
    //check that the event was found and read
    if(bytesThisHit == 0) return 0;
    bytesRead += bytesThisHit;
    _hitList->AddLast(_localBuffer->DuplicateHit());
  }
  
  // pass the data to daughter tasks
  process(*((const TObject*) _hitList));
  
  return bytesRead;
}

Long64_t NGMHitRandomIO::readBuffer(Long64_t eventToRead, double timeWindowNanoSeconds)
{
    // verify the buffer
  if(!_inputBuffer) return 0;
  _currEventNumber = eventToRead;
  Long64_t totalEvents = _inputBuffer->GetEntries();
  if(eventToRead >= totalEvents) return 0;
  
  // Fill the local hit list
  _hitList->Delete();
  int bytesRead = 0;
  int bytesThisHit = 0;
  bytesRead = _inputBuffer->GetEvent(_currEventNumber);
  if(bytesThisHit == 0) return -1;
  NGMTimeStamp firstTimeInSequence = _localBuffer->GetNGMTime();
  _hitList->AddLast(_localBuffer->DuplicateHit());

  while(bytesThisHit && _currEventNumber < totalEvents)
  {
    // attemp read from input buffer
    bytesThisHit = _inputBuffer->GetEvent(_currEventNumber);
    //check that the event was found and read
    if(bytesThisHit == 0) return 0;
    bytesRead += bytesThisHit;
    if(fabs(_localBuffer->TimeDiffNanoSec(firstTimeInSequence)) > timeWindowNanoSeconds)
    {
      break;
    }
    _hitList->AddLast(_localBuffer->DuplicateHit());
    _currEventNumber++;
  }
  
  // pass the data to daughter tasks
  process(*((const TObject*) _hitList));
  
  return _currEventNumber;

}


int NGMHitRandomIO::findNextTrigger(int nevents, double timeInNanoSec)
{
  // verify the buffer
  if(!_inputBuffer) return 0;
  if(_currEventNumber < 0) _currEventNumber = 0;
  Long64_t totalEvents = _inputBuffer->GetEntries();  
  
  bool eventfound = false;
  
  for(; _currEventNumber< totalEvents - nevents; _currEventNumber++)
  {
    if(totalEvents - _currEventNumber < nevents)
    {
      LOG<<"Reached End of Buffer"<<ENDM_INFO;
      return 0;
    }
    _inputBuffer->GetEvent(_currEventNumber);
    NGMHit* tHit = _localBuffer->DuplicateHit();
    _inputBuffer->GetEvent(_currEventNumber+nevents);
    if(_localBuffer->TimeDiffNanoSec(tHit->GetNGMTime()) < timeInNanoSec)
    {
      eventfound = true;
    }
    delete tHit;
    if(eventfound) break;
  }
  
  if(!eventfound)
  {
    LOG<<"Event not found "<<ENDM_INFO;
    return 0;
  }
  
  // Fill the local hit list
  _hitList->Delete();
  int bytesRead = 0;
  int bytesThisHit = 0;

  int neventsToRead = _countwidth;
  if(neventsToRead > totalEvents - _currEventNumber)
    neventsToRead = totalEvents - _currEventNumber -1;
  
  for(Long64_t ihit = _currEventNumber; ihit < _currEventNumber + neventsToRead; ihit++)
  {
    // attemp read from input buffer
    bytesThisHit = _inputBuffer->GetEvent(ihit);
    //check that the event was found and read
    if(bytesThisHit == 0) return 0;
    bytesRead += bytesThisHit;
    _hitList->AddLast(_localBuffer->DuplicateHit());
  }
  
  // pass the data to daughter tasks
  process(*((const TObject*) _hitList));
  
  return _currEventNumber;
}

ClassImp(NGMHitFilter)

NGMHitFilter::NGMHitFilter()
{
    _blockPMTBaselines=0;
}

NGMHitFilter::NGMHitFilter(const char* name, const char* title)
:NGMModule(name, title), _pileup(1000),_baselinelow(1000), _baselinehigh(1000), _energylow(1000), _energyhigh(1000), _neutroncutlow(1000), _neutroncuthigh(1000),_gammacutlow(1000), _gammacuthigh(1000), _accept(1000), _gatelength(1000,8),_padclow(1600),_padchigh(1600),_baselinecut(1000){
  partID = new NGMSimpleParticleIdent;
    _blockPMTBaselines = 0;
  for (int i = 0; i < _accept.GetSize(); i++) {
      _accept[i]=1;
      _pileup[i] = 0;
      _baselinelow[i]=-std::numeric_limits<float>::max();
      _baselinehigh[i]=std::numeric_limits<float>::max();
      _energylow[i]=-std::numeric_limits<float>::max();
      _energyhigh[i]=std::numeric_limits<float>::max();
      _neutroncutlow[i]=-std::numeric_limits<float>::max();
      _neutroncuthigh[i]=std::numeric_limits<float>::max();
      _gammacutlow[i]=-std::numeric_limits<float>::max();
      _gammacuthigh[i]=std::numeric_limits<float>::max();
      _baselinecut[i]=std::numeric_limits<float>::max();
  }
  for(int i = 0; i < _padclow.GetSize(); i++) {
      _padclow[i]=-std::numeric_limits<float>::max();;
      _padchigh[i]=std::numeric_limits<float>::max();
  }
}

NGMHitFilter::~NGMHitFilter(){
    if(_blockPMTBaselines) _blockPMTBaselines->Clear();
    delete _blockPMTBaselines;
}

void NGMHitFilter::SetAccept(int ichan, bool accept)
{
  _accept[ichan] = accept;
}

bool NGMHitFilter::init(){return true;}

bool NGMHitFilter::process(const TObject &tData)
{
  static TClass* tNGMHitType = gROOT->GetClass("NGMHit");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  static TClass* tObjStringType = gROOT->GetClass("TObjString");
  
  //Check data type
  if(tData.InheritsFrom(tNGMHitType)){
    
    const NGMHit* hit = (const NGMHit*)(&tData);
    if (AnaHit(hit)) {
      push(tData);
    }
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
    ProcessConfiguration(confBuffer);
    push(tData);
  }else if(tData.IsA() == tObjStringType){
    push(tData);
  }

  return true;
  
}

bool NGMHitFilter::ProcessConfiguration(const NGMSystemConfiguration* conf)
{
    // Initialize particle identifier
    partID->Init(conf);
    if(!_blockPMTBaselines){
        _blockPMTBaselines = new TObjArray();
        _blockPMTBaselines->SetOwner();
    }else{
        _blockPMTBaselines->Clear();
    }
    _blockPMTBaselines->Expand(conf->GetChannelParameters()->GetEntries());
    for(int ichan = 0; ichan<conf->GetChannelParameters()->GetEntries(); ichan++)
    {
        (*_blockPMTBaselines)[ichan]=new NGMBaseline(_gatelength[ichan][0], 100,_baselinecut[ichan] );
    }
    _psdScheme = 0;
    if(conf->GetSystemParameters()->GetParIndex("XIAPSD_SCHEME")>=0)
    {
        _psdScheme = conf->GetSystemParameters()->GetParValueI("XIAPSD_SCHEME",0);;
    }
    if(conf->GetSystemParameters()->GetParIndex("PSD_SCHEME")>=0)
    {
        _psdScheme = conf->GetSystemParameters()->GetParValueI("PSD_SCHEME",0);;
    }

    if(conf->GetName()==TString("PIXIE16"))
    {
        for(int ichan = 0; ichan<conf->GetChannelParameters()->GetEntries(); ichan++)
        {
            _gatelength[ichan][0] = conf->GetChannelParameters()->GetParValueD("QDCLen0",ichan) * 100.0;
            _gatelength[ichan][1] = conf->GetChannelParameters()->GetParValueD("QDCLen1",ichan) * 100.0;
            _gatelength[ichan][2] = conf->GetChannelParameters()->GetParValueD("QDCLen2",ichan) * 100.0;
            _gatelength[ichan][3] = conf->GetChannelParameters()->GetParValueD("QDCLen3",ichan) * 100.0;
            _gatelength[ichan][4] = conf->GetChannelParameters()->GetParValueD("QDCLen4",ichan) * 100.0;
            _gatelength[ichan][5] = conf->GetChannelParameters()->GetParValueD("QDCLen5",ichan) * 100.0;
            _gatelength[ichan][6] = conf->GetChannelParameters()->GetParValueD("QDCLen6",ichan) * 100.0;
            _gatelength[ichan][7] = conf->GetChannelParameters()->GetParValueD("QDCLen7",ichan) * 100.0;
        }
        
    }else if(conf->GetName()==TString("SIS3316")){
        //SIS3316 psdScheme 2 uses 5 gates per channel
        //XIA Scheme 1 and 0 all have eight channels
        for(int islot = 0; islot < conf->GetSlotParameters()->GetEntries(); islot++)
        {
            const sis3316card* card = dynamic_cast<const sis3316card*>(conf->GetSlotParameters()->GetParValueO("card",islot));
            for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
            {
                for(int iqdc =0; iqdc<SIS3316_QDC_PER_CHANNEL; iqdc++)
                {
                    _gatelength[SIS3316_CHANNELS_PER_CARD*islot+ichan][iqdc] = card->qdclength[ichan/SIS3316_CHANNELS_PER_ADCGROUP][iqdc];
                }
            }
        }

    }else{
        LOG<<"Not implemented for configuration style"<<conf->GetName()<<ENDM_FATAL;
    }
    return true;
}


bool NGMHitFilter::AnaHit(const NGMHit* hit)
{
  int plotIndex = partID->getPlotIndex(hit);
  if(plotIndex>=64) return false;

  int pixel = hit->GetPixel();
  double baseline = hit->GetQuadGate(0)/_gatelength[plotIndex][0];
  double energy = hit->GetEnergy();
  double pulseHeight = hit->GetPulseHeight();
  bool accept = true;
  bool trace = true;
  bool dPrint = false;
  //This code block needs check on whether this is a block detector
  for(int spmt = plotIndex; spmt<plotIndex+4; spmt++)
  {
      NGMBaseline* bl =((NGMBaseline*)((*_blockPMTBaselines)[spmt]));
      double pmtbaseline = 0.0;
      if(_psdScheme==2)
      {
          pmtbaseline = hit->GetGate(5*(spmt%4));
      }else{
          pmtbaseline = hit->GetGate(8*(spmt%4));
      }
      if(false&&bl->AnaBaseline(pmtbaseline))
      {
          accept=false;
      // Always execute this complete loop
      // NGMBaselineNeeds sample
         if(dPrint) LOG<<"Rejecting Baseline "<<spmt<<" Avg("<<bl->Avg()/bl->_gatelength<<") Gate("<<hit->GetGate(8*(spmt%4))/bl->_gatelength<<")"<<ENDM_INFO;

      }
  }
    
  if(_pileup[plotIndex]!=0)
  {
      if(hit->GetPileUpCounter()!=0)
          accept = false;
      if(!trace) return accept;
  }
  if (!_accept.At(plotIndex))
  { 
    if(dPrint)LOG<<"Rejecting Channel "<<hit->GetSlot()<<":"<<hit->GetChannel()<<" "<<plotIndex<<ENDM_INFO; 
    accept = false;
      if(!trace) return accept;
  }
  if (baseline < _baselinelow[plotIndex]
      || baseline > _baselinehigh[plotIndex])
  {
    if(dPrint)LOG<<"Rejecting Baseline "<<hit->GetSlot()<<":"<<hit->GetChannel()<<" "<<baseline<<ENDM_INFO;  
    accept =  false;
      if(!trace) return accept;
  }
  if (energy < _energylow[plotIndex]
      || energy > _energyhigh[plotIndex])
  {
      if(dPrint)LOG<<"Rejecting Energy "<<plotIndex<<" "<<hit->GetSlot()<<":"<<hit->GetChannel()<<" "<<energy<<" ("<<_energylow[plotIndex]<<","<<_energyhigh[plotIndex]<<")"<<ENDM_INFO;
    accept = false;
      if(!trace) return accept;
  }
    if (pixel>=0 && pixel<_padclow.GetSize() && (pulseHeight < _padclow[pixel]
        || pulseHeight > _padchigh[pixel]))
    {
        if(dPrint)LOG<<"Rejecting PixelADC Pixel("<<pixel<<") "<<hit->GetSlot()<<":"<<hit->GetChannel()<<" "<<pulseHeight<<" ("<<_padclow[pixel]<<","<<_padchigh[pixel]<<")"<<ENDM_INFO;
        accept = false;
        if(!trace) return accept;
    }
  if (hit->GetNeutronId() < _neutroncutlow[plotIndex]
      || hit->GetNeutronId() > _neutroncuthigh[plotIndex])
  {
    if(dPrint)LOG<<"Rejecting NeutronCut "<<hit->GetSlot()<<":"<<hit->GetChannel()<<" "<<hit->GetNeutronId()<<ENDM_INFO; 
    accept = false;
      if(!trace) return accept;
  }
  if (hit->GetGammaId() < _gammacutlow[plotIndex]
      || hit->GetGammaId() > _gammacuthigh[plotIndex])
  {
    if(dPrint)LOG<<"Rejecting GammaCut "<<hit->GetSlot()<<":"<<hit->GetChannel()<<" "<<hit->GetNeutronId()<<ENDM_INFO; 
    accept = false;
      if(!trace) return accept;
  }
  return accept;
}

bool NGMHitFilter::finish(){return true;}

void NGMHitFilter::_print_TArrayF(std::ostream &os, const TArrayF &arr, int nchan, const char *name) const {
    os << name << ":" << std::endl;
    if (nchan<0) nchan = arr.GetSize();
    for (unsigned int i = 0 ; i < nchan ; ++i) {
        os << arr[i] << " ";
    }
    os << std::endl;
}

void NGMHitFilter::print(int nchan, std::ostream &os) const {
    os << "NGMHitFilter " << GetName() << std::endl;
    os << "Pileup:" << std::endl;
    int ntoprint = (nchan<0) ? _pileup.GetSize() : nchan;
    for (unsigned int i = 0 ; i < ntoprint ; ++i) {
        os << _pileup[i] << " ";
    }
    os << std::endl;
    os << "Gatelengths:" << std::endl;
    for (unsigned int i = 0 ; i < ntoprint ; ++i) {
        for (unsigned int j = 0 ; j < 8 ; ++j) {
            os << _gatelength[i][j] << " ";
        }
        os << "// ";
    }
    os << std::endl;
    _print_TArrayF(os, _baselinelow, nchan, "Baseline low");
    _print_TArrayF(os, _baselinehigh, nchan, "Baseline high");
    _print_TArrayF(os, _energylow, nchan, "Energy low");
    _print_TArrayF(os, _energyhigh, nchan, "Energy high");
    _print_TArrayF(os, _neutroncutlow, nchan, "Neutron cut low");
    _print_TArrayF(os, _neutroncuthigh, nchan,"Neutron cut high");
    _print_TArrayF(os, _gammacutlow, nchan, "Gamma cut low");
    _print_TArrayF(os, _gammacuthigh, nchan, "Gamma cut high");
    _print_TArrayF(os, _padclow, nchan, "ADC value low");
    _print_TArrayF(os, _padchigh, nchan, "ADC value high");
    _print_TArrayF(os, _baselinecut, nchan, "Baseline cut");
    
}
