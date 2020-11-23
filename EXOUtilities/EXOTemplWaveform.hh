#ifndef EXOTemplWaveform_hh
#define EXOTemplWaveform_hh
#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef HEP_SYSTEM_OF_UNITS_H
#include "SystemOfUnits.hh"
#endif
#include "EXOErrorLogger.hh"
#include <vector> 
#include <string> 
#include <iostream> 
#include <cmath>
#include <complex> 
#include <numeric>
#include <cstddef> //for size_t

class TH1D;
template<typename _Tp>
class EXOTemplWaveform : public TObject {
   public:

    //////////////////////////////////////////////////////////
    // Get/Set-Data algorithms
    typedef std::vector<_Tp> VecType;

    _Tp* GetData() 
    { 
      // Return a pointer to the internal data
      return (fData.size() > 0 ) ? &(fData)[0] : NULL; 
    }

    const _Tp* GetData() const 
    { 
      // Return a const pointer to the internal data
      return (fData.size() > 0 ) ? &(fData)[0] : NULL; 
    }

    VecType& GetVectorData() 
    { 
      // Return the vector data
      return fData; 
    }

    const VecType& GetVectorData() const 
    { 
      // Return the vector data
      return fData; 
    }

    template<typename _Tn>
    void SetData( const _Tn* aData, size_t numberOfValues ) 
    { 
      // Set the data by inputting an array
      // The following handles all basic types.
      //fData.assign( aData, aData + numberOfValues ); 
      SetLength(numberOfValues);
      for (size_t i = 0; i < numberOfValues; i++) fData[i] = static_cast<_Tp>(aData[i]);
    }

    template<typename _Tn>
    void SetData( const _Tn& aData, size_t start_, size_t end_ )
    {
      // Set the data by assigning from aData.
      // aData can be any indexable object, such as a c-array or an EXOTemplWaveform.
      // This is useful partly for python, where it is difficult to do pointer arithmetic
      // with the other form of this function.
      SetLength(end_ - start_);
      for (size_t i = start_; i < end_; i++) fData[i-start_] = static_cast<_Tp>(aData[i]);
    }

    void SetLength( size_t length ) 
    { 
      // Set the length of the waveform
      fData.resize(length); 
    }

    size_t GetLength() const 
    { 
      // Return the length of the waveform
      return fData.size(); 
    }

    size_t size() const
    {
      // Return the length, this gets mapped to __len__ by pyROOT
      return GetLength();
    }

    Double_t GetSamplingFreq() const 
    { 
      // Returns sampling frequency (CLHEP units)
      return fSampleFreq;
    }

    Double_t GetSamplingPeriod() const 
    { 
      // Returns sampling period (CLHEP units)
      return 1./fSampleFreq;
    }

    void SetSamplingFreq(double freq) 
    { 
      // Sets sampling frequency (CLHEP units)
      fSampleFreq = freq;
    }

    void SetSamplingPeriod(double per) 
    { 
      // Sets sampling period (CLHEP units)
      fSampleFreq = 1./per;
    }

    Double_t GetTOffset() const 
    { 
      // Return the offset time
      return fTOffset;
    }

    void SetTOffset(double time) 
    { 
      // Sets the offset time 
      fTOffset = time;
    }

    Double_t GetMaxTime() const
    {
      // Get maximum time of the waveform
      return GetTimeAtIndex(GetLength()); 
    }

    Double_t GetMinTime() const
    {
      // Get the minimum time of the waveform
      return GetTOffset();
    }

    _Tp  At(size_t i) const 
    { 
      // Return value at i
      return fData.at(i); 
    } 

    _Tp& operator[](size_t i) 
    { 
      // Return (reference) value at i
      return fData[i]; 
    }

    const _Tp& operator[](size_t i) const 
    { 
      // Return value at i
      return fData[i]; 
    }

    void Zero()
    { 
      // Zero the waveform
      fData.assign(fData.size(), 0);
    }


    //////////////////////////////////////////////////////////
    // Similarity checks
    template<typename _Op>
    bool IsSimilarTo(const EXOTemplWaveform<_Op>& other) const
    {
      // Returns true if waveforms are similar, false if not.  At the
      // moment, this only checks the length of the waveform.
      return ( GetLength() == other.GetLength() && 
               GetSamplingFreq() == other.GetSamplingFreq() &&
               GetTOffset() == other.GetTOffset() );
    }

