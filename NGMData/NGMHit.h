#ifndef __NGMHIT_H__
#define __NGMHIT_H__


/// \brief NGMHit provides the pulse to a single pulse or detector hit.
///

#include "TTimeStamp.h"
#include "TObject.h"
#include "NGMWaveform.h"
#include "TClonesArray.h"
#include "TArrayS.h"
#include "TArrayD.h"
#include <list>

class TGraph; // Forward declaration
class TH1; // Forward declaration

// Class NGMTimeStamp is a time stamp with subnanosecond timing
class NGMTimeStamp;


Bool_t operator==(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);
Bool_t operator!=(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);
Bool_t operator< (const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);
Bool_t operator<=(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);
Bool_t operator> (const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);
Bool_t operator>=(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);


class NGMTimeStamp : public TTimeStamp
{
public:
  NGMTimeStamp();
  NGMTimeStamp(time_t t, Int_t nsec, Int_t picosecs = 0);
  NGMTimeStamp(TTimeStamp ts);
  void SetPicoSecs(const int picosecs);
  int GetPicoSecs() const;
  double TimeDiffNanoSec(const NGMTimeStamp& baseTime) const;
  void IncrementNs(double time_ns);

friend Bool_t operator==(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);
friend Bool_t operator!=(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);
friend Bool_t operator< (const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);
friend Bool_t operator<=(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);
friend Bool_t operator> (const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);
friend Bool_t operator>=(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs);

private:
  void NormalizePicoSecs();
    Short_t _picosecs;
  
  ClassDef(NGMTimeStamp,1)
};

class NGMHit : public TObject
{

public:

  ///Constructor
  NGMHit(){}

  ///Destructor
  virtual ~NGMHit() {}
   
  /// \brief Set Slot
  virtual void SetSlot(int /*newVal*/) { return; }
  
  /// \brief Get Slot
  virtual int GetSlot() const { return -1; }

  /// \brief Set Channel
  virtual void SetChannel(int /*newVal*/) { return; }
  /// \brief Get Channel
  virtual int GetChannel() const { return -1;}

  
  /// \brief Set Pulse Height
  virtual void SetPulseHeight(double /*newVal*/){}
  
  /// \brief Get Pulse Height
  virtual double GetPulseHeight() const {return 0.0;} 
  
  /// \brief Time of pulse  
  virtual const TTimeStamp& GetTimeStamp() const ;
  /// \brief Set Time of pulse  
  virtual void SetTimeStamp(const TTimeStamp /*newVal*/) { }
  /// \brief Raw Time of pulse  
  virtual const NGMTimeStamp& GetRawTime() const ;
  /// \brief Set Raw Time of pulse  
  virtual void SetRawTime(const NGMTimeStamp newVal) { SetTimeStamp(TTimeStamp(newVal.GetSec(),newVal.GetNanoSec()));}
  /// \brief Raw Clock of pulse  
  virtual ULong64_t GetRawClock() const { return 0;}
  /// \brief Set Clock of pulse  
  virtual void SetRawClock(const ULong64_t /*newVal*/) { }
  /// \brief Get the Summed Value of Gate 
  virtual int GetGateCount() const { return 0; }
  /// \brief Get the Summed Value of Gate 
  virtual int GetGate(int /*index*/) const { return 0; }
  /// \brief Get the Summed Value of Quad Block Gate 
  virtual int GetQuadGate(int /*index*/) const { return 0; }
  /// \brief Set the Summed Value of Gate 
  virtual void SetGate(int /*index*/, int /*newVal*/){}
    
  virtual void SetGateSize(int /*newVal*/){}
    
  /// \brief Modify Associated Waveform
  virtual NGMWaveform* ModWaveform() {return 0;}
  /// \brief Read Only Access to Associated Waveform
  virtual const NGMWaveform* GetWaveform() const { return 0; }
  /// \brief Create New Waveform
  virtual NGMWaveform* CreateWaveform() { return 0; }
  /// \brief Delete Waveform
  virtual void DeleteWaveform() {}

