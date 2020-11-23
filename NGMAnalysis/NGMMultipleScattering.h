#ifndef __NGMMULTIPLESCATTERING_H__
#define __NGMMULTIPLESCATTERING_H__


/*
 *  NGMMultipleScattering.h
 *  NGMDaq
 *
 *  Created by Jerome Verbeke on 09/26/08.
 *  Copyright 2007 LLNL. All rights reserved.
 *
 */

#include "NGMModule.h"
#include "TTimeStamp.h"
#include "TText.h"
#include "NGMHit.h"
#include <deque>

class NGMBufferedPacket; // forward declaration
class TH1; // forward declaration
class TH2; // forward declaration
class TGraph; // forward declaration
class NGMConfigurationTable; //forward declaration
class NGMSystemConfiguration; //forward declaration
class TList; //forward declaration
class TClonesArray; // forward declaration
class TTree; // forward declaration
class TCanvas; // forward declaration
class NGMZoomGui; // forward declaration
class TLegend; // forward declaration

struct rejection_st {
  TString* _det1;
  TString* _det2;
  float _mindistance;
  float _maxdistance;
};

struct detname2index_st {
  TString* _detname;
  int _index;
  int _length;
};

class NGMMultipleScatteringHit: public TObject {
public:
  NGMMultipleScatteringHit();
  NGMMultipleScatteringHit(NGMHit* tHit, bool value);
  virtual ~NGMMultipleScatteringHit();

  NGMHit* getHit();
  bool getKeep();
  NGMMultipleScatteringHit* getClosestNeighbour();
  bool isWithChild();

  NGMHit* _hit;
  bool _keep;
  NGMMultipleScatteringHit* _closestNeighbourHit;
  bool _isWithChild;

  ClassDef(NGMMultipleScatteringHit,2);
};

class NGMMultipleScattering: public NGMModule{
public:
  NGMMultipleScattering();
  NGMMultipleScattering(const char*, const char*);
  virtual ~NGMMultipleScattering();
  
  bool  init();
  bool  process(const TObject&);
  bool  finish();
  void  SetUseGeometry(bool useGeo  = true) {_useGeo = useGeo; } 
  bool  analyzeHit(const NGMHit* tHit);
  void enableNearestNeighbourRejection(bool enabled=true);
// This method drops all the hits of a burst that are in neighboring cells
// and within a certain time of each other). It is enabled by default

  void addRejectionEntry(TString det1, TString det2, float dt);
// Deprecated, to be used for fast neutrons only
// Does not work with setSpeed().
// Any count in det2 that is followed by a count in det1 within a time 
// interval dt (in nanoseconds) will be rejected. 
// Two entries must be entered if you want symmetry:
// addRejectionEntry("LS0024", "LS0001", 12.)
// addRejectionEntry("LS0001", "LS0024", 12.)
// This method was initially implemented for fast neutrons travelling 
// at 14,000 km/sec (1 MeV). Any fast neutron going from det2 to det1
// within dt will not be recorded in det1.
  
  // Add Rejection criteria derived from geometry information
  void InitFromGeometry(const NGMSystemConfiguration* sysConf);
  
  void SetMaximumDistanceForInitFromGeometry(double distance_cm) { _maximumDistanceForInitFromGeometry = distance_cm; }
  
  void addDistanceEntry(TString det1, TString det2, TString dettype, float mindistance, float maxdistance);
// Any count in det2 that is followed by a count in det1 within some time 
// interval dt will be rejected. The boundaries of the time interval are 
// determined by the ratios of the minimum and maximum distances to the 
// speed of each particle. The distances must be given in centimeters.
// Two entries must be entered if you want symmetry:
// addDistanceEntry("LS0024", "LS0001", "5:6", 2., 12.)
// addDistanceEntry("LS0001", "LS0024", "5:6", 2., 12.)
// In the example, the entry is given for 2 detected particle types,
// lsgamma and lsneutron. Detector types must be separated by ':'
 
   void setSpeed(int parttype, float minspeed, float maxspeed);
// To set the speed of particles in m/s.
// See NGMParticleIdent.h for parttype
//  gbgamma=1
//  gbmuon=2
//  mbgamma=3
//  mbmuon=4
//  lsgamma=5
//  lsneutron=6
//  lsmuon=7
//  hettlid=8
//  heid=9
// The speed of light is used for gammas by default.
// A minimum speed of 14,000,000 m/sec is used for fast neutrons 
// in the liquid scintillators by default. This speed corresponds 
// to 1 MeV neutrons, they can not be detected by liquid scintillators 
// at lower energies.

   bool enable(int parttype);
   bool disable(int parttype);
// enables/disables the multiple scattering filter for particle type parttype
// all particles but the helium tube related ones are enabled by default

   void setNeutron2GammaSuppression(bool option=true);
   bool getNeutron2GammaSuppression();

// When there are different possible neighbouring detectors in the vicinity, 
// from which a particle could have scattered, the closest one is picked
// when the following option is set.
   void setSeekClosestNeighbour(bool option=true);
   bool getSeekClosestNeighbour();

private:

  void addEntry(TString det1, TString det2, int dettype, float mindistance, float maxdistance);
  NGMMultipleScatteringHit* inTheNeighbourhood(NGMHit* tHit);
  void pushList(const NGMHit* tHit);
  void compute_deltans();
  int getDesiredIndex(TString detname, int dettype);
  int getFirstIndex(TString* detname, int dettype);
  int getLength(TString* detname, int dettype);
  int getMapIndex(int index, int dettype);
  int getMapIndex(TString* detname, int dettype);
  void setmap(TString detname, int ptype);
  void resetmap(TString detname, int ptype);
  void altermap(TString detname, int dettype);
  void ordermap();
  void computetimeintervals();
  void swap(int index1, int index2, int ptype);
  void printmap(int dettype);
  int maxListSize; // largest number of points displayed
  TList* pList; // list of hits recorded
  NGMTimeStamp* firstTime;
  NGMTimeStamp _runBegin;
  bool nearestNeighbourRejection;
  bool _useGeo;
  
  double _maximumDistanceForInitFromGeometry;  
  
  enum localconsts {maxparttypes = 11, maxScat=4};
// The following array will keep track of neighbours
  rejection_st* _reject[maxparttypes+1]; //!
  int _reject_current_index[maxparttypes+1];
  int _reject_size[maxparttypes+1];

  detname2index_st* _detname_map[maxparttypes+1]; //!
  int _detname_current_index[maxparttypes+1];
  int _detname_map_size[maxparttypes+1];

  bool method2;
  bool mapready;
  float largestdistance[maxparttypes+1];
  bool timeintervalsready;
  float maxlargesttimeinterval;

// These variables are used to store the speed of the different 
// particle types
  float minspeed[maxparttypes+1]; // stored in cm/nsec
  float maxspeed[maxparttypes+1]; // stored in cm/nsec

  bool enabledparticle[maxparttypes+1];
  // true if filtering applies to this particle
  // class statics must be initialized in implementation
  static const float cspeed; // speed of light in cm/nanosec.

  bool neutron2GammaSuppression;
  bool seekClosestNeighbour;

  int _rejectedCounts; // number of rejected counts

  ClassDef(NGMMultipleScattering,2);
};

#endif // __NGMMULTIPLESCATTERING_H__
