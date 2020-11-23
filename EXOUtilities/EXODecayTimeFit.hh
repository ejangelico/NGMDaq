#ifndef _EXODecayTimeFit_HH
#define _EXODecayTimeFit_HH

#include "EXOVWaveformTransformer.hh"
#include <cstddef> //for size_t

class EXODecayTimeFit : public EXOVWaveformTransformer 
{
  public:
    EXODecayTimeFit(); 
    virtual ~EXODecayTimeFit();
    
    /*! 
       Perform test.  Transform leaves anInput unchanged.  
       */
    virtual bool IsInPlace() const { return true; }


    //! Set the starting position of the Decay time fit
    void SetStartSample( size_t val ) { fStartSample = val; }
    size_t GetStartSample() const { return fStartSample; }

    //! Set the starting position of the Decay time fit
    void SetEndSample( size_t val ) { fEndSample = val; }
    size_t GetEndSample() const { return fEndSample; }

    //! Set the max_val to use as initial Guess
    void SetMaxValGuess(double val) {fMaxValGuess = val;}
    double GetMaxValGuess() const {return fMaxValGuess;}

    //! Set the tau to use as intial guess (Default is 400us)
    void SetTauGuess(double val) {fTauGuess = val;}
    double GetTauGuess() const {return fTauGuess;}
 
    

    //! Returns decaytime with CLHEP units of time
    double GetDecayTime() const { return fDecayTime; }

    //! Returns decaytime error with CLHEP units of time
    double GetDecayTimeError() const { return fDecayTimeError; }

    //! Returns decaytime chi2 with CLHEP units of time
    double GetDecayTimeChi2() const { return fDecayTimeChi2; }
        

  protected:
    virtual void TransformInPlace(EXODoubleWaveform& anInput) const;
    
    size_t fStartSample;
    size_t fEndSample;
    double fMaxValGuess;
    double fTauGuess;

    mutable double fDecayTime;
    mutable double fDecayTimeChi2;
    mutable double fDecayTimeError;
  };

#endif
