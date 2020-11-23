
Float_t getfeaturex(const char* filename,
	const char* histname,
	Int_t d2dx2_multiplier=-1,
	Int_t smooth_param=20,
	Double_t widfrac=0.2,
	Int_t downwid=3,
	Double_t peaksig=3
	)
{
	// I/O

	// Opens root file in read only mode
	TFile* inputfile = TFile::Open(filename);
	
	// get a particular named histogram into tmpHist
	TH1* tmpHist = (TH1*)(inputfile->Get(histname));
	
	// DATA MANIPULATION

	// make a copy of tmpHist to fill with log values
	TH1* logHist = tmpHist->Clone("logHist");

	//zero out all info in this object
	logHist->Reset();

	// make histo of log y values
	// Lowest bin is bin 1, highest is nbins
	
	for(int ibin =1; ibin < logHist->GetNbinsX(); ibin++)
	{
		// set the y values in logHist equal to the log of the y in tmpHist
		// set log(value = 0) to -1
		logHist->SetBinContent(ibin, log(max(0.1,tmpHist->GetBinContent(ibin))));
	}
	
	// simple inflection finder:
	// difference of two square filters

	// make a copy of logHist into filtHist
	TH1* filtHist = logHist->Clone("filtHist");

	// fill bins one at a time
	for(int ibin =1; ibin < logHist->GetNbinsX(); ibin++)
	{
		// up and down box widths are x-dependent
		Int_t boxwidup=widfrac*ibin;
		Int_t boxwiddn=downwid*boxwidup;

		// do the ends right
		Int_t upstart=max(ibin-boxwidup,1);
		Int_t upfinish=min(ibin+boxwidup,logHist->GetNbinsX()-1);
		Int_t dnstart=max(ibin-boxwiddn,1);
		Int_t dnfinish=min(ibin+boxwiddn,logHist->GetNbinsX()-1);

		// replace each value with its zero-area matched-filter second derivative
		// multiplier should be -1 when you want
		// a concave-up spectral slope transition to map to an up-going peak
		
		filtHist->SetBinContent(ibin,d2dx2_multiplier*(
			logHist->Integral(upstart,upfinish)/(Double_t)(upfinish-upstart+1)
			-logHist->Integral(dnstart,dnfinish)/(Double_t)(dnfinish-dnstart+1))
			);
		}

	// smooth second derivative
	filtHist->Smooth(smooth_param,-1,-1);

	//DISPLAY

	// get drawing region ready
	TCanvas* cCalibration1 = new TCanvas("cCalibration1","Peak Finder Display");
	cCalibration1->Divide(1,2);
	cCalibration1->SetWindowSize(1000,1000);

	TVirtualPad *pad1=cCalibration1->cd(1);
	pad1->SetGridx();
	logHist->Draw();
	TVirtualPad *pad2=cCalibration1->cd(2);
	pad2->SetGridx();
	filtHist->Draw();
	filtHist->ShowPeaks(peaksig);

	// create a persistent list of found peaks
	TSpectrum spectrum;
	spectrum.Search(filtHist,peaksig);

	// find maximum of all peaks
	Float_t pmax=0.;
	Float_t pmaxloc=0.;
	for (Int_t pctr=0;pctr<spectrum.GetNPeaks();pctr++)
	{
		// can't remember elegant search loop. this'll do
		if (spectrum.GetPositionY()[pctr]>pmax)
		{
			pmax=spectrum.GetPositionY()[pctr];
			pmaxloc=spectrum.GetPositionX()[pctr];
		}
	}
	//printf("location of max peak is %d\n",pmaxloc);

	return(pmaxloc);

}

