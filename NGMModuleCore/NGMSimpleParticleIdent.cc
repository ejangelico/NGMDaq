#include "NGMParticleIdent.h"
#include "NGMSimpleParticleIdent.h"
#include "NGMSystemConfiguration.h"
#include "NGMHit.h"
#include "NGMLogger.h"
#include "TColor.h"
#include "TLegend.h"
#include "TMarker.h"
#include <math.h>
#include "TObjString.h"

ClassImp(NGMSimpleParticleIdent);

NGMSimpleParticleIdent::NGMSimpleParticleIdent(){
  numberOfSlots=0;
  numberOfChannels=0;
  numberOfHWChannels=0;
  channelsperSlot = 8;
  baseSlot = 0;
  isSimu = false;
  
  _neutronidHigh = 3.0;
  _neutronidLow = -1.5;
  _gammaRejHigh = 4.0;
  _pileUpMax = 1.0;
	_tailRatioMin = 0.0;
	_tailRatioMax = 0.0;

  //SHOULD I DO THIS? AMG
  //AddCut(-1,-1,-1);
}

void NGMSimpleParticleIdent::setNeutronIdParams(float NeutronIdLow, float NeutronIdHigh, float gammaRejHigh)
{
  _neutronidHigh = NeutronIdHigh;
  _neutronidLow = NeutronIdLow;
  _gammaRejHigh = gammaRejHigh;  
}

void NGMSimpleParticleIdent::setNeutronPileUpRejection(bool setPileUp)
{
  if(setPileUp)
    _pileUpMax = 1.0;
  else
    _pileUpMax = 9999.0;
}


bool 
NGMSimpleParticleIdent::Init(const NGMSystemConfiguration* confBuffer){

    numberOfSlots = confBuffer->GetSlotParameters()->GetEntries();
    //LOG<<"Number of Slots: " << numberOfSlots<<ENDM_INFO;
    numberOfChannels = confBuffer->GetChannelParameters()->GetEntries();
  LOG<<"Found "<<confBuffer->GetName()<<" with " <<numberOfChannels << " entries."<<ENDM_INFO;
    TString confName(confBuffer->GetName());
    confName.ToUpper();
    if(confName=="NGMSIMULATIONCONFIGURATION")
    {
      isSimu = true;
      channelsperSlot = NGMSimpleParticleIdent::maxChannels;
      baseSlot = 0;
    }else if (confName=="SIS3320" || confName=="SIS3302" || confName=="SIS3150")
    {
      isSimu = false;
      channelsperSlot = 8;
      baseSlot = 0;
    }else if (confName=="SIS3316")
    {
        isSimu = false;
        channelsperSlot = 16;
        baseSlot = 0;
    }else if (confName=="PIXIE16ORNLV2")
    {
      isSimu = false;
      channelsperSlot = 16;
      baseSlot = 2;
    }else if (confName=="PIXIE16")
    {
      isSimu = false;
      channelsperSlot = 16;
      baseSlot = 0;
    }else{
      LOG<<"Unrecognized Configuration Object ("
         << confBuffer->GetName() <<")"<<ENDM_FATAL;
      baseSlot = 0;
    }
  
    bool markUnusedChannels = false;

    if(confBuffer->GetChannelParameters()->GetParIndex("ChanEnableConf")>=0)
    {
      markUnusedChannels = true;
    }
    int channelcount = confBuffer->GetChannelParameters()->GetEntries();
  
    // Set size of our arrays to accomodate this channel count
    _detType.Set(channelcount);
    _muonThreshold.Set(channelcount);
    _detNames.Clear();
    _detNames.SetOwner();
    _detNames.Expand(channelcount+1);
	for(int igate = 0; igate < 8; igate++){
		// Gate Width
		_gatewidth[igate].Set(numberOfChannels);
		_gateoffset[igate].Set(numberOfChannels);
	}
	
  // Set default muon threshold of 10MeV
  for(int ichan = 0; ichan < channelcount; ichan++)
    _muonThreshold[ichan] = 10000.0;
  
    // Find Detector Names from Table
    if(confBuffer->GetChannelParameters()->GetParIndex("DetectorName")>=0)
    {
      for(int ichan = 0; ichan < numberOfChannels; ichan++)
      {
        if(ichan>= maxChannels) break;
        TString detName = confBuffer->GetChannelParameters()->GetParValueS("DetectorName",ichan);
        if(detName=="") detName.Form("DET_%02d",ichan);
        LOG<<"Adding channel "<<detName.Data()<<ENDM_INFO;
        _detNames[ichan] = new TObjString(detName);

        if(markUnusedChannels)
        {
          if(confBuffer->GetChannelParameters()->GetParValueI("ChanEnableConf",ichan) == 0)
          {
            (dynamic_cast<TObjString*>(_detNames[ichan]))->String().Form("EMPTY_%03d",ichan); 
          }
        }
        if(detName.BeginsWith("GL") || detName.BeginsWith("GS")){
          _detType[ichan] = gammablock;
        }else if(detName.BeginsWith("MU") ){
          _detType[ichan] = muonblock;
        }else if(detName.BeginsWith("LS") || detName.BeginsWith("SB") || detName.BeginsWith("LIQ") || detName.BeginsWith("PSD")){
          _detType[ichan] = liquidscint;
        }else if(detName.BeginsWith("HETTL")){
          _detType[ichan] = hettl;          
        }else if(detName.BeginsWith("BF")){
          _detType[ichan] = bf;  
        }else if(detName.BeginsWith("DDLIPlastic")){
            _detType[ichan] = liquidscint;
        }else if(detName.BeginsWith("DDLILiquid")){
            _detType[ichan] = lsdualpmt;
        }else if(detName.BeginsWith("DDLINaI")){
            _detType[ichan] = gammanai;
        }else{
          _detType[ichan] = other;
        }

        char cbuff[1024];
        for(int igate = 0; igate < 8; igate++){
          // Gate Width
          sprintf(cbuff,"Gate%d_Length",igate+1);
          // SIS values are -1 of actual width
          if(confBuffer->GetSlotParameters()->GetParValueI(cbuff,0) > 0)
            _gatewidth[igate][ichan] = confBuffer->GetSlotParameters()->GetParValueI(cbuff,ichan/channelsperSlot);
          // Gate Start Index
          sprintf(cbuff,"Gate%d_StartIndex",igate+1);
          if(confBuffer->GetSlotParameters()->GetParValueI(cbuff,0) > 0)
            _gateoffset[igate][ichan] = confBuffer->GetSlotParameters()->GetParValueI(cbuff,ichan/channelsperSlot);
        }    
        
        //Loop up detector specific values
        int detRow = confBuffer->GetDetectorParameters()->FindFirstRowMatching("DetectorName",detName.Data());
        if(detRow>=0)
        {
          _muonThreshold[ichan] = confBuffer->GetDetectorParameters()->GetParValueD("MuonThreshold",detRow);
        }
      }
    }
    numberOfHWChannels = numberOfChannels; //I'M NOT SURE ABOUT THIS. AMG
  return true;
}

