//
//  NGMBlockAliveMonitor.cpp
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMBlockAliveMonitor.h"
#include "TClass.h"
#include "NGMSimpleParticleIdent.h"
#include "NGMHit.h"
#include "NGMSystemConfiguration.h"
#include "TFolder.h"
#include "NGMLogger.h"
#include "TDirectory.h"
#include <cmath>
#include "TMath.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TVirtualPad.h"
#include "TObjString.h"
#include "TROOT.h"
#include "NGMBufferedPacket.h"
#include "TProfile2D.h"
#include "sis3316card.h"

NGMBlockAliveMonitor::NGMBlockAliveMonitor()
{
  InitCommon();
}

NGMBlockAliveMonitor::NGMBlockAliveMonitor(const char* name, const char* title)
:NGMModule(name,title)
{
  InitCommon();
  partID = new NGMSimpleParticleIdent();
}

NGMBlockAliveMonitor::~NGMBlockAliveMonitor(){}

bool  NGMBlockAliveMonitor::init()
{
  return true;
}

bool  NGMBlockAliveMonitor::finish()
{
  return true;
}
void  NGMBlockAliveMonitor::LaunchDisplayCanvas()
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
  
  if(_blockAlive) _blockAlive->DrawCopy("colz");
  
  cCanvas->Modified();
  cCanvas->Update();  
  
  return;

}

void  NGMBlockAliveMonitor::ResetHistograms()
{
  if(_blockAlive) _blockAlive->Reset();
  if(_baselineMonitor)
  {
    _baselineMonitor->Reset();
  }else{
      TString hname;
      hname.Form("%s_Baselines",GetName());
      int numPMTs=_bmap.GetNBlocks()*4;
      _baselineMonitor = new TProfile2D(hname.Data(),hname.Data(),
                                        1800,0,1800,
                                        numPMTs,0,numPMTs,"S"); //The S means errors are spread not error on the mean
      GetParentFolder()->Add(_baselineMonitor);
      _baselineMonitor->SetDirectory(0);
      _baselineMonitor->GetXaxis()->SetCanExtend(true);
  }

}

void NGMBlockAliveMonitor::Print(Option_t* option) const {}

