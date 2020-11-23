#ifndef _EXOBaselineRemover_HH
#define _EXOBaselineRemover_HH

#include "EXOVWaveformTransformer.hh"
#include <cstddef> //for size_t

class EXOBaselineRemover : public EXOVWaveformTransformer 
{
  public:
    enum ERegionUnits { kSamples, kTime };
    EXOBaselineRemover(); 
 
    virtual bool IsInPlace() const { return true; }
    virtual void CalculateBaselineAndRMS(const EXODoubleWaveform& waveform) const;

    virtual double GetBaseline(const EXODoubleWaveform& waveform) const;
    
    //! Sets the region over which the baseline is averaged.  
    virtual void SetBaselineSamples( size_t nSamples ) 
      { fRegionUnits = kSamples; fBaselineSamples = nSamples; }
    virtual void SetStartSample( size_t iSample ) 
      { fRegionUnits = kSamples; fStartSample = iSample; }
    virtual void SetBaselineTime( double aTime ) 
      { fRegionUnits = kTime; fBaselineTime = aTime; }
    virtual void SetStartTime( double aTime ) 
      { fRegionUnits = kTime; fStartTime = aTime; }

    virtual inline size_t GetBaselineSamples() const { return fBaselineSamples; }
    virtual inline size_t GetStartSample() const { return fStartSample; }
    virtual inline double GetBaselineTime() const { return fBaselineTime; }
    virtual inline double GetStartTime() const { return fStartTime; }
    virtual inline double GetBaselineRMS(const EXODoubleWaveform& waveform) const 
      { CalculateBaselineAndRMS(waveform); return fBaselineRMS; }
    virtual inline double GetBaselineMean(const EXODoubleWaveform& waveform) const 
      { return GetBaseline(waveform); }

    virtual inline double GetBaselineRMS() const { return fBaselineRMS; }
    virtual inline double GetBaselineMean() const { return fBaselineMean; }

  protected:
    virtual void TransformInPlace(EXODoubleWaveform& anInput) const;
    ERegionUnits fRegionUnits;
    size_t fBaselineSamples;
    size_t fStartSample;
    double fBaselineTime;
    double fStartTime;
    mutable double fBaselineRMS;
    mutable double fBaselineMean;
};

#endif
