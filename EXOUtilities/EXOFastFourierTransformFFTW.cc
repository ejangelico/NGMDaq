#include "EXOFastFourierTransformFFTW.hh"
#include "EXOErrorLogger.hh"
#include <cassert>
#include <cstring>
#define USE_ROOT_FFTW 1
#define HAVE_FFTW 1
#ifdef USE_ROOT_FFTW
// The following is to avoid using ROOT's cludgy interface.  fftw_plan_dft_r2c
// is defined in libFFTW, but we also need the header information (copied
// here).  This will be modified when Vladimir completes the autoconf setup. 
extern "C" {
#if !defined(FFTW_NO_Complex) && defined(_Complex_I) && defined(complex) && defined(I)
#  define FFTW_DEFINE_COMPLEX(R, C) typedef R _Complex C
#else
#  define FFTW_DEFINE_COMPLEX(R, C) typedef R C[2]
#endif

#define FFTW_EXTERN extern
#define FFTW_DEFINE_API(X, R, C)                                           \
                                                                           \
FFTW_DEFINE_COMPLEX(R, C);                                                 \
                                                                           \
typedef struct X(plan_s) *X(plan);                                         \
                                                                           \
FFTW_EXTERN void *X(malloc)(size_t n);                                     \
                                                                           \
FFTW_EXTERN void X(free)(void *p);                                         \
                                                                           \
FFTW_EXTERN void X(execute)(const X(plan) p);                              \
                                                                           \
FFTW_EXTERN void X(destroy_plan)(X(plan) p);                               \
                                                                           \
FFTW_EXTERN X(plan) X(plan_dft_c2r)(int rank, const int *n,                \
                        C *in, R *out, unsigned flags);                    \
FFTW_EXTERN X(plan) X(plan_dft_r2c)(int rank, const int *n,                \
                        R *in, C *out, unsigned flags);                    

#define FFTW_CONCAT(prefix, name) prefix ## name
#define FFTW_MANGLE_DOUBLE(name) FFTW_CONCAT(fftw_, name)
#define FFTW_ESTIMATE (1U << 6)

FFTW_DEFINE_API(FFTW_MANGLE_DOUBLE, double, fftw_complex)
}
// End include FFTW header
#else
#ifdef HAVE_FFTW
#include "fftw3.h"
#endif
#endif

//______________________________________________________________________________
// This class implements the fftw3 and takes the data from an EXOWaveform and
// FFT's it into a EXOWaveformFT.  Given that the data is real, one can take
// advantage of this and improve speed and memory usage.  This class performs
// the FFT operation on the data of the EXOWaveform and EXOWaveformFT classes.
//
// To use this class, you can do the following:
//
//   EXODoubleWaveform wf; 
//   EXOWaveformFT wfFT;
//   EXOFastFourierTransformFFTW::GetFFT(wf.GetLength()).PerformFFT(wf, wfFT);
//
// Of course, you can save a reference to the FFT you want (assuming that all
// remaining FFTs will always use this logical length.  (Logical length means
// the length of the real data.  Complex data is stored in an array of size
// length/2 + 1.)  A reference/pointer can be saved as the follwoing:
//
//   EXOFastFourierTransformFFTW& fft = EXOFastFourierTransformFFTW::GetFFT(2048);
//
// To use the in-place functions PerformFFT_inplace and PerformInverseFFT_inplace,
// use GetInternalArray to input/output your data directly.  It will not be modified
// until another FFT of the same length is requested.
//      
// CLASS IMPLEMENTATION:  EXOFastFourierTransformFFTW.cc
//
// AUTHOR: M. Marino 
// CONTACT: 
// FIRST SUBMISSION: 
// 
// REVISION:
//
//   April 2014: Modifications so that FFTW is always done in-place on an internal array.
//               The array is allocated with fftw_malloc, to guarantee alignment (particularly
//               important as we move toward storing wisdom).  Copies should now be avoided
//               by accessing this internal array directly.
//
//______________________________________________________________________________


