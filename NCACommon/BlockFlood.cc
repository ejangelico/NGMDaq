/*
 *  NGMBlockFlood.cpp
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/5/08.
 *  Copyright 2008 LLNL. All rights reserved.
 *
 */

#include "NGMBlockFlood.h"
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
#include "TLine.h"
#include "sis3316card.h"

ClassImp(NGMBlockFlood)

NGMBlockFlood::NGMBlockFlood()
{
  InitCommon();
}

NGMBlockFlood::NGMBlockFlood(const char* name, const char* title)
: NGMModule(name,title)
{
  InitCommon();
  partID = new NGMSimpleParticleIdent;
}

void NGMBlockFlood::InitCommon()
{
  _displayType = 2; //0=individual tube energy, 1=total block energy, 2=floods
  _energyCutLow = -1.0;
  _energyCutHigh = -1.0;
}

NGMBlockFlood::~NGMBlockFlood()
{
  // A Module is the owner of its histograms, but not its canvases
  // Now we declare the Parent Folder as the Object owner
  //delete _channelHitCount;
  //delete _particleTypeHitCounts;
  //delete _config;
}
void  NGMBlockFlood::SetEnergyCut(double elow, double ehigh)
{
  _energyCutLow = elow;
  _energyCutHigh = ehigh;
}

bool NGMBlockFlood::init()
{
  return true;
}

double NGMBlockFlood::GetPMTAmplitude(const NGMHit* tHit,int ipmt)
{
    int chanSequence = partID->getPlotIndex(tHit);
    if(_psdScheme==1)
    {
        return tHit->GetGate(3+8*ipmt)/(double)_gatewidth[chanSequence+ipmt][3]
        - tHit->GetGate(0+8*ipmt)/(double)_gatewidth[chanSequence+ipmt][0];
    }else if (_psdScheme==2){
        return tHit->GetGate(4+5*ipmt)/(double)_gatewidth[chanSequence+ipmt][4]
        - tHit->GetGate(0+5*ipmt)/(double)_gatewidth[chanSequence+ipmt][0];
    }
    
    return 0.0;
}
double NGMBlockFlood::GetPSD(const NGMHit*){

    if(_psdScheme==1)
    {
        
    }else if (_psdScheme==2){
        
    }
    
    return 0.0;
}