  /// \brief Set PileUp Counter
  virtual void SetPileUpCounter(int /*newVal*/){}
  /// \brief Get PileUp Counter
  virtual int GetPileUpCounter() const { return -1;}
  /// \brief Set CFD Word Upper 8bits for sample number, Lower 8 bits for fraction of sample period
  virtual void SetCFDWord(unsigned short /*newVal*/) {}
  /// \brief Get CFD Word Upper 8bits for sample number, Lower 8 bits for fraction of sample period
  virtual unsigned short GetCFDWord() const {return 0;}
  /// \brief Set CFD Timing in clock units
  virtual void SetCFD(Float_t /*newVal*/) {}
  /// \brief Get CFD Timing in clock units
  virtual Float_t GetCFD() const {return 0;}
  /// \brief Set the PulseShapeDiscrimination Parameter
  virtual void SetPSD(Float_t /*newVal*/) {}
  /// \brief Get the PulseShapeDiscrimination Parameter
  virtual Float_t GetPSD() const { return 0.0; }
  /// \brief Set the TOF1 Parameter
  virtual void SetTOF1(Float_t /*newVal*/) {}
  /// \brief Get the PulseShapeDiscrimination Parameter
  virtual Float_t GetTOF1() const { return 0.0; }
  /// \brief Set the TOF2 Parameter
  virtual void SetTOF2(Float_t /*newVal*/) {}
  /// \brief Get the PulseShapeDiscrimination Parameter
  virtual Float_t GetTOF2() const { return 0.0; }
  /// \brief Get Time Difference
  virtual double TimeDiffNanoSec(const TTimeStamp& /*baseTime*/) const;
  /// \brief Get Time Difference
  virtual double TimeDiffNanoSec(const NGMTimeStamp& baseTime) const;
  
  /// \brief Get the number of samples in the waveform
  virtual int GetNSamples() const {return 0;}
  /// \brief Set the number of samples in the waveform
  virtual void SetNSamples(int /*newVal*/){ return; }
  
  /// \brief Get the ith sample in the waveform
  virtual int GetSample(int /*isample*/) const {return 0;}
  /// \brief Set the ith sample in the waveform
  virtual void SetSample(int /*isample*/, int /*newVal*/) { return; }
  /// \brief Get the waveform array
  virtual int* GetWaveformArray() { return 0; }
  /// \brief Get the root graph
  virtual TGraph* GetGraph() const;
  /// \brief Compute the sum of from the the waveform
  virtual int ComputeSum(int sumbegin, int sumlength) const;

  /// \brief Copy values from NGMHit object 
  virtual void CopyHit(const NGMHit*);
  /// \brief Create a new Copy of Hit
  virtual NGMHit* DuplicateHit() const {return 0;}
  /// \brief Compare to another object -1 lt, 0 eq, 1 gt
  virtual Int_t Compare(const TObject*) const {return 0;}
  
