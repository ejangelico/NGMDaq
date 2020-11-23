//
//  NGMPSDMaker.cpp
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMPSDMaker.h"
#include "TClass.h"
#include "NGMSimpleParticleIdent.h"
#include "NGMHit.h"
#include "NGMSystemConfiguration.h"
#include "TFolder.h"
#include "NGMLogger.h"
#include "TF1.h"
#include "TCutG.h"
#include "TVirtualFitter.h"
#include "TDirectory.h"
#include "TSpectrum.h"
#include <cmath>
#include "TGraphErrors.h"
#include "TFitter.h"
#include "TROOT.h"
#include "NGMHitPSD.h"
#include "NGMHitProcess.h"

NGMPSDMaker::NGMPSDMaker()
{
    InitCommon();
}

NGMPSDMaker::NGMPSDMaker(const char* name, const char* title)
:NGMModule(name,title)
{
    InitCommon();
    partID = new NGMSimpleParticleIdent();
}

NGMPSDMaker::~NGMPSDMaker(){}

bool  NGMPSDMaker::init()
{
    return true;
}

bool  NGMPSDMaker::process(const TObject& tData)
{
    static TClass* tNGMBufferedPacketType = TClass::GetClass("NGMBufferedPacket");
    static TClass* tNGMSystemConfigurationType = TClass::GetClass("NGMSystemConfiguration");
    static TClass* tNGMHitType = TClass::GetClass("NGMHit");
    static TClass* tObjStringType = TClass::GetClass("TObjString");
    
    if(tData.InheritsFrom(tNGMHitType)){
        const NGMHit* tHit = (const NGMHit*)(&tData);
        processHit(tHit);
    }else if(tData.InheritsFrom(tNGMBufferedPacketType)){
        
    }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
        const NGMSystemConfiguration* conf = dynamic_cast<const NGMSystemConfiguration*>(&tData);
        processConfiguration(conf);
    }else if(tData.InheritsFrom(tObjStringType)){
        const TObjString* controlMessage = (const TObjString*)(&tData);
        if(controlMessage->GetString() == "EndRunFlush")
        {
            LaunchDisplayCanvas();
        }else if(controlMessage->GetString() == "EndRunSave"){
            //LaunchDisplayCanvas();
        }else if(controlMessage->GetString() == "PlotUpdate"){
            LaunchDisplayCanvas();
        }else if(controlMessage->GetString() == "PlotReset"){
            ResetHistograms();
        }
    }
    return true;
}

bool  NGMPSDMaker::finish()
{
    return true;
}
void  NGMPSDMaker::LaunchDisplayCanvas()
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
    int maxItemsPerRow = 4;
    int nr = _hitPSD.size()/maxItemsPerRow;
    if(_hitPSD.size()%maxItemsPerRow>0) nr++;
    int nc = maxItemsPerRow;
    if(nr<1){
        nr=1;
        nc=_hitPSD.size();
    }
    
    cCanvas->Divide(nr,nc);
    if(_plotPSDE)
    {
        std::map<int,TH2*>::iterator hiter;
        int ipad = 0;
        for (hiter = _hitPSDvsE.begin(); hiter!=_hitPSDvsE.end(); hiter++) {
            cCanvas->cd(++ipad);
            hiter->second->Draw("colz");
        }
    }else{
        std::map<int,TH2*>::iterator hiter;
        int ipad = 0;
        for (hiter = _hitPSD.begin(); hiter!=_hitPSD.end(); hiter++) {
            cCanvas->cd(++ipad);
            hiter->second->Draw("colz");
        }        
    }
    cCanvas->Modified();
    cCanvas->Update();
    
    return;
    
}

void  NGMPSDMaker::ResetHistograms()
{
    std::map<int,TH2*>::iterator hiter;
    for (hiter = _hitPSD.begin(); hiter!=_hitPSD.end(); hiter++) {
        TString hname;
        hname.Form("%s_PSD_%s",GetName(), partID->getChannelName(hiter->first).Data());
        hiter->second->SetNameTitle(hname.Data(),hname.Data());
        hiter->second->Reset();
    }
    for (hiter = _hitPSDvsE.begin(); hiter!=_hitPSDvsE.end(); hiter++) {
        TString hname;
        hname.Form("%s_PSDvsE_%s",GetName(),partID->getChannelName(hiter->first).Data());
        hiter->second->SetNameTitle(hname.Data(),hname.Data());
        hiter->second->Reset();
    }
}

void NGMPSDMaker::SetPars(int chanSeq, int psdScheme, double keVPerADC)
{
    _hitprocess[chanSeq]->SetPSDScheme(psdScheme);
    _hitprocess[chanSeq]->SetkeVPerADC(keVPerADC);
}

