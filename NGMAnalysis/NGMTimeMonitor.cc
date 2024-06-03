/*
 *  NGMTimeMonitor.cpp
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/28/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <math.h>
#include "TH1.h"
#include "TH2.h"
#include "NGMLogger.h"
#include "NGMBufferedPacket.h"
#include "NGMSystemConfiguration.h"
#include "NGMTimeMonitor.h"
#include "NGMConfigurationTable.h"
#include "TSpectrum.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TList.h"
#include "TGraph.h"
#include "TClonesArray.h"
#include "TCanvas.h"
#include "TMarker.h"
#include "TSystem.h"
#include "TRandom.h"
#include "TVirtualPad.h"
#include "TThread.h"
#include "TH2.h"
#include "TF1.h"
#include "TObjString.h"
#include "NGMParticleIdent.h"
#include "NGMSimpleParticleIdent.h"
#include "THnSparse.h"
#include "TFolder.h"
#include "NGMSystem.h"
#include "TDirectory.h"

ClassImp(NGMTimeMonitor)

double* genLogBins(int nbins, double axismin, double axismax)
{
  double* dbins = new double[nbins+1];
  for(int ibin = 0; ibin <= nbins; ibin++)
  {
    dbins[ibin] = axismin*pow(axismax/axismin, ((double)ibin)/nbins);
  }
  return dbins; 
}

NGMTimeMonitor::NGMTimeMonitor()
{
  _timeDistributionPerChannel = 0;
  _prevTime=NGMTimeStamp(0,0);
  _firstTime= NGMTimeStamp(0,0);
  _hitsAnalyzed = 0;
  _hCfdWordMonitor = 0;
  _hitList = 0;
  _timeComparisonShort = 0;
  _timeComparisonLong = 0;
  _fillAllChannelByChannel =false;

  maxNanosecondsInList = 100000;
  partID = 0;
  _timerange_ns = 200.0;
  _timeTuple = 0;
  _doTuple = false;
  _nearestNeighborSkip = 1;
}

NGMTimeMonitor::NGMTimeMonitor(const char* name, const char* title)
: NGMModule(name,title)
{
  _prevTime=NGMTimeStamp(0,0);
  _firstTime= NGMTimeStamp(0,0);
  _hitsAnalyzed = 0;
  _hCfdWordMonitor = 0;
  _hitList = 0;
  _timeComparisonShort = 0;
  _timeComparisonLong = 0;
  _fillAllChannelByChannel =false;
  _timerange_ns = 200.0;

  maxNanosecondsInList = 100000;
  _timeDistributionPerChannel = 0;
  _timeTuple = 0;
  _doTuple = true;
  partID = new NGMSimpleParticleIdent();
  _nearestNeighborSkip = 1;
}

NGMTimeMonitor::~NGMTimeMonitor(){
  // Definitely need to clean up allocated memory here
  if(_hCfdWordMonitor) delete _hCfdWordMonitor;

}


bool NGMTimeMonitor::init(){

  return true;
}

void NGMTimeMonitor::enableDisplay(bool newVal)
{
  if(newVal)
  {
    TThread::Lock();
    _displaytimer.Connect("Timeout()","NGMTimeMonitor",this,"LaunchDisplay()");
    _displaytimer.Start(10000, kFALSE);
    TThread::UnLock();
  }else{
    _displaytimer.Stop();
  }
  
}

bool NGMTimeMonitor::process(const TObject &tData){
  
  static TClass* tNGMHitType = gROOT->GetClass("NGMHit");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  static TClass* tObjStringType = gROOT->GetClass("TObjString");
  //Check data type
  if(tData.InheritsFrom(tNGMHitType)){
    
    const NGMHit* tHit = (const NGMHit*)(&tData);
    if(partID != 0 && partID->IsSelected(tHit)) analyzeHit(tHit);
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
    _runNumber = confBuffer->getRunNumber();
    partID->Init(confBuffer);
    _hitsAnalyzed = 0;
    if(_timeTuple)
    {
      LOG<<"Closing file: "<<_timeTuple->GetCurrentFile()->GetName()<<ENDM_INFO;
      //need to close
      _timeTuple->GetCurrentFile()->ls();
      _timeTuple->GetCurrentFile()->Write();
      _timeTuple->GetCurrentFile()->Close();
      _timeTuple=0;
    }
    if(_doTuple)
    {
      TString fTupleName;
      fTupleName.Form("%s_%ld.root",GetName(),_runNumber);
      TDirectory* prevDirectory = gDirectory;
      LOG<<"Opening file: "<<fTupleName.Data()<<ENDM_INFO;
      TFile* fTuple= TFile::Open(fTupleName.Data(),"RECREATE");
      _timeTuple= new TTree("tt","Time");
      _timeTuple->Branch("tt", &_tTp, "ch1/S:ch2:e1/F:e2:n1:n2:g1:g2:psd1:psd2:t");
      fTuple->ls();
      prevDirectory->cd();
    }
    
    if(!_hCfdWordMonitor)
    {
      TThread::Lock();
      char hname[1024];
      sprintf(hname,"%s_%s",GetName(),"CFDWordMonitor");
      //LOG<<"Creating histogram "<<hname<<ENDM_INFO;
      _hCfdWordMonitor = new TH2F(hname,hname,partID->getNumberOfChannels(),0.0,partID->getNumberOfChannels(),
                                 256,0.0,256.0); // Units of clock cycles
      _hCfdWordMonitor->SetDirectory(gROOT);
      _hCfdWordMonitor->SetXTitle("Hardware Channel");
      _hCfdWordMonitor->SetYTitle("CFD Timing [Sample Clocks]");
      _hCfdWordMonitor->SetDirectory(0);
      // This means the output manager will save this histogram with the other summary output
      GetParentFolder()->Add(_hCfdWordMonitor);
      TThread::UnLock();
    }
    if(!_timeComparisonShort)
    {
      TThread::Lock();
      char hname[1024];
      sprintf(hname,"%s_%s",GetName(),"TimeComparisonShort");
      //LOG<<"Creating histogram "<<hname<<ENDM_INFO;
      _timeComparisonShort = new TH2F(hname,hname,partID->getNumberOfChannels(),0.0,partID->getNumberOfChannels(),
				      (int)(_timerange_ns*4.0),-_timerange_ns,_timerange_ns); // Units of clock cycles
      _timeComparisonShort->SetXTitle("Hardware Channel");
      _timeComparisonShort->SetYTitle("Relative Time [ns]");
      _timeComparisonShort->SetDirectory(0);
      // This means the output manager will save this histogram with the other summary output
      GetParentFolder()->Add(_timeComparisonShort);
      TThread::UnLock();
    }
    if(!_timeComparisonLong)
    {
      TThread::Lock();
      char hname[1024];
      sprintf(hname,"%s_%s",GetName(),"TimeComparisonLong");
      //LOG<<"Creating histogram "<<hname<<ENDM_INFO;
      _timeComparisonLong = new TH2F(hname,hname,partID->getNumberOfChannels(),0.0,partID->getNumberOfChannels(),
                                      _timerange_ns*4,-100.0*_timerange_ns,100.0*_timerange_ns); // Units of clock cycles
      _timeComparisonLong->SetXTitle("Hardware Channel");
      _timeComparisonLong->SetYTitle("Relative Time [ns]");
      _timeComparisonLong->SetDirectory(0);
      // This means the output manager will save this histogram with the other summary output
      GetParentFolder()->Add(_timeComparisonLong);
      TThread::UnLock();
    }


    
    initializeTimePlots();
    
    ResetHistograms();
    push(tData);
  }else if(tData.IsA() == tObjStringType){
     const TObjString* controlMessage = (const TObjString*)(&tData);
     if(controlMessage->GetString() == "EndSpillFlush")
     {
        if(_hitList&&getDebug())
        {
           LOG<<"Hits in list "<<_hitList->GetEntries()<<ENDM_INFO;
        }
        LaunchDisplayCanvas();
     }else if(controlMessage->GetString() == "EndRunSave")
     {
       
       LaunchDisplayCanvas();
       if(_timeTuple)
       {
         if(_timeTuple->GetCurrentFile())
         {
           
             LOG<<"Closing file: "<<_timeTuple->GetCurrentFile()->GetName()<<ENDM_INFO;
             //need to close
             _timeTuple->GetCurrentFile()->ls();
             _timeTuple->GetCurrentFile()->Write();
             _timeTuple->GetCurrentFile()->Close();
             _timeTuple=0;
           
         }
       }
     }else if(controlMessage->GetString() == "PlotUpdate")
     {
       LaunchDisplayCanvas();
     }
     push(tData);
  }
  return true;  
}


bool NGMTimeMonitor::finish(){

  return true;
}

bool NGMTimeMonitor::analyzeHit(const NGMHit* tHit){
  
  // This plotIndex variable calculation is specific to the Struck system...
  // This plot index really corresponds to something like the readout channel
  // and should be a variable extracted from the SystemConfiguration Table
  // for now its hardcoded...
  int totalChannels = partID->getNumberOfChannels();
  int plotIndex = partID->getPlotIndex(tHit);    
  _hitsAnalyzed++;
  if(_hitsAnalyzed == 1)
  {
    _prevTime = tHit->GetTimeStamp();
    _firstTime = tHit->GetTimeStamp();
    return true;
  }


  _hCfdWordMonitor->Fill(plotIndex,tHit->GetCFDWord()/256.0);

  /// Add this hit to our local chain
  /// We add a copy so we can keep it around as long as we like...
  _hitList->AddLast(tHit->DuplicateHit());

  // Remove those outside new window
  while(1){
    NGMHit* firstHit = (NGMHit*)  _hitList->First();
    NGMHit* lastHit = (NGMHit*) _hitList->Last();
    //if(!firstHit || !lastHit) break;
    double timeDiffns = lastHit->TimeDiffNanoSec(firstHit->GetNGMTime());
    if(fabs(timeDiffns)>maxNanosecondsInList){
      NGMHit* hitToRemove = (NGMHit*)(_hitList->First());
      _hitList->RemoveFirst();
      delete hitToRemove;
    }else{
      break;
    }
  }

  
  // We check if the first hit or more is outside our time window
  //unused:  int hitListSize = _hitList->GetSize();
    
  // Fill Channel by channel time differences
  // Use TIter for TList (doubly linked list)   
  TListIter tHitIter(_hitList);
  NGMHit* lastTime1 = 0;
  NGMHit* lastTime2 = (NGMHit*) (_hitList->Last());
  while( (lastTime1 = (NGMHit*) (tHitIter.Next())) )
  {
    int plotIndex2 = partID->getPlotIndex(lastTime2);
    int plotIndex1 = partID->getPlotIndex(lastTime1);
    if(lastTime1 == lastTime2) continue;
	   
    double timeDiff = lastTime2->TimeDiffNanoSec(lastTime1->GetNGMTime());
    //we have created histograms so that plotIndex2 is always >= that plotIndex1
    if(plotIndex1>plotIndex2)
     {
       // Swap plotIndexes
       int tmp = plotIndex1;
       plotIndex1 = plotIndex2;
       plotIndex2 = tmp;
       timeDiff*=-1.0;
     }
    
    int serialIndex = (int)(plotIndex1*totalChannels - (plotIndex1+1)*plotIndex1/2.0+plotIndex2);
    // Plot Nearest neighbors in 2D Plots
    if((plotIndex2-plotIndex1) == _nearestNeighborSkip)
    {
      _timeComparisonShort->Fill(plotIndex1,timeDiff);
      _timeComparisonLong->Fill(plotIndex1,timeDiff);
    }
    
    // Test if skipping all channel by channel comparisons
    if(!_fillAllChannelByChannel ) continue;
    
    char hname[1024];
    TH1* tdh = (TH1*)(_timeDistributionPerChannel->At(serialIndex));
    if(! tdh )
    {
      TString dname1 = partID->getChannelName(plotIndex1);
      TString dname2 = partID->getChannelName(plotIndex2);
      sprintf(hname,"%s_%s_%s_%s",GetName(),"timeDistribution",
              dname1.Data(),dname2.Data());
      TThread::Lock();
      tdh = new TH1F(hname,hname,1,0,1);
      tdh->SetDirectory(0);
      tdh->SetXTitle("Time Difference [ns]");
      _timeDistributionPerChannel->AddAt(tdh,serialIndex);
      GetParentFolder()->Add(tdh);
      
      TThread::UnLock();
      
    }
    
    // Check if this is the first fill of this run
    if(tdh->GetNbinsX()==1)
    {
      double histtimerangeBase = _timerange_ns;

      // create histogram for this channel
      double histtimerange = histtimerangeBase;
      if(partID->getChannelName(plotIndex2).BeginsWith("HE") || partID->getChannelName(plotIndex1).BeginsWith("HE"))
      {
        histtimerange*=100;
      }else if(plotIndex1 == plotIndex2){
        histtimerange*=100.0;
      }
      
      sprintf(hname,"%s_%s_%s_%s",GetName(),"timeDistribution",
              partID->getChannelName(plotIndex1).Data(),partID->getChannelName(plotIndex2).Data());
      tdh->SetName(hname);
      tdh->SetTitle(hname);
      tdh->SetBins((int)(_timerange_ns*4*5),-histtimerange,histtimerange); // Units of nanoseconds
      //LOG<<"Creating histogram "<<hname<<" Nbinsx("<<tdh->GetNbinsX()<<")"<<ENDM_INFO;
      
    }
      
    if(tdh) tdh->Fill(timeDiff);
    
    if(_doTuple){
    _tTp.ch1 = plotIndex1;
    _tTp.ch2 = plotIndex2;
    _tTp.e1 = lastTime1->GetPulseHeight();
    _tTp.e2 = lastTime2->GetPulseHeight();
    _tTp.n1 = lastTime1->GetNeutronId();
    _tTp.n2 = lastTime2->GetNeutronId();
    _tTp.g1 = lastTime1->GetGammaId();
    _tTp.g2 = lastTime2->GetGammaId();
    _tTp.psd1 = lastTime1->GetPSD();
    _tTp.psd2 = lastTime2->GetPSD();
    _tTp.t = timeDiff;
      _timeTuple->Fill();
    }
  }

  return true;
}

void NGMTimeMonitor::displayChain(TObjArray* pList, Long64_t first, Long64_t length)
{
  // All of this is probably not thread safe...

  NGMHit* tPart;
	static TCanvas* cTimeDisplay = 0;
  //unused:	static TCanvas* cTimeDisplay1 = 0;
  static TGraph* tgTimeDisplay = 0;
  //unused:  static TGraph* tgTimeDisplay1 = 0;
  static TClonesArray* tmTimeDisplay = 0;
  static TClonesArray* tmTextDisplay = 0;
  //unused:  static TClonesArray* tmTimeDisplay1 = 0;
  //unused:  static TClonesArray* tmTextDisplay1 = 0;
  NGMTimeStamp firstTime;
  //unused:  double timescale = 1000.0;
  
  if(cTimeDisplay == 0){
    cTimeDisplay = new TCanvas("cTimeDisplay");
	//gSystem->ProcessEvents();
  }
  if(tgTimeDisplay == 0){
    tgTimeDisplay = new TGraph(pList->GetLast()+1);
    tgTimeDisplay->SetName("tgTimeDisplay");
    gROOT->Add(tgTimeDisplay);
    //tgTimeDisplay->SetMarkerStyle(0);      
  }
  if(tmTimeDisplay == 0){
    tmTimeDisplay = new TClonesArray("TMarker");
  }
  tmTimeDisplay->Clear();
  if(tmTextDisplay == 0){
    tmTextDisplay = new TClonesArray("NGMHitText");
  }
  tmTextDisplay->Clear();
  
  if(!pList) return;
  if(first < 0) return;
  if(length == 0) length = pList->GetLast() + 1 - first;
  if(length <= 0) return;
  if(length+first-1 > pList->GetLast()) return;
  
  tgTimeDisplay->Set(length);
  
  std::cout<<"Displaying "<< first << " - " << first + length -1 <<std::endl;
  for(int ipart = first; ipart < first+length; ipart++){
    NGMHit* tPart = (NGMHit*) (pList->At(ipart));
    if(ipart == first) firstTime =tPart->GetNGMTime();
    tgTimeDisplay->SetPoint(ipart,
                            (tPart->TimeDiffNanoSec(firstTime)),
                            ipart-first);
  }
  
  cTimeDisplay->cd();
  cTimeDisplay->Clear();
  tgTimeDisplay->Draw("AP");
  tgTimeDisplay->GetHistogram()->SetTitle("Chain Display");
  tgTimeDisplay->GetHistogram()->SetXTitle("Time Since First in Sequence [ns]");
  tgTimeDisplay->GetHistogram()->SetYTitle("Number in Sequence");
  
  for(int ipart = first; ipart < first+length; ipart++){
    tPart = (dynamic_cast<NGMHit*> (pList->At(ipart)));
    double relativeTime = tPart->TimeDiffNanoSec(firstTime);
    int channelDisplay = partID->getPlotIndex(tPart)+1;
    std::cout<<"Ch("<<channelDisplay<<") Time("<<relativeTime<<")\n";
    TString ts("");
    ts+=(channelDisplay);
    int markercolor = kBlack;
    int markerstyle = 21;
    TMarker* tMarker = (TMarker*) new((*tmTimeDisplay)[ipart-first]) TMarker(relativeTime,ipart-first,markerstyle);
    tMarker->SetMarkerColor(markercolor);
    tMarker->Draw();
    //    TText* ttext = (TText*) new((*tmTextDisplay)[ipart-first]) TText(tPart->time-firstTime,ipart,ts);
    NGMHitText* ttext = (NGMHitText*) new((*tmTextDisplay)[ipart-first]) NGMHitText(relativeTime,ipart-first,partID->GetName(tPart).Data(), tPart->GetEnergy());
    ttext->SetTextColor(markercolor);
    ttext->Draw();
  } // loop over particles
  
}

ClassImp(NGMHitText)

bool NGMHitText::alldrawenergy = false;

NGMHitText::NGMHitText(Double_t x, Double_t y, const char* channel, double energy)
: TText(x,y,"tmp                   ")
{
  _channel = channel;
  _energy = energy;
  char cbuff[1024];
  sprintf(cbuff,"%s",_channel.Data());
  SetTitle(cbuff);
}

void NGMHitText::ToggleThis()
{
  if(drawenergy) drawenergy = false;
  else drawenergy = true;
  
  char cbuff[1024];
  if(drawenergy)
    sprintf(cbuff,"%s, %0.3f",_channel.Data(),_energy);
  else sprintf(cbuff,"%s",_channel.Data());
  SetTitle(cbuff);
}

void NGMHitText::ToggleAll()
{
  if(alldrawenergy) alldrawenergy = false;
  else alldrawenergy = true;
}


void NGMTimeMonitor::ResetHistograms()
{
  if(!_hitList) _hitList = new TList;
  _hitList->Delete();
  _hitsAnalyzed = 0;
  if(_hCfdWordMonitor)
    _hCfdWordMonitor->Reset();
  if(_timeComparisonShort) _timeComparisonShort->Reset();
  if(_timeComparisonLong) _timeComparisonLong->Reset();

  if(_timeDistributionPerChannel)
  for(int ihist = 0; ihist <= _timeDistributionPerChannel->GetLast(); ihist++)
    if(_timeDistributionPerChannel->At(ihist))
    {
      TString hname("UNUSED");
      hname+=ihist;
      TH1* thd = ((TH1*)(_timeDistributionPerChannel->At(ihist)));
      thd->Reset();
      thd->SetName(hname.Data());
      thd->SetTitle(hname.Data());
      thd->SetBins(1,0,1);
    }
}

void NGMTimeMonitor::setMaxNanoSecInList(double newVal) {maxNanosecondsInList = newVal; } // *MENU*

ClassImp(NGMTimeDisplay)

NGMTimeDisplay::NGMTimeDisplay()
: NGMModule()
{
  numberOfHWChannels = 8*14;
  numberOfSlots = 8;
  timeDiffWaterFall = 0;
  nskipsforwaterfall = 0;
  char cbuf[1024];
  for(int ichan = 0; ichan < maxChannels; ichan++)
  {
    sprintf(cbuf,"CH%02d",ichan+1);
    _detNames[ichan] = cbuf;
    _detColors[maxChannels] = kBlack;
  }
  
  cTimeDisplay = 0;
	cTimeDisplay1 = 0;
  tgTimeDisplay = 0;
  tgTimeDisplay1 = 0;
  tmTimeDisplay = 0;
  tmTextDisplay = 0;
  tmTimeDisplay1 = 0;
  tmTextDisplay1 = 0;
  hAxisChainDisplay = 0;
  
}

NGMTimeDisplay::NGMTimeDisplay(const char* name, const char* title)
: NGMModule(name, title)
{
  numberOfHWChannels = 8*14;
  numberOfSlots = 8;
  timeDiffWaterFall = 0;
  nskipsforwaterfall = 0;
  char cbuf[1024];
  for(int ichan = 0; ichan < maxChannels; ichan++)
  {
    sprintf(cbuf,"CH%02d",ichan+1);
    _detNames[ichan] = cbuf;
    _detColors[maxChannels] = kBlack;
  }
  
  cTimeDisplay = 0;
	cTimeDisplay1 = 0;
  tgTimeDisplay = 0;
  tgTimeDisplay1 = 0;
  tmTimeDisplay = 0;
  tmTextDisplay = 0;
  tmTimeDisplay1 = 0;
  tmTextDisplay1 = 0;
  hAxisChainDisplay = 0;
}

NGMTimeDisplay::~NGMTimeDisplay()
{
}

void NGMTimeDisplay::setNskipsForWaterFall(int newVal) {if(nskipsforwaterfall < maxskips) nskipsforwaterfall = newVal; } // *MENU*

int NGMTimeDisplay::getPlotIndex(const NGMHit* tHit) const 
{
  
  int plotIndex = tHit->GetSlot()*8/*(numberOfHWChannels/numberOfSlots)*/ + tHit->GetChannel();

  return plotIndex;
}

