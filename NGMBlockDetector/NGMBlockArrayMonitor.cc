//
//  NGMBlockArrayMonitor.cpp
//  NGMDaq
//
//  Created by Newby, Robert Jason on 7/8/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMBlockArrayMonitor.h"

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
#include "TCanvas.h"
#include "TVirtualPad.h"
#include "TLegend.h"
#include "TH2I.h"
#include "TSystem.h"

ClassImp(NGMBlockArrayMonitor)

NGMBlockArrayMonitor::NGMBlockArrayMonitor()
{
  InitCommon();
}

NGMBlockArrayMonitor::NGMBlockArrayMonitor(const char* name, const char* title)
: NGMModule(name,title)
{
  InitCommon();
  partID = new NGMSimpleParticleIdent;
}

void NGMBlockArrayMonitor::InitCommon()
{
  blocknrows = 0;
  blockncols = 0;
  _blockArray=0;
  _blockColsVTime = 0;
  _blockCols = 0;
  _minTime = 0;
}

NGMBlockArrayMonitor::~NGMBlockArrayMonitor()
{

}

bool NGMBlockArrayMonitor::init()
{
  return true;
}

void NGMBlockArrayMonitor::processConf(const NGMSystemConfiguration* confBuffer)
{
  
  //Lets loop through each of the channels groups and determine the Block row, column, and pixelLUT
  blocknrows = confBuffer->GetSystemParameters()->GetParValueI("BLOCKNROWS",0);
  blockncols = confBuffer->GetSystemParameters()->GetParValueI("BLOCKNCOLS",0);
  LOG<<"Using block system ("<<blocknrows<<","<<blockncols<<ENDM_INFO;
  partID->Init(confBuffer);
  _bmap.Init(confBuffer);
  int npixprow=_bmap.GetNPixOnASide();
  if(!_blockArray){
    TString hname;
    hname.Form("%s_BlockArray",GetName());
    _blockArray = new TH2I(hname,hname,blockncols*npixprow,0,blockncols*npixprow,blocknrows*npixprow,0,blocknrows*npixprow);
    _blockArray->SetDirectory(0);
    GetParentFolder()->Add(_blockArray);
  }else{
    _blockArray->SetBins(blockncols*npixprow,0,blockncols*npixprow,blocknrows*npixprow,0,blocknrows*npixprow);
  }
  if(!_blockColsVTime){
    TString hname;
    hname.Form("%s_BlockColsVTime",GetName());
    // Need to set time range dynamically...
    Int_t maxDur = int(confBuffer->GetSystemParameters()->GetParValueD("MaxDuration",0));
    _blockColsVTime =
    new TH2I(hname,hname,maxDur,0,maxDur,blockncols*npixprow,-0.5,blockncols*npixprow-0.5);
    _blockColsVTime->SetDirectory(0);
    GetParentFolder()->Add(_blockColsVTime);
  } else {
    Int_t maxDur = int(confBuffer->GetSystemParameters()->GetParValueD("MaxDuration",0));
    _blockColsVTime->SetBins(maxDur,0,maxDur,
                             blockncols*npixprow,-0.5,blockncols*npixprow-0.5);
  }
  if(!_blockCols){
    TString hname;
    hname.Form("%s_BlockCols",GetName());
    // Need to set time range dynamically...
    _blockCols =
    new TH1I(hname,hname,blockncols*npixprow,-0.5,blockncols*npixprow-0.5);
    _blockCols->SetDirectory(0);
    GetParentFolder()->Add(_blockCols);
  } else {
    _blockCols->SetBins(blockncols*npixprow,-0.5,blockncols*npixprow-0.5);
  }
  

}