  virtual NGMTimeStamp GetCalibratedTime() const;
  virtual NGMTimeStamp GetNGMTime() const;
  virtual NGMTimeStamp& GetNGMTime() = 0;
  virtual void SetCalibratedTime(NGMTimeStamp ts);
  virtual void SetEnergy(float newVal);
  virtual float GetEnergy() const ;
  virtual float GetNeutronId() const;
  virtual void SetNeutronId(float newVal);
  virtual float GetGammaId() const;
  virtual void SetGammaId(float newVal);
  virtual float GetBlockX() const;
  virtual void SetBlockX(float newVal);
  virtual float GetBlockY() const;
  virtual void SetBlockY(float newVal);
  virtual Int_t GetPixel() const {return -1;}
  virtual void SetPixel(Int_t /*newVal*/){}

  
  virtual Bool_t IsSortable() const {return true;}
  virtual void	Print(Option_t* option = "") const;
  virtual void  DisplayWaveform(int option = 0, int option2 =0,Option_t* chopt = "") const;
  virtual int CalcPileup(int peaktime, int gaptime, double threshold, int begin = 0, int end =-1) const;
  virtual int CountLETriggers(int baselineBegin, int baselineLength, double threshold, int begin, int end,int smooth = 0) const;
  virtual int CountLETriggersList(int baselineBegin, int baselineLength, double threshold, int begin, int end, int smooth, TArrayD& ph, TArrayD& ts) const;
  virtual int FastFilterZCPList( int risetime, int flattop,int threshold,int begin,int end, TArrayD& pt) const;
  virtual TH1* DisplayNormalizedIntegral(int baselinestart, int baselinelength, int integralstart, int integrallength) const;
  virtual void getPeakPosition( int &peakPos, int &peakVal) const;
  virtual int getMaxValue() const { return 0;}
  virtual void simpleQuadFit(double* x, double* y, double &xmax, double& ymax) const;
  virtual double getQuadTiming( int peakPos, double baseline, double& quadpeak) const;
  virtual double calcTimeToFraction( double tfrac, int baselineLength, int pulseLength, int prePeakSamples = 20, int baselineStart = 0) const;
  virtual NGMHit* FastFilter( int risetime, int flattop = 0, double cfdweight = 0.0) const;
  virtual NGMHit* FastFilter2( int risetime, int flattop = 0) const;
  virtual Float_t ZCPBipolar() const;
  virtual Float_t ZCPBipolarF(double fraction=0.5) const;
  virtual Float_t ZCPBipolarFF(int risetime, int flattop=0) const;
  NGMHit* FastFilterXIA(int risetime, int flattop, int CFDdelay, int cfdweight,int nwaveforms=1) const;
  virtual NGMHit* correctTailDecay(int g0begin, int g0length, double decaytime, int sbegin, int send = -1) const;

  ClassDef(NGMHit,1)
};
#ifdef NGMHITV1
class NGMHitv1 : public NGMHit
{
  
public:
  
  ///Constructor
  NGMHitv1();
  
  ///Destructor
  virtual ~NGMHitv1();
  
  
  void SetSlot(int newVal) {_slot = newVal;}
  int GetSlot() const { return _slot; }
  void SetChannel(int newVal) {_channel = newVal;}
  int GetChannel() const { return _channel; }
  void SetPulseHeight(double newVal) {_pulseHeight = newVal;}
  double GetPulseHeight() const { return _pulseHeight; }
  void SetTimeStamp(const TTimeStamp newVal);
  const TTimeStamp& GetTimeStamp() const { return _triggerTime; }
  int GetGate(int index) const ;
  void SetGate(int index, int newVal);
  virtual void SetPileUpCounter(int newVal);
  virtual int GetPileUpCounter() const;
  int GetGateCount() const { return 8; }
  /// \brief Modify Associated Waveform
  virtual NGMWaveform* ModWaveform() {return _waveform; }
  /// \brief Read Only Access to Associated Waveform
  virtual const NGMWaveform* GetWaveform() const {return _waveform; }
  /// \brief Create New Waveform
  virtual NGMWaveform* CreateWaveform();
  /// \brief Delete Waveform
  virtual void DeleteWaveform();
  /// \brief Get the number of samples in the waveform
  virtual int GetNSamples() const;
  /// \brief Set the number of samples in the waveform
  virtual void SetNSamples(int newVal);

  /// \brief Get the ith sample in the waveform
  virtual int GetSample(int isample) const;
  /// \brief Set the ith sample in the waveform
  virtual void SetSample(int isample, int newVal);

  virtual NGMTimeStamp GetNGMTime() const;
  virtual NGMTimeStamp& GetNGMTime();

  virtual Int_t Compare(const TObject*) const;
  virtual NGMHit* DuplicateHit() const;
  
private:
    
