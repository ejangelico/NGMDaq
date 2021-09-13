#ifndef __NGMSIS3316RAWREADER_H__
#define __NGMSIS3316RAWREADER_H__
#include "NGMReaderBase.h"
#include "TTimeStamp.h"
#include "TString.h"
#include "TSystem.h"
#include <cstdio>
#include <vector>
#include <map>
/// \brief NGMSIS3316RawReader is the reader implementation for the raw binar format
///

class sis3316card; //Forward Declaration
class NGMHitPoolv6; //Forward Declaration

class NGMSIS3316RawReader : public NGMReaderBase
{
public:
    /// \brief Default Constructor required for ROOT
    NGMSIS3316RawReader();
    /// \brief Constructor
    /// @param parent top module for which push method will be issued
    NGMSIS3316RawReader(NGMModule* parent);
    /// \brief Dtor
    ~NGMSIS3316RawReader();
    /// \brief Open Input File Stream
    /// @param fname is the pathname of associated root file to open
    Long64_t OpenInputFile(const char* fname);
    /// \breif Close Input File Stream
    Long64_t CloseInputFile();
    /// \breif ReadAll
    Long64_t ReadAll();
    /// \brief Sort Events from spill
    Long64_t SortSpill();
    int ParseEvent(sis3316card* vcard, int ichan,size_t evtpos,NGMHit* hit);
    int ParseBlockEvent(sis3316card* vcard, int ichan,size_t evtpos,NGMHit* hit);
    /// \brief Set Configuration for this run
    virtual void SetConfiguration(const NGMSystemConfiguration* sysConf);
    void PlotFirstWaveform(sis3316card* vcard, int ichan);
    virtual void GetStatus(TString& sStatus);
    Long64_t ReadNextSpillFromFile();

    Long64_t parsePacket(int slot, int chan, NGMBufferedPacket* packet);
    ULong64_t ParseClock(unsigned int* data);
    int ParseADCBlockData(sis3316card* vcard, int iblock,NGMBufferedPacket* packet);
    int ParseSingleChannelData(sis3316card* vcard, int ichan,NGMBufferedPacket* packet);
    int ParseEventDataSingle(sis3316card* vcard, int ichan, std::vector<unsigned long long> &evtPointer, std::map<unsigned long long,int> &evtid);
    Long64_t OpenRawBinaryFile(const char* pathname);
    static UInt_t sumarray(int alength, unsigned short * arrptr);
    Long64_t _nextbufferPosition;
    FILE* _inputfile;
    TString _inputfilename;
    long _inputfilesize;
    TTimeStamp _runBegin;
    bool _requestStop;
    TArrayD _NanoSecondsPerClock;
    TTimeStamp _firstTimeOfRun;
    TTimeStamp _earliestTimeInSpill;
    TTimeStamp _latestTimeInSpill;
    double _runduration;
    double _livetime;
    Long64_t _totalEventsThisRun;
    int _lastPlotIndex;
    int _filecounter;
    int _verbosity;
    int _psdScheme;
    NGMHitPoolv6* _hitPool;
    
    ClassDef(NGMSIS3316RawReader,5)
};




#endif //__NGMSIS3316RAWREADER_H__