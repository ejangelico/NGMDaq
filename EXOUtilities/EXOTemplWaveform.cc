#include "TH1D.h"
#include "TSystem.h"
#include "TROOT.h"
#include "EXOTemplWaveform.hh"
#include <algorithm>
//______________________________________________________________________________
//
//  EXOTemplWaveform
//
//  Class describes a generic waveform, including automatic conversion
//  between different types.  The template parameter describes the type of
//  data stored by the waveform.  There are several available typedefs:
//
//    EXODoubleWaveform
//    EXOWaveformFT
//    EXOIntWaveform
//    EXOFloatWaveform
//
//  Visualization of the class is handled by GimmeHist() which returns a
//  TH1D.  For example:
//
//    EXODoubleWaveform wf; 
//    wf.GimmeHist().Draw();
//
//  It is also possible (and desirable) to pass in a TH1D that the waveform
//  will fill, via:
//
//    TH1D hist;
//    wf.LoadIntoHist(hist);
//
//  This is preferable because it makes it obvious who owns the histogram.
//  Automatic conversion is also possible.  To go from an integer to a double
//  waveform, just do: 
// 
//  EXOIntWaveform intWF;
//  EXODoubleWaveform doubleWF = intWF;
//
//  For information on how to generate a fourier transform (i.e.
//  EXOWaveformFT), see EXOFastFourierTransformFFTW.
//
//  *************************************************************************
//  *NOTE when using in (python) scripts*: 
//    EXOTemplWaveforms are iterable so anything that takes an iterable will
//    work:
//  
//   wf = ROOT.EXODoubleWaveform()
//   ...
//   sum(wf) # returns sum
//   for i in wf: print i # prints out points
//   alist = map(ROOT.TMath.Sin, wf) # Performs Sin function and returns as list
//   aslice = wf(3,6) # Slicing, unfortunately the standard python list slice
//                      wf[3:6] is *not* supported. 
//
//  Conversion should happen as in normal code.  That is, the following python
//  code:
//
//    wf = ROOT.EXOWaveform() 
//    dblWF = ROOT.EXODoubleWaveform(wf) 
//    ...
//    # One can also call:
//    dblWF.__assign__(wf)
//
//  will automatically convert the integer waveform (which EXOWaveform is) to a
//  double waveform.  This will work for almost all types.  The following can
//  also be used to convert in CINT:
//
//    EXOWaveform wf;
//    EXODoubleWaveform dblWF;
//    dblWF = wf;
//
//______________________________________________________________________________
ClassImpT(EXOTemplWaveform, def)

typedef std::complex<double> CDbl;

//______________________________________________________________________________
// The following is an unfortunate workaround to deal with loading complex
// variables for I/O involving complex<double> etc. during standalone apps.  It
// is currently required with ROOT 5.28, we need to check if it's required for
// newer versions.
#include "TClass.h"
#include "EXOMiscUtil.hh"
class ROOTComplexWorkaround {
  private:
    bool fDidLoad;
    ROOTComplexWorkaround() : fDidLoad(false)
    { 
#ifndef STATIC
      //if(gSystem->Load("complex") == 0) fDidLoad = true;
#endif
    }
    ~ROOTComplexWorkaround()
    {
#ifndef STATIC
      // Necessary to ensure libraries are unloaded in the proper order when a program terminates.
      // Failure to do this can, in some cases, cause termination to hang.
      //if(fDidLoad) gSystem->Unload("complex");
#endif
    }
  public:
    static ROOTComplexWorkaround& GetWorkaround() 
    {
      static ROOTComplexWorkaround gfWorkaround; 
      return gfWorkaround;
    }
};

// Force the static instantiation and file scope.
static ROOTComplexWorkaround* gfWorkaroundPtr = NULL; 
//______________________________________________________________________________

