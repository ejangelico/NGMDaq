/*
 *  NGMMultipleScattering.cpp
 *  NGMDaq
 *
 *  Created by Jerome Verbeke on 06/07/07.
 *  Copyright 2007 LLNL. All rights reserved.
 *
 */

#include <math.h>
#include "TH2.h"
#include "NGMLogger.h"
#include "NGMBufferedPacket.h"
#include "NGMSystemConfiguration.h"
#include "NGMMultipleScattering.h"
#include "NGMConfigurationTable.h"
#include "TROOT.h"
#include "TObjString.h"
#include "NGMTimeMonitor.h"
#include "NGMZoomGui.h"
#include "TMath.h"
#include "NGMDisplay.h"
#include "TVector3.h"
#include "NGMParticleIdent.h"
#include "NGMSimpleParticleIdent.h"
#include "TGeoShape.h"
#include "TGeoVolume.h"
#include "TSystem.h"
#include "TGeoNode.h"

ClassImp(NGMMultipleScatteringHit);

const float NGMMultipleScattering::cspeed = 29.9792458; // speed of light in cm/nanosec.

NGMMultipleScatteringHit::NGMMultipleScatteringHit() {
  _hit=NULL;
  _keep=true;
  _closestNeighbourHit=NULL;
  _isWithChild=false;
}

NGMMultipleScatteringHit::NGMMultipleScatteringHit(NGMHit* tHit, bool value) {
  _hit=tHit;
  _keep=value;
  _closestNeighbourHit=NULL;
  _isWithChild=false;
}

NGMMultipleScatteringHit::~NGMMultipleScatteringHit() {
  if (_hit != NULL) delete _hit;
}

NGMHit* NGMMultipleScatteringHit::getHit() {
  return _hit;
}

bool NGMMultipleScatteringHit::getKeep() {
  return _keep;
}

bool NGMMultipleScatteringHit::isWithChild() {
  return _isWithChild;
}

NGMMultipleScatteringHit* NGMMultipleScatteringHit::getClosestNeighbour() {
  return _closestNeighbourHit;
}

ClassImp(NGMMultipleScattering);

NGMMultipleScattering::NGMMultipleScattering()
{
  firstTime = 0;
  nearestNeighbourRejection = true;
  maxListSize = 5000;
  method2 = true;
  mapready = true;
  timeintervalsready = false;
  _useGeo = false;
  _maximumDistanceForInitFromGeometry = 100.0; // cm
  
  // No allocations in default constructor ( rootism ).
  partID = 0;

// Initialize a number of arrays 
  for (int i=1; i<=maxparttypes; i++) {
    enabledparticle[i] = true;
    largestdistance[i] = 0.;

    _reject_size[i]=0;
    _reject_current_index[i]=0;
    _detname_map[i]=0;
    _detname_current_index[i]=0;
    _detname_map_size[i]=0;
    _reject[i] = 0;
  }
  enabledparticle[8] = false;
  enabledparticle[9] = false;

// We assume most particles travel at the speed of light, this is wrong for
// muons and must be changed
  for (int i=1; i<=maxparttypes; i++) {
    minspeed[i] = cspeed; // speed of light in cm/ns
    maxspeed[i] = cspeed;
  }

// lsneutron
  minspeed[NGMSimpleParticleIdent::lsneutron] = 1.4; // 14,000,000 m/sec
  maxspeed[NGMSimpleParticleIdent::lsneutron] = cspeed;  // we assume speed of light for fastest neutrons

// This never applies to hettlid or heid, neutrons are absorbed by He-3, and
// can not scatter to another helium tube.

  neutron2GammaSuppression = false;
  seekClosestNeighbour = false;

  _rejectedCounts=0;
}