    template<typename _Op>
    void MakeSimilarTo(const EXOTemplWaveform<_Op>& other)
    {
      // Make this waveform similar to the input.
      // Since this only operates on the timing information, not type information,
      // we can apply it to waveforms with different underlying data types;
      // This is useful, eg. for creating an EXOIntWaveform from a EXODoubleWaveform in the digitizer.
      SetLength(other.GetLength());
      SetSamplingFreq(other.GetSamplingFreq());
      SetTOffset(other.GetTOffset());
    }

    //////////////////////////////////////////////////////////
    // Comparison operator
    bool operator==(const EXOTemplWaveform<_Tp>& other) const
    {
      // Compare waveforms, only return true if everything is the same.
      return ( IsSimilarTo(other) and 
               std::equal( fData.begin(), fData.end(), other.fData.begin() ) ); 
    }

    //////////////////////////////////////////////////////////
    // Vector and scalar operations
    #define BINARY_OPERATOR_NO_CHECK(bin_op, other, tc)      \
      size_t n = GetLength();                                \
      for(size_t i=0; i<n; i++)                              \
        fData[i] bin_op static_cast<tc>(other[i]);               

    #define BINARY_OPERATOR(bin_op, other, tc)               \
    if (!IsSimilarTo(other)) {                               \
      std::cout << "Waveforms are not similar" << std::endl; \
    } else {                                                 \
      BINARY_OPERATOR_NO_CHECK(bin_op, other, tc)            \
    }                                                        \
    return *this;

    EXOTemplWaveform<_Tp>& operator*=(const EXOTemplWaveform<_Tp>& other) 
    {
      // Vector multiplication.  Waveforms must be similar (defined by
      // IsSimilarTo()) or this function will return without doing anything.
      BINARY_OPERATOR(*=, other, _Tp)
    }

    EXOTemplWaveform<_Tp>& operator/=(const EXOTemplWaveform<_Tp>& other) 
    {
      // Vector division.  Waveforms must be similar (defined by IsSimilarTo())
      // or this function will return without doing anything.  Divide by zero
      // is not checked!
      BINARY_OPERATOR(/=, other, _Tp)
    }

    EXOTemplWaveform<_Tp>& operator-=(const EXOTemplWaveform<_Tp>& other) 
    {
      // Vector subtraction.  Waveforms must be similar (defined by
      // IsSimilarTo()) or this function will return without doing anything.
      BINARY_OPERATOR(-=, other, _Tp)
    }

    EXOTemplWaveform<_Tp>& operator+=(const EXOTemplWaveform<_Tp>& other) 
    {
      // Vector addition.  Waveforms must be similar (defined by IsSimilarTo())
      // or this function will return without doing anything.
      BINARY_OPERATOR(+=, other, _Tp)
    }

    EXOTemplWaveform<_Tp>& operator+=(double value)
    {
      // Scalar addition
      size_t n = GetLength();
      for(size_t i=0; i<n; i++) fData[i] += static_cast<_Tp>(value);
      return *this;
    }

    EXOTemplWaveform<_Tp>& operator*=(double value) 
    {
      // Scalar multiplication
      size_t n = GetLength();
      for(size_t i=0; i<n; i++) fData[i] = static_cast<_Tp>(value*fData[i]); 
      return *this;
    }

    EXOTemplWaveform<_Tp>& operator-=(double value) 
    { 
      // Scalar subtraction 
      return (*this) += -value; 
    }

    EXOTemplWaveform<_Tp>& operator/=(double value) 
    { 
      // Scalar division 
      return (*this) *= (1.0/value); 
    }

    template<class _Function>
    void ApplyToEach(_Function f) 
    {
      // Apply the function or class overloading the operator() to the waveform
      // data.  This is a similar behavior to std::for_each in algorithm.  For
      // example, to apply a sqrt to the data, do something like: 
      //
      // my_waveform.ApplyToEach(TMath::Sqrt);
      //
      // which will apply the sqrt function to each point and save the result
      // in that point.

      size_t n = GetLength();
      for(size_t i=0; i<n; i++) {
        fData[i] = f(static_cast<_Tp>(fData[i])); 
      }
    }

