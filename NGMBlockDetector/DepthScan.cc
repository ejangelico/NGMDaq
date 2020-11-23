
#include "DepthScan.h"
using namespace std;

#include "TProof.h"
#include "TH1.h"
#include "TH2.h"
#include "TChain.h"
#include "TFile.h"

#include "InteractiveDecoder.h"

#include <iostream>

ClassImp(DepthScan)

DepthScan::DepthScan(const Char_t *rDir)
{
    
    TProof::Open("");
    
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);
    
    runDir = TString(rDir);
    
    bReadyToImage = false;
    mChain = 0;
    aChain = 0;
    vChain = 0;
    iDecoder = 0;
    maxHalfFOV = 0;
    
    TH1::AddDirectory(bPrevAddDirectory);

}

DepthScan::~DepthScan(){
    
    if(iDecoder) delete iDecoder;
    if(mChain) delete mChain;
    if(aChain) delete aChain;
    if(vChain) delete vChain;
    if(hImages) delete hImages;

}

TH2 * DepthScan::ImageAtDepth(Int_t dIndex){
 
    if(dIndex >= 0 && dIndex <numDist)
        return (TH2*)hImages->At(dIndex);
    else{
        printf("Error in  DepthScan::ImageAtDepth(%d) - array out of bounds\n",dIndex);
        return 0;
    }
}

void DepthScan::SetImagingParameters(Int_t maskRank, Double_t maskPixelSize,
                                              Double_t detectorMaskSeparation, Double_t sourceDistance,
                                              Double_t sourceMaskDistance, const Char_t *imagerName){
    
    printf("\tSetting up InteractiveDecoder\n");
    iDecoder = new InteractiveDecoder(maskRank, maskPixelSize,
                                      detectorMaskSeparation, sourceMaskDistance,
                                      imagerName,mChain,aChain,vChain);
    
    
}

void DepthScan::SetDistanceRange(Int_t num, Double_t minD, Double_t maxD){

    numDist = num;
    minDist = minD;
    maxDist = maxD;
    distStep = 0.0;
    if(num>1) distStep = (maxD-minD)/(num-1);
        
    if(iDecoder && mChain->GetEntries()>0 && aChain->GetEntries()>0 && vChain->GetEntries()>0)
        bReadyToImage = true;
    
}

void DepthScan::ImageDepths(Option_t* cut, Bool_t bSaveToFile){

    if(!bReadyToImage){
        printf("Error in DepthScan::ImageDepths() - not ready to image.\n");
        return;
    }
    
    mChain->SetProof();
    aChain->SetProof();
    vChain->SetProof();
    
    hImages = new TObjArray();
    
    printf("\tScanning depth\n");
    for(Int_t iDepth = 0; iDepth<numDist; iDepth++){
        Double_t newDist = minDist + distStep*iDepth;
        if(iDepth==0){
            iDecoder->ChangeSourceDistance(newDist);
            iDecoder->go(cut);
        }
        else{
            iDecoder->SetSourceDistance(newDist);
            fflush(stdout);
        }
        hImages->AddLast((TH2*)iDecoder->_image->Clone());
        if(iDecoder->_image->GetXaxis()->GetXmax() > maxHalfFOV)
            maxHalfFOV = iDecoder->_image->GetXaxis()->GetXmax();
        printf("\t\tDepth %d of %d (%f cm)\r",iDepth+1,numDist,newDist);
    }
    printf("\n");
    
    if(bSaveToFile) PrintImages();
    
}

void DepthScan::PrintImages(){
    

    TFile *fim = TFile::Open(Form("%s%s/imagesAtDepths.root",gSystem->Getenv("NGMRAWDATADIR"),runDir.Data()),"recreate");
    
    for(Int_t i=0; i<hImages->GetEntries(); i++){
        TH2 *h = (TH2*)ImageAtDepth(i);
        h->Write(Form("depth%d",i));
    }
    
    delete fim;
    

}

void DepthScan::GetDepthImagesFromFile(){
   
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

    TFile *fim = TFile::Open(Form("%s%s/imagesAtDepths.root",gSystem->Getenv("NGMRAWDATADIR"),runDir.Data()),"read");

    hImages = new TObjArray();
    for(Int_t iDepth = 0; iDepth<numDist; iDepth++){
        TH2 *h = (TH2*)fim->Get(Form("depth%d",iDepth));
        TH2 *hC = (TH2*)h->Clone(Form("h%d",iDepth));
        if(hC->GetXaxis()->GetXmax() > maxHalfFOV)
            maxHalfFOV = hC->GetXaxis()->GetXmax();
        hImages->AddLast(hC);
    }
        
    delete fim;
    
    TH1::AddDirectory(bPrevAddDirectory);

}


void DepthScan::AddMaskRun(TString theRun){
 
    if(!mChain) mChain = new TChain("HitTree");

    printf("\t\tMask file: %s\n",theRun.Data());
    mChain->AddFile(theRun.Data());

    
}

void DepthScan::AddAntiRun(TString theRun){
    
    if(!aChain) aChain = new TChain("HitTree");
    
    printf("\t\tAnti file: %s\n",theRun.Data());
    aChain->AddFile(theRun.Data());
    
    
}

void DepthScan::AddVoidRun(TString theRun){
    
    if(!vChain) vChain = new TChain("HitTree");
    
    printf("\t\tVoid file: %s\n",theRun.Data());
    vChain->AddFile(theRun.Data());
    
    
}