int 
NGMSimpleParticleIdent::getPlotIndex(const NGMHit* tHit) const
{  
  // Now channels per slot filled by inspecting configuration name
  // SIS3320: channelsperSlot = 0
  // NGMSimulationConfiguration: channelsperSlot = maxChannels
  
  // Check for some old single channel data
  if(tHit->GetSlot()<0 || tHit->GetChannel()<0) return 0;
  
  int plotIndex = (tHit->GetSlot()-baseSlot)*channelsperSlot + tHit->GetChannel();
  
  // Old Data Format Kludge
  if(tHit->GetSlot()>=0x4000)
    plotIndex = (tHit->GetSlot() - 0x4000)/0x800 * channelsperSlot + tHit->GetChannel();

  // Check for a bad channel id
  static bool badhitdetected = false;
  if(plotIndex < 0 || plotIndex >= maxChannels || plotIndex >= numberOfChannels)
  {
    if(!badhitdetected)
    {
      LOG<<"There is a rogue channel index "<<plotIndex<<" larger than max "<< numberOfChannels << " potentially very serious error.!"<<ENDM_WARN;
      badhitdetected = true;
    }
  }
  return plotIndex;
}

int 
NGMSimpleParticleIdent::getPlotIndex(const char* cDetName) const
{  
  for(int ichan = 0; ichan < maxChannels; ichan++)
  {
    if( ((dynamic_cast<TObjString*>(_detNames[ichan]))->String()) == cDetName ) return ichan;
  }
  LOG<<"Requested detector name not found"<<ENDM_WARN;
  return -1;
}

void 
NGMSimpleParticleIdent:: AddCut(int tpartid, double tminenergy, double tmaxenergy){
  // Add selection for all particles to be analyzed
  partSelect_rst tmpId;
  tmpId.partid = tpartid;
  tmpId.energymin = tminenergy;
  tmpId.energymax = tmaxenergy;
  particleSelection.push_back(tmpId);
}

int
NGMSimpleParticleIdent::GetDetectorType(int ichan) const
{
  if(ichan<0 || ichan>=maxChannels) return -1;
  return _detType[ichan];
}