//______________________________________________________________________________
template<typename _Tp>
TH1D* EXOTemplWaveform<_Tp>::GimmeHist(const std::string& label, Option_t* opt) const
{
  // Returns a histogram named EXOWaveformHist_[label/ID]. If the user does not
  // supply a label, the channel of the waveform  is used. The function first
  // searches gROOT for EXOWaveformHist_[label/ID]. If it is found, that hist
  // is used.  If it is not found, the function returns a newly allocated
  // histogram of the waveform. Either way, it is the user's responsibility to
  // delete the histogram.

  if(label == "") return GimmeHist(GetName(), opt);

  TDirectory* tmpDirectory = gDirectory;
  gROOT->cd();
  TH1D* hist = dynamic_cast< TH1D* >( 
      gROOT->FindObject(("EXOWaveformHist_" +  label).c_str()) );
  if(hist == NULL) {
    hist = new 
      TH1D(("EXOWaveformHist_" + label).c_str(), "", 1, 0, 1);
  }
  LoadIntoHist(*hist, opt);
  tmpDirectory->cd();
  return hist;
}			    

//______________________________________________________________________________
template<typename _Tp>
void EXOTemplWaveform<_Tp>::LoadIntoHist(TH1D& hist, Option_t* /*opt*/) const
{
  // Loads the waveform into a user-supplied hist. This function is safer
  // because it is obvious that the user owns (and therefore must later delete)
  // this histogram.
  hist.Reset();
  hist.SetMaximum(-1111);
  hist.SetMinimum(-1111);
  hist.SetTitle(GetName());
  double bin_width = GetSamplingPeriod()/CLHEP::microsecond;
  hist.SetBins(GetLength(), GetMinTime()/CLHEP::microsecond - 0.5*bin_width, 
                            GetMaxTime()/CLHEP::microsecond - 0.5*bin_width);  
  hist.SetXTitle("t [#mus]");
  hist.SetYTitle("ADC Units");
  hist.SetOption("L");

  for(size_t iSample = 0; iSample < GetLength(); iSample++) {
    hist.SetBinContent(iSample+1, (double)At(iSample));
  }
}

template<>
void EXOTemplWaveform<CDbl>::LoadIntoHist(TH1D&, Option_t*) const
{
  // Disable for CDbl
}

//______________________________________________________________________________
template<typename _Tp>
_Tp EXOTemplWaveform<_Tp>::Sum( size_t start, size_t stop) const
{
  // Get the sum of the waveform beginning at position start (including start)
  // and ending at stop (*not* including stop).  Default is to sum the entire
  // waveform.  This is equivalent to:
  // 
  // val = 0; 
  // for (i=start;i<stop;i++) val += wf[i];
  if ( stop > GetLength() ) stop = GetLength(); 
  if (start >= stop) return _Tp(0);
  return std::accumulate(fData.begin()+start, fData.begin()+stop, _Tp(0));
}

//______________________________________________________________________________
template<typename _Tp>
_Tp EXOTemplWaveform<_Tp>::GetMaxValue() const
{
  // Get the maximum of the waveform. Returns zero for complex waveforms
  return *(std::max_element(fData.begin(),fData.end()));
}

//______________________________________________________________________________
template<>
CDbl EXOTemplWaveform<CDbl>::GetMaxValue() const
{
  // Disable for CDbl
  return CDbl();
}

//______________________________________________________________________________
template<typename _Tp>
_Tp EXOTemplWaveform<_Tp>::GetMinValue() const
{ 
  // Get the minimum of the waveform. Returns zero for complex waveforms
  return *(std::min_element(fData.begin(),fData.end()));
}

//______________________________________________________________________________
template<>
CDbl EXOTemplWaveform<CDbl>::GetMinValue() const
{
  // Disable for CDbl
  return CDbl();
}

//______________________________________________________________________________
template<typename _Tp>
EXOTemplWaveform<_Tp> EXOTemplWaveform<_Tp>::SubWaveform(size_t begin, size_t end) const
{
  // get a sub-waveform of this waveform.  begin is the start of the waveform,
  // end is the point *after* which should be saved.  That is, to get a
  // sub-waveform beginning at the 10th point to the end of a 200 point
  // waveform, one would call SubWaveform(10, 200);
  if (begin >= GetLength()) begin = GetLength() - 1;
  if (end > GetLength()) end = GetLength();
  if (begin > end) begin = end;
  EXOTemplWaveform<_Tp> wf;
  wf.MakeSimilarTo(*this);
  wf.SetData(GetData() + begin, (end-begin));
  return wf;
}