  Int_t _slot;
  Int_t _channel;
  Double_t _pulseHeight;
  TTimeStamp _triggerTime;
  Int_t _gate[8];
  NGMWaveform* _waveform;
  Int_t _pileupcounter;
  
public:
  
    ClassDef(NGMHitv1,2)
};
#endif

#ifdef NGMHITV2
/// Second Raw Data Format
class NGMHitv2 : public NGMHit
{
  
public:
  
  ///Constructor
  NGMHitv2();
  
  ///Destructor
  virtual ~NGMHitv2();
  
  
  void SetSlot(int newVal) {_slot = newVal;}
  int GetSlot() const { return _slot; }
  void SetChannel(int newVal) {_channel = newVal;}
  int GetChannel() const { return _channel; }
  void SetPulseHeight(double newVal) {_pulseHeight = newVal;}
  double GetPulseHeight() const { return _pulseHeight; }
  void SetTimeStamp(const TTimeStamp newVal);
  const TTimeStamp& GetTimeStamp() const { return _triggerTime; }
  NGMTimeStamp GetNGMTime() const;
  NGMTimeStamp& GetNGMTime();
  int GetGateCount() const { return 8; }
  int GetGate(int index) const ;
  void SetGate(int index, int newVal);
  /// \brief Modify Associated Waveform
  virtual NGMWaveform* ModWaveform() {return 0; }
  /// \brief Read Only Access to Associated Waveform
  virtual const NGMWaveform* GetWaveform() const {return 0; }
  virtual void SetPileUpCounter(int newVal);
  virtual int GetPileUpCounter() const;
  virtual void SetCFDWord(unsigned short newVal) {_cfdtiming = newVal;}
  virtual unsigned short GetCFDWord() const { return _cfdtiming; }
  /// \brief Create New Waveform
  virtual NGMWaveform* CreateWaveform(){ return 0; }
  int GetNSamples() const;
  void SetNSamples(int newVal);
  
  /// \brief Get the ith sample in the waveform
  int GetSample(int isample) const;
  void SetSample(int isample, int newVal);

  /// \brief Copy optimized for this version of the class
  virtual void CopyHit(const NGMHit* hit);

  /// \brief Delete Waveform
  virtual void DeleteWaveform(){}
  
  virtual Int_t Compare(const TObject*) const;
  virtual NGMHit* DuplicateHit() const;
  
private:
    
  Int_t _slot;
  Int_t _channel;
  Double_t _pulseHeight;
  TTimeStamp _triggerTime;
  Int_t _gate[8];
  NGMArrayS _waveform;
  Int_t _pileupcounter;
  UShort_t _cfdtiming; // 8bits for cfd sample number from beginning of waveform, 8bits for fraction of sample width
  
public:
  
    ClassDef(NGMHitv2,2)
};

#endif
#ifdef NGMHITV3
// First Calibrated Data format
class NGMHitv3 : public NGMHitv2
{
  
public:
  
  ///Constructor
  NGMHitv3();
  
  ///Destructor
  virtual ~NGMHitv3(){}
  
  /// \brief Copy optimized for this version of the class
  virtual void CopyHit(const NGMHit* hit);
  
  virtual Int_t Compare(const TObject*) const;
  virtual NGMHit* DuplicateHit() const;
  virtual NGMTimeStamp GetNGMTime() const;  
  virtual NGMTimeStamp& GetNGMTime();
  virtual void SetCalibratedTime(NGMTimeStamp ts);
  virtual NGMTimeStamp GetCalibratedTime() const;
  virtual void SetEnergy(float newVal);
  virtual float GetEnergy() const ;
  virtual float GetNeutronId() const;
  virtual void SetNeutronId(float newVal);
  virtual float GetGammaId() const;
  virtual void SetGammaId(float newVal);

private:
    
  // Add some calculated values to be stored in memory
  Float_t _energy;
  NGMTimeStamp _calTime;
  Float_t _neutronSigma;
  Float_t _gammaSigma;
  
public:
    
