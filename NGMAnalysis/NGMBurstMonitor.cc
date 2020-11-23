#include "NGMBurstMonitor.h"
#include "NGMHit.h"
#include "NGMParticleIdent.h"
#include "NGMSimpleParticleIdent.h"
#include "TClass.h"
#include "NGMSystemConfiguration.h"
#include "TCanvas.h"
#include "NGMCanvas.h"
#include "TGraph.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TString.h"
#include "NGMMarker.h"
#include "TColor.h"
#include "TMath.h"
#include "TF1.h"
#include "NGMBufferedPacket.h"
#include "NGMLogger.h"
#include "TFolder.h"
#include "NGMMultipleScattering.h"
#include "NGMSystem.h"
#include "TSQLResult.h"
#include "TSQLServer.h"

#include <cmath>
#include <iostream>

NGMBurstMonitor::NGMBurstMonitor(){
  partID = 0;
  gBurst_gbgamma=0;
  gBurst_gbmuon=0;
  gBurst_mbgamma=0;
  gBurst_mbmuon=0;
  gBurst_lsneutron=0;
  gBurst_lsgamma=0;
  gBurst_lsmuon=0;
  gBurst_hettlid=0;
  gBurst_heid=0;
  _burstLenHist=0;
  _burstSizeHist=0;
  _hitsInBurst = 6;
  _NBurstZoomes = 0;
  _burstWindowNS = 10000;
  _graphWindowNS = 2000000;
  _runNumber = 0;
  cBurst = 0;
  _burstList = 0;
  _drawGraphics = true;
  _timeInitDone = false;
  _pushFullWindow = true;
  _parLabel = 0;
  _burstParLabel = 0;
  _legend = 0;
  _myZoomer = 0;
  _candidatePosition = 0;
  _burstTimingSummary = 0; //(TH1**) ( new size_t[partID->GetMaxParticleTypes()]);
  //for(int ipart = 0; ipart < partID->GetMaxParticleTypes(); ipart++)
	//_burstTimingSummary[ipart] = 0;
   doMultEnergyCut = false;
   lsnCutSlope = 0.0;
   lsnCutConstant = 0.0;
   lsgCutSlope = 0.0;
   lsnCutConstant = 0.0;
   gbCutSlope = 0.0;
   gbCutConstant = 0.0;
   _gbEnergyvsMult = 0;
   _lsnEnergyvsMult = 0;
   _lsgEnergyvsMult = 0;
  
}

NGMBurstMonitor::NGMBurstMonitor(const char* name, const char* title)
: NGMModule(name,title)
{
  partID = new NGMSimpleParticleIdent();
  gBurst_gbgamma=0;
  gBurst_gbmuon=0;
  gBurst_mbgamma=0;
  gBurst_mbmuon=0;
  gBurst_lsneutron=0;
  gBurst_lsgamma=0;
  gBurst_lsmuon=0;
  gBurst_hettlid=0;
  gBurst_heid=0;
  _burstLenHist=0;
  _burstSizeHist=0;
  _hitsInBurst = 6;
  _NBurstZoomes = 0;
  _burstWindowNS = 10000;
  _graphWindowNS = 2000000;
  _runNumber = 0;
  cBurst = 0;
  _burstList = 0;
  _drawGraphics = true;
  _timeInitDone = false;
  _pushFullWindow = true;
  _parLabel = 0;
  _burstParLabel = 0;
  _legend = 0;
  _myZoomer = 0;
  _candidatePosition = 0;
  _burstTimingSummary = (TH1**) ( new size_t[partID->GetMaxParticleTypes()]);
  for(int ipart = 0; ipart < partID->GetMaxParticleTypes(); ipart++)
	_burstTimingSummary[ipart] = 0;
  doMultEnergyCut = false;
  lsnCutSlope = 0.0;
  lsnCutConstant = 0.0;
  lsgCutSlope = 0.0;
  lsnCutConstant = 0.0;
  gbCutSlope = 0.0;
  gbCutConstant = 0.0;
  _gbEnergyvsMult = 0;
  _lsnEnergyvsMult = 0;
  _lsgEnergyvsMult = 0;
  
}

NGMBurstMonitor::~NGMBurstMonitor(){
//  for(int i = 0; i<partID->GetMaxParticleTypes(); i++)
//  {
//    delete _burstTimingSummary[i];
//  }
  delete [] _burstTimingSummary;
  delete partID;
//  delete gBurst_gbgamma;
//  delete gBurst_gbmuon;
//  delete gBurst_mbgamma;
//  delete gBurst_mbmuon;
//  delete gBurst_lsneutron;
//  delete gBurst_lsgamma;
//  delete gBurst_lsmuon;
//  delete gBurst_hettlid;
//  delete gBurst_heid;
//  delete _burstLenHist;
//  delete _burstSizeHist;
//  delete _burstList;
//  delete _parLabel;
//  delete _burstParLabel;
//  delete _legend;
  
}

void
NGMBurstMonitor:: AddRequirement(int tpartid, int mincounts, int maxcounts){
  // Add selection for all particles to be analyzed
  burstSelect_st tmpId;
  tmpId.partid = tpartid;
  tmpId.mincounts = mincounts;
  tmpId.maxcounts = maxcounts;
  _partSelection.push_back(tmpId);
}

void NGMBurstMonitor::AddChannelRequirement(ULong64_t mask1, ULong64_t mask2, ULong64_t mask3)
{
	_mask1.push_back(mask1);
	_mask2.push_back(mask2);
	_mask3.push_back(mask3);

	return;
}