bool NGMBlockFlood::process(const TObject& tData)
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
    blocknrows = confBuffer->GetSystemParameters()->GetParValueI("BLOCKNROWS",0);
    blockncols = confBuffer->GetSystemParameters()->GetParValueI("BLOCKNCOLS",0);
    _blockRowCol.ResizeTo(blocknrows*blockncols,2);
    detNum.resize(16);
    chanNum.resize(blocknrows*blockncols);
    _bmap.Init(confBuffer);
    for(int ichan = 0; ichan < confBuffer->GetChannelParameters()->GetEntries(); ichan+=4) {
      int iblock = ichan/4;
      if(!_bmap.IsBlock(ichan)) continue;
      TString detName(confBuffer->GetChannelParameters()->GetParValueS("DetectorName",ichan));
      if(detName!="" && detName!="NULL") {
	int detRow = confBuffer->GetDetectorParameters()->FindFirstRowMatching("DetectorName",detName);
        if(detRow>=0) {
	  detNum[iblock] = detRow;
	  chanNum[detRow] = iblock;
          _blockRowCol(detRow,0) = confBuffer->GetDetectorParameters()->GetParValueI("BLOCK_ROW",detRow);
          _blockRowCol(detRow,1) = confBuffer->GetDetectorParameters()->GetParValueI("BLOCK_COL",detRow);
	}
      }
    }
      _psdScheme = 0;
      if(confBuffer->GetSystemParameters()->GetParIndex("XIAPSD_SCHEME")>=0)
      {
          _psdScheme = confBuffer->GetSystemParameters()->GetParValueI("XIAPSD_SCHEME",0);;
      }
      if(confBuffer->GetSystemParameters()->GetParIndex("PSD_SCHEME")>=0)
      {
          _psdScheme = confBuffer->GetSystemParameters()->GetParValueI("PSD_SCHEME",0);;
      }
    if(_psdScheme==1||_psdScheme==0)
    {
        const NGMConfigurationParameter* nspar = confBuffer->GetSlotParameters()->GetColumn("NanoSecondsPerSample");
        //XIA Scheme 1 and 0 all have eight channels
        for(int ichan = 0; ichan < confBuffer->GetChannelParameters()->GetEntries(); ichan++)
          {
              double nsPerSample = 4.0;
              if (nspar)
              {
                  int slot = ichan/16;
                  if(nspar->GetValue(slot)>0.0)
                      nsPerSample = nspar->GetValue(slot);
                  
              }
          _gatewidth[ichan][0] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen0",ichan)*1000.0/nsPerSample;
          _gatewidth[ichan][1] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen1",ichan)*1000.0/nsPerSample;
          _gatewidth[ichan][2] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen2",ichan)*1000.0/nsPerSample;
          _gatewidth[ichan][3] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen3",ichan)*1000.0/nsPerSample;
          _gatewidth[ichan][4] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen4",ichan)*1000.0/nsPerSample;
          _gatewidth[ichan][5] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen5",ichan)*1000.0/nsPerSample;
          _gatewidth[ichan][6] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen6",ichan)*1000.0/nsPerSample;
          _gatewidth[ichan][7] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen7",ichan)*1000.0/nsPerSample;
        }
    }else if(_psdScheme==2){
        //SIS3316 psdScheme 2 uses 5 gates per channel
        //XIA Scheme 1 and 0 all have eight channels
        for(int islot = 0; islot < confBuffer->GetSlotParameters()->GetEntries(); islot++)
        {
            const sis3316card* card = dynamic_cast<const sis3316card*>(confBuffer->GetSlotParameters()->GetParValueO("card",islot));
            for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
            {
                for(int iqdc =0; iqdc<SIS3316_QDC_PER_CHANNEL; iqdc++)
                {
                    _gatewidth[SIS3316_CHANNELS_PER_CARD*islot+ichan][iqdc] = card->qdclength[ichan/SIS3316_CHANNELS_PER_ADCGROUP][iqdc];
                }
            }
        }
        
    }
    LOG<<"Using Widths "
       <<_gatewidth[0][0]<<" "
       <<_gatewidth[0][1]<<" "
       <<_gatewidth[0][2]<<" "
       <<_gatewidth[0][3]<<" "
       <<_gatewidth[0][4]<<" "
       <<_gatewidth[0][5]<<" "
       <<_gatewidth[0][6]<<" "
       <<_gatewidth[0][7]<<ENDM_INFO;

        
    partID->Init(confBuffer);
    
    _floodList.Expand(confBuffer->GetSlotParameters()->GetEntries()*4);
    _energyList.Expand(confBuffer->GetSlotParameters()->GetEntries()*4);
    _peakSumList.Expand(confBuffer->GetSlotParameters()->GetEntries()*4);
    _peakList.Expand(confBuffer->GetChannelParameters()->GetEntries());
    _psdList.Expand(confBuffer->GetChannelParameters()->GetEntries());
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

