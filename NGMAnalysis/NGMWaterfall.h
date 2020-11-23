#ifndef __NGMWATERFALL_H__
#define __NGMWATERFALL_H__


/*
 *  NGMWaterfall.h
 *  NGMDaq
 *
 *  Created by Jerome Verbeke on 06/07/07.
 *  Copyright 2007 LLNL. All rights reserved.
 *
 */

#include "NGMModule.h"
#include "TTimeStamp.h"
#include "TText.h"
#include "NGMHit.h"
#include <deque>

class NGMBufferedPacket; // forward declaration
class TH1; // forward declaration
class TH2; // forward declaration
class TGraph; // forward declaration
class NGMConfigurationTable; //forward declaration
class TList; //forward declaration
class TClonesArray; // forward declaration
class TTree; // forward declaration
class TCanvas; // forward declaration
class NGMZoomGui; // forward declaration
class TLegend; // forward declaration

class NGMWaterfall: public NGMModule{
public:
  NGMWaterfall();
  NGMWaterfall(const char*, const char*);
  virtual ~NGMWaterfall();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  bool  analyzeHit(const NGMHit* tHit);
  void doDraw(); // *MENU*
  void ResetHistograms(); // *MENU*
  void SetDisplayInterval(double delay); // *MENU*
  void SetNumberOfPointsDisplayed(int size); // *MENU*
  void DrawMarkerText(bool drawTextFlag = true){_drawTextFlag = drawTextFlag;} // *MENU*
  void SkipsChanged(Int_t pos);
  static double* genLogBins(int nbins, double axismin, double axismax);
  void UseZoomGui(bool useZoomGui = true){_useZoomGui = useZoomGui;}
  void fitPoisson(double elapsed_time);
  void enableAlarm(bool enabled=true);
  void setAlarmLevel(double likelihood);
// This sets the likelihood under which we raise an alarm. For this 
// likelihood, we consider all the data points in the range beginning
// after the short time scale bursty region (set by setMinimumTimeCutoff())
// and ending at the point where the long time scale curve (fit by a 
// function replicating a Poisson process) hits 0.5, i.e. half a count.
// If the likelihood of the data points in that region is lower than 
// the level set here, an alarm is raised.
  double getAlarmLevel();
  void setMinimumTimeCutoff(double cutoff_ns);
// This is the cutoff time that delimits the bursty short time scale
// region and the empty valley above it. This time cutoff is very much
// geometry dependent
  void enableBurstFilter(bool enabled=true);
// This method drops all the hits that are within the minimum cutoff
// time from the previous one. 
  void residualsScaled2Sigmas(bool enabled=true);
// This method scales the residuals to the sigma of the Poisson
  double getMinimumTimeCutoff();
  bool getAlarmStatus(); // returns true when the alarm was triggered
  void LaunchDisplayCanvas() { doDraw(); }; 
  void setNskips(int skip_value);
  bool setMaxTime(double time);
// Set the length of time in nanoseconds we want to analyze the data

  static double getFactor(int nbins, double axismin, double axismax);

private:

  unsigned long long _hitsAnalyzed;
  TGraph* timeDiffWaterfall;
  double dispInt; // time interval in seconds between waterfall updates
  int maxListSize; // largest number of points displayed
  TList* pList; // list of hits recorded
  TClonesArray* wmList; // waterfall marker list
  TList* wtList; // waterfall text list
  TList* smList; // stairstep marker list
  TList* stList; // stairstep text list
  bool _drawTextFlag;
  bool _useZoomGui;
  NGMTimeStamp* lastDraw;
  NGMTimeStamp* firstTime;
  NGMZoomGui* myZoomer; //!
  TH1F* h1;
  TH1* _residualPlot;
  TH1* _ronPlot;
  TLegend* leg;
  long long _runNumber; 
  NGMTimeStamp _runBegin;
  double xminbin, xmaxbin;
  int totalbins;
  bool alarmEnabled;
  double alarm_threshold;
  float delta_ns;
  bool alarm;
  bool sigmas;
  double min_bin_cutoff, max_bin_cutoff;
  int Nskips;
  bool burstFilterSetting;
  double maxTime;

  ClassDef(NGMWaterfall,3);
};

#endif // __NGMWATERFALL_H__
