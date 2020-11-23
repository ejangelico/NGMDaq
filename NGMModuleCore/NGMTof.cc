//
//  NGMTof.cpp
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMTof.h"
#include "TClass.h"
#include "NGMHit.h"
#include "NGMSystemConfiguration.h"
#include "TFolder.h"
#include "NGMLogger.h"
#include "TDirectory.h"
#include <cmath>
#include "NGMSimpleParticleIdent.h"

NGMTof::NGMTof()
{
  InitCommon();
}

NGMTof::NGMTof(const char* name, const char* title)
:NGMModule(name,title)
{
  InitCommon();
  partID = new NGMSimpleParticleIdent();
}

NGMTof::~NGMTof(){}

bool  NGMTof::init()
{
  return true;
}

bool  NGMTof::process(const TObject& tData)
{
  static TClass* tNGMBufferedPacketType = TClass::GetClass("NGMBufferedPacket");
  static TClass* tNGMSystemConfigurationType = TClass::GetClass("NGMSystemConfiguration");
  static TClass* tNGMHitType = TClass::GetClass("NGMHit");

  if(tData.InheritsFrom(tNGMHitType)){
    const NGMHit* tHit = (const NGMHit*)(&tData);
    processHit(tHit);
  }else if(tData.InheritsFrom(tNGMBufferedPacketType)){
    
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* conf = dynamic_cast<const NGMSystemConfiguration*>(&tData);
    processConfiguration(conf);
    push(tData);
  }else{
    push(tData);
  }

  return true;
}

bool  NGMTof::finish()
{
  return true;
}
void  NGMTof::LaunchDisplayCanvas()
{
  return;
}

void  NGMTof::ResetHistograms()
{
}

void NGMTof::Print(Option_t* option) const {}

bool NGMTof::processConfiguration(const NGMSystemConfiguration* conf)
{
  LOG<<"BlockPSDMaker processConfiguraiton"<<ENDM_INFO;
  partID->Init(conf);
  _hitsAnalyzed = 0;
  _triggerCount=0;
  if(!_hitList) _hitList = new TList();
  _hitList->Clear();
  return true;
}

void NGMTof::InitCommon()
{
  _triggerChannel=16*4;
  _prevCfHit=0;
  _hitsAnalyzed = 0;
  _triggerCount = 0;
  _maxNanosecondsInList = 1000.0;
  _doTimeSincePreviousOnly = false;
  _hitList = 0;
  scaleFactor = 10.0;
}

bool NGMTof::processHit(const NGMHit* tHit)
{
    _hitsAnalyzed++;
    
    if(_doTimeSincePreviousOnly){
        analyzeTimeSincePrev(tHit);
    }else{
        analyzeAllPairs(tHit);
    }
    return true;
}

void NGMTof::analyzeTimeSincePrev(const NGMHit* hit)
{
    int hwchan = partID->getPlotIndex(hit);
    if(hwchan==_triggerChannel){
        _triggerCount++;
        delete _prevCfHit;
        _prevCfHit = hit->DuplicateHit();
    }else{
        if(_prevCfHit)
        {
            analyzePair(_prevCfHit,hit);
        }
    }
}

void NGMTof::analyzeAllPairs(const NGMHit* hit)
{
    if(partID->getPlotIndex(hit)==_triggerChannel) _triggerCount++;
    
    /// Add this hit to our local chain
    /// We add a copy so we can keep it around as long as we like...
    _hitList->AddLast(hit->DuplicateHit());
    
    // Remove those outside new window
    while(1){
        NGMHit* firstHit = (NGMHit*)  _hitList->First();
        NGMHit* lastHit = (NGMHit*) _hitList->Last();
        //if(!firstHit || !lastHit) break;
        double timeDiffns = lastHit->TimeDiffNanoSec(firstHit->GetNGMTime());
        if(timeDiffns<0.0)
        {
            LOG<<" DATA NOT SORTED "
            <<"("<<firstHit->GetSlot()<<","<<firstHit->GetChannel()<<") "
            <<(Long64_t)firstHit->GetRawClock()<<" "
            <<"("<<lastHit->GetSlot()<<","<<lastHit->GetChannel()<<") "
            <<(Long64_t)lastHit->GetRawClock()<<" "
            <<ENDM_FATAL;
            _hitList->Clear("C");
            return;
        }
        if(timeDiffns>_maxNanosecondsInList){
            NGMHit* hitToRemove = (NGMHit*)(_hitList->First());
            _hitList->RemoveFirst();
            delete hitToRemove;
        }else{
            break;
        }
    }
    // Now with those remaining lets analyze all pairs with last added.
    // Fill Channel by channel time differences
    // Use TIter for TList (doubly linked list)
    TListIter tHitIter(_hitList);
    NGMHit* ihit = 0;
    NGMHit* lastHit = (NGMHit*) (_hitList->Last());
    while( (ihit = (NGMHit*) (tHitIter.Next())) )
    {
        if(ihit==lastHit) break;
        if(partID->getPlotIndex(ihit)==_triggerChannel
           && partID->getPlotIndex(lastHit)!=_triggerChannel)
        {
            analyzePair(ihit,lastHit);
        }else if(partID->getPlotIndex(ihit)!=_triggerChannel
                 && partID->getPlotIndex(lastHit)==_triggerChannel)
        {
            analyzePair(lastHit,ihit);
        }
    }

}
void NGMTof::analyzePair(const NGMHit* trig, const NGMHit* hit)
{
    double tof = hit->TimeDiffNanoSec(trig->GetNGMTime());
    if(tof<=_maxNanosecondsInList)
    {
        NGMHit* newHit = hit->DuplicateHit();
        int lastGateIndex = hit->GetGateCount();
        newHit->SetGateSize(lastGateIndex+1);
        newHit->SetGate(lastGateIndex,tof*scaleFactor);
        push(*((const TObject *)newHit));
        delete newHit;
    }
}
