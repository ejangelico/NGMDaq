#ifndef __NGMBASELINE_H__
#define __NGMBASELINE_H__

#include "TObject.h"
class NGMBaseline : public TObject
{
public:
    NGMBaseline();
    virtual ~NGMBaseline();
    NGMBaseline(double gatelength, int minentries, double sigthresh);
    bool AnaBaseline(int bgate);
    void Reset();
    bool push(int nextVal);
    double Avg() const;
    bool MinHere() const;
    double RMS() const;
    void Print(Option_t* option = "") const;
    double _gatelength;
    ULong64_t _minentries;
    ULong64_t _totalEntries;
    ULong64_t _baseSum;
    ULong64_t* _bgate; //! Instruct ROOT not to ignore heap allocated array in the streamer
    double _bcut;
    ULong64_t _consecutiveRejects;
    ULong64_t _maxConsecutiveRejects;
    ULong64_t _totalRejects;
    
    ClassDef(NGMBaseline,2)
};

#endif