    ClassDef(NGMHitv3,1)
};
#endif
/// Third Raw Data Format includes pico second precision in raw time stamp
/// since some clock choices result in non integer nanoseconds per clock.
class NGMHitv4 : public NGMHit
{
  
public:
  
  ///Constructor
  NGMHitv4();
  
  ///Destructor
  virtual ~NGMHitv4();
  
  
  void SetSlot(int newVal) {_slot = newVal;}
  int GetSlot() const { return _slot; }
  void SetChannel(int newVal) {_channel = newVal;}
  int GetChannel() const { return _channel; }
  void SetPulseHeight(double newVal) {_pulseHeight = newVal;}
  double GetPulseHeight() const { return _pulseHeight; }
  void SetTimeStamp(const TTimeStamp newVal);
  const TTimeStamp& GetTimeStamp() const { return _triggerTime; }
  void SetRawTime(const NGMTimeStamp newVal);
  const NGMTimeStamp& GetRawTime() const { return _triggerTime; }
  NGMTimeStamp GetNGMTime() const;
  NGMTimeStamp& GetNGMTime();
  int GetGateCount() const { return 8; }
  int GetGate(int index) const ;
  void SetGate(int index, int newVal);
  /// \brief Modify Associated Waveform
  virtual NGMWaveform* ModWaveform() {return 0; }
  /// \brief Read Only Access to Associated Waveform
  virtual const NGMWaveform* GetWaveform() const {return 0; }
  virtual void SetPileUpCounter(int newVal);
  virtual int GetPileUpCounter() const;
  virtual void SetCFDWord(unsigned short newVal) {_cfdtiming = newVal;}
  virtual unsigned short GetCFDWord() const { return _cfdtiming; }
  /// \brief Create New Waveform
  virtual NGMWaveform* CreateWaveform(){ return 0; }
  int GetNSamples() const;
  void SetNSamples(int newVal);
  
  /// \brief Get the ith sample in the waveform
  int GetSample(int isample) const;
  void SetSample(int isample, int newVal);

  /// \brief Copy optimized for this version of the class
  virtual void CopyHit(const NGMHit* hit);

  /// \brief Delete Waveform
  virtual void DeleteWaveform(){}
  
  virtual Int_t Compare(const TObject*) const;
  virtual NGMHit* DuplicateHit() const;

  virtual ULong64_t GetRawClock() const { return _rawclock;}
  virtual void SetRawClock(const ULong64_t newVal) { _rawclock = newVal; }
  virtual int getMaxValue() const ;
  
private:
    
  Int_t _slot;
  Int_t _channel;
  Float_t _pulseHeight;
  Int_t _gate[8];
  NGMArrayS _waveform;
  UShort_t _pileupcounter;
  UShort_t _cfdtiming; // 8bits for cfd sample number from beginning of waveform, 8bits for fraction of sample width
  NGMTimeStamp _triggerTime;
  ULong64_t _rawclock;

public:
  
    ClassDef(NGMHitv4,1)
};

// First Calibrated Data format
class NGMHitv5 : public NGMHitv4
{
  
public:
  
  ///Constructor
  NGMHitv5();
  
  ///Destructor
  virtual ~NGMHitv5(){}
  
  /// \brief Copy optimized for this version of the class
  virtual void CopyHit(const NGMHit* hit);
  
  virtual Int_t Compare(const TObject*) const;
  virtual NGMHit* DuplicateHit() const;
  virtual NGMTimeStamp GetNGMTime() const;  
  virtual NGMTimeStamp& GetNGMTime();
  virtual void SetCalibratedTime(NGMTimeStamp ts);
  virtual NGMTimeStamp GetCalibratedTime() const;
  virtual void SetEnergy(float newVal);
  virtual float GetEnergy() const ;
  virtual float GetNeutronId() const;
  virtual void SetNeutronId(float newVal);
  virtual float GetGammaId() const;
  virtual void SetGammaId(float newVal);

private:
    
