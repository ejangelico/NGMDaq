/*
 *  NGMRateMonitor.cpp
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/5/08.
 *  Copyright 2008 LLNL. All rights reserved.
 *
 */

#include "NGMRateMonitor.h"
#include "TH1D.h"
#include "TROOT.h"
#include "NGMSystemConfiguration.h"
#include "NGMHit.h"
#include "NGMBufferedPacket.h"
#include "NGMParticleIdent.h"
#include "NGMSimpleParticleIdent.h"
#include "NGMLogger.h"
#include "TObjString.h"
#include "TFolder.h"
#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TCanvas.h"
#include "TVirtualPad.h"
#include "TThread.h"
#include <sqlite3.h>

ClassImp(NGMRateMonitor)

NGMRateMonitor::NGMRateMonitor()
{
  InitCommon();
}

NGMRateMonitor::NGMRateMonitor(const char* name, const char* title)
: NGMModule(name,title)
{
  InitCommon();
  partID = new NGMSimpleParticleIdent;
}

void NGMRateMonitor::InitCommon()
{
  _channelHitCount = 0;
  _particleTypeHitCounts = 0;
  _neutronCountsPerDetector = 0;
  _gammaCountsPerDetector = 0;  

  _config = 0;
  _forceDBUpdate = false;
  _outputPeriodSec = 1.0;
  _previousOutput=0.0;
  _db = 0;
  _ppStmt = 0;
  _saveToDB = false;
}

NGMRateMonitor::~NGMRateMonitor()
{
  // A Module is the owner of its histograms, but not its canvases
  // Now we declare the Parent Folder as the Object owner
  //delete _channelHitCount;
  //delete _particleTypeHitCounts;
  //delete _config;
  _ratelog.close();
  sqlite3_finalize(_ppStmt);
  sqlite3_close(_db);
}

bool NGMRateMonitor::init()
{
  int rc = 0;
  
  if(!_channelHitCount)
  {
    TString hname(GetName());
    hname+="_ChannelCounts";
    TH1::AddDirectory(kFALSE);
    _channelHitCount = new TH1D(hname.Data(),hname.Data(),1,0,1);
    _channelHitCount->SetDirectory(0);
    GetParentFolder()->Add(_channelHitCount);
  }
  
  if(!_particleTypeHitCounts)
  {
    TString hname(GetName());
    hname+="_ParticleCounts";
    TH1::AddDirectory(kFALSE);
    _particleTypeHitCounts = new TH1D(hname.Data(),hname.Data(),1,0,1);
    _particleTypeHitCounts->SetDirectory(0);
    GetParentFolder()->Add(_particleTypeHitCounts);
  }
  _gapFinder.Reset();
  _ratelog.open(TString(GetTitle())+".log",std::ios::app);

  if(! _saveToDB) return true;
  
  TString dbName(GetName());
  dbName+=".db";
  rc = sqlite3_open(dbName.Data(), &_db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(_db));
    sqlite3_close(_db);
    return false;
  }
  rc = sqlite3_prepare_v2(_db,
                          "CREATE TABLE if not exists lsrates (timestamp text, runnumber int, runduration float, grate float, nrate float, mrate float, time float, tsr integer); CREATE INDEX if not exists lsrates_runnumber ON lsrates(runnumber); CREATE INDEX if not exists lsrates_runduration ON lsrates(runduration); CREATE INDEX if not exists lsrates_tsr ON lsrates(tsr);",
                          -1,
                          &_ppStmt,0);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL PREPARE error: %d %s\n", rc,sqlite3_errmsg(_db));
    sqlite3_close(_db);
    return false;
  }
  
  rc = sqlite3_step(_ppStmt);
  if( rc!=SQLITE_DONE ){
    fprintf(stderr, "SQL STEP error: %d\n", rc);
    sqlite3_close(_db);
    return false;
  }
  
  rc = sqlite3_finalize(_ppStmt);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL FINALIZE error: %d %s\n", rc, sqlite3_errmsg(_db));
    sqlite3_close(_db);
    return false;
  }

  rc = sqlite3_prepare_v2(_db,
                          "insert into lsrates VALUES (?,?,?,?,?,?,?,strftime('%s',?));",
                          -1,
                          &_ppStmt,0);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL PREPARE error: %d\n", rc);
    sqlite3_close(_db);
    return false;
  }
  
  return true;
}

