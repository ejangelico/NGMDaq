#include "NGMBlockMultipleScattering.h"
#include "NGMHit.h"
#include "NGMLogger.h"

ClassImp(NGMBlockMultipleScattering)

NGMBlockMultipleScattering::NGMBlockMultipleScattering()
{
    _maxtimediff_ns=100.0;
    _minDistanceInPixels = 14.0;
    _rejectNearestNeighborsBlocksOnly = false;
}

 NGMBlockMultipleScattering::~NGMBlockMultipleScattering(){
    NGMHit* thit = 0;
    while( (thit = nextHit(-1.0))){
        delete thit;
    }
}


void NGMBlockMultipleScattering::clear(){
  while(!_hitbuffer.empty()){
    NGMHit* retVal = _hitbuffer.front();
    _hitbuffer.pop_front();
    delete retVal;
  }
}

bool NGMBlockMultipleScattering::pushHit(NGMHit* hit)
{
    // A few simple but likely cases
    if(_hitbuffer.empty())
    {
        _hitbuffer.push_back(hit);
        return true;
    }
    
    if(hit->TimeDiffNanoSec(_hitbuffer.back()->GetNGMTime())>=0.0)
    {
      
      // Test if hit is a multiple scatter with previous hit
      for(std::list<NGMHit*>::iterator itr = _hitbuffer.begin();
          itr!=_hitbuffer.end(); itr++){
        if(_rejectNearestNeighborsBlocksOnly){
          if(_bmap.IsNeighboringBlock((*itr)->GetPixel(),hit->GetPixel())
             && hit->TimeDiffNanoSec((*itr)->GetNGMTime())<_maxtimediff_ns)
            return false;
        }else{
          if(_bmap.GetDistanceInPixels((*itr)->GetPixel(),hit->GetPixel()) <_minDistanceInPixels
             && hit->TimeDiffNanoSec((*itr)->GetNGMTime())<_maxtimediff_ns)
            return false;
        }
      }
      _hitbuffer.push_back(hit);
    }else{
      LOG<<" DATA NOT SORTED "<<ENDM_FATAL;
    }
  return true;
}

NGMHit* NGMBlockMultipleScattering::nextHit(double maxtimediff_ns)
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

NGMHit* NGMBlockMultipleScattering::nextHit()
{
    return nextHit(_maxtimediff_ns);
}

bool NGMBlockMultipleScattering::empty(double maxtimediff_ns)
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

bool NGMBlockMultipleScattering::empty()
{
    return empty(_maxtimediff_ns);
}

ClassImp(NGMBlockMultipleScatteringModule)


NGMBlockMultipleScatteringModule::NGMBlockMultipleScatteringModule()
{
  InitCommon();
}

NGMBlockMultipleScatteringModule::NGMBlockMultipleScatteringModule(const char* name, const char* title)
: NGMModule(name,title)
{
  InitCommon();
}

NGMBlockMultipleScatteringModule::~NGMBlockMultipleScatteringModule()
{
}

void NGMBlockMultipleScatteringModule::Print(Option_t* option) const
{
  NGMModule::Print();
}

void NGMBlockMultipleScatteringModule::InitCommon(){
}

void NGMBlockMultipleScatteringModule::setMaxNanoSecInList(double newVal){
  _ms._maxtimediff_ns = newVal;
}

void NGMBlockMultipleScatteringModule::setMinDistanceInPixels(double newVal){
  _ms._minDistanceInPixels = newVal;
}
void NGMBlockMultipleScatteringModule::setRejectNearestNeighborBlocksOnly(bool newVal)
{
  _ms._rejectNearestNeighborsBlocksOnly=newVal;
}

bool NGMBlockMultipleScatteringModule::processConfiguration(const NGMSystemConfiguration* conf)
{
  _ms._bmap.Init(conf);
  _ms.clear();
  push(*((const TObject*)(conf)));
  return true;
}

bool  NGMBlockMultipleScatteringModule::processMessage(const TObjString* mess)
{
  push(*((const TObject*)(mess)));
  return true;
}

bool  NGMBlockMultipleScatteringModule::processHit(const NGMHit* tHit)
{
    NGMHit* hit = _hitPool.GetHit();
    hit->CopyHit(tHit);
    if(!_ms.pushHit(hit))
      _hitPool.ReturnHit(hit);
    NGMHit* nexthit = 0;
    while( (nexthit =_ms.nextHit()) )
    {
      push(*((const TObject*)(nexthit)));
      _hitPool.ReturnHit( nexthit);
    }
  return true;
}

