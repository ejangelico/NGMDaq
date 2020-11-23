/*
 *  NGMWaterfall.cpp
 *  NGMDaq
 *
 *  Created by Jerome Verbeke on 06/07/07.
 *  Copyright 2007 LLNL. All rights reserved.
 *
 */

#include <math.h>
#include "TH2.h"
#include "NGMLogger.h"
#include "NGMBufferedPacket.h"
#include "NGMSystemConfiguration.h"
#include "NGMWaterfall.h"
#include "NGMConfigurationTable.h"
#include "TVirtualPad.h"
#include "TROOT.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "NGMMarker.h"
#include "TObjString.h"
#include "NGMParticleIdent.h"
#include "NGMSimpleParticleIdent.h"
#include "NGMTimeMonitor.h"
#include "NGMZoomGui.h"
#include "TMath.h"
#include "TThread.h"
#include "TLegend.h"
#include "TGScrollBar.h"
#include "TF1.h"
#include "TVirtualPad.h"
#include "TGraphErrors.h"
#include "TLatex.h"
#include "TFolder.h"

ClassImp(NGMWaterfall);

double* NGMWaterfall::genLogBins(int nbins, double axismin, double axismax)
{
  double* dbins = new double[nbins+1];
  double factor = getFactor(nbins, axismin, axismax);
  dbins[0] = axismin;
  for(int ibin = 1; ibin <= nbins; ibin++)
  {
    dbins[ibin] = axismin*pow(factor,ibin); 
  }
  return dbins; 
}

NGMWaterfall::NGMWaterfall()
{
  _hitsAnalyzed = 0;
  timeDiffWaterfall = 0;
  dispInt = 10; // 10 secs
  maxListSize = 5000;
  pList = 0;
  wmList = 0;
  wtList = 0;
  smList = 0;
  stList= 0;
  _drawTextFlag = false;
  lastDraw = 0;
  firstTime = 0; // first time in the data stream
  myZoomer = 0;
  h1 = 0;
  leg = 0;
  _runNumber = 0;
  _ronPlot = 0;
  _residualPlot = 0;
  xminbin = 1E-3;
  xmaxbin = 1E9;
  totalbins = 50;
  alarmEnabled = false;
  alarm_threshold = 1e-4;
  alarm_threshold = -0.30;
  alarm_threshold = -10;
  alarm = false;
  min_bin_cutoff = .8E3; // 0.8 microsecond
  max_bin_cutoff = 100E3; // 100 microsecond
  Nskips = 0;
  burstFilterSetting = false;
  maxTime = 1e20;

  partID = new NGMSimpleParticleIdent;
}

NGMWaterfall::NGMWaterfall(const char* name, const char* title)
: NGMModule(name,title)
{
  _hitsAnalyzed = 0;
  timeDiffWaterfall = 0;
  dispInt = 10; // 10 secs
  maxListSize = 5000;
  pList = 0;
  wmList = 0;
  wtList = 0;
  smList = 0;
  stList= 0;  
  _drawTextFlag = false;
  lastDraw = 0;
  firstTime = 0; // first time in the data stream
  myZoomer = 0;
  h1 = 0;
  leg = 0;
  _runNumber = 0;
  _ronPlot = 0;
  _residualPlot = 0;
  xminbin = 1E-1;
  xmaxbin = 1E9;
  totalbins = 40;
  alarmEnabled = false;
  alarm_threshold = 1E-4;
  alarm_threshold = -0.30;
  alarm_threshold = -10;
  alarm = false;
  min_bin_cutoff = .8E3; // 0.8 microsecond
  max_bin_cutoff = 100E3; // 100 microsecond
  Nskips = 0;
  burstFilterSetting = false;
  maxTime = 1e20;

  partID = new NGMSimpleParticleIdent();
}

NGMWaterfall::~NGMWaterfall(){
  delete partID;
  if(pList)
   pList->Delete();
  delete pList;

}

