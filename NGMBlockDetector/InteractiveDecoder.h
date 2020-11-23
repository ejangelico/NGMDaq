#ifndef __INTERACTIVEDECODER_H__
#define __INTERACTIVEDECODER_H__

#include "TObject.h"
#include "Rtypes.h"
#include "ImageMaker.h"
#include "THn.h"

class TTree;
class TH2;

class InteractiveDecoder : public TObject
{
    
public:
	InteractiveDecoder(Int_t maskRank, Double_t maskDetectorDistance, Double_t sourceMaskDistance, Double_t maskElementSize,
               Int_t numberPixelsX, Int_t numberPixelsY, TTree* maskTree, TTree* antiTree, TTree* voidTree);
	InteractiveDecoder(Int_t maskRank, Double_t maskElementSize, Double_t maskDetectorDistance, Double_t sourceMaskDistance,const char* imagerName, TTree* maskTree, TTree* antiTree, TTree* voidTree);
    InteractiveDecoder();
	~InteractiveDecoder(){}; //Needs cleanup
    void go(Option_t* cut);
    void SetSourceDistance(Double_t newSourceDist);
    void ChangeSourceDistance(Double_t newSourceDist);
    void FillDataCubes();
    void FillDataCube(TTree* tree, THnI* hcube);
    
    ImageMaker _imaker;
    TH2* _hMask;
    TH2* _hAnti;
    TH2* _hVoid;
    TH2* _image;
    THnI* _hMaskC;
    THnI* _hAntiC;
    THnI* _hVoidC;
    TTree* _maskTree;
    TTree* _antiTree;
    TTree* _voidTree;
    
	ClassDef(InteractiveDecoder,1)
};

#endif //#ifndef __INTERACTIVEDECODER_H__
