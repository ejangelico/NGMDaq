
#include "CoincidenceImager.h"

#include "TFile.h"
#include "TObjString.h"

using namespace std;
#include <iostream>

ClassImp(CoincidenceImager)
ClassImp(CoincidenceImageCut)

CoincidenceImager::CoincidenceImager(Int_t maskRank, Double_t maskElementSize, Double_t maskDetectorDistance, Double_t sourceMaskDistance, TString imagerName)
{
    
    maskChain = 0;
    antiChain = 0;
    chainInUse = 0;
    
    event = new CoincidentEvent();
    
    im = new ImageMaker(maskRank,maskElementSize,maskDetectorDistance,sourceMaskDistance,imagerName.Data());
    
    imageCuts = new TClonesArray("CoincidenceImageCut",1);
    imageCuts->SetOwner(true);
    numCuts = 0;
    fout = 0;
}

CoincidenceImager::~CoincidenceImager(){
    
    if(im) delete im;
    if(imageCuts) imageCuts->Clear("C");
    if(fout) delete fout;
    
}

void CoincidenceImager::AddFileToMaskChain(TString fileName){
    
    if(!maskChain) maskChain = new TChain("coincTree");
    maskChain->Add(fileName.Data());
    printf("%s added to mask chain\n",fileName.Data());
    
}

void CoincidenceImager::AddFileToAntiChain(TString fileName){
    
    if(!antiChain) antiChain = new TChain("coincTree");
    antiChain->Add(fileName.Data());
    printf("%s added to anti chain\n",fileName.Data());
    
}

void CoincidenceImager::PerformCoincidenceCuts(){
    
    if(maskChain && maskChain->GetEntries() > 0){
        maskChain->SetBranchAddress("coincData", &event);
        chainInUse = maskChain;
        
        printf("\tProcessing mask chain\n");
        
        DoCuts(0);
    }
    if(antiChain && antiChain->GetEntries() > 0){
        antiChain->SetBranchAddress("coincData", &event);
        chainInUse = antiChain;
        
        printf("\tProcessing mask chain\n");
        
        DoCuts(1);
    }

    fout = TFile::Open(Form("%s/CoincidenceImages.root",outPath.Data()),"recreate");

    ReconstructImages();
    SaveHistograms();
    
}

void CoincidenceImager::SaveHistograms(){
    
    
    for(Int_t iCut=0; iCut<numCuts; iCut++){
        CoincidenceImageCut *theCut = (CoincidenceImageCut*)imageCuts->At(iCut);
        
        TString dirName(theCut->ImageDescriptor());
        
        fout->cd(dirName);
        
        if(theCut->hHitPattern[0] && theCut->hHitPattern[0]->Integral() > 0)
            theCut->hHitPattern[0]->Write("maskHits");
        if(theCut->hHitPattern[1] && theCut->hHitPattern[1]->Integral() > 0)
            theCut->hHitPattern[1]->Write("antiHits");
        TObjString* objDesc = new TObjString(theCut->ImageTitle());
        objDesc->Write("ImageDescription");
        delete objDesc;
        
    }
    
    cout<<"\nOutput file: "<<fout->GetName()<<endl<<endl;
    
}


void CoincidenceImager::ReconstructImages(){

    if(!im){
        printf("Error in CoincidenceImager::ReconstructImages() - ImageMaker not configured.\n");
        return;
    }
    
    for(Int_t iCut=0; iCut<numCuts; iCut++){
        CoincidenceImageCut *theCut = (CoincidenceImageCut*)imageCuts->At(iCut);

        for(Int_t i=0; i<2; i++)
            if(!(theCut->hHitPattern[i]->Integral()>0.0)) theCut->hHitPattern[i] = 0;
        
        Bool_t bAddEdges = false;
        Bool_t bFixOffset = false; //best to keep this false and correct offset another way after decoding
        Bool_t bDoubleSample = true;
        
        TH2* hSum =dynamic_cast<TH2*>(theCut->hHitPattern[0]->Clone(Form("%s_Sum",theCut->hHitPattern[1]->GetName())));
        hSum->Add(theCut->hHitPattern[1]);
        double h0Before =theCut->hHitPattern[0]->Integral();
        theCut->hHitPattern[0]->Divide(hSum);
        double h0After =theCut->hHitPattern[0]->Integral();
        theCut->hHitPattern[0]->Scale(h0Before/h0After);
        double h1Before =theCut->hHitPattern[1]->Integral();
        theCut->hHitPattern[1]->Divide(hSum);
        double h1After =theCut->hHitPattern[1]->Integral();
        theCut->hHitPattern[1]->Scale(h1Before/h1After);
        
        
        theCut->hImage = im->GetImage3(theCut->hHitPattern[0],theCut->hHitPattern[1],bAddEdges,bFixOffset,bDoubleSample);
        
        TString dirName(theCut->ImageDescriptor());
        
        if(fout){
            fout->mkdir(dirName);
            fout->cd(dirName);
            
            if(theCut->hImage && TMath::Abs(theCut->hImage->Integral()) > 0.0)
                theCut->hImage->Write("image");
        }
        
    }
    
}


