//______________________________________________________________________________
//      
// CLASS DECLARATION:  EXOBaselineRemover.hh
//
// DESCRIPTION: 
//
// A class handling the removal of a baseline of waveform.  One can also call
// CalculateBaselineAndRMS and then use the access functions to returns the
// found values.  This class was originally written for MGDO, ported over to
// EXOUtilities by M. Marino
//
// AUTHOR: M. Marino
// CONTACT: 
// FIRST SUBMISSION:  
// 
// REVISION:
// 30 Aug 2010 Added get methods, A. Schubert 
// 31 Aug 2010 Added BaselineRMS, BaselineMean, R. Martin
// 

#include "EXOBaselineRemover.hh"
#include <cmath>

//______________________________________________________________________________
EXOBaselineRemover::EXOBaselineRemover() : 
  EXOVWaveformTransformer("EXOBaselineRemover"), 
  fRegionUnits(kSamples),
  fBaselineSamples(1), 
  fStartSample(0), 
  fBaselineTime(0), 
  fStartTime(0),
  fBaselineRMS(0), 
  fBaselineMean(0)
{ }

//______________________________________________________________________________
void EXOBaselineRemover::TransformInPlace(EXODoubleWaveform& input) const
{
  // Perform the baseline removal.
  //
  // Averages over a certain amount of time and then removes that 
  // value from the rest of the waveform.
  //

  input -= GetBaseline(input);
}

//______________________________________________________________________________
void EXOBaselineRemover::CalculateBaselineAndRMS(const EXODoubleWaveform& waveform) const
{

  size_t startSample = fStartSample;
  size_t nSamplesToAverage = fBaselineSamples;
  if(fRegionUnits == kTime) {
    startSample = (size_t) (fStartTime*waveform.GetSamplingFreq());
    nSamplesToAverage = (size_t) (fBaselineTime*waveform.GetSamplingFreq());
  }
  if ( startSample + nSamplesToAverage > waveform.GetLength() ) { 
    nSamplesToAverage = waveform.GetLength()-startSample; 
  }
  fBaselineMean = 0.0;
  double baselineAverageSquared = 0.0;
  double waveformAti=0.0;
  for ( size_t i=startSample; i<startSample+nSamplesToAverage; i++ ){
    waveformAti=waveform.At(i);
    fBaselineMean += waveformAti; 
    baselineAverageSquared += waveformAti*waveformAti; 
  }
  fBaselineMean /= nSamplesToAverage;
  baselineAverageSquared /= nSamplesToAverage;
  fBaselineRMS = sqrt( baselineAverageSquared - fBaselineMean*fBaselineMean);

}

//______________________________________________________________________________
double EXOBaselineRemover::GetBaseline(const EXODoubleWaveform& waveform) const
{
  // Calls CalculateBaselineAndRMS, returns the baseline of the waveform
  CalculateBaselineAndRMS(waveform);
  return fBaselineMean;
}
 
