//
//  NGMPixelADCMonitor.cpp
//  NGMDaq
//
//  Created by Newby, Robert Jason on 7/8/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMPixelADCMonitor.h"

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
#include "TSpectrum.h"
#include "TFile.h"
#include "TH2D.h"
#include "NGMBlockMapping.h"

ClassImp(NGMPixelADCMonitor)

NGMPixelADCMonitor::NGMPixelADCMonitor()
{
  InitCommon();
}

NGMPixelADCMonitor::NGMPixelADCMonitor(const char* name, const char* title)
: NGMModule(name,title)
{
  InitCommon();
  partID = new NGMSimpleParticleIdent;
}

void NGMPixelADCMonitor::InitCommon()
{
  blocknrows = 0;
  blockncols = 0;
  energyGate = 4;
  maxADC =4000;
  _blockArray=0;
  _hEnergyvsPixel = 0;
  _hPulseHeightvsPixel = 0;
  _tspectrumwidth = 20.0;
  _tspectrumpeakdynamicrange = 0.5;
}

NGMPixelADCMonitor::~NGMPixelADCMonitor()
{
  
}

bool NGMPixelADCMonitor::init()
{
  return true;
}

void NGMPixelADCMonitor::Print(Option_t* option) const
{
  LOG<<"Name:"<<GetName()<<ENDM_INFO;
  LOG<<"Blocknrows:"<<blocknrows<<ENDM_INFO;
  LOG<<"Blockncols:"<<blockncols<<ENDM_INFO;
  LOG<<"EnergyGate:"<<energyGate<<ENDM_INFO;
  LOG<<"MaxADC:"<<maxADC<<ENDM_INFO;
  //LOG<<"ParentFolder:"<<GetParentFolder()->GetName()<<ENDM_INFO;
  LOG<<ENDM_INFO;

}

bool NGMPixelADCMonitor::process(const TObject& tData)
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
    
    //Lets loop through each of the channels groups and determine the Block row, column, and pixelLUT
    blocknrows = confBuffer->GetSystemParameters()->GetParValueI("BLOCKNROWS",0);
    blockncols = confBuffer->GetSystemParameters()->GetParValueI("BLOCKNCOLS",0);
      NGMBlockMapping bmap;
      bmap.Init(confBuffer);
    int npix = blocknrows*blockncols*bmap.GetNPixOnASide()*bmap.GetNPixOnASide();
    if(!_hEnergyvsPixel){
      TString hName(GetName());
      hName+="_EnergyvsPixel";
      _hEnergyvsPixel = new TH2F(hName,hName,npix,0,npix,8000,0,8000.0);
      _hEnergyvsPixel->SetDirectory(0);
      GetParentFolder()->Add(_hEnergyvsPixel);
    }else{
      _hEnergyvsPixel->SetBins(npix,0.0,npix,8000,0,8000.0);
    }      
    if(!_hPulseHeightvsPixel){
      TString hName(GetName());
      hName+="_PulseHeightvsPixel";
      _hPulseHeightvsPixel = new TH2F(hName,hName,npix,0,npix,maxADC,0,maxADC);
      _hPulseHeightvsPixel->SetDirectory(0);
      GetParentFolder()->Add(_hPulseHeightvsPixel);
    }else{
      _hPulseHeightvsPixel->SetBins(npix,0,npix,maxADC,0,maxADC);
    }

   _blockRowCol.ResizeTo(confBuffer->GetChannelParameters()->GetEntries()/4,2);
    LOG<<" Using block detector array of "<<blocknrows<<" rows by "<<blockncols<<" columns." << ENDM_INFO;

    for(int ichan = 0; ichan < confBuffer->GetChannelParameters()->GetEntries(); ichan+=4)
    {
      int iblock = ichan/4;
      TString detName(confBuffer->GetChannelParameters()->GetParValueS("DetectorName",ichan));
      if(bmap.IsBlock(ichan))
      {
        int detRow = confBuffer->GetDetectorParameters()->FindFirstRowMatching("DetectorName",detName);
        const TH2* hFlood = 0;
        if(detRow>=0)
        {
          _blockRowCol(iblock,0) = confBuffer->GetDetectorParameters()->GetParValueI("BLOCK_ROW",detRow);
          _blockRowCol(iblock,1) = confBuffer->GetDetectorParameters()->GetParValueI("BLOCK_COL",detRow);
          LOG<<"Assigning detector "<<detName.Data()
          <<" to position ("<<_blockRowCol(iblock,0)
          <<","<<_blockRowCol(iblock,1)<<")"<<ENDM_INFO;
        }
      }
    }

    
    partID->Init(confBuffer);
    if(!_blockArray){
      _blockArray  = new TObjArray(blocknrows*10*blockncols*10);
    }
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