void NGMPSDMaker::initChanHist(int chanSeq)
{
    const double maxE = _maxE;
    const int ebins = 1000.0;
    const int adcbins = 1000.0;
    
    
    
    if(!_hitPSD.count(chanSeq)) {
        TString hname;
        hname.Form("%s_PSD_%s",GetName(),partID->getChannelName(chanSeq).Data());
        //Find Histogram in current folder
        TH2* htmp = dynamic_cast<TH2*>( GetParentFolder()->FindObject(hname));
        if(!htmp)
        {
            htmp = new TH2I(hname,hname,adcbins,0,_maxADC,1000,_minPSD,_maxPSD);
            GetParentFolder()->Add(htmp);
            htmp->SetDirectory(0);
            LOG<<"Adding histogram " << hname.Data() << ENDM_INFO;
        }else{
            htmp->SetBins(adcbins,0,_maxADC,1000,_minPSD,_maxPSD);
        }
        _hitPSD[chanSeq] = htmp;
    }
    
    if(!_hitPSDvsE.count(chanSeq)) {
        TString hname;
        hname.Form("%s_PSDvsE_%s",GetName(),partID->getChannelName(chanSeq).Data());
        //Find Histogram in current folder
        TH2* htmp = dynamic_cast<TH2*>( GetParentFolder()->FindObject(hname));
        if(!htmp)
        {
            htmp = new TH2I(hname,hname,ebins,0,maxE,1000,_minPSD,_maxPSD);
            GetParentFolder()->Add(htmp);
            htmp->SetDirectory(0);
        }
        htmp->SetBins(ebins,0,maxE,1000,_minPSD,_maxPSD);
        _hitPSDvsE[chanSeq] = htmp;
        LOG<<"Adding histogram " << hname.Data() << ENDM_INFO;
    }
}

void NGMPSDMaker::Print(Option_t* option) const {}

bool NGMPSDMaker::processConfiguration(const NGMSystemConfiguration* conf)
{
    LOG<<"BlockPSDMaker processConfiguraiton"<<ENDM_INFO;
    partID->Init(conf);
    for(int ichan =0; ichan<partID->getNumberOfChannels();ichan++)
    {
        if(_hitprocess.count(ichan)!=0 && _hitprocess[ichan]!=0){
        }else{
            _hitprocess[ichan]= new NGMHitProcess();
        }
        _hitprocess[ichan]->RecalcFromWaveforms(_doRecalcFromWaveforms);
        _hitprocess[ichan]->Init(conf,ichan);
        
    }
    ResetHistograms();
    return true;
}

void NGMPSDMaker::InitCommon()
{
    _maxE = 4000.0;
    _maxADC = 16000;
    _minPSD = 0.5;
    _maxPSD = 1.5;
    _plotPSDE = true;
    _doRecalcFromWaveforms = false;
}

bool NGMPSDMaker::processHit(const NGMHit* tHit)
{
    int chanSeq = partID->getPlotIndex(tHit);
    
    if(!_hitPSD.count(chanSeq)) initChanHist(chanSeq);
    
    TH2* hpsd = _hitPSD[chanSeq];
    TH2* hpsde = _hitPSDvsE[chanSeq];

    if(_hitprocess[chanSeq])
    {
        _hitprocess[chanSeq]->ProcessHit(tHit);
        hpsd->Fill(_hitprocess[chanSeq]->GetPulseHeight(),_hitprocess[chanSeq]->GetPSD());
        hpsde->Fill(_hitprocess[chanSeq]->GetEnergy(),_hitprocess[chanSeq]->GetPSD());
    }else{
        hpsd->Fill(tHit->GetPulseHeight(),tHit->GetPSD());
        hpsde->Fill(tHit->GetEnergy(),tHit->GetPSD());
    }
    return true;
}


