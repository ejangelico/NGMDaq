/*
 *  NGMMultiFormatReader.cpp
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/4/08.
 *  Copyright 2008 LLNL. All rights reserved.
 *
 */

#include "NGMMultiFormatReader.h"
#include "NGMReaderBase.h"
#include "NGMSystemConfiguration.h"
#include "TSystem.h"
#include "PIXIE16RawReaderv3.h"

ClassImp(NGMMultiFormatReader)

NGMMultiFormatReader::NGMMultiFormatReader()
{
  SetName("MultiFormatReader");
  SetTitle("MultiFormatReader");
  _reader = 0;
}

NGMMultiFormatReader::~NGMMultiFormatReader()
{
  if(_reader) delete _reader;
}

/// \brief Set Maximum Run Time
void NGMMultiFormatReader::SetMaxRunTime(Double_t maxruntime)
{
    if(_reader) _reader->SetMaxRunTime(maxruntime);
}


int NGMMultiFormatReader::RequestAcquisitionStop()
{
  printf("Requesting Acquisition Stop!\n");
  if(_reader) _reader->AbortRead();
  
  return 0;
}

/// \brief Request Update of all plots
void NGMMultiFormatReader::RequestPlotUpdate()
{
  if(_reader) _reader->RequestPlotUpdate();
}

int NGMMultiFormatReader::OpenInputFile(const char* filename)
{
  // Delete Previous reader
  if(_reader) delete _reader;
  
  _inputfilename = filename;
  if(_inputfilename.EndsWith(".root") && !_inputfilename.EndsWith("-conf.root"))
  {
    _filetype = "ROOT";
  }else if(_inputfilename.Contains("GageRaw")){
    _filetype = "GAGERaw";
  }else if(TString(gSystem->BaseName(_inputfilename)).Contains("ORNL")){
    _filetype = "PIXIE16ORNL";
  }else if(_inputfilename.Contains("Pixie16Raw")){
      _filetype = "PIXIE16Rawv4";
  }else if(TString(gSystem->BaseName(_inputfilename)).BeginsWith("SIS3305")){
    _filetype = "SIS3305";
  }else if(TString(gSystem->BaseName(_inputfilename)).BeginsWith("SIS3316Raw")){
      _filetype = "SIS3316Raw";
  }else if(TString(gSystem->BaseName(_inputfilename)).BeginsWith("SIS3302Raw")){
      _filetype = "SIS3302Raw";
  }else{
    _filetype = "SISRaw";
  }
  _reader= NGMReaderBase::Factory(_filetype, this);
  TimingCal.clear();
  _reader->SetTimingCal(&TimingCal);
  _reader->OpenInputFile(_inputfilename);
  _reader->SetSkipWaveforms(_skipWaveforms);
    
  if(_reader->GetConfiguration())
  {
    _config = _reader->GetConfiguration()->DuplicateConfiguration();
    // Only send the configuration object if you want to add histograms
    if(_reader->GetSeriesCount() == 0)
      push(*((const TObject*) _config));
  }
  
  // Reset the base class counter for number of times the
  // analysis tree has been saved.
  NGMSystem::_treeSaveSequence = 0;
  
  return 0;
}

int NGMMultiFormatReader::CloseInputFile()
{
  if(!_reader) return 1;
  
  _reader->SendEndRun();
  _reader->CloseInputFile();
  
  delete _reader;
  _reader = 0;
  delete _config;
  _config = 0;
  return 0;
}

void NGMMultiFormatReader::GetStatus(TString& status)
{
    if(!_reader)
    {
        status = "No Open File\n";
    }else{
        _reader->GetStatus(status);
    }
    return;
}


int NGMMultiFormatReader::StartAcquisition()
{
  if(!_reader) return 1;
  SetStopDaqOnCtlC();
  _reader->ReadAll();
  _reader->SendEndRun();
  _reader->SendPlotUpdate();
  ResetCtlC();
  return 0;
}

int NGMMultiFormatReader::ReadNextInSeries(const char* inputfilepath)
{

  _inputfilename = inputfilepath;
  if(!_reader){
    
    if(_inputfilename.EndsWith(".root") && !_inputfilename.EndsWith("-conf.root"))
    {
      _filetype = "ROOT";
    }else{
      _filetype = "SISRaw";
    }
    _reader= NGMReaderBase::Factory(_filetype, this);
  }else{
    // We should check that reader is really the correct type of reader
  }
   _reader->ReadNextInSeries(_inputfilename.Data());
  if( _reader->GetSeriesCount() == 1)
  {
    delete _config;
      _config = _reader->GetConfiguration()->DuplicateConfiguration();
  }

  return 0;
}