bool
NGMBurstMonitor::init(){
  if(_NBurstZoomes>0) cBurst = new NGMCanvas("cBurst","Zoomed Burst");

  char hname[1024];
  //Should be done more generically with a loop over particle types
  //Could not stick exactly to old color scheme sine this is by particle type nore detector type.

  //AMG Drawing a histogram with that has more than 30K bins and a fill color seems to crash some X servers!
  //Changed to graphs to make more general
  if(!gBurst_gbgamma){
    sprintf(hname,"%s_%s",GetName(),"BurstTime_gbgamma");
    gBurst_gbgamma = new TGraph(20000);
    gBurst_gbgamma->SetName(hname);
    gBurst_gbgamma->SetTitle(";Time (s);Counts");
    gBurst_gbgamma->SetLineColor(partID->GetColor(1));
    gBurst_gbgamma->SetFillColor(partID->GetColor(1));
    gBurst_gbgamma->SetMarkerColor(partID->GetColor(1));
    gBurst_gbgamma->SetMarkerStyle(partID->GetStyle(1));
    GetParentFolder()->Add(gBurst_gbgamma);
  }
  if(!gBurst_gbmuon){
    sprintf(hname,"%s_%s",GetName(),"BurstTime_gbmuon");
    gBurst_gbmuon = new TGraph(20000);
    gBurst_gbmuon->SetName(hname);
    gBurst_gbmuon->SetTitle(";Time (s);Counts");
    gBurst_gbmuon->SetLineColor(partID->GetColor(NGMSimpleParticleIdent::gbmuon));
    gBurst_gbmuon->SetFillColor(partID->GetColor(NGMSimpleParticleIdent::gbmuon));
    gBurst_gbmuon->SetMarkerColor(partID->GetColor(NGMSimpleParticleIdent::gbmuon));
    gBurst_gbmuon->SetMarkerStyle(partID->GetStyle(NGMSimpleParticleIdent::gbmuon));
    GetParentFolder()->Add(gBurst_gbmuon);
  }
  if(!gBurst_mbgamma){
    sprintf(hname,"%s_%s",GetName(),"BurstTime_mbgamma");
    gBurst_mbgamma = new TGraph(20000);
    gBurst_mbgamma->SetName(hname);
    gBurst_mbgamma->SetTitle(";Time (s);Counts");
    gBurst_mbgamma->SetLineColor(partID->GetColor(3));
    gBurst_mbgamma->SetFillColor(partID->GetColor(3));
    gBurst_mbgamma->SetMarkerColor(partID->GetColor(3));
    gBurst_mbgamma->SetMarkerStyle(partID->GetStyle(3));
    GetParentFolder()->Add(gBurst_mbgamma);
  } 
  if(!gBurst_mbmuon){
    sprintf(hname,"%s_%s",GetName(),"BurstTime_mbmuon");
    gBurst_mbmuon = new TGraph(20000);
    gBurst_mbmuon->SetName(hname);
    gBurst_mbmuon->SetTitle(";Time (s);Counts");
    gBurst_mbmuon->SetLineColor(partID->GetColor(NGMSimpleParticleIdent::mbmuon));
    gBurst_mbmuon->SetFillColor(partID->GetColor(NGMSimpleParticleIdent::mbmuon));
    gBurst_mbmuon->SetMarkerColor(partID->GetColor(NGMSimpleParticleIdent::mbmuon));
    gBurst_mbmuon->SetMarkerStyle(partID->GetStyle(NGMSimpleParticleIdent::mbmuon));
    GetParentFolder()->Add(gBurst_mbmuon);
  }
  if(!gBurst_lsneutron){
    sprintf(hname,"%s_%s",GetName(),"BurstTime_lsneutron");
    gBurst_lsneutron = new TGraph(20000);
    gBurst_lsneutron->SetName(hname);
    gBurst_lsneutron->SetLineColor(partID->GetColor(6));
    gBurst_lsneutron->SetFillColor(partID->GetColor(6));
    gBurst_lsneutron->SetMarkerColor(partID->GetColor(6));
    gBurst_lsneutron->SetMarkerStyle(partID->GetStyle(6));
    GetParentFolder()->Add(gBurst_lsneutron);
  }
  if(!gBurst_lsmuon){
    sprintf(hname,"%s_%s",GetName(),"BurstTime_lsmuon");
    gBurst_lsmuon = new TGraph(20000);
    gBurst_lsmuon->SetName(hname);
    gBurst_lsmuon->SetLineColor(partID->GetColor(7));
    gBurst_lsmuon->SetFillColor(partID->GetColor(7));
    gBurst_lsmuon->SetMarkerColor(partID->GetColor(7));
    gBurst_lsmuon->SetMarkerStyle(partID->GetStyle(7));
    GetParentFolder()->Add(gBurst_lsmuon);
  }
  if(!gBurst_lsgamma){
    sprintf(hname,"%s_%s",GetName(),"BurstTime_lsgamma");
    gBurst_lsgamma = new TGraph(20000);
    gBurst_lsgamma->SetName(hname);
    gBurst_lsgamma->SetLineColor(partID->GetColor(5));
    gBurst_lsgamma->SetFillColor(partID->GetColor(5));
    gBurst_lsgamma->SetMarkerColor(partID->GetColor(5));
    gBurst_lsgamma->SetMarkerStyle(partID->GetStyle(5));
    GetParentFolder()->Add(gBurst_lsgamma);
  }
  if(!gBurst_hettlid){
    sprintf(hname,"%s_%s",GetName(),"BurstTime_hettlid");
    gBurst_hettlid = new TGraph(20000);
    gBurst_hettlid->SetName(hname);
    gBurst_hettlid->SetLineColor(partID->GetColor(8));
    gBurst_hettlid->SetFillColor(partID->GetColor(8));
    gBurst_hettlid->SetMarkerColor(partID->GetColor(8));
    gBurst_hettlid->SetMarkerStyle(partID->GetStyle(8));
    GetParentFolder()->Add(gBurst_hettlid);
  }
  if(!gBurst_heid){
    sprintf(hname,"%s_%s",GetName(),"BurstTime_heid");
    gBurst_heid = new TGraph(20000);
    gBurst_heid->SetName(hname);
    gBurst_heid->SetLineColor(partID->GetColor(9));
    gBurst_heid->SetFillColor(partID->GetColor(9));
    gBurst_heid->SetMarkerColor(partID->GetColor(9));
    gBurst_heid->SetMarkerStyle(partID->GetStyle(9));
    GetParentFolder()->Add(gBurst_heid);
  }
  if(!_burstLenHist){
    sprintf(hname,"%s_%s",GetName(),"BurstLenDist");
    _burstLenHist = new TH1F(hname,";Burst Length (ns);Occurences",100,0,_burstWindowNS);
    _burstLenHist->SetDirectory(0);
    GetParentFolder()->Add(_burstLenHist);
  }

  if(!_burstSizeHist){
    sprintf(hname,"%s_%s",GetName(),"BurstSizeDist");
    _burstSizeHist = new TH1F(hname,";Burst Size (counts);Occurences",101,-0.5,100.5);
    _burstSizeHist->SetDirectory(0);
    GetParentFolder()->Add(_burstSizeHist);
  }
  if(!_gbEnergyvsMult){
    sprintf(hname,"%s_%s",GetName(),"GBEnergyvsMult");
    _gbEnergyvsMult = new TH2F(hname,hname,101,-0.5,100.5,100,0,100.0);
    _gbEnergyvsMult->SetDirectory(0);
    GetParentFolder()->Add(_gbEnergyvsMult);
  }
  if(!_lsnEnergyvsMult){
    sprintf(hname,"%s_%s",GetName(),"LSNEnergyvsMult");
    _lsnEnergyvsMult = new TH2F(hname,hname,101,-0.5,100.5,100,0,100.0);
    _lsnEnergyvsMult->SetDirectory(0);
    GetParentFolder()->Add(_lsnEnergyvsMult);
  }
  if(!_lsgEnergyvsMult){
    sprintf(hname,"%s_%s",GetName(),"LSGEnergyvsMult");
    _lsgEnergyvsMult = new TH2F(hname,hname,101,-0.5,100.5,100,0,100.0);
    _lsgEnergyvsMult->SetDirectory(0);
    GetParentFolder()->Add(_lsgEnergyvsMult);
  }
  return true;
}


bool
NGMBurstMonitor::process(const TObject &tData){
  static TClass* tNGMHitType = TClass::GetClass("NGMHit");
  static TClass* tNGMSystemConfigurationType = TClass::GetClass("NGMSystemConfiguration");
  static TClass* tObjStringType = TClass::GetClass("TObjString");
  //  static long long neutronCount =0;//TEMP

  //Check data type
  if(tData.InheritsFrom(tNGMHitType)){
    const NGMHit* tHit = (const NGMHit*)(&tData);
    //    if(partID->GetType(tHit)== 6) neutronCount++;//TEMP
	 if(getDebug()) tHit->Print();
    _gapfinder.nextTimeIsGap(tHit->GetNGMTime());
    if(partID != 0 && partID->IsSelected(tHit)) 
    {
      // Try block included because under some circumstances 
      // this routine throws an exception
      // such as when hitvec[_candidatePosition] is out of bounds
      try{
        analyzeHit(tHit);
      }
      catch(...)
      {
        LOG<<"Exception detected"<<ENDM_FATAL;
      }
    }
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
    partID->Init(confBuffer);
	// Should Check if GetMaxParticleTypes has changed and reallocate arrays
	// or use TObjArray instead of c-style arrays
	
    _runNumber = confBuffer->getRunNumber();
    //_hitsAnalyzed = 0;
    ResetHistograms();
    _timeInitDone = false;

    // Clear burst list 
	if(_burstList)
	{
		_burstList->Clear();
		delete _burstList;
		_burstList = 0;
	}
	push(tData);
  }else {
    if(tData.IsA() == tObjStringType){
      const TObjString* controlMessage = (const TObjString*)(&tData);
      if(controlMessage->GetString() == "EndRunSave") 
      {
	//		LOG<<"TOTAL LS NEUTRON COUNT WAS "<< neutronCount<<ENDM_INFO;//TEMP
	     LOG<<"Total Bursts for "<<GetName()<<" "<<_Num_burst<<ENDM_INFO;
         if(_drawGraphics) doDraw();
      }else if(controlMessage->GetString() == "EndSpillFlush") {
         LOG<<"Total Bursts for "<<GetName()<<" "<<_Num_burst<<ENDM_INFO;
         if(_drawGraphics) doDraw();
      } else if(controlMessage->GetString() == "EndRunFlush") 
      {
         LaunchDisplayCanvas();
      }else if(controlMessage->GetString() == "PlotUpdate")
      {
        LaunchDisplayCanvas();
      }else if(controlMessage->GetString() == "PlotReset")
      {
        ResetHistograms();
      }else if(controlMessage->GetString() == "GapDetected") {
        //Need to flush... old stuff
        _timeInitDone = false;
      }
      
	  push(tData);
    }
  }
  // Only push configuration object and control messages
  //push(tData);

  return true;
}

bool
NGMBurstMonitor::finish(){
  std::cout<<"NGMBurstMonitor::finish"<<std::endl;
  return true;
}

