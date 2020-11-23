void NGMSISScanDACs()
{

  // Initialize the DAQ
  NGMSystem* sisSystem = NGMSystem::getSystem();

  NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAna","NGMAna");
  NGMNeutronMonitor* nmon = new NGMNeutronMonitor("NMON","NMON");
  NGMPacketOutputFile* mPout = new NGMPacketOutputFile("NGMRaw","NGMRaw");
  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");

  mPout->setBasePathVariable("RawOutputPath");
  sisSystem->Add(ana);
  sisSystem->Add(mPout);
  ana->Add(mCal);
  mCal->Add(nmon);

  sisSystem->initModules();

  gROOT->Add(sisSystem);

  sisSystem->InitializeSystem();

  // Setup for the scan
  char cbuff[1024];

  int nscans = 5;
  int baselinemin = 35000;
  int baselinemax = 45000;
  double interval = (baselinemax - baselinemin)/(double)nscans;

  double runTime = 10; //seconds

  int maxChannels = NGMSystem::getSystem()->GetConfiguration()->GetChannelParameters()->GetEntries();

  NGMConfigurationTable* sysConf = NGMSystem::getSystem()->GetConfiguration()->GetSystemParameters();
  NGMConfigurationTable* slotConf = NGMSystem::getSystem()->GetConfiguration()->GetSlotParameters();
  NGMConfigurationTable* chanConf = NGMSystem::getSystem()->GetConfiguration()->GetChannelParameters();
  NGMConfigurationTable* detConf = NGMSystem::getSystem()->GetConfiguration()->GetDetectorParameters();

  sysConf->SetParameterS("Comment",0,"DAC HV");

  TH2* SIS_hDACScan_Baseline = new TH2F("SIS_hDACScan_Baseline","SIS_hDACScan_Baseline",
    nscans,baselinemin - interval/2.0, baselinemax + interval/2.0,
    maxChannels,0,maxChannels);

  TH2* SIS_hDACScan_BaselineRMS = new TH2F("SIS_hDACScan_BaselineRMS","SIS_hDACScan_BaselineRMS",
    nscans,baselinemin - interval/2.0, baselinemax + interval/2.0,
    maxChannels,0,maxChannels);

  TH2* SIS_hDACScan_BaselineNoise = new TH2F("SIS_hDACScan_BaselineNoise","SIS_hDACScan_BaselineNoise",
    nscans,baselinemin - interval/2.0, baselinemax + interval/2.0,
    maxChannels,0,maxChannels);

  // Setup relevant for DAC scan
  int prevReadoutRunMode = sysConf->GetParValueI("RunReadoutMode",0);
  sysConf->SetParameterI("RunReadoutMode",0,1);
  
  slotConf->SetParameterAsStringThatBeginsWith("RawDataSampleMode","1","ModEnableConf","");

  sysConf->SetParameterI("RunMaxSecondsCounter",0,runTime);
  sysConf->SetParameterI("RunCheckStopTimeFlag",0,1);

  for(int iscan = 0; iscan < nscans; iscan++)
  {
    int thisDAC = SIS_hDACScan_Baseline->GetXaxis()->GetBinCenter(iscan+1);
    TString thisDACstr;
    thisDACstr+=thisDAC;
    chanConf->SetParameterAsStringThatBeginsWith("DacOffset_Conf",thisDACstr.Data(),
                                                 "DetectorName","");

    // Take the data
    NGMSystem::getSystem()->StartAcquisition();
    
    // Now analyze

    // Find the histograms
    TProfile* baseline = (TProfile*) (gROOT->FindObjectAny("NMON_baseline"));
    TH1* baselineRMS = (TH1*) (gROOT->FindObjectAny("NMON_baselineRMS"));
    TH2* baselineSingleSample = (TH2*) (gROOT->FindObjectAny("NMON_singleSampleBaseline"));
    
    if(!baseline || ! baselineRMS || !baselineSingleSample) return;

    for(int ibin = 1; ibin <= SIS_hDACScan_Baseline->GetNbinsY(); ibin++)
    {
        SIS_hDACScan_Baseline->SetBinContent(iscan+1,ibin,baseline->GetBinContent(ibin));
        SIS_hDACScan_BaselineRMS->SetBinContent(iscan+1,ibin,baselineRMS->GetBinContent(ibin));
        TH1* tmp = baselineSingleSample->ProjectionX("_px",ibin,ibin);
        SIS_hDACScan_BaselineNoise->SetBinContent(iscan+1,ibin,tmp->GetRMS());
    }

    // Next Scan
  }

  TCanvas* tc = new TCanvas("cDACScan","DACScan");
  tc->Divide(1,3);
  gStyle->SetPalette(1);
  tc->cd(1);
  SIS_hDACScan_Baseline->Draw("colz");
  tc->cd(2);
  SIS_hDACScan_BaselineRMS->Draw("colz");
  tc->cd(3);
  SIS_hDACScan_BaselineNoise->Draw("colz");

}