bool NGMTimeDisplay::init(){
  TThread::Lock();
  _hitList = new TList;
  //timeDiffWaterFall = new TGraph(100);
  TThread::UnLock();
  return true;
}  

bool NGMTimeDisplay::finish(){
   return true;
}  


bool NGMTimeDisplay::process(const TObject& tData)
{
  static TClass* tTListType = TClass::GetClass("TSeqCollection");
  static TClass* tNGMSystemConfigurationType = TClass::GetClass("NGMSystemConfiguration");
  //Check data type
  if(tData.InheritsFrom(tTListType)){
    
    const TSeqCollection* hitList = (const TSeqCollection*)(&tData);
    displayChain(hitList,0,hitList->LastIndex()+1);
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
    _runBegin = confBuffer->GetTimeStamp();
    numberOfSlots = confBuffer->GetSlotParameters()->GetEntries();
    numberOfHWChannels = confBuffer->GetChannelParameters()->GetEntries();    
    //Lets pull out the Detector Names
    if(confBuffer->GetChannelParameters()->GetParIndex("DetectorName")>=0)
    {
      for(int ichan = 0; ichan < numberOfHWChannels; ichan++)
      {
        if(ichan>= maxChannels) break;
        _detNames[ichan] = confBuffer->GetChannelParameters()->GetParValueS("DetectorName",ichan);
        if(_detNames[ichan].BeginsWith("GL"))
          _detColors[ichan] = kBlue;
        else if(_detNames[ichan].BeginsWith("GS"))
          _detColors[ichan] = kCyan;
        else if(_detNames[ichan].BeginsWith("MU"))
          _detColors[ichan] = kRed;
        else if(_detNames[ichan].BeginsWith("LS"))
          _detColors[ichan] = kBlack;
        else if(_detNames[ichan].BeginsWith("HE"))
          _detColors[ichan] = kGreen;
      }
    }
    
  }

  push(tData);
    
  return true;  
}

