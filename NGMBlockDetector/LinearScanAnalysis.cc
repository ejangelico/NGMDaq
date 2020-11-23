
#include "LinearScanAnalysis.h"
using namespace std;

#include "TProof.h"
#include "TH1.h"
#include "TH2.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TMath.h"

#include "DepthScan.h"
#include "MyArray2d.h"

#include <iostream>

ClassImp(LinearScanAnalysis)

LinearScanAnalysis::LinearScanAnalysis(Int_t num)
{
    theImages = new TObjArray(num);
    thePositions = new TArrayD(num);
    
    keepForFile = new TObjArray();
}

LinearScanAnalysis::~LinearScanAnalysis(){
    
}

void LinearScanAnalysis::AddDepthScan(DepthScan *dScan, Double_t loc){
    
    if(theImages && thePositions){
        theImages->AddLast(dScan);
        thePositions->AddAt(loc,theImages->GetLast());
    }
}

void LinearScanAnalysis::RebinToCommon(Int_t binsPerSingleImage){
     
    Int_t numPos = theImages->GetEntries();
    for(Int_t iPos = 0; iPos<numPos; iPos++){
        DepthScan *ds = (DepthScan*)theImages->At(iPos);
        Double_t maxHalfFOV = ds->GetMaxHalfFOV();
        Double_t xLoc = thePositions->At(iPos);
        
        if(xLoc - maxHalfFOV < minRangeX) minRangeX = xLoc - maxHalfFOV;
        if(xLoc + maxHalfFOV > maxRangeX) maxRangeX = xLoc + maxHalfFOV;

        if(-maxHalfFOV < minRangeY) minRangeY = -maxHalfFOV;
        if( maxHalfFOV > maxRangeY) maxRangeY = maxHalfFOV;

        if(iPos==0){
            numDepths = ds->GetNumDepths();
            depthMin = ds->GetMinimumDepth()-ds->GetDepthStepSize()/2.0;
            depthMax = ds->GetMaximumDepth()+ds->GetDepthStepSize()/2.0;
            depthStep = ds->GetDepthStepSize();
        }
    }
    
    numBinsY = binsPerSingleImage;
    numBinsX = TMath::Nint( binsPerSingleImage * (maxRangeX-minRangeX)/(maxRangeY-minRangeY));
    
    theData = new TObjArray(numPos);
    
    for(Int_t iPos = 0; iPos<numPos; iPos++){
        
        DepthScan *ds = (DepthScan*)theImages->At(iPos);
        Double_t xLoc = thePositions->At(iPos);

        TObjArray *scaledImagesByDepth = new TObjArray(ds->GetNumDepths());
        
        for(Int_t iDepth=0; iDepth<ds->GetNumDepths(); iDepth++){
        
            TH2 *h = (TH2*)ds->ImageAtDepth(iDepth);
            MyArray2d *aImage = MyArray2d::GetArray(h);
            aImage->SetRangeX(aImage->GetLowX()+xLoc, aImage->GetHighX()+xLoc);
            
            MyArray2d *aFullScene = new MyArray2d(numBinsX,numBinsY,
                                                  minRangeX,maxRangeX,
                                                  minRangeY,maxRangeY);

            aFullScene->RehistogramArrayByArea(aImage);

            scaledImagesByDepth->AddLast(aFullScene);
                        
        }
        
        theData->AddLast(scaledImagesByDepth);
    }
    
}

TH2F * LinearScanAnalysis::GetCompositeImageAtDepth(Int_t iDepth){
    MyArray2d * aSumAtDepth = 0;
    
    for(Int_t iPos = 0; iPos<theImages->GetEntriesFast(); iPos++){
        TObjArray *scaledImages = (TObjArray*)theData->At(iPos);
        
        MyArray2d *aImageAtThisDepth = (MyArray2d*)scaledImages->At(iDepth);
        
        if(iPos==0)     cout<<"Image at depth "<<iDepth<<": "<<depthMin + (iDepth+0.5)*depthStep<<" "<<aImageAtThisDepth->Integral()<<endl;

        
        if(iPos==0) aSumAtDepth = aImageAtThisDepth->Clone();
        else aSumAtDepth->AddArrays(aSumAtDepth, aImageAtThisDepth);
    }
    
    TH2F *hSumAtDepth = (TH2F*)aSumAtDepth->GetArrayHistogram(Form("ImageAtDepth%d",iDepth));
    keepForFile->AddLast(hSumAtDepth);
    
    if(aSumAtDepth) return hSumAtDepth;
    else return 0;

}