NGMMultipleScattering::NGMMultipleScattering(const char* name, const char* title)
: NGMModule(name,title)
{
  pList = new TList;
  firstTime = 0;
  nearestNeighbourRejection = true;
  maxListSize = 5000;
  method2 = true;
  mapready = true;
  timeintervalsready = false;
  _useGeo = false;
  _maximumDistanceForInitFromGeometry = 100.0; // cm
  
  partID = new NGMSimpleParticleIdent;

// Initialize a number of arrays 
  for (int i=1; i<=maxparttypes; i++) {
    enabledparticle[i] = true;
    largestdistance[i] = 0.;

    _reject_size[i]=0;
    _reject_current_index[i]=0;

    _detname_map[i]=0;
    _detname_current_index[i]=0;
    _detname_map_size[i]=0;
    _reject[i] = 0;
  }
  enabledparticle[8] = false;
  enabledparticle[9] = false;

// We assume most particles travel at the speed of light, this is wrong for
// muons and must be changed
  for (int i=1; i<=maxparttypes; i++) {
    minspeed[i] = cspeed; // speed of light in cm/ns
    maxspeed[i] = cspeed;
  }

// lsneutron
  minspeed[NGMSimpleParticleIdent::lsneutron] = 1.4; // 14,000,000 m/sec
  maxspeed[NGMSimpleParticleIdent::lsneutron] = cspeed;  // speed of light for fastest neutrons

// This never applies to hettlid or heid, neutrons are absorbed by He-3, and
// can not scatter to another helium tube.

  neutron2GammaSuppression = false;
  seekClosestNeighbour = false;

  _rejectedCounts=0;
}

NGMMultipleScattering::~NGMMultipleScattering(){
  delete partID;
  if(pList)
   pList->Delete();
  delete pList;

}

bool NGMMultipleScattering::init(){
  if(!partID) partID =  new NGMSimpleParticleIdent;
  return true;
}

bool NGMMultipleScattering::process(const TObject &tData){
  
  static TClass* tNGMHitType = gROOT->GetClass("NGMHit");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  static TClass* tNGMBufferedPacketType = gROOT->GetClass("NGMBufferedPacket");
  static TClass* tObjStringType = gROOT->GetClass("TObjString");

  // Check if the map needs to be ordered
  if (method2 && !mapready) ordermap();
  if (!timeintervalsready) computetimeintervals();

/*
  printmap();
  exit(0);
*/

  //Check data type
  if(tData.InheritsFrom(tNGMBufferedPacketType)){
     // We assume this packet is probably a burst slotid == -1
     const NGMBufferedPacket* packetBuffer = (const NGMBufferedPacket*)(&tData);
     if(packetBuffer->getSlotId()!=-1) {
        LOG<<" Not expecting a raw packet.  Should be a burst packet "<<ENDM_WARN;
        return false;
     }
     if(firstTime == 0 && packetBuffer->getPulseCount()>0) {
       firstTime = new NGMTimeStamp(((NGMHit *)(packetBuffer->getHit(0)))->GetNGMTime());
     }
     for(int ipart = 0; ipart < packetBuffer->getPulseCount(); ipart++)
     {
        const NGMHit* tHit = packetBuffer->getHit(ipart);
        if(partID != 0 && partID->IsSelected(tHit)) analyzeHit(tHit);
        pushList(tHit);
     }
  }else if(tData.InheritsFrom(tNGMHitType)){
    const NGMHit* tHit = (const NGMHit*)(&tData);
    if(firstTime == 0) {
      firstTime = new NGMTimeStamp(tHit->GetNGMTime());
      if(partID != 0 && partID->IsSelected(tHit)){
        // We can push the first one no matter what
        analyzeHit(tHit);
        push(tData);
      }
    } else {
      if(partID != 0 && partID->IsSelected(tHit)) {        
        analyzeHit(tHit);
      }
      pushList(tHit);
    }
  } else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    LOG<<"Inherits from NGMSystemConfiguration"<<ENDM_INFO;
    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
    partID->Init(confBuffer);
    _runBegin = NGMTimeStamp(confBuffer->GetTimeStamp());
    delete firstTime;
    firstTime = 0;
    firstTime = new NGMTimeStamp(_runBegin);
    if(pList)
      pList->Delete();

    TString srunnumber;
    srunnumber+=confBuffer->getRunNumber();
    TString passname = confBuffer->GetSystemParameters()->GetParValueS("PassName",0);
    
    if(GetAllowDatabaseReads())
    {
      GetRunInfoFromDatabase(srunnumber.Data());
    }else{
      _facility = "";
      _series = "";
      _experimentname = "";
      if(confBuffer->GetSystemParameters()->GetParIndex("ExperimentName")>=0)
      {
       _experimentname = confBuffer->GetSystemParameters()->GetParValueS("ExperimentName",0);

      }
    }


    
    // Initialize Rejection Maps from Geometry
    if(_useGeo)
    {
      LOG<<" Using "<<_geoMacroName.Data()<<ENDM_INFO;
      
      InitFromGeometry(confBuffer);
      computetimeintervals();

    }
    
    push(tData);
  } else {
    if (tData.IsA() == tObjStringType) {
      const TObjString* controlMessage = (const TObjString*) (&tData);
      if (controlMessage->GetString() == "EndRunSave") {
        std::cout<<GetName()<< " suppressed " << _rejectedCounts << " hits." << std::endl;
      }
    }
    push(tData);
  }

  return true;  
}

