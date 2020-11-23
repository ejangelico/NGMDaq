#ifndef _EXORisetimeCalculation_HH
#define _EXORisetimeCalculation_HH

#include "EXOVWaveformTransformer.hh"
#include <cstddef> //for size_t

class EXORisetimeCalculation : public EXOVWaveformTransformer 
{
  public:
    EXORisetimeCalculation(); 
    virtual ~EXORisetimeCalculation();
    
    /*! 
       Perform test.  Transform leaves anInput unchanged.  
       */
    virtual bool IsInPlace() const { return true; }

    void SetPulsePeakHeight(double maximum) { fPeakHeight = maximum; }
    double GetPulsePeakHeight() const { return fPeakHeight; }

    //! Sets the position to begin scanning forward on the waveform for the mid point of the rising edge 
    void SetScanFrom( size_t scanfrom ) { fScanFrom = scanfrom; }
    size_t GetScanFrom() const { return fScanFrom; }

    //! Sets the initial threshold percentage (should be between 0, 1)
    void SetInitialThresholdPercentage( double threshold ) { fInitThreshold = threshold; }
    double GetInitialThresholdPercentage() const { return fInitThreshold; }

    //! Sets the final threshold percentage (should be between 0, 1)
    void SetFinalThresholdPercentage( double threshold ) { fFinalThreshold = threshold; }
    double GetFinalThresholdPercentage() const { return fFinalThreshold; }

    //! Sets the inital scanning percentage to find the middle of the pulse (should be between 0, 1)
    void SetInitialScanToPercentage( double threshold ) { fInitialScanToPercentage = threshold; }
    double GetInitialScanToPercentage() const { return fInitialScanToPercentage; }

    //! Returns risetime with CLHEP units of time
    double GetRiseTime() const { return fRiseTime; }

    //! Returns point of the initial threshold crossing in the waveform.
    size_t GetInitialThresholdCrossing() const { return fInitThresholdCrossing; }

    //! Returns point of the final threshold crossing in the waveform.
    size_t GetFinalThresholdCrossing() const { return fFinalThresholdCrossing; }

    //! Returns the initial threshold crossing estimate, based upon linear interpolation 
    double GetInitialThresholdCrossingEstimate() const { return fInitThresholdEstimate; }

    //! Returns the final threshold crossing estimate, based upon linear interpolation 
    double GetFinalThresholdCrossingEstimate() const { return fFinalThresholdEstimate; }


  protected:
    virtual void TransformInPlace(EXODoubleWaveform& anInput) const;
    double fInitThreshold;
    double fFinalThreshold;
    double fInitialScanToPercentage;
    double fPeakHeight;
    size_t fScanFrom;

    mutable double fRiseTime;
    mutable double fInitThresholdEstimate;
    mutable double fFinalThresholdEstimate;
    mutable size_t fInitThresholdCrossing;
    mutable size_t fFinalThresholdCrossing;

  };

#endif
