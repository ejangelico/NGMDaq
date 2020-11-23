double calculateMeanofLogHist(TH1* hist)
{
  if(!hist) return 0.0;
  double stats = 0.0;
  double runningsum = 0.0;
  for(int ibin = 11; ibin <= hist->GetNbinsX(); ibin++)
  {
    double thisVal = log(hist->GetBinContent(ibin)+1);
    stats+=thisVal;
    runningsum+=thisVal * hist->GetXaxis()->GetBinCenter(ibin);
  }
  if(stats>0.0) runningsum = runningsum/stats;

  return runningsum;
}


void singleIter(const char* detPattern, double targetCG)
{
  int chancount = 0;
  double maxv = 1.4;
  double minv = 0.8;
  double incv = 0.05;
  if(TString(detPattern).BeginsWith("LS"))
  {
    maxv = 1300.0;
    minv = 900.0;
    incv = 100.0;
  }
  NGMConfigurationTable* cChan = NGMSystem::getSystem()->GetConfiguration()->GetChannelParameters();
  NGMConfigurationTable* cHV = NGMSystem::getSystem()->GetConfiguration()->GetHVParameters();
  for(int ichan = 0; ichan < cChan->GetEntries(); ichan++)
  {
    TString thisChan(cChan->GetParValueS("DetectorName",ichan));
    if(thisChan.BeginsWith(detPattern))
    {
      chancount++;
    }
  }

  // Allocate Arrays
  TObjArray detNames(chancount);
  TArrayS errorStatus(chancount);
  TArrayD minVoltage(chancount);
  TArrayD maxVoltage(chancount);
  TArrayD firstVoltage(chancount);
  TArrayD secondVoltage(chancount);
  TArrayD incrementVoltage(chancount);
  TArrayD firstCG(chancount);
  TArrayD secondCG(chancount);
  int thisIndex = 0;
  for(int ichan = 0; ichan < cChan->GetEntries(); ichan++)
  {
    TString thisChan(cChan->GetParValueS("DetectorName",ichan));
    if(thisChan.BeginsWith(detPattern))
    {
      detNames[thisIndex] = new TObjString(thisChan.Data());
      std::cout<<"Analyzing "<< thisChan.Data()<<std::endl;
      // These values we will lookup from DetectorTable
      minVoltage[thisIndex] = minv;
      maxVoltage[thisIndex] = maxv;
      incrementVoltage[thisIndex] = incv;

      firstVoltage[thisIndex] = TString(cHV->LookupParValueAsString("Voltage","DetectorName",thisChan.Data())).Atof();
      secondVoltage[thisIndex] = TString(cHV->LookupParValueAsString("Voltage","DetectorName",thisChan.Data())).Atof() + incrementVoltage[thisIndex];
      firstCG[thisIndex] = 0.0;
      secondCG[thisIndex] = 0.0;
      errorStatus[thisIndex] = 0;
      // increment chan index
      thisIndex++;
    }
  }

  // Now we take data
  NGMSystem::getSystem()->GetConfiguration()->GetSystemParameters()->SetParameterFromString("RunMaxSecondsCounter",0,"60.0");
  NGMSystem::getSystem()->ConfigureSystem();
  // Wait for HV to Adjust
  TThread::Sleep(10);
  NGMSystem::getSystem()->StartAcquisition();

  // Data taking complete
  // now analyze data
  for(int ichan = 0; ichan < chancount; ichan++)
  {
    TString thisChan = ((TObjString*)(detNames[ichan]))->GetString();
    // Find the histogram
    TString histName("NMON_g2prime_");
    histName+=thisChan;
    TH1* cHist = (TH1*)(gROOT->FindObject(histName.Data()));
    if(!cHist)
    {
      std::cerr<<" Histogram "<<histName.Data()<<" not found!"<<std::endl;
      errorStatus[ichan] = 1;
      continue;
    }
    double cMean = calculateMeanofLogHist(cHist);
    firstCG[ichan] = cMean;
  }

  // Now set second voltages and take data
  for(int ichan = 0; ichan < chancount; ichan++)
  {
    if(errorStatus[ichan]) continue;

    TString thisChan = ((TObjString*)(detNames[ichan]))->GetString();
    if(secondVoltage[ichan]>minVoltage[ichan] && secondVoltage[ichan]<maxVoltage[ichan])
    {
      cHV->SetParameterAsStringThatBeginsWith("Voltage",TString("")+=secondVoltage[ichan],
                                              "DetectorName",thisChan.Data());
    }else{
      errorStatus[ichan] = 1;
    }
  }

  // Now we take data
  NGMSystem::getSystem()->GetConfiguration()->GetSystemParameters()->SetParameterFromString("RunMaxSecondsCounter",0,"60.0");
  NGMSystem::getSystem()->ConfigureSystem();
  // Wait for HV to Adjust
  TThread::Sleep(10);
  NGMSystem::getSystem()->StartAcquisition();


 // Data taking complete
  // now analyze data
  for(int ichan = 0; ichan < chancount; ichan++)
  {

    if(errorStatus[ichan]) continue;

    TString thisChan = ((TObjString*)(detNames[ichan]))->GetString();
    // Find the histogram
    TString histName("NMON_g2prime_");
    histName+=thisChan;
    TH1* cHist = (TH1*)(gROOT->FindObject(histName.Data()));
    if(!cHist)
    {
      std::cerr<<" Histogram "<<histName.Data()<<" not found!"<<std::endl;
      continue;
    }
    double cMean = calculateMeanofLogHist(cHist);
    secondCG[ichan] = cMean;
    
    double slope = (secondVoltage[ichan]-firstVoltage[ichan])/(secondCG[ichan]-firstCG[ichan]);
    double targetHV = firstVoltage[ichan]+ (targetCG - firstCG[ichan])*slope;
    std::cout<<thisChan.Data()
        <<" "<<firstVoltage[ichan]<<" "<<firstCG[ichan]
        <<" "<<secondVoltage[ichan]<<" "<<secondCG[ichan]
        <<" "<<slope<<" "<<targetHV
        <<std::endl;

    if(targetHV>minVoltage[ichan]&&targetHV<maxVoltage[ichan])
    {
      cHV->SetParameterAsStringThatBeginsWith("Voltage",TString("")+=targetHV,
                                              "DetectorName",thisChan.Data());
    }else{
      errorStatus[ichan] = 1;
      std::cerr<<" Channel "<<thisChan.Data()<<" out of range "<<targetHV<<std::endl;
    }
  }

  // Cleanup
  for(int ichan = 0; ichan < chancount; ichan++)
  {
    delete detNames[ichan];
    detNames[ichan] = 0;
  }
}