bool NGMMultipleScattering::finish(){

  return true;
}

bool NGMMultipleScattering::analyzeHit(const NGMHit* thisHit){

  NGMHit* tHit = thisHit->DuplicateHit();
    
  if (pList == 0) pList = new TList();

  // Adding points to the list until we get maxListSize points
  if (pList->GetSize() >= maxListSize) {
    // When list is full, remove the first element and add to the end
     TObject* hitToRemove = pList->First();
     pList->RemoveFirst();
     delete hitToRemove;
  }

  if (pList->IsEmpty()) {
    pList->Add(new NGMMultipleScatteringHit(tHit, true));
  } else {
    if (!nearestNeighbourRejection) {
      pList->Add(new NGMMultipleScatteringHit(tHit, true));
    } else { // nearest neighbour rejection is on
      NGMMultipleScatteringHit* priorMScatHit;
      if ((priorMScatHit = inTheNeighbourhood(tHit)) == NULL) {
        pList->Add(new NGMMultipleScatteringHit(tHit, true));
      } else {
        // We keep the fast neutron but not we do not display it
        NGMMultipleScatteringHit* currentMScatHit = new NGMMultipleScatteringHit(tHit, false);
        pList->Add(currentMScatHit);
        // This hit was a scattered hit off priorMScatHit, check whether 
        // the prior hit priorMScatHit was also itself scattered hit 
        // (recursively)
	//        int level=0;
//        if (priorMScatHit != NULL)  std::cerr << "NGMMultipleScattering::pushList " << partID->GetName(currentMScatHit->getHit()) << " " << currentMScatHit->getHit()->TimeDiffNanoSec(*firstTime)+4.6819492e+04 << " " << partID->GetName(currentMScatHit->getHit()); 
        while (priorMScatHit != NULL) {
          // The energy of this particle is added to the energy of the 
          // previous neighboring particle
//          std::cerr << "<-" << partID->GetName(priorMScatHit->getHit()); 
//          std::cerr << "NGMMultipleScattering::analyzeHit, previous energy in prior " << partID->GetName(priorMScatHit->getHit()) << " = " << priorMScatHit->getHit()->GetEnergy() << std::endl;
//          std::cerr << "NGMMultipleScattering::analyzeHit, previous energy in current " << partID->GetName(currentMScatHit->getHit()) << " = " << currentMScatHit->getHit()->GetEnergy() << std::endl;
          priorMScatHit->getHit()->SetEnergy(priorMScatHit->getHit()->GetEnergy()+currentMScatHit->getHit()->GetEnergy());
          priorMScatHit->_isWithChild = true;
//          std::cerr << "NGMMultipleScattering::pushList " << priorMScatHit->getHit()->TimeDiffNanoSec(*firstTime)+4.6819492e+04 << " " << partID->GetName(priorMScatHit->getHit()) << " is with child ? " << priorMScatHit->isWithChild() << std::endl;
//          std::cerr << "NGMMultipleScattering::analyzeHit, adding energy to prior " << ++level << "th level, total is " << priorMScatHit->getHit()->GetEnergy() << std::endl;
          currentMScatHit->getHit()->SetEnergy(0.);
//          std::cerr << "NGMMultipleScattering::analyzeHit, new energy in prior " << partID->GetName(priorMScatHit->getHit()) << " = " << priorMScatHit->getHit()->GetEnergy() << std::endl;
//          std::cerr << "NGMMultipleScattering::analyzeHit, new energy in current " << partID->GetName(currentMScatHit->getHit()) << " = " << currentMScatHit->getHit()->GetEnergy() << std::endl;
          currentMScatHit->_closestNeighbourHit = priorMScatHit;
          NGMMultipleScatteringHit* tmpHit = priorMScatHit;
          priorMScatHit = priorMScatHit->getClosestNeighbour();
          currentMScatHit = tmpHit;
        }
//        std::cerr << std::endl; 
        return false;
      }
    }
  }
  return true;
}

void NGMMultipleScattering::enableNearestNeighbourRejection(bool enabled) {
  nearestNeighbourRejection = enabled;
};