int
NGMSimpleParticleIdent::GetType(const NGMHit* tHit) const {
  int plotIndex = getPlotIndex(tHit);
  
  // PSD parameters
  // To be added as user setable datamembers
  
  if(_detType[plotIndex] == gammablock){
    // Should test for muon energy
    if(tHit->GetEnergy() > _muonThreshold[plotIndex])
		return gbmuon;
    return gbgamma;
  }else if(_detType[plotIndex] == muonblock){
    // Test for gammas or muons
    if(tHit->GetEnergy() > _muonThreshold[plotIndex])
		return mbmuon;
	return mbgamma;
  }else if(_detType[plotIndex] == liquidscint){

    if(tHit->GetEnergy() > _muonThreshold[plotIndex])
		return lsmuon;    

    // Neutron ID is based on separation from Neutron Band
    // and no pileup detected
    // We should also consider adding the tail g3vsg4 consistency check
    
    //Test for Old Neutron Gates and Simulation
    if((tHit->GetGammaId()==0.0 && tHit->GetNeutronId()==-9999.0)
       || (tHit->GetGammaId()==-9999.0 && tHit->GetNeutronId()==0.0)
       || isSimu)
    {
      if(tHit->GetNeutronId() == 0.0)
        return lsneutron;
      else
        return lsgamma;
    }
		
    
		// If not the old binary yes/no then we will asumme its the new sigmalized formulation
    if(_neutronidLow <= tHit->GetNeutronId() && tHit->GetNeutronId()<_neutronidHigh
       && (_gammaRejHigh<0.0 || tHit->GetGammaId()>_gammaRejHigh)
       && (_gammaRejHigh>0.0 || tHit->GetGammaId()<_gammaRejHigh)
       && tHit->GetPileUpCounter() <= _pileUpMax)
		{
            return lsneutron;
		}
      if(-2.0 <= tHit->GetGammaId() && tHit->GetGammaId()<2.0)
      {
          return lsgamma;
      }
    // Default to lsmuon
    return lsmuon;
  }else if(_detType[plotIndex] == hettl){
    return hettlid;
  }else if(_detType[plotIndex] == he){
    return heid;
  }else if(_detType[plotIndex] == bf){
    return bfgamma;
  }else if(_detType[plotIndex] == other){
    return exttrig;
  }
  return 0;
}

TString 
NGMSimpleParticleIdent::GetName(const NGMHit* tHit) const {
  int plotIndex = getPlotIndex(tHit);
  
  if(plotIndex>=_detNames.GetEntries()) return TString("");
     
  return (dynamic_cast<TObjString*>(_detNames[plotIndex]))->String();
}

bool 
NGMSimpleParticleIdent::IsSelected(const NGMHit* tHit) const {
  // Check our local list of particle cuts to see if 
  // this cut is valid
  // If no cuts or first cut is -1 assume inclusive
  if(particleSelection.size() == 0) return true;
  if(particleSelection.size()==0 || particleSelection[0].partid == -1) return true;
  // Loop over cuts and find matching entry if found check energy cut
/*
  TString name = GetName(tHit);
  if(strcmp(name.Data(), "LS1900")>=0 && strcmp(name.Data(), "LS2188")<=0) return false;
  if(strcmp(name.Data(), "LS2501")>=0 && strcmp(name.Data(), "LS2789")<=0) return false;
*/
  int partid = GetType(tHit);
  for(int isel = 0; isel < (int)particleSelection.size(); isel++)
  {
    if(particleSelection[isel].partid == partid)
    {
      if(particleSelection[isel].energymin < 0.0)
      {
        return true;
      }else{
        if(particleSelection[isel].energymin < tHit->GetEnergy()
           &&  tHit->GetEnergy() < particleSelection[isel].energymax )
          return true;
        else
          return false;
      }
    }
  }
  // If partid not found, reject the particle
  return false;
}


Style_t
NGMSimpleParticleIdent::GetStyle(const NGMHit* tHit) const {
  int type = GetType(tHit);
  return GetStyle(type);
}

Color_t 
NGMSimpleParticleIdent::GetColor(const NGMHit* tHit) const {
  int type = GetType(tHit);
  return GetColor(type);
}

Style_t
NGMSimpleParticleIdent::GetStyle(int type) const {
  if(type==gbgamma){
    //return kCircle;
	return 7;
  }else if(type==mbgamma){
    return kOpenSquare;
  }else if(type==lsneutron){
    return kFullSquare;
  }else if(type==lsgamma){
    return kFullCircle;
  }else if(type==hettlid){
    return kFullStar;
  }else if(type==heid){
    return kFullTriangleUp;
  }else{
    return 3;
  }
}

