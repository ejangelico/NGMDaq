#ifndef __PDSTImager_H__
#define __PDSTImager_H__

#include "TObject.h"
#include "Rtypes.h"
#include "ImageMaker.h"
#include "THn.h"
#include "TSelector.h"
#include "TList.h"
#include <vector>

class TTree;
class TH2;
class NGMSystemConfiguration;
class NGMBlockMapping;
class NGMBlockPSD;
class NGMHit;
class TBranch;

class PDSTImager : public TObject
{
    
public:
	PDSTImager(Int_t maskRank, Double_t maskDetectorDistance, Double_t sourceMaskDistance, Double_t maskElementSize,
               Int_t numberPixelsX, Int_t numberPixelsY, TTree* maskTree, TTree* antiTree, TTree* voidTree);
	PDSTImager(Int_t maskRank, Double_t maskElementSize, Double_t maskDetectorDistance, Double_t sourceMaskDistance,const char* imagerName, TTree* maskTree, TTree* antiTree, TTree* voidTree);
    PDSTImager();
	~PDSTImager(){}; //Needs cleanup
    void go();
    void SetSourceDistance(Double_t newSourceDist);
    void SetConfiguration(NGMSystemConfiguration* conf){_conf = conf;}
    TH2* CreateDetectorLiveTimeScaling(std::vector<double> livetime,double maxlivetime);
    void ScaleHistos(TList* histList, TH2* hScale);
    static TH2* CreateVoidFromMaskAnti(TH2* mask, TH2* anti);
    
    NGMSystemConfiguration* _conf;
    ImageMaker _imaker;
    TTree* _maskTree;//!
    TTree* _antiTree;//!
    TTree* _voidTree;//!
    std::vector<double> masklivetime;
    std::vector<double> antilivetime;
    std::vector<double> voidlivetime;
    TList* histList;
    
	ClassDef(PDSTImager,1)
};


class PDSTImagerSelector : public TSelector {
    public :
    TTree          *fChain;   //!pointer to the analyzed TTree or TChain
    NGMSystemConfiguration* conf;
    NGMBlockMapping *bmap;
    std::vector<NGMBlockPSD*> bpsd;
    // Declaration of leaf types
    NGMHit         *hit;
    // List of branches
    TBranch* hit_br; //!
    
    TH2* fDetImage_Neutron;
    TH2* fDetImage_Neutron_Tight;
    TH2* fDetImage_Gamma;
    TString maskAntiVoid;
    
    PDSTImagerSelector(TTree * /*tree*/ =0)
    : fChain(0), conf(0), bmap(0), hit(0), hit_br(0), fDetImage_Neutron(0), fDetImage_Neutron_Tight(0), fDetImage_Gamma(0) { }
    virtual ~PDSTImagerSelector() { }
    virtual Int_t   Version() const { return 2; }
    virtual void DoConf(const char* fname);
    virtual void    Begin(TTree *tree);
    virtual void    SlaveBegin(TTree *tree);
    virtual void    Init(TTree *tree);
    virtual Bool_t  Notify();
    virtual Bool_t  Process(Long64_t entry);
    virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0);// { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
    virtual void    SetOption(const char *option) { fOption = option; }
    virtual void    SetObject(TObject *obj) { fObject = obj; }
    virtual void    SetInputList(TList *input) { fInput = input; }
    virtual TList  *GetOutputList() const { return fOutput; }
    virtual void    SlaveTerminate();
    virtual void    Terminate();
    
    ClassDef(PDSTImagerSelector,0);
};


#endif //#ifndef __PDSTImager_H__
