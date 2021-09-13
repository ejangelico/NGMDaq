#ifndef __NGMPSDMaker_H__
#define __NGMPSDMaker_H__
//
//  NGMPSDMaker.h
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMModule.h"
#include "NGMHit.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "NGMSpy.h"
#include "TMatrixD.h"
#include "NGMBlockMapping.h"
#include <map>
#include "TSpline.h"

// Forward declarations
class TCanvas;
class TH2;
class TSpline3;
class TCutG;
class NGMHitProcess;

/// \brief  NGMPSDMaker analyzes a stream of hits and
/// produces a block array image for a block array system.

class NGMPSDMaker : public NGMModule
{
public:
    NGMPSDMaker();
    NGMPSDMaker(const char* name, const char* title);
    ~NGMPSDMaker();
    
    bool  init();
    bool  process(const TObject&);
    bool  finish();
    void  LaunchDisplayCanvas(); // *MENU*
    void  ResetHistograms();
    void SetMaxADC(int newVal){_maxADC = newVal;}
    void SetMaxE(double newVal){_maxE = newVal;}
    void SetPSDLimits(double min, double max){ _minPSD = min; _maxPSD = max; }
    void Print(Option_t* option = "") const;
    bool fitNeutronGammaBand(const char* detectorPrefix,double minx = 50.0,double maxx = 4000.0, int nbins = 12, bool ADCorE = true);
    bool CalculateFOM1(TH2* hNScore);
    TSpline3* MakeCubicSpline(const char* name, TH1* hX, TH1* hY);
    void SetPars(int chanSeq, int psdScheme, double keVPerADC);
    void SetPlotPSDE(bool newVal=true){_plotPSDE = newVal;}
    void RecalcFromWaveforms(bool doRecalc = true ) {_doRecalcFromWaveforms = doRecalc;}

private:
    bool processConfiguration(const NGMSystemConfiguration* conf);
    bool  processHit(const NGMHit* tHit);
    void InitCommon();
    void initChanHist(int hwchan);
    
    int _maxADC;
    double _maxE;
    double _minPSD;
    double _maxPSD;
    bool _plotPSDE;
    bool _doRecalcFromWaveforms;
    std::map<int,TH2*> _hitPSD;//!
    std::map<int,TH2*> _hitPSDvsE;//!
    std::map<int,NGMHitProcess*> _hitprocess;//!
    
    ClassDef(NGMPSDMaker,2)
    
};
#endif
