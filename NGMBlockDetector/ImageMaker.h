#ifndef __IMAGEMAKER_H__
#define __IMAGEMAKER_H__
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TF2.h"
#include "TH3.h"
#include "TObject.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TMarker.h"
#include "TMath.h"
#include "TBox.h"
#include "TEllipse.h"
#include "TLatex.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TArrayD.h"
#include "TGraph.h"
#include "TObjArray.h"
#include <vector>

class ImagerRunDescription : public TObject
{
public:
    ImagerRunDescription();
    virtual ~ImagerRunDescription();
    TString imagername;
    double detectormaskseparation;
    TString imagetimebegin;
    TString sourcename;
    double maskthickness;
    double maskpixelsize;
    double sourcedistance;
    int maskrank;
    TString imagerrunname;
    TString description;
    
    int maskruncount;
    std::vector<long long> maskrunlist;
    int antiruncount;
    std::vector<long long> antirunlist;
    
    void AppendMaskRun(long long runval) { maskrunlist.push_back(runval); }
    void AppendAntiRun(long long runval) { antirunlist.push_back(runval); }
    
    ClassDef(ImagerRunDescription,1);
};

class ImageMaker : public TObject
{
public:
	ImageMaker(Int_t maskRank, Double_t maskDetectorDistance,
			   Double_t sourceMaskDistance, Double_t maskElementSize,
               Int_t numberPixelsX, Int_t numberPixelsY,
			   Double_t detectorPixelSize=4.25*2.54/10.0); //default to L40 value
	ImageMaker(Int_t maskRank, Double_t maskElementSize,
			   Double_t maskDetectorDistance, Double_t sourceMaskDistance,
			   const Char_t *imagerName);
	~ImageMaker();
	
	void Initialize(Int_t maskRank);
    void SetDistances(Double_t newMaskDetDist, Double_t newMaskSrcDist);
	Double_t GetSourceMaskDistance() const {return sourceMaskDist;}
	Double_t GetDetectorMaskDistance() const {return maskDetDist;}
	TH2D *GetImage(TH2D *hMaskDetectorHitPattern, TH2D *hAntiDetectorHitPattern, Bool_t bOffset=false);
	TH2D *GetImage2(TH2D *hMaskDetectorHitPattern, TH2D *hAntiDetectorHitPattern, Bool_t bOffset=false);
	TH2D *GetImage3(TH2D *hMaskDetectorHitPattern, TH2D *hAntiDetectorHitPattern, Bool_t bOffset=false, Bool_t bAdd=false, Bool_t bDoubleSample=true);
	TH2D *GetImage3(TH2I *hMaskDetectorHitPattern, TH2I *hAntiDetectorHitPattern, Bool_t bOffset=false, Bool_t bAdd=false, Bool_t bDoubleSample=true);
	TH2D *GetImage3(TH2 *hMaskDetectorHitPattern, TH2 *hAntiDetectorHitPattern, Bool_t bOffset=false, Bool_t bAdd=false, Bool_t bDoubleSample=true);
    TH3D *GetImage3D(TH3* hMaskDetectorHitPattern, TH3 *hAntiDetectorHitPattern, TH3* hVoid,Bool_t bOffset=false, Bool_t bAdd=false);
    
	TH2D *GetImageHistogram();
	TH2D *GetCurrentMaskPattern();
	void GetDecoder();
	TH2D *GetDecoderHistogram() {return hDecoder;}
	TH2D *GetStandardDecoderHistogram() {return hDecoderStandard;}
	TH2D *GetData() {return hDHP;}
    TH3D *GetData3D() { return hData3D; }
	TH2D *GetRebinnedData() {return hRebinnedDet;}
    void SetData(TH2D *hMaskDetectorHitPattern, TH2D *hAntiDetectorHitPattern);
	void SetDecoder(TH2D *hDec) {hDecoder=hDec;}
	TH2D *GetExpectedHitPatternMask(Double_t imX=0, Double_t imY=0);
	TH2D *GetExpectedHitPatternAnti(Double_t imX=0, Double_t imY=0);
	TH2D *GetExpectedHitPatternDiff(Double_t imX=0, Double_t imY=0);
    Double_t GetTotalDetectorCounts(Bool_t bAdd=false) {
        if(!bAdd) return totalDetectorCounts;
        else return totalDetectorCountsAdd;
    }
    Int_t GetNumPixX() const { return numPixX;}
    Int_t GetNumPixY() const { return numPixY;}
  const TArrayD& GetOffsetPar() const {return offsetPar; }
  const TArrayD& GetOffsetParError() const {return offsetParE; }
    Double_t CalcImageCounts(Int_t& pixelsUsed, Double_t zeroSupThresh=-1e9, Int_t imageIdx=3) const;
    TH1F* GetOffsetHist() {return hOffsetHist;}
    Double_t GetCorrection() const;
    static TH2D* makeFloat(TH2* h);
    static TH3D* make3DFromSerial(TH2* h,int ncols=40);
    
private:
	void MakeCorrectionCurve();
	TH2D *hDHPm, *hDHPa, *hDHP;
    TH2D *hRebinnedDet, *hRebinnedDetAdd, *hDetAbsAxis;
	TH2D *hMaskPattern, *hDecoder, *hImage, *hImage2, *hImage3;
    TH2D *hDecoderStandard;
	TH2D *hDecM, *hDecA, *hDecD;
	TH2D *hMaskScaled, *hAntiScaled, *hDiffScaled;
    TH2D *hExpMask, *hExpAnti, *hExpDiff;
    TH3D *hImage3D, *hData3D;
	TH1F *hOffsetHist;
    TGraph* avgPixelWeights;
    
	TH2D *GetMaskPattern(Int_t base);
	void Rehistogram(TH2D *hOrig, TH2D *hNew, TH2D *hNewAdd);
    TH2D *GetExpectedHitPattern(Double_t imX=0.0, Double_t imY=0.0, Int_t iType=0);

	Int_t iMask, mRank;
	Int_t numPixX, numPixY;
	Double_t focalLength, sourceMaskDist, maskElSize, maskDetDist;
	Double_t detPixelSize, maskRangeX, maskRangeY;
    Double_t detRangeX, detRangeY;
    Int_t minZero, maxZero;
    Double_t totalDetectorCounts, totalDetectorCountsAdd;
  TArrayD offsetPar;
  TArrayD offsetParE;
//    Double_t fgaus(Double_t *x, Double_t *par);
		
	ClassDef(ImageMaker,8)
};
#endif