bool NGMWaterfall::init(){
  return true;
}

bool NGMWaterfall::process(const TObject &tData){
  
  static TClass* tNGMHitType = gROOT->GetClass("NGMHit");
  static TClass* tObjStringType = gROOT->GetClass("TObjString");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  static TClass* tNGMBufferedPacketType = gROOT->GetClass("NGMBufferedPacket");

  //Check data type
  if(tData.InheritsFrom(tNGMBufferedPacketType)){
     // We assume this packet is probably a burst slotid == -1
     const NGMBufferedPacket* packetBuffer = (const NGMBufferedPacket*)(&tData);
     if(packetBuffer->getSlotId()!=-1) {
        LOG<<" Not expecting a raw packet.  Should be a burst packet "<<ENDM_WARN;
        return false;
     }
    if(firstTime == 0 && packetBuffer->getPulseCount()>0) {
      firstTime = new NGMTimeStamp(((NGMHit *)(packetBuffer->getHit(0)))->GetNGMTime());
    }
     for(int ipart = 0; ipart < packetBuffer->getPulseCount(); ipart++)
     {
        const NGMHit* tHit = packetBuffer->getHit(ipart);
        if(partID != 0 && partID->IsSelected(tHit)) analyzeHit(tHit);
     }
//     doDraw();
  }else if(tData.InheritsFrom(tNGMHitType)){
//    LOG<<"Inherits from NGMHitv"<<ENDM_INFO;
    const NGMHit* tHit = (const NGMHit*)(&tData);
    if(firstTime == 0) firstTime = new NGMTimeStamp(tHit->GetNGMTime());
    if(partID != 0 && partID->IsSelected(tHit)) analyzeHit(tHit);
  } else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    LOG<<"Inherits from NGMSystemConfiguration"<<ENDM_INFO;
    const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
    partID->Init(confBuffer);
    _runNumber = confBuffer->getRunNumber();
    _runBegin = NGMTimeStamp(confBuffer->GetTimeStamp());
    delete firstTime;
    firstTime = 0;
    firstTime = new NGMTimeStamp(_runBegin);
    _hitsAnalyzed = 0;
    if(pList)
      pList->Delete();
    ResetHistograms();
  } else {
    if(tData.IsA() == tObjStringType){
      const TObjString* controlMessage = (const TObjString*)(&tData);
      if(controlMessage->GetString() == "EndRunSave") {
      }else if(controlMessage->GetString() == "EndRunFlush") {
        LaunchDisplayCanvas();
      }else if(controlMessage->GetString() == "EndSpillFlush") {
      }else if(controlMessage->GetString() == "PlotUpdate") {
		  LaunchDisplayCanvas();
      }
    }
  }
  push(tData);

  return true;  
}


bool NGMWaterfall::finish(){

  return true;
}

