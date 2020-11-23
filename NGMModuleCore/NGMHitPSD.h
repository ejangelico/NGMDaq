#ifndef __NGMHITPSD_H__
#define __NGMHITPSD_H__

#include "TSpline.h"

class TCutG;// forward declaration
class NGMHit; //forward declaration

class NGMHitPSD : public TNamed
{
public:
    NGMHitPSD();
    virtual ~NGMHitPSD(){}
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
    
    ClassDef(NGMHitPSD,1)
};
#endif