    //////////////////////////////////////////////////////////
    // Common operations on lists of numbers 
    virtual _Tp GetMaxValue() const; 
    virtual _Tp GetMinValue() const;
    _Tp Sum( size_t start = 0, size_t stop = (size_t)-1 ) const;

    template<typename _Op>
    _Op StdDevSquared( size_t start = 0, size_t stop = (size_t)-1) const
    {
      // Get the std dev squared of the waveform beginning at position start and
      // ending at stop.  Default is to sum the entire waveform.  This is
      // equivalent to:
      //
      // val = 0; 
      // avg = Sum(start, stop);
      // for (i=start;i<stop;i++) val += wf[i]*wf[i];
      // return val/(stop - start) - avg*avg;
    
      if ( stop > GetLength() ) stop = GetLength();
      if (start >= stop) return _Op(0);
      _Op avg = static_cast<_Op>(Sum( start, stop ))/(stop-start);
      _Tp temp = std::inner_product(fData.begin()+start, fData.begin()+stop,
                                    fData.begin()+start, _Tp(0));
      return static_cast<_Op>(temp)/(stop-start) - avg*avg;
    }

    //////////////////////////////////////////////////////////
    // Iterators
    typedef typename VecType::iterator Iter;
    typedef typename VecType::const_iterator CIter;
    Iter  begin()       { return fData.begin(); }
    CIter begin() const { return fData.begin(); }
    Iter  end()         { return fData.end(); }
    CIter end()   const { return fData.end(); }

  public:
    //////////////////////////////////////////////////////////
    // Constructors
    EXOTemplWaveform() : fSampleFreq(CLHEP::megahertz),
                         fTOffset(0.0) 
    {
      // Default constructor
      LoadGlobalLibs();
    }

    EXOTemplWaveform( const _Tp* aData, size_t length ) :
      fSampleFreq(CLHEP::megahertz), fTOffset(0.0) 
    {
      // Constructor to copy array-based data 
      LoadGlobalLibs();
      SetData<_Tp>(aData, length); 
    }

    template<typename _Op>
    EXOTemplWaveform( const EXOTemplWaveform<_Op>&  aWF ) :
      fSampleFreq(aWF.GetSamplingFreq()), 
      fTOffset(aWF.GetTOffset()) 
    {
      // Copy constructor, make a new waveform based on another waveform
      LoadGlobalLibs();
      SetData<_Op>(aWF.GetData(), aWF.GetLength());
    }

    EXOTemplWaveform( const TObject& aWF );

    EXOTemplWaveform<_Tp>& operator=( const TObject& aWF);

    template<typename _Op>
    EXOTemplWaveform<_Tp>& operator=( const EXOTemplWaveform<_Op>&  aWF ) 
    {
      // Assignment operator, make a new waveform based on another waveform
      if ((void*)this == (void*)&aWF) return *this;
      SetData(aWF.GetData(), aWF.GetLength());
      MakeSimilarTo(aWF); 
      return *this;
    }

    virtual ~EXOTemplWaveform() {}
 
    //////////////////////////////////////////////////////////
    // Visualization
    virtual TH1D* GimmeHist(const std::string& label="", Option_t* opt = "") const;
    virtual void LoadIntoHist(TH1D& hist, Option_t* opt = "") const;

    //////////////////////////////////////////////////////////
    // Conversion
    template<typename _Op>
    EXOTemplWaveform<_Op> Convert() const
    {
      EXOTemplWaveform<_Op> wf(*this);
      return wf;
    }
    void ConvertFrom(const TObject& aWF);

    EXOTemplWaveform<_Tp> SubWaveform(size_t begin_ = 0, size_t end_ = (size_t)-1) const;
    void Append(const EXOTemplWaveform<_Tp>& wf); 
    EXOTemplWaveform<_Tp> operator()(size_t begin_, size_t end_) const
    { 
      return SubWaveform(begin_, end_); 
    }
    


    //////////////////////////////////////////////////////////
    // Interpolation functions
    virtual _Tp InterpolateAtPoint( Double_t time ) const;  
    virtual void Refine(EXOTemplWaveform<_Tp>& aWF) const; 

    //////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////
    // Time functions
    virtual size_t GetIndexAtTime(Double_t Time) const;
    virtual Double_t GetTimeAtIndex(size_t Index) const;

  protected:
    VecType   fData;              // data  : Waveform data 
    Double_t  fSampleFreq;        // Sampling frequency
    Double_t  fTOffset;           // Time offset 

