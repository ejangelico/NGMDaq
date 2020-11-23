/**                                                            
 *      
 * CLASS DECLARATION:  EXOVWaveformTransformer.hh
 *
 * DESCRIPTION: 
 *
 * Abstract class handling transformations, processing of waveforms
 *
 * AUTHOR: M. Marino
 * CONTACT: 
 * FIRST SUBMISSION: 
 * 
 * REVISION:
 * 
 */

#ifndef _EXOVWaveformTransformer_HH
#define _EXOVWaveformTransformer_HH

#include <string> 
#include "EXOTemplWaveform.hh" 

class EXOVWaveformTransformer
{
  public:
    // We require the derived classes to define their names since this is an important way to distinguish between
    // them.  
    EXOVWaveformTransformer( const std::string& aTransformationName ) :
      fName(aTransformationName)
      { } 
    virtual ~EXOVWaveformTransformer() { }
  
    virtual bool IsInPlace() const = 0;
    virtual bool IsOutOfPlace() const { return !IsInPlace(); }

    virtual void Transform(EXODoubleWaveform* input, EXODoubleWaveform* output = NULL) const;
    const std::string& GetStringName() const { return fName; }
    const char* GetName() const { return fName.c_str(); }
    
  protected:
    virtual void TransformInPlace(EXODoubleWaveform& input) const;
    virtual void TransformOutOfPlace(const EXODoubleWaveform& input, EXODoubleWaveform& output) const;
  
  private:
    // Make the default constructor private to force usage of the other constructor.
    EXOVWaveformTransformer();

    std::string fName; // Name of the transformation class.
    mutable EXODoubleWaveform fTmpWaveform; // Temporary waveform, use only by base class. 
};

#endif