  // Add some calculated values to be stored in memory
  Float_t _energy;
  NGMTimeStamp _calTime;
  Float_t _neutronSigma;
  Float_t _gammaSigma;
  
public:
    
    ClassDef(NGMHitv5,1)
};

// First Calibrated Data format
class NGMHitv6 : public NGMHit
{
  
public:
  
  ///Constructor
  NGMHitv6();
  NGMHitv6(int ngates);
  ///Destructor
  virtual ~NGMHitv6(){}
  void SetSlot(int newVal) {_slot = newVal;}
  int GetSlot() const { return _slot; }
  void SetChannel(int newVal) {_channel = newVal;}
  int GetChannel() const { return _channel; }

  /// \brief Copy optimized for this version of the class
  virtual void CopyHit(const NGMHit* hit);
  
  virtual Int_t Compare(const TObject*) const;
  virtual NGMHit* DuplicateHit() const;
  virtual NGMTimeStamp GetNGMTime() const;  
  virtual NGMTimeStamp& GetNGMTime();
  virtual void SetCalibratedTime(NGMTimeStamp ts);
  virtual NGMTimeStamp GetCalibratedTime() const;
  virtual void SetEnergy(float newVal);
  virtual float GetEnergy() const ;
  virtual float GetNeutronId() const;
  virtual void SetNeutronId(float newVal);
  virtual float GetGammaId() const;
  virtual void SetGammaId(float newVal);
  virtual float GetBlockX() const;
  virtual void SetBlockX(float newVal);
  virtual float GetBlockY() const;
  virtual void SetBlockY(float newVal);
  virtual Int_t GetPixel() const { return _pixel;} 
  virtual void SetPixel(Int_t newVal){ _pixel = newVal;}
  
  void SetTimeStamp(const TTimeStamp newVal){ _calTime = newVal; }
  const TTimeStamp& GetTimeStamp() const { return _calTime; }
  void SetRawTime(const NGMTimeStamp newVal){ _calTime = newVal; }
  const NGMTimeStamp& GetRawTime() const { return _calTime; }
  int GetGateCount() const { return _gate.GetSize(); }
  int GetGate(int index) const ;
  void SetGate(int index, int newVal);
  void SetGateSize(int newVal) { _gate.Set(newVal); }
  /// \brief Modify Associated Waveform
  virtual NGMWaveform* ModWaveform() {return 0; }
  /// \brief Read Only Access to Associated Waveform
  virtual const NGMWaveform* GetWaveform() const {return 0; }
  virtual void SetPileUpCounter(int newVal);
  virtual int GetPileUpCounter() const;
  virtual void SetCFD(Float_t newVal) {_cfdtiming = newVal;}
  virtual Float_t GetCFD() const { return _cfdtiming; }
  virtual void SetPSD(Float_t newVal) {_psd = newVal;}
  virtual Float_t GetPSD() const { return _psd; }
  virtual void SetPulseHeight(double newVal){_pulseHeight = newVal;}
  virtual double GetPulseHeight() const {return _pulseHeight;} 
  virtual int GetQuadGate(int index) const;

  /// \brief Create New Waveform
  virtual NGMWaveform* CreateWaveform(){ return 0; }
  int GetNSamples() const;
  void SetNSamples(int newVal);
  /// \brief Get the waveform array
  int* GetWaveformArray() { return _waveform.GetArray(); }
  
  /// \brief Get the ith sample in the waveform
  int GetSample(int isample) const;
  void SetSample(int isample, int newVal);
    
  /// \brief Delete Waveform
  virtual void DeleteWaveform(){}
  
  virtual ULong64_t GetRawClock() const { return _rawclock;}
  virtual void SetRawClock(const ULong64_t newVal) { _rawclock = newVal; }
  virtual int getMaxValue() const ;
  
private:
  
