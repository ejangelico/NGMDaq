#ifndef _EXOPoleZeroCorrection_HH
#define _EXOPoleZeroCorrection_HH

#include "EXOVWaveformTransformer.hh"

class EXOPoleZeroCorrection : public EXOVWaveformTransformer 
{
  public:
    EXOPoleZeroCorrection(); 
    virtual ~EXOPoleZeroCorrection();
  
    virtual bool IsInPlace() const { return false; }
    
    /*! Set the decay constant (tau). */
    virtual void SetDecayConstant(double aVal) { fDecayConstant = aVal; }
    virtual inline double GetDecayConstant() const { return fDecayConstant; }
  
    /*! Set the baseline of the wf (asymptote). */
    virtual void SetRestingBaselineValue(double aVal) { fRestingBaselineValue = aVal; }
    virtual inline double GetRestingBaselineValue() const { return fRestingBaselineValue; }
  
  protected:
    virtual void TransformOutOfPlace(const EXODoubleWaveform& anInput, EXODoubleWaveform& anOutput) const;
    double fDecayConstant;
    double fRestingBaselineValue;

};

#endif
