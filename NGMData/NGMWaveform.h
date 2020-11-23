#ifndef __NGMWAVEFORM_H__
#define __NGMWAVEFORM_H__

/// \brief NGMWaveform contains the trace for a single channel for one acquisition.
///
#include "TObject.h"
#include "TArrayS.h"
#include "TH1.h"

//class TArrayS;  //Forward Declaration

class NGMArrayS : public TObject, public TArrayS
{
public:
	NGMArrayS(){}
	virtual ~NGMArrayS(){}

	ClassDef(NGMArrayS,1)
};

class NGMArraySFixed : public TObject
{
public:
	NGMArraySFixed();
	virtual ~NGMArraySFixed(){}
	int GetSize() const;
	void Set(int newCnt);
	void Set(int idx,short newVal);
	short At(int idx) const;
private:
	int _cnt;
	short _array[1000];

	ClassDef(NGMArraySFixed,1)
};

class NGMWaveform : public TObject
{

public:

   ///Constructor
   NGMWaveform() {}

   ///Destructor
  virtual ~NGMWaveform() {}
  /// \brief Get the time of first sample
  virtual Long64_t GetT0() const { return 0; }
  /// \brief Get the time of first sample
  virtual void SetT0(Long64_t) {}
  /// \brief Get the time between samples in ns
  virtual double GetDt() const { return 0.0; }
  /// \brief Set the time between samples in ns
  virtual void SetDt(double) {}
  /// \brief Get the index of the hardware trigger sample
  virtual int GetTrig() const { return 0; }
  /// \brief Get the index of the hardware trigger sample
  virtual void SetTrig(int) {}
  /// \breif Get the number of samples in the waveform
  virtual int GetNSamples() const { return 0; }
  /// \breif Set the number of samples in the waveform
  virtual void SetNSamples(int) {}
  
  /// \brief Get the ith sample in the waveform
  virtual short GetSample(int /*isample*/) const { return 0;}
  /// \brief Set the ith sample in the waveform
  virtual void SetSample(int /*isample*/, short /*newVal*/){}
  /// \breif Get the ith sample in the waveform converted to voltage
  virtual double GetVoltage(int /*isample*/) const { return 0.0; }
  /// \breif Duplicate Waveform
  virtual NGMWaveform* DuplicateWaveform() = 0;
  /// \breif Copy Wavform to this object
  virtual void CopyWaveform(const NGMWaveform* oldWave);

  
  ClassDef(NGMWaveform,1)
};


class NGMWaveformv1 : public NGMWaveform
{
  
public:
  
  ///Constructor
  NGMWaveformv1();
  
  ///Destructor
  virtual ~NGMWaveformv1();
  /// \brief Get the time of first sample
  Long64_t GetT0() const { return _t0; }
  void SetT0(Long64_t newVal) { _t0 = newVal; }
  /// \brief Get the time between samples
  double GetDt() const {return _dt; }
  void SetDt(double newVal) { _dt = newVal; }
  /// \brief Get the index of the hardware trigger sample
  int GetTrig() const { return _trigsample; }
  void SetTrig(int newVal) { _trigsample = newVal; }
  /// \breif Get the number of samples in the waveform
  int GetNSamples() const;
  void SetNSamples(int newVal);
  
  /// \brief Get the ith sample in the waveform
  short GetSample(int isample) const;
  void SetSample(int isample, short newVal);
  /// \breif Get the ith sample in the waveform converted to voltage
  double GetVoltage(int /*isample*/) const {return 0.0;}
  /// \breif Duplicate Waveform
  virtual NGMWaveform* DuplicateWaveform();
  
private:
  Long64_t _t0;
  Double_t _dt;
  Int_t _trigsample;
  TArrayS* _sampleArray;
  
public:
  ClassDef(NGMWaveformv1,2)
};


#endif //__NGMSYSTEM_H__
