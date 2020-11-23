void mergeSortWithCalibrator(const char* fname)
{
  //
  NGMPacketBufferIO* fin = new NGMPacketBufferIO("fin","fin");
  //NGMPacketSocketInput* pin = new NGMPacketSocketInput("pin","pin");
  //NGMPacketOutputFile* pout = new NGMPacketOutputFile("RawOutC","RawOutC");
  NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAna","NGMAna");
  NGMNeutronMonitor* nmon = new NGMNeutronMonitor("NMON","NMON");

  //NGMConstantFractionTiming* mCfd = new NGMConstantFractionTiming("CFD","CFD");
  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("merge","merge");
  NGMTimeMonitor* mTime = new NGMTimeMonitor("TIME","TIME");
  NGMHitOutputFile* mHitOut = new NGMHitOutputFile("HitOut","HitOut");

  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(1); // Keep things quiet
  //mTBuffer->SetPacketLimit(100);

  mHitOut->setBasePathVariable("");
  mHitOut->setBasePath("");

  
  fin->Add(ana);
  //pin->Add(pout);
  //pin->Add(ana);

  ana->Add(mCal);
  mCal->Add(mMerge);
  mCal->Add(nmon);
  mMerge->Add(mTime);
  mMerge->Add(mHitOut);
  
  //pin->initModules();
  //gROOT->Add(pin);
  //new TBrowser;
  //pin->openSocket();
  //pin->LaunchReceiveLoopThread();
  //pout->closeOutputFile();


  fin->initModules();
  fin->openInputFile(fname);
  fin->run();
  mHitOut->closeOutputFile();

}
