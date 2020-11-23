#ifndef __NGMBlockDetectorCalibrator_H__
#define __NGMBlockDetectorCalibrator_H__


/*
 *  NGMBlockDetectorCalibrator.h
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
#include "NGMBaseline.h"
#include <map>
#include <queue>
#include "NGMHitProcess.h"

class NGMBufferedPacket; // forward declaration
class TH1; // forward declaration
class TH2; // forward declaration
class NGMConfigurationTable; //forward declaration
class NGMHit; //forward declaration
class TProfile;// forward declaration
class TCutG; //forward declaration
class NGMHit; //forward declartion

class NGMBlockDetectorCalibrator: public NGMModule{
public:
  NGMBlockDetectorCalibrator();
  NGMBlockDetectorCalibrator(const char*, const char*);
  virtual ~NGMBlockDetectorCalibrator();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  bool  analyzePacket(NGMBufferedPacket* packet);
  bool  analyzeHitBlock(NGMHit* tHit);
  bool  analyzeHitSingle(NGMHit* tHit);
  int   getPlotIndex(const NGMHit* tHit) const;
  void  setConstantFraction(double newVal) { _constantfraction = newVal; }
  bool  SaveNeutronCuts(const char* filename); // *MENU*
  bool  ReadNeutronCuts(const char* filename, const char* prefixname); // *MENU*
  bool  SetROIFileName(const char* filename); // *MENU*
  bool  SetCalFileName(const char* filename); // *MENU*
  bool FindCalibrationFileFromDB(const char* runnumber); // *MENU*
  TMatrixD& GetPSDWidthScaling() {return _pixelSigmaScaling;}

protected:
    void InitCommon();
    bool  processConfiguration(const NGMSystemConfiguration* conf);
    bool  processHit(const NGMHit* tHit);
    bool  processPacket(const NGMBufferedPacket* packet);
    bool  processMessage(const TObjString* mess);

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
  // Neutron Indentification
  TCutG* _neutronGate[maxChannels];
  TH1* _nMean[maxChannels];
  TH1* _nSigma[maxChannels];
  TH1* _gMean[maxChannels];
  TH1* _gSigma[maxChannels];
  
  // Constant Fraction Timing
  double _constantfraction;
  // Timing Calibrations
  double _timingCal[maxChannels];
  double _cfdscale[maxChannels];
  
  TString _roiFileName;
  TString _calFileName;
  int _channelenabled[maxChannels];
  TMatrixD _pixelLUT[maxChannels/4];
  float _pixelLUTMinX[maxChannels/4];
  float _pixelLUTMaxX[maxChannels/4];
  float _pixelLUTMinY[maxChannels/4];
  float _pixelLUTMaxY[maxChannels/4];
  TMatrixD _pixelEnergyScale[maxChannels/4];
  TMatrixD _blockRowCol;
  TMatrixD _pixelkeVADC;
  TMatrixD _pixelSigmaScaling;
  int _psdScheme;
  std::map<int,NGMBlockPSD> _blockPSD;
  NGMBlockMapping _bmap;
  std::map<int,NGMHitProcess> _singleDetectorPSD;
  std::vector<double> _nombaselines;
    
public:

  ClassDef(NGMBlockDetectorCalibrator,11)
};

class NGMBlockBaseline : public NGMBaseline
{
  public:
  NGMBlockBaseline(){}
  virtual ~NGMBlockBaseline(){}
  NGMBlockBaseline(double gatelength, int minentries, double tau);
  bool AnaHit(NGMHit*);
  double _tau;
  double _rms; //cached rms
  double _sigthresh; // threshold for correcting hit
  
  ClassDef(NGMBlockBaseline,1)
};


#endif // __NGMBlockDetectorCalibrator_H__
