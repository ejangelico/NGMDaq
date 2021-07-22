#ifndef __NGMREADERBASE_H__
#define __NGMREADERBASE_H__
/*
 *  NGMReaderBase.h
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/3/08.
 *  Copyright 2008 LLNL. All rights reserved.
 *
 */

/// \brief NGMReaderBase defines the interface for all file readers
///
/// There are two basic formats: root and raw binary.
/// Root format files contain at least two objects, a NGMSystemConfiguration object
/// and a TTree of either NGMHits or NGMBufferedPackets.
/// The raw binary format is the SIS Raw format, but is always accompanied by a
/// root file containing the NGMSystemConfiguration object as in the case of the 
/// root format above.

#include "Rtypes.h"
#include "TArrayD.h"
#include "TArrayI.h"
#include "TTimeStamp.h"
#include "TString.h"
#include "TSystem.h"
#include <fstream>
#include "NGMSystemConfiguration.h"
#include "NGMModule.h"

 // forward declarations
class TFile;
class TTree;
class NGMBufferedPacket;
class NGMHit;
class NGMTimingCal;

class NGMReaderBase
{
public:
  /// \brief Default Constructor required for ROOT
  NGMReaderBase();
  /// \brief Constructor
  /// @param parent top module for which push method will be issued
  NGMReaderBase(NGMModule* parent);
  /// \brief destructor
  virtual ~NGMReaderBase();
  /// \breif Open Input File Stream
  /// @param fname is the pathname of file to open
  virtual Long64_t OpenInputFile(const char* fname){ return 0;}
  /// \brief Close Input File Stream
  virtual Long64_t CloseInputFile(){ return 0;}
  /// \brief ReadAll
  virtual Long64_t ReadAll(){ return 0;}
  /// \brief Set Pause
  virtual Long64_t SetPause(){ _pause = true; return 0;}
  /// \brief Resume Read
  virtual Long64_t Resume(){ _pause = false; return 0;}
  /// \brief Abort Read
  virtual Long64_t AbortRead(){ _abortread = true; return 0;}
  /// \brief Request PlotUpdate signal to be issued down chain
  virtual void RequestPlotUpdate(){ _sendplotupdate = true; }
  /// \brief Set Configuration for this run  
  virtual void SetConfiguration(const NGMSystemConfiguration* sysConf);
  /// \brief Get Configuration associated with current file   
  virtual NGMSystemConfiguration* GetConfiguration() { return _config; }
  /// \brief Get Parent NGMModule associated with this reader   
  virtual NGMModule* GetParent() { return _parent; }
  /// \brief Send signals to close out analysis for this run 
  virtual void SendEndRun();
  /// \brief Send signals to Update Plots
  virtual void SendPlotUpdate();
  /// \brief Read Run Series
  virtual void ReadRunSeries(const char* inputrunlist);
  /// \brief Open the next file for reading in a series.  Closed with SendEndRun signal.
  virtual void ReadNextInSeries(const char* inputfilepath);
  /// \brief GetSeriesCount
  virtual int GetSeriesCount() const {return _seriesCount;}
  /// \brief GetSeriesCount
  virtual void SetMaxRunTime(Double_t maxruntime) {_maxruntime =maxruntime;}
    /// \brief SetTimingCal
    virtual void SetTimingCal(NGMTimingCal* tcal) {_tcal=tcal;}
    /// \brief GetStatus
    virtual void GetStatus(TString& sStatus) {sStatus="NGMReaderBase::GetStatus Not Implemented\n";}
  void SetSkipWaveforms(bool newVal = true) {_skipWaveforms=newVal;}

  /// \brief Reader Factory by string ROOT, or SISRaw, (or later COG)
  static NGMReaderBase* Factory(const char* readerType, NGMModule* parent);
  
protected:
  NGMModule* _parent;
  NGMSystemConfiguration* _config;
  bool _pause;
  bool _abortread;
  bool _sendplotupdate;
  int _seriesCount;
  Long64_t _bytesRead;
  Double_t _timeSinceLastProcessSrv;
  Double_t _maxruntime;
  NGMTimingCal* _tcal;
  bool _skipWaveforms=false;

