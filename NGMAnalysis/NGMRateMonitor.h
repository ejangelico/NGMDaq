#ifndef __NGMRATEMONITOR_H__
#define __NGMRATEMONITOR_H__
/*
 *  NGMRateMonitor.h
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/5/08.
 *  Copyright 2008 LLNL. All rights reserved.
 *
 */

#include "NGMModule.h"
#include "NGMHit.h"
#include "NGMTimeGapFinder.h"
#include <fstream>

// Forward declarations
class NGMSystemConfiguration;
class sqlite3;
class sqlite3_stmt;

/// \brief  NGMRateMonitor analyzes a stream of hits and monitors channel-by-channel
/// hit rates as well as rates per particle type.


class NGMRateMonitor : public NGMModule
{
public:
  NGMRateMonitor();
  NGMRateMonitor(const char* name, const char* title);
  ~NGMRateMonitor();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  void  LaunchDisplayCanvas(); // *MENU*
  void  ResetHistograms();
  double GetLiveTime() const { return _gapFinder.GetLiveTime(); }
  double GetRunDuration() const { return _gapFinder.GetRunDuration(); }
  const TH1* GetChannelCounts() const {return _channelHitCount; }
  const TH1* GetParticleCounts() const {return _particleTypeHitCounts; }
  void SetForceDBUpdate(bool forceUpdate = true) { _forceDBUpdate = forceUpdate; }
  void SetOutputPeriod(double newVal){ _outputPeriodSec = newVal; }
  void SetSaveToDB(bool saveToDB = true) {_saveToDB=saveToDB;}

private:
  void InitCommon();
  void LogRates();
    
  bool  analyzeHit(const NGMHit* tHit);
  void UpdateDatabaseRunHeader();
  
  Long64_t _runnumber;
  TH1* _channelHitCount;
  TH1* _particleTypeHitCounts;
  TH1* _neutronCountsPerDetector;
  TH1* _gammaCountsPerDetector;

  double _livetime;
  double _runduration;
  NGMTimeStamp _firstTimeOfRun;
  NGMTimeStamp _timeOfPreviousHit;
  NGMTimeGapFinder _gapFinder;
  NGMSystemConfiguration* _config;
  bool _forceDBUpdate;
  double _outputPeriodSec;
  double _previousOutput;
  std::ofstream _ratelog;//! ROOTCINT has no streamer for this
  sqlite3 *_db; //!
  sqlite3_stmt *_ppStmt; //!
  bool _saveToDB;
  
ClassDef(NGMRateMonitor,5)
  
};

#endif // __NGMRATEMONITOR_H__
