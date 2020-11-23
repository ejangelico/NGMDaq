#ifndef __CoincidenceMaker_H__
#define __CoincidenceMaker_H__

#define PARTICLE_TYPES 4
#define DT_PULSES_TO_SAVE 1000

#include "TObject.h"
#include "TString.h"
#include "TChain.h"
#include "TH2F.h"
#include "NGMHit.h"
#include "NGMModule.h"

class TFolder; //Forward declaration

class CoincidenceVariables : public TObject
{
public:
    CoincidenceVariables() { ClearData(); }
//    virtual ~CoincidenceVariables() {;}
    
    void ClearData();
    
    Int_t pixelX;
    Int_t pixelY;
    Double_t relativeTime;
    Int_t particleType;
    Double_t energy;
    
    ClassDef(CoincidenceVariables,1)
};

class CoincidentEvent : public TObject
{
public:
    CoincidentEvent();
    virtual ~CoincidentEvent();
    
    void ClearData();
    
    TClonesArray *liqCoinc;
    static TClonesArray *sLiqCoinc;
    
    Int_t numLiqGamma;
    Int_t numLiqNeutron;
    CoincidenceVariables *ImagerHit;
    
    void AddLiqCoinc(Int_t iPartID, Double_t tDiff, Double_t en);
    
    ClassDef(CoincidentEvent,1)
};

class CoincidenceMaker : public TObject
{
public:
    CoincidenceMaker();
    ~CoincidenceMaker();

    void ConstructCoincidences();
    void SetCoincidenceWindow(Double_t halfWinNano) {halfWindowNanoSec = halfWinNano;}
    void SetDTSlotChannel(Int_t theSlot, Int_t theChan);
    void SetLiqSlot(Int_t theSlot, Int_t nLiquids);
    void SetOutputFileName(TString ofname);
    void SetDetectorPixels(Int_t nDetPix) {numDetPix = nDetPix;}
    
    void AddFileToChain(TString fileName);
    void SetupHistograms(TFolder* saveFolder = 0);
    void SaveHistograms();
    void processHit(const NGMHit* hit);
    
private:
	
    Bool_t bFirstPrint;
    Bool_t bFirstDT;
    NGMTimeStamp mostRecentDTtime;
    Int_t currentDTindex;

    
    TChain *chain;
    
    TTree *coincTree;

    NGMHit *theHit;
    
    TFile *fout;
    CoincidentEvent *processedEvent;
    
    std::list<NGMHit*> _liqHits;
    std::list<NGMHit*> _imagerHits;

    NGMTimeStamp dtTimes[DT_PULSES_TO_SAVE];

    NGMHit *hit0;
    
    Int_t dtSlot, dtChan;
    Int_t liqSlot;
    Int_t numLiq;
    Int_t numDetPix;
    Double_t halfWindowNanoSec;
    
    TH2F **hLiqImTimeDiff;
    TH1F *hDTvsImTime[PARTICLE_TYPES];

    void DoOrientation();
    void AddImagerHit(const NGMHit *theImagerHit);
    void AddLiquidHit(const NGMHit *theLiquidHit);
    Int_t GetParticleStatus(const NGMHit *theHit);
    
    ClassDef(CoincidenceMaker,1)
};

class CoincToTuple : public NGMModule
{
public:
    CoincToTuple();
    CoincToTuple(const char* name, const char* title);
    ~CoincToTuple();
    
    bool  init();
    bool  finish();
    bool processConfiguration(const NGMSystemConfiguration* conf);
    bool processHit(const NGMHit* tHit);
    void  LaunchDisplayCanvas(); // *MENU*
    void  ResetHistograms();
    
    CoincidenceMaker* coim; //!
    
private:
    void InitCommon();
    
    ClassDef(CoincToTuple,2)
    
};



#endif