bool NGMPSDMaker::fitNeutronGammaBand(const char* detectorPrefix, double minx,double maxx, int nbins,bool ADCorE)
{
    
    //unused:  NGMConfigurationTable* calTable = conf->GetDetectorParameters();
    // Lets get a pointer to the local directory
    int ibin = 1;
    TFolder* histDir = GetParentFolder();
    TString hName(GetName());
    if(ADCorE)
        hName+="_PSD_";
    else
        hName+="_PSDvsE_";
    
    hName+=detectorPrefix;
    
    TH2* h2DOrig = (TH2*) histDir->FindObjectAny(hName);
    
    TCanvas* cCanvas = 0;
    
    cCanvas = (TCanvas*) GetParentFolder()->FindObjectAny("cNGammaFit");
    
    if(!cCanvas)
    {
        cCanvas = new TCanvas("cNGammaFit","cNGammaFit");
        cCanvas->Divide(1,2);
    }
    
    if(!h2DOrig) return false;
    // Were going to use a set variable width bins
    // To accomodate fission spectra statistics
    h2DOrig->GetXaxis()->SetRange(1,h2DOrig->GetNbinsX());
    
    TH1* hProj = h2DOrig->ProjectionX("_px");
    int minbin = TMath::Max(hProj->GetXaxis()->FindBin(minx),1);
    minx=hProj->GetXaxis()->GetBinLowEdge(minbin);
    
    int maxbin = TMath::Min(hProj->GetXaxis()->FindBin(maxx),hProj->GetNbinsX());
    for(ibin = 1; ibin < minbin; ibin++) hProj->SetBinContent(ibin,0);
    for(ibin = maxbin+1; ibin <= hProj->GetNbinsX(); ibin++) hProj->SetBinContent(ibin,0);
    hProj->ComputeIntegral();
    
    TArrayD xval(nbins+1);
    double scale = 1.0/(double)(nbins);
    int ix = 0;
    xval[ix++] = minx;
    ibin = hProj->GetXaxis()->FindBin(minx);
    int prevHistBin = 0;
    while(ix<=nbins && ibin<=hProj->GetNbinsX())
    {
        //find bin where integral includes ix*scale fraction of distribution
        while(hProj->GetIntegral()[ibin]<scale*ix && ibin <= hProj->GetNbinsX()) ibin++;
        
        if(ibin==prevHistBin){
            //We need to force a bin since our scale demands a finer binning than in the original histogram
            ibin++;
            xval[ix]= hProj->GetXaxis()->GetBinLowEdge(ibin);
            prevHistBin = ibin;
        }else if(ix==nbins){
            xval[ix]= hProj->GetXaxis()->GetBinUpEdge(ibin);
            prevHistBin = ibin;
        }else{
            xval[ix]= hProj->GetXaxis()->GetBinLowEdge(ibin);
            prevHistBin = ibin;
        }
        std::cout<<ix<<" : "<<xval[ix]<<std::endl;
        ix++;
    }
    
    TString hNScoreRebin(hName);
    hNScoreRebin+="_Rebin";
    
	TH2* h2D = new TH2D(hNScoreRebin,hNScoreRebin,xval.GetSize()-1,xval.GetArray(),200,_minPSD,_maxPSD);
	for(int xbin = 1; xbin <= h2DOrig->GetNbinsX(); xbin++)
	{
		double xval1 = h2DOrig->GetXaxis()->GetBinCenter(xbin);
		for(int ybin = 1; ybin <= h2DOrig->GetNbinsY(); ybin++)
		{
			double yval = h2DOrig->GetYaxis()->GetBinCenter(ybin);
			h2D->Fill(xval1,yval, h2DOrig->GetBinContent(xbin,ybin));
		}
	}
    GetParentFolder()->Add(h2D);
    
    CalculateFOM1(h2D);
    
	// Now find the sigam, means, and chi2
	char cbuf[1024];
	TH1* hNnorm = (TH1*)(histDir->FindObjectAny(hNScoreRebin+"_0"));
    hNnorm->SetName(hNScoreRebin+"_Nnorm");
	TH1* hNmean = (TH1*)(histDir->FindObjectAny(hNScoreRebin+"_1"));
    hNmean->SetName(hNScoreRebin+"_Nmean");
	TH1* hNsig = (TH1*)(histDir->FindObjectAny(hNScoreRebin+"_2"));
    hNsig->SetName(hNScoreRebin+"_Nsigma");
    TH1* hGnorm = (TH1*)(histDir->FindObjectAny(hNScoreRebin+"_3"));
    hGnorm->SetName(hNScoreRebin+"_Gnorm");
	TH1* hGmean = (TH1*)(histDir->FindObjectAny(hNScoreRebin+"_4"));
    hGmean->SetName(hNScoreRebin+"_Gmean");
	TH1* hGsig = (TH1*)(histDir->FindObjectAny(hNScoreRebin+"_5"));
    hGsig->SetName(hNScoreRebin+"_Gsigma");
    //unused:	TH1* hchi2 = (TH1*)(histDir->FindObjectAny(hNScoreRebin+"_chi2"));
    
    //Lets resample the original histogram to establish an average x for each bin within 1 sigma
    // of each peak
    TString hMeanX_Neutron(hNScoreRebin);
    hMeanX_Neutron+="AVGX_Neutron";
    TH1* h1D_Neutron = new TH1D(hMeanX_Neutron,hMeanX_Neutron,xval.GetSize()-1,xval.GetArray());
    h1D_Neutron->SetDirectory(0);
    TH1* h1D_Neutron_Den = new TH1D(hMeanX_Neutron+"_Den",hMeanX_Neutron+"_Den",xval.GetSize()-1,xval.GetArray());
    h1D_Neutron_Den->SetDirectory(0);
	for(int xbin = 1; xbin <= h2DOrig->GetNbinsX(); xbin++)
	{
		double xval1 = h2DOrig->GetXaxis()->GetBinCenter(xbin);
        //find rebinned bin number
        double xrebin = h2D->GetXaxis()->FindBin(xval1);
        double nsigma = fabs(hNsig->GetBinContent(xrebin));
        double nmean = hNmean->GetBinContent(xrebin);
		for(int ybin = 1; ybin <= h2DOrig->GetNbinsY(); ybin++)
		{
            
			double yval = h2DOrig->GetYaxis()->GetBinCenter(ybin);
            double normalized_yval = (yval-nmean)/nsigma;
            if(fabs(normalized_yval)<1.0)
            {
                h1D_Neutron->Fill(xval1, h2DOrig->GetBinContent(xbin,ybin)*xval1);
                h1D_Neutron_Den->Fill(xval1, h2DOrig->GetBinContent(xbin,ybin));
            }
		}
	}
    h1D_Neutron->Divide(h1D_Neutron_Den);
    h1D_Neutron_Den->Delete();
    
    GetParentFolder()->Add(h1D_Neutron);
    
    TString hMeanX_Gamma(hNScoreRebin);
    hMeanX_Gamma+="AVGX_Gamma";
    TH1* h1D_Gamma = new TH1D(hMeanX_Gamma,hMeanX_Gamma,xval.GetSize()-1,xval.GetArray());
    h1D_Gamma->SetDirectory(0);
    TH1* h1D_Gamma_Den = new TH1D(hMeanX_Gamma+"_Den",hMeanX_Gamma+"_Den",xval.GetSize()-1,xval.GetArray());
    h1D_Gamma_Den->SetDirectory(0);
	for(int xbin = 1; xbin <= h2DOrig->GetNbinsX(); xbin++)
	{
		double xval1 = h2DOrig->GetXaxis()->GetBinCenter(xbin);
        //find rebinned bin number
        double xrebin = h2D->GetXaxis()->FindBin(xval1);
        double nsigma = fabs(hGsig->GetBinContent(xrebin));
        double nmean = hGmean->GetBinContent(xrebin);
		for(int ybin = 1; ybin <= h2DOrig->GetNbinsY(); ybin++)
		{
            
			double yval = h2DOrig->GetYaxis()->GetBinCenter(ybin);
            double normalized_yval = (yval-nmean)/nsigma;
            if(fabs(normalized_yval)<1.0)
            {
                h1D_Gamma->Fill(xval1, h2DOrig->GetBinContent(xbin,ybin)*xval1);
                h1D_Gamma_Den->Fill(xval1, h2DOrig->GetBinContent(xbin,ybin));
            }
		}
	}
    h1D_Gamma->Divide(h1D_Gamma_Den);
    h1D_Gamma_Den->Delete();
    GetParentFolder()->Add(h1D_Gamma);
    
    //Now lets create a TGraphErrors
    TString tfGmeanName(hNScoreRebin);
    tfGmeanName+="_Gmean";
    TSpline3* csGmean = MakeCubicSpline(tfGmeanName,h1D_Gamma,hGmean);
    
    //Now lets create a TGraphErrors
    TString tfGsigName(hNScoreRebin);
    tfGsigName+="_Gsig";
    TSpline3* csGsig = MakeCubicSpline(tfGsigName,h1D_Gamma,hGsig);
    
    //Now lets create a TGraphErrors
    TString tfNmeanName(hNScoreRebin);
    tfNmeanName+="_Nmean";
    TSpline3* csNmean = MakeCubicSpline(tfNmeanName,h1D_Neutron,hNmean);
    //Now lets create a TGraphErrors
    TString tfNsigName(hNScoreRebin);
    tfNsigName+="_Nsig";
    TSpline3* csNsig = MakeCubicSpline(tfNsigName,h1D_Neutron,hNsig);
    
    NGMHitPSD* psd = new NGMHitPSD();
    psd->SetNameTitle( TString(detectorPrefix)+"_PSD",
                      TString(detectorPrefix)+"_PSD");
    psd->SetCalibration(csGmean, csGsig, csNmean, csNsig);
    if(!ADCorE)//ADCorE=True: ADC ADCorE=False: E
        psd->adcore = 1;
    GetParentFolder()->Add(psd);
    
    // We create three curves:
    // 1 Not gamma upper bound
    // 2 Neutron Lower bound
    // 3 Neutron Upper bound
    sprintf(cbuf,"%s_NotGammaUpper",h2DOrig->GetName());
    TH1* hNotGammaUpper = (TH1*) (hNnorm->Clone(cbuf));
    hNotGammaUpper->Reset();
    sprintf(cbuf,"%s_NeutronLower",h2DOrig->GetName());
    TH1* hNeutronLower = (TH1*) (hNnorm->Clone(cbuf));
    hNeutronLower->Reset();
    sprintf(cbuf,"%s_NeutronUpper",h2DOrig->GetName());
    TH1* hNeutronUpper = (TH1*) (hNnorm->Clone(cbuf));
    hNeutronUpper->Reset();
    
    
    hNotGammaUpper->Add(hGmean);
    hNotGammaUpper->Add(hGsig,-5.0);
    
    hNeutronLower->Add(hNmean);
    hNeutronLower->Add(hNsig,-1.2);
    
    hNeutronUpper->Add(hNmean);
    hNeutronUpper->Add(hNsig,3.0);
    
    
    
    
    // Create the graphical 2D cut
    int npoints = 0;
    sprintf(cbuf,"%s_NCut",h2DOrig->GetName());
    TCutG* tcut = new TCutG;
    tcut->SetName(cbuf);
    GetParentFolder()->Add(tcut);
    ibin = 0;
    // First find where the neutron upper is above the !gamma
    for(ibin = 1; ibin <= h2D->GetNbinsX(); ibin++)
    {
        if(hNotGammaUpper->GetBinContent(ibin)< hNeutronUpper->GetBinContent(ibin))
            break;
    }
    int firstbin = ibin;
    double lowervalue = hNotGammaUpper->GetBinContent(ibin) > hNeutronLower->GetBinContent(ibin) ?
    hNotGammaUpper->GetBinContent(ibin) : hNeutronLower->GetBinContent(ibin);
    
    tcut->SetPoint(npoints++, hNeutronLower->GetXaxis()->GetBinLowEdge(ibin),lowervalue);
    
    // Lets add points up to maximum
    for(ibin = firstbin; ibin <= h2D->GetNbinsX(); ibin++)
    {
        lowervalue = hNotGammaUpper->GetBinContent(ibin) > hNeutronLower->GetBinContent(ibin) ?
        hNotGammaUpper->GetBinContent(ibin) : hNeutronLower->GetBinContent(ibin);
        tcut->SetPoint(npoints++, hNeutronLower->GetXaxis()->GetBinCenter(ibin),lowervalue);
    }
    ibin--;
    // Now lets turn the corner
    lowervalue = hNotGammaUpper->GetBinContent(ibin) > hNeutronLower->GetBinContent(ibin) ?
    hNotGammaUpper->GetBinContent(ibin) : hNeutronLower->GetBinContent(ibin);
    tcut->SetPoint(npoints++, hNeutronLower->GetXaxis()->GetBinUpEdge(ibin),lowervalue);
    tcut->SetPoint(npoints++, hNeutronLower->GetXaxis()->GetBinUpEdge(ibin),hNeutronUpper->GetBinContent(ibin));
    
    // Lets work our way down the upper edge
    for(; ibin >= firstbin; ibin--)
    {
        tcut->SetPoint(npoints++, hNeutronUpper->GetXaxis()->GetBinCenter(ibin),hNeutronUpper->GetBinContent(ibin));
    }
    ibin++;
    // Lets add the lower edge of the first bin
    tcut->SetPoint(npoints++, hNeutronUpper->GetXaxis()->GetBinLowEdge(ibin),hNeutronUpper->GetBinContent(ibin));
    
    // Then close the loop
    tcut->SetPoint(npoints++,tcut->GetX()[0], tcut->GetY()[0]);
    
    //  delete h2D;
    //  delete hNnorm;
    //  delete hNmean;
    //  delete hNsig;
    //  delete hGnorm;
    //  delete hGmean;
    //  delete hGsig;
    //  delete hchi2;
    
    cCanvas->cd(1);
    h2D->Draw("colz");
    tcut->Draw("LSAME");
    hNmean->SetLineColor(kBlue);
    hGmean->SetLineColor(kRed);
    
    
    cCanvas->cd(2);
    hNeutronUpper->SetLineColor(kBlue);
    hNeutronUpper->Draw();
    hNeutronLower->SetLineColor(kBlack);
    hNeutronLower->Draw("SAME");
    hNotGammaUpper->SetLineColor(kRed);
    hNotGammaUpper->Draw("SAME");
    
    return true;
}