  // Add some calculated values to be stored in memory
  Int_t _slot;
  Int_t _channel;
  ULong64_t _rawclock;
  Float_t _cfdtiming; // Units of Clock
  Float_t _pulseHeight;
  Float_t _energy;
  NGMTimeStamp _calTime;
  Float_t _neutronSigma;
  Float_t _gammaSigma;
  Float_t _x;
  Float_t _y;
  Int_t _pileupcounter;
  TArrayI _waveform;
  TArrayI _gate;
  Int_t _pixel;
  Float_t _psd;
  
public:
  
  ClassDef(NGMHitv6,3)
};

// First Calibrated Data format
class NGMHitv7 : public NGMHit
{
    
public:
    
    ///Constructor
    NGMHitv7();
    ///Destructor
    virtual ~NGMHitv7(){}
    
    /// \brief Copy optimized for this version of the class
    virtual void CopyHit(const NGMHit* hit);
    
    virtual Int_t Compare(const TObject*) const;
    virtual NGMHit* DuplicateHit() const;
    virtual void SetEnergy(float newVal);
    virtual float GetEnergy() const ;
    virtual float GetNeutronId() const;
    virtual void SetNeutronId(float newVal);
    virtual float GetGammaId() const;
    virtual void SetGammaId(float newVal);
    virtual Int_t GetPixel() const { return _pixel;}
    virtual void SetPixel(Int_t newVal){ _pixel = newVal;}
    virtual NGMTimeStamp GetNGMTime() const;
    virtual NGMTimeStamp& GetNGMTime();
    
    /// \brief Read Only Access to Associated Waveform
    virtual void SetPileUpCounter(int newVal);
    virtual int GetPileUpCounter() const;
    virtual void SetPSD(Float_t newVal) {_psd = newVal;}
    virtual Float_t GetPSD() const { return _psd; }
    
    /// \brief Create New Waveform
    virtual void SetTOF1(Float_t newVal) {_tof1=newVal;}
    virtual Float_t GetTOF1() const { return _tof1; }
    virtual void SetTOF2(Float_t newVal) {_tof2=newVal;}
    virtual Float_t GetTOF2() const { return _tof2; }

    /// \brief Create New Waveform
    virtual NGMWaveform* CreateWaveform(){ return 0; }
        
private:
    
    // Add some calculated values to be stored in memory
    Float_t _energy;
    Float_t _neutronSigma;
    Float_t _gammaSigma;
    Int_t _pileupcounter;
    Int_t _pixel;
    NGMTimeStamp _timestamp;
    Float_t _psd;
    Float_t _tof1;
    Float_t _tof2;
public:
    
    ClassDef(NGMHitv7,4)
};


// Copied from NGMHitv6 with different wfm data type, for Stanford charge-readout setup
class NGMHitv8 : public NGMHit
{
  
public:
  
  ///Constructor
  NGMHitv8();
  NGMHitv8(int ngates);
  ///Destructor
  virtual ~NGMHitv8(){}
  void SetSlot(int newVal) {_slot = newVal;}
  int GetSlot() const { return _slot; }
  void SetChannel(int newVal) {_channel = newVal;}
  int GetChannel() const { return _channel; }

  /// \brief Copy optimized for this version of the class
  virtual void CopyHit(const NGMHit* hit);
  
  virtual Int_t Compare(const TObject*) const;
  virtual NGMHit* DuplicateHit() const;
  virtual NGMTimeStamp GetNGMTime() const;  
  virtual NGMTimeStamp& GetNGMTime();
  virtual void SetCalibratedTime(NGMTimeStamp ts);
  virtual NGMTimeStamp GetCalibratedTime() const;
  virtual void SetEnergy(float newVal);
  virtual float GetEnergy() const ;
  virtual float GetNeutronId() const;
  virtual void SetNeutronId(float newVal);
  virtual float GetGammaId() const;
  virtual void SetGammaId(float newVal);
  virtual float GetBlockX() const;
  virtual void SetBlockX(float newVal);
  virtual float GetBlockY() const;
  virtual void SetBlockY(float newVal);
  virtual Int_t GetPixel() const { return _pixel;} 
  virtual void SetPixel(Int_t newVal){ _pixel = newVal;}
  