  private:
    void LoadGlobalLibs();

  ClassDefT(EXOTemplWaveform,3)

};

typedef EXOTemplWaveform<Double_t> EXODoubleWaveform; 
typedef EXOTemplWaveform<Int_t>    EXOIntWaveform;
typedef EXOTemplWaveform<Float_t>  EXOFloatWaveform;
typedef EXOTemplWaveform<size_t>   EXOLocationWaveform;   // Should not be saved because size_t is system dependent
typedef EXOTemplWaveform<Char_t>   EXOBoolWaveform;   
typedef EXOTemplWaveform<std::complex<double> >  EXOCmplDblWaveform;

//_______________________________________________________________________________
template <typename _Tp>
size_t EXOTemplWaveform<_Tp>::GetIndexAtTime(Double_t Time) const
{
  // Return the index of the element corresponding to time Time.  If Time lies
  // between two elements of the waveform, we return the index of the earlier
  // one.  If Time does not lie within the waveform, log an error and return
  // GetLength().
  if(Time < GetTOffset()) {
    LogEXOMsg("Input time precedes the waveform.  Returning GetLength().", EEError);
    return GetLength();
  }
  if(Time >= GetSamplingPeriod()*GetLength() + GetTOffset()) {
    LogEXOMsg("Input time follows the waveform.  Returning GetLength().", EEError);
    return GetLength();
  }
  return static_cast<size_t>( (Time - GetTOffset())*GetSamplingFreq() );
}

//_______________________________________________________________________________
template <typename _Tp>
inline
Double_t EXOTemplWaveform<_Tp>::GetTimeAtIndex(size_t Index) const
{
  // Return the time corresponding to the element at index Index.  If Index >
  // GetLength(), log an error and return the time that would correspond to
  // index GetLength().
  if(Index > GetLength()) {
    LogEXOMsg("Input index follows the waveform.  "
              "Returning a time that follows the waveform as well.", EEError);
    return GetSamplingPeriod()*GetLength() + GetTOffset();
  }
  return GetSamplingPeriod()*Index + GetTOffset();
}

//_______________________________________________________________________________
template <typename _Tp>
void EXOTemplWaveform<_Tp>::Refine(EXOTemplWaveform<_Tp>& aWF) const
{
  // Refine this waveform into passed-in waveform.  We interpolate this
  // waveform to fit the requirements of the passed-in waveform (length,
  // sampling frequency) where we have interpolated between points (by calling
  // InterpolateAtPoint).  The properties of aWF (length, sampling frequency)
  // should of course be set before calling this function.  
  
  for(size_t i = 0; i < aWF.GetLength(); i++) {
    aWF[i] = InterpolateAtPoint(aWF.GetTimeAtIndex(i));
  }

}

//______________________________________________________________________________
template<typename _Tp>
inline
_Tp EXOTemplWaveform<_Tp>::InterpolateAtPoint( Double_t time ) const
{
  // Linear interpolation function between two points.  This function returns
  // the value of the first point if the requested time is before the fTOffset
  // or the value of the last point if it's beyond the length of the waveform.
  // If the waveform has 0 length, the function returns 0.
  if ( GetLength() == 0 ) return _Tp(0);
  Double_t frac_entry = ( (time - fTOffset)*fSampleFreq );
  if (frac_entry < 0) return operator[](0);

  size_t entry = static_cast<size_t>(frac_entry);
  if ( entry >= GetLength()-1 ) return operator[](GetLength()-1);

  // Now do linear interpolation
  frac_entry -= entry;
  return (_Tp)((1.0-frac_entry)*operator[](entry) + frac_entry*operator[](entry+1));

}

//_______________________________________________________________________________
class EXOWaveformRegion {
  public:
    size_t beginning;
    size_t end;
    EXOWaveformRegion() : beginning(0), end(0) {}
    EXOWaveformRegion(size_t aBeg, size_t anEnd) : beginning(aBeg), end(anEnd) {}
    bool IsInRegion(size_t test) { return (test >= beginning && test <= end); }
    bool operator==(const EXOWaveformRegion& other) const {
      return beginning == other.beginning and end == other.end;
    }
    bool operator!=(const EXOWaveformRegion& other) const {return not (*this == other);}
};

#endif /* EXOTemplWaveform_hh */
