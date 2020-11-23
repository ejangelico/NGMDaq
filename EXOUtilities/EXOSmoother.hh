#ifndef _EXOSmoother_HH
#define _EXOSmoother_HH

#include "EXOVWaveformTransformer.hh"

class EXOSmoother : public EXOVWaveformTransformer 
{
  public:
    // This transform smooths waveform data.  It employs a normal
    // triangular smoother.  One can overload the SmoothData() function to
    // easily implement a different smoothing algorith.  The size of the
    // smoothing is controlled by the fSmoothSize member variable set by the
    // SetSmoothSize() function. 
    //
    EXOSmoother( size_t aSmoothSize = 1, 
      const std::string& name = "EXOSmoother" ); 
    virtual ~EXOSmoother() {}
    
    virtual bool IsInPlace() const { return false; }

    ///* Smooth this region in the waveform. */
    virtual void SetSmoothRegion( const EXOWaveformRegion& aVal ) { fSmoothRegion = aVal; }

    ///* Sets the characteristic length of the smoothing function. */
    virtual void SetSmoothSize( size_t aVal ) { fSmoothSize = aVal; }
    virtual inline size_t GetSmoothSize() const { return fSmoothSize; }
  
  protected:
    virtual void TransformOutOfPlace(const EXODoubleWaveform& anInput, EXODoubleWaveform& anOutput) const;
    // The following function is the workhorse of the processor.  The
    // SmoothData() function smooths an array of double over a particular
    // length.
    virtual void SmoothData( const std::vector<double>& input, 
                             std::vector<double>& output, 
                             const EXOWaveformRegion region ) const;

    virtual void TriangularSmoothData( const std::vector<double>& input, 
                             std::vector<double>& output, 
                             size_t offset, size_t smoothLength ) const;

    EXOWaveformRegion fSmoothRegion;
    size_t fSmoothSize;
};

#endif