void
NGMBurstMonitor::ResetHistograms(){
  
  _gapfinder.Reset();
  
  int npoints = gBurst_gbgamma->GetN();
  for(int i=0;i<npoints;i++){
    gBurst_gbgamma->SetPoint(i,-1,0);
    gBurst_gbmuon->SetPoint(i,-1,0);
    gBurst_mbgamma->SetPoint(i,-1,0);
    gBurst_mbmuon->SetPoint(i,-1,0);
    gBurst_lsneutron->SetPoint(i,-1,0);
    gBurst_lsmuon->SetPoint(i,-1,0);
    gBurst_lsgamma->SetPoint(i,-1,0);
    gBurst_hettlid->SetPoint(i,-1,0);
    gBurst_heid->SetPoint(i,-1,0);
  }
  
  _Num_burst = 0;
  _timeInitDone = false;
  
//   for(int i=npoints-1;i>0;i--){
//     gBurst_gbgamma->RemovePoint(i);
//     gBurst_mbgamma->RemovePoint(i);
//     gBurst_lsneutron->RemovePoint(i);
//     gBurst_lsgamma->RemovePoint(i);
//     gBurst_hettlid->RemovePoint(i);
//     gBurst_heid->RemovePoint(i);
//   }

  if(_burstTimingSummary)
  for(int ipart = 0; ipart < partID->GetMaxParticleTypes(); ipart++)
  {
  	if(_burstTimingSummary[ipart])
	_burstTimingSummary[ipart]->Reset();
  }
  
  char hName[128];
  TString runStr;
  runStr+=_runNumber;
  sprintf(hName,"Run %s;Time (s);Counts", runStr.Data());
  gBurst_gbgamma->SetTitle(hName);
  gBurst_gbmuon->SetTitle(hName);
  gBurst_mbgamma->SetTitle(hName);
  gBurst_mbmuon->SetTitle(hName);
  gBurst_lsneutron->SetTitle(hName);
  gBurst_lsmuon->SetTitle(hName);
  gBurst_lsgamma->SetTitle(hName);
  gBurst_hettlid->SetTitle(hName);
  gBurst_heid->SetTitle(hName);

  _burstLenHist->Reset();
  sprintf(hName,"Run %s;Burst Lengh (ns);Occurences", runStr.Data());
  _burstLenHist->SetTitle(hName);

  _burstSizeHist->Reset();
  sprintf(hName,"Run %s;Burst Size (counts);Occurences", runStr.Data());
  _burstSizeHist->SetTitle(hName);
  _candidatePosition = 0;
  _gbEnergyvsMult->Reset();
  _lsnEnergyvsMult->Reset();
  _lsgEnergyvsMult->Reset();
  
} 