bool NGMWaterfall::analyzeHit(const NGMHit* thisHit){

  // Return if we have exceeded the maximum time
  if (thisHit->TimeDiffNanoSec(*firstTime) > maxTime) return true;
  
  if (lastDraw == 0) lastDraw = new NGMTimeStamp(thisHit->GetNGMTime());

  NGMHit* tHit = thisHit->DuplicateHit();
  _hitsAnalyzed++;

  if (pList == 0) pList = new TList();

  // Adding points to the list until we get maxListSize points
  if (pList->GetSize() >= maxListSize) {
    // When list is full, remove the first element and add to the end
     TObject* hitToRemove = pList->First();
     pList->RemoveFirst();
     delete hitToRemove;
  }

  if (pList->IsEmpty()) {
    pList->Add(tHit);
  } else {
    double deltat = tHit->TimeDiffNanoSec(((NGMHit*) pList->Last())->GetNGMTime());
    if (deltat > min_bin_cutoff || !burstFilterSetting) { // we skip all events with time differences > min_bin_cutoff
      pList->Add(tHit);
    } else return true;
  }
  
  if(!_ronPlot){
    char hname[1024];
    sprintf(hname,"%s_%s",GetName(),"RonPlot");
    _ronPlot = new TH1F(hname,hname,totalbins,genLogBins(totalbins,xminbin,xmaxbin)) ;
    _ronPlot->SetTitle(hname);
    _ronPlot->SetXTitle("Time between pulses [ns]");
    _ronPlot->Reset();
    _ronPlot->SetDirectory(0);
    GetParentFolder()->Add(_ronPlot);
  }

  if(!_residualPlot){
      char hname[1024];
      sprintf(hname,"%s_%s",GetName(),"ResidualPlot");
      _residualPlot = new TH1F(hname,hname,totalbins,genLogBins(totalbins,xminbin,xmaxbin)) ;
      _residualPlot->SetTitle(hname);
      _residualPlot->SetXTitle("Time between pulses [ns]");
      if (sigmas) {
        _residualPlot->SetYTitle("Residual scaled to std. dev.");
      } else {
        _residualPlot->SetYTitle("Residual");
      }
      _residualPlot->Reset();
      _residualPlot->SetDirectory(0);
      _residualPlot->SetStats(false);
      GetParentFolder()->Add(_residualPlot);

  }

  long long length = pList->GetSize();
  if(firstTime && (Nskips+3 <= length))
  {
    TIterator* tIter = pList->MakeIterator(kIterBackward);
    NGMHit* priorHit;
    for (int i=0; i<Nskips+1; i++) tIter->Next();
    priorHit = (NGMHit*) tIter->Next();
    double tDiff = tHit->TimeDiffNanoSec(priorHit->GetNGMTime());
    _ronPlot->Fill(tDiff);
  }
  
  // Displaying the waterfall every dispInt seconds
  if(fabs(tHit->TimeDiffNanoSec(*lastDraw)) > 1e9*dispInt){
    delete lastDraw;
    lastDraw = new NGMTimeStamp(tHit->GetNGMTime());
    doDraw();
  }

  return true;
}

void NGMWaterfall::SkipsChanged(Int_t newNskips){
  if (newNskips == Nskips) return;
  doDraw();
  Nskips = newNskips;
}

