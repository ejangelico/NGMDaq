#ifndef _EXOExtremumFinder_HH
#define _EXOExtremumFinder_HH

#include "EXOVWaveformTransformer.hh"
#include <cstddef> //for size_t

class EXOExtremumFinder : public EXOVWaveformTransformer 
{
public:
  EXOExtremumFinder(); 
  
  virtual inline bool IsInPlace() const { return true; }
  
  /// Set to true to find the maximum of a waveform. Default is true. 
  virtual void SetFindMaximum(bool findMax = true) { fFindMaximum = findMax; }
  virtual void SetFindMinimum(bool findMin = true) { fFindMaximum = !findMin; }
  
  /// Sets the local minimum for the search 
  virtual void SetLocalMinimumTime( double locMinTime ) { fLocMinTime = locMinTime;}
  
  /// Sets the local maximum for the search 
  virtual void SetLocalMaximumTime( double locMaxTime ) { fLocMaxTime = locMaxTime;} 
  
  /// Returns the location of the extremum point found by the last Transform call. 
  virtual size_t GetTheExtremumPoint() const { return fTheExtremumPoint; }
  
  /// Returns the value of the extremum point found by the last Transform call. 
  virtual double GetTheExtremumValue() const { return fTheExtremumValue; }
  
protected:
  virtual void TransformInPlace(EXODoubleWaveform& anInput) const;
  bool fFindMaximum;
  double fLocMinTime;
  double fLocMaxTime;
  mutable double fTheExtremumValue;
  mutable size_t fTheExtremumPoint;
  
  virtual void FindExtremum(const EXODoubleWaveform&) const;
  
};

#endif