bool NGMBlockArrayMonitor::process(const TObject& tData)
{
  
  static TClass* tNGMBufferedPacketType = gROOT->GetClass("NGMBufferedPacket");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  static TClass* tNGMHitType = gROOT->GetClass("NGMHit");
  static TClass* tObjStringType = gROOT->GetClass("TObjString");
  
  //Check data type
  if(tData.InheritsFrom(tNGMHitType)){
    
    const NGMHit* tHit = (const NGMHit*)(&tData);
    
    analyzeHit(tHit);
    
  }else if(tData.InheritsFrom(tNGMBufferedPacketType)){
    
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* confBuffer = dynamic_cast<const NGMSystemConfiguration*>(&tData);
    processConf(confBuffer);
    ResetHistograms();
    
  }else if(tData.IsA() == tObjStringType){
    const TObjString* controlMessage = (const TObjString*)(&tData);
    if(controlMessage->GetString() == "EndRunFlush")
    {
      LaunchDisplayCanvas();
    }else if(controlMessage->GetString() == "EndRunSave"){
      LaunchDisplayCanvas();
    }else if(controlMessage->GetString() == "PlotUpdate"){
      LaunchDisplayCanvas();
    }else if(controlMessage->GetString() == "PlotReset"){
      ResetHistograms();
    }
  }
  
  push(tData);
  return true;
}

bool NGMBlockArrayMonitor::analyzeHit(const NGMHit* tHit)
{
  if (_minTime == 0){
    _minTime = tHit->GetTimeStamp().GetSec();
    _beginTime = tHit->GetTimeStamp();
  }
  int npixprow = _bmap.GetNPixOnASide();
  Double_t timeDiff = tHit->TimeDiffNanoSec(_beginTime)/1e9;

  if(_blockArray)
    _blockArray->Fill(tHit->GetPixel()%(npixprow*blockncols),
                      tHit->GetPixel()/(npixprow*blockncols));
  if(_blockColsVTime)
    _blockColsVTime->Fill(timeDiff,//tHit->GetTimeStamp().GetSec()-_minTime,
			  tHit->GetPixel()%(npixprow*blockncols));
  if(_blockCols)
    _blockCols->Fill(tHit->GetPixel()%(npixprow*blockncols));
  return true;
}


bool  NGMBlockArrayMonitor::finish()
{
  return true;
}

void  NGMBlockArrayMonitor::LaunchDisplayCanvas()
{
  TString cName;
  cName.Form("%s_Canvas",GetName());

  TVirtualPad* cCanvas = (TCanvas*)(GetParentFolder()->FindObject(cName.Data()));
  if(!cCanvas)
  {
    cCanvas = (TCanvas*)(gROOT->FindObjectAny(cName.Data()));
    GetParentFolder()->Add(cCanvas);
  }

  if(!cCanvas)
  {
    cCanvas= new TCanvas(cName.Data(),cName.Data());
    GetParentFolder()->Add(cCanvas);
  }
  cCanvas->Clear();

  cCanvas->Divide(1,2);

  cCanvas->cd(1);
  if(_blockArray) _blockArray->DrawCopy("colz");

  cCanvas->cd(2);
  if(_blockCols)  _blockCols->DrawCopy();
  
  cCanvas->cd();

  cCanvas->Update();  
  gSystem->ProcessEvents();
  return;
}


void  NGMBlockArrayMonitor::ResetHistograms()
{
  if(_blockArray) _blockArray->Reset();
  if(_blockColsVTime) _blockColsVTime->Reset();
  if(_blockCols) _blockCols->Reset();
  _minTime = 0;
}

ClassImp(NGMBlockArrayMonitorDisplay)

NGMBlockArrayMonitorDisplay::NGMBlockArrayMonitorDisplay()
{
  cBlockDisplay = 0;
}

NGMBlockArrayMonitorDisplay::NGMBlockArrayMonitorDisplay(const char* moduleName)
{
  TString cName("cBlockArray_");
  _moduleName = moduleName;
  cName+=_moduleName;
  cBlockDisplay = new TCanvas(cName,cName);
  spy.Connect();
}

Bool_t NGMBlockArrayMonitorDisplay::HandleTimer(TTimer* timer)
{
  TString hname;
  hname.Form("%s_BlockArray",_moduleName.Data());
  TH2* hHist = (TH2*)(spy.RequestObject(hname.Data()));
  if(hHist)
  {
    // Let the histogram be deleted when the canvas is cleared
    hHist->SetBit(TObject::kCanDelete);
    hHist->Draw("colz");
  }
  cBlockDisplay->Modified();
  cBlockDisplay->Update();
  timer->Start(1000,true);
  return true;
}

