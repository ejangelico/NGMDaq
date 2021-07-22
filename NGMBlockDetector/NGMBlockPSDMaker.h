#ifndef __NGMBLOCKPSDMAKER_H__
#define __NGMBLOCKPSDMAKER_H__
//
//  NGMBlockPSDMaker.h
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMModule.h"
#include "NGMHit.h"
#include "TObjArray.h"
#include "NGMSpy.h"
#include "TMatrixD.h"
#include "TObjString.h"
#include "NGMBlockMapping.h"
#include <map>
#include "TSpline.h"

// Forward declarations
class TCanvas;
class TH2;
class TSpline3;
class TCutG;

/// \brief  NGMBlockPSDMaker analyzes a stream of hits and 
/// produces a block array image for a block array system.

class NGMBlockPSDMaker : public NGMModule
{
public:
  NGMBlockPSDMaker();
  NGMBlockPSDMaker(const char* name, const char* title);
  ~NGMBlockPSDMaker();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  void  LaunchDisplayCanvas(); // *MENU*
  void  ResetHistograms();
  void SetMaxADC(int newVal){_maxADC = newVal;}
  void SetPSDLimits(double min, double max){ _minPSD = min; _maxPSD = max; }
  void Print(Option_t* option = "") const;
  bool fitNeutronGammaBand(const char* detectorPrefix,double minx = 50.0,double maxx = 4000.0, int nbins = 12, bool ADCorE = true);
  bool CalculateFOM1(TH2* hNScore);
  TSpline3* MakeCubicSpline(const char* name, TH1* hX, TH1* hY);
private:
  bool processConfiguration(const NGMSystemConfiguration* conf);
  bool  processHit(const NGMHit* tHit);
  void InitCommon();
  void initBlockHist(int hwchan);

  int _maxADC;
  double _minPSD;
  double _maxPSD;
  NGMBlockMapping _bmap;
  std::map<int,TH2*> _blockPSD;//!
  std::map<int,TH2*> _blockPSDvsE;//!
  std::map<int,TH2*> _blockPSDp[5];//!
  std::map<int,TH2*> _blockPSDvsEp[5];//!

 
  ClassDef(NGMBlockPSDMaker,3)
  
};

class NGMBlockPSD : public TNamed
{
public:
  NGMBlockPSD();
  virtual ~NGMBlockPSD(){}
  void SetCalibration(TSpline3* _gMean, TSpline3* _gSig, TSpline3* _nMean, TSpline3* _nSig);
  double GetNSigma(const NGMHit*) const;
  double GetGSigma(const NGMHit*) const;  
  double GetNSigma(double psd,double e) const;
  double GetGSigma(double psd,double e) const;
  TCutG* GetNeutronCut(double emin=0.0, double emax=4000.0, double nsig=2.0, double gammaSigExclusion=5.0, int Npx=1000) const;
  int adcore;
  TSpline3 _gMean;
  TSpline3 _gSig;
  TSpline3 _nMean;
  TSpline3 _nSig;
  
  ClassDef(NGMBlockPSD,2)
};

#endif // __NGMBLOCKPSDMAKER_H__

