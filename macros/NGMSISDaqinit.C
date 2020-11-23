void NGMSISDaqinit(const char* configFile = "NGMConfig.root")
{ 
// Shared for optimization
  //NGMBufferedPacket* outputBuffer = new NGMBufferedPacketv1;

  gStyle->SetPalette(1);

  NGMSystem* sisSystem = NGMSystem::getSystem();

  NGMAnalysisInput* ana = new NGMAnalysisInput("OnlAna","OnlAna");
  NGMNeutronMonitor* nmon = new NGMNeutronMonitor("NMON","NMON");
  NGMTimeMonitor* mTime = new NGMTimeMonitor("TIME","TIME");
  //NGMHitOutputFile* mHitOut = new NGMHitOutputFile("HitOut","HitOut");
  //NGMPacketSocketOutput* mSocketOut = new NGMPacketSocketOutput("mSocket","mSocket");
  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");

  // MergeSort should follow Calibrator
  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("merge","merge");
  NGMCountDist* countdistshort = new NGMCountDist("CountDistShort","CountDistShort");
  NGMCountDist* countdistlong = new NGMCountDist("CountDistLong","CountDistLong");
  countdistlong->setGateInterval(1000.0);
  countdistshort->setGateInterval(10.0);
 
  // Merge assuming no overlap in packets
  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(1); // Keep things quiet
  
  NGMPacketOutputFile* mPout = new NGMPacketOutputFile("NGMRaw","NGMRaw");
  mPout->setBasePathVariable("RawOutputPath");
  //mHitOut->setBasePathVariable("RawOutputPath");

  //mPout->setPassThrough(true);

  NGMSimpleParticleIdent* partID = countdistshort->getParticleIdent();
  //partID->AddCut(1,-1.,0.); // gbgamma
  //partID->AddCut(2,-1.,0.); // gbmuon
  partID->AddCut(3,-1.,0.); // mbgamma
  partID->AddCut(4,-1.,0.); // mbmuon
  //partID->AddCut(5,-1.,0.); // lsgamma
  //partID->AddCut(6,-1.,0.); // lsneutron
  //partID->AddCut(7,-1.,0.); // lsmuon
  partID->AddCut(8,-1.,0.); // hettlid
  //partID->AddCut(9,-1.,0.); // heid

  NGMSimpleParticleIdent* partIDlong = countdistlong->getParticleIdent();
  //partIDlong->AddCut(1,-1.,0.); // gbgamma
  //partIDlong->AddCut(2,-1.,0.); // gbmuon
  partIDlong->AddCut(3,-1.,0.); // mbgamma
  partIDlong->AddCut(4,-1.,0.); // mbmuon
  //partIDlong->AddCut(5,-1.,0.); // lsgamma
  //partIDlong->AddCut(6,-1.,0.); // lsneutron
  //partIDlong->AddCut(7,-1.,0.); // lsmuon
  partIDlong->AddCut(8,-1.,0.); // hettlid
  //partIDlong->AddCut(9,-1.,0.); // heid


  //sisSystem->Add(mPout);

  // Optimization Hack
  //sisSystem->_outBuffer = outputBuffer;
  //mPout->setLocalBuffer(outputBuffer);

  //sisSystem->Add(mSocketOut);
  //sisSystem->Add(mTBuffer);
  //mTBuffer->Add(ana);
  
  sisSystem->Add(ana);
  sisSystem->Add(mPout);
  ana->Add(mCal);
  mCal->Add(nmon);
  //ana->Add(mPout);
  //mCal->Add(mMerge);
  //mMerge->Add(countdistshort);
  //mMerge->Add(countdistlong);
  //mMerge->Add(mTime);
 // mMerge->Add(mHitOut);

  /*
  sisSystem->Add(mCfd);
  mCfd->Add(mMerge);
  mMerge->Add(mTime);
  //mMerge->Add(mHitOut);
  sisSystem->Add(nmon);
  sisSystem->Add(ana);
  */
  sisSystem->initModules();
  
  //countdist->setGateInterval(10.0);
  gROOT->Add(sisSystem);

  new TBrowser("NGMDaq",sisSystem,"NGMDaq");

  //mSocketOut->openSocket("rasputin3.llnl.gov");
  //mSocketOut->openSocket("localhost");

  sisSystem->InitializeSystem();
  if(TString(configFile!="")) sisSystem->readConfigFile(configFile);
  sisSystem->ConfigureSystem();
  sisSystem->LaunchGui();


}