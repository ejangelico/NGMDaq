//
//  AnaAverageWaveform.cpp
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "AnaAverageWaveform.h"
#include "TClass.h"
#include "NGMHit.h"
#include "NGMSystemConfiguration.h"
#include "TFolder.h"
#include "NGMLogger.h"
#include "TDirectory.h"
#include "TObjString.h"
#include <cmath>
#include "TMath.h"
#include "NGMBufferedPacket.h"
#include "TH2.h"
#include "NGMSimpleParticleIdent.h"
#include "NGMSystem.h"
#include "TVirtualPad.h"
#include "TCanvas.h"
#include "TROOT.h"


AnaAverageWaveform::AnaAverageWaveform()
{
    InitCommon();
}

AnaAverageWaveform::AnaAverageWaveform(const char* name, const char* title)
:NGMModule(name,title)
{
    InitCommon();
    partID = new NGMSimpleParticleIdent();
}

AnaAverageWaveform::~AnaAverageWaveform(){}

bool  AnaAverageWaveform::init()
{
    return true;
}

bool  AnaAverageWaveform::finish()
{
    return true;
}
void  AnaAverageWaveform::LaunchDisplayCanvas()
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
    
    //
    cCanvas->Divide(1,2);
    cCanvas->cd(1);
    if(_psde) _psde->DrawCopy("colz");
    cCanvas->cd(2);
    _psda.nWaveform->DrawCopy();
    _psda.gWaveform->SetLineColor(kRed);
    _psda.gWaveform->DrawCopy("same");
    
    cCanvas->Modified();
    cCanvas->Update();
    
    return;
    
}

void  AnaAverageWaveform::ResetHistograms()
{
    _eventsanalyzed = 0;
    if(_psde)
    {
        _psde->Reset();
    }else{
        TString hname;
        hname.Form("%s_psde",GetName());
        _psde = new TH2D(hname.Data(),hname.Data(),
                         1,0.0,4000.0,20000,-2.0,2.0);
        _psde->SetDirectory(0);
        _psde->GetXaxis()->SetCanExtend(true);
        GetParentFolder()->Add(_psde);
    }
}

void AnaAverageWaveform::Print(Option_t* option) const {}

bool AnaAverageWaveform::processConfiguration(const NGMSystemConfiguration* conf)
{
    partID->Init(conf);
    _psda.SetWaveformParams(4.0,0,10000.0);
    _psda.nWaveform->SetDirectory(0); GetParentFolder()->Add(_psda.nWaveform);
    _psda.gWaveform->SetDirectory(0); GetParentFolder()->Add(_psda.gWaveform);
    _psda.gGattiWeights->SetDirectory(0); GetParentFolder()->Add(_psda.gGattiWeights);

    ResetHistograms();
    return true;
}

void AnaAverageWaveform::InitCommon()
{
    channelToAnalyze = 0;
    _psde = 0;
    _emin=350.0;
    _emax=500.0;
//    //EJ299
//    _npsdmin=0.6;
//    _npsdmax=0.75;
//    _gpsdmin=0.75;
//    _gpsdmax=1.0;
    //EJ309
    _npsdmin=0.45;
    _npsdmax=0.62;
    _gpsdmin=0.62;
    _gpsdmax=0.8;

}

bool AnaAverageWaveform::processPacket(const NGMBufferedPacket* packet)
{
    for(int ihit = 0; ihit < packet->getPulseCount();ihit++)
    {
        processHit(packet->getHit(ihit));
    }
    return true;
}

bool AnaAverageWaveform::processHit(const NGMHit* tHit)
{
    int hwchan = partID->getPlotIndex(tHit);
    if(hwchan!=channelToAnalyze) return false;
    
    double bl  = tHit->ComputeSum(0,800)/800.0;
    //double e = (tHit->getMaxValue()-bl)*477.0/1000.0;
    //double eprompt = tHit->ComputeSum(975,15) - bl*15.0;
    double eprompt = tHit->ComputeSum(975,10) - bl*10.0;
    double etail = tHit->ComputeSum(1000,275) - bl*275.0;
    double etotal = tHit->ComputeSum(975,250) - bl*250.0;
    double psd = eprompt/etotal;
    //double e = etotal*477.0/3800.0; // EJ299HV1000 etotal:ComputeSum(975,250)
    //double e = etotal*477.0/8000.0; // EJ309ScionixHV1120 etotal:ComputeSum(975,250) 1120V
    double e = etotal*477.0/4000.0; // EJ309ScionixHV1120 etotal:ComputeSum(975,250) 1000V
    
    //psd = etail/etotal;
    
    if(_emin<e
       &&e<_emax
       //&& tHit->GetPileUpCounter()==0
       && tHit->CalcPileup(2, 0, 100,1100)<1
       && tHit->CalcPileup(2, 0, 100,0,800)<1
       )
    {
        _psda.baseline=bl;
        _psda.normalization = etotal;
        //double psd2 = _psda.ComputePSD(tHit);
        _psde->Fill(e,psd);
        _eventsanalyzed++;
        if(_npsdmin<psd&&psd<_npsdmax)
        {
            _psda.AddNeutron(tHit,bl,etotal);
        }else if(_gpsdmin<psd&&psd<_gpsdmax){
            _psda.AddGamma(tHit,bl,etotal);
        }
    }
    if(_eventsanalyzed>1000000)
    {
        NGMSystem::getSystem()->RequestAcquisitionStop();  
    }
    return true;
}

bool AnaAverageWaveform::processMessage(const TObjString* controlMessage)
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