void NGMTimeDisplay::displayChain(const TSeqCollection* pList, Long64_t first, Long64_t length)
{
  // All of this is probably not thread safe...
  
  NGMHit* tPart;
  NGMTimeStamp firstTime;
  //unused:  double timescale = 1000.0;
  
  TVirtualPad* prevPad = gPad;
  
  char cbuff[1024];
  sprintf(cbuff,"c%s_TimeDisplay",GetName());
  TCanvas* cTimeDisplay = (TCanvas*)(gROOT->FindObject(cbuff));

  if(cTimeDisplay == 0){
    TThread::Lock();
    cTimeDisplay = new TCanvas(cbuff,cbuff);
    cTimeDisplay->Divide(1,2);
    TThread::UnLock();
  }
  
  TVirtualPad* cChainPad = cTimeDisplay->cd(1);
  
  cChainPad->Clear();
  //cTimeDisplay->Clear();

  if(!hAxisChainDisplay)
	{
    sprintf(cbuff,"h%s_ChainDisplay",GetName());
    TThread::Lock();
		hAxisChainDisplay = new TH2F(cbuff,cbuff,1000,0.0,1.0,1000,0.0,1.0);
    hAxisChainDisplay->SetXTitle("Elapsed Time [ns]");
    hAxisChainDisplay->SetYTitle("Number in Sequence");
    TThread::UnLock();
	}  
  
  if(!hAxisWaterfallDisplay)
	{
    sprintf(cbuff,"h%s_WaterfallDisplay",GetName());
    TThread::Lock();
		hAxisWaterfallDisplay = new TH2F(cbuff,cbuff,1000,0.0,1.0,1000,0.0,1.0);
    hAxisWaterfallDisplay->SetXTitle("Elapsed Time [ns]");
    hAxisWaterfallDisplay->SetYTitle("Time to Next [ns]");
    TThread::UnLock();
	}  
  
  if(tgTimeDisplay == 0){
    sprintf(cbuff,"tg%s_TimeDisplay",GetName());
    tgTimeDisplay = new TGraph(pList->LastIndex()+1);
    tgTimeDisplay->SetName(cbuff);
    gROOT->Add(tgTimeDisplay);    
  }
  if(tmTimeDisplay == 0){
    tmTimeDisplay = new TClonesArray("TMarker");
  }
  tmTimeDisplay->Clear();

  if(tmTextDisplay == 0){
    tmTextDisplay = new TClonesArray("NGMHitText");
  }
  tmTextDisplay->Clear();

  if(tmTimeDisplay1 == 0){
    tmTimeDisplay1 = new TClonesArray("TMarker");
  }
  tmTimeDisplay1->Clear();
  
  if(!pList) return;
  if(first < 0) return;
  if(length == 0) length = pList->LastIndex() + 1 - first;
  if(length <= 0) return;
  if(length+first-1 > pList->LastIndex()) return;
  
  tgTimeDisplay->Set(length);
  
  std::cout<<"Displaying "<< first << " - " << first + length -1 <<std::endl;
  for(int ipart = first; ipart < first+length; ipart++){
    NGMHit* tPart = (NGMHit*) (pList->At(ipart));
    if(ipart == first) firstTime =tPart->GetNGMTime();
    tgTimeDisplay->SetPoint(ipart,
                            (tPart->TimeDiffNanoSec(firstTime)),
                            ipart-first);
  }
  
  //tgTimeDisplay->Draw("AP");
  hAxisChainDisplay->GetYaxis()->SetRangeUser(1E-1,1E7);
  hAxisChainDisplay->SetBins(100,tgTimeDisplay->GetX()[0],tgTimeDisplay->GetX()[length-1],
                          100,0,length);
  
  hAxisChainDisplay->SetTitle("Chain Display");
  hAxisChainDisplay->SetXTitle("Time Since First in Sequence [ns]");
  hAxisChainDisplay->SetYTitle("Number in Sequence");
  hAxisChainDisplay->Draw("AXIS");
  
  double minTimeInRun = 0.0;
  double maxTimeInRun = 0.0;
  
  for(int ipart = first; ipart < first+length; ipart++){
    tPart = (dynamic_cast<NGMHit*> (pList->At(ipart)));

    double relativeTime = tPart->TimeDiffNanoSec(firstTime);
    int plotIndex = getPlotIndex(tPart);
    int channelDisplay = getPlotIndex(tPart)+1;
    //std::cout<<"Ch("<<channelDisplay<<") Time("<<relativeTime<<")\n";
    TString ts("");
    ts+=(channelDisplay);
    int markercolor = _detColors[plotIndex];
    int markerstyle = 21;
    TMarker* tMarker = (TMarker*) new((*tmTimeDisplay)[ipart-first]) TMarker(relativeTime,ipart-first,markerstyle);
    tMarker->SetMarkerColor(markercolor);
    tMarker->SetMarkerSize(0.5);
    tMarker->Draw();
    
    if(false){
    //    TText* ttext = (TText*) new((*tmTextDisplay)[ipart-first]) TText(tPart->time-firstTime,ipart,ts);
    NGMHitText* ttext = (NGMHitText*) new((*tmTextDisplay)[ipart-first]) NGMHitText(relativeTime,ipart-first,_detNames[plotIndex], tPart->GetEnergy());
    ttext->SetTextSize(0.02);
    ttext->SetTextColor(markercolor);
    ttext->Draw();
    }
    
    if(ipart < first+length -1 )
    {
      NGMHit* partNext =  (dynamic_cast<NGMHit*> (pList->At(ipart+1)));
      double timeDiffNext = partNext->TimeDiffNanoSec(tPart->GetNGMTime());
      double timeInRun = tPart->TimeDiffNanoSec(_runBegin);
      if(ipart == first) minTimeInRun = timeInRun;
      if(ipart == first+length - 2) maxTimeInRun = timeInRun;
      TMarker* tMarker = (TMarker*) new((*tmTimeDisplay1)[ipart-first]) TMarker(timeInRun,timeDiffNext,markerstyle);
      tMarker->SetMarkerColor(markercolor);
      tMarker->SetMarkerSize(0.5);
    }
  } // loop over particles
  
  cTimeDisplay->Modified();
  cTimeDisplay->Update();

  TVirtualPad* cWaterFallPad = cTimeDisplay->cd(2);
  cWaterFallPad->SetLogy();
  cWaterFallPad->Clear();

  hAxisWaterfallDisplay->SetBins(1000,minTimeInRun,maxTimeInRun,100,1E-1,1E8);
  hAxisWaterfallDisplay->Draw("AXIS");
  for(int imarker = 0; imarker < length; imarker++)
  {
    TMarker* tMark = dynamic_cast<TMarker*>(tmTimeDisplay1->At(imarker));
    if(tMark) tMark->Draw();
  }
  
  cWaterFallPad->Modified();
  cWaterFallPad->Update();
  if(prevPad!=cWaterFallPad) prevPad->cd();
}