void NGMWaterfall::doDraw() {
  if (leg == 0) leg = partID->GetLegend(0.9, 1, 1,0.3);

  //if (h1) delete h1;

  TString cName;
  cName.Form("%s_Canvas",GetName());
  
  // This is important since the user could have deleted the canvas since the last call
  TCanvas* cWaterfall = (TCanvas*)(GetParentFolder()->FindObject(cName.Data()));
  
  if(!cWaterfall)
  {
    if(_useZoomGui && !gROOT->IsBatch() && !myZoomer){
      myZoomer = new NGMZoomGui;
      TGScrollBar* skipScroll = myZoomer->AddScrollBar("Skip");
      skipScroll->SetRange(110,10);
      skipScroll->SetPosition(0);
      skipScroll->Connect("PositionChanged(Int_t)","NGMWaterfall",this,"SkipsChanged(Int_t)");
      cWaterfall = myZoomer->getCanvas();
      cWaterfall->SetName(cName.Data());
    } else {
      cWaterfall = new TCanvas(cName.Data(), cName.Data(),900,1000);
    }
    cWaterfall->Divide(3,1);
    TVirtualPad* tpad = cWaterfall->cd(1);
    tpad->SetPad(0.0,0.66,1.0,1.0);
    tpad = cWaterfall->cd(2);
    tpad->SetPad(0.0,0.33,1.0,0.66);
    tpad = cWaterfall->cd(3);
    tpad->SetPad(0.0,0.0,1.0,0.33);
    GetParentFolder()->Add(cWaterfall);
  }

  NGMTimeStamp* previousTimes;
  previousTimes = (NGMTimeStamp*) malloc((Nskips+1)*sizeof(NGMTimeStamp));
  double timeSinceFirst = 0.0, timeDifference = 0.0;

  if(!pList) return;
  long long length = pList->GetSize();
  if(Nskips+3 > length) return;
  
  NGMHit* tHit;

// Special treatment for first hit
  if (firstTime == 0) {
    tHit = (NGMHit*) pList->First();
    firstTime = new NGMTimeStamp();
    *firstTime = tHit->GetNGMTime();
  }

  TVirtualPad* wPad = cWaterfall->cd(1);

  if(!wmList){
    wmList = new TClonesArray("NGMMarker",maxListSize);
    wmList->ExpandCreate(maxListSize);
  }
  if(wmList->GetEntriesFast()<pList->GetEntries())
  {
    LOG<<"Expanding Waterfall Marker List "<<GetName()<<ENDM_INFO;
     wmList->ExpandCreate(pList->GetEntries());
  }

  wPad->SetLogy();
  wPad->Clear();
  // This is terrible inefficient, we should create a Marker in sync with the pList
  // as the pList is populated.  The exception to this optimization is if we are 
  // going to dynamically recalculate varying skip settings.
  //wmList->Delete();

  double x_min = ((NGMHit*) pList->At(Nskips))->TimeDiffNanoSec(*firstTime);
  double x_max = ((NGMHit*) pList->Last())->TimeDiffNanoSec(*firstTime);
  double y_min = 1e200, y_max = -1e200;

  TString hAxisName;
  hAxisName.Form("%s_axisHist",GetName());
  
  h1 = (TH1F*)(GetParentFolder()->FindObject(hAxisName.Data()));
  if(!h1)
  {
     h1 = new TH1F(hAxisName.Data(),"",1000000,x_min, x_max);
     h1->SetDirectory(0);
    GetParentFolder()->Add(h1);
  }
  h1->SetBins(1000000,x_min, x_max);
  h1->SetBins(1000000,x_min,x_max);

  h1->SetStats(false);
  h1->SetXTitle("Time Since First in Sequence [ns]");
  h1->SetYTitle("Time since previous count [ns]");
  h1->GetYaxis()->SetRangeUser(0.01, 1E7);
  char namestr[128];
  TString runStr;
  runStr+=_runNumber;
  sprintf(namestr,"Run %s Waterfall Nskips=%d",runStr.Data(),Nskips);
  h1->SetTitle(namestr);
  h1->Draw();
  leg->Draw();

// Loop over 2nd through N points to draw points
  TIterator* tHitIter = pList->MakeIterator();
  int nprevioustimes=0;
  while (nprevioustimes <= Nskips) {
    tHit = (NGMHit*) (tHitIter->Next());
    if (tHit == NULL) return; // not enough elements
    previousTimes[nprevioustimes] = tHit->GetNGMTime();
    nprevioustimes++;
  }
  
  int markerIndex = 0;
  while( (tHit = (NGMHit*)(tHitIter->Next())) ) {
     timeSinceFirst = tHit->TimeDiffNanoSec(*firstTime);
     timeDifference = tHit->TimeDiffNanoSec(previousTimes[0]);
     if(timeDifference < 0.1) timeDifference = 0.1;

    // Set a marker
    NGMMarker* tMarker = (NGMMarker*)(wmList->At(markerIndex));
    tMarker->SetX(timeSinceFirst);
    tMarker->SetY(timeDifference);
    tMarker->SetMarkerStyle(partID->GetStyle(tHit));
    tMarker->SetMarkerColor(partID->GetColor(tHit));
    tMarker->SetMarkerSize(0.5);
	  tMarker->SetLabel(partID->GetName(tHit));
    tMarker->Draw();
    markerIndex++;
    if(_drawTextFlag){
      // Set the marker name
      NGMHitText* ttext = new NGMHitText(timeSinceFirst, timeDifference, partID->GetName(tHit), tHit->GetEnergy()); 
      ttext->SetTextSize(0.02);
      ttext->SetTextColor(partID->GetColor(tHit));
      ttext->Draw();
    }
    if (timeDifference < y_min && timeDifference > 0.) y_min = timeDifference;
    if (timeDifference > y_max) y_max = timeDifference;
    for(int i = 0; i < Nskips; i++) previousTimes[i] = previousTimes[i+1];
    previousTimes[Nskips] = tHit->GetNGMTime();
  }
  free(previousTimes);
  
  //h1->SetBins(1000000,x_min,x_max);
  h1->GetYaxis()->SetRangeUser(y_min, y_max);
  if(_ronPlot)
  {
    h1->GetYaxis()->SetRangeUser(_ronPlot->GetXaxis()->GetXmin(),
                                 _ronPlot->GetXaxis()->GetXmax());
  }
  if(myZoomer) myZoomer->setAxis(h1->GetXaxis());

  
  if(0){
  // Now do the stair step
  TVirtualPad* sPad = cWaterfall->cd(2);
  
  sPad->SetLogy(false);
  sPad->Clear();
  
  TString sAxisName;
  sAxisName.Form("%s_StairAxisHist",GetName());

  TH1* hsAxis = (TH1*)(gROOT->FindObjectAny(sAxisName.Data()));
  if(!hsAxis)
  {
     hsAxis = new TH1F(sAxisName.Data(),"",1000000,x_min, x_max);
     h1->SetXTitle("Time Since First in Sequence [ns]");
     h1->SetYTitle("Count Sequence");
     hsAxis->SetDirectory(gROOT);
  }
  
  //
  hsAxis->SetStats(false);
  hsAxis->SetBins(1000000,x_min,x_max);
  hsAxis->Draw();
  
  // Loop over 2nd through N points to draw points
  for(int ipart = +1; ipart < length; ipart++) {
     tHit = (NGMHit*) pList->At(ipart);
     timeSinceFirst = tHit->TimeDiffNanoSec(*firstTime);
     
     // Set a marker
     NGMMarker* tMarker = new NGMMarker;
     tMarker->SetY(ipart);
     tMarker->SetX(timeSinceFirst);
     tMarker->SetMarkerStyle(partID->GetStyle(tHit));
     tMarker->SetMarkerColor(partID->GetColor(tHit));
     tMarker->SetLabel(partID->GetName(tHit));
     tMarker->Draw();
     
     if(_drawTextFlag){
        // Set the marker name
        NGMHitText* ttext = new NGMHitText(ipart, timeSinceFirst, partID->GetName(tHit), tHit->GetEnergy()); 
        ttext->SetTextSize(0.02);
        ttext->SetTextColor(partID->GetColor(tHit));
        cWaterfall->cd();
        ttext->Draw();
     }
  }
  hsAxis->GetYaxis()->SetRangeUser(0.0, length);
  if (myZoomer) myZoomer->setAxis(h1->GetXaxis());
  sPad->Update();
  }// if

  // Ron Plot
  if(_ronPlot) {
    TVirtualPad* sPad = cWaterfall->cd(2);
    sPad->SetLogx(true);
    sPad->Clear();
    sPad->SetLogy(true);
    _ronPlot->Draw("HIST E1");
    if (alarmEnabled) fitPoisson(timeSinceFirst);
  }
  cWaterfall->Update();
}