bool
NGMBurstMonitor::analyzeHit(const NGMHit* tHit){
  if(!_timeInitDone){
    _lastBurst = tHit->GetNGMTime();
    _firstTime = tHit->GetNGMTime();
    _lastDraw = tHit->GetNGMTime();//AMG HACK
    _candidatePosition = 0;
    
    while(hitvec.size())
    {
      delete hitvec.front();
      hitvec.pop_front();
    }
    
    _timeInitDone = true;
  }
   
//  if(Num_burst>20000){ //Is this really needed?
//    std::cout<<"Max bursts exceeded"<<std::endl;
//    return true;
//  }
  
  if (hitvec.size()>10000){
    std::cout<<"ERROR NGMBurstMonitor vector size limit hit"<<std::endl;
    return false; //Is this OK for high rate and/or long window
  }
  hitvec.push_back(new NGMMultipleScatteringHit(tHit->DuplicateHit(),true));
  //  if(tHit->TimeDiffNanoSec(hitvec.front()->GetNGMTime())<_graphWindowNS) return true;
  
  // we would like to keep at least one hit beyond the window
  int vecSize = hitvec.size();
  
  // First lets check to see that we have enough queued prior to the candidate position
  // if not we increment
    if(hitvec.at(_candidatePosition)->getHit()->TimeDiffNanoSec(hitvec.at(0)->getHit()->GetNGMTime()) < _graphWindowNS)
    {
      if(vecSize > 1){
        _candidatePosition++;
      }else{
        _candidatePosition = 0;
      }
      // Wait for next hit
      return true;
    }    
  
  if(hitvec.size() < 3)
  {
    return true;
  }
  
  
  
  // Test for a sequence of rejected hits associated with a burst
  // This algorithm assumes that the last hit in the hitvec buffer of a previous burst found
  // is never identified as part of the burst.
  bool prevRejected = false;
  // Now remove any hits at the beginning that are beyond the beginning of the graphWindowNS save one.
  while(hitvec.at(_candidatePosition)->getHit()->TimeDiffNanoSec(hitvec.at(1)->getHit()->GetNGMTime())>_graphWindowNS){
    
    // Instead of deleting this we instead will check
    // if this hit has been associated with a burst.
    // If we wish we can push out hits not associated with a burst.
    if(hitvec.front()->getKeep())
    {
      prevRejected = false;
      push(*((const TObject*)(hitvec.front()->getHit())));
    }else{
      //GapDetected
      if(!prevRejected)
      {
        TObjString gapDetected("GapDetected");
        push(*((const TObject*)&gapDetected));        
      }
      prevRejected = true;
    }
    delete hitvec.front();
    hitvec.pop_front();
    _candidatePosition--;
	vecSize = hitvec.size();
	if(hitvec.size() < 3) break;
  }
  
  // Now test if we have enough data after the candidate Position
  // if not well wait for the next hit
  if(hitvec.at(hitvec.size()-1)->getHit()->TimeDiffNanoSec(hitvec.at(_candidatePosition)->getHit()->GetNGMTime()) < _graphWindowNS)
  {
     return true;
  }  

  // Dont overlap with previous burst
  if (hitvec.at(_candidatePosition)->getHit()->TimeDiffNanoSec(_lastBurst) <= 0.0)
  {
     _candidatePosition++;
     return true;
  }
  
  // At this point we have determined that we have enough hits before and after the candidate position
  // appropriately consider it as a burst and have the required contextual hits determined by _graphWindowNS
  
  vecSize = hitvec.size(); // DON'T CHANGE THE VECTOR SIZE NOW!!!
  int i = _candidatePosition;
  {
	
    if ((i+_hitsInBurst < vecSize) &&( hitvec.at(i+_hitsInBurst-1)->getHit()->TimeDiffNanoSec(hitvec.at(i)->getHit()->GetNGMTime()) < _burstWindowNS))
      {	  
	int gbgammaCount=0;
	int gbmuonCount=0;
	int mbgammaCount=0;
	int mbmuonCount=0;
	int lsgammaCount=0;
	int lsneutronCount=0;
	int lsmuonCount=0;
	int hettlidCount=0;
	int heidCount=0;
  int exttrigCount=0;

  double gbESum=0.0;
  double mbESum = 0.0;
  double lsgammaESum = 0.0;
  double lsneutronESum = 0.0;
        
	ULong64_t tmask1 = 0;
	ULong64_t tmask2 = 0;
	ULong64_t tmask3 = 0;


	int ihit=i;
	for(ihit=i; ihit<vecSize;ihit++){
	  if (hitvec.at(ihit)->getHit()->TimeDiffNanoSec(hitvec.at(i)->getHit()->GetNGMTime()) > _burstWindowNS) break;
	  int plotIndex = partID->getPlotIndex(hitvec.at(ihit)->getHit());
	  // Test if channel mask is being applied
	  if(_mask1.size()>0){
		  if(plotIndex >=0 || plotIndex < 64){
			tmask1 = (0x1ULL<<plotIndex)|tmask1;
		  }else if (plotIndex<128) {
			tmask2 = (0x1ULL<<(plotIndex - 64))|tmask2;
		  }else if (plotIndex<192) {
			tmask3 = (0x1ULL<<(plotIndex - 128))|tmask3;
		  }
	  }
	    
	  int pID = partID->GetType(hitvec.at(ihit)->getHit());
	  if (pID == NGMSimpleParticleIdent::gbgamma) { //Need to get accsess to these through something other than hard coding
	    gbgammaCount++;
      gbESum+=hitvec.at(ihit)->getHit()->GetEnergy();
	  } else if(pID == NGMSimpleParticleIdent::gbmuon) {
	    gbmuonCount++;
      gbESum+=hitvec.at(ihit)->getHit()->GetEnergy();
	  } else if(pID == NGMSimpleParticleIdent::mbgamma) {
	    mbgammaCount++;
      mbESum+=hitvec.at(ihit)->getHit()->GetEnergy();
	  } else if(pID == NGMSimpleParticleIdent::mbmuon) {
	    mbmuonCount++;
      mbESum+=hitvec.at(ihit)->getHit()->GetEnergy();
	  } else if(pID == NGMSimpleParticleIdent::lsgamma) {
	    lsgammaCount++;
      lsgammaESum+=hitvec.at(ihit)->getHit()->GetEnergy();
	  } else if(pID == NGMSimpleParticleIdent::lsneutron) {
	    lsneutronCount++;
      lsneutronESum+=hitvec.at(ihit)->getHit()->GetEnergy();
	  } else if(pID == NGMSimpleParticleIdent::lsmuon) {
	    lsmuonCount++;
      lsgammaESum+=hitvec.at(ihit)->getHit()->GetEnergy();
	  } else if(pID == NGMSimpleParticleIdent::hettlid) {
	    hettlidCount++;
	  } else if(pID == NGMSimpleParticleIdent::heid) {
	    heidCount++;
	  } else if(pID == NGMSimpleParticleIdent::exttrig) {
	    exttrigCount++;
	  } else {
	    LOG<<"WARNING NGMBurstMonitor:analyzeHit particle type "<<pID<<" unsupported. Hit not used."<<ENDM_WARN;
	  }
	}	
	
        _gbEnergyvsMult->Fill(gbgammaCount+gbmuonCount,gbESum/1000.0);
        _lsnEnergyvsMult->Fill(lsneutronCount,lsneutronESum/1000.0);
        _lsgEnergyvsMult->Fill(lsgammaCount,lsgammaESum/1000.0);
        
	if(getDebug()) LOG<< (Long64_t)tmask1 << " : " << (Long64_t)tmask2 << " : "<<(Long64_t)tmask3<< ENDM_INFO;
	
	// Test if channel mask is being applied and apply rejection
	if(_mask1.size()>0){
		bool burstChanMaskAccepted = false;
		for(int imask = 0; imask < (int)_mask1.size(); imask++)
		{
			if((((_mask1[imask])&tmask1) == _mask1[imask])
				&& (((_mask2[imask])&tmask2) == _mask2[imask])
				&& (((_mask3[imask])&tmask3) == _mask3[imask]))
				burstChanMaskAccepted = true;
		}
		if(!burstChanMaskAccepted) return true;
	}
	bool rejectedFromSelection = false;
	for(int isel = 0; isel < (int)_partSelection.size(); isel++){
	  if(_partSelection[isel].partid == NGMSimpleParticleIdent::gbgamma) {
	    if(gbgammaCount<_partSelection[isel].mincounts || gbgammaCount>_partSelection[isel].maxcounts) { rejectedFromSelection = true; break; }
	  } else if (_partSelection[isel].partid == NGMSimpleParticleIdent::gbmuon){
	    if(gbmuonCount<_partSelection[isel].mincounts || gbmuonCount>_partSelection[isel].maxcounts) { rejectedFromSelection = true; break; }
	  } else if (_partSelection[isel].partid == NGMSimpleParticleIdent::mbgamma){
	    if(mbgammaCount<_partSelection[isel].mincounts || mbgammaCount>_partSelection[isel].maxcounts) { rejectedFromSelection = true; break; }
	  } else if (_partSelection[isel].partid == NGMSimpleParticleIdent::mbmuon){
	    if(mbmuonCount<_partSelection[isel].mincounts || mbmuonCount>_partSelection[isel].maxcounts) { rejectedFromSelection = true; break; }
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::lsgamma){
	    if(lsgammaCount<_partSelection[isel].mincounts || lsgammaCount>_partSelection[isel].maxcounts) { rejectedFromSelection = true; break; }
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::lsneutron){
	    if(lsneutronCount<_partSelection[isel].mincounts || lsneutronCount>_partSelection[isel].maxcounts) { rejectedFromSelection = true; break; }
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::lsmuon){
	    if(lsmuonCount<_partSelection[isel].mincounts || lsmuonCount>_partSelection[isel].maxcounts) { rejectedFromSelection = true; break; }
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::hettlid){
	      if(hettlidCount<_partSelection[isel].mincounts || hettlidCount>_partSelection[isel].maxcounts) { rejectedFromSelection = true; break; }
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::heid){
	      if(heidCount<_partSelection[isel].mincounts || heidCount>_partSelection[isel].maxcounts) { rejectedFromSelection = true; break; }
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::exttrig){
	      if(exttrigCount<_partSelection[isel].mincounts || exttrigCount>_partSelection[isel].maxcounts) { rejectedFromSelection = true; break; }
	  }else{
	    LOG<<"WARNING from"<<GetName()<<". Unsupported particle type "<< _partSelection[isel].partid<<ENDM_WARN;
	  }
	}
	
	// If rejected we'll try again when the next pulse comes in
	if(rejectedFromSelection)
   {
      _candidatePosition++;
      return true;
	}
   
	//Rejection of burst is not as straight forward. I now do it here to reject all the hits in the burst.
	rejectedFromSelection = false;
	for(int isel = 0; isel < (int)_partSelection.size(); isel++){
	  if(_partSelection[isel].partid == NGMSimpleParticleIdent::gbgamma) {
	    if(_partSelection[isel].mincounts<0 && gbgammaCount>=abs(_partSelection[isel].mincounts)) { rejectedFromSelection = true; break; }
	  } else if (_partSelection[isel].partid == NGMSimpleParticleIdent::gbmuon){
	    if(_partSelection[isel].mincounts<0 && gbmuonCount>=abs(_partSelection[isel].mincounts)) { rejectedFromSelection = true; break; } 	      
	  } else if (_partSelection[isel].partid == NGMSimpleParticleIdent::mbgamma){
	    if(_partSelection[isel].mincounts<0 && mbgammaCount>=abs(_partSelection[isel].mincounts)) { rejectedFromSelection = true; break; } 	      
	  } else if (_partSelection[isel].partid == NGMSimpleParticleIdent::mbmuon){
	    if(_partSelection[isel].mincounts<0 && mbmuonCount>=abs(_partSelection[isel].mincounts)) { rejectedFromSelection = true; break; }	      
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::lsgamma){
	    if(_partSelection[isel].mincounts<0 && lsgammaCount>=abs(_partSelection[isel].mincounts)) { rejectedFromSelection = true; break; }
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::lsneutron){
	    if(_partSelection[isel].mincounts<0 && lsneutronCount>=abs(_partSelection[isel].mincounts)) { rejectedFromSelection = true; break; }
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::lsmuon){
	    if(_partSelection[isel].mincounts<0 && lsmuonCount>=abs(_partSelection[isel].mincounts)) { rejectedFromSelection = true; break; }
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::hettlid){
	    if(_partSelection[isel].mincounts<0 && hettlidCount>=abs(_partSelection[isel].mincounts)) { rejectedFromSelection = true; break; }
	  }else if (_partSelection[isel].partid == NGMSimpleParticleIdent::exttrig){
	    if(_partSelection[isel].mincounts<0 && exttrigCount>=abs(_partSelection[isel].mincounts)) { rejectedFromSelection = true; break; }
	  }else{
	    LOG<<"WARNING from"<<GetName()<<". Unsupported particle type "<< _partSelection[isel].partid<<ENDM_WARN;
	  }
	}
   
	// If rejected we'll try again when the next pulse comes in
	if(rejectedFromSelection)
   {
      _candidatePosition++;
      return true;
   }

        if(doMultEnergyCut)
        {
          bool rejectedFromEnergyMultiplicityCut = true;
          // Test the mutliplicity cuts for 
          if( lsneutronESum > (lsnCutSlope*lsneutronCount + lsnCutConstant) ) rejectedFromEnergyMultiplicityCut = false;
          if( lsgammaESum > (lsgCutSlope*(lsgammaCount+lsmuonCount) + lsgCutConstant) ) rejectedFromEnergyMultiplicityCut = false;
          if( gbESum > (gbCutSlope*(gbgammaCount + gbmuonCount)+gbCutConstant) ) rejectedFromEnergyMultiplicityCut = false;

          // If rejected we'll try again when the next pulse comes in
          if(rejectedFromEnergyMultiplicityCut)
          {
            _candidatePosition++;
            return true;
          }
          
        }
        
   // Looks like we have a real burst so lets analyze and package it.
	_lastBurst =  hitvec.at(ihit-1)->getHit()->GetNGMTime();

	gBurst_gbgamma->SetPoint(_Num_burst, 1E-9*hitvec.at(i)->getHit()->TimeDiffNanoSec(_firstTime), -1.0*gbgammaCount);
	if(mbgammaCount)gBurst_mbgamma->SetPoint(_Num_burst, 1E-9*hitvec.at(i)->getHit()->TimeDiffNanoSec(_firstTime),-1.0*mbgammaCount);
	if(lsneutronCount)gBurst_lsneutron->SetPoint(_Num_burst, 1E-9*hitvec.at(i)->getHit()->TimeDiffNanoSec(_firstTime),lsneutronCount);
	if(lsmuonCount)gBurst_lsmuon->SetPoint(_Num_burst, 1E-9*hitvec.at(i)->getHit()->TimeDiffNanoSec(_firstTime),lsmuonCount);
	if(lsgammaCount)gBurst_lsgamma->SetPoint(_Num_burst, 1E-9*hitvec.at(i)->getHit()->TimeDiffNanoSec(_firstTime),-1.0*lsgammaCount);
	if(hettlidCount)gBurst_hettlid->SetPoint(_Num_burst, 1E-9*hitvec.at(i)->getHit()->TimeDiffNanoSec(_firstTime),hettlidCount);
	if(heidCount)gBurst_heid->SetPoint(_Num_burst, 1E-9*hitvec.at(i)->getHit()->TimeDiffNanoSec(_firstTime),heidCount);
	if(gbmuonCount)gBurst_gbmuon->SetPoint(_Num_burst, 1E-9*hitvec.at(i)->getHit()->TimeDiffNanoSec(_firstTime),-1.0*gbmuonCount);
	if(mbmuonCount)gBurst_mbmuon->SetPoint(_Num_burst, 1E-9*hitvec.at(i)->getHit()->TimeDiffNanoSec(_firstTime),-1.0*mbmuonCount);
	_Num_burst++;
	
	_burstLenHist->Fill(hitvec.at(ihit-1)->getHit()->TimeDiffNanoSec(hitvec.at(i)->getHit()->GetNGMTime()));
	
	// Find the average time of the burst using an average 
	// weighted by the inverse of the time to next hit
	Long64_t trigHitId = i;
//	if(getDebug()) { std::cout<<"Trig found for "; hitvec.at(trigHitId)->Print();}
	if(!_burstList)
	{
	    TString vString(hitvec.at(0)->IsA()->GetName());
	    //unused:	int vhit = TString(vString(vString.Length()-1,1)).Atoi();
		_burstList = new NGMBufferedPacketv2(-1,1,TTimeStamp(0,0),6);
	}
	_burstList->Clear();
	_burstList->setTimeStamp(TTimeStamp(hitvec.at(trigHitId)->getHit()->GetNGMTime().GetSec(),hitvec.at(trigHitId)->getHit()->GetNGMTime().GetNanoSec()));
	_burstList->setSlotId(-1);
	if(_pushFullWindow){
	  for(int lhit = 0; lhit < (int)hitvec.size(); lhit++)
		{
			if( fabs(hitvec.at(lhit)->getHit()->TimeDiffNanoSec(hitvec.at(trigHitId)->getHit()->GetNGMTime())) < _graphWindowNS/2.0 )
      {
				_burstList->addHit(hitvec.at(lhit)->getHit());
        // Mark that this hit has been marked as part of a burst
        hitvec.at(lhit)->_keep = false;
      }
			if(lhit == trigHitId)
				_burstList->setChannelId(_burstList->getPulseCount()-1);
		}
	} else {
	  for(int lhit = trigHitId; lhit < (int)hitvec.size(); lhit++)
		{
			if( fabs(hitvec.at(lhit)->getHit()->TimeDiffNanoSec(hitvec.at(trigHitId)->getHit()->GetNGMTime())) < _burstWindowNS )
      {
				_burstList->addHit(hitvec.at(lhit)->getHit());
        hitvec.at(lhit)->_keep = false;

      }
			if(lhit == trigHitId)
				_burstList->setChannelId(_burstList->getPulseCount()-1);
		}
	}
         
         FindBurstStart(_burstList);
   
         
	push(*((const TObject*)_burstList));

	anaBurst(trigHitId);
	anaBurstTiming(_burstList);
	_burstSizeHist->Fill(ihit-i);

//	//AMG TEST
//	if(fabs(hitvec.at(i)->getHit()->TimeDiffNanoSec(_lastDraw)) > 2E10){
//	  _lastDraw = hitvec.at(i)->getHit()->GetNGMTime();
//	  if(getDebug()) LOG<<"TRYING DRAW"<<ENDM_INFO;
//	    if(_drawGraphics) doDraw(hitvec.at(i)->getHit()->TimeDiffNanoSec(_firstTime)/1E9);
//	}

	
   }		      

  }
  
  _candidatePosition++;
  
  return true;
}

