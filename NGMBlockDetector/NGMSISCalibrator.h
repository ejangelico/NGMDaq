#ifndef __NGMSISCalibrator_H__
#define __NGMSISCalibrator_H__


/*
 *  NGMSISCalibrator.h
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/28/06.
 *  Copyright 2006 LLNL. All rights reserved.
 *
 */

#include "NGMModule.h"
#include "TTimeStamp.h"
#include "TString.h"
#include "TMatrixD.h"
#include "NGMBlockMapping.h"
#include "NGMBlockPSDMaker.h"
#include <map>
#include <queue>

class NGMBufferedPacket; // forward declaration
class TH1; // forward declaration
class TH2; // forward declaration
class NGMConfigurationTable; //forward declaration
class NGMHit; //forward declaration
class TProfile;// forward declaration
class TCutG; //forward declaration
class NGMHit; //forward declartion

class NGMSISCalibrator: public NGMModule{
public:
  NGMSISCalibrator();
  NGMSISCalibrator(const char*, const char*);
  virtual ~NGMSISCalibrator();
  
  bool  init();
  bool  finish();
protected:
  bool  processConfiguration(const NGMSystemConfiguration* conf);
  bool  processPacket(const NGMBufferedPacket* packet);
  bool  processHit(const NGMHit* tHit);
  bool  processMessage(const TObjString* mess);
  void  InitCommon();
  bool  analyzeHit(NGMHit*);
  bool  analyzePacket(NGMBufferedPacket* packet);
private:
  enum localconsts { ngates = 8, maxChannels = 5000};
  int nChannels;
  int numberOfHWChannels;
  int numberOfSlots;
  TString _detNames[maxChannels];
  int _detType[maxChannels];
  int _gatewidth[maxChannels][ngates];
  int _gateoffset[maxChannels][ngates];
  double _nanosecondsPerSample[maxChannels];
  int _mainGateLength[maxChannels];
  int _mainGatePreTrigger[maxChannels];
  int _rawsampleStartIndex[maxChannels];
  int blocknrows;
  int blockncols;
  
  // Energy Calibrations
  double _keVperADC[maxChannels];

  // Timing Calibrations
  double _timingCal[maxChannels];
  
  int _channelenabled[maxChannels];
  TMatrixD _pixelEnergyScale[maxChannels];
  TMatrixD _blockRowCol;
  TArrayD _detkeVADC;
  int _psdScheme;
  std::map<int,NGMBlockPSD> _blockPSD;
  
public:

  ClassDef(NGMSISCalibrator,1)
};


#endif // __NGMSISCalibrator_H__
