void read60keVRaw(const char* fname = "", int maxspills = 0)
{

  int fromFile = 0;
  if(TString(fname)!="") fromFile =1;

  //
  NGMPacketBufferIO* fin = new NGMPacketBufferIO("fin","fin");
  NGMPacketSocketInput* pin = new NGMPacketSocketInput("pin","pin");
  NGMPacketOutputFile* pout = new NGMPacketOutputFile("RawOutC","RawOutC");
  NGMAnalysisInput* ana = new NGMAnalysisInput("OffAna","OffAna");
  NGMNeutronMonitor* nmon = new NGMNeutronMonitor("NMON","NMON");
  NGMNeutronMonitor* nmon1 = new NGMNeutronMonitor("NMONBeam","NMONBeam");
  NGMNeutronMonitor* nmon2 = new NGMNeutronMonitor("NMONPostBeam","NMONPostBeam");
  NGMPacketOutputFile* burstOut = new NGMPacketOutputFile("burstOut","burstOut");
  
  NGMExtTrigger* FNeutronBurst = new NGMExtTrigger("FNeutronBurst","FNeutronBurst");
  NGMTimeSelection* timeWindow1 = new NGMTimeSelection("timeWindow1","timeWindow1");  
  NGMTimeSelection* timeWindow2 = new NGMTimeSelection("timeWindow2","timeWindow2");
  timeWindow1->setTimeSelection(0.0,1.5E5);
  timeWindow2->setTimeSelection(1.5E5,6E6);
  
  FNeutronBurst->setGeneratorIndex(48);
  
  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  mCal->SetROIFileName("ROIs20071206.root");
  
  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("merge","merge");
  NGMHitOutputFile* mHitOut = new NGMHitOutputFile("HitOut","HitOut");

  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(1); // Keep things quiet
  //mTBuffer->SetPacketLimit(100);
  //fin->setDebug();
  pout->setBasePathVariable("RawOutputPath");
  pout->setBasePathVariable("");
  pout->setBasePath("");

 // mHitOut->setBasePathVariable("RawOutputPath");
  mHitOut->setBasePathVariable("");
  mHitOut->setBasePath("./");
//  mHitOut->setBasePathVariable("");
  //mHitOut->setBasePath("./");
  burstOut->setBasePathVariable("");
  burstOut->setBasePath("./");
  
  if(fromFile)
    fin->Add(ana);
//  else
    pin->Add(ana);

  ana->Add(mCal);
  mCal->setConstantFraction(0.06);
  mCal->Add(nmon);
  mCal->Add(mMerge);
  //mMerge->Add(gammaBurst);
  mMerge->Add(FNeutronBurst);
  //FNeutronBurst->Add(burstOut);
  FNeutronBurst->Add(timeWindow1);
  FNeutronBurst->Add(timeWindow2);
  timeWindow1->Add(nmon1);
  timeWindow2->Add(nmon2);
  
  //mMerge->Add(mTime);
  //mMerge->Add(mHitOut);
//  ana->Add(mMerge);  
  
  new TBrowser;
  
  if(!fromFile)
  {
    pin->initModules();
    gROOT->Add(pin);
    pin->openSocket();
    pin->LaunchReceiveLoopThread();
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
	    if(maxspills>0 && (nbuffers+1)>=maxspills) break;
      nbuffers++;
    }
    TObjString endRunFlush("EndRunFlush");
    TObjString endRunSave("EndRunSave");
    fin->push(*((const TObject*)&endRunFlush));
    fin->push(*((const TObject*)&endRunSave));
  }


}
