// crossgain.C
// ca. 16apr07 - rjn originally in calibration.C
// 7may07 rew - new hack based on tests of 070425 gain curve data
// old version used second derivative
// to see where the 2.6 edge joined the background spectrum.
// This version looks for where the E*flux spectrum crosses a threshold
// as it flattens out at the high energy background


// the original routine was a cint macro
// to understand how to use available histogram methods
// to analyze features in a spectrum.
// that code is in calibration()

// getfeaturex only contains the data manipulation
// for feature extraction.

// get drawing region ready
TCanvas* cCalibration1 = new TCanvas("cCalibration1","Peak Finder Display");
// and give me a browser
//TBrowser* Browser1 = new TBrowser("root browser");

// the resolution-dependent smoother
void ressmooth(  TH1* inHist,
	Double_t widfrac=0.2
	)
{
	// because the for-loop repeatedly uses the input spectrum,
	// get a place to put the smoothed spectrum
	TH1* smooHist = inHist->Clone("smooHist");
	smooHist->Reset();

	Int_t nbins = inHist->GetNbinsX();

	// fill bins one at a time
	for(int ibin = 1; ibin < nbins+1; ibin++)
	{
		// each ibin's box width is x-dependent
		// but do the ends right
		Int_t limitwid=min(ibin-1,nbins-1-ibin-1);
		Int_t boxwidup=min(widfrac*ibin/2,limitwid);

		// this is giving trouble at the top end. that explains the two -1 in limitwid
		smooHist->SetBinContent(
			ibin,
			inHist->Integral(ibin-boxwidup,ibin+boxwidup)/(2*boxwidup+1)
			);
	}

	// overwrite the input and get rid of the intermediate
	//smooHist->Copy(*inHist);
	//*((TH1F*)inHist) = *((TH1F*)smooHist);
	//std::cout<<"smooHist: "<<smooHist->GetBinContent(100)<<std::endl;
	inHist->Reset();
	inHist->Add(smooHist);
	
	delete smooHist;
}

// the omnibus crossover-finder that:
// smoothes
// multiplies by ADC counts
// projects the data
// finds the nearly constant E*flux in background
// determines a threshold
// finds where it down-crosses that threshold
Float_t getfeaturex(const TH1* inHist,
	Double_t widfrac=0.2, // energy resolution
	Int_t n_smooth=1, // number of smooths
	Double_t threshtopeak=1.1 // parameter to whatever find peaks routine is used
	)
{

	Int_t nbins=inHist->GetNbinsX();

	TString procName(inHist->GetName());
	procName+="_procHist";

	
	// smooth the source histogram with a
	// resolution-scaled boxcar n_smooth times
	// before running the inflexion finder
	for (int smctr=0;smctr<n_smooth;smctr++)
	{
		ressmooth(procHist, widfrac);
	}

	TString featureName(inHist->GetName());
	featureName+="_featureHist";

	// get a place to put the filtered spectrum
	TH1* featureHist = procHist->Clone(featureName);
	featureHist->Reset();

	Float_t xvalues=featureHist->GetXaxis();

	/*
	// create a spectrum so you can fill
	//  a persistent list of found peaks
	// using unreliable outcome of Search
	TSpectrum spectrum;
	spectrum.Search(featureHist,peaksig,"goff,nobackground");

	// find maximum of all peaks 
	Float_t peakmax=0.;
	Float_t peakmaxloc=0.;
	for (Int_t pctr=0;pctr<spectrum.GetNPeaks();pctr++)
	{
		// I can't remember the elegant search loop. this'll do
		if (spectrum.GetPositionY()[pctr]>pmax)
		{
			peakmax=spectrum.GetPositionY()[pctr];
			peakmaxloc=spectrum.GetPositionX()[pctr];
		}
	}
	//printf("location of max peak is %d\n",peakmaxloc);
	*/
	
	// ShowPeaks is more of a display routine
	// that does not seem to return the peaks to the code
	//featureHist->ShowPeaks(peaksig);

	// some display stuff used during code development
	TVirtualPad *pad1=cCalibration1->cd(1);
	inHist->Draw();
	pad1->SetLogy();
	pad1->Modified();

	procHist->SetDirectory(gROOT);
	TVirtualPad *pad2=cCalibration1->cd(2);
	procHist->Draw();
	pad2->Modified();

	featureHist->SetDirectory(gROOT);
	TVirtualPad *pad3=cCalibration1->cd(3);
	featureHist->Draw();
	pad3->Modified();

	cCalibration1->Modified();
	TString gOutName(procHist->GetName());
	gOutName+=".png";
	cCalibration1->Print(gOutName);

	return(featureloc);
}