  void SetTimeStamp(const TTimeStamp newVal){ _calTime = newVal; }
  const TTimeStamp& GetTimeStamp() const { return _calTime; }
  void SetRawTime(const NGMTimeStamp newVal){ _calTime = newVal; }
  const NGMTimeStamp& GetRawTime() const { return _calTime; }
  int GetGateCount() const { return _gate.GetSize(); }
  int GetGate(int index) const ;
  void SetGate(int index, int newVal);
  void SetGateSize(int newVal) { _gate.Set(newVal); }
  /// \brief Modify Associated Waveform
  virtual NGMWaveform* ModWaveform() {return 0; }
  /// \brief Read Only Access to Associated Waveform
  virtual const NGMWaveform* GetWaveform() const {return 0; }
  virtual void SetPileUpCounter(int newVal);
  virtual int GetPileUpCounter() const;
  virtual void SetCFD(Float_t newVal) {_cfdtiming = newVal;}
  virtual Float_t GetCFD() const { return _cfdtiming; }
  virtual void SetPSD(Float_t newVal) {_psd = newVal;}
  virtual Float_t GetPSD() const { return _psd; }
  virtual void SetPulseHeight(double newVal){_pulseHeight = newVal;}
  virtual double GetPulseHeight() const {return _pulseHeight;} 
  virtual int GetQuadGate(int index) const;

  /// \brief Create New Waveform
  virtual NGMWaveform* CreateWaveform(){ return 0; }
  int GetNSamples() const;
  void SetNSamples(int newVal);
  /// \brief Get the waveform array
  int* GetWaveformArray() { return &(_waveform[0]); }
  
  /// \brief Get the ith sample in the waveform
  int GetSample(int isample) const;
  void SetSample(int isample, int newVal);
    
  /// \brief Delete Waveform
  virtual void DeleteWaveform(){}
  
  virtual ULong64_t GetRawClock() const { return _rawclock;}
  virtual void SetRawClock(const ULong64_t newVal) { _rawclock = newVal; }
  virtual int getMaxValue() const ;
  
private:
  
  // Add some calculated values to be stored in memory
  Int_t _slot;
  Int_t _channel;
  ULong64_t _rawclock;
  Float_t _cfdtiming; // Units of Clock
  Float_t _pulseHeight;
  Float_t _energy;
  NGMTimeStamp _calTime;
  Float_t _neutronSigma;
  Float_t _gammaSigma;
  Float_t _x;
  Float_t _y;
  Int_t _pileupcounter;
  std::vector<int> _waveform;
  TArrayI _gate;
  Int_t _pixel;
  Float_t _psd;
  
public:
  
  ClassDef(NGMHitv8,1)
};



class NGMHitPoolv5 : public TObject
{
private:
  NGMHitPoolv5();
  static NGMHitPoolv5* _ptr;
  
public:
  NGMHitPoolv5 *instance();
  
  TClonesArray a;
  TArrayS cnt;
  
  ClassDef(NGMHitPoolv5,1);  
};

class NGMHitPoolv6 : public TObject
{
public:
    NGMHitPoolv6();
    virtual ~NGMHitPoolv6();
    NGMHit* GetHit();
    void ReturnHit(NGMHit* hit);

private:
    std::list<NGMHit*> _hitreservoir;
    ClassDef(NGMHitPoolv6,0);
};

class NGMHitRefv5 : public NGMHit
{
  public:
  
  private:
  unsigned int idx;
  
  ClassDef(NGMHitRefv5,1);
};

#endif //__NGMHIT_H__