EXOFastFourierTransformFFTW::FFTMap EXOFastFourierTransformFFTW::fMap;
EXOFastFourierTransformFFTW::EXOFastFourierTransformFFTW(size_t length) : 
  fTheForwardPlan(NULL),
  fTheInversePlan(NULL),
  fInternalArray(fftw_malloc(sizeof(double)*2*(length/2 + 1))),
  fLength(length)
{
  // Default constructor.

  // Produce plans at construction.
  // (Necessary because planning overwrites the input arrays,
  // so we must do this before users start manipulating fInternalArray.)
  const int temp = fLength;
  fTheForwardPlan = fftw_plan_dft_r2c( 1, &temp,
         static_cast<double*>(fInternalArray),
         static_cast<fftw_complex*>(fInternalArray),
         FFTW_ESTIMATE );
  fTheInversePlan = fftw_plan_dft_c2r( 1, &temp,
         static_cast<fftw_complex*>(fInternalArray),
         static_cast<double*>(fInternalArray),
         FFTW_ESTIMATE );
}

//______________________________________________________________________________
EXOFastFourierTransformFFTW::EXOFastFourierTransformFFTW(const EXOFastFourierTransformFFTW& other) : 
  fTheForwardPlan(NULL),
  fTheInversePlan(NULL),
  fInternalArray(fftw_malloc(sizeof(double)*2*(other.fLength/2 + 1))),
  fLength(other.fLength)
{
  // Copy constructor.  Do not copy the plans of the other FFT
  // FixME: we could copy the plans of the other FFT if plans were handled in shared pointers.

  // Produce plans at construction.
  // (Necessary because planning overwrites the input arrays,
  // so we must do this before users start manipulating fInternalArray.)
  const int temp = fLength;
  fTheForwardPlan = fftw_plan_dft_r2c( 1, &temp,
         static_cast<double*>(fInternalArray),
         static_cast<fftw_complex*>(fInternalArray),
         FFTW_ESTIMATE );
  fTheInversePlan = fftw_plan_dft_c2r( 1, &temp,
         static_cast<fftw_complex*>(fInternalArray),
         static_cast<double*>(fInternalArray),
         FFTW_ESTIMATE );
}

//______________________________________________________________________________
EXOFastFourierTransformFFTW& 
  EXOFastFourierTransformFFTW::operator=(const EXOFastFourierTransformFFTW& other) 
{
#ifdef HAVE_FFTW
  if(fLength != other.fLength) {
    // FixME: Does fftw_destroy_plan also destroy wisdom?
    if (fTheForwardPlan != NULL) fftw_destroy_plan((fftw_plan)fTheForwardPlan);
    if (fTheInversePlan != NULL) fftw_destroy_plan((fftw_plan)fTheInversePlan);
    fftw_free(fInternalArray);
    fInternalArray = fftw_malloc(sizeof(double)*2*(other.fLength/2 + 1));
    fTheForwardPlan = NULL;
    fTheInversePlan = NULL;
    fLength = other.fLength;
  }
#endif
  return *this;
}

//______________________________________________________________________________
EXOFastFourierTransformFFTW& EXOFastFourierTransformFFTW::GetFFT( size_t length )
{
  // Return Object given a processing length.
  FFTMap::iterator iter;
  if ( (iter = fMap.find(length)) == fMap.end() ) {
      iter = fMap.insert(std::make_pair(length,new EXOFastFourierTransformFFTW(length))).first;
  }
  return *(iter->second);
}

//______________________________________________________________________________
EXOFastFourierTransformFFTW::~EXOFastFourierTransformFFTW()
{
  // Delete.
#ifdef HAVE_FFTW
  if (fTheForwardPlan != NULL) fftw_destroy_plan((fftw_plan)fTheForwardPlan);
  if (fTheInversePlan != NULL) fftw_destroy_plan((fftw_plan)fTheInversePlan);
  if (fInternalArray != NULL) fftw_free(fInternalArray);
#endif
}