bool NGMBlockAliveMonitor::processConfiguration(const NGMSystemConfiguration* conf)
{
  _runBeginTime = NGMTimeStamp(0,0);
  _bmap.Init(conf);
  partID->Init(conf);
  ResetHistograms();
    if(conf->GetChannelParameters()->GetColumn("QDCLen0"))
    {
        for(int ichan = 0; ichan < conf->GetChannelParameters()->GetEntries(); ichan++)
        {
            _gatewidth[ichan][0] = conf->GetChannelParameters()->GetParValueD("QDCLen0",ichan)*100;
            _gatewidth[ichan][1] = conf->GetChannelParameters()->GetParValueD("QDCLen1",ichan)*100;
            _gatewidth[ichan][2] = conf->GetChannelParameters()->GetParValueD("QDCLen2",ichan)*100;
            _gatewidth[ichan][3] = conf->GetChannelParameters()->GetParValueD("QDCLen3",ichan)*100;
            _gatewidth[ichan][4] = conf->GetChannelParameters()->GetParValueD("QDCLen4",ichan)*100;
            _gatewidth[ichan][5] = conf->GetChannelParameters()->GetParValueD("QDCLen5",ichan)*100;
            _gatewidth[ichan][6] = conf->GetChannelParameters()->GetParValueD("QDCLen6",ichan)*100;
            _gatewidth[ichan][7] = conf->GetChannelParameters()->GetParValueD("QDCLen7",ichan)*100;
            
        }
    }else if(conf->GetSlotParameters()->GetColumn("card")){
        
        for(int islot = 0; islot < conf->GetSlotParameters()->GetEntries(); islot++)
        {
            if(! conf->GetSlotParameters()->GetParValueI("ModEnable",islot)) continue;
            const sis3316card* card = dynamic_cast<const sis3316card*>(conf->GetSlotParameters()->GetParValueO("card",islot));
            for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
            {
                for(int iqdc =0; iqdc<SIS3316_QDC_PER_CHANNEL; iqdc++)
                {
                    _gatewidth[SIS3316_CHANNELS_PER_CARD*islot+ichan][iqdc] = card->qdclength[ichan/SIS3316_CHANNELS_PER_ADCGROUP][iqdc];
                }
            }
        }
        
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

  return true;
}

void NGMBlockAliveMonitor::InitCommon()
{
  _runBeginTime = NGMTimeStamp(0,0);
  _blockAlive = 0;
  _baselineMonitor = 0;
}

bool NGMBlockAliveMonitor::processPacket(const NGMBufferedPacket* packet)
{
  for(int ihit = 0; ihit < packet->getPulseCount();ihit++)
  {
    processHit(packet->getHit(ihit));
  }
  return true;
}

bool NGMBlockAliveMonitor::processHit(const NGMHit* tHit)
{
  int hwchan = partID->getPlotIndex(tHit);
  if(!_bmap.IsBlock(hwchan)) return false;
  int blockSeq = _bmap.GetBlockSequence(hwchan);
  
  if( _runBeginTime.GetSec()==0)
  {
    _runBeginTime = tHit->GetNGMTime();
  }
  
  double hittime = TMath::Max(0.0,tHit->TimeDiffNanoSec(_runBeginTime))/1e9;
  
  if(!_blockAlive) {
    TString hname;
    hname.Form("%s_Alive",GetName());
    //Find Histogram in current folder
    TH2* htmp = dynamic_cast<TH2*>( GetParentFolder()->FindObject(hname));
    if(!htmp)
    {
      htmp = new TH2I(hname,hname,1800,0,1800,_bmap.GetNBlocks(),0,_bmap.GetNBlocks());
      GetParentFolder()->Add(htmp);
      htmp->SetDirectory(0);
      htmp->GetXaxis()->SetCanExtend(true);
    }
    htmp->SetBins(1800,0,1800,_bmap.GetNBlocks(),0,_bmap.GetNBlocks());
    _blockAlive = htmp;
    LOG<<"Adding histogram " << hname.Data() << ENDM_INFO;
  }
  int ngates=tHit->GetGateCount()/4;
  _blockAlive->Fill(hittime,blockSeq);
  for(int ipmt = 0; ipmt<4;ipmt++)
  {
      int ibin = blockSeq*4+ipmt;
      int baselinegate=ipmt*ngates;
      if(ibin<0)
      {
          LOG<<"Negative baseline for "<<hwchan<<ENDM_WARN;
      }else{
          _baselineMonitor->Fill(hittime,ibin,tHit->GetGate(baselinegate)/_gatewidth[ibin][0]);
      }
  }
 
  return true;
}

bool NGMBlockAliveMonitor::processMessage(const TObjString* controlMessage)
{
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
  return true;
}

void NGMBlockAliveMonitor::GetDurationPerBlock(int nblocks, Double_t* duration, Double_t* duration2, Double_t* rates)
{
    if(!duration) return;
    int nbmax = TMath::Min(nblocks,_blockAlive->GetYaxis()->GetNbins());
    for(int iblock = 0; iblock < nbmax; iblock++)
    {
        //Find first non-zero bin
        int firstbin = 1;
        for(firstbin=1; firstbin<=_blockAlive->GetXaxis()->GetNbins();firstbin++)
        {
            if(_blockAlive->GetBinContent(firstbin,iblock+1)>1)
            {
                break;
            }
        }
        if(firstbin==_blockAlive->GetXaxis()->GetNbins())
        {
            duration[iblock]=0.0;
            continue;
        }
 
        //Find last non-zero bin
        int lastbin = firstbin;
        for(; lastbin<=_blockAlive->GetXaxis()->GetNbins();lastbin++)
        {
            if(_blockAlive->GetBinContent(lastbin,iblock+1)<2)
            {
                lastbin--;
                break;
            }
        }
        duration[iblock] =  _blockAlive->GetXaxis()->GetBinUpEdge(lastbin) - _blockAlive->GetXaxis()->GetBinLowEdge(firstbin);
    }
    
    for(int iblock = 0; iblock < nbmax; iblock++)
    {
        //Find first non-zero bin
        double tdur = 0.0;
        double tcounts = 0;
        double timePerBin = _blockAlive->GetXaxis()->GetBinWidth(1);
        for(int firstbin=1; firstbin<=_blockAlive->GetXaxis()->GetNbins();firstbin++)
        {
            int binContent = _blockAlive->GetBinContent(firstbin,iblock+1);
            if(binContent>0)
            {
                tdur+=timePerBin;
                tcounts+=binContent;
            }
        }
        duration2[iblock]=tdur;
        rates[iblock]=tcounts;
    }
    return;
}
