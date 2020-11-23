#include "NGMSystemConfiguration.h"
#include "TH1.h"
#include <limits>

ClassImp(NGMSystemConfiguration)

NGMSystemConfiguration* NGMSystemConfiguration::DuplicateConfiguration() const
{
  return 0;
}

void NGMSystemConfiguration::CopyConfiguration(const NGMSystemConfiguration* sysToCopy)
{

   if(!sysToCopy) return;
   bool prevStatus = TH1::AddDirectoryStatus();
   TH1::AddDirectory(false);
   SetName(sysToCopy->GetName());
   SetTitle(sysToCopy->GetTitle());
   SetTimeStamp(sysToCopy->GetTimeStamp());
   setRunNumber(sysToCopy->getRunNumber());
   if(GetSystemParameters())
      GetSystemParameters()->CopyTable(sysToCopy->GetSystemParameters());
   if(GetSlotParameters())
      GetSlotParameters()->CopyTable(sysToCopy->GetSlotParameters());
   if(GetChannelParameters())
      GetChannelParameters()->CopyTable(sysToCopy->GetChannelParameters());
   if(GetHVParameters())
      GetHVParameters()->CopyTable(sysToCopy->GetHVParameters());
   if(GetDetectorParameters())
      GetDetectorParameters()->CopyTable(sysToCopy->GetDetectorParameters());
   TH1::AddDirectory(prevStatus);
}

void NGMSystemConfiguration::LaunchGUI()
{
   //NGMDaqGui* confGui = new NGMDaqGui(gClient->GetRoot(),800,600);
   //confGui->DisplayConfiguration(this);
}


ClassImp(NGMSystemConfigurationv1)

NGMSystemConfigurationv1::NGMSystemConfigurationv1(){
  _fSystemPars = 0;
  _fSlots = 0;
  _fChannels = 0;
  _fDetectors = 0;
  _fHVControl = 0;
  _runNumber = std::numeric_limits<long long>::max();
}

NGMSystemConfigurationv1::NGMSystemConfigurationv1(const char* confName, int nslots, int nchannels)
: NGMSystemConfiguration(confName,nslots,nchannels) {
  _fSystemPars= new NGMConfigurationTablev1(1);
  _fSlots= new NGMConfigurationTablev1(nslots);
  _fChannels= new NGMConfigurationTablev1(nchannels);
  _fDetectors = new NGMConfigurationTablev1(nchannels);
  _fHVControl = new NGMConfigurationTablev1(nchannels);
  _runNumber = std::numeric_limits<long long>::max();
}

NGMSystemConfiguration* NGMSystemConfigurationv1::DuplicateConfiguration() const
{
  NGMSystemConfiguration* newConf = new NGMSystemConfigurationv1(GetName(),1,1);
  newConf->CopyConfiguration(this);
  return newConf;
}


NGMSystemConfigurationv1::~NGMSystemConfigurationv1(){
  if(_fSystemPars) delete _fSystemPars;
  if(_fSlots) delete _fSlots;
  if(_fChannels) delete _fChannels;
  if(_fDetectors) delete _fDetectors;
  if(_fHVControl) delete _fHVControl;
}

void NGMSystemConfigurationv1::PrintTables(std::ostream &ostr) const{
  ostr<<"Configuration Name: "<< GetName()  << std::endl;
  ostr<<"Run Number: "<< getRunNumber()  << std::endl;
  ostr<<"-------------- System Parameters -------------- "<< std::endl;
  _fSystemPars->PrintTable(ostr);
  ostr<<"--------------  Slot Parameters  -------------- "<< std::endl;
  _fSlots->PrintTable(ostr);
  ostr<<"-------------- Channel Parameters -------------- "<< std::endl;
  _fChannels->PrintTable(ostr);
  ostr<<"-------------- Detector Parameters -------------- "<< std::endl;
  if(_fDetectors)
	_fDetectors->PrintTable(ostr);
}

void NGMSystemConfigurationv1::SetTimeStampNow()
{
  _timestamp.Set(); // set _timestamp to current time, UTC
}

long long NGMSystemConfigurationv1::getRunNumber() const {

  if(_runNumber == std::numeric_limits<long long>::max()){
    // This means runnumber was never explicitly set
    // We'll generate it from the timestamp
    long long runNumber = _timestamp.GetDate();
    runNumber*=1000000L;
    runNumber+=_timestamp.GetTime();
    return runNumber;
  }
  return _runNumber;
}


