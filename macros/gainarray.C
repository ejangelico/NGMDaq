// gainarray.C
// ca. 16apr07 - rjn originally in calibration.C
// 23apr07 rew - new hack


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
		Int_t boxwidup=min((widfrac*ibin)/2,limitwid);

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

// the omnibus biggest-feature-finder that:
// finds the log
// smoothes
// uses a second derivative boxcar
// scales by something like the counting error
Float_t getfeaturex(const TH1* inHist,
	Bool_t uselog=1,
	Double_t widfrac=0.2, // energy resolution
	Int_t n_smooth=1, // number of smooths
	Double_t d2widfrac=1.0, // energy resolution
	Int_t downwid=3, // ratio of down to up when doing "second deriv" boxcar
	Int_t d2dx2_maxflag=0, // concave up or down. zero means min in second deriv = concave up.
	Bool_t useweight=1, // try to use weighting to get peaks in feature
	Bool_t cliplow=1 // clip region to look for concave point
	//Double_t peaksig=3 // parameter to whatever find peaks routine is used
	)
{

	Int_t nbins=inHist->GetNbinsX();

	TString procName(inHist->GetName());
	procName+="_procHist";

	// make a copy of tmpHist to fill with log values
	TH1* procHist = inHist->Clone(procName);
		
	if (uselog)
	{
		//zero out all info in this object
		procHist->Reset();

		// make histo of log y values
		// Lowest bin is bin 1, highest is nbins
		for(int ibin =1; ibin < procHist->GetNbinsX(); ibin++)
		{
			// set the y values in logHist equal to the log of the y in tmpHist
			// set log(value = 0) to -1
			procHist->SetBinContent(ibin, log(max(0.1,inHist->GetBinContent(ibin))));
		}
	}
	
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

	// simple inflection finder:
	// difference of two square filters
	for(int ibin =1; ibin < nbins; ibin++)
	{
		// up and down box widths are x-dependent
		// this is the culprit in getting the damn sawtooth second deriv
		Int_t boxwiddn=(Int_t)((Double_t) (ibin*downwid)*d2widfrac*0.5);
		Int_t boxwidup=boxwiddn/downwid;
		// do the ends right
		// the next lines have the effect that, 
		// near the ends, the up and down boxes are the same, 
		// and so the difference goes to zero
		Int_t limitwid=min(ibin-1,nbins-1-ibin);
		boxwidup=min(boxwidup,limitwid);
		boxwiddn=min(boxwiddn,limitwid);

		// replace each value with its zero-area matched-filter boxcar second derivative.
		// NOTE:
		// the Integral function is (allegedly) a sum unless "width" is specified,
		// so don't specify it
		featureHist->SetBinContent(ibin,
			(procHist->Integral(ibin-boxwidup,ibin+boxwidup)/(2*boxwidup+1)
			-procHist->Integral(ibin-boxwiddn,ibin+boxwiddn)/(2*boxwiddn+1)));
	}
	
	// finally, scale the wacked-out output array by
	// a counting-error-like value, i.e. sqrt(counts)
	if (useweight)
	{
		for(int ibin =1; ibin < nbins; ibin++)
		{
			// this weighting has too much black magic!
			// here we weight with the (logarithmic) prochist instead of the error
			// and offset it by 1 because of the way we filled the log for zero counts
			featureHist->SetBinContent(ibin,
				(procHist->GetBinContent(ibin)) ? 
				//(featureHist->GetBinContent(ibin))*exp(procHist->GetBinContent(ibin)/2.) : 0
				(featureHist->GetBinContent(ibin))*(procHist->GetBinContent(ibin)+1) : 0
				);
		}
	}

	// simple finder of extrema
	// first find maximum in spectrum, that shows the threshold
	// then look for the maximum in the feature array above the spectrum max
	Int_t procmaxloc= (cliplow) ? procHist->GetMaximumBin(): 1;
	//printf("max bin in proc array = %d\n",procmaxloc);
	// I wish I could set the range for finding the max in the feature array
	// but this method is "not defined in the current scope" (???)
	// SetAxisRange(procmaxloc,nbins);
	// so instead, avoid the peak by a factor of 2
	for (int ibin=1;ibin < procmaxloc*2; ibin++)
		featureHist->SetBinContent(ibin,0);
	// and search for biggest peak in the rest of the feature histogram
	Int_t featurebin= (d2dx2_maxflag) ? featureHist->GetMaximumBin() : featureHist->GetMinimumBin();
	Float_t featureloc=featureHist->GetXaxis()->GetBinCenter(featurebin);

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
