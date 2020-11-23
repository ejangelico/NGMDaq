#ifndef __NGMPSDANALYZER_H__
#define __NGMPSDANALYZER_H__

#include "TNamed.h"
#include "TProfile.h"

class NGMHit;

class NGMPSDAnalyzer : public TNamed
{
public:
  NGMPSDAnalyzer();
  NGMPSDAnalyzer(const char* name, const char* title);
  virtual ~NGMPSDAnalyzer(){}
  
  void AddNeutron(const NGMHit* hit, double baseline = 0.0, double normalization = 1.0);
  void AddGamma(const NGMHit* hit, double baseline = 0.0, double normalization = 1.0);
  void SetWaveformParams(double nsPerSample,
                         double baselineBegin,
                         double baselineEnd);
  void ComputeGattiWeights();
  virtual double ComputePSD(const NGMHit* hit) { return 0.0; }
  void SetNGates(int ngates);
  void SetGates(int ngates, int* begin, int* length);
  void SetGate(int igate, double begin_ns, double length_ns);
  int GetGate(int igate);
  virtual void Print(Option_t* option);
  
  double nsPerSample;
  
  TProfile* nWaveform;
  TProfile* gWaveform;
  TProfile* gGattiWeights;
  TArrayI gbegin;
  TArrayI glength;
  //Recent calculated values
  TArrayI _gate;
  double normalization;
  double baseline;
  
  ClassDef(NGMPSDAnalyzer,1)
};

class NGMPSDFIS : public NGMPSDAnalyzer
{
public:
  NGMPSDFIS();
  NGMPSDFIS(const char* name, const char* title);
  double ComputePSD(const NGMHit* hit);
  
  ClassDef(NGMPSDFIS,1)
};

class NGMPSDDelayedGate : public NGMPSDAnalyzer
{
public:
  NGMPSDDelayedGate();
  NGMPSDDelayedGate(const char* name, const char* title);
  double ComputePSD(const NGMHit* hit);
  
  ClassDef(NGMPSDDelayedGate,1)
};

class NGMPSDGatti : public NGMPSDAnalyzer
{
public:
  NGMPSDGatti();
  NGMPSDGatti(const char* name, const char* title);
  double ComputePSD(const NGMHit* hit);
  void SetGattiWeights( double beginTime, double endTime, TProfile* gWeights = 0);
  double gattiBeginTime;
  double gattiEndTime;
  ClassDef(NGMPSDGatti,1)
};

class NGMPSDGattiTail : public NGMPSDAnalyzer
{
public:
  NGMPSDGattiTail();
  NGMPSDGattiTail(const char* name, const char* title);
  double ComputePSD(const NGMHit* hit);
  void SetGattiWeights( double beginTime, double endTime, TProfile* gWeights = 0);
  double gattiBeginTime;
  double gattiEndTime;
  ClassDef(NGMPSDGattiTail,1)
};

inline int NGMPSDAnalyzer::GetGate(int igate){
  return _gate[igate];
}

#endif // __NGMPSDANALYZER_H__
