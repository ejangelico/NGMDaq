#ifndef __NGMTIMINGCAL_H__
#define __NGMTIMINGCAL_H__
#include "TObject.h"
#include <list>
#include <vector>
#include "NGMHit.h"
#include "NGMParticleIdent.h"
// forward declaration
//class NGMHit;

class NGMTimingCal : public TObject
{
public:
    NGMTimingCal();
    virtual ~NGMTimingCal();
    void SetNChannels(int nchannels);
    void SetOffset(int chan, double offset);
    void pushHit(NGMHit* hit);
    void clear();
    bool empty();
    bool empty(double maxtimediff_ns);
    NGMHit* nextHit();
    NGMHit* nextHit(double maxtimediff_ns);
    int _nchannels;
    double _maxtimediff_ns;
    std::vector<double> _timingOffset;
    std::list<NGMHit*> _hitbuffer;
    NGMParticleIdent* _partId;
    
    ClassDef(NGMTimingCal,2)
};

#endif // __NGMTIMINGCAL_H__