void LinearScanAnalysis::DrawImageAtDepth(Int_t iDepth){
 

    TCanvas* myCanvas = (TCanvas*)(gROOT->FindObjectAny("cImageAtDepthCanvas"));
    if(!myCanvas){
        myCanvas = new TCanvas("cImageAtDepthCanvas","cImageAtDepthCanvas");
    }
    myCanvas->Clear();

    GetCompositeImageAtDepth(iDepth)->DrawCopy("colz");

    myCanvas->Update();
//    cin.ignore();
    
}

void LinearScanAnalysis::DrawImageAtHeight(Int_t iHeightLo, Int_t iHeightHi, Bool_t bAdd){
    
    cout<<"Image at heights between "<<iHeightLo<<" and "<<iHeightHi<<endl;

    TCanvas* myCanvas = (TCanvas*)(gROOT->FindObjectAny("cImageAtDepthCanvas"));
    if(!myCanvas){
        myCanvas = new TCanvas("cImageAtDepthCanvas","cImageAtDepthCanvas");
    }
    myCanvas->Clear();
    
    if(bAdd) GetCompositeImageAtHeight(iHeightLo,iHeightHi)->DrawCopy("colz");
    else GetCompositeImageAtHeightMult(iHeightLo,iHeightHi)->DrawCopy("colz");

    myCanvas->Update();
    cin.ignore();
    
}

//void LinearScanAnalysis::DrawImageAtHeight(Double_t heightLo, Double_t heightHi, Bool_t bAdd){
//    
//    cout<<"Image at heights between "<<heightLo<<" and "<<heightHi<<endl;
//    
//    TCanvas* myCanvas = (TCanvas*)(gROOT->FindObjectAny("cImageAtDepthCanvas"));
//    if(!myCanvas){
//        myCanvas = new TCanvas("cImageAtDepthCanvas","cImageAtDepthCanvas");
//    }
//    myCanvas->Clear();
//    
//    if(bAdd) GetCompositeImageAtHeight(heightLo,heightHi,bAdd)->DrawCopy("colz");
//    else GetCompositeImageAtHeight(heightLo,heightHi,bAdd)->DrawCopy("colz");
//    
//    myCanvas->Update();
//    cin.ignore();
//    
//}

TH2F * LinearScanAnalysis::GetCompositeImageAtHeight(Double_t heightLo, Double_t heightHi, Bool_t bAdd){
 
    Int_t iHlo, iHhi;
    MyArray2d tempArray = MyArray2d(numBinsY,1,minRangeY,maxRangeY,0,1);
    tempArray.GetArrayIndexX(heightLo,iHlo);
    tempArray.GetArrayIndexX(heightHi,iHhi);
    
    if(bAdd) return GetCompositeImageAtHeight(iHlo, iHhi);
    else return GetCompositeImageAtHeightMult(iHlo, iHhi);
    
}

