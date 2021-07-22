#include "NGMTimingCal.h"
#include "NGMHit.h"
#include "TObjString.h"

bool NGMTimingCalHitCompare(NGMHit* first, NGMHit* second)
{
    return (second->TimeDiffNanoSec(first->GetNGMTime())>0.0);
}

NGMTimingCal::NGMTimingCal()
{
    _nchannels=0;
    _maxtimediff_ns=100.0;
    _partId=0; // Not owned by this object

}

 NGMTimingCal::~NGMTimingCal(){
    NGMHit* thit = 0;
    while( (thit = nextHit(-1.0))){
        delete thit;
    }
}


void NGMTimingCal::SetNChannels(int nchannels)
{
    _nchannels=nchannels;
    _timingOffset.resize(_nchannels,0.0);
}

void NGMTimingCal::SetOffset(int chan, double offset)
{
    _timingOffset[chan]=offset;
}

void NGMTimingCal::clear(){
  while(!_hitbuffer.empty()){
    NGMHit* retVal = _hitbuffer.front();
    _hitbuffer.pop_front();
    delete retVal;
  }
}

void NGMTimingCal::pushHit(NGMHit* hit)
{
    int chanSeq = hit->GetPixel();
    if(_partId) chanSeq = _partId->getPlotIndex(hit);
    if(_timingOffset.size()>0)
        hit->GetNGMTime().IncrementNs(_timingOffset[chanSeq]);
    // A few simply but likely cases
    if(_hitbuffer.empty())
    {
        _hitbuffer.push_back(hit);
        return;
    }
    
    if(hit->TimeDiffNanoSec(_hitbuffer.back()->GetNGMTime())>=0.0)
    {
        _hitbuffer.push_back(hit);
        return;
    }
    
    //We'll let the list merge method insert at the right location
    
    std::list<NGMHit*> tmpList;
    tmpList.push_back(hit);
    _hitbuffer.merge(tmpList,NGMTimingCalHitCompare);

}

NGMHit* NGMTimingCal::nextHit(double maxtimediff_ns)
{
    NGMHit* retVal = 0;
    if(!_hitbuffer.empty()
       &&(maxtimediff_ns<0.0
           || _hitbuffer.back()->TimeDiffNanoSec(_hitbuffer.front()->GetNGMTime())>maxtimediff_ns)
       )
    {
        retVal = _hitbuffer.front();
        _hitbuffer.pop_front();
    }
    return retVal;
}

NGMHit* NGMTimingCal::nextHit()
{
    return nextHit(_maxtimediff_ns);
}

bool NGMTimingCal::empty(double maxtimediff_ns)
{
    bool retVal = true;
    if(!_hitbuffer.empty()
       &&(maxtimediff_ns<0.0
          || _hitbuffer.back()->TimeDiffNanoSec(_hitbuffer.front()->GetNGMTime())>maxtimediff_ns)
       )
    {
        retVal = false;
    }
    return retVal;
}


bool NGMTimingCal::empty()
{
    return empty(_maxtimediff_ns);
}