NGMMultipleScatteringHit* NGMMultipleScattering::inTheNeighbourhood(NGMHit* tHit) {
// Checking for hits preceding and in the neighbourhood of tHit

  TString detname = partID->GetName(tHit);
  int dettype = partID->GetType(tHit);
  int firstIndex = getFirstIndex(&detname, dettype);
  int length = getLength(&detname, dettype);

  if (!enabledparticle[dettype]) return 0;

  // No neighbourhood for that type of particle
  if (length == 0) return 0;

  TIterator* tIter = pList->MakeIterator(kIterBackward);
  NGMMultipleScatteringHit* priorMSHit;
// If seekClosestNeighbour is set, we also want the absolute closest neighbour
  NGMMultipleScatteringHit* closestMSHit=NULL;
  double closestdistance=1e20;
  while ( (priorMSHit = (NGMMultipleScatteringHit*) tIter->Next()) != NULL &&
          !priorMSHit->isWithChild()) {
    NGMHit* priorHit = (NGMHit*) priorMSHit->getHit();
    int priordettype = partID->GetType(priorHit);
// if (dettype != priordettype) continue;
   if (dettype != priordettype) {
// If 2 detected events are of different types, they are not neighbours
// by default, unless it is a gamma produced by a fast neutron for liquid
// scintillators
     if (!getNeutron2GammaSuppression()) continue;
     else {
// Suppression of gammas following fast neutrons in liquid scintillators is on
       if (!(dettype == NGMSimpleParticleIdent::lsgamma && priordettype == NGMSimpleParticleIdent::lsneutron)) continue;
     }
   }
    double timeDiff = tHit->TimeDiffNanoSec(priorHit->GetNGMTime());
// If we exceed the maximum distance at the particle minimum speed,
// this count is not a neighbour
    if (timeDiff*minspeed[dettype] > largestdistance[dettype]) {
      delete tIter;
      return 0;
    }
// We skip all events such that even at the slowest speed of the particle
// of that type, it would be further away in such a long time interval.
    TString priordetname = partID->GetName(priorHit);
    for(int index = firstIndex; index<firstIndex+length; index++) {
// In the following if statement, we answer the questions for the specific 
// time interval timeDiff :
// (a) do we exceed the maximum distance at the particle minimum speed ? 
//     Yes, this count is not a neighbour
//     No, move on to question (b)
// (b) can we reach the minimum distance at the particle maximum speed ? 
//     No, this count is not a neighbour
//     Yes, this count is a neighbour.
      if (timeDiff*minspeed[dettype] < _reject[dettype][index]._maxdistance &&
          timeDiff*maxspeed[dettype] > _reject[dettype][index]._mindistance) 
        if (strcmp(*(_reject[dettype][index]._det2), priordetname) == 0) {
          if(_reject[dettype][index]._mindistance < closestdistance) {
            if (!getSeekClosestNeighbour()) {
              delete tIter;
              return priorMSHit;
            }
// If this detector is closer, it becomes the neighbour
            closestdistance = _reject[dettype][index]._mindistance;
            closestMSHit = priorMSHit;
          }
//          cerr << "detectors = " << detname << ", " << priordetname << " dt = " << timeDiff << " time = " << 1e-9*tHit->TimeDiffNanoSec(*firstTime) << endl;
// We return the closest neighbouring event
//          return priorMSHit;
        }
    }
  }
  delete tIter;
  if (getSeekClosestNeighbour()) return closestMSHit;
  else return NULL;
};

void NGMMultipleScattering::addRejectionEntry(TString det1, TString det2, float dt) {
  int dettype = NGMSimpleParticleIdent::lsneutron;
// Check if this is the largest distance
  addEntry(det1, det2, dettype, 0., 1.4*dt); // 14,000 km/sec or 1.4 cm/ns.
}

