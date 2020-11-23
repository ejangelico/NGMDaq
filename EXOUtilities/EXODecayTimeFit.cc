//______________________________________________________________________________
//      
// CLASS DECLARATION:  EXODecayTimeFit.hh
//
// DESCRIPTION: 
//
// A class finding the decay time of a waveform.  Fit the last few samples of
// the waveform and return the computed decay time.  
//
// AUTHOR: M. Jewell
// CONTACT: 
// FIRST SUBMISSION: 
// 
// REVISION:
// v1: 8-24-2016 Create first version
// 


#include "EXODecayTimeFit.hh"
#include "EXOErrorLogger.hh"
#include "SystemOfUnits.hh"
#include "TH1D.h"
#include "TF1.h"
#include "TCanvas.h"

//______________________________________________________________________________
EXODecayTimeFit::EXODecayTimeFit(): 
  EXOVWaveformTransformer("EXODecayTimeFit"), 
  fStartSample(0), 
  fEndSample(0),
  fMaxValGuess(0.0),
  fTauGuess(400.0),
  fDecayTime(0.0)
{
}

//______________________________________________________________________________
void EXODecayTimeFit::TransformInPlace(EXODoubleWaveform& anInput) const
{
  if(anInput.GetLength() < 2)
  {
    LogEXOMsg("Waveform is too short", EEError);
    return;
  }
  if(anInput.GetSamplingPeriod() <= 0.0) {
    LogEXOMsg("Waveform has invalid sampling period", EEAlert);
    return;
  }

  TH1D* wfhist = anInput.GimmeHist();
  TF1 *exp_decay = new TF1("exp_decay", "[0]*exp(-x/[1])", fStartSample, fEndSample);
  
  //double fSampleGuess = fTauGuess/(anInput.GetSamplingPeriod()/CLHEP::microsecond);

  exp_decay->SetParameters(fMaxValGuess, fTauGuess);
  
  TFitResultPtr fit_result = wfhist->Fit("exp_decay", "QWBRS");

  //TCanvas* ctest = new TCanvas("ctest");
  //wfhist->Draw();
  //ctest->Update();

  fDecayTime = exp_decay->GetParameter(1); 
  fDecayTimeChi2 = exp_decay->GetChisquare()/exp_decay->GetNDF();
  fDecayTimeError = exp_decay->GetParError(1);

  //std::cout << "Decay Time = " << fDecayTime << " +/- " << fDecayTimeError << " chi2 = " << fDecayTimeChi2 << " ndf " << exp_decay->GetNDF() << " true chi2 " << exp_decay->GetChisquare() << std::endl;
  
  //int pause;
  //std::cout << "Pause" << std::endl;
  //std::cin >> pause;


}

//______________________________________________________________________________
EXODecayTimeFit::~EXODecayTimeFit()
{
}

