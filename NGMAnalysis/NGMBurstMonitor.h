#ifndef __NGMBURSTMONITOR_H__
#define __NGMBURSTMONITOR_H__

#include "NGMModule.h"
#include "NGMHit.h"
#include "TGraph.h"
#include "TVirtualPad.h"
#include "TCanvas.h"
#include "TH1F.h"
#include <vector>
#include <deque>
#include "TH2.h"
#include "TH1.h"
#include "NGMBufferedPacket.h"
#include "TLegend.h"
#include "NGMZoomGui.h"
#include "TPaveText.h"
#include "NGMTimeGapFinder.h"

struct burstSelect_st
{
  int partid;
  int mincounts;
  int maxcounts;
};

class NGMMultipleScatteringHit; //Forward Declaration

class NGMBurstMonitor: public NGMModule{
public:
  NGMBurstMonitor();
  NGMBurstMonitor(const NGMBurstMonitor&){}
  NGMBurstMonitor(const char*, const char*);
  virtual ~NGMBurstMonitor();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  void ResetHistograms(); // *MENU*
  void setHitsInBurst(int hitsInBurst){_hitsInBurst = hitsInBurst;}
  void setNumberZoomedGraphs(int ngraphs = 50) {_NBurstZoomes = ngraphs;}
  void setBurstWindowNS(int burstWindowNS){_burstWindowNS = burstWindowNS;}
  void setGraphWindowNS(int graphWindowNS){_graphWindowNS = graphWindowNS;}
  void AddRequirement(int tpartid, int mincounts, int maxcounts=100000);
  void AddChannelRequirement(ULong64_t mask1, ULong64_t mask2 = 0, ULong64_t mask3 = 0);
  void doDraw(double hiVal=0, bool zoomer = true); // *MENU*
  void setDrawGraphics(bool newVal = true) { _drawGraphics = newVal; } // *MENU*
  void LaunchDisplayCanvas();
  void setPushFullWindow(bool pushFullWindow = true) { _pushFullWindow = pushFullWindow; }
  TPaveText* getParLabel(Double_t x1, Double_t y1, Double_t x2, Double_t y2, Option_t* option = "br");
  void Print(Option_t* option = "") const;
  double MeasureShape(int partid, int distid, double threshold);
  void MeasureShapeWriteDB(int partid, int distid, double threshold);
  static double MeasureShape(TH1* tDist, double threshold);
  double getLiveTime() const { return _gapfinder.GetLiveTime();}
  
private:
  bool  analyzeHit(const NGMHit* tHit); 
  bool  anaBurst(int index);
  bool  anaBurstTiming(NGMBufferedPacket* burst);
  bool FindBurstStart(NGMBufferedPacket* packet);

  TGraph* gBurst_gbgamma;
  TGraph* gBurst_gbmuon;
  TGraph* gBurst_mbgamma;
  TGraph* gBurst_mbmuon;
  TGraph* gBurst_lsneutron;
  TGraph* gBurst_lsmuon;
  TGraph* gBurst_lsgamma;
  TGraph* gBurst_hettlid;
  TGraph* gBurst_heid;
  TH1F*   _burstLenHist;
  TH1**  _burstTimingSummary; //!
  TH1F*   _burstSizeHist;
  TH2*   _gbEnergyvsMult;
  TH2*   _lsnEnergyvsMult;
  TH2*   _lsgEnergyvsMult;
  
  
  TCanvas* cBurst;
  NGMBufferedPacket* _burstList; //!

  std::deque<NGMMultipleScatteringHit*>::size_type _candidatePosition; //!
  std::deque<NGMMultipleScatteringHit*> hitvec; //!
  std::vector<TVirtualPad*> padvec; //!
  std::vector<burstSelect_st> _partSelection;
  std::vector<ULong64_t> _mask1;
  std::vector<ULong64_t> _mask2;
  std::vector<ULong64_t> _mask3;
  
  int _hitsInBurst;
  int _NBurstZoomes;
  long _burstWindowNS;
  long _graphWindowNS;
  long long _runNumber;
  bool _drawGraphics;
  long _Num_burst;
  bool _timeInitDone;
  bool _pushFullWindow;
  
  NGMTimeStamp _lastBurst;
  NGMTimeStamp _firstTime;
  NGMTimeStamp _lastDraw;
  NGMZoomGui* _myZoomer; //!
  TPaveText* _parLabel;
  TPaveText* _burstParLabel;
  TLegend* _legend;
  NGMTimeGapFinder _gapfinder;
  
public:
  bool doMultEnergyCut;
  double lsnCutSlope;
  double lsnCutConstant;
  double lsgCutSlope;
  double lsgCutConstant;
  double gbCutSlope;
  double gbCutConstant;
  
private:

  ClassDef(NGMBurstMonitor,5)
};


#endif 