bool NGMRateMonitor::process(const TObject& tData)
{
  
  static TClass* tNGMBufferedPacketType = gROOT->GetClass("NGMBufferedPacket");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  static TClass* tNGMHitType = gROOT->GetClass("NGMHit");
  static TClass* tObjStringType = gROOT->GetClass("TObjString");
  
  //Check data type
  if(tData.InheritsFrom(tNGMHitType)){
    
    const NGMHit* tHit = (const NGMHit*)(&tData);
    if(partID->IsSelected(tHit))
      analyzeHit(tHit);
    
  }else if(tData.InheritsFrom(tNGMBufferedPacketType)){
        
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
    
    delete _config;
    _config = confBuffer->DuplicateConfiguration();
        
    partID->Init(_config);
    _previousOutput=0.0;
    _runnumber = _config->getRunNumber();
    
    // Set Histogram to match channel count
    int chanCount = _config->GetChannelParameters()->GetEntries();
    _channelHitCount->SetBins(chanCount,0,chanCount);
    
    // Set Histogram to match particle type count
    // first particle type is 1
    int particleCount = partID->GetMaxParticleTypes();
    _particleTypeHitCounts->SetBins(particleCount,1,particleCount+1);
    for(int itype = 1; itype <= particleCount; itype++)
    {
      _particleTypeHitCounts->GetXaxis()->SetBinLabel(itype, partID->GetParticleName(itype));
    }
    
    ResetHistograms();
  }else if(tData.IsA() == tObjStringType){
    const TObjString* controlMessage = (const TObjString*)(&tData);
    if(controlMessage->GetString() == "EndRunFlush")
    {
      int totalEvents = (int)(_channelHitCount->Integral());
      LOG<<GetName()<<" Run Duration "<<_gapFinder.GetRunDuration()*1E-9
      <<" LiveTime "<<_gapFinder.GetLiveTime()*1E-9
      <<" Total Events "<<totalEvents
      <<ENDM_INFO;

      LaunchDisplayCanvas();
    }else if(controlMessage->GetString() == "EndRunSave"){
      if(GetUpdateDB())
      {
        UpdateDatabaseRunHeader();
      }
    }else if(controlMessage->GetString() == "PlotUpdate"){
      LOG<<GetName()<<" RunDuration("<<_gapFinder.GetRunDuration()*1e-9<<")"<<ENDM_INFO;
       LaunchDisplayCanvas();
    }else if(controlMessage->GetString() == "PlotReset"){
      ResetHistograms();
    }
  }
  
  push(tData);
  return true;
}
void NGMRateMonitor::LogRates()
{
  int rc = 0;
  
  //(timestamp datetime,runnumber int, runduration float, grate float, nrate float, mrate float)
  double runduration = _gapFinder.GetRunDuration()*1e-9;
  TString timestamp = _gapFinder.GetLastTime().AsString("s");
  double timeseq = _gapFinder.GetRunDuration()*1e-9 - _previousOutput;
  if(0)
  {
    _ratelog<<_gapFinder.GetLastTime().AsString("s")
      <<" "<<_runnumber
      <<" "<<runduration
      <<" "<<_particleTypeHitCounts->GetBinContent(NGMSimpleParticleIdent::lsgamma)/_outputPeriodSec
      <<" "<<_particleTypeHitCounts->GetBinContent(NGMSimpleParticleIdent::lsneutron)/_outputPeriodSec
      <<" "<<_particleTypeHitCounts->GetBinContent(NGMSimpleParticleIdent::lsmuon)/_outputPeriodSec
      <<std::endl;
  }
  
  rc = sqlite3_bind_text(_ppStmt,1,timestamp.Data(),timestamp.Length(),SQLITE_STATIC);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL BIND TEXT error timestamp: %d\n", rc);
  }
  
  rc = sqlite3_bind_int64(_ppStmt,2,_runnumber);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL BIND TEXT error runnumber: %d\n", rc);
    return;
  }
  rc = sqlite3_bind_double(_ppStmt,3,runduration);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL BIND DOUBLE error: %d\n", rc);
    sqlite3_close(_db);
    return;
  }
  rc = sqlite3_bind_double(_ppStmt,4,_particleTypeHitCounts->GetBinContent(NGMSimpleParticleIdent::lsgamma)/timeseq);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL BIND DOUBLE error: %d\n", rc);
  }
  rc = sqlite3_bind_double(_ppStmt,5,_particleTypeHitCounts->GetBinContent(NGMSimpleParticleIdent::lsneutron)/timeseq);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL BIND DOUBLE error: %d\n", rc);
    return;
  }
  rc = sqlite3_bind_double(_ppStmt,6,_particleTypeHitCounts->GetBinContent(NGMSimpleParticleIdent::lsmuon)/timeseq);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL BIND DOUBLE error : %d\n", rc);
    return;
  }
  rc = sqlite3_bind_double(_ppStmt,7,timeseq);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL BIND DOUBLE error time : %d\n", rc);
    return;
  }
  
  rc = sqlite3_bind_text(_ppStmt,8,timestamp.Data(),timestamp.Length(),SQLITE_STATIC);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL BIND TEXT error tsr: %d\n", rc);
  }

  
  int maxtries = 100;
  int ntries = 0;
  rc = sqlite3_step(_ppStmt);
  if( rc!=SQLITE_DONE ){
    fprintf(stderr, "SQL STEP error: %d %s\n", rc, sqlite3_errmsg(_db));
  }
  while( rc!=SQLITE_DONE ){
    ntries++;
    if(ntries > maxtries)
    {
      LOG<<"Too many write database attempts "<<ENDM_WARN;
      return;
    }
    
    TThread::Sleep(0,100000000L);
    rc = sqlite3_reset(_ppStmt);
    if( rc!=SQLITE_OK ){
      fprintf(stderr, "\tSQL RESET error: %d %s\n", rc, sqlite3_errmsg(_db));
      continue;
    }
    rc = sqlite3_step(_ppStmt);
    if( rc!=SQLITE_DONE ){
      fprintf(stderr, "\t\tSQL STEP error: %d %s\n", rc, sqlite3_errmsg(_db));
      continue; 
    }
  }
  
  rc = sqlite3_reset(_ppStmt);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL STEP error: %d %s\n", rc, sqlite3_errmsg(_db));
    return;
  }
  
  rc = sqlite3_clear_bindings(_ppStmt);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL Clear bindings error: %d %s\n", rc, sqlite3_errmsg(_db));
    return;
  }
  
  if(_channelHitCount)
    _channelHitCount->Reset();
  if(_channelHitCount)
    _particleTypeHitCounts->Reset();
  _previousOutput = _gapFinder.GetRunDuration()*1e-9;    

}