bool NGMPixelADCMonitor::analyzeHit(const NGMHit* tHit)
{
  int thisPix = tHit->GetPixel();

  // Can't do anything with an undetermined pixel
  if(thisPix<0) return false;
  _hEnergyvsPixel->Fill(thisPix,tHit->GetEnergy());
  _hPulseHeightvsPixel->Fill(thisPix,tHit->GetPulseHeight());
  if(0){
    TH1* htemp = dynamic_cast<TH1*>(_blockArray->At(tHit->GetPixel()));
    if(!htemp)
    {
      TString hname;
      hname.Form("%s_PixelADC_%05d",GetName(),tHit->GetPixel());
      htemp = new TH1D(hname,hname,maxADC,0,maxADC);
      htemp->SetDirectory(0);
      GetParentFolder()->Add(htemp);
      _blockArray->AddAt(htemp,tHit->GetPixel());
    }

    htemp->Fill(tHit->GetPulseHeight());

  }
  
  return true;
}


bool  NGMPixelADCMonitor::finish()
{
  return true;
}

void  NGMPixelADCMonitor::LaunchDisplayCanvas()
{
  return;
}


void  NGMPixelADCMonitor::ResetHistograms()
{
  for(int ihist = 0; ihist<= _blockArray->GetLast();ihist++)
  {
    if(_blockArray->At(ihist)) ((TH1*)(_blockArray->At(ihist)))->Reset();
  }
}

double NGMPixelADCMonitor::FindComptonEdge(TH1* hADC, int filterScale, double tspectrumWidth, double tspectrumDynamicRange)
{
  // First Find the compton edge from spectra
  
  TH1* tFilter = 0;
  if(!tFilter){
    tFilter = (TH1*) hADC->Clone(TString(hADC->GetName())+="_filter");
    tFilter->SetDirectory(0);
  }
  tFilter->Reset();
  int trapValue;
  int minLength = 2*filterScale;
  unsigned int sum1 =0;
  unsigned int sum2 =0;
  //Begin by calculating sums with first sample = 0
  for(int isample = 1; isample <= filterScale; isample++){
    sum1+=(unsigned int)(hADC->GetBinContent(isample));
    sum2+=(unsigned int)(hADC->GetBinContent(isample + filterScale));
  }
  
  trapValue = sum2 - sum1; // + 0x10000;      
  // We invert the signal and cut everthing below 0 so as not to confuse the peak finder
  if(trapValue>0) trapValue = 0;
  tFilter->SetBinContent(filterScale + 1,-trapValue);
  
  for(int isample = 2; isample <= tFilter->GetNbinsX() - minLength; isample++){
    sum1-=(unsigned int)(hADC->GetBinContent(isample-1));
    sum2-=(unsigned int)(hADC->GetBinContent(isample-1 + filterScale));
    sum1+=(unsigned int)(hADC->GetBinContent(isample + filterScale));
    sum2+=(unsigned int)(hADC->GetBinContent(isample + 2*filterScale));
    trapValue = sum2 - sum1; // + 0x10000;
    if(trapValue>0) trapValue = 0;    
    tFilter->SetBinContent(isample + filterScale,-trapValue);
  }
  
  // Repeat last value so truncation does not look like a "peak"
  for(int isample = tFilter->GetNbinsX() - minLength; isample <=tFilter->GetNbinsX(); isample++ )
  {
	  tFilter->SetBinContent(isample+filterScale,tFilter->GetBinContent(tFilter->GetNbinsX() - minLength + filterScale));
  }
  
  
  // Use the ROOT peak finder to find the right edge of the compton plateau
  // Its only good if we find two peaks and we'll want the second. For Cs137,
  //tFilter->GetXaxis()->SetRangeUser(100.0,1000.0);

  TSpectrum peakFinder;
  peakFinder.Search(tFilter,tspectrumWidth,"",tspectrumDynamicRange); // Min separation of 20 sigma
	// The peak positions are not necessary sorted by xposition
	// find the peak at largest x
	double peakpos = 0.0;
    double maxHeight = 0.0;
	for(int ipeak = 0; ipeak < peakFinder.GetNPeaks(); ipeak++)
	{
		if(peakpos<peakFinder.GetPositionX()[ipeak]) peakpos = peakFinder.GetPositionX()[ipeak];
	}

    // Find the peak with the largest y
	//for(int ipeak = 0; ipeak < peakFinder.GetNPeaks(); ipeak++)
	//{
	//	if(maxHeight<peakFinder.GetPositionY()[ipeak]) peakpos = peakFinder.GetPositionX()[ipeak];
	//}

    
  return peakpos;
}

