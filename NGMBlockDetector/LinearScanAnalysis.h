#ifndef __LinearScanAnalysis_H__
#define __LinearScanAnalysis_H__

#include "TObject.h"
#include "TString.h"
#include "TObjArray.h"
#include "TArrayD.h"

class DepthScan;
class TH2F;
class MyArray2d;

class LinearScanAnalysis : public TObject
{
public:
	LinearScanAnalysis(Int_t num);
	~LinearScanAnalysis();
	
    void AddDepthScan(DepthScan *dScan, Double_t loc=0.0);
    void RebinToCommon(Int_t binsPerSingleImage);
    
    void WriteToFile(const Char_t *scanDirectory);
    
    TH2F * GetCompositeImageAtDepth(Int_t iDepth);
    TH2F * GetCompositeImageAtHeight(Int_t iHeightLo, Int_t iHeightHi);
    TH2F * GetCompositeImageAtHeightMult(Int_t iHeightLo, Int_t iHeightHi);
    TH2F * GetCompositeImageAtHeight(Double_t heightLo, Double_t heightHi, Bool_t bAdd=true);
    
    MyArray2d * GetArrayOfValidPixels(MyArray2d* theFullArray);

    void DrawImageAtDepth(Int_t iDepth);
    void DrawImageAtHeight(Int_t iHeightLo, Int_t iHeightHi, Bool_t bAdd=true);
//    void DrawImageAtHeight(Double_t heightLo, Double_t heightHi, Bool_t bAdd=true);
    
private:
    
    Int_t numScans;
    TObjArray *theImages;
    TArrayD *thePositions;
    
    Double_t minRangeX, maxRangeX;
    Double_t minRangeY, maxRangeY;
    Int_t numBinsX, numBinsY;
    
    Int_t numDepths;
    Double_t depthMin, depthMax, depthStep;
    
    TObjArray *theData;
    
    TObjArray *keepForFile;
    
	ClassDef(LinearScanAnalysis,1)
};
#endif