bool NGMRateMonitor::analyzeHit(const NGMHit* tHit)
{
  int ptype = partID->GetType(tHit);
  int plotIndex = partID->getPlotIndex(tHit);
  
  _gapFinder.nextTimeIsGap(tHit->GetNGMTime());
  _channelHitCount->Fill(plotIndex);
  _particleTypeHitCounts->Fill(ptype);
  if(_saveToDB && _gapFinder.GetRunDuration()*1e-9 - _previousOutput > _outputPeriodSec)
  {
    LogRates();
  }
  return true;
}


bool  NGMRateMonitor::finish()
{
  return true;
}

void  NGMRateMonitor::LaunchDisplayCanvas()
{
  TString cName;
  cName.Form("%s_Canvas",GetName());

  TVirtualPad* cCanvas = (TCanvas*)(GetParentFolder()->FindObject(cName.Data()));
  if(!cCanvas)
  {
    cCanvas= new TCanvas(cName.Data(),cName.Data());
    GetParentFolder()->Add(cCanvas);
  }
  cCanvas->Clear();
  cCanvas->Divide(1,2);
  cCanvas->cd(1)->SetLogy();
  cCanvas->Modified();
  cCanvas->Update();
  
  TH1::AddDirectory(false);
  TH1* cRate = dynamic_cast<TH1*>(_channelHitCount->Clone());
  cRate->SetTitle(TString(GetName())+"_PerChannelRate");
  cRate->Scale(1.0/(_gapFinder.GetLiveTime()*1E-9));
  cRate->DrawCopy();
  delete cRate;
  
  cCanvas->cd(2)->SetLogy();
  cCanvas->Modified();
  cCanvas->Update();
  TH1::AddDirectory(false);


  TH1* pRate =  dynamic_cast<TH1*>(_particleTypeHitCounts->Clone());
  pRate->SetTitle(TString(GetName())+"_PerParticleRate");
  pRate->Scale(1.0/(_gapFinder.GetLiveTime()*1E-9));
  pRate->DrawCopy();
 
  delete pRate;

  cCanvas->Modified();
  cCanvas->Update();

  TH1::AddDirectory(true);
  
}

