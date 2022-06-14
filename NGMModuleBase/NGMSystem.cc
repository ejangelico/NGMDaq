#include "NGMSystem.h"
#include "NGMSystemConfiguration.h"
#include "NGMConfigurationTable.h"
#include "TFile.h"
#include "NGMLogger.h"
#include "TThread.h"
#include "TGClient.h"
#include "NGMDaqGui.h"
#include "TFolder.h"
#include "NGMSpyServ.h"
#include "TROOT.h"
#include "TTimer.h"
#include "TSysEvtHandler.h"
#include "TClass.h"

ClassImp(NGMSystem)

std::vector<NGMSystem*> NGMSystem::_ngmsystems;

NGMSystem::NGMSystem()
: NGMModule("NGMSystem","Default NGMSystem"){
  _config = 0;
  acqThread = 0;
  _treeSaveSequence = 0;
  spySrv = 0;
  stopDaqOnCtlC = 0;
  registerSystem(this);
}

void NGMSystem::registerSystem(NGMSystem* newSystem)
{
  if(newSystem) _ngmsystems.push_back(newSystem);
}

NGMSystem* NGMSystem::getSystem(int nsystem){
  if((int)_ngmsystems.size()>nsystem)
    return _ngmsystems[nsystem];
  return 0;
}

NGMSystem::~NGMSystem(){
  // deleting a null pointer is ok -JN 2011Jul26
  delete _config;
  delete spySrv;
}

int NGMSystem::readConfigFile(const char* configfile){
  TFile* confIn = new TFile(configfile);
  NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*) confIn->Get("NGMSystemConfiguration");
  confIn->GetList()->Remove(sysConf);
  delete confIn;
  if(!sysConf){
    LOG<<" Configuration not found in file "<<configfile<<ENDM_WARN;
    return -1;
  }
  if(_config){
    delete _config;
  }
  _config = sysConf;
  LOG<<" Configuration file loaded from "<<configfile<<ENDM_INFO;
  
  return 0;
}

int NGMSystem::saveConfigFile(const char* configfile){
  if(!_config){
    LOG<<" Configuration not initialized... Cannot save configuration!"<<ENDM_INFO;
  }
  TFile* confOut = new TFile(configfile,"RECREATE");
  confOut->WriteTObject(_config,"NGMSystemConfiguration");
  confOut->Close();
  delete confOut;
  LOG<<" Configuration file saved to "<<configfile<<ENDM_INFO;
  return 0;
}

int NGMSystem::SaveAnaTree() {
  
  // Generate name
  TString ofilename("NGMAna");
  if(GetPassName()!="")
  {
    ofilename+="_";
    ofilename+=GetPassName();
  }
  ofilename+="_";
  ofilename+=_runnumber;
  // Append save sequence
  TString sSeq;
  sSeq.Form("%02d",_treeSaveSequence);
  ofilename+="_";
  ofilename+=sSeq;
  ofilename+="-ngm.root";
  
  TFile* anaOut = new TFile(ofilename,"RECREATE");
  anaOut->WriteTObject(GetParentFolder(),GetParentFolder()->GetName());
  anaOut->Close();
  delete anaOut;
  LOG<<" Analysis file saved to "<<ofilename<<ENDM_INFO;
  // Increment tree save sequence to indicate how many times we have
  // saved a snapshot from this run.
  _treeSaveSequence++;
  
  return 0;
}


bool NGMSystem::init(){
  //Only an NGMSystem adds itself to the top level folder
  GetParentFolder()->Add(this);
  return true;
}

bool NGMSystem::process(const TObject& tdata){
  return true;
}

bool NGMSystem::finish(){
  return true;
}

void NGMSystem::LaunchAcquisitionStartThread(){
  if(!acqThread)
    acqThread = new TThread("memberfunction",(void(*)(void *)) &StartThreadedAcquisitionMethod, (void*) this);
  acqThread->Run();
  return;
}

void* NGMSystem::StartThreadedAcquisitionMethod(void * arg){
  NGMSystem* tSystem = (NGMSystem*) arg;
  tSystem->StartAcquisition();
  return 0;
}

void NGMSystem::LaunchGui()
{
  NGMDaqGui* gui = new NGMDaqGui(gClient->GetRoot(),800,800);
  gui->DisplayConfiguration(GetConfiguration());
}

void NGMSystem::LaunchSpyServ(int port)
{
  delete spySrv;
  spySrv = new NGMSpyServ(port);
}

void NGMSystem::ProcessSpyServ()
{
  if(spySrv)
    spySrv->CheckForRequests();
}

 void NGMSystem::GetStatus(TString& status)
{
    status="NGMSystem::GetStatus Not Implemented in Derived Class\n";
}

void NGMSystem::SetStopDaqOnCtlC()
{
    if(!stopDaqOnCtlC)
    {
        stopDaqOnCtlC = new TSignalHandler(kSigInterrupt);
        stopDaqOnCtlC->Connect("Notified()",this->IsA()->GetName(),this,"RequestAcquisitionStop()");
    }
    
    TSignalHandler* isig = 0;
    TIterator* itr = gSystem->GetListOfSignalHandlers()->MakeIterator();
    while((isig=(TSignalHandler*)(itr->Next())))
    {
        if(isig->GetSignal()==kSigInterrupt&&isig!=stopDaqOnCtlC)
        {
            gSystem->RemoveSignalHandler(isig);
        }
    }
    gSystem->GetListOfSignalHandlers()->Print();
    stopDaqOnCtlC->Add();
    printf("Press Ctl-C to Interrupt %s\n",
           this->IsA()->GetName());
    gSystem->GetListOfSignalHandlers()->Print();
}


void NGMSystem::ResetCtlC()
{
    gSystem->ResetSignal(kSigInterrupt);
    printf("Reset ROOT Interrupt Handler\n");
}
