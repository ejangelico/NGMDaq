#ifndef __PIXIE16RawReaderv4_H__
#define __PIXIE16RawReaderv4_H__
//
//  PIXIE16RawReaderv4.h
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/15/11.
//  Copyright 2011 ORNL. All rights reserved.
//
#include "NGMReaderBase.h"
#include "TArrayD.h"
#include "TTimeStamp.h"
#include <fstream>

class NGMBufferedPacket; //forward declaration
class NGMTimingCal; //forward declaration
class NGMHitPoolv6; //forward declaration

/// \brief PIXIE16RawReaderv4 is the reader implementation
///  for the raw PIXIE16 binary format
///
class PIXIE16RawReaderv4 : public NGMReaderBase
{
public:
  /// \brief Default Constructor required for ROOT
  PIXIE16RawReaderv4();
  /// \brief Constructor
  /// @param parent top module for which push method will be issued
  PIXIE16RawReaderv4(NGMModule* parent);
  /// \brief Dtor
  ~PIXIE16RawReaderv4();
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
  Long64_t ReadNextBufferFromFilev2();
  Long64_t readBytesWaiting(Long64_t nbytesToRead, unsigned int* rawbuffer, bool waitOnData = false, size_t localOffsetInBytes = 0);
  Long64_t writePacket(int packetlength, unsigned int *data, NGMTimingCal* _outlist, int moduleid);
  Long64_t OpenRawBinaryFile(const char* pathname);
  int AnaStats(NGMBufferedPacket* packet);
    
  bool MinDataPresentForMerge();
  void deleteRawBuffers();
  void createRawBuffers(int nmodules);
  bool pushHit(NGMHit* hit);
  
  Long64_t _nextbufferPosition;
  std::ifstream* _inputfile;
  TString _inputfilename;
  // One raw buffer per module to retain incomplete events
  unsigned int * _rawhdrbuffer; //!
  unsigned int _nrawbuffers; //!
  unsigned int ** _rawbuffer; //!
  unsigned int * _rawbufferstart; //!
  std::vector<int> _activeSlot; //!
  std::vector<NGMTimingCal*> _hitBuffers; //!
  NGMTimingCal* _hitStream; //!
  NGMHitPoolv6* _hitPool; //!

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
  double _minBufferDuration_ns;
  unsigned int _fileversion;
  
  ClassDef(PIXIE16RawReaderv4,2)
};



#endif //__PIXIE16RawReaderv4_H__
