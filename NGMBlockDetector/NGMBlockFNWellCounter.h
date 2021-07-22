#ifndef __NGMBLOCKFNWELLCOUNTER_H__
#define __NGMBLOCKFNWELLCOUNTER_H__

#include "NGMModule.h"
#include <list>
#include "NGMHit.h"
#include "NGMParticleIdent.h"
#include "NGMBlockMapping.h"
#include "TTree.h"
#include "TObjString.h"

#include "TH2.h"
#include "TH3.h"


class NGMBlockFNWellCounter_tuple_st : public TObject {
public:
  NGMBlockFNWellCounter_tuple_st(){}
  virtual ~NGMBlockFNWellCounter_tuple_st(){}
  double t;
  double d;
  double e1;
  double e2;
  ClassDef(NGMBlockFNWellCounter_tuple_st,1)
};

class NGMBlockFNWellCounter : public NGMModule
{
public:
  NGMBlockFNWellCounter();
  NGMBlockFNWellCounter(const char* name, const char* title);
  virtual ~NGMBlockFNWellCounter();
  bool init(){ return true; }
  bool finish(){ return true; }
  void Print(Option_t* option = "") const;
  void setMaxNanoSecInList(double newVal); // *MENU*
  double getDuration() const {return _duration;}
  Long64_t getTotalSingles() const { return _totalSingles; }
  int getTotalDoubles(const double maxtime_ns =100.0, const double mindistance_pix = 0 ) const ;
  int getTotalDoubles3d(const double maxtime_ns =100.0, const double mindistance_pix = 0, const double minenergy = 0.0 ) const;
    
  TH2* getDoublesVsDistance() {return _hDoublesVsDistance;}
  TH3* getDoublesVsDistanceVsEmin() {return _hDoublesVsDistanceVsEmin;}
  TH1* getEnergy() {return _energy;}

private:
  
  // Buffered hit list
  bool pushHit(NGMHit* hit);
  void clear();
  bool empty();
  bool empty(double maxtimediff_ns);
  NGMHit* nextHit();
  NGMHit* nextHit(double maxtimediff_ns);

  
  bool  processConfiguration(const NGMSystemConfiguration* conf);
  bool  processHit(const NGMHit* tHit);
  bool  processMessage(const TObjString* mess);
  void InitCommon();
  void CloseTuple();
  
  std::list<NGMHit*> _hitbuffer;
  
  NGMHitPoolv6 _hitPool; //!
  NGMBlockMapping _bmap;
  TH2* _hDoublesVsDistance;
  TH3* _hDoublesVsDistanceVsEmin;
  TH2* _hTimingDiffPerColumn;
  TH1* _energy;
  TH2* _hNEnergyvsTof;
  TH2* _hNEnergyvsTofPrevious;
  double _maxtimediff_ns;
  double _duration;
  Long64_t _totalSingles;
  NGMTimeStamp* _runBeginTime;
  NGMTimeStamp _preceedingGamma;
  NGMBlockFNWellCounter_tuple_st _tbuf; //!
  TTree* _tupleOut = 0; //!
  
  ClassDef(NGMBlockFNWellCounter,6)
};

#endif // __NGMBLOCKFNWELLCOUNTER_H__