void NGMMultipleScattering::addEntry(TString det1, TString det2, int dettype, float mindistance, float maxdistance) {
  static int _reject_minincrement = 1000;

  if (maxdistance > largestdistance[dettype]) largestdistance[dettype] = maxdistance;
// Increase the vector size if necessary
  if (_reject_current_index[dettype] >= _reject_size[dettype]) {
    _reject_size[dettype] += _reject_minincrement;
    if(_reject[dettype] == 0)
    {
      _reject[dettype] = (rejection_st *) malloc(_reject_size[dettype]*sizeof(rejection_st));
    }else{
      _reject[dettype] = (rejection_st *) realloc(_reject[dettype], _reject_size[dettype]*sizeof(rejection_st));
    }
  }

// Check if det1 exists already
   int desiredindex = getDesiredIndex(det1, dettype);
   if (method2) { // always append
     mapready = false;
     _reject[dettype][_reject_current_index[dettype]]._det1 = new TString(det1);
     _reject[dettype][_reject_current_index[dettype]]._det2 = new TString(det2);
     _reject[dettype][_reject_current_index[dettype]]._mindistance = mindistance; 
     _reject[dettype][_reject_current_index[dettype]]._maxdistance = maxdistance; 
// 1 MeV fast neutrons travel at 14,000 km/sec = 14e6 m/sec = 14e8 cm/sec
// 1.4 cm/nsec
     if (desiredindex == -1) setmap(det1, dettype);
     else altermap(det1, dettype);
   } else {
     if (desiredindex == -1) { // If not, append the new entry to the list
       _reject[dettype][_reject_current_index[dettype]]._det1 = new TString(det1);
       _reject[dettype][_reject_current_index[dettype]]._det2 = new TString(det2);
       _reject[dettype][_reject_current_index[dettype]]._mindistance = mindistance;
       _reject[dettype][_reject_current_index[dettype]]._maxdistance = maxdistance;
       setmap(det1, dettype);
     } else { // insert the new entry where it belongs
       for (int i=_reject_current_index[dettype]-1; i>=desiredindex; i--) { // make room
         _reject[dettype][i+1]._det1 = _reject[dettype][i]._det1;
         _reject[dettype][i+1]._det2 = _reject[dettype][i]._det2;
         _reject[dettype][i+1]._mindistance = mindistance;
         _reject[dettype][i+1]._maxdistance = maxdistance;
       }
       _reject[dettype][desiredindex]._det1 = new TString(det1); // insert new entry
       _reject[dettype][desiredindex]._det2 = new TString(det2);
       _reject[dettype][desiredindex]._mindistance = mindistance;
       _reject[dettype][desiredindex]._maxdistance = maxdistance;
       resetmap(det1, dettype);
    }
  }
  _reject_current_index[dettype]++;
}

void NGMMultipleScattering::addDistanceEntry(TString det1, TString det2, TString dettype_str, float mindistance, float maxdistance) {
  // Loop over all detected particle types
  TObjArray* str_array = dettype_str.Tokenize(":");
  for(int i=0; i<str_array->GetEntriesFast(); i++) {
    const char* c = ((TObjString*) str_array->At(i))->String().Data();
    int dettype = atoi(c);
    addEntry(det1, det2, dettype, mindistance, maxdistance);
//    std::cerr << "adding a detector type " << dettype << " in NGMMultipleScattering::addDistanceEntry" << std::endl;
  }
  str_array->Delete();
  delete str_array;
}

int NGMMultipleScattering::getFirstIndex(TString* detname, int dettype){
  for(int i=0; i<_detname_current_index[dettype]; i++) {
    if (strcmp(*detname, *(_detname_map[dettype][i]._detname)) == 0) return _detname_map[dettype][i]._index;
  }
  return -1;
}

int NGMMultipleScattering::getLength(TString* detname, int dettype){
  for(int i=0; i<_detname_current_index[dettype]; i++) {
    if (strcmp(*detname, *(_detname_map[dettype][i]._detname)) == 0) return _detname_map[dettype][i]._length;
  }
  return 0;
}

int NGMMultipleScattering::getDesiredIndex(TString detname, int dettype){
  for(int i=_detname_current_index[dettype]-1; i>=0; i--) {
    if (strcmp(detname, *(_detname_map[dettype][i]._detname)) == 0) return _detname_map[dettype][i]._index+_detname_map[dettype][i]._length;
  }
  return -1;
}

int NGMMultipleScattering::getMapIndex(int index, int dettype){
  for(int i=0; i<_detname_current_index[dettype]; i++) {
    if (index >= _detname_map[dettype][i]._index && index <_detname_map[dettype][i]._index+_detname_map[dettype][i]._length) {
      return i;
    }
  }
  LOG << "Could not find the index of the detector in the liquid scintillator name to index mapping." << ENDM_WARN;
  return -1;
}

int NGMMultipleScattering::getMapIndex(TString* name, int dettype){
  for(int i=0; i<_detname_current_index[dettype]; i++) {
    if (strcmp(*name, (_detname_map[dettype][i]._detname)->Data()) == 0) return i;
  }
  LOG << "Could not find the index of the detector in the liquid scintillator name to index mapping." << ENDM_WARN;
  return -1;
}

