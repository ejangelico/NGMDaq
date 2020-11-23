#ifndef __NGMBlockArrayMonitor_H__
#define __NGMBlockArrayMonitor_H__
/*
 *  NGMBlockArrayMonitor.h
 *  NGMDaq
 *
 *  Created by Jason Newby on 07/08/11.
 *  Copyright 2011 ORNL. All rights reserved.
 *
 */

#include "NGMModule.h"
#include "NGMHit.h"
#include "TObjArray.h"
#include "NGMSpy.h"
#include "NGMBlockMapping.h"

// Forward declarations
class TTimer;
class TCanvas;
/// \brief  NGMBlockArrayMonitor analyzes a stream of hits and 
/// produces a block array image for a block array system.


class NGMBlockArrayMonitor : public NGMModule
{
public:
  NGMBlockArrayMonitor();
  NGMBlockArrayMonitor(const char* name, const char* title);
  ~NGMBlockArrayMonitor();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  void  LaunchDisplayCanvas(); // *MENU*
  void  ResetHistograms();
  
private:
  void processConf(const NGMSystemConfiguration* confBuffer);
  void InitCommon();
  enum localconsts { ngates = 8, maxChannels = 5000};  
  bool  analyzeHit(const NGMHit* tHit);
  int blocknrows;
  int blockncols;
  TH2* _blockArray;
  TH2* _blockColsVTime;
  TH1* _blockCols;
  time_t _minTime;
  NGMTimeStamp _beginTime;
  NGMBlockMapping _bmap;
    
  ClassDef(NGMBlockArrayMonitor,5)
  
};


class NGMBlockArrayMonitorDisplay : public TObject
{
  TCanvas* cBlockDisplay;
  NGMSpy spy;
  TString _moduleName;
  
public:
  NGMBlockArrayMonitorDisplay();
  NGMBlockArrayMonitorDisplay(const char* moduleName);
  
  virtual Bool_t HandleTimer(TTimer* timer);
  
  ClassDef(NGMBlockArrayMonitorDisplay,0)
};

#endif // __NGMBlockArrayMonitor_H__
