#ifndef __NGMTIMEGAPFINDER_H__
#define __NGMTIMEGAPFINDER_H__
/*
 *  NGMTimeGapFinder.h
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/5/08.
 *  Copyright 2008 LLNL. All rights reserved.
 *
 */

/// \brief NGMTimeGapFinder extracts livetime and average hit rate
/// from a hit stream by identifying large timing gaps in the data
/// inconsistent with the time average rate.  This might have trouble 
/// with any runs where the average rate varies dramatically during 
/// the run.

#include "Rtypes.h"
#include "NGMHit.h"

class NGMTimeGapFinder : public TNamed
{
public:
  
  NGMTimeGapFinder();
  virtual ~NGMTimeGapFinder() ;
  double GetDeadTime() const ;
  double GetLiveTime() const ;
  double GetLiveTimePct() const ;
  double GetRunDuration() const ;
  TH1D  *GetGapHistogram() const { return _gaps ; }
  void   SetAcquireLiveHistogram(int live = 1) ;  // turn on acquisition of live hist
  TH1D  *GetLiveHistogram() const { return _live ; }
  TH1   *DrawGaps(const Option_t *opt = "")  ;  // draw gaps histogram
  bool   nextTimeIsGap(NGMTimeStamp nexttime);
  bool   Reset();
  const NGMTimeStamp GetLastTime() const { return _timeOfPreviousHit; }
  
private:
  Long64_t     _counts;
  Long64_t     _ngaps;
  TH1D        *_gaps;
  TH1D        *_live;
  double       _deadtime;
  double       _averageInterval;
  NGMTimeStamp _firstTimeOfRun;
  NGMTimeStamp _timeOfPreviousHit;
  NGMTimeStamp _firstTimeSinceGap;

  // functions
  void         Init_gaps();  // initialize _gaps histogram
  void         Init_live();  // initialize _live histogram
  
  ClassDef(NGMTimeGapFinder,1)
};


//______________________________________________________________________________
//
//  Inline Implementations
//

inline double
NGMTimeGapFinder::GetDeadTime() const
{
  return _deadtime;
}

inline double
NGMTimeGapFinder::GetLiveTime() const
{
  return (GetRunDuration()  - _deadtime);
}

inline double
NGMTimeGapFinder::GetLiveTimePct() const
{
  return GetLiveTime() / ( GetLiveTime() + GetDeadTime() ) ;
}

inline double
NGMTimeGapFinder::GetRunDuration() const
{
  return _timeOfPreviousHit.TimeDiffNanoSec(_firstTimeOfRun);
}


#endif // __NGMTIMEGAPFINDER_H__