TH2F * LinearScanAnalysis::GetCompositeImageAtHeight(Int_t iHeightLo, Int_t iHeightHi){
    
    MyArray2d * aSumAtHeight = new MyArray2d(numBinsX,numDepths,minRangeX,maxRangeX,depthMin,depthMax);
    MyArray2d * aSumForAvg = new MyArray2d(numBinsX,numDepths,minRangeX,maxRangeX,depthMin,depthMax);
    
    for(Int_t iPos = 0; iPos<theImages->GetEntriesFast(); iPos++){
        TObjArray *scaledImages = (TObjArray*)theData->At(iPos);
        
        for(Int_t iDepth=0; iDepth<scaledImages->GetEntriesFast(); iDepth++){
            MyArray2d *aImageAtThisDepth = (MyArray2d*)scaledImages->At(iDepth);
            TH2 *hImageOrig  = ((DepthScan*)theImages->At(iPos))->ImageAtDepth(iDepth);
            
            Int_t iH0, iH1;
            Double_t dTest0 = hImageOrig->GetYaxis()->GetBinLowEdge(iHeightLo);
            Double_t dTest1 = hImageOrig->GetYaxis()->GetBinLowEdge(iHeightHi+1);
            aImageAtThisDepth->GetArrayIndexY(dTest0, iH0);
            aImageAtThisDepth->GetArrayIndexY(dTest1, iH1);

            Double_t depthScale = pow( (depthMin + (iDepth+0.5)*depthStep)/(depthMin + 0.5*depthStep) , 2.0);
            
            if(iPos==0) cout<<"Avg-Depth "<<iDepth<<": Image at heights between "<<aImageAtThisDepth->GetBinPositionY(iH0)<<" "<<aImageAtThisDepth->GetBinPositionY(iH1)<<endl;

            MyArray2d *aTemp = GetArrayOfValidPixels(aImageAtThisDepth);
            
            for(Int_t iH=iH0; iH<=iH1; iH++){
                for(Int_t ix=0; ix<aImageAtThisDepth->GetLengthX(); ix++){
                    aSumAtHeight->AddVal(ix, iDepth, depthScale*aImageAtThisDepth->val(ix,iH));
                    aSumForAvg->AddVal(ix, iDepth, aTemp->val(ix,iH));
                }
            }
            
            delete aTemp;
        }
        
    }
    
    aSumAtHeight->DivideArrays(aSumAtHeight, aSumForAvg);
    
    TH2F *hSumAtHeight = (TH2F*)aSumAtHeight->GetArrayHistogram(Form("ImageAtHeight%d_%d",iHeightLo,iHeightHi));
    keepForFile->AddLast(hSumAtHeight);

    if(aSumAtHeight) return hSumAtHeight;
    else return 0;


}

