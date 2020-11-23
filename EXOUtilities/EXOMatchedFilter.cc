#include "EXOMatchedFilter.hh"
#include "EXOErrorLogger.hh"
#include "EXOFastFourierTransformFFTW.hh"
#include <sstream>

#define USE_ROOT_FFTW 1
#define HAVE_FFTW 1

#ifdef HAVE_FFTW
#ifdef USE_ROOT_FFTW
extern "C" void *fftw_malloc(size_t n);
extern "C" void fftw_free(void *p);
#else
#include "fftw3.h"
#endif
#endif

bool EXOMatchedFilter::fUseNewArrayInterface = false;

EXOMatchedFilter::EXOMatchedFilter() :
  EXOVWaveformTransformer("EXOMatchedFilter" ), 
  fFFT(NULL),
  fOffset(0),
  fInternalArray(NULL),
  fInternalArrayLength(0)
{
  if (not EXOFastFourierTransformFFTW::IsAvailable()) {
    LogEXOMsg("Matched Filter requires a distribution with FFTW!", EEAlert);
  }
}

EXOMatchedFilter::~EXOMatchedFilter()
{
  if(fInternalArray != NULL) fftw_free(fInternalArray);
}

void EXOMatchedFilter::SetTemplateToMatch(const EXODoubleWaveform& wf,
                                          size_t nsample,
                                          Int_t offset)
{
  // Set template filter to match

  fOffset = offset;
  if ( nsample < wf.GetLength() ) {
    LogEXOMsg("nsample requested that will truncate the input waveform", EEDebug);
  }

  // Fetch the appropriate FFT, and save it.
  fFFT = &EXOFastFourierTransformFFTW::GetFFT(nsample);

  EXODoubleWaveform theory_signal;
  theory_signal.MakeSimilarTo(wf);
  theory_signal.SetLength(nsample);
  double per = wf.GetSamplingPeriod();
  for(size_t i=0; i<nsample; i++) {
    theory_signal[i] = wf.InterpolateAtPoint(((Int_t)i - fOffset)*per);
  }

  fFFT->PerformFFT(theory_signal, fMatchedFilter);

  // Remove baseline
  fMatchedFilter[0] *= 0;

  // If we have a noise power spectrum, use it.
  if ( fNoisePowerSpectrum.GetLength() != 0 ) {
    if ( !fNoisePowerSpectrum.IsSimilarTo( fMatchedFilter ) ) {
      //std::cout << "Fail noise spectrum" << std::endl;
      LogEXOMsg("Noise power spectrum has been given, but it is not similar to the input waveform.  Not processing", EEError);
    } 
    else { 
      //std::cout << "Using noise spectrum" << std::endl;
      fMatchedFilter /= fNoisePowerSpectrum; 
    }
  }

  // multiplicative factor; not sure why it's necessary, the reference I'm using included it, I figure it can't hurt.
  // This was 4 in the old code
  fMatchedFilter *= 4;
  // Now take the complex conjugate
  size_t len = fMatchedFilter.GetLength();
  for (size_t i=0; i<len;i++) fMatchedFilter[i] = std::conj(fMatchedFilter[i]);

}

void EXOMatchedFilter::TransformInPlace(EXODoubleWaveform& wf) const
{
#ifdef HAVE_FFTW
  if(not WaveformMatchesFilterSettings(wf)) {
    std::ostringstream os;
    os << " Input waveform and matched filter waveform do not match: "
       << " length =  "<< wf.GetLength() <<", matched filter length = "<< (fMatchedFilter.GetLength()-1)*2;
    LogEXOMsg(os.str().c_str(), EEError); 
    return;
  }

  // Make use of in-place FFTs.  We fill the array ourselves at the beginning (and retrieve it at the end).
  void* fftw_array;
  if(fUseNewArrayInterface) {
    size_t BytesNeeded = sizeof(double)*2*fFFT->GetFreqDomainLength();
    if(fInternalArrayLength < BytesNeeded) {
      if(fInternalArray != NULL) fftw_free(fInternalArray);
      fInternalArray = fftw_malloc(BytesNeeded);
      fInternalArrayLength = BytesNeeded;
    }
    fftw_array = fInternalArray;
  }
  else fftw_array = fFFT->GetInternalArray<void>();
  memcpy(fftw_array, reinterpret_cast<const void*>(&wf[0]), sizeof(double)*wf.GetLength());

  // Perform the convolution
  if(fUseNewArrayInterface) {
#ifdef USE_ROOT_FFTW
    LogEXOMsg("ROOT FFTW does not provide the new-array interface to fftw", EEAlert);
#else
    fftw_execute_dft_r2c(reinterpret_cast<fftw_plan>(fFFT->GetForwardPlan()),
                         reinterpret_cast<double*>(fftw_array),
                         reinterpret_cast<fftw_complex*>(fftw_array));
#endif
  }
  else {
    fFFT->PerformFFT_inplace();
  }
  for(size_t i = 0; i < fFFT->GetFreqDomainLength(); i++) {
    reinterpret_cast<std::complex<double>*>(fftw_array)[i] *= fMatchedFilter[i];
  }
  if(fUseNewArrayInterface) {
#ifdef USE_ROOT_FFTW
    LogEXOMsg("ROOT FFTW does not provide the new-array interface to fftw", EEAlert);
#else
    fftw_execute_dft_c2r(reinterpret_cast<fftw_plan>(fFFT->GetInversePlan()),
                         reinterpret_cast<fftw_complex*>(fftw_array),
                         reinterpret_cast<double*>(fftw_array));
#endif
  }
  else {
    fFFT->PerformInverseFFT_inplace();
  }

  // Retrieve values from fftw_array into wf.
  // Simultaneously, restore conventions from Numerical recipes by multiplying by 0.5.
  // Also, eliminate offset, since the user shouldn't need to deal with it.
  for(Int_t i = wf.GetLength() - 1; i >= fOffset; i--) {
    wf[i] = 0.5*reinterpret_cast<double*>(fftw_array)[i-fOffset];
  }
  if ( fOffset != 0 ) {
    for (Int_t i = fOffset - 1; i >= 0; i--) wf[i] = 0.0; 
  }
#else
  LogEXOMsg("Cannot use EXOMatchedFilter when we don't have an fft library", EEAlert);
#endif
}