TH2F *fillgainarray(
	const Int_t minch=0,
	const Int_t maxch=23,
	const Bool_t uselog=1,
	const Double_t widfrac=0.2,
	const Int_t n_smooth=1,
	const Double_t d2widfrac=1.0,
	const Int_t downwid=3,
	const Int_t d2dx2_maxflag=0,
	const Bool_t useweight=1,
	const Bool_t cliplow=1
	//const Int_t rebinval=1,
	//const Double_t peakboxwid=50
	)
{
	const Int_t nchans=maxch-minch+1;
	
	//unhappily hard-coded stuff for early tests
	const Int_t nfiles=9;
	char *filenames[nfiles]= 
	{
		"NGMAna20070425013517ngm.root",
		"NGMAna20070425023518ngm.root",
		"NGMAna20070425033520ngm.root",
		"NGMAna20070425043521ngm.root",
		"NGMAna20070425053522ngm.root",
		"NGMAna20070425063524ngm.root",
		"NGMAna20070425073525ngm.root",
		"NGMAna20070425083527ngm.root",
		"NGMAna20070425093528ngm.root"
	};

	// more unhappily done stuff:
	// silly way to fill the coords of the 2D hist
	// until the vals are passed in with the filenames
	Double_t *filenums = new Double_t[nfiles+1];
	Double_t *chnums = new Double_t[nchans+1];
	// note "bins" act like picket-fence pickets: one more than number of bins
	for (Int_t fctr=0;fctr<=nfiles;fctr++) filenums[fctr]=(Double_t)fctr;
	for(Int_t chctr=minch;chctr<=maxch;chctr++) chnums[chctr-minch]=(Double_t)chctr;

	TH2F *gainTH2 = new TH2F("gainTH2","gain array",nfiles,0,nfiles,nchans,minch,maxch+1);

	cCalibration1->SetWindowSize(1000,1000);
	cCalibration1->Divide(1,3);

	for (Int_t fctr=0;fctr<nfiles;fctr++)
	{
		// Opens root file in read-only mode
		TFile* inputfile = TFile::Open(filenames[fctr]);
		if (!inputfile) continue;
		for(Int_t chctr=minch;chctr<=maxch;chctr++)
		{
			char histname[128];
			sprintf(histname,"NMON_g2prime_%d",chctr);

			// get a particular named histogram into tmpHist
			TH1* g2primeHist = (TH1*)(inputfile->Get(histname));
			if (g2primeHist)
			{
				g2primeHist->SetDirectory(gROOT);
				sprintf(histname,"%s_%d",histname,fctr);
				g2primeHist->SetName(histname);
				//g2primeHist->Rebin(rebinval);

				gainTH2->SetBinContent(fctr+1,chctr-minch+1,
					getfeaturex(g2primeHist,
						uselog,widfrac,n_smooth,d2widfrac,downwid,
						d2dx2_maxflag,useweight,cliplow));
				gSystem->ProcessEvents();
			}
			printf("%d\t",gainTH2->GetBinContent(fctr+1,chctr-minch+1));
		} // loop on channels
 	if (inputfile) inputfile->Close();
	} // loop on files
	printf("\n");
	return(gainTH2);
}