bool NGMPSDMaker::CalculateFOM1(TH2* hNScore)
{
    if(!hNScore) return false;
    
    int gbase = 3;
    int nbase = 0;
    
    TF1* fFOM1 = new TF1("fFOM1","gaus(0)+gaus(3)",0.0,1.0);
    fFOM1->SetParameter(0,hNScore->GetMaximum()/2.0); // gaussian peak height gamma
    fFOM1->SetParameter(3,hNScore->GetMaximum()/10.0); // gaussian peak height neutron
    fFOM1->SetParameter(1,hNScore->GetMean(2) - hNScore->GetRMS(2)); // mean of gamma
    fFOM1->SetParameter(4,hNScore->GetMean(2) + hNScore->GetRMS(2)); // mean of neutrons
    fFOM1->SetParameter(2,hNScore->GetRMS(2)*0.1); //width of gammas
    fFOM1->SetParameter(5,hNScore->GetRMS(2)*0.1); //width of neutrons
    
    fFOM1->SetParLimits(0,0.0,hNScore->GetMaximum()*1.1); // gaussian peak height gamma
    fFOM1->SetParLimits(3,0.0,hNScore->GetMaximum()*1.1); // gaussian peak height neutron
    fFOM1->SetParLimits(1,0.0,hNScore->GetMean(2)*4.0); // mean of gamma
    fFOM1->SetParLimits(4,0.0,hNScore->GetMean(2)*4.0); // mean of neutrons
    fFOM1->SetParLimits(2,0.0,hNScore->GetRMS(2)); //width of gammas
    fFOM1->SetParLimits(5,0.0,hNScore->GetRMS(2)); //width of neutrons
    
    // For now this fits each slice in y with a double gaussian
    hNScore->FitSlicesY(fFOM1,1,0,0);
    char cbuff[1024];
    TH1D* hPar[7];
    // retrive the histograms
    for(int ipar = 0; ipar < 7; ipar++){
        sprintf(cbuff,"%s_%d",hNScore->GetName(),ipar);
        if(ipar == 6)
            sprintf(cbuff,"%s_%s",hNScore->GetName(),"chi2");
        hPar[ipar] = (TH1D*) (gDirectory->FindObjectAny(cbuff));
        if(!hPar[ipar]){
            LOG<<" Cannot find histogram from fitted slices for "<<hNScore->GetName()<<ENDM_WARN;
            return false;
        }else{
            hPar[ipar]->SetDirectory(0);
            GetParentFolder()->Add(hPar[ipar]);
        }
    }
    // Redo with manual slice by slice fit
    if(1){
        // Not sure why but the fit slices doesnt seem to work very well
        // The main problem is that since we have good statistics for the first bin
        // its better to let the parameters evolve as we go to large energy
        // The behavior of FitSlicesY is to reset the fit parameters each
        // time which has its advantages
        // But it does create all our histograms
        // Well think more about this later...
        //fFOM1->SetParameter(0,hNScore->GetMaximum()/2.0); // gaussian peak height gamma
        //fFOM1->SetParameter(3,hNScore->GetMaximum()/2.0); // gaussian peak height neutron
        //fFOM1->SetParameter(1,hNScore->GetMean(2) - hNScore->GetRMS(2)); // mean of gamma
        //fFOM1->SetParameter(4,hNScore->GetMean(2) + hNScore->GetRMS(2)); // mean of neutrons
        //fFOM1->SetParameter(2,hNScore->GetRMS(2)*0.1); //width of gammas
        //fFOM1->SetParameter(5,hNScore->GetRMS(2)*0.1); //width of neutrons
        //
        //fFOM1->SetParLimits(0,0.0,hNScore->GetMaximum()); // gaussian peak height gamma
        //fFOM1->SetParLimits(3,0.0,hNScore->GetMaximum()); // gaussian peak height neutron
        //fFOM1->SetParLimits(1,0.0,hNScore->GetMean(2)*4.0); // mean of gamma
        //fFOM1->SetParLimits(4,0.0,hNScore->GetMean(2)*4.0); // mean of neutrons
        //fFOM1->SetParLimits(2,0.0,hNScore->GetRMS(2)); //width of gammas
        //fFOM1->SetParLimits(5,0.0,hNScore->GetRMS(2)); //width of neutrons
        
        fFOM1->SetParameters(5000,0.05,0.02,500,0.2,0.02);
        
        int startBin = hNScore->GetNbinsX()*0.75;
        
        // Lets redo the fitting projection by projection.
        double dblJunk=0;
        int intJunk=0;
        for(int ibin = startBin; ibin <= hPar[0]->GetNbinsX(); ibin++){
            TH1* htmp = hNScore->ProjectionY("_temp",ibin,ibin,"e");
            
            
            if(ibin==startBin){
                fFOM1->SetParameter(0,htmp->GetMaximum()/2.0); // gaussian peak height gamma
                fFOM1->SetParameter(3,htmp->GetMaximum()/2.0); // gaussian peak height neutron
                fFOM1->SetParameter(1,htmp->GetMean() - htmp->GetRMS()); // mean of gamma
                fFOM1->SetParameter(4,htmp->GetMean() + htmp->GetRMS()); // mean of neutrons
                fFOM1->SetParameter(2,htmp->GetRMS()*0.01); //width of gammas
                fFOM1->SetParameter(5,htmp->GetRMS()*0.1); //width of neutrons
                TSpectrum ts;
                ts.Search(htmp,2,"",0.02);
                if(ts.GetNPeaks()==2)
                {
                    double x1 = ts.GetPositionX()[0];
                    double x2 = ts.GetPositionX()[1];
                    if(x1>x2)
                    {
                        fFOM1->SetParameter(1,x2); // mean of neutrons
                        fFOM1->SetParameter(4,x1); // mean of gammas
                        fFOM1->SetParameter(0,ts.GetPositionY()[1]); // height of neutrons
                        fFOM1->SetParameter(3,ts.GetPositionY()[0]); // height of gammas
                    }else{
                        fFOM1->SetParameter(1,x1); // mean of gamma
                        fFOM1->SetParameter(4,x2); // mean of neutrons
                        fFOM1->SetParameter(0,ts.GetPositionY()[0]); // height of gammas
                        fFOM1->SetParameter(3,ts.GetPositionY()[1]); // height of neutrons
                    }
                }else{
                    std::cout<<"Unable to find 2 peaks fitting slice "<<ibin<<". Instead found "<<ts.GetNPeaks()<<std::endl;
                }
            }else{
                fFOM1->SetParameter(0,hPar[0]->GetBinContent(ibin-1));
                fFOM1->SetParameter(1,hPar[1]->GetBinContent(ibin-1));
                fFOM1->SetParameter(2,hPar[2]->GetBinContent(ibin-1));
                fFOM1->SetParameter(3,hPar[3]->GetBinContent(ibin-1));
                fFOM1->SetParameter(4,hPar[4]->GetBinContent(ibin-1));
                fFOM1->SetParameter(5,hPar[5]->GetBinContent(ibin-1));
            }
            std::cout<<"Fitting bin: "<<ibin<<" "<<hNScore->GetXaxis()->GetBinLowEdge(ibin);
            for(int ipar = 0; ipar<6;ipar++)
                std::cout<<" : "<<fFOM1->GetParameter(ipar);
            std::cout<<std::endl;
            htmp->Fit(fFOM1,"N");
            
            if(0)if(TVirtualFitter::GetFitter()->GetStats(dblJunk,dblJunk,dblJunk,intJunk,intJunk)!=0){
              std::cout<<"Fitting error on bin "<<ibin<<std::endl;
                TSpectrum ts;
                ts.Search(htmp,2,"",0.02);
                if(ts.GetNPeaks()==2)
                {
                    fFOM1->SetParameter(2,hPar[2]->GetBinContent(ibin-1));
                    fFOM1->SetParameter(5,hPar[5]->GetBinContent(ibin-1));
                    double x1 = ts.GetPositionX()[0];
                    double x2 = ts.GetPositionX()[1];
                    if(x1>x2)
                    {
                        fFOM1->SetParameter(1,x2); // mean of neutrons
                        fFOM1->SetParameter(4,x1); // mean of gammas
                        fFOM1->SetParameter(0,ts.GetPositionY()[1]); // height of neutrons
                        fFOM1->SetParameter(3,ts.GetPositionY()[0]); // height of gammas
                    }else{
                        fFOM1->SetParameter(1,x1); // mean of gamma
                        fFOM1->SetParameter(4,x2); // mean of neutrons
                        fFOM1->SetParameter(0,ts.GetPositionY()[0]); // height of gammas
                        fFOM1->SetParameter(3,ts.GetPositionY()[1]); // height of neutrons
                    }
                }else{
                    std::cout<<"Unable to find 2 peaks fitting slice "<<ibin<<". Instead found "<<ts.GetNPeaks()<<std::endl;
                }
                htmp->Fit(fFOM1,"N");
              if(TVirtualFitter::GetFitter()->GetStats(dblJunk,dblJunk,dblJunk,intJunk,intJunk)!=0)std::cout<<"Second failed"<<std::endl;
            }
            
            //TCanvas* myc = new TCanvas();
            //htmp->DrawCopy()->SetDirectory(0);
            //fFOM1->DrawCopy("same");
            //myc->Update();
            // Save the results
            if(htmp->GetEntries()>1)
            {
                std::cout<<"Gbase "<<gbase << " Nbase"<< nbase<<std::endl;
                // Compare gaussian means... Neutrons should be the smaller
                if(fFOM1->GetParameter(nbase+1) < fFOM1->GetParameter(gbase+1))
                {
                    int nbasenew = gbase;
                    gbase = nbase;
                    nbase=nbasenew;
                }
                std::cout<<"Gbase "<<gbase << " Nbase"<< nbase<<std::endl;
                for(int ipar = 0; ipar < 3; ipar++)
                {
                    hPar[gbase+ipar]->SetBinContent(ibin,fFOM1->GetParameter(ipar));
                    hPar[gbase+ipar]->SetBinError(ibin,fFOM1->GetParError(ipar));
                    hPar[nbase+ipar]->SetBinContent(ibin,fFOM1->GetParameter(ipar+3));
                    hPar[nbase+ipar]->SetBinError(ibin,fFOM1->GetParError(ipar+3));
                }
                hPar[6]->SetBinContent(ibin, fFOM1->GetChisquare());
                
                std::cout<<"Saving bin: "<<ibin<<" "<<hNScore->GetXaxis()->GetBinLowEdge(ibin);
                for(int ipar = 0; ipar<6;ipar++)
                    std::cout<<" : "<<fFOM1->GetParameter(ipar);
                std::cout<<std::endl;
                std::cout<<"Retrieving bin: "<<ibin<<" "<<hNScore->GetXaxis()->GetBinLowEdge(ibin);
                for(int ipar = 0; ipar<6;ipar++)
                    std::cout<<" : "<<hPar[ipar]->GetBinContent(ibin);
                std::cout<<std::endl;
            }
            //delete htmp;
        }
        // Lets Work our way back
        fFOM1->SetParameter(0,hPar[0]->GetBinContent(startBin));
        fFOM1->SetParameter(1,hPar[1]->GetBinContent(startBin));
        fFOM1->SetParameter(2,hPar[2]->GetBinContent(startBin));
        fFOM1->SetParameter(3,hPar[3]->GetBinContent(startBin));
        fFOM1->SetParameter(4,hPar[4]->GetBinContent(startBin));
        fFOM1->SetParameter(5,hPar[5]->GetBinContent(startBin));
        
        for(int ibin = startBin-1; ibin >=1; ibin--){
            //fFOM1->SetParameters(5000,0.05,0.02,500,0.2,0.02);//AMG
            TH1* htmp = hNScore->ProjectionY("_temp",ibin,ibin,"e");
            
            //Make a better guess
            if(0){
                fFOM1->SetParameter(0,htmp->GetMaximum()/2.0); // gaussian peak height gamma
                fFOM1->SetParameter(3,htmp->GetMaximum()/2.0); // gaussian peak height neutron
                fFOM1->SetParameter(1,htmp->GetMean() - htmp->GetRMS()); // mean of gamma
                fFOM1->SetParameter(4,htmp->GetMean() + htmp->GetRMS()); // mean of neutrons
                fFOM1->SetParameter(2,htmp->GetRMS()*0.1); //width of gammas
                fFOM1->SetParameter(5,htmp->GetRMS()*0.1); //width of neutrons
                
                TSpectrum ts;
                ts.Search(htmp,4.0,"",0.01);
                if(ts.GetNPeaks()==2)
                {
                    double x1 = ts.GetPositionX()[0];
                    double x2 = ts.GetPositionX()[1];
                    if(x1>x2)
                    {
                        fFOM1->SetParameter(1,x2); // mean of neutrons
                        fFOM1->SetParameter(4,x1); // mean of gammas
                        fFOM1->SetParameter(0,ts.GetPositionY()[1]); // height of neutrons
                        fFOM1->SetParameter(3,ts.GetPositionY()[0]); // height of gammas
                    }else{
                        fFOM1->SetParameter(1,x1); // mean of gamma
                        fFOM1->SetParameter(4,x2); // mean of neutrons
                        fFOM1->SetParameter(0,ts.GetPositionY()[0]); // height of gammas
                        fFOM1->SetParameter(3,ts.GetPositionY()[1]); // height of neutrons
                    }
                }else{
                    std::cout<<"Unable to find 2 peaks fitting slice "<<ibin<<". Instead found "<<ts.GetNPeaks()<<std::endl;
                }
            }
            htmp->Fit(fFOM1,"N");
            //Test for refit
            if(0){
                if(TVirtualFitter::GetFitter()->GetStats(dblJunk,dblJunk,dblJunk,intJunk,intJunk)!=3){
                  std::cout<<"Fitting error on bin "<<ibin<<std::endl;
                    fFOM1->SetParameter(0,hPar[0]->GetBinContent(ibin+1));
                    fFOM1->SetParameter(1,hPar[1]->GetBinContent(ibin+1));
                    fFOM1->SetParameter(2,hPar[2]->GetBinContent(ibin+1));
                    fFOM1->SetParameter(3,hPar[3]->GetBinContent(ibin+1));
                    fFOM1->SetParameter(4,hPar[4]->GetBinContent(ibin+1));
                    fFOM1->SetParameter(5,hPar[5]->GetBinContent(ibin+1));
                    fFOM1->SetParameters(5000,0.05,0.02,500,0.2,0.02);
                    htmp->Fit(fFOM1,"N");
                  if(TVirtualFitter::GetFitter()->GetStats(dblJunk,dblJunk,dblJunk,intJunk,intJunk)!=3)std::cout<<"Second failed"<<std::endl;
                }
            }
            //TCanvas* myc = new TCanvas();
            //htmp->DrawCopy()->SetDirectory(0);
            //fFOM1->DrawCopy("same");
            //myc->Update();
            // Save the results
            if(htmp->GetEntries()>1)
            {
                // Compare gaussian means... Neutrons should be the smaller
                if(fFOM1->GetParameter(nbase+1) < fFOM1->GetParameter(gbase+1))
                {
                    int nbasenew = gbase;
                    gbase = nbase;
                    nbase=nbasenew;
                }
                
                for(int ipar = 0; ipar < 3; ipar++)
                {
                    hPar[gbase+ipar]->SetBinContent(ibin,fFOM1->GetParameter(ipar));
                    hPar[gbase+ipar]->SetBinError(ibin,fFOM1->GetParError(ipar));
                    hPar[nbase+ipar]->SetBinContent(ibin,fFOM1->GetParameter(ipar+3));
                    hPar[nbase+ipar]->SetBinError(ibin,fFOM1->GetParError(ipar+3));
                }
                hPar[6]->SetBinContent(ibin, fFOM1->GetChisquare());
            }
            delete htmp;
        }
    }//Redo slice by slice
    
    // Clone a copy to put the FOM Calculation in
    sprintf(cbuff,"%s_FOM1",hNScore->GetName());
    TH1D* hFOM = (TH1D*)( hPar[4]->Clone(cbuff) );
    hFOM->SetTitle(cbuff);
    hFOM->SetDirectory(0);
    GetParentFolder()->Add(hFOM);
    sprintf(cbuff,"%s_FOMDen",hNScore->GetName());
    TH1D* hFOMD = (TH1D*)( hPar[2]->Clone(cbuff) );
    hFOMD->SetDirectory(0);
    GetParentFolder()->Add(hFOMD);
    hFOMD->SetTitle(cbuff);
    hFOM->Add(hPar[1], -1.0);
    hFOMD->Add(hPar[5]);
    hFOM->Divide(hFOMD);
    // The FOM uses width not sigma so divide by two
    hFOM->Scale(1.0/sqrt(8.0*log(2.0)));
    //*hFOM = (*(hPar[4]) - *(hPar[1]))/(*(hPar[2])+*(hPar[5]));
    
    //  GetParentFolder()->Add(hFOM);
    
    LOG<<"Saving FOM in histogram "<<hFOM->GetName()<<" in "<<GetParentFolder()->GetName()<<ENDM_INFO;
    //  delete hFOMD;
    //  delete fFOM1;
    
    return true;
}

