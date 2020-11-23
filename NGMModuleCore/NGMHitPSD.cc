#include "NGMHitPSD.h"
#include "NGMHit.h"
#include "TCutG.h"

ClassImp(NGMHitPSD)

NGMHitPSD::NGMHitPSD()
{
    adcore=0;
}
void NGMHitPSD::SetCalibration(TSpline3* gMean, TSpline3* gSig, TSpline3* nMean, TSpline3* nSig)
{
    _gMean = *gMean;
    _gSig = *gSig;
    _nMean = *nMean;
    _nSig = *nSig;
}

double NGMHitPSD::GetNSigma(double psd,double e) const
{
    return (psd - _nMean.Eval(e))/_nSig.Eval(e) ;
}

double NGMHitPSD::GetGSigma(double psd,double e) const
{
    return (psd - _gMean.Eval(e))/_gSig.Eval(e) ;
}


double NGMHitPSD::GetNSigma(const NGMHit* hit) const
{
    if(adcore==0)
        return (hit->GetPSD() - _nMean.Eval(hit->GetPulseHeight()))/_nSig.Eval(hit->GetPulseHeight()) ;
    else
        return (hit->GetPSD() - _nMean.Eval(hit->GetEnergy()))/_nSig.Eval(hit->GetEnergy()) ;
    
}

double NGMHitPSD::GetGSigma(const NGMHit* hit) const
{
    if(adcore==0)
        return (hit->GetPSD() - _gMean.Eval(hit->GetPulseHeight()))/_gSig.Eval(hit->GetPulseHeight()) ;
    else
        return (hit->GetPSD() - _gMean.Eval(hit->GetEnergy()))/_gSig.Eval(hit->GetEnergy()) ;
}

TCutG* NGMHitPSD::GetNeutronCut(double emin, double emax, double nsig, double gammaSigExclusion, int Npx) const
{
    double dE = (emax-emin)/Npx;
    
    //Find First xbin where gamma exclusion is not less then nBotCut
    int iE = 0;
    for(iE = 0; iE<Npx+1;iE++)
    {
        double eval = iE*dE+emin;
        double gExcl = _gMean.Eval(eval) -gammaSigExclusion*_gSig.Eval(eval);
        double nTop = _nMean.Eval(eval) +nsig*_nSig.Eval(eval);
        double nBot= _nMean.Eval(eval) -nsig*_nSig.Eval(eval);
        if(gExcl>nBot) break;
    }
    int firstiE = iE;
    TString cutName(GetName());
    cutName+="_CutG";
    cutName.ToLower();
    TCutG* gr = new TCutG(cutName.Data(),(Npx-firstiE+1)*2+1);
    
    int ipoint = 0;
    for(iE = firstiE; iE<Npx+1;iE++)
    {
        double eval = iE*dE+emin;
        double gExcl = _gMean.Eval(eval) -gammaSigExclusion*_gSig.Eval(eval);
        double nTop = _nMean.Eval(eval) +nsig*_nSig.Eval(eval);
        
        gr->SetPoint(ipoint++,eval,TMath::Min(gExcl,nTop));
    }
    for(iE = Npx; iE>=firstiE; iE--)
    {
        double eval = iE*dE+emin;
        double gExcl = _gMean.Eval(eval) -gammaSigExclusion*_gSig.Eval(eval);
        double nBot= _nMean.Eval(eval) -nsig*_nSig.Eval(eval);
        
        gr->SetPoint(ipoint++,eval,TMath::Min(gExcl,nBot));
    }
    gr->SetPoint(ipoint++,gr->GetX()[0],gr->GetY()[0]);
    
    return gr;
}
