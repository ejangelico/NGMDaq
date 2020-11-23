#ifndef __NGMBlockFlood_H__
#define __NGMBlockFlood_H__
/*
 *  NGMBlockFlood.h
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/5/08.
 *  Copyright 2008 LLNL. All rights reserved.
 *
 */

#include "NGMModule.h"
#include "NGMHit.h"
#include "TObjArray.h"
#include "NGMSpy.h"
#include "TMatrixD.h"
#include "NGMBlockMapping.h"

// Forward declarations
class TTimer;
class TCanvas;
/// \brief  NGMBlockFlood analyzes a stream of hits and 
/// produces a flood image for a block detector.


class NGMBlockFlood : public NGMModule
{
public:
  NGMBlockFlood();
  NGMBlockFlood(const char* name, const char* title);
  ~NGMBlockFlood();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  void  LaunchDisplayCanvas(); // *MENU*
  void  ResetHistograms();
  void  SetEnergyCut(double elow = -1.0, double ehigh = -1.0);
  // 0=individual tube energy, 1=total block energy, 2=floods
  void  SetDisplayType(int newVal) {_displayType=newVal;} // *MENU*
    double GetPMTAmplitude(const NGMHit*,int ipmt);
    double GetPSD(const NGMHit*);
private:
  void InitCommon();
  enum localconsts { ngates = 8, maxChannels = 5000};  
  bool  analyzeHit(const NGMHit* tHit);
  double _energyCutLow;
  double _energyCutHigh;
  TObjArray _energyList;
  TObjArray _peakList;
  TObjArray _peakSumList;
  TObjArray _floodList;
  TObjArray _psdList;
  int _gatewidth[maxChannels][ngates];
  int blocknrows;
  int blockncols;
  TMatrixD _blockRowCol;
  // Detector number per hardware channel block
  std::vector<int> detNum;
  std::vector<int> chanNum;
  int _displayType;
  int _psdScheme;
  NGMBlockMapping _bmap;
ClassDef(NGMBlockFlood,10)
  
};


class NGMBlockFloodDisplay : public TObject
{
  TCanvas* cFloodDisplay;
  NGMSpy spy;
public:
  NGMBlockFloodDisplay();
  
  virtual Bool_t HandleTimer(TTimer* timer);

  ClassDef(NGMBlockFloodDisplay,0)
};

#endif // __NGMBlockFlood_H__