void NGMTimeMonitor::LaunchDisplayCanvas()
{
	TCanvas* cTimeDisplay = 0;
   TString cName;
   cName.Form("%s_%s",GetName(),"Canvas");

	cTimeDisplay = (TCanvas*) (GetParentFolder()->FindObject(cName));
	if(!cTimeDisplay)
	{
		LOG<<" Creating canvas "<<cName<<ENDM_INFO;
      TThread::Lock();
		cTimeDisplay = new TCanvas(cName,cName);
      TThread::Lock();
    GetParentFolder()->Add(cTimeDisplay);
      
	}
  cTimeDisplay->Clear();
  cTimeDisplay->Divide(1,2);

	TVirtualPad* tpad1 = cTimeDisplay->cd(1);
  tpad1->Modified();
  tpad1->Update();
  _timeComparisonShort->DrawCopy("colz");
	TVirtualPad* tpad2 = cTimeDisplay->cd(2);
  tpad2->Modified();
  tpad2->Update();
  _timeComparisonLong->DrawCopy("colz");
  tpad2->Modified();
  tpad2->Update();
}


TH1* NGMTimeMonitor::mergeTiming(TString detectorName,NGMSystemConfiguration* sysConf, TString histPrefix)
{

	double thisChanOffset = sysConf->GetDetectorParameters()->GetParValueD("IterTimingOffset",sysConf->GetDetectorParameters()->FindFirstRowMatching("DetectorName", detectorName.Data()));
    
	TCollection* histNameList = GetParentFolder()->GetListOfFolders();

	TH1* histSum = 0;
    TObject* tmp = 0;
    int thisChanIndex = sysConf->GetChannelParameters()->FindFirstRowMatching("DetectorName", detectorName.Data());
    // Lets build a pairwise list by iterating through the detector table
    for(int idet = 0; idet<sysConf->GetDetectorParameters()->GetEntries(); idet++){
        TString otherDetectorName =sysConf->GetDetectorParameters()->GetParValueS("DetectorName", idet);
        if (otherDetectorName == detectorName || otherDetectorName.Contains("HE")) continue;
        int otherChanIndex = sysConf->GetChannelParameters()->FindFirstRowMatching("DetectorName", otherDetectorName.Data());
        TString hTimingHistName=Form("%s_timeDistribution_%s_%s",
                                     GetName(),
                                     thisChanIndex<otherChanIndex?detectorName.Data():otherDetectorName.Data(),
                                     thisChanIndex<otherChanIndex?otherDetectorName.Data():detectorName.Data()
                                     );
        
        TH1* tHist = dynamic_cast<TH1*>( GetParentFolder()->FindObject(hTimingHistName.Data()));
        
        if(!tHist) continue;

        // Check and see if this is a histogram of interest
        TString hName = tHist->GetName();
        TString selfHistName = detectorName;
        selfHistName += "_";
        selfHistName += detectorName;
        TString combinedHist = histPrefix+detectorName;
        
//        double otherChanOffset = TString(sysConf->GetDetectorParameters()->LookupParValueAsString("IterTimingOffset",
//                                                                                                  "DetectorName",otherDetectorName.Data())).Atof();
        
        double otherChanOffset = sysConf->GetDetectorParameters()->GetParValueD("IterTimingOffset",sysConf->GetDetectorParameters()->FindFirstRowMatching("DetectorName", otherDetectorName.Data()));

        // Lets do something with this histogram
        TString newHName = "calTiming_";
        newHName+=detectorName;
        // Use the same histogram for subsequent iterations
        if(!histSum)
        {
            histSum = (TH1*)(GetParentFolder()->FindObject(newHName.Data()));
            if(histSum) histSum->Reset();
        }
        if(!histSum)
        {
            histSum = (TH1*)(tHist->Clone(newHName.Data()));
            histSum->SetDirectory(0);
            histSum->SetTitle(newHName.Data());
            GetParentFolder()->Add(histSum);
            histSum->Reset();
        }
        
        // We always keep detector minus otherDetectors convention
        double histSign = -1.0;
        if(thisChanIndex>otherChanIndex)
        { histSign = 1.0; }
        //std::cout<<" Adding "<<tHist->GetName()<<" "<<histSign<<std::endl;
        //std::cout<<"Applying offsets "<<detectorName.Data()<<"("<<thisChanOffset<<") "<<otherDetectorName.Data()<<"("<<histSign*otherChanOffset<<")"<<std::endl;
        for(int ibin = 1; ibin <= tHist->GetNbinsX(); ibin++)
        {
            histSum->Fill((tHist->GetXaxis()->GetBinCenter(ibin)*histSign - thisChanOffset + otherChanOffset), tHist->GetBinContent(ibin));
        }
    }
	return histSum;
}



