/**        
 *      
 * CLASS DECLARATION:  EXOFastFourierTransformFFTW.hh
 *
 * DESCRIPTION: 
 *
 * EXOFastFourierTransformFFTW handles the FT of a waveform.  This is very useful especially when it 
 * comes to the convolution of different functions with a waveform.  The current
 * library used is the fftw3 library: see http://www.fftw.org/
 *
 * AUTHOR: M. Marino 
 * CONTACT: 
 * FIRST SUBMISSION: 
 * 
 * REVISION:
 * 
 */

#ifndef _EXOFastFourierTransformFFTW_HH
#define _EXOFastFourierTransformFFTW_HH

#include "EXOWaveformFT.hh"
#include <map>
#include <cstddef> //for size_t

class EXOFastFourierTransformFFTW 
{
  public:
    
    class FFTMap : public std::map<size_t, EXOFastFourierTransformFFTW*>
    {
      public:
        ~FFTMap();
    };

    // Perform a Fourier Transform on the data in aWaveform, storing it in 
    // aWaveformFT. 
    virtual void PerformFFT( 
      const EXODoubleWaveform& aWaveform, 
      EXOWaveformFT& aWaveformFT );

    // Perform an inverse Fourier Transform on the data in aWaveformFT, storing 
    // it in  aWaveformFT. 
    virtual void PerformInverseFFT( 
      EXODoubleWaveform& aWaveform,  
      const EXOWaveformFT& aWaveformFT);

    // Provide direct access to the raw FFT function.  Internal array is used.
    virtual void PerformFFT_inplace();

    // Provide direct access to the raw FFT function.  Internal array is used.
    virtual void PerformInverseFFT_inplace();

    static EXOFastFourierTransformFFTW& GetFFT(size_t length); 

    // Direct access to the internal array (for optimization when copy steps are undesirable).
    // You should specify a return type like:
    //   double* real_fft = fft.GetInternalArray<double>();
    //   std::complex<double>* complex_fft = fft.GetInternalArray<std::complex<double> >();
    template<typename T>
    T* GetInternalArray() {return reinterpret_cast<T*>(fInternalArray);}

    // Also provide access to the plans themselves -- mainly to use the new-array interface.
    void *GetForwardPlan() const {return fTheForwardPlan;}
    void *GetInversePlan() const {return fTheInversePlan;}
    
    static bool IsAvailable();

    size_t GetTimeDomainLength() const {return fLength;}
    size_t GetFreqDomainLength() const {return fLength/2 + 1;}

  protected:
    void *fTheForwardPlan; 
    void *fTheInversePlan; 
    void *fInternalArray;
    size_t fLength;
    static FFTMap fMap;
    EXOFastFourierTransformFFTW(size_t length);
    EXOFastFourierTransformFFTW(const EXOFastFourierTransformFFTW&);
    EXOFastFourierTransformFFTW& operator=(const EXOFastFourierTransformFFTW&);
    virtual ~EXOFastFourierTransformFFTW();
  private:
    EXOFastFourierTransformFFTW();
    
 
};

#endif /* _EXOFastFourierTransformFFTW_HH */
