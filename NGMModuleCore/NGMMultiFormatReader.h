#ifndef __NGMMULTIFORMATREADER_H__
#define __NGMMULTIFORMATREADER_H__
/*
 *  NGMMultiFormatReader.h
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/4/08.
 *  Copyright 2008 LLNL. All rights reserved.
 *
 */

#include "NGMSystem.h"
#include "TString.h"
#include "NGMTimingCal.h"

class NGMReaderBase;

/// \brief Top level reader module capable of determining appropriate reader

class NGMMultiFormatReader : public NGMSystem
{
public:
  
  /// \brief Constructor
  NGMMultiFormatReader();
  /// \brief Dtor
  ~NGMMultiFormatReader();
  
  /// \brief Open the input file
  ///
  virtual int OpenInputFile(const char*); // *MENU*
  
  /// \brief Close the input file
  virtual int CloseInputFile(); // *MENU*

  /// \brief Start the hardware acquisition
  /// 
  virtual int StartAcquisition(); // *MENU*
  
  /// \brief Start the hardware acquisition
  /// 
  virtual int RequestAcquisitionStop(); // *MENU*
  
  /// \brief Get Name Identifying this pass
  TString GetPassName() const { return _passname; }
  
  /// \brief Get Name Identifying this pass
  void SetPassName(TString passname) { _passname = passname; }
  
  /// \brief Request Update of all plots
  void RequestPlotUpdate();
  
  /// \brief Read a file as part of a sequence of identically configured runs.
  int ReadNextInSeries(const char* inputfilepath);
  
  /// \brief Set Maximum Run Time
  void SetMaxRunTime(Double_t maxruntime);

  virtual void GetStatus(TString& status);

  virtual void SetSkipWaveforms(bool newVal = true) {_skipWaveforms = newVal;}
    
  NGMTimingCal TimingCal;
    
private:
  TString _inputfilename;
  TString _filetype;
  TString _passname;
  bool _skipWaveforms = false;
    
private:
  NGMReaderBase* _reader; //!
  
  ClassDef(NGMMultiFormatReader,3)
};


#endif