void NGMWaterfall::SetNumberOfPointsDisplayed(int size)
{
  maxListSize = size;
}

void NGMWaterfall::SetDisplayInterval(double delay)
{
  dispInt = delay;
}

void NGMWaterfall::ResetHistograms()
{
  _hitsAnalyzed = 0;
  if(_ronPlot) _ronPlot->Reset();
  if(_residualPlot) _residualPlot->Reset();
}

void NGMWaterfall::fitPoisson(double elapsed_time) {
  TF1 *Poisson_fit;

  Poisson_fit = new TF1("Poisson_fit", "[0]*pow([1]*x,[2]+1)*exp(-1.*[1]*x)", xminbin, xmaxbin); 
//  Poisson_fit = new TF1("Poisson_fit", "[0]*pow([1]*x,[2]+1)*exp(-1.*[1]*x)+[3]*pow([1]*x,[5]+1)*exp(-1.*[4]*x)", xminbin, xmaxbin); 

  Poisson_fit->SetParNames("amplitude", "rate", "Nskips");
  //unused:  int nsteps = 0;
  //unused:  int maxsteps = 100;
  double xmin=xminbin;
  double xmax=xmaxbin;
  double rate=_hitsAnalyzed/elapsed_time; // same as countrate
  //unused:  double newrate;
  //unused:  double ratio = 1.;
  //unused:  double convergence = 0.001;
  Poisson_fit->SetLineColor(kRed);
  Poisson_fit->FixParameter(2, Nskips);

  Poisson_fit->SetParameter(1, rate);
  double ncounts = _ronPlot->Integral(_ronPlot->FindBin(xmin), _ronPlot->FindBin(xmax));
  Poisson_fit->SetParameter(0, ncounts/20);

/*
  Poisson_fit->FixParameter(5, Nskips);
  Poisson_fit->SetParameter(4, 10*rate);
  Poisson_fit->SetParameter(3, pow(1./ncounts,Nskips+1));
*/

/*
   Method 0: 
   First, we compute the countrate of the Poisson over the entire range
   Then, we check where this Poisson would have had half a count, 
   left of the point on the x axis where x=1/countrate. We use this new 
   point x as the minimum in the range. We refit the data points with 
   the Poisson to determine the new count rate. We iterate until 
   convergence or too many steps
  while (nsteps < maxsteps && fabs(ratio) > convergence) {
    _ronPlot->Fit("Poisson_fit", "R", "SAME", xmin, xmax);
    newrate = Poisson_fit->GetParameter("rate");
    ratio = (newrate-rate)/rate;
    rate = newrate;
// Figure out a way to compute the root of the Poisson_fit formula over 
// the range ]xmin: 1/rate[
    double x = 1./newrate;
    double y = Poisson_fit->Eval(x);
    while (y>.5) {
      x = x/1.2;
      y = Poisson_fit->Eval(x);
    };
    xmin = x;
cerr << nsteps
     << "ncounts = " << ncounts 
     << " amplitude = " << amplitude 
     << " xmin = " << xmin 
     << endl;
    nsteps++;
  }
  if (nsteps == maxsteps && fabs(ratio) > convergence) {
    LOG << "The Poisson fit did not converge in " << maxsteps << " steps." <<  ENDM_WARN;
  }
   end of Method 0: 
*/

/*
   Method 4
   We compute the Poisson fit between 100 us and xmax
*/
  _ronPlot->Fit("Poisson_fit", "R", "SAME", 100e3, xmax);
  rate = Poisson_fit->GetParameter("rate");
/*
   end of Method 4
*/

  TGraphErrors* poissonGraph = new TGraphErrors;
  poissonGraph->SetMarkerStyle(kFullDotSmall);
  poissonGraph->SetMarkerColor(kRed);
  poissonGraph->SetLineColor(kRed);
  ncounts = _ronPlot->Integral(_ronPlot->FindBin(xmin), _ronPlot->FindBin(xmax));
  for (int bin=1; bin<=totalbins; bin++) {
    double x = _ronPlot->GetBinCenter(bin);
    double y = Poisson_fit->Eval(x);
    poissonGraph->SetPoint(bin, x, y);
    poissonGraph->SetPointError(bin, 0., sqrt(y*(ncounts-1-y)/(ncounts-1)));
  }
  poissonGraph->Draw("P");
  poissonGraph->SetBit(kCanDelete);
  
  int minbin = _ronPlot->FindBin(min_bin_cutoff);
  //unsused:  int maxbin = _ronPlot->FindBin(max_bin_cutoff);

/*
  int maxbin = _ronPlot->FindBin(max_bin_cutoff);
  double sig = 0.;
  for (int bin=minbin; bin<=maxbin; bin++) {
    float datay = _ronPlot->GetBinContent(bin);
    double x = _ronPlot->GetBinCenter(bin);
    double theoryy = poissonGraph->Eval(x);
    double sigmay = poissonGraph->GetErrorY(bin);
    double nsigma = (datay-theoryy)/sigmay;
    sig += fabs(nsigma);
  }
*/

  // residual plot
  TString cName;
  cName.Form("c%s_Waterfall",GetName());
  TCanvas* cWaterfall = (TCanvas*)(GetParentFolder()->FindObject(cName.Data()));
  if(!cWaterfall) return;
  TVirtualPad* sPad = cWaterfall->cd(3);
  
  sPad->SetLogx(true);
  sPad->Clear();
  _residualPlot->Draw();

  TGraph* residuals = new TGraph;
  residuals->SetMarkerStyle(kFullDotMedium);
/*
  if (sig > alarm_threshold ) {
    residuals->SetMarkerColor(kRed);
  } else {
    residuals->SetMarkerColor(kGreen);
  }
*/
  residuals->SetMarkerColor(kRed);
  double maxresiduals = 0.;
//  for (int bin=minbin; bin<=maxbin; bin++) {
  for (int bin=1; bin<=totalbins; bin++) {
    float datay = _ronPlot->GetBinContent(bin);
    double x = _ronPlot->GetBinCenter(bin);
    double theoryy = poissonGraph->Eval(x);
    double residual = datay-theoryy;
    if (sigmas) residual /= poissonGraph->GetErrorY(bin);
    residuals->SetPoint(bin, x, residual);
    if (fabs(residual) > maxresiduals && x > min_bin_cutoff) maxresiduals = fabs(residual);
  }
  _residualPlot->GetYaxis()->SetRangeUser(-1*maxresiduals, maxresiduals); 
  residuals->Draw("P");

/* 
    Let's draw the Poisson distribution that we would have had just
    from the cosmic-rays on the detector and concrete slab, no HEU
*/

//  Compute the likelihood of having the measured counts given the 
//  Poisson model
  double logL = 0;
  double likelihoodmaxrange = xmin;
//  likelihoodmaxrange = 100E3;

/*
   Method 2:
   Find the point on the x-axis where the Poisson probability
   is equal to 0.01. This point is going to be the upper limit
   of our range of interest
  double x = 1./rate;
  double y = ncounts;
  nsteps = 0;
  while (nsteps < maxsteps && y>.01) {
    x = x/1.2;
    y = Poisson_fit->Eval(x)/ncounts;
cerr << nsteps << " x = " << x << " y = " << y << endl;
    nsteps++;
  };
  if (nsteps == maxsteps) {
    LOG << "We did not converge to the Poisson == 0.01 point in " << maxsteps << " steps." <<  ENDM_WARN;
  }
  cerr << "factorf = " << getFactor(totalbins, xminbin, xmaxbin) << endl;
  likelihoodmaxrange = x;
*/

/* 
  Method 3:
  Use non-zero bins above minbin until we have at least 'maxcount' 
  counts and compute the likelihood ratio in these bins
  likelihoodmaxrange = xmaxbin;
  end of Method 3
*/

  int upperbin = _ronPlot->FindBin(likelihoodmaxrange);
  if (upperbin < minbin) {
    char cbuff[1024];
    sprintf(cbuff,"WARNING: The fast neutron count rate is high (%g n/s).", 1e9*rate);
    TLatex *message = new TLatex(.13, 0.83, cbuff);
    message->SetNDC();
    message->Draw();
    char cbuff2[1024];
    sprintf(cbuff2,"The valley of interest [%g ns, %g ns] for analysis is not usable.", min_bin_cutoff, likelihoodmaxrange);
    TLatex *message2 = new TLatex(.13, 0.77, cbuff2);
    message2->SetNDC();
    message2->Draw();
    return;
  }
  float sumcounts = 0;
  double poissoncounts = 0.;
  for (int bin=1; bin<=totalbins; bin++) {
    poissoncounts += Poisson_fit->Eval(_ronPlot->GetBinCenter(bin));
  }
/* 
  Method 3:
  int maxcount = 20;
  for (int bin=minbin; bin<=upperbin && sumcounts<maxcount; bin++) {
*/
/*
  end of Method 3:
*/
  for (int bin=minbin; bin<=upperbin; bin++) {
    float datay = _ronPlot->GetBinContent(bin);
    double theoryy = Poisson_fit->Eval(_ronPlot->GetBinCenter(bin));
    sumcounts += datay;
    if (datay != 0.) {
      logL += log(theoryy/poissoncounts)*(datay-theoryy);
// cerr << bin << " datay = " << datay << " theoryy = " << theoryy << " ncounts = " << poissoncounts << " ln(./.)*(datay-theoryy) = " << log(theoryy/poissoncounts)*(datay-theoryy) << endl;
    }
  }
//  logL /= (upperbin-minbin+1);
  if (logL<alarm_threshold) alarm = true;
  char cbuff[1024];
  if (alarm) {
    sprintf(cbuff,"ALARM: In range [%g ns, %g ns], ln(likelihood of data) = %g.", min_bin_cutoff, likelihoodmaxrange, logL);
  } else {
    sprintf(cbuff,"In range [%g ns, %g ns], ln(likelihood of data) = %g.", min_bin_cutoff, likelihoodmaxrange, logL);
  }
  TLatex *message = new TLatex(.13, 0.83, cbuff);
  message->SetNDC();
  message->Draw();

  if (Nskips == 0) {
//  Compute the probability of having n counts in the time interval
//  range [min_bin_cutoff; max_bin_cutoff]
    double normprob = exp(-1.*Poisson_fit->GetParameter("rate")*min_bin_cutoff)-exp(-1.*Poisson_fit->GetParameter("rate")*likelihoodmaxrange);
    double expected_counts = ncounts*normprob;
    char cbuff2[1024];
    sprintf(cbuff2,"Expected/measured number of counts = %g / %d.", expected_counts, (int) sumcounts);
    TLatex *message2 = new TLatex(.13, 0.77, cbuff2);
    message2->SetNDC();
    message2->Draw();
  }
}

