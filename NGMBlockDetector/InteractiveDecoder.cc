#include "InteractiveDecoder.h"
#include "TStopwatch.h"
#include "TTree.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "NGMHit.h"
#include <iostream>

InteractiveDecoder::InteractiveDecoder(Int_t maskRank, Double_t maskDetectorDistance, Double_t sourceMaskDistance, Double_t maskElementSize,
                   Int_t numberPixelsX, Int_t numberPixelsY, TTree* maskTree, TTree* antiTree, TTree* voidTree)
:_imaker(maskRank,maskDetectorDistance,sourceMaskDistance,maskElementSize,numberPixelsX,numberPixelsY),
_hMask(0),
_hAnti(0),
_hVoid(0),
_image(0),
_hMaskC(0),
_hAntiC(0),
_hVoidC(0),
_maskTree(maskTree),
_antiTree(antiTree),
_voidTree(voidTree)
{
    
}

InteractiveDecoder::InteractiveDecoder(Int_t maskRank, Double_t maskElementSize, Double_t maskDetectorDistance, Double_t sourceMaskDistance,const char* imagerName, TTree* maskTree, TTree* antiTree, TTree* voidTree)
:_imaker(maskRank,maskElementSize,maskDetectorDistance,sourceMaskDistance,imagerName),
_hMask(0),
_hAnti(0),
_hVoid(0),
_image(0),
_hMaskC(0),
_hAntiC(0),
_hVoidC(0),
_maskTree(maskTree),
_antiTree(antiTree),
_voidTree(voidTree)
{
    
}
InteractiveDecoder::InteractiveDecoder()
:
//_imaker(19,100.0,100.0,1.4,40,40),
_imaker(19,1.2,100.0,115.5,"P40"),
_hMask(0),
_hAnti(0),
_hVoid(0),
_image(0),
_hMaskC(0),
_hAntiC(0),
_hVoidC(0),
_maskTree(0),
_antiTree(0),
_voidTree(0)
{

}

void InteractiveDecoder::go(Option_t* cut)
{
    
    if(!_maskTree||!_antiTree||!_voidTree) return;
    
    TString name = "Decoder";
    Int_t numberPixelsX = _imaker.GetNumPixX();
    Int_t numberPixelsY = _imaker.GetNumPixY();
    TString hVoidName;
    hVoidName.Form("%s_void",name.Data());
    TString hMaskName;
    hMaskName.Form("%s_mask",name.Data());
    TString hAntiName;
    hAntiName.Form("%s_anti",name.Data());
    
    if(!_hVoid)
    {
        _hVoid= new TH2D(hVoidName.Data(),hVoidName.Data(),
                         numberPixelsX,0,numberPixelsX,
                         numberPixelsY,0,numberPixelsY);
    }

    if(!_hMask)
    {
        _hMask= new TH2D(hMaskName.Data(),hMaskName.Data(),
                         numberPixelsX,0,numberPixelsX,
                         numberPixelsY,0,numberPixelsY);
    }

    if(!_hAnti)
    {
        _hAnti= new TH2D(hAntiName.Data(),hAntiName.Data(),
                         numberPixelsX,0,numberPixelsX,
                         numberPixelsY,0,numberPixelsY);
    }

    TCanvas* myCanvas = (TCanvas*)(gROOT->FindObjectAny("cInteractiveDecoder"));
    if(!myCanvas){
        myCanvas = new TCanvas("cInteractiveDecoder","cInteractiveDecoder");
    }
    myCanvas->Clear();
    myCanvas->Divide(2,2);
    
    Long64_t maxVoidEntries = _maskTree->GetEntries();
    maxVoidEntries*=5;
    
    TString drawCommand;
    myCanvas->cd(1);
    drawCommand.Form("_pixel/%d:_pixel%%%d>>%s",numberPixelsX,numberPixelsX,hVoidName.Data());
    //_voidTree->Draw(drawCommand.Data(),cut,"colz",maxVoidEntries);
    myCanvas->cd(2);
    drawCommand.Form("_pixel/%d:_pixel%%%d>>%s",numberPixelsX,numberPixelsX,hMaskName.Data());
    _maskTree->Draw(drawCommand.Data(),cut,"colz");
    myCanvas->cd(3);
    drawCommand.Form("_pixel/%d:_pixel%%%d>>%s",numberPixelsX,numberPixelsX,hAntiName.Data());
    _antiTree->Draw(drawCommand.Data(),cut,"colz");
    std::cout<<"Complete Histogram Filling"<<std::endl;
    std::cout<<"Begin Scaling Histograms"<<std::endl;
    _hVoid->Reset();
    _hVoid->Add(_hMask);
    _hVoid->Add(_hAnti);
    int numPix = float(_hVoid->GetNbinsX()*_hVoid->GetNbinsY());
    double scale = numPix/double(_hVoid->Integral());
    _hVoid->Scale(scale);
    _hMask->Divide(_hVoid);
    _hAnti->Divide(_hVoid);
    std::cout<<"Begin Imaging"<<std::endl;
    
    myCanvas->cd(4);
    _image = _imaker.GetImage3(_hMask,_hAnti);
    _image->Draw("colz");
    
    myCanvas->Update();
}

