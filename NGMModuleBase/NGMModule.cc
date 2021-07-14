#include "NGMModule.h"
#include "NGMSystemConfiguration.h"
#include "NGMBufferedPacket.h"
#include "NGMHit.h"
#include "NGMLogger.h"
#include "TSystem.h"
//#include "TObjString.h"
#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include <string>
#include <fstream>
#include "TFolder.h"
#include "TClass.h"
#include "NGMParticleIdent.h"

//using namespace std;

ClassImp(NGMModule);

NGMModule::NGMModule(){
  NGMModule::InitCommon();
}

NGMModule::NGMModule(const char* name, const char* title)
  :TTask(name,title)
{
  NGMModule::InitCommon();
}

NGMModule::~NGMModule()
{
    //RecursivelyRemoveChildren();
}

void NGMModule::InitCommon()
{
  _debug = false;
  _initialized = false;
  partID = 0;
  _updateDB = false;
  _parentFolder = 0;
  _runnumber = 0;
}

void NGMModule::setNeutronIdParams(double NeutronIdLow, double NeutronIdHigh, double gammaRejHigh)
{
  if(!partID) return;
  partID->setNeutronIdParams( NeutronIdLow,  NeutronIdHigh,  gammaRejHigh);
}
void NGMModule::AddCut(int tpartid, double tminenergy, double tmaxenergy)
{
  if(!partID) return;
  partID->AddCut( tpartid,  tminenergy,  tmaxenergy);
}
void NGMModule::initModules(){
  if(_initialized)
  {
    LOG<<"Chain Alread Initialized"<<GetName()<<ENDM_WARN;
    return;
  }
  
  // Create a Folder for this module to store its data in
  TString folderName(GetName());
  folderName+="Folder";
  
  _parentFolder = new TFolder( folderName.Data(), folderName.Data() );
  //The following makes the folder owner of all Added objects and responsible
  // for the delete
  _parentFolder->SetOwner();
  //This breaks under the ROOT behavior of TTask and TFolder
  //_parentFolder->Add(this);
  
  if(!init()) LOG<<"Initialization error:"<<GetName()<<ENDM_WARN;
  
  TIter next(fTasks);
  TTask *task;
  while((task=(TTask*)next())) {
		NGMModule* module = dynamic_cast<NGMModule*>(task);
    module->initModules();
    _parentFolder->Add(module->GetParentFolder());
  }
  _initialized = true;
}

bool  NGMModule::process(const TObject& tData)
{
  static TClass* tNGMSystemConfigurationType = TClass::GetClass("NGMSystemConfiguration");
  static TClass* tNGMBufferedPacketType = TClass::GetClass("NGMBufferedPacket");
  static TClass* tNGMHitType = TClass::GetClass("NGMHit");
  static TClass* tObjStringType = TClass::GetClass("TObjString");
  
  if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    return processConfiguration(dynamic_cast<const NGMSystemConfiguration*>(&tData));
  }else if(tData.InheritsFrom(tNGMBufferedPacketType)){
    return processPacket(dynamic_cast<const NGMBufferedPacket*>(&tData));
  }else if(tData.InheritsFrom(tNGMHitType)){
    return processHit(dynamic_cast<const NGMHit*>(&tData));
  }else if(tData.InheritsFrom(tObjStringType)){
    return processMessage(dynamic_cast<const TObjString*>(&tData));
  }
  return true;
}

void NGMModule:: push(const TObject& data){
  
  if(!_initialized){
    LOG<<"NGMModule is not initialized, process request denied:"<<GetName()<<ENDM_WARN;
    return;
  }
  
  TIter next(fTasks);
  TTask *task;
  while((task=(TTask*)next())) {
    if(task->IsActive())
		dynamic_cast<NGMModule*>(task)->process(data);    
  }  
}

void NGMModule::RecursivelyRemoveChildren()
{
    if(!_initialized){
        LOG<<"NGMModule is not initialized, process request denied:"<<GetName()<<ENDM_WARN;
        return;
    }
    
    TIter next(fTasks);
    TTask *task;
    while((task=(TTask*)next())) {
        dynamic_cast<NGMModule*>(task)->RecursivelyRemoveChildren();
    }
    fTasks->Clear();
}