const char* NGMSimpleParticleIdent::GetParticleName(int type) const
{
  if(type==gbgamma){
    return "gbgamma";
  }else if(type==gbmuon){
    return "gbmuon";
  }else if(type==mbgamma){
    return "mbgamma";
  }else if(type==mbmuon){
    return "mbmuon";
  }else if(type==lsneutron){
    return"lsneutron";
  }else if(type==lsgamma){
    return "lsgamma";
  }else if(type==lsmuon){
    return "lsmuon";
  }else if(type==hettlid){
    return "hettlid";
  }else if(type==heid){
    return "heid";
  }else if(type==bfgamma){
    return "bfgamma";
  }else if(type==exttrig){
    return "exttrig";
  }else{
    return "unknown";
  }

}

Color_t 
NGMSimpleParticleIdent::GetColor(int type) const {
  if(type==gbgamma){
    return kRed;
  }else if(type==gbmuon){
    return kCyan;
  }else if(type==mbgamma){
    return 28;
  }else if(type==mbmuon){
    return kBlue;
  }else if(type==lsneutron){
    return TColor::GetColor(212,170,43);
  }else if(type==lsgamma){
    return kMagenta;
  }else if(type==lsmuon){
    return kGreen;
  }else if(type==hettlid){
    return kBlack;
  }else if(type==heid){
    return kYellow;
  }else{
    return 3;
  }
}

TLegend*
NGMSimpleParticleIdent::GetLegend(Double_t x1, Double_t y1, Double_t x2, Double_t y2, const char* header, Option_t* option){
  TLegend* legend = new TLegend(x1,y1,x2,y2,header,option);
  const char* partNames[9] ={"gbgamma","gbmuon","mbgamma","mbmuon","lsgamma","lsneutron","lsmuon","hettlid","heid"};
  char cutVals[9][128];
  for (int i=0; i<9;i++){
    sprintf(cutVals[i],"Removed");
  }

  //Must mimic IsSelected
  if(particleSelection.size()==0 || particleSelection[0].partid == -1){
    for (int i=0; i<9;i++){
      sprintf(cutVals[i],"No Cut");
    }
  } else {
    for(int isel = 0; isel < (int)particleSelection.size(); isel++){
      if(particleSelection[isel].partid < 1 || particleSelection[isel].partid>9) continue;
      if(strcmp("Removed",cutVals[particleSelection[isel].partid-1])==0){//Since only the first cut is used by IsSelected
	if(particleSelection[isel].energymin<0)  sprintf(cutVals[particleSelection[isel].partid-1],"No Cut");
	else sprintf(cutVals[particleSelection[isel].partid-1],"%5.5g-%5.5g keV",particleSelection[isel].energymin,particleSelection[isel].energymax);
      }
    }
  }
  
  for (int i=1; i<9;i++){
    TMarker* mark = new TMarker;
    mark->SetMarkerColor(GetColor(i));
    mark->SetMarkerStyle(GetStyle(i));
    legend->AddEntry(mark,partNames[i-1],"p");
    legend->AddEntry(mark,cutVals[i-1],"");
  }
  return legend;
}

TString*
NGMSimpleParticleIdent::GetLabel(){
  TString* label = new TString("");
  char buffer[1024];
  char labelc[1024];
  const char* partNames[9] ={"gbgamma","gbmuon","mbgamma","mbmuon","lsgamma","lsneutron","lsmuon","hettlid","heid"};
  char cutVals[9][128];
  for (int i=0; i<9;i++){
    sprintf(cutVals[i],"Removed");
  }

  //Must mimic IsSelected
  if(particleSelection.size()==0 || particleSelection[0].partid == -1){
    *label += "No cut";
  } else {
    bool first = true;
    for(int isel = 0; isel < (int)particleSelection.size(); isel++){
      if(particleSelection[isel].partid < 1 || particleSelection[isel].partid>9) continue;
      if(strcmp("Removed",cutVals[particleSelection[isel].partid-1])==0){//Since only the first cut is used by IsSelected
        if(!first) *label += "|";
        first = false;
        *label += partNames[particleSelection[isel].partid-1];
	if(particleSelection[isel].energymin<0) {
          *label += "(no cut)";
	} else {
          sprintf(buffer, "(%g-%g keV)",particleSelection[isel].energymin,particleSelection[isel].energymax);
          *label += buffer;
        }
      }
    }
  }
  return label;
}

int NGMSimpleParticleIdent::getGateOffset(int ichan, int igate) const
{
  return _gateoffset[igate][ichan];
}

int NGMSimpleParticleIdent::getGateWidth(int ichan, int igate) const
{
  return _gatewidth[igate][ichan];
}

TString NGMSimpleParticleIdent::getChannelName(int ichan) const 
{
  if(!_detNames[ichan]) return TString("");
  return (dynamic_cast<TObjString*>(_detNames[ichan]))->String();
}
