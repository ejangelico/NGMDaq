#ifndef __NGMBlockAliveMonitor_H__
#define __NGMBlockAliveMonitor_H__
//
//  NGMBlockAliveMonitor.h
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMModule.h"
#include "NGMBlockMapping.h"
#include <map>
#include "NGMHit.h"

// Forward declarations
class TH2;
class TProfile2D;

/// \brief  NGMBlockAliveMonitor analyzes a stream of hits and 
/// produces a block array image for a block array system.

class NGMBlockAliveMonitor : public NGMModule
{
public:
  NGMBlockAliveMonitor();
  NGMBlockAliveMonitor(const char* name, const char* title);
  ~NGMBlockAliveMonitor();
  
  bool  init();
  bool  finish();
  void  LaunchDisplayCanvas(); // *MENU*
  void  ResetHistograms();
  void Print(Option_t* option = "") const;
  void GetDurationPerBlock(int nblocks, Double_t* duration, Double_t* duration2, Double_t* rates);

private:
  bool processConfiguration(const NGMSystemConfiguration* conf);
  bool processHit(const NGMHit* tHit);
  bool processPacket(const NGMBufferedPacket* packet);
  bool processMessage(const TObjString* mess);
  void InitCommon();

  NGMTimeStamp _runBeginTime;
  TH2* _blockAlive;
  TProfile2D* _baselineMonitor;
  NGMBlockMapping _bmap;
  enum localconsts { ngates = 8, maxChannels = 5000};
  int _gatewidth[maxChannels][ngates];
  int _psdScheme;
    
  ClassDef(NGMBlockAliveMonitor,3)
  
};

#endif // __NGMBlockAliveMonitor_H__

