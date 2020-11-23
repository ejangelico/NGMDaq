/*
 *  NGMTimeGapFinder.cpp
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/5/08.
 *  Copyright 2008 LLNL. All rights reserved.
 *
 */

#include "NGMTimeGapFinder.h"
#include "NGMLogger.h"

#include "TMath.h"
#include "TText.h"
#include "TCanvas.h"
#include "TVirtualPad.h"
#include "TH2D.h"
#include "THStack.h"
#include "TStyle.h"

ClassImp(NGMTimeGapFinder)

//______________________________________________________________________________
NGMTimeGapFinder::NGMTimeGapFinder()
{
  _gaps = 0;  // don't let Reset do the wrong thing
  _live = 0;  // don't let Reset do the wrong thing
  Reset();
}

//______________________________________________________________________________
NGMTimeGapFinder::~NGMTimeGapFinder()
{
  delete _gaps;
  delete _live;
}

//______________________________________________________________________________
bool
NGMTimeGapFinder::Reset()
{
  _counts            = -1;
  _ngaps             =  0;

  _deadtime          =  0;
  _averageInterval   =  0;

  if (_gaps) { _gaps->Reset(); } else { _gaps = 0; }
  if (_live) { _live->Reset(); } else { _live = 0; }
	
  return true;
}

//______________________________________________________________________________
void
NGMTimeGapFinder::SetAcquireLiveHistogram(int liveOn)
{
  Init_live();
}

//______________________________________________________________________________
bool
NGMTimeGapFinder::nextTimeIsGap(NGMTimeStamp nexttime)
{
  // This method determines whether there is a hole in the data
  // stream, meaning whether we are between buffers. The criteria
  // used to determine whether there is a hole is the following:
  // The average time between counts is determined as soon as we 
  // have at least 'minpart' particles in the buffer, and is 
  // recomputed every 'minpart' particles. Then, if the interval
  // to the next particle is at least 'mlogpoisson0' times this
  // average time, it is determined to be a hole.
  // Returns:
  //   true when a gap is detected between 2 buffers
  //        for the first time in the data stream
  //
  // For this method to work, we can not have any particle selection
  // applied to it, it has to see all particles

  const int minpart = 100;  // Min number of hits to evaluate gap detection
  static const double mlogpoisson0 = -1.*TMath::Log(1e-11);

  bool isAGap = false;

  if (_counts == -1) {           // Check for first hit
    _firstTimeOfRun = nexttime;
	_timeOfPreviousHit = nexttime;
	_firstTimeSinceGap = nexttime;
  }

  double timeSincePrevious = nexttime.TimeDiffNanoSec(_timeOfPreviousHit);
  if(timeSincePrevious < 0.0)
  {
      LOG<<" DATA NOT SORTED "<<ENDM_FATAL;
  }
  _timeOfPreviousHit = nexttime;
  _counts++;             // Update for this hit
	
  // Check for the gap
  _averageInterval = GetLiveTime()/(_counts - _ngaps);

  if(    (_counts > minpart)
	  && (timeSincePrevious > mlogpoisson0*_averageInterval)
		 ) {
	// gap detected!  and sufficient counts
	isAGap = true;     
	//LOG<<"Gap detected"<<ENDM_INFO;
	_ngaps++;
	if (! _gaps) Init_gaps();
	_gaps->Fill(timeSincePrevious);
	_deadtime+= timeSincePrevious;
	_firstTimeSinceGap = nexttime;

  } else {

	if ( _live ) _live->Fill(timeSincePrevious);

  }
  
  return isAGap;
}

//______________________________________________________________________________
void
NGMTimeGapFinder::Init_gaps()
{
  // initialize _gaps histogram

  //LOG<<"\tKnitting gaps"<<ENDM_INFO;
  
  if (!_gaps) delete _gaps;
  bool olddir = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);
  _gaps = new TH1D(TString(GetName()) + "_Gaps",
				   "Detected Time Gaps", 50, 1, 0);
  TH1::AddDirectory(olddir);
  // 50 bins, automatic binning
  _gaps->GetXaxis()->SetTitle("Gap Width (ns)");
  _gaps->GetYaxis()->SetTitle("Number of Gaps");
  _gaps->Fill(1);
}