void caliFromDirectory(TList* dirList)
{
  if(dirList == 0) return;

  TList* chanName = new TList();
  TList* graphList = new TList();

  std::cout<<"Analyzing "<<dirList->LastIndex()+1 <<" files."<<std::endl;

  // Lets build the list of channels and voltages from the Channel Table
  for( int idir = 0; idir <= dirList->LastIndex(); idir++)
  {
    TDirectory* thisDir = (TDirectory*) dirList->At(idir);
    thisDir->cd();
    std::cout<<"Analyzing "<<thisDir->GetName()<<std::endl;
    NGMSystemConfiguration* conf = 
      (NGMSystemConfiguration*)(thisDir->FindObjectAny("NGMSystemConfiguration"));
    NGMConfigurationTable* cChan = conf->GetChannelParameters();
    NGMConfigurationTable* cHV = conf->GetHVParameters();


    for(int ichan = 0; ichan < cChan->GetEntries(); ichan++)
    {

      if(idir == 0) 
      {
        chanName->AddLast(new TObjString(cChan->GetParValueS("DetectorName",ichan)));
        TGraph* thisChanHVGraph = new TGraph(dirList->LastIndex()+1);
        graphList->Add(thisChanHVGraph);
        thisChanHVGraph->SetName(cChan->GetParValueS("DetectorName",ichan));
        thisChanHVGraph->SetTitle(cChan->GetParValueS("DetectorName",ichan));
        gROOT->Add(thisChanHVGraph);
      }
 
      TGraph* thisGraph = (TGraph*) graphList->At(ichan);
      // Find HV for this channel
      TString tchanName = ((TObjString*)(chanName->At(ichan)))->GetString();

      double thisHV = 0.0;
      for(int ihv = 0; ihv < cHV->GetEntries(); ihv++)
      {
        if(tchanName == cHV->GetParValueS("DetectorName",ihv))
        {
          thisHV = cHV->GetParValueD("Voltage",ihv);
          break;
        }
      }


      // Find the Plot for this channel
      TString g2plotName = "NMON_g2prime_";
      g2plotName += tchanName;
      
      double logcg = 0.001;  // Choose non zero value so that it shows up on log plots

      TH1* hg2prime = (TH1*)(thisDir->FindObjectAny(g2plotName.Data()));
      if(hg2prime)
      {
        TH1* hg2primeLog = (TH1*) (hg2prime->Clone(g2plotName+"LOG"));
		hg2primeLog->Rebin(16);
        for(int ibin = 1; ibin <= hg2prime->GetNbinsX(); ibin++)
        {
          hg2primeLog->SetBinContent(ibin,log(hg2primeLog->GetBinContent(ibin)+1.0));
        }
        logcg = hg2primeLog->GetMean();
        //delete hg2primeLog;
      }

      thisGraph->SetPoint(idir,thisHV, logcg);
      if(ichan == 0)
      {
        std::cout<<ichan<<" "<<thisHV<<" "<<logcg<<std::endl;
      }
    }
  }

  // Now Loop over all graphs and display curves
  TCanvas* cHVScan = new TCanvas("cHVScan","cHVScan");
  cHVScan->SetBorderSize(1);
  cHVScan->Divide(10,12,0.0001,0.0001);
  for(int ichan =0; ichan <= graphList->LastIndex()+1; ichan++)
  {
    //cHVScan->cd(ichan+1)->SetLogy();
    cHVScan->cd(ichan+1);
    TGraph* thisGraph = (TGraph*) graphList->At(ichan);
    if(thisGraph)
    {
      thisGraph->SetMarkerStyle(21);
      thisGraph->SetLineColor(kBlack);
      thisGraph->Draw("ALP");
    }
  }

}

void CalCG()
{
  const char* fileList[] = {"HVScan900V20070824181458ngm.root",
                            "HVScan1000V20070824183600ngm.root",
                            "HVScan1100V20070824185707ngm.root",
                            "HVScan1200V20070824191816ngm.root",
                            "HVScan1299V20070824193923ngm.root"
                            };

  TList* tfileList = new TList();

  for(int ifile = 0; ifile < 5; ifile++)
  {
    TFile* tf = TFile::Open(fileList[ifile]);
    tfileList->Add(tf);
  }

  caliFromDirectory(tfileList);

}