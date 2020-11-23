#ifndef _EXOTrapezoidalFilter_HH
#define _EXOTrapezoidalFilter_HH

#include "EXOVWaveformTransformer.hh"
#include <vector>

class EXOTrapezoidalFilter : public EXOVWaveformTransformer 
{
  public:
    EXOTrapezoidalFilter(); 
    
    virtual inline bool IsInPlace() const { return false; }

    /// Set the ramp time of the filter.
    virtual inline void SetRampTime(double aVal) { fRampTime = aVal; }

    /// Set the flat time of the filter.
    virtual inline void SetFlatTime(double aVal) { fFlatTime = aVal; }

    /// Set the decay constant (tau) for the pole-zero correction (=0 by default for no correction (tau -> infinity))
    virtual void SetDecayConstant(double aVal) { fDecayConstant = aVal; }

    /// Put trap filter output in same units as input wf (set to false for marginally improved performance)
    virtual void SetDoNormalize(bool doNormalize = true) { fDoNormalize = doNormalize; }

    virtual inline double GetRampTime() const { return fRampTime; }
    virtual inline double GetFlatTime() const { return fFlatTime; }
    virtual inline double GetDecayConstant() const { return fDecayConstant; }
    virtual inline bool   GetDoNormalize() const { return fDoNormalize; }

  protected:
    virtual void TransformOutOfPlace(const EXODoubleWaveform& anInput, EXODoubleWaveform& anOutput) const;
    double fRampTime;      ///< duration of rising edge of trapezoid (CLHEP time units)
    double fFlatTime;      ///< duration of flat top of trapezoid (CLHEP time units)
    double fDecayConstant; ///< decay constant for pole-zero correction (CLHEP time units)
    bool fDoNormalize;     ///< flag to set whether trapezoid will be normalized to the input wf y-scale

  private:
    template<bool WithDecayConstant, bool WithFlatStep>
    void TemplatedTransform(const EXODoubleWaveform& anInput, EXODoubleWaveform& anOutput,
                            double decayConstant = 0.0) const;
};

template<bool WithDecayConstant, bool WithFlatStep>
void EXOTrapezoidalFilter::TemplatedTransform(const EXODoubleWaveform& anInput, EXODoubleWaveform& anOutput, double decayConstant /*= 0.0*/) const
{
    //! Perform the transformation.  This cannot be done in place. 
    /*!
        The trapezoidal filter performs a pole-zero correction for an
        RC circuit and averages values ( using ramp-time ). 
        Baseline removal should be performed before initiating
        this transformation.

        This transformation multiplies the output waveform by 
        decayTime*rampTime*sf*sf

        where sf is the sampling frequency of the input waveform.
     */

  if(anInput.GetLength() <= 1) return;

  anOutput.MakeSimilarTo(anInput);
  
  const size_t rampStep = static_cast<size_t>(fRampTime*anInput.GetSamplingFreq());
  const size_t flatStep = (WithFlatStep ? static_cast<size_t>(fFlatTime*anInput.GetSamplingFreq()) : 0UL);
  double movingAvg = anInput.At(0); // Tracks p(n), in the notation of Jordanov and Knoll.
  
  if(WithDecayConstant) anOutput[0] = (decayConstant+1.)*anInput.At(0);
  else anOutput[0] = anInput.At(0);

  // We must check bounds -- early entries don't have full memory yet.

  for(size_t i = 1UL;
      i < std::min(anInput.GetLength(), rampStep);
      i++) {
    double scratch = anInput[i];
    if(WithDecayConstant) {
      movingAvg += scratch;
      anOutput[i] = anOutput[i-1] + movingAvg + decayConstant*scratch;
    }
    else anOutput[i] = anOutput[i-1] + scratch;
  }

  for(size_t i = std::max(size_t(1), rampStep);
      i < std::min(anInput.GetLength(), flatStep+rampStep);
      i++) {
    double scratch = anInput[i] - anInput[i-rampStep];
    if(WithDecayConstant) {
      movingAvg += scratch;
      anOutput[i] = anOutput[i-1] + movingAvg + decayConstant*scratch;
    }
    else anOutput[i] = anOutput[i-1] + scratch;
  }

  for(size_t i = std::max(size_t(1), flatStep+rampStep);
      i < std::min(anInput.GetLength(), flatStep+2*rampStep);
      i++) {
    double scratch = anInput[i] - anInput[i-rampStep] - anInput[i-flatStep-rampStep];
    if(WithDecayConstant) {
      movingAvg += scratch;
      anOutput[i] = anOutput[i-1] + movingAvg + decayConstant*scratch;
    }
    else anOutput[i] = anOutput[i-1] + scratch;
  }

  for(size_t i = std::max(size_t(1), flatStep+2*rampStep);
      i < anInput.GetLength();
      i++) {
    double scratch = anInput[i] - anInput[i-rampStep] - anInput[i-flatStep-rampStep] + anInput[i-flatStep-2*rampStep];
    if(WithDecayConstant) {
      movingAvg += scratch;
      anOutput[i] = anOutput[i-1] + movingAvg + decayConstant*scratch;
    } 
    else anOutput[i] = anOutput[i-1] + scratch;
  }

  if(fDoNormalize) {
    double norm = rampStep;
    if(WithDecayConstant) norm *= decayConstant;
    anOutput /= norm;
  }
}

#endif