double NGMWaterfall::getFactor(int nbins, double axismin, double axismax) {
  return pow(axismax/axismin, (1./nbins));
}

void NGMWaterfall::setAlarmLevel(double likelihood) {
  alarm_threshold = likelihood;
}

double NGMWaterfall::getAlarmLevel() {
  return alarm_threshold;
}

void NGMWaterfall::setMinimumTimeCutoff(double cutoff_ns) {
  min_bin_cutoff = cutoff_ns;
}

double NGMWaterfall::getMinimumTimeCutoff() {
  return min_bin_cutoff;
}

bool NGMWaterfall::getAlarmStatus() {
  return alarm;
}

void NGMWaterfall::setNskips(int skip_value) {
 if(firstTime == 0) {
    Nskips = skip_value;
  } else {
    LOG << "Can not change the value of Nskips during a run" << ENDM_FATAL;
    exit(1);
  }
}

void NGMWaterfall::enableAlarm(bool enabled) {
  alarmEnabled = enabled;
};

void NGMWaterfall::enableBurstFilter(bool enabled) {
  burstFilterSetting = enabled;
};

void NGMWaterfall::residualsScaled2Sigmas(bool enabled) {
  sigmas = enabled;
};

bool NGMWaterfall::setMaxTime(double time) {
  if (time > 0) {
    maxTime = time;
    return true;
  } else {
    LOG << "The length of the data segment to be analyzed must be strictly positive." << ENDM_FATAL;
    return false;
  }
}