TSpline3* NGMPSDMaker::MakeCubicSpline(const char* name, TH1* hX, TH1* hY)
{
    TString tgname(name);
    tgname+="_tg";
    TGraphErrors* tfe = new TGraphErrors(hX->GetNbinsX());
    tfe->SetNameTitle(tgname,tgname);
    GetParentFolder()->Add(tfe);
    int igpoint = 0;
    for(int ibin = 1; ibin <=tfe->GetN();ibin++)
    {
        if(hX->GetBinContent(ibin)==0.0) continue;
        tfe->SetPoint(igpoint,hX->GetBinContent(ibin),hY->GetBinContent(ibin));
        tfe->SetPointError(igpoint,0.0,hY->GetBinError(ibin));
        igpoint++;
    }
    //Now lets extend the first bin to the bin lower edge with a linear extrapolation
    if(0){
        double slope = (tfe->GetY()[2]- tfe->GetY()[1])/(tfe->GetX()[2]- tfe->GetX()[1]);
        double y_intercept = tfe->GetY()[1]-tfe->GetX()[1]*slope;
        tfe->SetPoint(0,0.0,y_intercept);
        tfe->SetPointError(0,0.0,0.0);
    }
    //A linear extrapolation is not a very good choice!
    if(0){
        int lp = tfe->GetN()-2;
        double xt = hY->GetXaxis()->GetXmax();
        double slope = (tfe->GetY()[lp]- tfe->GetY()[lp-1])/(tfe->GetX()[lp]- tfe->GetX()[lp-1]);
        tfe->SetPoint(lp+1,xt,tfe->GetY()[lp]+(xt-tfe->GetX()[lp])*slope);
        tfe->SetPointError(lp+1,0.0,0.0);
    }
    
    TF1* fit = 0;
    TString prevFitter = TVirtualFitter::GetDefaultFitter();
    if(tgname.Contains("Mean"))
    {
        TF1* fit1 = new TF1("fit1","[0]+[1]*exp(-(x-[3])/[2])",0,6000.0);
        fit1->SetParameter(0,tfe->GetY()[tfe->GetN()-1]);
        fit1->SetParameter(1,tfe->GetY()[0]-tfe->GetY()[tfe->GetN()-1]);
        fit1->SetParameter(2,2000.0);
        fit1->SetParameter(3,0.0);
        tfe->Fit(fit1);
        fit = fit1;
    }else{
        double lx = tfe->GetX()[tfe->GetN()-1];
        double ly = tfe->GetY()[tfe->GetN()-1];
        double fx = tfe->GetX()[0];
        double fy = tfe->GetY()[0];
        double fitbegin=fx;
        double xoffsetguess=0.0;//fitbegin/2.0;
        double yrange=(fy-ly);
        double yoffsetguess=ly-0.1*yrange;
        double kguess = log((ly-yoffsetguess)/(fy-yoffsetguess))/log((lx-xoffsetguess)/(fx-xoffsetguess));
        double ampguess = (ly-yoffsetguess)/pow(lx-xoffsetguess,kguess);
        TF1* fit2 = new TF1("fit2","[0]+[1]*(x-[3])**([2])",100,6000.0);
        fit2->SetParameter(0,yoffsetguess);
        fit2->SetParameter(1,ampguess);
        fit2->SetParameter(2,kguess);
        fit2->SetParameter(3,xoffsetguess);
        std::cout<<"Guess for ("<<fx<<","<<fy<<") ("<<lx<<","<<ly<<")"<<std::endl;
        for (int ipar = 0; ipar<4; ipar++)
            std::cout<<"["<<ipar<<"] "<<fit2->GetParameter(ipar)<<std::endl;
        tfe->Fit(fit2);
        TFitter::SetMaxIterations(5000);
        fit = fit2;
    }
    TString csname(name);
    csname+="_cs";
    TSpline3* cs = new TSpline3(csname,tfe->GetX()[0],8000.0,fit,1000.0);
    cs->SetName(csname);
    GetParentFolder()->Add(cs);
    return cs;
}

