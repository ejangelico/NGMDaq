//______________________________________________________________________________
//      
// CLASS DECLARATION:  EXOExtremumFinder.hh
//
// DESCRIPTION: 
//
// A class finding the extremum of a waveform.  One can also call FindExtremum
// directly.  Access to the extremum is through the access functions
// GetTheExtremumValue() and GetTheExtremumPoint().  Originally in the MGDO
// framework written by M. Marino, ported to EXOUtilities by M. Marino
//
// AUTHOR: M. Marino
// CONTACT: 
// FIRST SUBMISSION: 
// 
// REVISION:
// 
// 

#include "EXOExtremumFinder.hh"
#include "EXOErrorLogger.hh"

//______________________________________________________________________________
EXOExtremumFinder::EXOExtremumFinder() : 
  EXOVWaveformTransformer("EXOExtremumFinder"), 
  fFindMaximum(true),
  fLocMinTime(0.0),
  fLocMaxTime(0.0),
  fTheExtremumValue(0.0),
  fTheExtremumPoint(0)
{}

//______________________________________________________________________________
void EXOExtremumFinder::TransformInPlace(EXODoubleWaveform& anInput) const
{
  if(anInput.GetLength() < 1)
  {
    LogEXOMsg("Waveform has 0 length", EEError);
    return;
  }
  // These are basically the same code, but we're trying to 
  // minimize checking of which type of search this is to save
  // time.
  FindExtremum(anInput);
  
}

//______________________________________________________________________________
void EXOExtremumFinder::FindExtremum(const EXODoubleWaveform& wf) const
{
  // Overloadable function that finds the maximum.  Derived classes can
  // overload this to provide a more sophisticated method to find the extremum
  // value of a waveform. 

  // FIXME: should probably use EXOWaveformRegion.
  size_t locMaxStep = (fLocMaxTime == 0.0) ? wf.GetLength()
                      : (size_t) wf.GetIndexAtTime(fLocMaxTime);
  size_t locMinStep = (size_t) wf.GetIndexAtTime(fLocMinTime);
  
  //set the extremum to be at the first point to be tested
  fTheExtremumPoint = locMinStep;
  fTheExtremumValue = wf.At(locMinStep);
  if (fFindMaximum) {
    for(size_t i=locMinStep;i<locMaxStep;i++) {
      if(wf.At(i) > fTheExtremumValue) {
        fTheExtremumPoint = i;
        fTheExtremumValue = wf.At(i);
      }
    }
  } else {
    for(size_t i=locMinStep;i<locMaxStep;i++) {
      if(wf.At(i) < fTheExtremumValue) {
        fTheExtremumPoint = i;
        fTheExtremumValue = wf.At(i);
      }
    }
  }
}