//______________________________________________________________________________
void EXOFastFourierTransformFFTW::PerformFFT( const EXODoubleWaveform& aWaveform, 
                                              EXOWaveformFT& aWaveformFT )
{
  // Performs an Real-to-complex FFT on aWaveform, returning the data in
  // aWaveformFT.  aWaveformFT will be resized to be aWaveform.GetLength()/2 +
  // 1.  That is, since a DFT of real data generates hermitian data, only half
  // of this data must be stored.  For more details see http://www.fftw.org
  //
  // There is a double copy.  aWaveform gets copied internally before the
  // fourier transfrom.  The resulting FT waveform is then copied into
  // aWaveformFT.  To avoid this, consider calling PerformFFT_inplace() directly.

#ifdef HAVE_FFTW
  if ( fLength != aWaveform.GetLength() ) {
    LogEXOMsg("Called without correct length", EEError);
    return;
  }
  assert(fInternalArray);
  memcpy(fInternalArray, reinterpret_cast<const void*>(&aWaveform[0]),
         sizeof(double)*aWaveform.GetLength());
  PerformFFT_inplace();
  aWaveformFT.SetData(GetInternalArray<std::complex<double> >(), GetFreqDomainLength());
  aWaveformFT.SetSamplingFreq(aWaveform.GetSamplingFreq());
#else
  LogEXOMsg("Compiled without FFTW3", EEError);
#endif
  
}

//______________________________________________________________________________
void EXOFastFourierTransformFFTW::PerformInverseFFT( EXODoubleWaveform& aWaveform,  
                                                     const EXOWaveformFT& aWaveformFT )
{
  // Performs an Complex-to-Real inverse FFT on aWaveformFT, returning the data
  // in aWaveform.  aWaveform will be resized to be the logical size, n,  of
  // this transformation (defined when EXOFastFourierTransformFFTW::GetFFT was
  // called).  The format of aWaveformFT is such that it contains 0 -> n/2
  // values (DC to nyquist frequency) and not a total array of n complex
  // values.  This is because the complex FT of real data is hermitian, so the
  // latter half of the data (n/2 -> n) would be redundant.  For more details
  // see http://www.fftw.org

#ifdef HAVE_FFTW
  if ( fLength/2 + 1 != aWaveformFT.GetLength() ) {
    LogEXOMsg("Called without correct length", EEError);
    return;
  }
  assert(fInternalArray);
  memcpy(fInternalArray, reinterpret_cast<const void*>(&aWaveformFT[0]),
         sizeof(std::complex<double>)*aWaveformFT.GetLength());
  PerformInverseFFT_inplace();
  aWaveform.SetData(GetInternalArray<double>(), GetTimeDomainLength());
  aWaveform.SetSamplingFreq(aWaveformFT.GetSamplingFreq());
#else
  LogEXOMsg("Compiled without FFTW3", EEError);
#endif

}

//______________________________________________________________________________
void EXOFastFourierTransformFFTW::PerformFFT_inplace()
{
  // Perform an in-place forward transform on our internal array.
  // No copying is done.
#ifdef HAVE_FFTW
  assert(fInternalArray);
  assert(fTheForwardPlan);
  fftw_execute( (fftw_plan)fTheForwardPlan );
#else
  LogEXOMsg("Compiled without FFTW3", EEError);
#endif
}

//______________________________________________________________________________
void EXOFastFourierTransformFFTW::PerformInverseFFT_inplace()
{
  // Perform an in-place inverse transform on our internal array.
  // No copying is done.
#ifdef HAVE_FFTW
  assert(fInternalArray);
  assert(fTheInversePlan);
  fftw_execute( (fftw_plan)fTheInversePlan );
#else
  LogEXOMsg("Compiled without FFTW3", EEError);
#endif
}

//______________________________________________________________________________
bool EXOFastFourierTransformFFTW::IsAvailable()
{
  // Returns is an FFTW is available in this build.
#ifdef HAVE_FFTW
  return true;
#else
  return false;
#endif
}

//______________________________________________________________________________
EXOFastFourierTransformFFTW::FFTMap::~FFTMap()
{
  for (iterator f=begin();f!=end();f++) delete f->second;
}
