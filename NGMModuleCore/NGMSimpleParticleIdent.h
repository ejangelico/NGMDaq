#ifndef __NGMSIMPLEPARTICLEIDENT__
#define __NGMSIMPLEPARTICLEIDENT__

#include "TObject.h"
#include "TObjArray.h"
#include "TString.h"
#include "Gtypes.h"
#include <vector>
#include "TArrayS.h"
#include "TArrayD.h"
#include "NGMParticleIdent.h"

class NGMHit;
class NGMSystemConfiguration;
class TLegend;
class partSelect_rst ;

class NGMSimpleParticleIdent: public NGMParticleIdent{
 public:
  NGMSimpleParticleIdent();
  virtual ~NGMSimpleParticleIdent() {}

  bool Init(const NGMSystemConfiguration* confBuffer);
  void AddCut(int tpartid, double tminenergy, double tmaxenergy);
  bool IsSelected(const NGMHit* tHit) const;
  int GetType(const NGMHit* tHit) const;
  TString GetName(const NGMHit* tHit) const;
  Style_t GetStyle(const NGMHit* tHit) const;
  Color_t GetColor(const NGMHit* tHit) const;
  Style_t GetStyle(int type) const;
  Color_t GetColor(int type) const;
  TLegend* GetLegend(Double_t x1, Double_t y1, Double_t x2, Double_t y2, const char* header = "", Option_t* option = "brNDC");
  TString* GetLabel();
  enum partid {gbgamma = 1, gbmuon = 2, mbgamma = 3, mbmuon = 4,
               lsgamma = 5, lsneutron = 6, lsmuon = 7, 
               hettlid = 8, heid = 9, bfgamma = 10, exttrig = 11};
  enum partstat { maxparticletypes = 11 };

  int GetMaxParticleTypes() const { return maxparticletypes; }
  const char* GetParticleName(int type) const;

  int getPlotIndex(const NGMHit* tHit) const;
  int getGateOffset(int ichan, int igate) const;
  int getGateWidth(int ichan, int igate) const;
  int getNumberOfChannels() const { return numberOfChannels;}
  TString getChannelName(int ichan) const ;
  int GetDetectorType(int ichan) const;
  int getPlotIndex(const char* cDetName) const;
  void setNeutronIdParams(float NeutronIdLow, float NeutronIdHigh, float gammaRejHigh);
  void setNeutronPileUpRejection(bool setPileUp = true);
  void setTailTest(double min, double max){_tailRatioMin = min; _tailRatioMax = max; }
  //DON'T REALLY NEAD maxskips HERE. AMG
  enum localconsts {maxChannels = 1000001, maxskips = 20};
  enum detectortype {gammablock = 1, muonblock = 2, liquidscint = 3, hettl = 4, he = 5, bf = 6, other = 7, lsdualpmt = 8, gammanai = 9};
  private:
  //IS THIS OK SINCE WE PLAN TO WRITE OUT THE CLASS? AMG
  std::vector<partSelect_rst> particleSelection;
  TArrayS _detType;
  TArrayD _muonThreshold;
  
  //NOT SURE I REALLY NEED THE NAMES HERE, BUT I WILL LEAVE THEM IN. AMG
  TObjArray _detNames;
  TArrayS _gatewidth[8];
  TArrayS _gateoffset[8];

  int numberOfSlots;
  int numberOfChannels;
  int numberOfHWChannels;
  int channelsperSlot;
  int baseSlot;
  bool isSimu;
  float _neutronidHigh;
  float _neutronidLow;
  float _gammaRejHigh;
  int _pileUpMax;
  double _tailRatioMin;
  double _tailRatioMax;
	
  ClassDef(NGMSimpleParticleIdent,7);
};

#endif // #ifndef __NGMSIMPLEPARTICLEIDENT__