void loop_on_data()
{
  //unhappily hard coded stuff for early tests
  Int_t minch=12;
  Int_t maxch=23;
  const Int_t nchans=maxch-minch+1;
  const Int_t nfiles=4;
  
  char *filenames[nfiles]= 
  {"NGMAna20070420215009ngm.root",
   "NGMAna20070420215212ngm.root",
   "NGMAna20070420215414ngm.root",
   "NGMAna20070420215615ngm.root"
  };
	Int_t gainarr[nfiles][nchans];
  
	for (Int_t fctr=0;fctr<nfiles;fctr++)
	{
		for(Int_t chctr=minch;chctr<=maxch;chctr++)
		{
			char histname[128];
      sprintf(histname,"NMON_g2prime_%2d",chctr);
			gainarr[fctr][chctr-minch]=
				getfeaturex(filenames[fctr],histname,-1,1000,0.2,3,3);
      printf("%d\t",gainarr[fctr][chctr-minch]);
		}
    printf("\n");
	}
}

void calibration(const char* filename,
	const char* histname="NMON_g2prime_12",
	Int_t d2dx2_multiplier=-1,
	Int_t smooth_param=20,
	Double_t widfrac=0.2,
	Int_t downwid=3,
	Double_t peaksig=3
	)
{
	// I/O

	// Opens root file in read only mode
	TFile* inputfile = TFile::Open(filename);
	
	// get a particular named histogram into tmpHist
	TH1* tmpHist = (TH1*)(inputfile->Get(histname));
	
	// DATA MANIPULATION

	// make a copy of tmpHist to fill with log values
	TH1* logHist = tmpHist->Clone("logHist");

	//zero out all info in this object
	logHist->Reset();

	// make histo of log y values
	// Lowest bin is bin 1, highest is nbins
	
	for(int ibin =1; ibin < logHist->GetNbinsX(); ibin++)
	{
		// set the y values in logHist equal to the log of the y in tmpHist
		// set log(value = 0) to -1
		logHist->SetBinContent(ibin, log(max(0.1,tmpHist->GetBinContent(ibin))));
	}
	

	// simple inflection finder:
	// difference of two square filters

	// make a copy of logHist into filtHist
	TH1* filtHist = logHist->Clone("filtHist");

	// fill bins one at a time
	for(int ibin =1; ibin < logHist->GetNbinsX(); ibin++)
	{
		// up and down box widths are x-dependent
		Int_t boxwidup=widfrac*ibin;
		Int_t boxwiddn=downwid*boxwidup;

		// do the ends right
		Int_t upstart=max(ibin-boxwidup,1);
		Int_t upfinish=min(ibin+boxwidup,logHist->GetNbinsX()-1);
		Int_t dnstart=max(ibin-boxwiddn,1);
		Int_t dnfinish=min(ibin+boxwiddn,logHist->GetNbinsX()-1);

		// replace each value with its zero-area matched-filter second derivative
		// multiplier should be -1 when you want
		// a concave-up spectral slope transition to map to an up-going peak
		filtHist->SetBinContent(ibin,d2dx2_multiplier*(
			logHist->Integral(upstart,upfinish)/(Double_t)(upfinish-upstart+1)
			-logHist->Integral(dnstart,dnfinish)/(Double_t)(dnfinish-dnstart+1))
			);
	}

	// smooth second derivative
	filtHist->Smooth(smooth_param,-1,-1);

	//DISPLAY

	// get drawing region ready
	TCanvas* cCalibration1 = new TCanvas("cCalibration1","Peak Finder Display");
	cCalibration1->Divide(1,2);
	cCalibration1->SetWindowSize(1000,1000);

	TVirtualPad *pad1=cCalibration1->cd(1);
	pad1->SetGridx();
	logHist->Draw();
	TVirtualPad *pad2=cCalibration1->cd(2);
	pad2->SetGridx();
	filtHist->Draw();
	filtHist->ShowPeaks(peaksig);
	
	// create a persistent list of found peaks
	TSpectrum spectrum;
	spectrum.Search(filtHist,peaksig);

	// find maximum of all peaks
	Float_t pmax=0.;
	Float_t pmaxloc=0.;
	for (Int_t pctr=0;pctr<spectrum.GetNPeaks();pctr++)
	{
		// can't remember elegant search loop. this'll do
		if (spectrum.GetPositionY()[pctr]>pmax)
		{
			pmax=spectrum.GetPositionY()[pctr];
			pmaxloc=spectrum.GetPositionX()[pctr];
		}
	}
	printf("location of max peak is %d\n",pmaxloc);
	
}