bool NGMBlockFlood::analyzeHit(const NGMHit* tHit)
{

  int plotIndex = partID->getPlotIndex(tHit);
    if(plotIndex>=64) return false;
    if(tHit->GetEnergy()<500) return false;
  int blockIndex = plotIndex/4;
  if(!_energyList[blockIndex])
  {
    TString hname;
    hname.Form("%s_Energy_%s",GetName(),partID->getChannelName(plotIndex).Data());
    TH1I* htmp = new TH1I(hname,hname,1000,0,1000.0);
    //htmp->SetBit(TH1::kCanRebin);
    _energyList[plotIndex/4] = htmp;
    htmp->SetDirectory(0);
    GetParentFolder()->Add(htmp);
  }
  TH1* hEnergy = dynamic_cast<TH1*>(_energyList[blockIndex]);
  hEnergy->Fill(tHit->GetEnergy());

  //  // Skip flood fill if outside energy cut
  // // Instead use NGMHitFilter to make cuts as below
  //  if(_energyCutLow>0.0 &&
  //     (tHit->GetEnergy()<_energyCutLow||_energyCutHigh<tHit->GetEnergy()))
  //    return true;

  if(!_floodList[plotIndex/4])
  {
    TString hname;
    hname.Form("%s_Flood_%s",GetName(),partID->getChannelName(plotIndex).Data());
    //    TH2I* htmp = new TH2I(hname,hname,600,-0.1,1.1,600,-0.1,1.1);
    // This histogram min/max is coupled to the (x,y) to pixel conversion in BlockDetectorCalibration!!!!!
    TH2I* htmp = new TH2I(hname,hname,512,0.0,1.0,512,0.0,1.0);
    _floodList[blockIndex] = htmp;
    LOG<<"Adding histogram "<<hname.Data()<<ENDM_INFO;
    htmp->SetDirectory(0);
    GetParentFolder()->Add(htmp);
  }
  
  TH2* hflood = dynamic_cast<TH2*>(_floodList[blockIndex]);
  hflood->Fill(tHit->GetBlockX(),tHit->GetBlockY());

  if(!_peakSumList[plotIndex/4])
  {
    TString hname;
    hname.Form("%s_PeakSum_%s",GetName(),partID->getChannelName(plotIndex).Data());
    TH1I* htmp = new TH1I(hname,hname,1600,0.0,16000.0);
    _peakSumList[plotIndex/4] = htmp;
    htmp->SetDirectory(0);
    GetParentFolder()->Add(htmp);
  }
  TH1* hPeakSum = dynamic_cast<TH1*>(_peakSumList[plotIndex/4]);
  float peakVal = tHit->GetPulseHeight();
    peakVal = 0.0;
    for(int ipmt = 0; ipmt < 4; ipmt++)
    {
        if(!_peakList[plotIndex+ipmt])
        {
            TString hname;
            hname.Form("%s_PeakPMT_%s_%d",GetName(),partID->getChannelName(plotIndex).Data(),ipmt);
            TH1I* htmp = new TH1I(hname,hname,1600,0.0,16000.0);
            _peakList[plotIndex+ipmt] = htmp;
            htmp->SetDirectory(0);
            GetParentFolder()->Add(htmp);
        }
        TH1* hPeak = dynamic_cast<TH1*>(_peakList[plotIndex+ipmt]);
        float pmtpval=GetPMTAmplitude(tHit, ipmt);
        peakVal+=pmtpval;
        hPeak->Fill(pmtpval);
    }
    hPeakSum->Fill(peakVal);

  if(!_psdList[blockIndex]) {
    TString hname;
    hname.Form("%s_PSD_%s",GetName(),partID->getChannelName(plotIndex).Data());
    TH2I* htmp = new TH2I(hname,hname,1000,0,4000,1000,0.85,1.15);
    _psdList[blockIndex] = htmp;
    LOG<<"Adding histogram " << hname.Data() << ENDM_INFO;
    htmp->SetDirectory(0);
    GetParentFolder()->Add(htmp);
  }

  TH2* hpsd = dynamic_cast<TH2*>(_psdList[blockIndex]);
  hpsd->Fill(peakVal,tHit->GetPSD());
  
  for(int ipmt = 0; ipmt < 4; ipmt++)
    {
      if(!_peakList[plotIndex+ipmt])
	{
	  TString hname;
	  hname.Form("%s_PeakPMT_%s_%d",GetName(),partID->getChannelName(plotIndex).Data(),ipmt);
	  TH1I* htmp = new TH1I(hname,hname,4000,0,4000.0);
	  _peakList[plotIndex+ipmt] = htmp;
	  htmp->SetDirectory(0);
	  GetParentFolder()->Add(htmp);
	}
      TH1* hPeak = dynamic_cast<TH1*>(_peakList[plotIndex+ipmt]);
      hPeak->Fill(GetPMTAmplitude(tHit, ipmt));
    }
  return true;
}


bool  NGMBlockFlood::finish()
{
  return true;
}

