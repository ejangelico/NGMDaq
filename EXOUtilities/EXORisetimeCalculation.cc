//______________________________________________________________________________
//      
// CLASS DECLARATION:  EXORisetimeCalculation.hh
//
// DESCRIPTION: 
//
// A class finding the risetime of a waveform.  Before this transform is called
// the baseline of the pulse should be removed and the maximum value of the
// waveform should be set using SetPulsePeakHeight.  This is to allow the user
// to use different methods to find the maximum and maximize usefulness of this
// class.  This algorithm always assumes the the pulse is rising/falling left
// to right.  To use this class, the baseline should first be removed from the
// waveform.  The initial threshold should be set using
// SetInitialThresholdPercentage and final SetFinalThresholdPercentage.  The
// pulse peak height should be set by the user using SetPulsePeakHeight.  It
// can be found using the EXOExtremumFinder transformation.  Originally written
// for the MGDO software package, ported to EXOUtilities by M. Marino
//
// AUTHOR: M. Marino
// CONTACT: 
// FIRST SUBMISSION: 
// 
// REVISION:
// 30 Aug 2010 - Added get methods, A. Schubert
// 8  Jun 2011 - added more get methods, P. Finnerty
// 


#include "EXORisetimeCalculation.hh"
#include "EXOErrorLogger.hh"

//______________________________________________________________________________
EXORisetimeCalculation::EXORisetimeCalculation() : 
  EXOVWaveformTransformer("EXORisetimeCalculation"), 
  fInitThreshold(0.0),
  fFinalThreshold(0.0), 
  fInitialScanToPercentage(0.5), 
  fPeakHeight(0), 
  fScanFrom(0), 
  fRiseTime(0.0), 
  fInitThresholdEstimate(0.0), 
  fFinalThresholdEstimate(0.0), 
  fInitThresholdCrossing(0), 
  fFinalThresholdCrossing(0)
{
}

