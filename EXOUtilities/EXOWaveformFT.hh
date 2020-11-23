#ifndef EXOWaveformFT_hh
#define EXOWaveformFT_hh

#ifndef EXOTemplWaveform_hh
#include "EXOTemplWaveform.hh" 
#endif

#include <cstddef> //for size_t

class EXOWaveformFT : public EXOCmplDblWaveform {

  public:
    typedef std::complex<double> CDbl;
    EXOWaveformFT() : EXOCmplDblWaveform() {}
    EXOWaveformFT( const EXOWaveformFT& awf ) : 
      EXOCmplDblWaveform(awf) {} 

    EXOWaveformFT( const CDbl* ptr, size_t length ) : 
      EXOCmplDblWaveform(ptr, length) {} 

    EXOWaveformFT& operator=( const EXOWaveformFT& awf )
      { EXOCmplDblWaveform::operator=(awf); return *this; }

    CDbl InterpolateAtPoint( Double_t frequency ) const;

    void LoadIntoHist(TH1D& hist, Option_t* opt) const;
    
    // Disable GetTimeAtIndex
    Double_t GetTimeAtIndex(size_t /*index*/) const { return 0.0; }

    Double_t GetFrequencyAtIndex(size_t index) const;

    void Refine(EXOCmplDblWaveform& aWF) const;

  ClassDef( EXOWaveformFT, 1 )
};

#endif /* EXOWaveform_hh */