void NGMModule:: finishModules(){

  if(!finish()) LOG<<"Closing error:"<<GetName()<<ENDM_WARN;

  TIter next(fTasks);
  TTask *task;
  while((task=(TTask*)next())) {
	if(task->IsActive())
		dynamic_cast<NGMModule*>(task)->finishModules();
  }
  _initialized = false;
}

void NGMModule::sendEndRunFlushAndSave(){
  TObjString endRunFlush("EndRunFlush");
  TObjString endRunSave("EndRunSave");
  push(*((const TObject*)&endRunFlush));
  push(*((const TObject*)&endRunSave));
}

NGMParticleIdent* NGMModule::getParticleIdent(){
  return partID;
}

NGMModule* NGMModule::FindModule(const char* name) const 
{
  TString nameToFind(name);
  TListIter titer(GetListOfTasks());
  NGMModule* tmpTask = 0;
  while((tmpTask = (NGMModule*)titer()))
  {
    if(nameToFind == tmpTask->GetName())
    {
      return tmpTask;
    }else{
      //Lets look recursively
      NGMModule* tfind = tmpTask->FindModule(name);
      if(tfind) return tfind;
    }
  }
  return 0;
}

void NGMModule::SetAllowDatabaseReads(bool newVal)
{
  _allowDatabaseReads = newVal;
}

bool NGMModule::GetAllowDatabaseReads() const
{
 return _allowDatabaseReads;
}

bool NGMModule::GetRunInfoFromDatabase(const char* runnumber)
{
    bool entryfound = false;
   _runnumber  = TString(runnumber).Atoll();
    
    TSQLServer* db = GetDBConnection();
    TSQLResult* res = 0;
    if(!db)
    {
      LOG<<"Error accessing database"<<ENDM_WARN;
      return false;
    }
    
    //First test if this entry is already in database
 //   TString sqlStatement("select runnumber,facility,series,experimentname, geofilename,calibrationfile,testobjectid from runconfiglookup where runnumber=\"");
    TString sqlStatement("select runnumber,facility,series, experimentname, geofilename,calibrationfile,testobjectid from (select `r`.`runnumber` AS `runnumber`,`r`.`facility` AS `facility`,`r`.`series` AS `series`,`r`.`experimentname` AS `experimentname`,`g`.`filename` AS `geofilename`,`r`.`calibrationfile` AS `calibrationfile`,`r`.`testobjectid` AS `testobjectid` from (`filelog` `r` left join `geometry` `g` on((`r`.`geomid` = `g`.`geomid`)))) rtc where rtc.runnumber=\"");

    sqlStatement+=runnumber;
    sqlStatement+="\"";
    
    res = db->Query(sqlStatement.Data());
    
    if(!res)
    {
      //To Get here we probably have executed a query for which we don't have permissions...
      LOG<<" Error executing query :"<<sqlStatement.Data()<<ENDM_WARN;
      db->Close();
      delete db;
      return false;
    }

    if(res->GetRowCount()>0)
    {
      entryfound = true;
      TSQLRow* row = res->Next();
      _facility = row->GetField(1);
      _series = row->GetField(2);
      _experimentname = row->GetField(3);
      _geoMacroName = row->GetField(4);
      _calibFileName = row->GetField(5);
      _testobjectid = row->GetField(6);
      LOG<<"DB Found "<<runnumber
        <<" "<<_facility
        <<" "<<_series
        <<" "<<_geoMacroName
        <<" "<<_testobjectid
        <<ENDM_INFO;
      delete row;
    }else{
      LOG<<"Run "<<runnumber<<" not found in database"<<ENDM_WARN;
      _facility = "";
      _series = "";
      _experimentname = "";
      _geoMacroName = "";
      _calibFileName = "";
      _testobjectid = "";
    }
  
    delete res; 
    db->Close();
    delete db;
  
    return true;
}

TString NGMModule::GetDBPassword()
{
  std::string password;
  std::ifstream pfile(TString(gSystem->HomeDirectory())+"/.ngmdbpw");
  pfile>>password;
  pfile.close();
  
  TString spw(password.c_str());
  return spw;
}

TString NGMModule::GetDBUser()
{
  std::string user;
  std::ifstream pfile(TString(gSystem->HomeDirectory())+"/.ngmdbuser");
  pfile>>user;
  pfile.close();
  
  TString spw(user.c_str());
  if(spw=="") spw = "ngmdaq";
  return spw;
}