void  NGMRateMonitor::UpdateDatabaseRunHeader()
{
  
  TString runNumber;
  runNumber+=_runnumber;
  TString startTime;
  startTime = _config->GetTimeStamp().AsString("cl"); // compact local time (PST)
  startTime = TString(startTime(0,startTime.Index(".",0)));
  
  TString timeOnly(startTime(11,8));
  TString dateOnly(startTime(0,10));
  
  TString saveWaveforms;
  if(_config->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 0)
  {
    saveWaveforms = "Never";
  }else if(_config->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 1)
  {
    saveWaveforms = "Always";
  }else if(_config->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 2)
  {
    saveWaveforms = "BeginOfBuffer";
  }else if(_config->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 3)
  {
    saveWaveforms = "OnPileup";
  }else if(_config->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 4)
  {
    saveWaveforms = "BeginOfBufferOrOnPileup";
  }
  
  TString sdBuffering;
  if(_config->GetSystemParameters()->GetParValueI("RunReadoutMode",0) == 1)
  {
    sdBuffering = "Single";
  }else if(_config->GetSystemParameters()->GetParValueI("RunReadoutMode",0) == 2)
  {
    sdBuffering = "Double";
  }      
  
  double bufferFillFraction = _config->GetSystemParameters()->GetParValueD("BufferFractionPerSpill",0);
  
  TString runTimeComment = _config->GetSystemParameters()->GetParValueS("Comment",0);
  runTimeComment.ReplaceAll("\"",1,"",0);
  TString runExperimentName = _config->GetSystemParameters()->GetParValueS("ExperimentName",0);
  runExperimentName.ReplaceAll("\"",1,"",0);
  TString runDaqName = _config->GetSystemParameters()->GetParValueS("DaqName",0);
  runDaqName.ReplaceAll("\"",1,"",0);
  
  // Hardware thresholds
  TString sGSThreshold = _config->GetChannelParameters()->FindFirstParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","GS01");
  TString sGLThreshold = _config->GetChannelParameters()->FindFirstParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","GL01");
  TString sLSThreshold = _config->GetChannelParameters()->FindFirstParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","LS01");
  TString sMUThreshold = _config->GetChannelParameters()->FindFirstParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","MU01");
  if(sGSThreshold=="") sGSThreshold = "0";
  if(sGLThreshold=="") sGLThreshold = "0";
  if(sLSThreshold=="") sLSThreshold = "0";
  if(sMUThreshold=="") sMUThreshold = "0";
  
  
  double elapsedTime = 0.0;
  double liveTime = 0.0;
  TString selapsedTime;
  TString sliveTimeFraction;
  double gsrates = 0.0;
  double glrates = 0.0;
  double lsrates = 0.0;
  double lsnrates = 0.0;
  double herates = 0.0;
  double murates = 0.0;
  double systemRates = 0.0;
  
  elapsedTime = _gapFinder.GetRunDuration()*1E-9; // convert to secs
  liveTime = _gapFinder.GetLiveTime()*1E-9; // convert to secs
  selapsedTime.Form("%.1f",elapsedTime);
  sliveTimeFraction.Form("%.2f",liveTime/elapsedTime*100.0);

  TString activeChannelList;
  TString activeChannelListWRate;

  lsnrates = _particleTypeHitCounts->GetBinContent(NGMSimpleParticleIdent::lsneutron);
  for(int ichan = 0; ichan < _config->GetChannelParameters()->GetEntries(); ichan++)
  {
    // This classification should really use the NGMParticleIdent class -JN
    TString chanName = _config->GetChannelParameters()->GetParValueS("DetectorName",ichan);
    if(chanName.BeginsWith("GS"))
    {
      gsrates+=_channelHitCount->GetBinContent(ichan+1);
    }else if(chanName.BeginsWith("GL"))
    {
      glrates+=_channelHitCount->GetBinContent(ichan+1);
    }else if(chanName.BeginsWith("LS"))
    {
      lsrates+=_channelHitCount->GetBinContent(ichan+1);
    }else if(chanName.BeginsWith("HE"))
    {
      herates+=_channelHitCount->GetBinContent(ichan+1);
    }else if(chanName.BeginsWith("MU"))
    {
      murates+=_channelHitCount->GetBinContent(ichan+1);
    }
    
    double trate = 0.0;
    if(liveTime>0.0) trate = _channelHitCount->GetBinContent(ichan+1)/liveTime;
    TString tmpEntry;
    tmpEntry.Form("%0.1f",trate);
    
    if(chanName == "") continue;
    double ratethreshold = 0;
    if(chanName.BeginsWith("GS")||chanName.BeginsWith("GL"))
    {
      ratethreshold = 1.0;
    }else if(chanName.BeginsWith("MU"))
    {
      ratethreshold = 1.0;
    }else if(chanName.BeginsWith("HETTL"))
    {
      ratethreshold = 0.1;
    }else if(chanName.BeginsWith("LS"))
    {
      ratethreshold = 1.0;
    }
    if( trate > ratethreshold)
    {
      activeChannelList+=chanName+", ";
      activeChannelListWRate+=(chanName+"("+tmpEntry+ "), ");
    }
    
    
  }

  // Chop the last comma and space
  activeChannelList.Chop(); activeChannelList.Chop(); 
  activeChannelListWRate.Chop(); activeChannelListWRate.Chop(); 
  
  
  if(liveTime>0.0)
  {
    gsrates*=1.0/liveTime;
    glrates*=1.0/liveTime;
    lsrates*=1.0/liveTime;
    lsnrates*=1.0/liveTime;
    herates*=1.0/liveTime;
    murates*=1.0/liveTime;
    systemRates = gsrates + glrates + lsrates + herates + murates;   
  }
  
  // Connect to database
  TSQLServer* db = GetDBConnection();
  if(!db)
  {
    LOG<<"Unable to connect to run database"<<ENDM_WARN;
    return;
  }
  TSQLResult* res;
  
  //First test if this runnumber is already in database
  TString sqlStatement("select runnumber,runduration from filelog where runnumber=\"");
  sqlStatement+=_runnumber;
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
    if(TString(row->GetField(1))!=""&&!_forceDBUpdate)
    {
      delete row;
      delete res;
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
    updateStatement+=" runnumber=\""; updateStatement+=_runnumber; updateStatement+="\", ";
  }
  
  updateStatement+=" starttime=\""; updateStatement+=timeOnly.Data(); updateStatement+="\", ";
  updateStatement+=" date=\""; updateStatement+=dateOnly; updateStatement+="\",";
  updateStatement+=" runduration="; updateStatement+=selapsedTime; updateStatement+=", ";
  updateStatement+=" buffering=\""; updateStatement+=sdBuffering;updateStatement+="\", ";
  updateStatement+=" bufferfraction="; updateStatement+=TString::Format("%.0f",bufferFillFraction*100.0).Data(); updateStatement+=", ";
  updateStatement+=" livetimefraction="; updateStatement+=sliveTimeFraction.Data();updateStatement+=", ";
  updateStatement+=" thresholds_gl="; updateStatement+=sGLThreshold.Data();updateStatement+=", ";
  updateStatement+=" thresholds_gs="; updateStatement+=sGSThreshold.Data();updateStatement+=", ";
  updateStatement+=" thresholds_ls="; updateStatement+=sLSThreshold.Data();updateStatement+=", ";
  updateStatement+=" thresholds_mu="; updateStatement+=sMUThreshold.Data();updateStatement+=", ";
  updateStatement+=" daqcomment=\""; updateStatement+=runTimeComment.Data(); updateStatement+="\", ";
  updateStatement+=" experimentname=\""; updateStatement+=runExperimentName.Data(); updateStatement+="\", ";
  updateStatement+=" daqname=\""; updateStatement+=runDaqName.Data(); updateStatement+="\", ";
  
  updateStatement+=" activechannellist=\""; updateStatement+=activeChannelList.Data(); updateStatement+="\", ";
  updateStatement+=" activechannelrates=\""; updateStatement+=activeChannelListWRate.Data(); updateStatement+="\", ";

  updateStatement+=" systemrate="; updateStatement+=TString::Format("%.2f",systemRates).Data(); updateStatement+=", ";
  updateStatement+=" rate_gb="; updateStatement+=TString::Format("%.2f",gsrates+glrates).Data();updateStatement+=", ";
  updateStatement+=" rate_mb="; updateStatement+=TString::Format("%.2f",murates).Data();updateStatement+=", ";
  updateStatement+=" rate_ls="; updateStatement+=TString::Format("%.2f",lsrates).Data();updateStatement+=", ";
  updateStatement+=" rate_lsn="; updateStatement+=TString::Format("%.2f",lsnrates).Data();updateStatement+=", ";
  updateStatement+=" rate_3he="; updateStatement+=TString::Format("%.2f",herates).Data();updateStatement+=" ";
  
  if(runNumberFound){
    updateStatement+="WHERE runnumber=\"";
    updateStatement+=_runnumber;
    updateStatement+="\"";
  }
  // Execute the db statement
  LOG<<updateStatement.Data()<<ENDM_INFO;
  res = db->Query(updateStatement.Data());
  delete res;
  
  db->Close();
  delete db;
  
}


void  NGMRateMonitor::ResetHistograms()
{
  _gapFinder.Reset();
  if(_channelHitCount)
    _channelHitCount->Reset();
  if(_channelHitCount)
    _particleTypeHitCounts->Reset();
  
}