void  NGMBlockFlood::LaunchDisplayCanvas()
{
    printf("Begin Flood Display Type:%d\n",_displayType);
  Int_t iDisplayType = _displayType; //0=individual tube energy, 1=total block energy, 2=floods

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

  int icolors[4] = {kRed,kGreen,kBlue,kOrange};

  if(iDisplayType==0){
    
    cCanvas->Divide(blockncols,blocknrows);

    for(int idet = 0 ; idet < blocknrows*blockncols; idet++){
      //Canvas set up to look from back
      cCanvas->cd((blocknrows-_blockRowCol[idet][0]-1)*blockncols+(blockncols-_blockRowCol[idet][1]-1)+1);
      TH1* peakSum = (dynamic_cast<TH1*>(_peakSumList[chanNum[idet]]));
      if(!peakSum) continue;
      peakSum->SetLineColor(kBlack);
      peakSum->SetAxisRange(10,8000.0,"x");
      peakSum->DrawCopy();
      for(int ipmt = 0 ; ipmt < 4; ipmt++) {
	double q[1];
	double probSum[1] = {.95};
	TLine l;
	if(_peakList[ipmt+chanNum[idet]*4])
	  {
	    TH1* peak = (dynamic_cast<TH1*>(_peakList[ipmt+chanNum[idet]*4]));
	    peak->SetLineColor(icolors[ipmt]);
	    peak->DrawCopy("same");
	    peak->GetQuantiles(1,q,probSum);
	    l.SetLineColor(icolors[ipmt]);
	    l.DrawLine(q[0],0,q[0],peak->GetMaximum());
        double percenterr=1.0-q[0]/780.0;
        int trn8 = (int) 8*percenterr/.14;
        printf("%s %f %f %d/8 turns\n",peak->GetName(),q[0],percenterr,trn8);

	  }
      }
    }

  }
  else if(iDisplayType==1){

    cCanvas->Divide(1,2);
    TVirtualPad* allCanvas = cCanvas->cd(1);
    TVirtualPad* columnCanvas = cCanvas->cd(2);

    allCanvas->Divide(2,1);
    columnCanvas->Divide(blockncols,1);

    Bool_t firstDraw[2] = {false,false};

    for(int idet = 0 ; idet < blocknrows*blockncols; idet++){
      //Canvas set up to look from back
      TH1* peakSum = (dynamic_cast<TH1*>(_peakSumList[chanNum[idet]]));
      if(!peakSum) continue;

      double q[1];
      double probSum[1] = {.93};
      TLine l;

      peakSum->SetLineColor(icolors[chanNum[idet]%blocknrows]);
      peakSum->GetQuantiles(1,q,probSum);
      l.SetLineColor(icolors[chanNum[idet]%blocknrows]);
      peakSum->SetAxisRange(10,8000,"x");
      double percenterr=1.0-q[0]/1750.0;
      int trn8 = (int) 8*percenterr/.34;
      printf("%s %f %f %d/8 turns\n",peakSum->GetName(),q[0],percenterr,trn8);
      //printf("%s %f %f\n",peakSum->GetName(),q[0],1.0-q[0]/1750.0);
      allCanvas->cd(1);
      if(firstDraw[0]==false){
	peakSum->DrawCopy();
	l.DrawLine(q[0],0,q[0],peakSum->GetMaximum());
	firstDraw[0] = true;
      }
      else{
	peakSum->DrawCopy("same");
	l.DrawLine(q[0],0,q[0],peakSum->GetMaximum());
      }

      allCanvas->cd(2);
      if(_blockRowCol[idet][0]==_blockRowCol[idet][1]){
	if(firstDraw[1]==false){
	  peakSum->DrawCopy();
	  l.DrawLine(q[0],0,q[0],peakSum->GetMaximum());
	  firstDraw[1] = true;
	}
	else{
	  peakSum->DrawCopy("same");
	  l.DrawLine(q[0],0,q[0],peakSum->GetMaximum());
	}
      }
      
      columnCanvas->cd( (blockncols-_blockRowCol[idet][1]-1)+1 );
      if(chanNum[idet]%blocknrows == 0){
	peakSum->DrawCopy();
	l.DrawLine(q[0],0,q[0],peakSum->GetMaximum());
      }
      else{
      	peakSum->DrawCopy("same");
	l.DrawLine(q[0],0,q[0],peakSum->GetMaximum());
      }

    }

  }
  else if(iDisplayType==2){
    
    cCanvas->Divide(blockncols,blocknrows);

    for(int idet = 0 ; idet < blocknrows*blockncols; idet++){
      //Canvas set up to look from front
      cCanvas->cd((blocknrows-_blockRowCol[idet][0]-1)*blockncols+_blockRowCol[idet][1]+1);

      if(!_floodList[chanNum[idet]]) continue;
      TH1* htmp = dynamic_cast<TH1*>(_floodList[chanNum[idet]]);
      htmp->DrawCopy("colz");

    }
  }

  cCanvas->Modified();
  cCanvas->Update();  
  gSystem->ProcessEvents();

  return;

}


