#ifndef __NGMTOF_H__
#define __NGMTOF_H__
//
//  NGMTof.h
//  NGMDaq
//
//  Created by Newby, Robert Jason on 08/02/12.
//  Copyright 2012 ORNL. All rights reserved.
//

#include "NGMModule.h"
#include "NGMHit.h"

/// \brief  NGMTof analyzes a stream of hits and 
/// and uses a designated channel as the trigger for a ToF measurment.

class NGMTof : public NGMModule
{
public:
  NGMTof();
  NGMTof(const char* name, const char* title);
  ~NGMTof();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  void  LaunchDisplayCanvas(); // *MENU*
  void  ResetHistograms();
  void Print(Option_t* option = "") const;
  void SetTriggerChannel(int newVal) {_triggerChannel=newVal;}
  void setMaxNanoSecInList(double newVal) { _maxNanosecondsInList=newVal; } // *MENU*
  void doTimeSincePreviousOnly(bool newVal = true) {_doTimeSincePreviousOnly = newVal;}
  ULong64_t  getTriggerCount() const {return _triggerCount;}
  void setScaleFactor(double newVal){scaleFactor = newVal;}
    
private:
  bool processConfiguration(const NGMSystemConfiguration* conf);
  bool  processHit(const NGMHit* tHit);
  void analyzeTimeSincePrev(const NGMHit* hit);
  void analyzeAllPairs(const NGMHit* hit);
  void analyzePair(const NGMHit* trig, const NGMHit* hit);
  void InitCommon();
  int _triggerChannel;
  bool _doTimeSincePreviousOnly;
  NGMHit* _prevCfHit; //!
  TList* _hitList; //!
  double _maxNanosecondsInList;
  ULong64_t _hitsAnalyzed;
  ULong64_t _triggerCount;
  double scaleFactor;

  ClassDef(NGMTof,4)
  
};


#endif // __NGMTOF_H__