//______________________________________________________________________________
template<typename _Tp>
void EXOTemplWaveform<_Tp>::Append(const EXOTemplWaveform<_Tp>& wf) 
{
  if (wf.GetSamplingFreq() != GetSamplingFreq() ) {
    LogEXOMsg("Cannot append waveforms with different frequencies", EEError);    
    return;
  }
  fData.insert( fData.end(), wf.GetVectorData().begin(), wf.GetVectorData().end() );
}

//______________________________________________________________________________
#define SYNTHESIZE_CASE_TYPE_AND_INIT_CHECK(atype, aWF)            \
  if (aWF.IsA()->InheritsFrom(EXOTemplWaveform<atype >::Class())) {\
    const EXOTemplWaveform<atype >& refCast =                      \
        static_cast<const EXOTemplWaveform<atype >&>(aWF);         \
    MakeSimilarTo(refCast);                                        \
    SetData(refCast.GetData(), refCast.GetLength());               \
    return;  } 
  

template<typename _Tp>
EXOTemplWaveform<_Tp>::EXOTemplWaveform(const TObject& aWF)
{
  // A constructor that takes a TObject argument, purpose being to give
  // accesibility to scripts, since both CINT and pyROOT don't handle
  // templates. 
  LoadGlobalLibs();
  ConvertFrom(aWF);
}

template<typename _Tp>
EXOTemplWaveform<_Tp>& EXOTemplWaveform<_Tp>::operator=(const TObject& aWF)
{
  // An assignment operator that takes a TObject argument, purpose being to
  // give accesibility to scripts, since both CINT and pyROOT don't handle
  // templates. 
  if (this == &aWF) return *this; 
  ConvertFrom(aWF);
  return *this;
}

//______________________________________________________________________________
template<typename _Tp>
void EXOTemplWaveform<_Tp>::ConvertFrom(const TObject& aWF)
{
  // Convert from the anput waveform
  SYNTHESIZE_CASE_TYPE_AND_INIT_CHECK(Int_t, aWF)
  SYNTHESIZE_CASE_TYPE_AND_INIT_CHECK(Double_t, aWF)
  SYNTHESIZE_CASE_TYPE_AND_INIT_CHECK(Float_t, aWF)
  SYNTHESIZE_CASE_TYPE_AND_INIT_CHECK(Char_t, aWF)
  SYNTHESIZE_CASE_TYPE_AND_INIT_CHECK(unsigned long, aWF)
  SYNTHESIZE_CASE_TYPE_AND_INIT_CHECK(unsigned int, aWF)
  //SYNTHESIZE_CASE_TYPE_AND_INIT_CHECK(std::complex<double>, aWF)

  LogEXOMsg("Input waveform type not recognized!", EEError);
}

//______________________________________________________________________________
template<typename _Tp>
void EXOTemplWaveform<_Tp>::LoadGlobalLibs()
{
  // A hook to load global libraries for particular classes.  Only
  // used by complex class.
}

template<>
void EXOTemplWaveform<CDbl>::LoadGlobalLibs()
{
  // Force loading of complex libraries.  
  if(!gfWorkaroundPtr) gfWorkaroundPtr = &ROOTComplexWorkaround::GetWorkaround();
}


//______________________________________________________________________________
// The following are necessary to ensure that the above functions are generated.
template class EXOTemplWaveform<Int_t>; 
template class EXOTemplWaveform<Double_t>; 
template class EXOTemplWaveform<Float_t>; 
template class EXOTemplWaveform<Char_t>; 
template class EXOTemplWaveform<unsigned long>; 
template class EXOTemplWaveform<unsigned int>; 
template class EXOTemplWaveform<std::complex<double> >; 
