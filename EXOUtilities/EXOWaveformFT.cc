//______________________________________________________________________________
//                                                                        
// EXOWaveformFT                                                           
//                                                                        
// Encapsulates waveform Fourier Transform data. The major difference
// between this and a simple EXOCmplDblWaveform is that this waveform
// is based in frequency instead of time. That is, the indices are
// related to frequency rather than to time. 
//
// The format of aWaveformFT is such that it contains 0 -> n/2 values (DC to
// nyquist frequency) and not a total array of n complex values.  This is
// because the complex FT of real data is hermitian, so the latter half of the
// data (n/2 -> n) would be redundant.  For more details see
// http://www.fftw.org
//
// Also, see EXOFastFourierTransformFFTW.
//  
//______________________________________________________________________________

#include "EXOWaveformFT.hh"
#include "TH1D.h"

ClassImp( EXOWaveformFT )

//______________________________________________________________________________
EXOWaveformFT::CDbl EXOWaveformFT::InterpolateAtPoint( Double_t freq ) const
{
  // Linear interpolation function between two frequency points.  This function
  // returns the value of the first point if the requested *frequency* is below
  // 0 or the value of the last point if it's beyond the Nyquist frequency
  // (fSampleFreq/2).  If the waveform has 0 length, the function returns 0.

  if ( GetLength() == 0 ) return CDbl(0,0);
  Double_t nyquist_frequency = 0.5*GetSamplingFreq();
  Double_t bin_width = nyquist_frequency/(GetLength() - 1);

  // This gives us the expected entry number
  Double_t frac_entry = freq/bin_width; 
  if (frac_entry < 0) return At(0);

  size_t entry = static_cast<size_t>(frac_entry);
  if ( entry >= GetLength()-1 ) return At(GetLength()-1);

  // Now do linear interpolation
  frac_entry -= entry;
  return ((1.0-frac_entry)*At(entry) + frac_entry*At(entry+1)); 

} 

//______________________________________________________________________________
void EXOWaveformFT::LoadIntoHist(TH1D& hist, Option_t* opt) const
{
  // Loads the waveform into a user-supplied hist. This function is safer
  // because it is obvious that the user owns (and therefore must later delete)
  // this histogram.  EXOWaveformFT also allows options to be passed in:
  //
  // GimmeHist("", "Real") or LoadIntoHist(hist, "Real") // return Hist with Reals 
  // GimmeHist("", "Imag") or LoadIntoHist(hist, "Imag") // return Hist with
  //                                                     // Imaginary components 
  // GimmeHist("", "Abs") or LoadIntoHist(hist, "Abs")   // return Hist with
  //                                                     // Absolute value 
  // 

  hist.Reset();
  hist.SetMaximum(-1111);
  hist.SetMinimum(-1111);
  hist.SetTitle(GetName());
  if (GetLength() <= 1) {
    LogEXOMsg("Length <= 1, unable to produce histogram", EEError);    
    return;
  }
  Double_t nyquist_frequency = 0.5*GetSamplingFreq()/CLHEP::megahertz;
  Double_t bin_width = nyquist_frequency/(GetLength() - 1);
  hist.SetBins(GetLength(), -0.5*bin_width, nyquist_frequency + 0.5*bin_width);  
  hist.SetXTitle("f [MHz]");
  hist.SetOption("L");

  TString option = opt;
  option.ToLower();
  if (option.Contains("real")) {
    // Plot reals 
    hist.SetYTitle("Real (ADC Units)");
    for(size_t iSample = 0; iSample < GetLength(); iSample++) {
      hist.SetBinContent(iSample+1, At(iSample).real());
    }
  } else if (option.Contains("imag")) {
    // Plot imag 
    hist.SetYTitle("Imaginary (ADC Units)");
    for(size_t iSample = 0; iSample < GetLength(); iSample++) {
      hist.SetBinContent(iSample+1, At(iSample).imag());
    }
  } else { 
    // Default is abs
    hist.SetYTitle("Abs (ADC Units)");
    for(size_t iSample = 0; iSample < GetLength(); iSample++) {
      hist.SetBinContent(iSample+1, std::abs(At(iSample)));
    }
  }
}

//______________________________________________________________________________
Double_t GetFrequencyAtIndex(const EXOCmplDblWaveform& aWF, size_t index) 
{
  // Function to deal with inheritance issues between EXOWaveformFT and
  // EXOCmplDblWaveform.  Return frequency corresponding to the element at
  // index.

  if (index >= aWF.GetLength()) return 0.5*aWF.GetSamplingFreq();
  Double_t bin_width = 0.5*aWF.GetSamplingFreq()/(aWF.GetLength() - 1);
  return index*bin_width;
}

//______________________________________________________________________________
Double_t EXOWaveformFT::GetFrequencyAtIndex(size_t index) const
{
  // Return the frequency corresponding to the element at index. If index is
  // beyond the length of the waveform, simply return the nyquist frequency. 
  return ::GetFrequencyAtIndex(*this, index);
}

//______________________________________________________________________________
void EXOWaveformFT::Refine(EXOCmplDblWaveform& aWF) const
{
  // Refine this waveformFT into passed-in waveform.FT.  We interpolate this
  // waveform to fit the requirements of the passed-in waveform (length,
  // sampling frequency) where we have interpolated between points (by calling
  // InterpolateAtPoint).  The properties of aWFFT (length, sampling frequency)
  // should of course be set before calling this function.  

  for(size_t i = 0; i < aWF.GetLength(); i++) {
    aWF[i] = InterpolateAtPoint(::GetFrequencyAtIndex(aWF, i));
  }
}