//______________________________________________________________________________
void
NGMTimeGapFinder::Init_live()
{
  // initialize _live histogram

  //LOG<<"\tKnitting live"<<ENDM_INFO;
  
  if (!_live) delete _live;
  
  _live = new TH1D(TString(GetName()) + "_Live",
				   "Live Time Interval Distribution",
				   50, 1, 0);
  // 50 bins, automatic binning
  _live->GetXaxis()->SetTitle("Live Time Interval (ns)");
  _live->GetYaxis()->SetTitle("Number of Intervals");
  _live->Fill(1);
}

//______________________________________________________________________________
TH1 *
NGMTimeGapFinder::DrawGaps(Option_t *opt)
{
  // Draw gaps histogram on current pad

  TString options(opt);

  // draw a copy of gaps, so we can find the range
  if (!_gaps) { Init_gaps(); }

  TH1D *g = (TH1D*)_gaps->Clone();
  g->SetLineColor(kRed);
  g->Draw(options);
  gStyle->SetOptStat("n"); // name only in stats box
  gPad->SetLogy();
  gPad->Update();

  // hack to plot live intervals too
  if (_live) {

	TVirtualPad *c = gPad;
	gPad->Divide(2,1);
	c->cd(1);
	g->Draw(options);
	gPad->SetLogy();

	TH1D *l = (TH1D*)_live->Clone();
	c->cd(2);
	l->Draw(options);
	gPad->SetLogy();

	/*
	//  Tried TStack, but doesnt' do well when the x axis ranges are vastly different
	THStack *s = new THStack(TString(g->GetName()) + "_DrawStack",
							 g->GetTitle());
	s->Add(g);
	s->Add(l);
	s->Draw(options + "NOSTACK");

	// Tried to overplot, also failed
	// get the data range from g
	double xlow = g->GetXaxis()->GetXmin();
	double xup  = g->GetXaxis()->GetXmax();
	double ylow = g->GetMinimum();
	double yup  = g->GetMaximum();
	
	// draw _live once, to get its range
	TH1 *l = _live->DrawCopy(options);

	// now use the larger of the two ranges
	int nbins = 10;
	xlow =      min(xlow, l->GetXaxis()->GetXmin());
	xup  = 1.1* max(xup,  l->GetXaxis()->GetXmax());
	ylow = 1;  // min(ylow, l->GetMinimum());
	yup  = 1.1* max(yup,  l->GetMaximum());

	TH2D *r = new TH2D(TString(g->GetName()) + "_drawHack", g->GetTitle(),
					   nbins, xlow, xup,  nbins, ylow, yup);
	r->Draw(options + "AXIS");
	g->Draw(options + " SAME");
	l->Draw(options + " SAME");

	*/
	
  } // if (_live)

  
  // label maker
  struct label {
	float xndc;  // x position, in NDC		
	float yndc;	 // prev y position, in NDC	
	float dyndc; // y decrement
	TText lab;
	
	label(const double x,
		  const double y,
		  const double dy) {
	  xndc  = x;
	  yndc  = y;
	  dyndc = dy;
	  lab.SetTextSize(0.03);
	  lab.SetNDC();
	}
	void bel(TString f) { lab.DrawText(xndc, (yndc -= dyndc),f); }
  } la(0.15, 0.89, 0.04);
  
  la.bel(TString::Format("Average interval = %G (ns)", _averageInterval));
  la.bel(TString::Format(""));                         // blank line
  la.bel(TString::Format("Total counts = %lli",          _counts));
  la.bel(TString::Format("Number of gaps = %lli",        _ngaps));
  la.bel(TString::Format(""));                         // blank line
  la.bel(TString::Format("Livetime = %G %G%%",         GetLiveTime(), GetLiveTimePct()));
  la.bel(TString::Format("Deadtime = %G",              _deadtime));

  return g;
}
  
