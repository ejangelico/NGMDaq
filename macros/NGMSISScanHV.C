void NGMSISScanHV()
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

  int nscans = 6;
  double baselinemin = 0.9;
  double baselinemax = 1.4;
  double interval = (baselinemax - baselinemin)/(double)(nscans-1);

  double runTime = 60*20; //seconds

  int maxChannels = NGMSystem::getSystem()->GetConfiguration()->GetChannelParameters()->GetEntries();

  NGMConfigurationTable* sysConf = NGMSystem::getSystem()->GetConfiguration()->GetSystemParameters();
  NGMConfigurationTable* slotConf = NGMSystem::getSystem()->GetConfiguration()->GetSlotParameters();
  NGMConfigurationTable* chanConf = NGMSystem::getSystem()->GetConfiguration()->GetChannelParameters();
  NGMConfigurationTable* detConf = NGMSystem::getSystem()->GetConfiguration()->GetDetectorParameters();
  NGMConfigurationTable* hvConf = NGMSystem::getSystem()->GetConfiguration()->GetHVParameters();


  // Setup relevant for DAC scan
  int prevReadoutRunMode = sysConf->GetParValueI("RunReadoutMode",0);
  sysConf->SetParameterI("RunReadoutMode",0,1);
  
  sysConf->SetParameterS("Comment",0,"Scan HV");

  slotConf->SetParameterAsStringThatBeginsWith("RawDataSampleMode","1","ModEnableConf","");

  sysConf->SetParameterI("RunMaxSecondsCounter",0,runTime);
  sysConf->SetParameterI("RunCheckStopTimeFlag",0,1);
  sysConf->SetParameterS("UVHV_ACTIVE",0,"HVON");

  for(int iscan = 0; iscan < nscans; iscan++)
  {
    double thisHV = baselinemin + iscan*interval;
    sprintf(cbuff,"HVScan%dV",(int)(thisHV*1000.0));
    ana->SetName(cbuff);
   
    sprintf(cbuff,"%f",thisHV);
    hvConf->SetParameterAsStringThatBeginsWith("Voltage",cbuff,
                                                 "DetectorName","GS");
    hvConf->SetParameterAsStringThatBeginsWith("Voltage",cbuff,
                                                 "DetectorName","GL");
    hvConf->SetParameterAsStringThatBeginsWith("Voltage",cbuff,
                                                 "DetectorName","MU");

    sprintf(cbuff,"%f",thisHV*1000.0);
    hvConf->SetParameterAsStringThatBeginsWith("Voltage",cbuff,
                                                 "DetectorName","LS");
   
    
    sisSystem->ConfigureSystem();
    // Wait for HV to Adjust
    TThread::Sleep(20);
    // Take the data
    sisSystem->StartAcquisition();
    
    // Next Scan
  }

}