TPaveText*
NGMBurstMonitor::getParLabel(Double_t x1, Double_t y1, Double_t x2, Double_t y2, Option_t* option){
  TPaveText * parLabel = new TPaveText(x1,y1,x2,y2,option);
  char line[128];
  parLabel->SetTextAlign(22);
  parLabel->SetBorderSize(1);
  parLabel->SetTextSize(0.03);
  parLabel->AddText("Paramaters");
  parLabel->AddText("Min size:");
  sprintf(line,"%d",_hitsInBurst);
  parLabel->AddText(line);
  parLabel->AddText("Window (ns):");
  sprintf(line,"%d",(int)_burstWindowNS);
  parLabel->AddText(line);
  parLabel->AddText("Selections:");
  parLabel->AddText(" ID  Min   Max  ");
  for(int isel = 0; isel < (int)_partSelection.size(); isel++){
    sprintf(line,"%2d %4d %7d",_partSelection[isel].partid,_partSelection[isel].mincounts,_partSelection[isel].maxcounts);
    parLabel->AddText(line);
  }
  return parLabel;
}


bool
NGMBurstMonitor::anaBurst(int index){
  long count = _Num_burst;
  if(count > _NBurstZoomes) return false; //Only save the first 50 burst graphs

  if (!_legend) _legend = partID->GetLegend(0.88,0.3,0.995,0.995);
  if (!_burstParLabel) _burstParLabel = getParLabel(0.88,0.5,0.995,0.995,"NDC");

  if (hitvec.at(index)->getHit()->TimeDiffNanoSec(hitvec.front()->getHit()->GetNGMTime()) < _graphWindowNS/2.0){
    LOG<<"WARNING from"<<GetName()<<". Not enough hits for early time."<<ENDM_WARN;
  }
  if(hitvec.back()->getHit()->TimeDiffNanoSec(hitvec.at(index)->getHit()->GetNGMTime()) < _graphWindowNS/2.0) {
    LOG<<"WARNING from"<<GetName()<<". Not enough hits for late time."<<ENDM_WARN;
  }   
  if(getDebug())
	LOG<<"Burst "<<count<<" at "<<
		hitvec.at(index)->getHit()->GetNGMTime().GetSec()<<"s "<<hitvec.at(index)->getHit()->GetNGMTime().GetNanoSec()<<ENDM_INFO;

  TString runStr;
  runStr+=count;
  if(!cBurst) return false;
  cBurst->cd();
  TPad* _pad = new TPad("_pad",runStr.Data(),0,0,1,1);
  _pad->SetNumber(255);//Stupid kluge. See NGMMarker
  padvec.push_back(_pad);

  _pad->Divide(1,2);
  char graphTitle[256];
  runStr="";
  runStr+=_runNumber;
  sprintf(graphTitle,"Run %s, Time %u %u %u ;Time (s); Total Counts", runStr.Data(),
	  hitvec.at(index)->getHit()->GetNGMTime().GetDate(), hitvec.at(index)->getHit()->GetNGMTime().GetTime(), hitvec.at(index)->getHit()->GetNGMTime().GetNanoSec());
  TGraph* countGraph = new TGraph(hitvec.size());
  countGraph->SetTitle(graphTitle);
  TGraph* trigFallGraph = new TGraph(hitvec.size()-1);
  sprintf(graphTitle,"Run %s, Time %u %u %u ;Time (s); Time Till Next (ns)", runStr.Data(),
	  hitvec.at(index)->getHit()->GetNGMTime().GetDate(), hitvec.at(index)->getHit()->GetNGMTime().GetTime(), hitvec.at(index)->getHit()->GetNGMTime().GetNanoSec());
  trigFallGraph->SetTitle(graphTitle);

  
  sprintf(graphTitle,"%s_ZoomRate_%d",GetName(),(int)count);
  countGraph->SetName(graphTitle);
  sprintf(graphTitle,"%s_ZoomWaterFall_%d",GetName(),(int)count);
  trigFallGraph->SetName(graphTitle);
 
  
  for(int i=0; i<(int)hitvec.size(); i++){
    countGraph->SetPoint(i,-1E-9 * hitvec.at(index)->getHit()->TimeDiffNanoSec(hitvec.at(i)->getHit()->GetNGMTime()),i+1);
    if(i+1!=(int)hitvec.size()) trigFallGraph->SetPoint(i,-1E-9 * hitvec.at(index)->getHit()->TimeDiffNanoSec(hitvec.at(i)->getHit()->GetNGMTime()),
				    hitvec.at(i+1)->getHit()->TimeDiffNanoSec(hitvec.at(i)->getHit()->GetNGMTime()));
  }
  _pad->cd(1);
  TH1F* h1 = new TH1F("htemp","",20000,_graphWindowNS*-0.5E-9,_graphWindowNS*0.5E-9);
  h1->SetStats(false);
  h1->SetDirectory(0);
  h1->SetTitle(countGraph->GetTitle());
  h1->Draw();
  //countGraph->Draw("*");
  int ih=0;
  for(int i=0; i<(int)hitvec.size()-1; i++){
    NGMMarker* mark = new NGMMarker;
    mark->SetX(-1E-9 * hitvec.at(index)->getHit()->TimeDiffNanoSec(hitvec.at(i)->getHit()->GetNGMTime()));
    if(mark->GetX() < _graphWindowNS*-0.5E-9) {
      delete mark;
      continue; 
    } else if(mark->GetX() > _graphWindowNS*0.5E-9){
      delete mark;
      break;
    }
    ih++;
    mark->SetY(ih);
    mark->SetMarkerStyle(partID->GetStyle(hitvec.at(i)->getHit()));
    mark->SetMarkerColor(partID->GetColor(hitvec.at(i)->getHit()));
	mark->SetLabel(partID->GetName(hitvec.at(i)->getHit()));
    mark->Draw();
  }
  h1->GetYaxis()->SetRangeUser(0,1.1*ih);
  h1->GetYaxis()->SetLimits(0,1.1*ih);

  h1 = new TH1F("htemp","",20000,_graphWindowNS*-0.5E-9,_graphWindowNS*0.5E-9);
  h1->SetStats(false);
  h1->SetDirectory(0);
  _pad->cd(2)->SetLogy();
  h1->SetTitle(trigFallGraph->GetTitle());
  h1->Draw();
  //trigFallGraph->Draw("*");
  for(int i=0; i<(int)hitvec.size()-1; i++){
    NGMMarker* mark = new NGMMarker;
    mark->SetMarkerStyle(partID->GetStyle(hitvec.at(i)->getHit()));
    mark->SetMarkerColor(partID->GetColor(hitvec.at(i)->getHit()));
    mark->SetLabel(partID->GetName(hitvec.at(i)->getHit()));
	mark->SetX(-1E-9 * hitvec.at(index)->getHit()->TimeDiffNanoSec(hitvec.at(i)->getHit()->GetNGMTime()));
    mark->SetY(hitvec.at(i+1)->getHit()->TimeDiffNanoSec(hitvec.at(i)->getHit()->GetNGMTime()));
    if((mark->GetX() < _graphWindowNS*-0.5E-9) ||
       (mark->GetX() > _graphWindowNS*0.5E-9))  continue;
    mark->Draw();
  }
  h1->GetYaxis()->SetRangeUser(0.1,1E7);
  _pad->cd(1);
  _legend->Draw();
  _pad->cd(1)->SetGridx();
  _pad->cd(1)->SetPad(0,0.5,1,1);
  _pad->cd(1)->SetRightMargin(0.12);
  _pad->cd(2)->SetGridy();
  _pad->cd(2)->SetPad(0,0,1,0.5);
  _pad->cd(2)->SetRightMargin(0.12);
  _burstParLabel->Draw();
  _pad->cd(0);

  //NEED TO FIND A WAY TO SAVE IDENTIFIED GRAPHS. SAVE THE PLAIN FOR NOW
  GetParentFolder()->Add(countGraph);
  
  GetParentFolder()->Add(trigFallGraph);
  //cBurst->cd();

  //Uncomment to save the graphs
  /*
  _pad->Draw();
  char fname[128];
  sprintf(fname,"burst_%d.pdf",count);
  _pad->SaveAs(fname);
  cBurst->GetListOfPrimitives()->Remove(_pad);
  cBurst->Clear();
  */
  //gSystem->Sleep(100);

  return true;
}