void NGMMultipleScattering::setmap(TString detname, int dettype){
  static int minincrement = 100;

// Increase the vector size if necessary
  if (_detname_current_index[dettype] >= _detname_map_size[dettype]) {
    _detname_map_size[dettype] += minincrement;
    if(_detname_map[dettype] == 0){
      _detname_map[dettype] = (detname2index_st *) malloc(_detname_map_size[dettype]*sizeof(detname2index_st));
    }else{
      _detname_map[dettype] = (detname2index_st *) realloc(_detname_map[dettype], _detname_map_size[dettype]*sizeof(detname2index_st));
    }
  }

// append the new entry
  _detname_map[dettype][_detname_current_index[dettype]]._detname = new TString(detname);
  _detname_map[dettype][_detname_current_index[dettype]]._index = _reject_current_index[dettype];
  _detname_map[dettype][_detname_current_index[dettype]]._length = 1;
  _detname_current_index[dettype]++;
}

void NGMMultipleScattering::resetmap(TString detname, int dettype){
  for(int i=_detname_current_index[dettype]-1; i>=0; i--) {
    if (strcmp(detname, *(_detname_map[dettype][i]._detname)) != 0) { // alter the entries past detname
      _detname_map[dettype][i]._index = _detname_map[dettype][i]._index+1;
    } else { // we found the detector
      _detname_map[dettype][i]._length = _detname_map[dettype][i]._length+1;
      break;
    }
  }
}

void NGMMultipleScattering::ordermap(){

  // First step is to set the starting points of each liquid
  // scintillator segment in the liquid scintillator to index map
  // We create simultaneously the current_indices array which keeps
  // track of the latest free location for a liquid detector in a 
  // segment.
  for (int dettype=1; dettype<=maxparttypes; dettype++) {
    int current_index=0;
    int* current_indices = new int [_detname_current_index[dettype]];
    for(int i=0; i<_detname_current_index[dettype]; i++) {
      _detname_map[dettype][i]._index = current_index;
      current_indices[i] = _detname_map[dettype][i]._index;
      current_index += _detname_map[dettype][i]._length;
    }
    if (current_index != _reject_current_index[dettype]) LOG << "Problem ordering the map" << ENDM_WARN;


    // Second step is to sort the liquid scintillator array
    // We loop over each element in the array, and place it where
    // it belongs by swapping its location with another element
    for(int i=0; i<_reject_current_index[dettype]; i++) {
      // find index in mapping for location i in _reject[dettype] array
      int mapindex = getMapIndex(i, dettype);
      while (strcmp((_reject[dettype][i]._det1)->Data(), (_detname_map[dettype][mapindex]._detname)->Data()) != 0) {
        int newmapindex = getMapIndex(_reject[dettype][i]._det1, dettype);
        swap(i, current_indices[newmapindex], dettype); // put element i where it belongs
        current_indices[newmapindex]++;
      }
    }
    delete current_indices;
  }
  mapready = true;
}

void NGMMultipleScattering::computetimeintervals(){
  maxlargesttimeinterval = 0.;
  for (int i=1; i<=maxparttypes; i++) {
    if (maxlargesttimeinterval < largestdistance[i]/minspeed[i]) 
      maxlargesttimeinterval = largestdistance[i]/minspeed[i];
  }
  timeintervalsready = true;
}

void NGMMultipleScattering::altermap(TString detname, int dettype){
  for(int i=_detname_current_index[dettype]-1; i>=0; i--) {
    if (strcmp(detname, *(_detname_map[dettype][i]._detname)) == 0) {
      // we found the detector
      _detname_map[dettype][i]._length = _detname_map[dettype][i]._length+1;
      break;
    }
  }
}

void NGMMultipleScattering::swap(int index1, int index2, int dettype){

//  cerr << "Swapping entries " << index1 << " and " << index2 << endl;
  TString* str1 = _reject[dettype][index1]._det1;
  TString* str2 = _reject[dettype][index1]._det2;
  float mind = _reject[dettype][index1]._mindistance;
  float maxd = _reject[dettype][index1]._maxdistance;
     
  _reject[dettype][index1]._det1 = _reject[dettype][index2]._det1;
  _reject[dettype][index1]._det2 = _reject[dettype][index2]._det2;
  _reject[dettype][index1]._mindistance = _reject[dettype][index2]._mindistance;
  _reject[dettype][index1]._maxdistance = _reject[dettype][index2]._maxdistance;

  _reject[dettype][index2]._det1 = str1;
  _reject[dettype][index2]._det2 = str2;
  _reject[dettype][index2]._mindistance = mind;
  _reject[dettype][index2]._maxdistance = maxd;
}

