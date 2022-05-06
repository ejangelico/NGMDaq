#ifndef __AnaAverageWaveform_H__
#define __AnaAverageWaveform_H__
//
//  AnaAverageWaveform.h
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMModule.h"
#include "NGMPSDAnalyzer.h"
#include "TObjString.h"
class NGMBufferedPacket; //Forward Declaration
class NGMHit; //Forward Declaration
class TH2; //Forward Declaration

/// \brief  AnaAverageWaveform analyzes a stream of hits and
/// and averages waveforms

class AnaAverageWaveform : public NGMModule
{
public:
    AnaAverageWaveform();
    AnaAverageWaveform(const char* name, const char* title);
    ~AnaAverageWaveform();
    
    bool  init();
    bool  finish();
    void  LaunchDisplayCanvas(); // *MENU*
    void  ResetHistograms();
    void Print(Option_t* option = "") const;
    int channelToAnalyze;
    NGMPSDGatti _psda;
    double _emin;
    double _emax;
    double _npsdmin;
    double _npsdmax;
    double _gpsdmin;
    double _gpsdmax;
    TH2* _psde;
    unsigned long long _eventsanalyzed;
    
private:
    bool processConfiguration(const NGMSystemConfiguration* conf);
    bool processHit(const NGMHit* tHit);
    bool processPacket(const NGMBufferedPacket* packet);
    bool processMessage(const TObjString* mess);
    void InitCommon();
    
    
    
    ClassDef(AnaAverageWaveform,1)
    
};

#endif // __AnaAverageWaveform_H__

