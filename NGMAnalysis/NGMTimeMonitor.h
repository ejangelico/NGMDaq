#ifndef __NGMTIMEMONITOR_H__
#define __NGMTIMEMONITOR_H__


/*
 *  NGMNeutronMonitor.h
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/28/06.
 *  Copyright 2006 LLNL. All rights reserved.
 *
 */

#include "NGMModule.h"
#include "TTimeStamp.h"
#include "TText.h"
#include "NGMHit.h"
#include <vector>
#include "TObjArray.h"
#include "TH2.h"
#include "TH1.h"
#include "TList.h"

class NGMBufferedPacket; // forward declaration
class TGraph; // forward declaration
class NGMConfigurationTable; //forward declaration
class TSeqCollection; //forward declaration
class TClonesArray; // forward declaration
class TTree; // forward declaration
class TCanvas; // forward declaration
class NGMSystemConfiguration; //forward declaration
class TDirectory;  //forward declaration
class THnSparse; //forward declaration

#include "TTimer.h"

struct partSelection_st
{
  int partid;
  double energymin;
  double energymax;
};

struct ngmtimemonitortuple_st
{
    short ch1;
    short ch2;
    float e1;
    float e2;
    float n1;
    float n2;
    float g1;
    float g2;
    float psd1;
    float psd2;
    float t;
};

class NGMTimeMonitor: public NGMModule{
public:
  NGMTimeMonitor();
  NGMTimeMonitor(const NGMTimeMonitor&){}
  NGMTimeMonitor(const char*, const char*);
  virtual ~NGMTimeMonitor();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  bool  analyzeHit(const NGMHit* tHit);
  void displayChain(TObjArray* pList, Long64_t first, Long64_t length);
  void setMaxNanoSecInList(double newVal); // *MENU*
  void ResetHistograms(); // *MENU*
  void enableDisplay(bool newVal = true); // *MENU*
  void Print(Option_t* option = "") const;
  void initializeTimePlots();
  void LaunchDisplayCanvas();
  void SetFillAllChannelByChannel(bool newVal = true) {_fillAllChannelByChannel = newVal;} // *MENU*
  void SetTimeRangeNS(double timerange) {_timerange_ns = timerange;}
  void SetNearestNeighborSkip(int newVal = 4) { _nearestNeighborSkip = newVal;}
    
  NGMSystemConfiguration* calTiming(double fitRange = 2.0, NGMSystemConfiguration* sysConf = 0); // *MENU*
  TH1* mergeTiming(TString detectorName,
					NGMSystemConfiguration* sysConf=0,
					TString histPrefix="TIMEMONITOR_timeDistribution_");

private:

  unsigned long long _hitsAnalyzed;
  NGMTimeStamp _prevTime;
  NGMTimeStamp _firstTime;
  bool _fillAllChannelByChannel;
  
  TObjArray* _timeDistributionPerChannel; //TH1
  TH2* _timeComparisonShort;
  TH2* _timeComparisonLong;  
  
  TH2* _hCfdWordMonitor;
  TList* _hitList; //!
  double maxNanosecondsInList;
 
  Long64_t _runNumber;   
  TTimer _displaytimer;
  
  double _timerange_ns;
  bool _doTuple;//!
  TTree* _timeTuple;//!
  ngmtimemonitortuple_st _tTp;//!
  int _nearestNeighborSkip;
  ClassDef(NGMTimeMonitor,7)
};

class NGMHitText: public TText
{
  
private:
  static bool alldrawenergy;
  bool drawenergy;
  TString _channel;
  double _energy;
public:
    NGMHitText(){;}
  NGMHitText(Double_t x, Double_t y, const char* chanid, double energy);
  void ToggleAll(); // *MENU*
  void ToggleThis();  // *MENU*
  
  virtual ~NGMHitText(){;}
  
  ClassDef(NGMHitText,1)
};

class NGMTimeDisplay: public NGMModule{
public:
  NGMTimeDisplay();
  NGMTimeDisplay(const char*, const char*);
  virtual ~NGMTimeDisplay();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  int  getPlotIndex(const NGMHit* tHit) const;
  void displayChain(const TSeqCollection* pList, Long64_t first, Long64_t length);
  void setNskipsForWaterFall(int newVal); // *MENU*
  
private:
    
  enum localconsts {maxChannels = 120, maxskips = 20};
	TCanvas* cTimeDisplay;
	TCanvas* cTimeDisplay1;
  TGraph* tgTimeDisplay;
  TGraph* tgTimeDisplay1;
  TClonesArray* tmTimeDisplay;
  TClonesArray* tmTextDisplay;
  TClonesArray* tmTimeDisplay1;
  TClonesArray* tmTextDisplay1;
  TTimeStamp _runBegin;
  
  TH2* hAxisChainDisplay;
  TH2* hAxisWaterfallDisplay;
  
  TGraph* timeDiffWaterFall;
  int nskipsforwaterfall;
  TString _detNames[maxChannels];
  int _detColors[maxChannels];
  int numberOfHWChannels;
  int numberOfSlots;
  TList* _hitList;
  
  ClassDef(NGMTimeDisplay,1)
};



#endif // __NGMTIMEMONITOR_H__