void CoincidenceImager::AddImage(const char* imageDesc, Double_t dtLow, Double_t dtHi, Double_t liqLo, Double_t liqHi,
                                 Int_t iImgPID, Int_t iLiqPID, Int_t iMult, Int_t numDetPix,
                                 Double_t IHitEmin, Double_t IHitEmax){
    
    CoincidenceImageCut *theCut = (CoincidenceImageCut*)imageCuts->ConstructedAt(numCuts++);
    theCut->_imageDescriptor = imageDesc;
    theCut->SetDetectorPixels(numDetPix,numCuts-1);
    theCut->dtRange[0] = dtLow;
    theCut->dtRange[1] = dtHi;
    theCut->liqRange[0] = liqLo;
    theCut->liqRange[1] = liqHi;
    theCut->iImagerPartID = iImgPID;
    theCut->iLiqPartID = iLiqPID;
    theCut->iMult = iMult;
    theCut->imagerHitEnergyRange[0] = IHitEmin;
    theCut->imagerHitEnergyRange[1] = IHitEmax;
    
}

void CoincidenceImager::DoCuts(Int_t iOrientation){
    
    Bool_t bFirstPrint = false;
    
    for(Int_t iEntry=0; iEntry<chainInUse->GetEntries(); iEntry++){
        chainInUse->GetEntry(iEntry);
        
        if(iEntry%100000==0){
            if(bFirstPrint) fflush(stdout);
            printf("\tTree entry %d of %lld    (%.3f)\r",iEntry,chainInUse->GetEntries(),double(iEntry+1)/double(chainInUse->GetEntries()));
            bFirstPrint = true;
        }
        
        for(Int_t iCut=0; iCut<numCuts; iCut++){
            CoincidenceImageCut *theCut = (CoincidenceImageCut*)imageCuts->At(iCut);
            
            if(event->ImagerHit->particleType == theCut->iImagerPartID
               && (theCut->imagerHitEnergyRange[0]<0.0
                   || (theCut->imagerHitEnergyRange[0] < event->ImagerHit->energy
                       && event->ImagerHit->energy < theCut->imagerHitEnergyRange[1]))){
                if(event->ImagerHit->relativeTime > theCut->dtRange[0] && event->ImagerHit->relativeTime < theCut->dtRange[1]){
                    
                    Int_t liqMult = 0;
                    if(theCut->iMult>0)
                    for(Int_t iLiq=0; iLiq<event->liqCoinc->GetEntries(); iLiq++){
                        CoincidenceVariables *theLiq = (CoincidenceVariables*)event->liqCoinc->At(iLiq);

                        if(theLiq->particleType == theCut->iLiqPartID){
                            if(theLiq->relativeTime > theCut->liqRange[0] && theLiq->relativeTime < theCut->liqRange[1])
                                liqMult++;
                        }
                    }
                    
                    if(liqMult >= theCut->iMult) theCut->hHitPattern[iOrientation]->Fill(event->ImagerHit->pixelX,event->ImagerHit->pixelY);
                    
                }
            }
            
        }
        
    }
    printf("\n");
    
}

CoincidenceImageCut::CoincidenceImageCut(){
    
    hHitPattern[0] = 0;
    hHitPattern[1] = 0;
    hImage = 0;
    
}

CoincidenceImageCut::~CoincidenceImageCut(){
    
    if(hHitPattern[0]) delete hHitPattern[0];
    if(hHitPattern[1]) delete hHitPattern[1];
    if(hImage) delete hImage;
    
}

TString CoincidenceImageCut::ImageDescriptor(){
    return _imageDescriptor;
}

TString CoincidenceImageCut::ImageTitle(){
    
    //time description
    TString sTimeDT(Form("DT_%.1fus_%.1fus",dtRange[0]/1000.0,dtRange[1]/1000.0));
    TString sTimeLiq(Form("Liq_%.1fns_%.1fns",liqRange[0],liqRange[1]));
   
    sTimeDT.ReplaceAll("-","m");
    sTimeDT.ReplaceAll(".","p");
    sTimeLiq.ReplaceAll("-","m");
    sTimeLiq.ReplaceAll(".","p");

    //Imager particle description
    TString sImgPID("ImgNeutron");
    if(iImagerPartID==1) sImgPID = "ImgThermal";
    if(iImagerPartID==2) sImgPID = "ImgGamma";
    if(iImagerPartID==3) sImgPID = "ImgOther";
    
    //liquid particle description
    TString sLiqPID("LiqNeutron");
    if(iLiqPartID==1) sLiqPID = "LiqThermal";
    if(iLiqPartID==2) sLiqPID = "LiqGamma";
    if(iLiqPartID==3) sLiqPID = "LiqOther";
    
    //multiplicity description
    TString sMult(Form("Mult_%d",iMult));
    
    TString desc(Form("%s_%s_%s_%s_%s_Elow%.0f_Ehi%.0f",sTimeDT.Data(),sTimeLiq.Data(),sImgPID.Data(),sLiqPID.Data(),sMult.Data(),
                      imagerHitEnergyRange[0],imagerHitEnergyRange[1]));
    desc.ReplaceAll("-","m");
    desc.ReplaceAll(".","p");

    return desc;
    
}



void CoincidenceImageCut::SetDetectorPixels(Int_t detPix, Int_t iID){
    
    hHitPattern[0] = new TH2D(Form("hHitPattern0_%d",iID),"Mask hit pattern",detPix,0,detPix,detPix,0,detPix);
    hHitPattern[1] = new TH2D(Form("hHitPattern1_%d",iID),"Anti hit pattern",detPix,0,detPix,detPix,0,detPix);
    
}