NGMSystemConfiguration* NGMTimeMonitor::calTiming( double fitRange, NGMSystemConfiguration* sysConf )
{

  if(!sysConf) sysConf = NGMSystem::getSystem()->GetConfiguration();

	if(!sysConf) return 0;

	NGMConfigurationTable* cChan = sysConf->GetChannelParameters();
	NGMConfigurationTable* cDet = sysConf->GetDetectorParameters();
	
	if(cDet->GetParIndex("CalTimingOffset")<0)
	{
	    std::cout<< "Add Parameter CalTimingOffset" <<std::endl;
		cDet->AddParameterD("CalTimingOffset",0.0,-1E6,1E6);
		cDet->SetParameterToDefault("CalTimingOffset");
	}
	if(cDet->GetParIndex("CalcTimingOffset")<0)
	{
	    std::cout<< "Add Parameter CalcTimingOffset" <<std::endl;
		cDet->AddParameterD("CalcTimingOffset",0.0,-1E6,1E6);
		cDet->SetParameterToDefault("CalcTimingOffset");
	}
	if(cDet->GetParIndex("IterTimingOffset")<0)
	{
	    std::cout<< "Add Parameter IterTimingOffset" <<std::endl;
		cDet->AddParameterD("IterTimingOffset",0.0,-1E6,1E6);
		cDet->SetParameterToDefault("IterTimingOffset");
	}
	
	
	std::cout<<"Total Channels: "<<cChan->GetEntries()<<std::endl;
	
	TH1* hTimingOffset = (TH1*)(GetParentFolder()->FindObjectAny("hTimingOffset"));
	if(!hTimingOffset)
	{
		hTimingOffset = new TH1F("hTimingOffset","hTimingOffset",
					cChan->GetEntries(), 0, cChan->GetEntries());
    hTimingOffset->SetDirectory(0);
    GetParentFolder()->Add(hTimingOffset);
	}
	TH1* hTimingOffsetIter = (TH1*)(gROOT->FindObjectAny("hTimingOffsetIter"));
	if(!hTimingOffsetIter)
	{
		hTimingOffsetIter = new TH1F("hTimingOffsetIter","hTimingOffsetIter",
					cChan->GetEntries(), 0, cChan->GetEntries());
    hTimingOffsetIter->SetDirectory(0);
    GetParentFolder()->Add(hTimingOffsetIter);

	}
	cDet->SetQuiet(true);
  TString histPrefix;
  histPrefix.Form("%s_timeDistribution_",GetName());

	for(int idet = 0; idet < cDet->GetEntries(); idet++)
	{
        TString detName = cDet->GetParValueS("DetectorName", idet);
        int idetRow = cDet->FindFirstRowMatching("DetectorName", detName.Data());
        if(idetRow<0) continue;
        int ichan = cChan->FindFirstRowMatching("DetectorName", detName.Data());
        if(ichan<0) continue;
        
		if(detName.Contains("HE")) continue;
		std::cout<<"Analyzing "<<detName.Data()<<std::endl;
		TH1* tHist =  mergeTiming(detName.Data(),sysConf,histPrefix);
		if(!tHist) continue;
		tHist->GetXaxis()->SetRangeUser(-40,40);
		int maximumTiming = (int)tHist->GetXaxis()->GetBinCenter( tHist->GetMaximumBin() );

		TF1* mygaus = new TF1("mygaus","gaus(0)",maximumTiming-fitRange,maximumTiming+fitRange);
        mygaus->SetParLimits(2, 0.0, 20.0);
		mygaus->SetParNames("Constant","Mean","Sigma");
		//mygaus->SetParameters(tHist->GetMaximum(),tHist->GetXaxis()->GetBinCenter( tHist->GetMaximumBin() ),fitRange);
		mygaus->SetParameters(tHist->GetMaximum(),tHist->GetMean(),tHist->GetRMS());
		//tHist->GetXaxis()->SetRangeUser(maximumTiming-fitRange,maximumTiming+fitRange);
		
        mygaus->Print();
		tHist->Fit(mygaus,"LR");
	    //tHist->Fit(mygaus,"LRQ");
		// Test if there are enough statistics
		if(mygaus->GetParameter(0) < 1E1)
		{	
			LOG<<" Not enough statistics for channel "<< detName.Data()<<ENDM_INFO;
			//cDet->SetParameterAsStringMatching("CalcTimingOffset",TString("")+=(0.0),"DetectorName",detName.Data());
			continue;
		}
		// Check for nonsensible result
		if(fabs(mygaus->GetParameter(1))>1E2)
		{
			LOG<<" Likely fit failure "<< detName.Data()<<" Fitted Offset: "<<mygaus->GetParameter(1)<<ENDM_INFO;
			//cDet->SetParameterAsStringMatching("CalcTimingOffset",TString("")+=(0.0),"DetectorName",detName.Data());
			continue;		
		}
		hTimingOffset->SetBinContent(ichan+1, mygaus->GetParameter(1));
		std::cout<<detName.Data()<<" "<<mygaus->GetParameter(1)<<std::endl;
        cDet->SetParameterD("CalcTimingOffset",idetRow,mygaus->GetParameter(1));
		delete mygaus;
	}
	cDet->SetQuiet(false);
	
	// Update the new iterative offests based on the calculated offsets
    for(int idet = 0; idet < cDet->GetEntries(); idet++)
	{
		TString detName = cDet->GetParValueS("DetectorName",idet);
        int chanRow = cChan->FindFirstRowMatching("DetectorName", detName.Data());
        if(chanRow<0) continue;
		if(detName.Contains("HE")) continue;
        double observedOffset = cDet->GetParValueD("CalcTimingOffset",idet);
		double iterTimingOffset = cDet->GetParValueD("IterTimingOffset",idet);
		cDet->SetQuiet(true);
		hTimingOffsetIter->SetBinContent(chanRow+1,observedOffset);
        std::cout<<detName.Data()<<" Iter("<<iterTimingOffset<<") Observed("<<observedOffset<<")"<<std::endl;
        cDet->SetParameterD("IterTimingOffset",idet,iterTimingOffset+0.5*observedOffset);
		cDet->SetQuiet(false);
	}
	
	hTimingOffset->Draw();
    gPad->Update();
	return sysConf;
}