void NGMMultipleScattering::printmap(int dettype){
  std::cout << "index\tdetector_name\tfirst_index\tlength" << std::endl;
  for(int i=0; i<_detname_current_index[dettype]; i++) {
    std::cout << i << " "
         << *(_detname_map[dettype][i]._detname) << " " 
         << _detname_map[dettype][i]._index << " " 
         << _detname_map[dettype][i]._length << std::endl;
  }
}

void NGMMultipleScattering::InitFromGeometry(const NGMSystemConfiguration* sysConf)
{
  // For real data we will need to include the channel by channel timing resolution
  // This would come from a calibration file associated with the run
  
  // Attempt to find geometry file
  TString fullGeoPathName("");
  TString ngmsearchpath(gSystem->Getenv("NGMCALDIR"));
  
  // Append trailing slash if needed
  if(ngmsearchpath!="" && !ngmsearchpath.EndsWith("/")) ngmsearchpath+="/";

  // There is some reverse logic here, 0 is returned if the file _is_ found
  if(!gSystem->AccessPathName(_geoMacroName.Data()))
  {
    fullGeoPathName = _geoMacroName;
  }else if(!gSystem->AccessPathName(ngmsearchpath + _geoMacroName)){
    fullGeoPathName = ngmsearchpath + _geoMacroName;
  }else{
    // The Display object will have to know what to do
    fullGeoPathName = "";
  }
  
  NGMDisplay::Instance()->InitFromMacro(fullGeoPathName.Data(),sysConf->getRunNumber());
  
  TObjArray* detGeoList = NGMDisplay::Instance()->GetNodes();
  TObjArray* detPosList = NGMDisplay::Instance()->GetPositions();
  
  // We should include a parameter that characterizes a maximum distance.
  
  for(int id1 = 0; id1 < detGeoList->GetEntries(); id1++)
  {
    TString detname1(detGeoList->At(id1)->GetName());
    detname1 = detname1(0,detname1.Length()-2);
    TVector3& pos1 = *((TVector3*)(detPosList->At(id1)));
    TGeoShape* shape1 = (dynamic_cast<TGeoNodeMatrix*>(detGeoList->At(id1)))->GetVolume()->GetShape();
    double maxpl1 = NGMDisplay::Instance()->getMaxRadialPathLength(shape1);
    
    int plotIndex1 = partID->getPlotIndex(detname1.Data());
    // Skip unknown channels
    if(plotIndex1 < 0) continue;
    
    int dettype1 = partID->GetDetectorType(plotIndex1);
    // Skip channels with unknown detector type
    if(dettype1 < 0) continue;  
    
    for(int id2 = id1+1; id2 < detGeoList->GetEntries(); id2++)
    {
      TString detname2(detGeoList->At(id2)->GetName());
      detname2 = detname2(0,detname2.Length()-2);

      int plotIndex2 = partID->getPlotIndex(detname2.Data());
      // Skip unkown channels
      if(plotIndex2 < 0) continue;
      
      int dettype2 = partID->GetDetectorType(plotIndex2);
      // Skip channels with unknown detector type
      if(dettype1 < 0) continue;  
      // Skip channels of differing types
      // This functionality could be added later to accomodate
      // a) different detector types like gamma blocks and liquid scintillator blocks
      // b) LS gammas with LS neutrons
      if(dettype1 != dettype2) continue;
      
      TVector3& pos2 = *((TVector3*)(detPosList->At(id2)));
      double separation = (pos2-pos1).Mag();
      
      // Only add entries for separations less than maximal distance
      if(separation > _maximumDistanceForInitFromGeometry) continue;
      
      TGeoShape* shape2 = (dynamic_cast<TGeoNodeMatrix*>(detGeoList->At(id2)))->GetVolume()->GetShape();
      double maxpl2 = NGMDisplay::Instance()->getMaxRadialPathLength(shape2);
      double minsep = TMath::Max(0.0, separation - maxpl2 - maxpl1);
      double maxsep = separation + maxpl2 + maxpl1;
      if(dettype1 == NGMSimpleParticleIdent::gammablock)
      {
        addEntry(detname1, detname2, (int)NGMSimpleParticleIdent::gbgamma,
                          minsep, maxsep);
        addEntry(detname2, detname1, (int)NGMSimpleParticleIdent::gbgamma,
                         minsep, maxsep);
      }else if(dettype1 == NGMSimpleParticleIdent::liquidscint){
        addEntry(detname1, detname2, (int)NGMSimpleParticleIdent::lsgamma,
                          minsep, maxsep );
        addEntry(detname2, detname1, (int)NGMSimpleParticleIdent::lsgamma,
                         minsep, maxsep );
        addEntry(detname1, detname2, (int)NGMSimpleParticleIdent::lsneutron,
                          minsep, maxsep);
        addEntry(detname2, detname1, (int)NGMSimpleParticleIdent::lsneutron,
                         minsep, maxsep);
      }// if dettype1
    }// chanid2
  }// chanid1
  
}

