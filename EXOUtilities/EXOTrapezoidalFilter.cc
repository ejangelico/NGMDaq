//______________________________________________________________________________
//                                                             
//      
// CLASS DECLARATION:  EXOTrapezoidalFilter.hh
// 
// DESCRIPTION: 
// 
// A class handling the trapezoidal filter of a waveform.
// Algorithm from V.T Jordanov, G.F. Knoll, NIM A345 (1994) 337-345.
// 
// AUTHOR: M. Marino
// CONTACT: 
// FIRST SUBMISSION: 
// 
// REVISION:
// 30 Aug 2010 Added get methods, A. Schubert 
// 3 Dec 2011 Back-ported to EXOUtilities by M. Marino 
// 
//______________________________________________________________________________

#include "EXOTrapezoidalFilter.hh"

EXOTrapezoidalFilter::EXOTrapezoidalFilter() : 
  EXOVWaveformTransformer("EXOTrapezoidalFilter"), 
  fRampTime(0.),
  fFlatTime(0.),
  fDecayConstant(0.),
  fDoNormalize(true)
{
}

void EXOTrapezoidalFilter::TransformOutOfPlace(const EXODoubleWaveform& anInput, EXODoubleWaveform& anOutput) const
{
  // Calls TemplatedTransform with appropriate template parameters.
  // (Template parameters are handled at compile time, so they allow more aggressive optimization.)

  double decayConstant = fDecayConstant*anInput.GetSamplingFreq();
  const size_t flatStep = static_cast<size_t>(fFlatTime*anInput.GetSamplingFreq());

  if(decayConstant != 0.0) {
    if(flatStep != 0UL) TemplatedTransform<true, true>(anInput, anOutput, decayConstant);
    else TemplatedTransform<true, false>(anInput, anOutput, decayConstant);
  }
  else {
    if(flatStep != 0UL) TemplatedTransform<false, true>(anInput, anOutput);
    else TemplatedTransform<false, false>(anInput, anOutput);
  }
}
