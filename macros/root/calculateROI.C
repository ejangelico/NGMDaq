TCutG* calculateROI(TH2* h2DOrig)
{
double xval[10];
xval[0] = 200.0;
xval[1] = 300.0;
xval[2] = 400.0;
xval[3] = 500.0;
xval[4] = 1000.0;
xval[5] = 1500.0;
xval[6] = 2000.0;
xval[7] = 2500.0;
xval[8] = 3000.0;
xval[9] = 3500.0;

	TH2* h2D = new TH2F("h2D","h2D",9,xval,1000,0.0,1.2);
	for(int xbin = 1; xbin <= h2DOrig->GetNbinsX(); xbin++)
	{
		double xval1 = h2DOrig->GetXaxis()->GetBinCenter(xbin);
		for(int ybin = 1; ybin <= h2DOrig->GetNbinsY(); ybin++)
		{
			double yval = h2DOrig->GetYaxis()->GetBinCenter(ybin);
			h2D->Fill(xval1,yval, h2DOrig->GetBinContent(xbin,ybin));		
		}
	}

  NGMNeutronMonitor::CalculateFOM1(h2D);
  
	// Now find the sigam, means, and chi2
	char cbuf[1024];
	TH1* hNnorm = (TH1*)(gROOT->FindObject("h2D_3"));
	TH1* hNmean = (TH1*)(gROOT->FindObject("h2D_4"));
	TH1* hNsig = (TH1*)(gROOT->FindObject("h2D_5"));
	TH1* hGnorm = (TH1*)(gROOT->FindObject("h2D_0"));
	TH1* hGmean = (TH1*)(gROOT->FindObject("h2D_1"));
	TH1* hGsig = (TH1*)(gROOT->FindObject("h2D_2"));
	TH1* hchi2 = (TH1*)(gROOT->FindObject("h2D_chi2"));
	// check for swap of gamma and neutron
  if(hNmean->GetBinContent(2) < hGmean->GetBinContent(2))
  {
    hNnorm = (TH1*)(gROOT->FindObject("h2D_0"));
    hNmean = (TH1*)(gROOT->FindObject("h2D_1"));
    hNsig = (TH1*)(gROOT->FindObject("h2D_2"));
    hGnorm = (TH1*)(gROOT->FindObject("h2D_3"));
    hGmean = (TH1*)(gROOT->FindObject("h2D_4"));
    hGsig = (TH1*)(gROOT->FindObject("h2D_5"));
    hchi2 = (TH1*)(gROOT->FindObject("h2D_chi2"));    
  }
  // We create three curves:
  // 1 Not gamma upper bound
  // 2 Neutron Lower bound
  // 3 Neutron Upper bound
  sprintf(cbuf,"%s_NotGammaUpper",h2DOrig->GetName());
  TH1* hNotGammaUpper = (TH1*) (hNnorm->Clone(cbuf));
  hNotGammaUpper->Reset();
  sprintf(cbuf,"%s_NeutronLower",h2DOrig->GetName());  
  TH1* hNeutronLower = (TH1*) (hNnorm->Clone(cbuf));
  hNeutronLower->Reset();
  sprintf(cbuf,"%s_NeutronUpper",h2DOrig->GetName());  
  TH1* hNeutronUpper = (TH1*) (hNnorm->Clone(cbuf));
  hNeutronUpper->Reset();
  
  
  hNotGammaUpper->Add(hGmean);
  hNotGammaUpper->Add(hGsig,10.0);
  
  hNeutronLower->Add(hNmean);
  hNeutronLower->Add(hNsig,-1.2);
  
  hNeutronUpper->Add(hNmean);
  hNeutronUpper->Add(hNsig,3.0);
  
  // Create the graphical 2D cut
  int npoints = 0;
  sprintf(cbuf,"%s_NCut",h2DOrig->GetName());
  TCutG* tcut = new TCutG;
  tcut->SetName(cbuf);
  gROOT->Add(tcut);
  int ibin = 0;
  // First find where the neutron upper is above the !gamma
  for(ibin = 1; ibin <= h2D->GetNbinsX(); ibin++)
  {
    if(hNotGammaUpper->GetBinContent(ibin)< hNeutronUpper->GetBinContent(ibin))
      break;
  }
  int firstbin = ibin;
  double lowervalue = hNotGammaUpper->GetBinContent(ibin) > hNeutronLower->GetBinContent(ibin) ?
    hNotGammaUpper->GetBinContent(ibin) : hNeutronLower->GetBinContent(ibin);
    
  tcut->SetPoint(npoints++, hNeutronLower->GetXaxis()->GetBinLowEdge(ibin),lowervalue);
  
  // Lets add points up to 3000
  for(ibin = firstbin; ibin <= h2D->GetXaxis()->FindBin(2600.0); ibin++)
  {
    lowervalue = hNotGammaUpper->GetBinContent(ibin) > hNeutronLower->GetBinContent(ibin) ?
      hNotGammaUpper->GetBinContent(ibin) : hNeutronLower->GetBinContent(ibin);
    tcut->SetPoint(npoints++, hNeutronLower->GetXaxis()->GetBinCenter(ibin),lowervalue);
  }
  ibin--;
  // Now lets turn the corner
  lowervalue = hNotGammaUpper->GetBinContent(ibin) > hNeutronLower->GetBinContent(ibin) ?
    hNotGammaUpper->GetBinContent(ibin) : hNeutronLower->GetBinContent(ibin);
  tcut->SetPoint(npoints++, hNeutronLower->GetXaxis()->GetBinUpEdge(ibin),lowervalue);
  tcut->SetPoint(npoints++, hNeutronLower->GetXaxis()->GetBinUpEdge(ibin),hNeutronUpper->GetBinContent(ibin));
  
  // Lets work our way down the upper edge
  for(; ibin >= firstbin; ibin--)
  {
    tcut->SetPoint(npoints++, hNeutronUpper->GetXaxis()->GetBinCenter(ibin),hNeutronUpper->GetBinContent(ibin));
  }
  ibin++;
  // Lets add the lower edge of the first bin
  tcut->SetPoint(npoints++, hNeutronUpper->GetXaxis()->GetBinLowEdge(ibin),hNeutronUpper->GetBinContent(ibin));
  
  // Then close the loop
  tcut->SetPoint(npoints++,tcut->GetX()[0], tcut->GetY()[0]);

  delete h2D;
  delete hNnorm;
  delete hNmean;
  delete hNsig;
  delete hGnorm;
  delete hGmean;
  delete hGsig;
  delete hchi2;    
  
  
  return tcut;
}