  ClassDef(NGMReaderBase,8)
};

/// \brief NGMRootReader is the reader implementation for the root format
///
class NGMRootReader : public NGMReaderBase
  {
  public:
    /// \brief Default Constructor required for ROOT
    NGMRootReader();
    /// \brief Constructor
    /// @param parent top module for which push method will be issued
    NGMRootReader(NGMModule* parent);
    /// \brief Dtor
    ~NGMRootReader();
    /// \brief Open Input File Stream
    /// @param fname is the pathname of file to open
    Long64_t OpenInputFile(const char* fname);
    /// \brief Close Input File Stream
    Long64_t CloseInputFile();
    /// \brief ReadAll data in open file
    Long64_t ReadAll();
    /// \brief Set Configuration for this run  
    void SetConfiguration(const NGMSystemConfiguration* sysConf);

  private:
    Long64_t ReadAllBufferedPackets();
    Long64_t ReadAllHits();
    TFile* _inputFile; //! inputFile object
    TTree* _inputBuffer;//! inputStream within inputFile object
    Long64_t _currEventNumber; //! index of current event
    
  protected:
    bool FindNextFile();

    Long64_t _bytesread;
    Long64_t _numberOfSlots;
    Long64_t _numberOfHWChannels;
    Long64_t _channelsperSlot;
    Long64_t _prevBufferIndex;
    Long64_t _spillCounter;
    bool _readingHits;
    bool _readUnsortedPackets;
    
    NGMBufferedPacket* _localBuffer; //! local buffer used for IO
    NGMHit* _localHit; //! local buffer used for hitio
    
    ClassDef(NGMRootReader,1)  
  };

/// \brief NGMSISRawReader is the reader implementation for the raw binar format
///
class NGMSISRawReader : public NGMReaderBase
  {
  public:
    /// \brief Default Constructor required for ROOT
    NGMSISRawReader();
    /// \brief Constructor
    /// @param parent top module for which push method will be issued
    NGMSISRawReader(NGMModule* parent);
    /// \brief Dtor
    ~NGMSISRawReader();
    /// \brief Open Input File Stream
    /// @param fname is the pathname of associated root file to open
    Long64_t OpenInputFile(const char* fname);
    /// \breif Close Input File Stream
    Long64_t CloseInputFile();
    /// \breif ReadAll
    Long64_t ReadAll();
    /// \brief Set Configuration for this run  
    virtual void SetConfiguration(const NGMSystemConfiguration* sysConf);

    Long64_t ReadNextBufferFromFile();
    Long64_t readBytesWaiting(Long64_t nbytesToRead, bool waitOnData = false);
    Long64_t writePacket(int packetlength, unsigned int *data);
    Long64_t OpenRawBinaryFile(const char* pathname);
    
    Long64_t _nextbufferPosition;
    std::ifstream* _inputfile;
    TString _inputfilename;
    unsigned int * _rawbuffer; //! 
    NGMBufferedPacket* _outBuffer; //!
    TTimeStamp _runBegin;
    bool _requestStop;
    TArrayD _NanoSecondsPerClock;
    TArrayI _CardModel;
    TArrayI _GateMode; // for 3302 this is either 0 or 1
    TTimeStamp _firstTimeOfRun;
    TTimeStamp _earliestTimeInSpill;
    TTimeStamp _latestTimeInSpill;
    double _runduration;
    double _livetime;
    Long64_t _totalEventsThisRun;
    int _lastPlotIndex;
    int _filecounter;
    
    ClassDef(NGMSISRawReader,3)  
  };

/// \brief NGMSISRawReader is the reader implementation for the raw binar format
///
class NGMSIS3305RawReader : public NGMReaderBase
{
public:
  /// \brief Default Constructor required for ROOT
  NGMSIS3305RawReader();
  /// \brief Constructor
  /// @param parent top module for which push method will be issued
  NGMSIS3305RawReader(NGMModule* parent);
  /// \brief Dtor
  ~NGMSIS3305RawReader();
  /// \brief Open Input File Stream
  /// @param fname is the pathname of associated root file to open
  Long64_t OpenInputFile(const char* fname);
  /// \breif Close Input File Stream
  Long64_t CloseInputFile();
  /// \breif ReadAll
  Long64_t ReadAll();
  /// \brief Set Configuration for this run  
  virtual void SetConfiguration(const NGMSystemConfiguration* sysConf);
  