void NGMBurstMonitor::doDraw(double hiVal, bool zoomer){
  
  //Draw to zoomer or standard Module canvas depending on zoomer variable
  
  // don't do zoomer in batch mode
  if(zoomer && gROOT->IsBatch()) return;
  
  if(!_parLabel) _parLabel = getParLabel(0.88,0.1,0.995,0.6,"NDC");
  TCanvas* c2 = 0;
  if(zoomer){
    if(!_myZoomer) _myZoomer = new NGMZoomGui;
    c2 = _myZoomer->getCanvas();
    c2->SetGridy();
    c2->Clear();
    c2->SetPad(0,0,1,1);
    c2->SetRightMargin(0.12);
    c2->cd();    
  }else{
    TString cName;
    cName.Form("%s_Canvas",GetName());
    c2 = (TCanvas*)(gROOT->FindObject(cName.Data()));
    if(c2){
      c2->cd(1);
    }
  }
  if(!c2)
  {
    LOG<<"Unable to find or create canvas"<<ENDM_WARN;
  }
  
  gBurst_gbgamma->Draw("A B");
  c2->Update();
  double maxes[9] = {TMath::MaxElement(gBurst_gbgamma->GetN(),gBurst_gbgamma->GetY()),
		     TMath::MaxElement(gBurst_mbgamma->GetN(),gBurst_mbgamma->GetY()),
		     TMath::MaxElement(gBurst_lsneutron->GetN(),gBurst_lsneutron->GetY()),
		     TMath::MaxElement(gBurst_lsgamma->GetN(),gBurst_lsgamma->GetY()),
		     TMath::MaxElement(gBurst_hettlid->GetN(),gBurst_hettlid->GetY()),
		     TMath::MaxElement(gBurst_heid->GetN(),gBurst_heid->GetY()),
		     TMath::MaxElement(gBurst_mbmuon->GetN(),gBurst_mbmuon->GetY()),
		     TMath::MaxElement(gBurst_gbmuon->GetN(),gBurst_gbmuon->GetY()),
			 TMath::MaxElement(gBurst_lsmuon->GetN(),gBurst_lsmuon->GetY())};
  double mins[9] = {TMath::MinElement(gBurst_gbgamma->GetN(),gBurst_gbgamma->GetY()),
		    TMath::MinElement(gBurst_mbgamma->GetN(),gBurst_mbgamma->GetY()),
		    TMath::MinElement(gBurst_lsneutron->GetN(),gBurst_lsneutron->GetY()),
		    TMath::MinElement(gBurst_lsgamma->GetN(),gBurst_lsgamma->GetY()),
		    TMath::MinElement(gBurst_hettlid->GetN(),gBurst_hettlid->GetY()),
		    TMath::MinElement(gBurst_heid->GetN(),gBurst_heid->GetY()),
		    TMath::MinElement(gBurst_mbmuon->GetN(),gBurst_mbmuon->GetY()),
		    TMath::MinElement(gBurst_gbmuon->GetN(),gBurst_gbmuon->GetY()),
		    TMath::MinElement(gBurst_lsmuon->GetN(),gBurst_lsmuon->GetY())};
  gBurst_gbgamma->GetYaxis()->SetRangeUser(1.1*TMath::MinElement(9,mins),1.1*TMath::MaxElement(9,maxes));
  gBurst_gbgamma->GetXaxis()->SetRangeUser(0,gBurst_gbgamma->GetXaxis()->GetXmax());

  TLegend *leg = new TLegend(0.88,0.995,0.995,0.60);
  leg->AddEntry(gBurst_gbgamma,"gbgamma","lp");
  if (gBurst_mbgamma->GetN()>0) { 
    gBurst_mbgamma->Draw("B");
    TGraph* tg = (TGraph*)gBurst_mbgamma->DrawClone("P");
    tg->SetMarkerStyle(kFullSquare);
    tg->SetMarkerSize(0.6);
    leg->AddEntry(tg,"mbgamma","lp");
  }
  if (gBurst_lsneutron->GetN()>0){
    gBurst_lsneutron->Draw("B");
    TGraph* tg = (TGraph*)gBurst_lsneutron->DrawClone("P");
    tg->SetMarkerStyle(kFullSquare);
    tg->SetMarkerSize(0.6);
    leg->AddEntry(tg,"lsneutron","lp");
  }  
  if (gBurst_lsgamma->GetN()>0){
    gBurst_lsgamma->Draw("B");
    TGraph* tg = (TGraph*)gBurst_lsgamma->DrawClone("P");
    tg->SetMarkerStyle(kFullSquare);
    tg->SetMarkerSize(0.6);
    leg->AddEntry(tg,"lsgamma","lp");
  }
  if (gBurst_hettlid->GetN()>0){ 
    gBurst_hettlid->Draw("B");
    TGraph* tg = (TGraph*)gBurst_hettlid->DrawClone("P");
    tg->SetMarkerStyle(kFullSquare);
    tg->SetMarkerSize(0.6);
    leg->AddEntry(tg,"hettl","lp");
  }
  if (gBurst_heid->GetN()>0){
    gBurst_heid->Draw("B");
    TGraph* tg = (TGraph*)gBurst_heid->DrawClone("P");
    tg->SetMarkerStyle(kFullSquare);
    tg->SetMarkerSize(0.6);
    leg->AddEntry(tg,"he","lp");
  }
  if (gBurst_gbmuon->GetN()>0){
	gBurst_gbmuon->Draw("B");
	TGraph* tg = (TGraph*)gBurst_gbmuon->DrawClone("P");
	tg->SetMarkerStyle(kFullSquare);
	tg->SetMarkerSize(0.6);
	leg->AddEntry(tg,"gbmuon","lp");
  }
  if (gBurst_mbmuon->GetN()>0){
	gBurst_mbmuon->Draw("B");
	TGraph* tg = (TGraph*)gBurst_mbmuon->DrawClone("P");
	tg->SetMarkerStyle(kFullSquare);
	tg->SetMarkerSize(0.6);
	leg->AddEntry(tg,"mbmuon","lp");
   }
  if (gBurst_lsmuon->GetN()>0){
	gBurst_lsmuon->Draw("B");
	TGraph* tg = (TGraph*)gBurst_mbmuon->DrawClone("P");
	tg->SetMarkerStyle(kFullSquare);
	tg->SetMarkerSize(0.6);
	leg->AddEntry(tg,"lsmuon","lp");
   }

  gBurst_gbgamma->Draw("B"); //redraw to bring to front
  leg->Draw();
  _parLabel->Draw();
  gBurst_gbgamma->GetXaxis()->SetLimits(0,gBurst_gbgamma->GetXaxis()->GetBinUpEdge(gBurst_gbgamma->GetXaxis()->GetNbins()));
  TF1* myf = new TF1("myf","0",-1E100,1E100);
  myf->SetLineWidth(1);
  myf->Draw("same");
  
  
  if(zoomer&&cBurst){
  //Lets draw the markers that let you navigate the burst graphs
    for(int ip=0;ip<(int)padvec.size();ip++){
      int iburst = atoi(padvec.at(ip)->GetTitle());
      NGMMarker* mark = new NGMMarker;
      mark->SetMarkerStyle(kOpenSquare);
      mark->SetActive();
      //    if(zoomer&&cBurst) 
      mark->SetAssociated(cBurst,padvec.at(ip));
      mark->SetY(0);
      double x=0;
      double y=0;
      gBurst_gbgamma->GetPoint(iburst-1,x,y);
      mark->SetX(x);
      c2->cd();
      mark->Draw();
    }
  }
  if(zoomer){
    _myZoomer->setAxis(gBurst_gbgamma->GetXaxis());
    if(hiVal>0){
      _myZoomer->doSlider1((Int_t)(2/hiVal*gBurst_gbgamma->GetXaxis()->GetNbins()));
      _myZoomer->doSlider2((Int_t)(gBurst_gbgamma->GetXaxis()->GetNbins()*0.91));
    }
  }
}

