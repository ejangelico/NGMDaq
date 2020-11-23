//______________________________________________________________________________
//      
// CLASS DECLARATION:  EXOPoleZeroCorrection.hh
//
// DESCRIPTION: 
//
// A class handling the pole zero correction of a waveform.
//
// AUTHOR: M. Marino
// CONTACT: 
// FIRST SUBMISSION: 
// 
// REVISION:
// 30 Aug 2010 Added get method for fDecayConstant, A. Schubert 
// 13 Nov 2011 Added functionality to specify the resting voltage (fRestingBaselineValue), P. Finnerty
// 3 Dec 2011 Back-ported to EXOUtilities by M. Marino 
//
//______________________________________________________________________________

#include "EXOPoleZeroCorrection.hh"

EXOPoleZeroCorrection::EXOPoleZeroCorrection() : 
  EXOVWaveformTransformer("EXOPoleZeroCorrection"),
  fDecayConstant(1.),
  fRestingBaselineValue(0.0) 
{
}

void EXOPoleZeroCorrection::TransformOutOfPlace(const EXODoubleWaveform& anInput, EXODoubleWaveform& anOutput) const 
{
  //  
  //  This transformation cannot be done in place.  The following algorithm is
  //  used:

  //  y[n] = y[n-1] + (x[n] - baseline) - exp(-1/(tau*sf))*(x[n-1] - baseline)
  //  
  //  where sf is the sampling frequency of the input waveform, baseline is the
  //  resting voltage of the waveform, tau is the decay time constant.
  //  

  if(anInput.GetLength() < 1) return;

  anOutput.MakeSimilarTo(anInput);//added by Ryan Martin-07/30/2010
  /*
  double decayConstant = fDecayConstant*anInput.GetSamplingFrequency();  
  anOutput[0] = (decayConstant+1.)*anInput.At(0);
  for(size_t i=1;i<anInput.GetLength();i++)
  {
    anOutput[i] = anOutput[i-1] + decayConstant*(anInput.At(i)-anInput.At(i-1)) + anInput.At(i);
  }*/
  
  //added by Ryan Martin-07/30/2010:
  //modified by Paddy Finnerty-11/13/2011 (subtract fRestingBaselineValue)
  double constant = exp(-1./(fDecayConstant*anInput.GetSamplingFreq()));
  anOutput[0] = anInput.At(0);
  for(size_t i=1;i<anInput.GetLength();i++)
  {
    anOutput[i] = anOutput[i-1] - 
                  constant*(anInput.At(i-1) - fRestingBaselineValue ) + 
                  (anInput.At(i)- fRestingBaselineValue);
  }

}

EXOPoleZeroCorrection::~EXOPoleZeroCorrection()
{
}
