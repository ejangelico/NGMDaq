#ifndef __NGMHitProcess_H__
#define __NGMHitProcess_H__
//
//  NGMHitProcess.h
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//
#include <vector>
#include <map>
#include <string>
#include "TObject.h"
#include <vector>
#include "TMatrixD.h"
#include "NGMHit.h"
class NGMSystemConfiguration; // forward declaration
class NGMHitPSD; //forward declaration

class NGMHitProcess : public TObject
{
  
public:
    NGMHitProcess();
    virtual ~NGMHitProcess(){}
    bool Init(const NGMSystemConfiguration* conf, int chanseq);
    bool ProcessHit( const NGMHit* hit) ;
    bool ProcessHit( NGMHit* hit);
    double GetPulseHeight() const {return _pulseHeight;}
    double GetPSD() const {return _psd;}
    double GetBaseline() const {return _baseline;}
    virtual void        Print(Option_t *option="") const;
    double GetEnergy() const {return _energy;}
    double GetNeutronId() const {return _neutronSigma;}
    double GetGammaId() const {return _gammaSigma;}
    double GetPSDVar() const {return _psdvar;}
    int GetChanSeq() const {return _chanSeq;}
    void SetBaseline(double nomBaseline, double nomBaselineRMS, double rctau);
    void SetPileupCorrection(int doPileup);
    void SetPSDScheme(int psdScheme){_psdScheme=psdScheme;}
    void SetkeVPerADC(double keVPerADC){_pixelkeVADC=keVPerADC;}
    void RecalcFromWaveforms(bool doRecalc = true ) {_doRecalcFromWaveforms = doRecalc;}
private:
    
    int _chanSeq;
    int _psdScheme;
    std::vector<int> _gatewidth;
    std::vector<int> _gatestart;
    int _waveformstart;
    int _waveformlength;
    double _pixelkeVADC;
    const NGMHitPSD *_hitpsd;
    int _doPileupCorrection;
    double _nominalBaseline;
    double _nominalBaselineRMS;
    double _rctau;
    double _nsPerClock;
    bool _doRecalcFromWaveforms;
    
    // cached values not to be saved
    double _baseline; //!
    double _psd; //!
    double _psdvar; //!
    double _pulseHeight; //!
    double _prompt; //!
    double _energy; //!
    double _neutronSigma; //!
    double _gammaSigma; //!
    NGMTimeStamp _ts; //!
    
  ClassDef(NGMHitProcess,3)

};
#endif //__NGMHitProcess_H__
