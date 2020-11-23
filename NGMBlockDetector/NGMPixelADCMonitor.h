#ifndef __NGMPixelADCMonitor_H__
#define __NGMPixelADCMonitor_H__
//
//  NGMPixelADCMonitor.h
//  NGMDaq
//
//  Created by Newby, Robert Jason on 7/8/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMModule.h"
#include "NGMHit.h"
#include "TObjArray.h"
#include "NGMSpy.h"
#include "TMatrixD.h"

// Forward declarations
class TTimer;
class TCanvas;
/// \brief  NGMPixelADCMonitor analyzes a stream of hits and 
/// produces a block array image for a block array system.

class NGMPixelADCMonitor : public NGMModule
{
public:
  NGMPixelADCMonitor();
  NGMPixelADCMonitor(const char* name, const char* title);
  ~NGMPixelADCMonitor();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  void  LaunchDisplayCanvas(); // *MENU*
  void  ResetHistograms();
  void SetMaxADC(int newVal){maxADC = newVal;}
  void SetEnergyGate(int newVal){energyGate = newVal;}
  void Print(Option_t* option = "") const;
  static double FindComptonEdge(TH1* hADC, int filterScale = 20, double width = 20.0, double dynamicrange = 0.5);
  void SaveComptonCalibration(NGMSystemConfiguration* conf, const char* fname, double energy = 477.0, int filterScale = 20);
  void SetComptonEdgeParameters(double width = 20.0, double dynamicrange = 0.5)
  {
    _tspectrumwidth = width;
    _tspectrumpeakdynamicrange = dynamicrange;
  }
private:
  void InitCommon();
  enum localconsts { ngates = 8, maxChannels = 5000};  
  bool  analyzeHit(const NGMHit* tHit);
  int blocknrows;
  int blockncols;
  int maxADC;
  int energyGate;
  int _gatewidth[maxChannels][ngates];
  TObjArray* _blockArray;
  TMatrixD _blockRowCol;
  TH2* _hEnergyvsPixel;
  TH2* _hPulseHeightvsPixel;
  double _tspectrumwidth;
  double _tspectrumpeakdynamicrange;

  ClassDef(NGMPixelADCMonitor,4)
  
};


class NGMPixelADCMonitorDisplay : public TObject
{
  TCanvas* cBlockDisplay;
  NGMSpy spy;
  TString _moduleName;
  int _pixel;
  
public:
  NGMPixelADCMonitorDisplay();
  NGMPixelADCMonitorDisplay(const char* moduleName);
  void SetPixel(int newVal){_pixel = newVal;}
  virtual Bool_t HandleTimer(TTimer* timer);
  
  ClassDef(NGMPixelADCMonitorDisplay,0)
};


#endif //__NGMPixelADCMonitor_H__