void  NGMBlockFlood::ResetHistograms()
{  for(int iplot = 0; iplot < _energyList.GetSize();iplot++)
    {
      if(!_energyList[iplot]) continue;
      TH1* htmp = dynamic_cast<TH1*>(_energyList[iplot]);
      TString hname;
      hname.Form("%s_Energy_%s",GetName(),partID->getChannelName(iplot*4).Data());
      htmp->SetNameTitle(hname.Data(),hname.Data());
      htmp->Reset();
    }
  for(int iplot = 0; iplot < _peakList.GetSize();iplot++)
  {
    if(!_peakList[iplot]) continue;
    TH1* htmp = dynamic_cast<TH1*>(_peakList[iplot]);
    TString hname;
    hname.Form("%s_PeakPMT_%s_%d",GetName(),partID->getChannelName(iplot).Data(),iplot%4);
    htmp->SetNameTitle(hname.Data(),hname.Data());
    htmp->Reset();
  }

  for(int iplot = 0; iplot < _peakSumList.GetSize();iplot++)
  {
    if(!_peakSumList[iplot]) continue;
    TH1* htmp = dynamic_cast<TH1*>(_peakSumList[iplot]);
    TString hname;
    hname.Form("%s_PeakSum_%s",GetName(),partID->getChannelName(iplot*4).Data());
    htmp->SetNameTitle(hname.Data(),hname.Data());
    htmp->Reset();
  }
  
  for(int iplot = 0; iplot < _floodList.GetSize();iplot++)
  {
    if(!_floodList[iplot]) continue;
    TH2* htmp = dynamic_cast<TH2*>(_floodList[iplot]);
    htmp->Reset();
    TString hname;
    hname.Form("%s_Flood_%s",GetName(),partID->getChannelName(iplot*4).Data());
    htmp->SetNameTitle(hname.Data(),hname.Data());
  }

  for(int iplot = 0; iplot < _psdList.GetSize();iplot++)
  {
    if(!_psdList[iplot]) continue;
    TH2* htmp = dynamic_cast<TH2*>(_psdList[iplot]);
    TString hname;
    hname.Form("%s_PSD_%s",GetName(),partID->getChannelName(iplot*4).Data());
    htmp->SetNameTitle(hname.Data(),hname.Data());
    htmp->Reset();      
  }

}

ClassImp(NGMBlockFloodDisplay)

NGMBlockFloodDisplay::NGMBlockFloodDisplay()
{
  cFloodDisplay = new TCanvas("cFloodDisplay","Flood");
  spy.Connect();
}

Bool_t NGMBlockFloodDisplay::HandleTimer(TTimer* timer)
{
  TH2* hHist = (TH2*)(spy.RequestObject("Flood_Flood_00"));
  if(hHist)
  {
    hHist->Draw("colz");
    // Let the histogram be deleted when the canvas is cleared
    hHist->SetBit(TObject::kCanDelete);
  }
  cFloodDisplay->Modified();
  cFloodDisplay->Update();
  timer->Start(1000,true);
  return true;
}