void saveROIs(TFile* infile)
{
  if(!infile) return;

  TFile* roiOut = new TFile("ROIs.root","RECREATE");

  TString prefix("NMON_g3pRg2pCal");

  infile->cd();
  NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*)(infile->Get("SIS3150"));
  NGMConfigurationTable* chanConf = sysConf->GetChannelParameters();

  char cbuf[1024];
  for(int ichan = 0; ichan < chanConf->GetEntries(); ichan++)
  {
      // Some bad channels
      //if(islot == 0 && ichan == 0) continue;
      //if(islot == 6 && ichan == 2) continue;
      //if(islot == 10 && ichan == 0) continue;
      //if(islot == 10 && ichan == 1) continue;
      //if(islot == 10 && ichan == 2) continue;
     
      TString detName(chanConf->GetParValueS("DetectorName",ichan));
      if(!detName.BeginsWith("LS")) continue;

      sprintf(cbuf,"%s_%s",prefix.Data(),detName.Data());
      TH2* h2D = (TH2*)(tf->Get(cbuf));
      if(!h2D)
      {
        std::cerr<<" Cannot find histogram " << cbuf <<std::endl;
        continue;
      }
      TCutG* mycut = calculateROI(h2D);
      if(!mycut)
      {
        std::cerr<<" calculateROI returned null cutter."<<std::endl;
        continue;
      }
      std::cout<<"Saving "<<mycut->GetName()<<std::endl;
      roiOut->WriteTObject(mycut,mycut->GetName());
  }

  roiOut->Close();
  delete roiOut;
}