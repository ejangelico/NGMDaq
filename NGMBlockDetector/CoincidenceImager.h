#ifndef __CoincidenceImager_H__
#define __CoincidenceImager_H__

#include "CoincidenceImager.h"
#include "CoincidenceMaker.h"
#include "ImageMaker.h"
#include "TParameter.h"
#include <vector>
#include "TObject.h"
#include "TChain.h"

class CoincidenceImageCut : public TObject
{
public:
    CoincidenceImageCut();
    ~CoincidenceImageCut();
    
    void SetDetectorPixels(Int_t detPix, Int_t iID);

    Double_t dtRange[2];
    Double_t liqRange[2];
    Double_t imagerHitEnergyRange[2];
    Int_t iImagerPartID;
    Int_t iLiqPartID;
    Int_t iMult;
    enum {Neutron,Thermal,Gamma, Other} CIParticleType;
    
    TH2D *hHitPattern[2]; //mask and antimask
    TH2D *hImage;
    
    TString ImageDescriptor();
    TString ImageTitle();
    TString _imageDescriptor;

    ClassDef(CoincidenceImageCut,3)
};

class CoincidenceImager : public TObject
{
public:
    CoincidenceImager(Int_t maskRank, Double_t maskElementSize, Double_t maskDetectorDistance, Double_t sourceMaskDistance, TString imagerName);
    ~CoincidenceImager();
    
    void AddFileToMaskChain(TString fileName);
    void AddFileToAntiChain(TString fileName);
    
    void SetOutputPath(TString theDir) {outPath = theDir;}
    
    void PerformCoincidenceCuts();
    
    void AddImage(const char* imageDesc, Double_t dtLow, Double_t dtHi, Double_t liqLo, Double_t liqHi,
                  Int_t iImgPID, Int_t iLiqPID, Int_t iMult, Int_t numDetPix,
                  Double_t IHitEmin = -1.0, Double_t IHitEmax = -1.0);
    
    void SaveHistograms();
    
private:
	
    TChain *maskChain;
    TChain *antiChain;
    TChain *chainInUse;
    
    TClonesArray *imageCuts;

    ImageMaker *im;
    CoincidentEvent *event;
    
    TFile *fout;
    
    TString outPath;
    Int_t numCuts;
    
    void DoCuts(Int_t iOrientation);
    void ReconstructImages();

    ClassDef(CoincidenceImager,1)
};

#endif