bool NGMBurstMonitor::anaBurstTiming(NGMBufferedPacket* burst)
{
  //unused:   const double histEnergyMax = 20000.0;
	const double timingMax = 100000.0;
	const double timingMin = -100000.0;
	const int timingBins = 200000;
	char cbuf[1024];
	
	int refHitidx = burst->getChannelId();
	NGMTimeStamp refTime = burst->getHit(refHitidx)->GetNGMTime();
	
	// Lets fill time-energy histograms for each hit in the chain
	for(int ihit = 0; ihit < burst->getPulseCount(); ihit++)
	{
		const NGMHit* tHit = burst->getHit(ihit);
		int ptype = partID->GetType(tHit);
    if(ptype<1) continue;
		// Does Histogram exist? If not lets create it now.
		if(!_burstTimingSummary) return false;
		if(!_burstTimingSummary[ptype-1])
		{
			sprintf(cbuf,"%s_TimingProfile_%s",GetName(),partID->GetParticleName(ptype));
			_burstTimingSummary[ptype-1] = new TH1D( cbuf, cbuf, timingBins, timingMin, timingMax);
			GetParentFolder()->Add(_burstTimingSummary[ptype-1]);
      _burstTimingSummary[ptype-1]->SetDirectory(0);
		}
		_burstTimingSummary[ptype-1]->Fill(burst->getHit(ihit)->TimeDiffNanoSec(refTime));
	}
	
	return true;
}

void  NGMBurstMonitor::LaunchDisplayCanvas()
{
  if(!_drawGraphics) return;
	TCanvas* cMonDisplay = 0;
	char cName[1024];
	sprintf(cName,"%s_Canvas",GetName());
	cMonDisplay = (TCanvas*) (gROOT->FindObject(cName));
	if(!cMonDisplay)
	{
		LOG<<" Creating canvas "<<cName<<ENDM_INFO;
		cMonDisplay = new TCanvas(cName,cName);
    GetParentFolder()->Add(cMonDisplay);
		cMonDisplay->Divide(1,2);
	}
  // Draw Knapp plot on first canvas
  doDraw(0.0,0);
  
  TVirtualPad* tpad1 = cMonDisplay->cd(2);
  bool firstDrawn = true;
	
	double minx = 0.0;
	double maxx = 0.0;
	for(int ipart = 0; ipart < partID->GetMaxParticleTypes(); ipart++)
  {
    if(_burstTimingSummary[ipart])
    {	
			double tminx = +1E9;
			double tmaxx = -1E9;
			for(int ibin = 1; ibin <= _burstTimingSummary[ipart]->GetNbinsX(); ibin++)
				if(_burstTimingSummary[ipart]->GetBinContent(ibin)>0)
				{
					tminx = _burstTimingSummary[ipart]->GetXaxis()->GetBinLowEdge(ibin);
					break;
				}
			for(int ibin = _burstTimingSummary[ipart]->GetNbinsX(); ibin > 0; ibin--)
				if(_burstTimingSummary[ipart]->GetBinContent(ibin)>0)
				{
					tmaxx = _burstTimingSummary[ipart]->GetXaxis()->GetBinCenter(ibin);
					break;
				}
			if(tminx<minx) minx = tminx;
			if(tmaxx>maxx) maxx = tmaxx;
		}
	}
	
	
  for(int ipart = 0; ipart < partID->GetMaxParticleTypes(); ipart++)
  {
    if(_burstTimingSummary[ipart])
    {
      _burstTimingSummary[ipart]->SetLineColor(partID->GetColor(ipart+1));
      if(firstDrawn){
				_burstTimingSummary[ipart]->GetXaxis()->SetRangeUser(minx,maxx);
        _burstTimingSummary[ipart]->Draw();
        firstDrawn = false;
      }else{
        _burstTimingSummary[ipart]->Draw("same");
      }
    }
  }
  tpad1->SetLogy();
	tpad1->Modified();
	tpad1->Update();
}