void NGMPixelADCMonitor::SaveComptonCalibration(NGMSystemConfiguration* conf, const char* fname, double energy, int filterScale)
{
  
  if(!(conf->GetDetectorParameters()->GetColumn("BlockEnergyCal")))
  {
    conf->GetDetectorParameters()->AddParameterO("BlockEnergyCal",0);
  }
    NGMBlockMapping bmap;
    bmap.Init(conf);
    int pIR = bmap.GetNPixOnASide();
  TString h2DName;
  h2DName.Form("%s_PulseHeightvsPixel",GetName());
  TH2* h2D = dynamic_cast<TH2*>(GetParentFolder()->FindObjectAny(h2DName.Data()));
  // Lets attempt to loop over all detector blocks...
  int totalBlocks = blocknrows*blockncols;
  for(int iblock = 0; iblock<totalBlocks; iblock++)
  {
    int ichan = iblock*4;
    TString detName(conf->GetChannelParameters()->GetParValueS("DetectorName",ichan));
    if(detName!="")
    {
      int detRow = conf->GetDetectorParameters()->FindFirstRowMatching("DetectorName",detName);
      if(detRow>=0)
      {
        int blockRow = _blockRowCol(iblock,0);
        int blockCol = _blockRowCol(iblock,1);
        TString hCalName;
        hCalName.Form("EnergyCal_%s",detName.Data());
        TH2* hCal = new TH2D(hCalName,hCalName,pIR,0,pIR,pIR,0,pIR);
        hCal->SetDirectory(0);
        for(int blockPix = 0; blockPix<pIR*pIR; blockPix++)
        {
          int globalPixCol = blockCol*pIR+blockPix%pIR;
          int globalPixRow = blockRow*pIR+blockPix/pIR;
          int gPixel = globalPixRow*blockncols*pIR + globalPixCol;
          TString hname;
          hname.Form("%s_PixelADC_%05d",GetName(),gPixel);
          TH1* hADC = h2D->ProjectionY(hname.Data(),gPixel+1,gPixel+1);
          if(!hADC)
          {
            LOG<<" Unable to find "<<hname.Data()<<ENDM_WARN;
            continue;
          }
          double edgeADC = FindComptonEdge(hADC,filterScale,_tspectrumwidth,_tspectrumpeakdynamicrange);
          hCal->SetBinContent(blockPix%pIR+1,blockPix/pIR+1,energy/edgeADC);
          LOG<<"blockCol(" <<blockCol
            <<"blockRow(" <<blockRow
            <<") BlockPix("<<blockPix
            <<") globalPixCol("<<globalPixCol
            <<") globalPixRow("<<globalPixRow
            <<") gPixel("<<gPixel
            <<") edgeADC("<<edgeADC
            <<ENDM_INFO;
          
        }
        conf->GetDetectorParameters()->SetParameterO("BlockEnergyCal",detRow,hCal);
      }
    }

  }
  TFile* newCalFile = TFile::Open(fname,"NEW");
  if(newCalFile)
  {
    newCalFile->WriteTObject(conf,"NGMSystemConfiguration");
    newCalFile->Close();
  }
}


ClassImp(NGMPixelADCMonitorDisplay)

NGMPixelADCMonitorDisplay::NGMPixelADCMonitorDisplay()
{
  cBlockDisplay = 0;
}

NGMPixelADCMonitorDisplay::NGMPixelADCMonitorDisplay(const char* moduleName)
{
  TString cName("cBlockArray_");
  _moduleName = moduleName;
  cName+=_moduleName;
  cBlockDisplay = new TCanvas(cName,cName);
  spy.Connect();
}

Bool_t NGMPixelADCMonitorDisplay::HandleTimer(TTimer* timer)
{
  TString hname;
  hname.Form("%s_PixelADC_%02d",_moduleName.Data(),_pixel);
  TH1* hHist = (TH1*)(spy.RequestObject(hname.Data()));
  if(hHist)
  {
    // Let the histogram be deleted when the canvas is cleared
    hHist->SetBit(TObject::kCanDelete);
    hHist->Draw("");
  }
  cBlockDisplay->Modified();
  cBlockDisplay->Update();
  timer->Start(1000,true);
  return true;
}

