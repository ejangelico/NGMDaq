#include "EXOVWaveformTransformer.hh"
#include "EXOErrorLogger.hh"
#include <cassert>

//______________________________________________________________________________
// EXOVWaveformTransformer
// 
// Abstract class handling transformations, processing of waveforms.  It
// handles the different cases for in-place and out-of-place transformations
// efficiently.
//
// To derive from this class:
//
// User must define IsInPlace(). Return true and overload TransformInPlace() if
// the derived class is most effectively coded as an in-place transformation.
// Otherwise return false and overload TransformOutOfPlace(). If a class has
// efficient versions for both in-place and out-of-place transformations,
// overload IsInPlace() and IsOutOfPlace() to both return true, and implement
// the versions in the respective TransformIn/OutOfPlace() function.  For the
// latter, the most efficient is to implement a function that contains the same
// algorithm and have both TransformInPlace and TransformOutOfPlace call this
// function. 

void EXOVWaveformTransformer::Transform(EXODoubleWaveform* input, EXODoubleWaveform* output) const 
{
  // TransformWaveform must be defined by derived classes.  This function takes
  // an input waveform and performs some type of transformation on it.  The output is set
  // to NULL by default meaning the result of the transformation is stored in the input
  // waveform.  Otherwise the transformation is stored in the output waveform.


  // The user requests an in-place or out-of-place transform by either
  // supplying a NULL output (for in-place) or a valid output pointer (for
  // out-of-place). We want to call the appropriate function in the derived
  // class that performs the transformation in the optimal way. Do this in
  // the base class so that the derived classes don't need to.

  if(input == NULL) {
    LogEXOMsg("input is NULL.", EEError);
    return;
  }

  // If an output waveform was given, then clearly the user wants an out-of-place transform.
  // In case the appropriate function is not defined in the derived class, the base versions
  // will do the right thing.
  if(output == NULL) TransformInPlace(*input);
  else {
    output->MakeSimilarTo(*input);
    TransformOutOfPlace(*input, *output);
  }
}

void EXOVWaveformTransformer::TransformInPlace(EXODoubleWaveform& input) const
{
  // This function gets called if a user requests an in-place transform but the derived transform is out-of-place.
  // Use fTmpWaveform to do the transform, then copy back into input.

  // If IsInPlace is true, then the derived class has a mistake -- we should never have gotten to this point.
  assert(not IsInPlace());
  // If IsOutOfPlace is false, then there's no transform in the derived class at all.
  assert(IsOutOfPlace());

  fTmpWaveform.MakeSimilarTo(input);
  TransformOutOfPlace(input, fTmpWaveform);
  input = fTmpWaveform;
}

void EXOVWaveformTransformer::TransformOutOfPlace(const EXODoubleWaveform& input, EXODoubleWaveform& output) const
{
  // This function gets called if a user requests an out-of-place transform but the derived transform is in-place.
  // Do a copy into output, then transform that.

  // If IsOutOfPlace is true, then the derived class has a mistake -- we should never have gotten to this point.
  assert(not IsOutOfPlace());
  // If IsInPlace is false, then there's no transform in the derived class at all.
  assert(IsInPlace());

  output = input;
  TransformInPlace(output);
}