bool NGMBurstMonitor::FindBurstStart(NGMBufferedPacket* packet)
{
  // Hardecoded to use the first in burst as start point
   packet->setChannelId(0);
   return false;

   if(packet->getPulseCount()<2) return false;
 
   // First lets form a time to next distribution
   double mintime = 1E20;
   int minidx = -1;
   double meantime = 0.0;
   for(int ihit = 0; ihit < packet->getPulseCount()-1;ihit++)
   {
      double timediff = packet->getHit(ihit+1)->TimeDiffNanoSec(packet->getHit(ihit)->GetNGMTime());
      meantime+= timediff;
      if(minidx == -1)
      {
         mintime = timediff;
         minidx = ihit;
      }else{
         if(timediff < mintime)
         {
            mintime = timediff;
            minidx = ihit;
         }
      }
   }
   
   meantime = meantime/(double)(packet->getPulseCount() -1);
   
   // Now lets form the variance   
   double vartime = 0.0;
   for(int ihit = 0; ihit < packet->getPulseCount()-1;ihit++)
   {
      vartime+= pow(packet->getHit(ihit+1)->TimeDiffNanoSec(packet->getHit(ihit)->GetNGMTime()) - meantime, 2.0);
   }
   vartime = sqrt(vartime/(double)(packet->getPulseCount() -1));
   
   // Lets find the first one that is one standard deviation away
   int firstLow = -1;
   for(int ihit = 0; ihit < packet->getPulseCount()-1;ihit++)
   {
      double timediff = packet->getHit(ihit+1)->TimeDiffNanoSec(packet->getHit(ihit)->GetNGMTime());
      if(timediff < meantime/10.0)
      {
         firstLow = ihit;
         break;
      }  
   }
   
   if(firstLow == -1)
   {
      packet->setChannelId(minidx);
   }else{
      packet->setChannelId(firstLow);
   }
   return true;
}

void NGMBurstMonitor::Print(Option_t* option) const
{
   TCanvas* nMonDisplay = 0;
	char cName[1024];
	sprintf(cName,"%s_Canvas",GetName());
	nMonDisplay = (TCanvas*) (gROOT->FindObjectAny(cName));
  if(!nMonDisplay) return;
   TString printName;
   printName+=_runNumber;
   printName+="_";
   printName+=cName;
   printName+=".eps";
   nMonDisplay->Print(printName.Data());
}

double NGMBurstMonitor::MeasureShape(int partid, int distid, double threshold)
{
  TString histname;
  TString sPart;
  TH1* hTime = 0;
  switch (distid) {
    case 1: // Length in Time Distribution
      if(!_burstLenHist){
        histname.Form("%s_%s",GetName(),"BurstLenDist");
        _burstLenHist = (TH1F*)(GetParentFolder()->FindObject(histname.Data()));
        if(!_burstLenHist)
        {
          LOG<<" Histogram "<<histname.Data()<<" not found."<<ENDM_WARN;
          return -1.0;
        }
      }
      return MeasureShape(_burstLenHist, threshold);
      break;
    case 2: // Time Profile Distribution
      if(!partID) return -1.0;
       sPart = partID->GetParticleName(partid);
      histname.Form("%s_TimingProfile_%s",GetName(),sPart.Data());
      if(!hTime){
        hTime = (TH1*)(GetParentFolder()->FindObject(histname.Data()));
        if(!hTime)
        {
          LOG<<" Histogram "<<histname.Data()<<" not found."<<ENDM_WARN;
          return -1.0;
        }
      }
      return MeasureShape(hTime, threshold);
      break;
    case 0: // Multiplicity Distribution
    default:
      //find shape
      if(!_burstSizeHist){
        histname.Form("%s_%s",GetName(),"BurstSizeDist");
        _burstSizeHist = (TH1F*) (GetParentFolder()->FindObject(histname.Data()));
        if(!_burstSizeHist)
        {
          LOG<<" Histogram "<<histname.Data()<<" not found."<<ENDM_WARN;
          return -1.0;
        }
      }
      return MeasureShape(_burstSizeHist, threshold);
      break;

  }
  
  
  return -1.0;
}

double NGMBurstMonitor::MeasureShape(TH1* tDist, double threshold)
{
  if(!tDist) return 0.0;
  double integral = tDist->Integral();
  double sum = tDist->GetBinContent(1);
  double prevsum = sum;
  double actthreshold = threshold*integral;
  
  for(int ibin = 2; ibin <= tDist->GetNbinsX();ibin++)
  {
    sum = prevsum + tDist->GetBinContent(ibin);
    if(sum/integral>threshold)
    {
      if(prevsum == 0.0)
      {
        LOG<<" Threshold crossed in the first bin."<<ENDM_WARN;
        return 0.0;
      }
      //We have crossed...
      return ( actthreshold - prevsum )/tDist->GetBinContent(ibin)*tDist->GetXaxis()->GetBinWidth(ibin) + tDist->GetXaxis()->GetBinLowEdge(ibin-1);
    }
    prevsum = sum;
  }
  return 0.0;
}

void NGMBurstMonitor::MeasureShapeWriteDB(int partid, int distid, double threshold)
{
  TString modname(GetName());
  TString passname = NGMSystem::getSystem()->GetPassName();
  TSQLServer* db = 0;
  TSQLResult* res = 0;
  
  db = GetDBConnection();
  if(!db)
  {
    LOG<<"Error accessing database"<<ENDM_WARN;
    return;
  }
  
  double shape = MeasureShape( partid, distid, threshold );
  double shape_error = 0.0;
  
  // Convert Varibles to strings
  TString srunnumber; srunnumber+=_runNumber;
  TString sduration; sduration.Form("%f",_gapfinder.GetLiveTime());
  TString sdwelltime; sdwelltime.Form("%f",_gapfinder.GetRunDuration());
  TString spartid; spartid.Form("%d",partid);
  TString sdistributiontype; sdistributiontype.Form("%d",distid);
  TString sthreshold; sthreshold.Form("%f",threshold);
  TString sshape; sshape.Form("%f",shape);
  TString sshape_error; sshape_error.Form("%f",shape_error);
  
  //First test if this entry is already in database
  TString sqlStatement("select runnumber from countdist where runnumber=\"");
  sqlStatement+=_runnumber;
  sqlStatement+="\" AND pass=\"";
  sqlStatement+=passname;
  sqlStatement+="\" AND modulename=\"";
  sqlStatement+=modname;
  sqlStatement+="\"";
  
  res = db->Query(sqlStatement);
  bool entryfound = false;
  if(res->GetRowCount()>0)
    entryfound = true;
  delete res;
  
  // Lets always Add a new entry in the database
  // a new entry will be added each time plot is updated
  // with runduration providing accumulating data time.
  entryfound = false;
  
  // Update Database Entry
  TString updateStatement;
  if(entryfound){
    updateStatement="UPDATE burstshape SET";
  }else{
    updateStatement="INSERT INTO burstshape SET";
    updateStatement+=" runnumber=\""; updateStatement+=srunnumber; updateStatement+="\", ";
    updateStatement+=" pass=\""; updateStatement+=passname; updateStatement+="\", ";
    updateStatement+=" modulename=\""; updateStatement+=modname; updateStatement+="\",";
  }
  
  updateStatement+=" runduration=\""; updateStatement+=sduration; updateStatement+="\", ";
  updateStatement+=" dwelltime=\""; updateStatement+=sdwelltime; updateStatement+="\", ";
  updateStatement+=" partid=\""; updateStatement+=sdistributiontype; updateStatement+="\", ";
  updateStatement+=" distributiontype=\""; updateStatement+=sduration; updateStatement+="\", ";
  updateStatement+=" threshold=\""; updateStatement+=sthreshold; updateStatement+="\", ";
  updateStatement+=" shape=\""; updateStatement+=sshape; updateStatement+="\", ";
  updateStatement+=" shape_error=\""; updateStatement+=sduration; updateStatement+="\" ";
  
  if(entryfound){
    updateStatement+="WHERE runnumber=\"";
    updateStatement+=srunnumber;
    updateStatement+="\" AND pass=\"";
    updateStatement+=passname;
    updateStatement+="\" AND modulename=\"";
    updateStatement+=modname;
    updateStatement+="\"";        
  }
  
  
  // Execute insert
  std::cout<<updateStatement.Data()<<std::endl;
  res = db->Query(updateStatement.Data());
  delete res;
  db->Close();
  delete db;
  
  
}