void InteractiveDecoder::ChangeSourceDistance(Double_t newSourceDist){

    _imaker.SetDistances(_imaker.GetDetectorMaskDistance(),newSourceDist);
    
}

void InteractiveDecoder::SetSourceDistance(Double_t newSourceDist)
{
    _imaker.SetDistances(_imaker.GetDetectorMaskDistance(),newSourceDist);
    _image = _imaker.GetImage3(_hMask,_hAnti);
    _image->Draw("colz");
    TCanvas* myCanvas = (TCanvas*)(gROOT->FindObjectAny("cInteractiveDecoder"));
    if(myCanvas) myCanvas->Update();

}
void InteractiveDecoder::FillDataCube(TTree* tree, THnI* hcube)
{
    NGMHit* hit = 0;
    tree->SetBranchAddress("HitTree",&hit);
    Long64_t nentries=tree->GetEntries();
    double x[5];
    for(int ientry = 0; ientry < nentries; ientry++)
    {
        tree->GetEntry(ientry);
        x[0]=hit->GetPixel()%40;
        x[1]=hit->GetPixel()/40;
        x[2]=hit->GetEnergy();
        x[3]=hit->GetNeutronId();
        x[4]=hit->GetGammaId();
        hcube->Fill(x);
    }
    tree->SetBranchAddress("HitTree",0);
}

void InteractiveDecoder::FillDataCubes()
{

    TString name = "Decoder";
    Int_t numberPixelsX = _imaker.GetNumPixX();
    Int_t numberPixelsY = _imaker.GetNumPixY();
    TString hVoidName;
    hVoidName.Form("%s_voidC",name.Data());
    TString hMaskName;
    hMaskName.Form("%s_maskC",name.Data());
    TString hAntiName;
    hAntiName.Form("%s_antiC",name.Data());
    Int_t nDim = 5;
    Int_t nBins[5] = {40,40,100,20,20};
    Double_t xMin[5] = {0.0,0.0,0.0,-10.0,-10.0};
    Double_t xMax[5] = {40.0,40.0,4000.0,10.0,10.0};
    
    if(!_hVoidC)
    {
        _hVoidC= new THnI(hVoidName.Data(),hVoidName.Data(),
                          nDim,nBins,xMin,xMax);
    }
    
    if(!_hMaskC)
    {
        _hMaskC= new THnI(hMaskName.Data(),hMaskName.Data(),
                          nDim,nBins,xMin,xMax);
    }
    
    if(!_hAntiC)
    {
        _hAntiC= new THnI(hAntiName.Data(),hAntiName.Data(),
                          nDim,nBins,xMin,xMax);
    }
    FillDataCube(_voidTree,_hVoidC);
    FillDataCube(_maskTree,_hMaskC);
    FillDataCube(_antiTree,_hAntiC);
    
}

