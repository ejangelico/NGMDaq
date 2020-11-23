#ifndef __DepthScan_H__
#define __DepthScan_H__

#include "TObject.h"
#include "TString.h"

class TChain;
class InteractiveDecoder;
class TH2;

class DepthScan : public TObject
{
public:
	DepthScan(const Char_t *rDir);
	~DepthScan();
    
    void AddMaskRun(TString theRun);
    void AddAntiRun(TString theRun);
    void AddVoidRun(TString theRun);
    
    void SetImagingParameters(Int_t maskRank, Double_t maskPixelSize,
                              Double_t detectorMaskSeparation, Double_t sourceDistance,
                              Double_t sourceMaskDistance, const Char_t *imagerName);
    void SetDistanceRange(Int_t num, Double_t minD, Double_t maxD);
    void ImageDepths(Option_t* cut, Bool_t bSaveToFile=true);
    void GetDepthImagesFromFile();

    TH2 * ImageAtDepth(Int_t dIndex);
    
    Double_t GetMinimumDepth() {return minDist;}
    Double_t GetMaximumDepth() {return maxDist;}
    Double_t GetDepthStepSize() {return distStep;}
    Int_t GetNumDepths() {return numDist;}
    TString GetRunName() {return runDir;}
    Double_t GetMaxHalfFOV() {return maxHalfFOV;}
    
    void PrintImages();
    
private:
    
    TString runDir;
    Int_t numDist;
    Double_t minDist, maxDist, distStep;
    Bool_t bReadyToImage;
    Double_t maxHalfFOV;
    
    TChain *mChain, *aChain, *vChain;
    InteractiveDecoder *iDecoder;
    
    TObjArray *hImages;
    
	ClassDef(DepthScan,1)
};
#endif