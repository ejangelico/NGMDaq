#ifndef EXOMatchedFilter_hh
#define EXOMatchedFilter_hh

#include "EXOVWaveformTransformer.hh"
#include "EXOWaveformFT.hh"
#include <cstddef> //for size_t

class EXOFastFourierTransformFFTW;

class EXOMatchedFilter : public EXOVWaveformTransformer
{

  public:
    EXOMatchedFilter();
    ~EXOMatchedFilter();
 
    // This transform is optimized for an in-place transformation
    virtual bool IsInPlace() const { return true; }
    void SetTemplateToMatch(const EXODoubleWaveform& wf,
                            size_t nsample,
                            Int_t offset);
    void SetNoisePowerSqrMag(const EXODoubleWaveform& wf = EXODoubleWaveform() )
      { fNoisePowerSpectrum = wf; }

    template <typename _Tp>
    bool WaveformMatchesFilterSettings(const EXOTemplWaveform<_Tp>& wf) const;

    // Reset the matched filter
    void Reset() { fMatchedFilter.SetLength(0); fOffset = 0; }

    // If we're trying to do many matched filters simultaneously, each needs its own array;
    // set this flag to use the new-array fftw interface.
    // Note that if you're using ROOT's fft library, this may fail.
    static bool fUseNewArrayInterface;
               
  protected: 
    virtual void TransformInPlace(EXODoubleWaveform& anInput) const;
    EXOWaveformFT fMatchedFilter;
    EXODoubleWaveform fNoisePowerSpectrum;
    EXOFastFourierTransformFFTW* fFFT;
    Int_t fOffset;
    mutable void* fInternalArray; //! Allocated with fftw_malloc to guarantee alignment.
    mutable size_t fInternalArrayLength;
};

template<typename _Tp>
bool EXOMatchedFilter::WaveformMatchesFilterSettings(const EXOTemplWaveform<_Tp>& wf) const
{
  // Template allows us to defer conversion to EXODoubleWaveform until later,
  // which can lead to better efficiency.  But of course you *will* need to
  // convert to EXODoubleWaveform before you can use your waveform.
  return (wf.GetLength()/2 + 1 == fMatchedFilter.GetLength() &&
          wf.GetSamplingFreq() == fMatchedFilter.GetSamplingFreq());
}

#endif