TSQLServer* NGMModule::GetDBConnection()
{
  TString sServerURL("mysql://ngmgrp.llnl.gov/ngmrunlog");
  TString sNGMURL = gSystem->Getenv("NGMDBSERVER");
  if(sNGMURL!=""){
    sServerURL = sNGMURL;
  }
  
  
  TSQLServer* db = TSQLServer::Connect(sServerURL.Data(), GetDBUser(), GetDBPassword());
  if(!db)
  {
    LOG<<"Error accessing database at:"<<sServerURL.Data()<<ENDM_WARN;
  }
  
  return db;
}

void  NGMModule::UpdateDatabaseRunHeader(NGMSystemConfiguration* sysConfig, TSQLServer* db, bool forceUpdate)
{
  
  TString runNumber;
  runNumber+=sysConfig->getRunNumber();
  TString startTime;
  startTime = sysConfig->GetTimeStamp().AsString("cl"); // compact local time (PST)
  startTime = TString(startTime(0,startTime.Index(".",0)));
  
  TString timeOnly(startTime(11,8));
  TString dateOnly(startTime(0,10));
  
  TString runTimeComment = sysConfig->GetSystemParameters()->GetParValueS("Comment",0);
  runTimeComment.ReplaceAll("\"",1,"",0);
  
  TString runExperimentName = sysConfig->GetSystemParameters()->GetParValueS("ExperimentName",0);
  runExperimentName.ReplaceAll("\"",1,"",0);
  
  TString runDaqName = sysConfig->GetSystemParameters()->GetParValueS("DaqName",0);
  runDaqName.ReplaceAll("\"",1,"",0);

  TString runType = sysConfig->GetSystemParameters()->GetParValueS("RunType", 0);
  if(runType=="") runType="UNKNOWN";
  runType.ReplaceAll("\"",1,"",0);
  
  TString daqOperator = sysConfig->GetSystemParameters()->GetParValueS("DaqOperator", 0);
  if(daqOperator=="") daqOperator="UNKNOWN";
  daqOperator.ReplaceAll("\"",1,"",0);
  
  TString facility = sysConfig->GetSystemParameters()->GetParValueS("Facility", 0);
  facility.ReplaceAll("\"",1,"",0);
  
  TString sConfiguration = sysConfig->GetSystemParameters()->GetParValueS("Configuration", 0);
  sConfiguration.ReplaceAll("\"",1,"",0);

  TString sSimulation = sysConfig->GetSystemParameters()->GetParameterAsString("Simulation", 0);
  if(sSimulation=="") sSimulation="0";
  sSimulation.ReplaceAll("\"",1,"",0);

  TString saveWaveforms;
  if(sysConfig->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 0)
  {
    saveWaveforms = "Never";
  }else if(sysConfig->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 1)
  {
    saveWaveforms = "Always";
  }else if(sysConfig->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 2)
  {
    saveWaveforms = "BeginOfBuffer";
  }else if(sysConfig->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 3)
  {
    saveWaveforms = "OnPileup";
  }else if(sysConfig->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 4)
  {
    saveWaveforms = "BeginOfBuffer/OnPileup";
  }
  
  TString sdBuffering;
  if(sysConfig->GetSystemParameters()->GetParValueI("RunReadoutMode",0) == 1)
  {
    sdBuffering = "Single";
  }else if(sysConfig->GetSystemParameters()->GetParValueI("RunReadoutMode",0) == 2)
  {
    sdBuffering = "Double";
  }      
  
  double bufferFillFraction = sysConfig->GetSystemParameters()->GetParValueD("BufferFractionPerSpill",0);
  
  
  // Hardware thresholds
  TString sGSThreshold = sysConfig->GetChannelParameters()->FindFirstParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","GS");
  TString sGLThreshold = sysConfig->GetChannelParameters()->FindFirstParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","GL");
  TString sLSThreshold = sysConfig->GetChannelParameters()->FindFirstParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","LS");
  TString sMUThreshold = sysConfig->GetChannelParameters()->FindFirstParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","MU");
  if(sGSThreshold=="") sGSThreshold = "0";
  if(sGLThreshold=="") sGLThreshold = "0";
  if(sLSThreshold=="") sLSThreshold = "0";
  if(sMUThreshold=="") sMUThreshold = "0";
  
  TString activeChannelList;
  TString activeChannelListWRate;
  
  for(int ichan = 0; ichan < sysConfig->GetChannelParameters()->GetEntries(); ichan++)
  {
    // This classification should really use the NGMParticleIdent class -JN
    TString chanName = sysConfig->GetChannelParameters()->GetParValueS("DetectorName",ichan);
    
    if(chanName == "") continue;
    // Skip disabled channels if this is not a simulated file
    if(sSimulation!="1")
      if(sysConfig->GetChannelParameters()->GetParValueI("ChanEnableConf", ichan)==0) continue;
    
    activeChannelList+=chanName+", ";
    
    
  }
  
  // Chop the last comma and space
  activeChannelList.Chop(); activeChannelList.Chop();   
    
  // Connect to database
  bool ownDB = false;
  if(!db)
  {
    db = GetDBConnection();
    if(!db)
    {
      LOG<<"Unable to connect to run database"<<ENDM_WARN;
      return;
    }
    // We allocated the connection
    // so lets remind ourselves to delete it at the end of the method
    ownDB = true;
  }
  
  TSQLResult* res;
  
  //First test if this runnumber is already in database
  TString sqlStatement("select runnumber,runduration from filelog where runnumber=\"");
  sqlStatement+=runNumber;
  sqlStatement+="\"";
  
  res = db->Query(sqlStatement);
  std::cout<<" Number of rows: "<<res->GetRowCount()<<std::endl;
  bool runNumberFound = false;
  if(res->GetRowCount()>0)
  {
    runNumberFound = true;
    // For now lets not overwrite a previous entry
    // We will use runduration to represent the entire record
    TSQLRow* row = res->Next();
    if(!forceUpdate)
    {
      delete row;
      delete res;
      if(ownDB)
        delete db;
      return;
    }else{
      delete row;
    }
  }
  delete res;
  res = 0;
  
  // Form Update or insert statement
  TString updateStatement;
  if(runNumberFound){
    updateStatement="UPDATE filelog SET";
  }else{
    updateStatement="INSERT INTO filelog SET";
    updateStatement+=" runnumber=\""; updateStatement+=runNumber; updateStatement+="\", ";
  }
  
  updateStatement+=" starttime=\""; updateStatement+=timeOnly.Data(); updateStatement+="\", ";
  updateStatement+=" date=\""; updateStatement+=dateOnly; updateStatement+="\",";
  updateStatement+=" buffering=\""; updateStatement+=sdBuffering;updateStatement+="\", ";
  updateStatement+=" bufferfraction="; updateStatement+=TString::Format("%.0f",bufferFillFraction*100.0).Data(); updateStatement+=", ";
  updateStatement+=" thresholds_gl="; updateStatement+=sGLThreshold.Data();updateStatement+=", ";
  updateStatement+=" thresholds_gs="; updateStatement+=sGSThreshold.Data();updateStatement+=", ";
  updateStatement+=" thresholds_ls="; updateStatement+=sLSThreshold.Data();updateStatement+=", ";
  updateStatement+=" thresholds_mu="; updateStatement+=sMUThreshold.Data();updateStatement+=", ";
  updateStatement+=" daqcomment=\""; updateStatement+=runTimeComment.Data(); updateStatement+="\", ";
  updateStatement+=" experimentname=\""; updateStatement+=runExperimentName.Data(); updateStatement+="\", ";
  updateStatement+=" daqname=\""; updateStatement+=runDaqName.Data(); updateStatement+="\", ";
  
  updateStatement+=" runtype=\""; updateStatement+=runType.Data(); updateStatement+="\", ";
  updateStatement+=" daqoperator=\""; updateStatement+=daqOperator.Data(); updateStatement+="\", ";
  updateStatement+=" facility=\""; updateStatement+=facility.Data(); updateStatement+="\", ";
  updateStatement+=" configuration=\""; updateStatement+=sConfiguration.Data(); updateStatement+="\", ";
  updateStatement+=" simulation=\""; updateStatement+=sSimulation.Data(); updateStatement+="\", ";
  
  updateStatement+=" activechannellist=\""; updateStatement+=activeChannelList.Data(); updateStatement+="\" ";
  
  if(runNumberFound){
    updateStatement+="WHERE runnumber=\"";
    updateStatement+=runNumber;
    updateStatement+="\"";
  }
  // Execute the db statement
  LOG<<updateStatement.Data()<<ENDM_INFO;
  res = db->Query(updateStatement.Data());
  delete res;
  
  if(ownDB)
  {
    db->Close();
    delete db;
  }
  
}
