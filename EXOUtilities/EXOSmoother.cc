//______________________________________________________________________________
// EXOSmoother
//
// DESCRIPTION: 
//
// A class handling the smoothing of a waveform.
//
// AUTHOR: M. Marino
// CONTACT: 
// FIRST SUBMISSION: 
// 
// REVISION:
// 30 Aug 2010 Added get method for fSmoothSize, A. Schubert
// 11 Apr 2012 Ported from MGDO to EXOUtil by M. Marino
// 
//______________________________________________________________________________

#include "EXOSmoother.hh"
#include "EXOErrorLogger.hh"

EXOSmoother::EXOSmoother( size_t aSmoothSize, const std::string& aName ) : 
  EXOVWaveformTransformer( aName ),
  fSmoothSize(aSmoothSize)
{
}

void EXOSmoother::TransformOutOfPlace(const EXODoubleWaveform& anInput, EXODoubleWaveform& anOutput) const
{
  if( anInput.GetLength() <= 1 ) {
    LogEXOMsg(" anInput of length zero or one", EEError);
    return;
  }

  // Smooth fSmoothRegion, or the whole waveform if fSmoothRegion is still the default.
  // Don't modify fSmoothRegion directly, or this function won't be thread-safe.
  EXOWaveformRegion RegionToSmooth(0, anInput.GetLength()-1);
  if(fSmoothRegion != EXOWaveformRegion()) RegionToSmooth = fSmoothRegion;

  if(RegionToSmooth.end <= RegionToSmooth.beginning) {
    LogEXOMsg( "Smoothing variables set incorrectly.  Please Check.", EEError);
    return; 
  }
  // Calls the (possibly) overloaded function giving it the appropriate
  // variables.
  anOutput = anInput;
  SmoothData( anInput.GetVectorData(),
              anOutput.GetVectorData(),
              RegionToSmooth);
}

void EXOSmoother::SmoothData( const std::vector<double>& input, 
                               std::vector<double>& output, 
                               const EXOWaveformRegion region ) const
{
  // We do a common triangular smooth and the smooth size shrinks as it gets
  // closer to the edge of the waveform.
 
  size_t place = region.beginning;
  while (  place <= region.end ) {
    size_t amountToTheLeft  =  (place >= fSmoothSize) ? fSmoothSize : place;
    size_t amountToTheRight =  (input.size() - 1 - place >= fSmoothSize) ? 
                               fSmoothSize : input.size() - 1 - place;
 
    size_t smoothing = ( amountToTheRight < amountToTheLeft ) ? amountToTheRight : amountToTheLeft;
    TriangularSmoothData( input, output, place, smoothing ); 
    place++;
  }
}

void EXOSmoother::TriangularSmoothData( const std::vector<double>& input, 
                                         std::vector<double>& output, 
                                         size_t offset, size_t smoothLength ) const
{
  double divideBy = 
    static_cast<double>( (1+smoothLength)*(1+smoothLength) ); 
  output[offset] = input[offset]*(smoothLength + 1);
  for(size_t j=1;j<=smoothLength;j++) {
    output[offset] += (smoothLength + 1 - j)*((input[offset+j]) + (input[offset-j])); 
  } 
  output[offset] /= divideBy;
}

