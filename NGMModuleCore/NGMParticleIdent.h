#ifndef __PARTICLEIDENT__
#define __PARTICLEIDENT__

#include "TObject.h"
#include "TString.h"
#include "Gtypes.h"
#include <vector>
class NGMHit;
class NGMSystemConfiguration;
class TLegend;

struct partSelect_st
{
    int partid;
    double energymin;
    double energymax;
};

class partSelect_rst : public TObject
{
public:
  partSelect_rst();
  virtual ~partSelect_rst(){}
  
  int partid;
  double energymin;
  double energymax;
  ClassDef(partSelect_rst,1);
};

class NGMParticleIdent: public TObject{
 public:
  NGMParticleIdent() {}
  virtual ~NGMParticleIdent() {}

  virtual bool Init(const NGMSystemConfiguration* confBuffer) {return false;}
  virtual void AddCut(int tpartid, double tminenergy, double tmaxenergy){}
  virtual bool IsSelected(const NGMHit* tHit) const {return false;}
  virtual int GetType(const NGMHit* tHit) const {return 0;}
  virtual TString GetName(const NGMHit* tHit) const {return TString("");}
  virtual Style_t GetStyle(const NGMHit* tHit) const {return 0;}
  virtual Color_t GetColor(const NGMHit* tHit) const {return 0;}
  virtual Style_t GetStyle(int type) const {return 0;}
  virtual Color_t GetColor(int type) const {return 0;}
  virtual int GetMaxParticleTypes() const {return 0;}
  virtual const char* GetParticleName(int type) const { return ""; }
  virtual TLegend* GetLegend(Double_t x1, Double_t y1, Double_t x2, Double_t y2, const char* header = "", Option_t* option = "brNDC"){return 0;}
  virtual TString* GetLabel(){return 0;}
  virtual int getPlotIndex(const NGMHit* tHit) const { return 0; }
  virtual int getPlotIndex(const char* cDetName) const {return -1;}
  virtual int getGateOffset(int ichan, int igate) const {return 0; }
  virtual int getGateWidth(int ichan, int igate) const {return 0; }
  virtual int getNumberOfChannels() const { return 0;}
  virtual TString getChannelName(int ichan) const {return TString("");}
  virtual int GetDetectorType(int ichan) const {return -1;}
  virtual void setNeutronIdParams(float NeutronIdLow, float NeutronIdHigh, float gammaRejHigh){}
  virtual void setNeutronPileUpRejection(bool setPileUp = true){}
  virtual void setTailTest(double min, double max){}

  ClassDef(NGMParticleIdent,2);
};

#endif