  Long64_t ReadNextBufferFromFile();
  Long64_t readBytesWaiting(Long64_t nbytesToRead, bool waitOnData = false);
  Long64_t writePacket(int packetlength, unsigned int *data);
  Long64_t OpenRawBinaryFile(const char* pathname);
  
  Long64_t _nextbufferPosition;
  std::ifstream* _inputfile;
  TString _inputfilename;
  unsigned int * _rawbuffer; //! 
  NGMBufferedPacket* _outBuffer; //!
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
  int _curchannel;
  
  ClassDef(NGMSIS3305RawReader,1)  
};

/// \brief NGMGageRawReader is the reader implementation for the raw binar format
///
class NGMGageRawReader : public NGMReaderBase
  {
  public:
    /// \brief Default Constructor required for ROOT
    NGMGageRawReader();
    /// \brief Constructor
    /// @param parent top module for which push method will be issued
    NGMGageRawReader(NGMModule* parent);
    /// \brief Dtor
    ~NGMGageRawReader();
    /// \brief Open Input File Stream
    /// @param fname is the pathname of associated root file to open
    Long64_t OpenInputFile(const char* fname);
    /// \breif Close Input File Stream
    Long64_t CloseInputFile();
    /// \breif ReadAll
    Long64_t ReadAll();
    /// \brief Set Configuration for this run  
    virtual void SetConfiguration(const NGMSystemConfiguration* sysConf);
    
    Long64_t ReadNextBufferFromFile();
    Long64_t readBytesWaiting(Long64_t nbytesToRead, bool waitOnData = false);
    Long64_t writePacket(double time, int chanid, int nsamples);
    
    Long64_t _nextbufferPosition;
    std::ifstream* _inputfile;
    TString _inputfilename;
    unsigned int * _rawbuffer; //! 
    NGMBufferedPacket* _outBuffer; //!
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
    
    ClassDef(NGMGageRawReader,1)  
  };

/// \brief NGMPIXIE16ORNLRawReader is the reader implementation
///  for the raw PIXIE16 binary format
///
class NGMPIXIE16ORNLRawReader : public NGMReaderBase
{
public:
  /// \brief Default Constructor required for ROOT
  NGMPIXIE16ORNLRawReader();
  /// \brief Constructor
  /// @param parent top module for which push method will be issued
  NGMPIXIE16ORNLRawReader(NGMModule* parent);
  /// \brief Dtor
  ~NGMPIXIE16ORNLRawReader();
  /// \brief Open Input File Stream
  /// @param fname is the pathname of associated root file to open
  Long64_t OpenInputFile(const char* fname);
  /// \breif Close Input File Stream
  Long64_t CloseInputFile();
  /// \breif ReadAll
  Long64_t ReadAll();
  /// \brief Set Configuration for this run  
  virtual void SetConfiguration(const NGMSystemConfiguration* sysConf);
  
  Long64_t ReadNextBufferFromFile();
  Long64_t ReadNextBufferFromFilev1();
  Long64_t ReadNextBufferFromFilev2();
  Long64_t readBytesWaiting(Long64_t nbytesToRead, bool waitOnData = false, size_t localOffsetInBytes = 0);
  Long64_t writePacket(int packetlength, unsigned int *data);
  Long64_t writePacketv2(int packetlength, unsigned int *data);
  Long64_t OpenRawBinaryFile(const char* pathname);
  int AnaStats(NGMBufferedPacket* packet);
  
  Long64_t _nextbufferPosition;
  std::ifstream* _inputfile;
  TString _inputfilename;
  unsigned int * _rawbuffer; //! 
  NGMBufferedPacket* _outBuffer; //!
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
  
  ClassDef(NGMPIXIE16ORNLRawReader,1)  
};

#endif // __NGMREADERBASE_H__