void NGMTimeMonitor::initializeTimePlots(){
  
  int totalChannels = partID->getNumberOfChannels();
  
  
  if(!_timeDistributionPerChannel)
    _timeDistributionPerChannel = new TObjArray;

  _timeDistributionPerChannel->Expand((Int_t)((totalChannels+1)*(totalChannels/2.0)));
  // Rather adding at the beginning lets only allocate histogram memory 
  // if channel pair are present within the data
  return;
  for(int plotIndex1 = 0; plotIndex1 < totalChannels; plotIndex1++){ // Row
    TString dname1 = partID->getChannelName(plotIndex1).Data();
    if(dname1.BeginsWith("EMPTY")) continue;
    
    for(int plotIndex2 = plotIndex1; plotIndex2 < totalChannels; plotIndex2++){ //Column
      
      int serialIndex = (int)(plotIndex1*totalChannels - (plotIndex1+1)*plotIndex1/2.0+plotIndex2);
      TString dname2 = partID->getChannelName(plotIndex2).Data();
      if(dname2.BeginsWith("EMPTY")) continue;
      
      char hname[1024];
      TH1* tdh = (TH1*)(_timeDistributionPerChannel->At(serialIndex));
      if(! tdh )
      {
        sprintf(hname,"%s_%s_%s_%s",GetName(),"timeDistribution",
                dname1.Data(),dname2.Data());
        TThread::Lock();
        tdh = new TH1F(hname,hname,1,0,1);
        tdh->SetDirectory(0);
        tdh->SetXTitle("Time Difference [ns]");
        _timeDistributionPerChannel->AddAt(tdh,serialIndex);
        GetParentFolder()->Add(tdh);
        
        TThread::UnLock();
      }
      
      // create histogram for this channel
      double histtimerange = _timerange_ns/2.0;
      if(partID->getChannelName(plotIndex2).BeginsWith("HE") || partID->getChannelName(plotIndex1).BeginsWith("HE"))
      {
        histtimerange*=100;
      }else if(plotIndex1 == plotIndex2){
        histtimerange*=100.0;
      }
      
      sprintf(hname,"%s_%s_%s_%s",GetName(),"timeDistribution",
              partID->getChannelName(plotIndex1).Data(),partID->getChannelName(plotIndex2).Data());
      //LOG<<"Creating histogram "<<hname<<ENDM_INFO;
      tdh->SetName(hname);
      tdh->SetTitle(hname);
      tdh->SetBins((int)(_timerange_ns*4.0),-histtimerange,histtimerange); // Units of nanoseconds
    }
  }
}

void NGMTimeMonitor::Print(Option_t* option) const
{

//   TCanvas* nMonDisplay = 0;
//	char cName[1024];
//	sprintf(cName,"%s_Canvas",GetName());
//	nMonDisplay = (TCanvas*) (GetParentFolder()->FindObjectAny(cName));
//   TString printName;
//   printName+=_runNumber;
//   printName+="_";
//   printName+=cName;
//   printName+=".eps";
//   if(nMonDisplay) nMonDisplay->Print(printName.Data());
}