TH2F * LinearScanAnalysis::GetCompositeImageAtHeightMult(Int_t iHeightLo, Int_t iHeightHi){
    
    TObjArray *projAtHeight = new TObjArray(theImages->GetEntriesFast());
    for(Int_t iPos = 0; iPos<theImages->GetEntriesFast(); iPos++){
        MyArray2d *theArrayAtThisPosition = new MyArray2d(numBinsX,numDepths,minRangeX,maxRangeX,depthMin,depthMax);
        
        TObjArray *scaledImages = (TObjArray*)theData->At(iPos);
        
        for(Int_t iDepth=0; iDepth<scaledImages->GetEntriesFast(); iDepth++){
            MyArray2d *aImageAtThisDepth = (MyArray2d*)scaledImages->At(iDepth);
            TH2 *hImageOrig  = ((DepthScan*)theImages->At(iPos))->ImageAtDepth(iDepth);

            Int_t iH0, iH1;
            Double_t dTest0 = hImageOrig->GetYaxis()->GetBinLowEdge(iHeightLo);
            Double_t dTest1 = hImageOrig->GetYaxis()->GetBinLowEdge(iHeightHi+1);
            aImageAtThisDepth->GetArrayIndexY(dTest0, iH0);
            aImageAtThisDepth->GetArrayIndexY(dTest1, iH1);

            Double_t depthScale = pow( (depthMin + (iDepth+0.5)*depthStep)/(depthMin + 0.5*depthStep) , 2.0);
            
            if(iPos==0) cout<<"Mult-Depth "<<iDepth<<": Image at heights between "<<aImageAtThisDepth->GetBinPositionY(iH0)<<" "<<aImageAtThisDepth->GetBinPositionY(iH1)<<endl;

            for(Int_t iH=iH0; iH<=iH1; iH++){
                for(Int_t ix=0; ix<aImageAtThisDepth->GetLengthX(); ix++){
                    theArrayAtThisPosition->AddVal(ix, iDepth, depthScale*aImageAtThisDepth->val(ix,iH));
                    //theArrayAtThisPosition->AddVal(ix, iDepth, aImageAtThisDepth->val(ix,iH));
                }
            }
        }
        projAtHeight->AddLast(theArrayAtThisPosition);
        keepForFile->AddLast( theArrayAtThisPosition->GetArrayHistogram(Form("pos%d_%d_%d",iPos,iHeightLo,iHeightHi)) );
    }

    
    MyArray2d *aProduct = new MyArray2d(numBinsX,numDepths,minRangeX,maxRangeX,depthMin,depthMax);
    MyArray2d *aNumUsed = new MyArray2d(numBinsX,numDepths,minRangeX,maxRangeX,depthMin,depthMax);
    aProduct->SetUniform();
    for(Int_t iPos = 0; iPos<projAtHeight->GetEntriesFast(); iPos++){
        MyArray2d *aProj = (MyArray2d*)projAtHeight->At(iPos);
        for(Int_t ix=0; ix<aProj->GetLengthX(); ix++){
            for(Int_t iy=0; iy<aProj->GetLengthY(); iy++){
                
                Double_t valToUse = aProj->val(ix, iy);
                if(TMath::Abs(valToUse) < 1e-10) valToUse = 1.0;
                else aNumUsed->AddVal(ix, iy, 1.0);
                
                aProduct->SetVal(ix, iy, valToUse*aProduct->val(ix,iy));
                
            }
        }
    }
    
    MyArray2d *aProductSign = aProduct->Clone();
    
    for(Int_t ix=0; ix<aProduct->GetLengthX(); ix++){
        for(Int_t iy=0; iy<aProduct->GetLengthY(); iy++){
            if(aNumUsed->val(ix,iy) > 1.0){
                Double_t dSign = 1.0;
                if(aProduct->val(ix,iy) < 0.0) dSign = -1.0;
                aProduct->SetVal( ix, iy, TMath::Power(TMath::Abs(aProduct->val(ix,iy)),1.0/aNumUsed->val(ix,iy)) );
                aProductSign->SetVal( ix, iy, dSign*TMath::Power(TMath::Abs(aProduct->val(ix,iy)),1.0/aNumUsed->val(ix,iy)) );
            }
            else{
                aProduct->SetVal(ix, iy, 0.0);
                aProductSign->SetVal(ix, iy, 0.0);
            }
        }
    }
    
    TH2F *hProduct = (TH2F*)aProduct->GetArrayHistogram(Form("Product%d_%d",iHeightLo,iHeightHi));
    keepForFile->AddLast(hProduct);
    TH2F *hProductSign = (TH2F*)aProductSign->GetArrayHistogram(Form("ProductSign%d_%d",iHeightLo,iHeightHi));
    keepForFile->AddLast(hProductSign);
    TH2F *hNumUsed = (TH2F*)aNumUsed->GetArrayHistogram(Form("NumUsed%d_%d",iHeightLo,iHeightHi));
    keepForFile->AddLast(hNumUsed);
    
    if(aProduct) return hProduct;
    else return 0;

}

MyArray2d * LinearScanAnalysis::GetArrayOfValidPixels(MyArray2d* theFullArray){
    
    MyArray2d *validArray = theFullArray->Clone();
    for(Int_t ix=0; ix<theFullArray->GetLengthX(); ix++){
        for(Int_t iy=0; iy<theFullArray->GetLengthY(); iy++){
            if(TMath::Abs(theFullArray->val(ix,iy))>1e-10) validArray->SetVal(ix, iy, 1.0);
            else validArray->SetVal(ix, iy, 0.0);
        }
    }
    
    return validArray;
}

void LinearScanAnalysis::WriteToFile(const Char_t *scanDirectory){
 
    TFile *fout = TFile::Open(Form("%s/scanResults.root",scanDirectory),"recreate");
    fout->cd();
    
    TObjArrayIter next(keepForFile);
    TObject* object;
    while ( ( object = next() ) ) {
        TH2F* h = (TH2F*) object;
        h->Write(h->GetName());
    }

//    TObjArrayIter next(theImages);
//    TObject* object;
//    Int_t iD = 0;
//    while ( ( object = next() ) ) {
//        // Do something with object
//        DepthScan* ds = (DepthScan*) object;
//        ds->Write(Form("depthScan%d",iD));
//        iD++;
//    }

    printf("\nOutput file: %s\n\n",fout->GetName());
    delete fout;
    
}
