void NGMSISDaqinit(const char* configFile = "NGMConfig2007Dec20.root")
{ 
// Shared for optimization
  //NGMBufferedPacket* outputBuffer = new NGMBufferedPacketv1;
 //Standardish values
  //deadTimeNS=4000
  //burstSize=10
  //waterFallInterval=5
  //Particle Cuts:
  //See NGMParticleIdent.h
  // gbgamma=1
  // gbmuon=2
  // mbgamma=3
  // mbmuon=4
  // lsgamma=5
  // lsneutron=6
  // lsmuon=7
  // hettlid=8
  // heid=9

  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);

  NGMSystem* sisSystem = NGMSystem::getSystem();

  NGMAnalysisInput* ana = new NGMAnalysisInput("OnlAna","OnlAna");
  NGMNeutronMonitor* nmon = new NGMNeutronMonitor("NMON","NMON");
  NGMTimeMonitor* mTime = new NGMTimeMonitor("TIME","TIME");
  //NGMHitOutputFile* mHitOut = new NGMHitOutputFile("HitOut","HitOut");
  //NGMPacketSocketOutput* mSocketOut = new NGMPacketSocketOutput("mSocket","mSocket");
  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  NGMRawDisplay* RawD = new NGMRawDisplay("RawD","RawD");
  
  NGMBurstMonitor* mBurst = 0;
  int burstSize = 2;
  double burstWindow = 100.0;
  double burstGraphWindow = 10000.0;
  if(burstSize){
    mBurst = new NGMBurstMonitor("BURST","BURST");
    //burstFall = new NGMBurstWaterfall("BURSTFALL","BURSTFALL");
    mBurst->setPushFullWindow(true);
    //mBurst->Add(burstFall);
    mBurst->setHitsInBurst(burstSize);
    mBurst->setBurstWindowNS(burstWindow);
    mBurst->setGraphWindowNS(burstGraphWindow);
//   mBurst->getParticleIdent()->AddCut(1,500.,1e10);
//    mBurst->getParticleIdent()->AddCut(2,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(3,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(4,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(5,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(6,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(7,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(8,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(9,-1.,0.);
//     mBurst->AddRequirement(3,-1);
    mBurst->AddRequirement(6,2);
//     mBurst->AddRequirement(6,3);
  }
  double waterFallInterval = 6000000.0;
  int waterFallPoints = 10000;
  NGMWaterfall* waterfall = 0;
  if(waterFallInterval){
    waterfall = new NGMWaterfall("WATERFALL","WATERFALL");
    waterfall->SetDisplayInterval(waterFallInterval);
    waterfall->SetNumberOfPointsDisplayed(waterFallPoints);
//     waterfall->getParticleIdent()->AddCut(1,0.,550.);
//     waterfall->getParticleIdent()->AddCut(2,-1.,0.);
//     waterfall->getParticleIdent()->AddCut(3,2500.0,1E10);
//     waterfall->getParticleIdent()->AddCut(4,-1.,0.);
//     waterfall->getParticleIdent()->AddCut(5,7000.0,1E10);
     waterfall->getParticleIdent()->AddCut(6,-1.,0.);
//     waterfall->getParticleIdent()->AddCut(7,-1.,0.);
//     waterfall->getParticleIdent()->AddCut(8,-1.,0.);
//     waterfall->getParticleIdent()->AddCut(9,-1.,0.);
  }

    NGMWaterfall* waterfall2 = 0;
  if(waterFallInterval){
    waterfall2 = new NGMWaterfall("WATERFALL2","WATERFALL2");
    waterfall2->SetDisplayInterval(waterFallInterval);
    waterfall2->SetNumberOfPointsDisplayed(waterFallPoints);
//     waterfall2->getParticleIdent()->AddCut(1,0.,550.);
//     waterfall2->getParticleIdent()->AddCut(2,-1.,0.);
//     waterfall2->getParticleIdent()->AddCut(3,2500.0,1E10);
//     waterfall2->getParticleIdent()->AddCut(4,-1.,0.);
//     waterfall2->getParticleIdent()->AddCut(5,7000.0,1E10);
     waterfall2->getParticleIdent()->AddCut(6,-1.,0.);
//     waterfall2->getParticleIdent()->AddCut(7,-1.,0.);
     waterfall2->getParticleIdent()->AddCut(8,-1.,0.);
//     waterfall2->getParticleIdent()->AddCut(9,-1.,0.);
  }

   NGMWaterfall* waterfall3 = 0;
  if(waterFallInterval){
    waterfall3 = new NGMWaterfall("WATERFALL3","WATERFALL3");
    waterfall3->SetDisplayInterval(waterFallInterval);
    waterfall3->SetNumberOfPointsDisplayed(waterFallPoints);
//     waterfall3->getParticleIdent()->AddCut(1,0.,550.);
//     waterfall3->getParticleIdent()->AddCut(2,-1.,0.);
//     waterfall3->getParticleIdent()->AddCut(3,2500.0,1E10);
//     waterfall3->getParticleIdent()->AddCut(4,-1.,0.);
//     waterfall3->getParticleIdent()->AddCut(5,7000.0,1E10);
//     waterfall3->getParticleIdent()->AddCut(6,-1.,0.);
//     waterfall3->getParticleIdent()->AddCut(7,-1.,0.);
//     waterfall3->getParticleIdent()->AddCut(8,-1.,0.);
//     waterfall3->getParticleIdent()->AddCut(9,-1.,0.);
  }

   NGMWaterfall* waterfall4 = 0;
  if(waterFallInterval){
    waterfall4 = new NGMWaterfall("WATERFALL4","WATERFALL4");
    waterfall4->SetDisplayInterval(waterFallInterval);
    waterfall4->SetNumberOfPointsDisplayed(waterFallPoints);
//     waterfall4->getParticleIdent()->AddCut(1,0.,550.);
//     waterfall4->getParticleIdent()->AddCut(2,-1.,0.);
     waterfall4->getParticleIdent()->AddCut(3,2500.0,1E10);
     waterfall4->getParticleIdent()->AddCut(4,-1.,0.);
//     waterfall4->getParticleIdent()->AddCut(5,7000.0,1E10);
//     waterfall4->getParticleIdent()->AddCut(6,-1.,0.);
//     waterfall4->getParticleIdent()->AddCut(7,-1.,0.);
//     waterfall4->getParticleIdent()->AddCut(8,-1.,0.);
//     waterfall4->getParticleIdent()->AddCut(9,-1.,0.);
  }
 
  // MergeSort should follow Calibrator
  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("merge","merge");

  NGMExtTrigger* ddTrigger = new NGMExtTrigger("ddTrigger","ddTrigger");
  ddTrigger->setGeneratorIndex(48);

  // Merge assuming no overlap in packets
  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(1); // Keep things quiet
  
  NGMPacketOutputFile* mPout = new NGMPacketOutputFile("NGMRaw","NGMRaw");
  mPout->setBasePathVariable("RawOutputPath");

  NGMTimeSelection* timeWindow1 = new NGMTimeSelection("timeWindow1","timeWindow1");  
  NGMTimeSelection* timeWindow2 = new NGMTimeSelection("timeWindow2","timeWindow2");
  timeWindow1->setTimeSelection(0.0,9.0E4);
  timeWindow2->setTimeSelection(9.0E4,6E6);
  
  NGMNeutronMonitor* nmon1 = new NGMNeutronMonitor("NMONBeam","NMONBeam");
  NGMNeutronMonitor* nmon2 = new NGMNeutronMonitor("NMONBurst","NMONBurst");

  sisSystem->Add(ana);
  sisSystem->Add(mPout);
  sisSystem->Add(RawD);
  ana->Add(mCal);
  mCal->Add(nmon);
  //ana->Add(mPout);
  mCal->Add(mMerge);
  if(mBurst)
  {
    mMerge->Add(mBurst);
    mBurst->setDrawGraphics(false);
    NGMHitSerialize* hitSerial = new NGMHitSerialize("hitSer","hitSer");
    mBurst->Add(hitSerial);
    NGMWaterfall* burstWaterFall = new NGMWaterfall("BurstWaterfall","BurstWaterfall");
    hitSerial->Add(burstWaterFall);
    hitSerial->Add(nmon2);
  }
  if (waterfall)
    mMerge->Add(waterfall);
  if (waterfall2)
    mMerge->Add(waterfall2);
  if (waterfall3)
    mMerge->Add(waterfall3);
  if (waterfall4)
    mMerge->Add(waterfall4);
 
  sisSystem->initModules();
  
  // Only set modules inactive after init
  RawD->SetActive(false);
  //ddTrigger->SetActive(kFALSE);

  mCal->SetROIFileName("ROIs2007Dec20.root");

  //countdist->setGateInterval(10.0);
  gROOT->Add(sisSystem);

  new TBrowser("NGMDaq",sisSystem,"NGMDaq");

  //mSocketOut->openSocket("rasputin3.llnl.gov");
  //mSocketOut->openSocket("localhost");

  if(TString(configFile!="")) sisSystem->readConfigFile(configFile);
  sisSystem->InitializeSystem();
  sisSystem->ConfigureSystem();
  sisSystem->LaunchGui();


}