//______________________________________________________________________________
void EXORisetimeCalculation::TransformInPlace(EXODoubleWaveform& anInput) const
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
  if(fInitThreshold >= fInitialScanToPercentage or fInitialScanToPercentage >= fFinalThreshold) {
    LogEXOMsg("Initial thresholds set incorrectly in EXORisetimeCalculation", EEAlert);
    return;
  }
  if(fPeakHeight == 0) {
    LogEXOMsg("Peak height set to zero", EEAlert);
    return;
  }

  fInitThresholdCrossing = 0;
  fFinalThresholdCrossing = anInput.GetLength();
  size_t middleOfPulse = 0;

  /* First find the middle of the pulse. */
  size_t i = (fScanFrom >= anInput.GetLength()) ? anInput.GetLength()-1 : fScanFrom;

  if (fPeakHeight < 0) {
      while (i < anInput.GetLength()) {
        if ( anInput.At(i) <= fInitialScanToPercentage*fPeakHeight ) {
          middleOfPulse = i;
          break;
        }  
        i++;
      }
  } else {
     while (i < anInput.GetLength()) {
        if ( anInput.At(i) >= fInitialScanToPercentage*fPeakHeight ) {
          middleOfPulse = i;
          break;
        }  
        i++;
      }
  }
  // Check for the various ways this can fail
  if(middleOfPulse == 0) {
    // We never did find a point that exceeded the scanto threshold.
    // We could try scanning over the whole waveform, but what if there was a specific signal the user tried to select?
    // Just exit.
    LogEXOMsg("Failed to find midpoint of signal", EEError);
    return;
  }
  if(fPeakHeight < 0 ? anInput[middleOfPulse-1] <= fInitialScanToPercentage*fPeakHeight :
                       anInput[middleOfPulse-1] >= fInitialScanToPercentage*fPeakHeight) {
    // middleOfPulse is not actually the first datapoint to exceed threshold.  Try scanning backward.
    while(middleOfPulse > 0) {
      if(fPeakHeight < 0 ? anInput[middleOfPulse-1] > fInitialScanToPercentage*fPeakHeight :
                           anInput[middleOfPulse-1] < fInitialScanToPercentage*fPeakHeight) {
        break;
      }
      middleOfPulse--;
    }
    if(middleOfPulse == 0) {
      // Well, we scanned all the way back to the beginning, and never fell below the threshold.  Give up.
      LogEXOMsg("Midpoint of signal occurs before waveform begins", EEError);
      return;
    }
  }
  // OK, we can guarantee that 0 < middleOfPulse < anInput.GetLength(),
  // anInput[middleOfPulse-1] is below threshold and anInput[middleOfPulse] meets or exceeds threshold.
  // We have also guaranteed (by an earlier check) that the thresholds are strictly ordered.

  /* Then find beginning of the pulse. */
  /* It occurs somewhere strictly before middleOfPulse */
  /* If we fail to find it, exit with an error */
  i = middleOfPulse;
  if (fPeakHeight < 0) {
      while (i > 0) {
        if ( anInput.At(i-1) >= fInitThreshold*fPeakHeight ) {
          fInitThresholdCrossing = i-1;
          break;
        }  
        i--;
        if(i == 0) {
          LogEXOMsg("Failed to find initial threshold crossing", EEError);
          return;
        }
      }
  } else {
      while (i > 0) {
        if ( anInput.At(i-1) <= fInitThreshold*fPeakHeight ) {
          fInitThresholdCrossing = i-1;
          break;
        }  
        i--;
        if(i == 0) {
          LogEXOMsg("Failed to find initial threshold crossing", EEError);
          return;
        }
      }
  }
  // OK, if we've reached this point at all, then we can guarantee fInitThreshold was set to the first point that meets or falls below threshold.
  // 0 <= fInitThresholdCrossing < middleOfPulse < anInput.GetLength()
  // and for fPeakHeight > 0:
  // anInput[fInitThresholdCrossing] <= fInitThreshold*fPeakHeight < fInitialScanToPercentage*fPeakHeight <= anInput[middleOfPulse]

  i = middleOfPulse;
  if (fPeakHeight < 0) {
      while (i < anInput.GetLength()) {
        if ( anInput.At(i) <= fFinalThreshold*fPeakHeight ) {
          fFinalThresholdCrossing = i;
          break;
        }  
        i++;
      }
  } else {
      while (i < anInput.GetLength()) {
        if ( anInput.At(i) >= fFinalThreshold*fPeakHeight ) {
          fFinalThresholdCrossing = i;
          break;
        }  
        i++;
      }
  }
  if(fFinalThresholdCrossing == anInput.GetLength()) {
    // We never did find a point where the waveform met or exceeded fFinalThreshold*fPeakHeight.
    // We can't go back -- this is clearly a signal here -- so fail.
    LogEXOMsg("Signal failed to exceed final crossing threshold", EEError);
    return;
  }

  // Now linearly interpolate to grab a more exact time
  // y = first*x
  // Note first and second are not zero.  We've already checked that GetSamplingPeriod() doesn't return 0.
  double first = -(anInput[fInitThresholdCrossing] - anInput[fInitThresholdCrossing+1])/anInput.GetSamplingPeriod();
  double second = (anInput[fFinalThresholdCrossing] - anInput[fFinalThresholdCrossing-1])/anInput.GetSamplingPeriod();
  fFinalThresholdEstimate = (fFinalThresholdCrossing - 1)*anInput.GetSamplingPeriod() + 
                            (fFinalThreshold*fPeakHeight - anInput[fFinalThresholdCrossing-1])/second;
  fInitThresholdEstimate = fInitThresholdCrossing*anInput.GetSamplingPeriod() + 
                           (fInitThreshold*fPeakHeight - anInput[fInitThresholdCrossing])/first;

  fRiseTime = fFinalThresholdEstimate - fInitThresholdEstimate; 

}

//______________________________________________________________________________
EXORisetimeCalculation::~EXORisetimeCalculation()
{
}
