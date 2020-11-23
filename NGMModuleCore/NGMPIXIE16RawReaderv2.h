#ifndef __NGMPIXIE16RawReaderv2_H__
#define __NGMPIXIE16RawReaderv2_H__
//
//  NGMPIXIE16RawReaderv2.h
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

/// \brief NGMPIXIE16RawReaderv2 is the reader implementation
///  for the raw PIXIE16 binary format
///
class NGMPIXIE16RawReaderv2 : public NGMReaderBase
{
public:
  /// \brief Default Constructor required for ROOT
  NGMPIXIE16RawReaderv2();
  /// \brief Constructor
  /// @param parent top module for which push method will be issued
  NGMPIXIE16RawReaderv2(NGMModule* parent);
  /// \brief Dtor
  ~NGMPIXIE16RawReaderv2();
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
  Long64_t readBytesWaiting(Long64_t nbytesToRead, bool waitOnData = false, size_t localOffsetInBytes = 0);
  Long64_t writePacket(int packetlength, unsigned int *data, NGMBufferedPacket* &_outBuffer);
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
  
  ClassDef(NGMPIXIE16RawReaderv2,1)  
};



#endif //__NGMPIXIE16RawReaderv2_H__