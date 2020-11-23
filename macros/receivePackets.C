void receivePackets( const char* fname = "")
{
  
  int fromFile = 0;
  if(TString(fname)!="") fromFile = 1;
  //
  NGMPacketBufferIO* fin = new NGMPacketBufferIO("fin","fin");
  NGMAnaGui* gui = new NGMAnaGui("cGui","cGui");
  NGMPacketSocketInput* pin = new NGMPacketSocketInput("pin","pin");
  NGMPacketOutputFile* pout = new NGMPacketOutputFile("RawOutC","RawOutC");
  NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAna","NGMAna");
  NGMNeutronMonitor* nmon = new NGMNeutronMonitor("NMON","NMON");

  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");

  mCal->SetROIFileName("ROIs60keV.root");

  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("merge","merge");
  NGMTimeMonitor* mTime = new NGMTimeMonitor("TIME","TIME");
  NGMBurstMonitor* ddTrigger = new NGMBurstMonitor("ddTrigger","ddTrigger");
    ddTrigger->setHitsInBurst(1);
    ddTrigger->setBurstWindowNS(100.0);
    ddTrigger->setGraphWindowNS(1E5);
    ddTrigger->AddRequirement(NGMSimpleParticleIdent::exttrig,1,10000);
  ddTrigger->setDrawGraphics(false);

  NGMHitOutputFile* mHitOut = new NGMHitOutputFile("HitOut","HitOut");

  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(1); // Keep things quiet
  //mTBuffer->SetPacketLimit(100);

  pout->setBasePathVariable("RawOutputPath");
  pout->setBasePathVariable("");
  pout->setBasePath("");

 // mHitOut->setBasePathVariable("RawOutputPath");
  mHitOut->setBasePathVariable("");
  mHitOut->setBasePath("./");
  mTime->setMaxNanoSecInList(1E3);
//  mHitOut->setBasePathVariable("");
  //mHitOut->setBasePath("./");
  mTime->setMaxNanoSecInList(1E3);
  
  if(fromFile){
    fin->Add(ana);
    //gui->Add(ana);
  }else{
    //pin->Add(ana);
  }
  ana->Add(mCal);
  mCal->setConstantFraction(0.06);
  mCal->Add(nmon);

  mCal->Add(mMerge);
  //mMerge->Add(ddTrigger);
  mMerge->Add(mTime);
  mTime->setMaxNanoSecInList(1000.0);
  //mMerge->Add(mHitOut);
   
  new TBrowser;
  
  if(!fromFile)
  {
    pin->initModules();
    gROOT->Add(pin);
    pin->openSocket();
    //pin->LaunchReceiveLoopThread();
    pin->ReceiveLoop();
  //pout->closeOutputFile();
  }else{
    gROOT->Add(fin);
    fin->initModules();
    fin->openInputFile(fname);
    int nbuffers = 0;
    while(fin->readNextSpill())
    {
      if(nbuffers%1 == 0)
         gSystem->ProcessEvents();
      if(nbuffers>0) break;
      nbuffers++;
    }
    TObjString endRunFlush("EndRunFlush");
    TObjString endRunSave("EndRunSave");
    fin->push(*((const TObject*)&endRunFlush));
    fin->push(*((const TObject*)&endRunSave));
  }


}