bool NGMMultipleScattering::enable(int parttype){
  if (parttype == NGMSimpleParticleIdent::hettlid || parttype == NGMSimpleParticleIdent::heid) {
    LOG << "Multiple scattering filter can not be enabled for particle type " 
        << parttype << ENDM_WARN;
    return false;
  } else {
    enabledparticle[parttype] = true;
    return true;
  }
}

bool NGMMultipleScattering::disable(int parttype){
  enabledparticle[parttype] = false;
  return true;
}

void NGMMultipleScattering::setSpeed(int parttype, float min, float max){
  if (parttype == NGMSimpleParticleIdent::hettlid || parttype == NGMSimpleParticleIdent::heid) {
    LOG << "Multiple scattering filter is not applicable for particle type " 
        << parttype << ENDM_WARN;
    return;
  }

  minspeed[parttype] = min*100*1e-9; // Conversion from m/s to cm/ns
  maxspeed[parttype] = max*100*1e-9;
}

void NGMMultipleScattering::setNeutron2GammaSuppression(bool option) {
  neutron2GammaSuppression = option;
}

bool NGMMultipleScattering::getNeutron2GammaSuppression() {
  return neutron2GammaSuppression;
}

void NGMMultipleScattering::setSeekClosestNeighbour(bool option) {
  seekClosestNeighbour = option;
}

bool NGMMultipleScattering::getSeekClosestNeighbour() {
  return seekClosestNeighbour;
}

void NGMMultipleScattering::pushList(const NGMHit* tHit) {
  // We can push all the hits that have _keep == true
  // and that are more than some time interval ahead of the
  // last hit. We determine the time interval this way: first
  // we look at the maximum distance between any 2 detectors
  // for a given particle type, second we assume a particle of
  // that type will not scatter over lengths greater than maxScat
  // that distance. This is assuming that the track length between
  // the first and last scatters of a particle will not exceed
  // maxScat*(maximum distance between any 2 detectors). Third,
  // we divide that distance by the minimum speed for that 
  // particle type
  if (pList->IsEmpty()) return;

  NGMMultipleScatteringHit* firstMScatHit;
  NGMHit* firstHit;
  double timeDiff;
  while ( (firstMScatHit = (NGMMultipleScatteringHit*) pList->First()) != NULL &&
           (firstHit = firstMScatHit->getHit()) != NULL &&
           (timeDiff = tHit->TimeDiffNanoSec(firstHit->GetNGMTime())) > 
            maxScat*maxlargesttimeinterval) {
    if (firstMScatHit->getKeep()) {
      push(*firstHit);
//      std::cerr << "NGMMultipleScattering::pushList pushing " << partID->GetName(firstHit) << " (type=" << partID->GetType(firstHit) << ") time=" << firstHit->TimeDiffNanoSec(*firstTime)+4.6819492e+04 << " energy=" << firstHit->GetEnergy() << " maxScat*maxinterval=" << maxScat*maxlargesttimeinterval << std::endl; 
    } else {
//      std::cerr << "NGMMultipleScattering::pushList dropping " << partID->GetName(firstHit) << " (type=" << partID->GetType(firstHit) << ") time=" << firstHit->TimeDiffNanoSec(*firstTime)+4.6819492e+04 << " energy=" << firstHit->GetEnergy() << " maxScat*maxinterval=" << maxScat*maxlargesttimeinterval << std::endl; 
      _rejectedCounts++;
    }
    TObject* hitToRemove = pList->First();
    pList->RemoveFirst();
    delete hitToRemove;
  };
}
