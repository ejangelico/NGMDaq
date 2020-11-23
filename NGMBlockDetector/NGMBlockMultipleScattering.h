#ifndef __NGMBlockMultipleScattering_H__
#define __NGMBlockMultipleScattering_H__
#include "TObject.h"
#include <list>
#include <vector>
#include "NGMHit.h"
#include "NGMParticleIdent.h"
#include "NGMBlockMapping.h"
#include "NGMModule.h"

class NGMBlockMultipleScattering : public TObject
{
public:
    NGMBlockMultipleScattering();
    virtual ~NGMBlockMultipleScattering();
    bool pushHit(NGMHit* hit);
    void clear();
    bool empty();
    bool empty(double maxtimediff_ns);
    NGMHit* nextHit();
    NGMHit* nextHit(double maxtimediff_ns);
    double _maxtimediff_ns;
    std::list<NGMHit*> _hitbuffer;
    NGMBlockMapping _bmap;
    double _minDistanceInPixels;
    bool _rejectNearestNeighborsBlocksOnly;
  
    ClassDef(NGMBlockMultipleScattering,2)
};

class NGMBlockMultipleScatteringModule : public NGMModule
{
public:
  NGMBlockMultipleScatteringModule();
  NGMBlockMultipleScatteringModule(const char* name, const char* title);
  virtual ~NGMBlockMultipleScatteringModule();
  bool init(){ return true; }
  bool finish(){ return true; }
  void Print(Option_t* option = "") const;
  void setMaxNanoSecInList(double newVal); // *MENU*
  void setMinDistanceInPixels(double newVal); // *MENU*
  void setRejectNearestNeighborBlocksOnly(bool newVal = true);
  
private:
  bool processConfiguration(const NGMSystemConfiguration* conf);
  bool  processHit(const NGMHit* tHit);
  bool  processMessage(const TObjString* mess);
  void InitCommon();
  NGMBlockMultipleScattering _ms; //!
  NGMHitPoolv6 _hitPool; //!
  
  ClassDef(NGMBlockMultipleScatteringModule,1)
};


#endif // __NGMBlockMultipleScattering_H__
