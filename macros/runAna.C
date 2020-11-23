void runAna(const char* fname=0, int maxSpills = 0,
	    int deadTimeNS=0, int burstSize=0, int gateInterval=0, int doTimeMon=0,
	    float waterFallInterval=0, int pulseHight=3000, int burstWindow=100, int burstGraphWindow=0, int countPID=1, int waterFallPoints=1000, int simulation=0, int GrandCanyonAnalysis=0)
{
  //Lets print some info
  if(!fname ||(deadTimeNS==0 && burstSize==0 && gateInterval==0 && doTimeMon==0 && waterFallInterval==0)){
    cout<<"The arguments are:"<<endl;
    cout<<"fname : The RAW data file to be analyzed."<<endl<<endl;
    cout<<"maxSpills : The number of spills to be looped over."<<endl;
    cout<<"            (0 means loop over entire file.)"<<endl<<endl;
    cout<<"deadTimeNS : The amount of deatime (ns) to impose after large pulse.See <pulseHight>."<<endl;
    cout<<"             (If 0 the dead time module is disabled.)"<<endl<<endl;
    cout<<"burstSize : The number of counts in <burstWindow> time required to make a burst. Particle selection requires macro changes."<<endl;
    cout<<"            (If 0 the burst monitor is disabled.)"<<endl<<endl;
    cout<<"gateInterval : The \"random\" time gate interval (ns) for the count distribution module."<<endl;
    cout<<"               (If 0 the count distribution module is disabled.)"<<endl<<endl;
    cout<<"doTimeMon : 1 enables the time monitor. 0 disables the time monitor"<<endl<<endl;
    cout<<"waterFallInterval : Time (s) between display updates of the waterfall graph. Doesn't change amount of data analyzed"<<endl;
    cout<<"                    (If 0 the waterfall module is disabled.)"<<endl<<endl;
    cout<<"pulseHight : The pulse height (in ADCs?) above which deadTimeNS dead time is imposed on the channel."<<endl<<endl;
    cout<<"burstWindow : The time window (ns) in which to look for <burstSize> hits to make a burst."<<endl<<endl;
    cout<<"burstGraphWindow : The time window (ns) to display in the burst monitor step and waterfall plots."<<endl;
    cout<<"                   (0 means 10*<burstWindow>)"<<endl<<endl;
    cout<<"countPID : The particle type to analyze in the count distribution module. See macro comments for values"<<endl<<endl;
    cout<<"waterFallPoints : The maximum number of points to display in the waterfall module."<<endl<<endl;
    cout<<"                  Once the maximum is reached, the earliest hit is removed before the most recent is added."<<endl<<endl;
    cout<<"simulation : 1 if we are reading in a simulation file."<<endl<<endl;
    cout<<"GrandCanyonAnalysis : 1 if we want to perform a Grand Canyon Analysis at the end of the run."<<endl<<endl;
    return;
}

  if(burstGraphWindow==0) burstGraphWindow = 10*burstWindow;

  gROOT->SetStyle("Plain");
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
  //You can also use NGMSimpleParticleIdent::gbmuon and the like

  if(simulation == 1) {
    NGMHitIO* fin = new NGMHitIO("fin","fin");
  } else {
    NGMPacketBufferIO* fin = new NGMPacketBufferIO("fin","fin");
    NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
    NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("mMerge","mMerge");
  }
  NGMAnaGui* mAnaGui = 0;
  if (!gROOT->IsBatch()) mAnaGui = new NGMAnaGui("mAnaGui","mAnaGui");
  NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAna","NGMAna");
  
  
  NGMDeadTimeModule* deadTime = 0;
  if(deadTimeNS){
    deadTime = new NGMDeadTimeModule("DEAD","DEAD");
    deadTime->getParticleIdent()->AddCut(1,-1.,0.); //BAD NAME, MY (AMG) FAULT. THINK PARTICLE SELECTION
    deadTime->setDeadTimeAll(deadTimeNS);
    deadTime->setPulseHeightThresh(pulseHight);
    deadTime->setDeadTime(14,-1);
    deadTime->setDeadTime(10,-1);
    deadTime->setDeadTime(9,-1);
    deadTime->setDeadTime(4,-1);
  }
  
  NGMTimeMonitor* mTime = 0;
  if(doTimeMon){
    mTime = new NGMTimeMonitor("TIME","TIME");
    mTime->addParticleSelection(1,-1,0); //Does not use standard getParticleIdent for now
  }
  
  NGMCountDist* countdist = 0;
  if(gateInterval){
//    countdist = new NGMCorrelatedCountDist("CountDist","CountDist");
    countdist = new NGMCountDist("CountDist","CountDist");
    countdist->setGateInterval(gateInterval);
    countdist->getParticleIdent()->AddCut(countPID,-1.,0.);
    countdist->SetDisplayInterval(10.);

    if (countPID == 8) {
      countdist->setPromptStepForY2FandZ2F(false);
    } else {
      countdist->setPromptStepForY2FandZ2F(true);
    }
    countdist->setAllGraphs(true);
    countdist->useFastNeutronEquation(false);
    if (GrandCanyonAnalysis == 1) {
      char sorc = 'u';
      int ind = 5;
      NGMGrandCanyonGenerator* GrandCanyonGenerator = new NGMGrandCanyonGenerator("GrandCanyonGenerator", "GrandCanyonGenerator");
      GrandCanyonGenerator->setSpontFissIsotope(sorc);
      GrandCanyonGenerator->setMultiplyingIsotope(ind);
      GrandCanyonGenerator->setMrange(5., 15.);
      GrandCanyonGenerator->setMintervals(20);
      GrandCanyonGenerator->seteffrange(.01, .10);
      GrandCanyonGenerator->seteffintervals(18);
      GrandCanyonGenerator->setRextrange(.05, 8.);
      GrandCanyonGenerator->setRextintervals(30);
      GrandCanyonGenerator->setMinBnTerms(10);

      NGMCountDistGenerator* CountDistGenerator = new NGMCountDistGenerator();
      CountDistGenerator->setChecks(true,.2);
      CountDistGenerator->setAutoTermsBn(true);
      countdist->setGrandCanyonAnalysis(GrandCanyonGenerator, CountDistGenerator);
    }
  }

  NGMHitOutputASCIIFile* ascii = new NGMHitOutputASCIIFile("ascii","ascii");
  ascii->getParticleIdent()->AddCut(6,-1.,0.); // lsneutron
  ascii->getParticleIdent()->AddCut(8,-1.,0.); // hettlid

  NGMBurstMonitor* mBurst = 0;
  if(burstSize){
    mBurst = new NGMBurstMonitor("BURST","BURST");
    burstFall = new NGMBurstWaterfall("BURSTFALL","BURSTFALL");
    mBurst->setPushFullWindow(false);
    mBurst->Add(burstFall);
    mBurst->setNumberZoomedGraphs(50);
    mBurst->setHitsInBurst(burstSize);
    mBurst->setBurstWindowNS(burstWindow);
    mBurst->setGraphWindowNS(burstGraphWindow);
    mBurst->getParticleIdent()->AddCut(1,200.,1e10);
//    mBurst->getParticleIdent()->AddCut(2,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(3,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(4,-1.,0.);
    mBurst->getParticleIdent()->AddCut(5,200.,1e10);
    mBurst->getParticleIdent()->AddCut(6,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(7,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(8,-1.,0.);
//    mBurst->getParticleIdent()->AddCut(9,-1.,0.);
//     mBurst->AddRequirement(3,-1);
    mBurst->AddRequirement(6,4);
//     mBurst->AddRequirement(6,3);
  }

  //NGMWaterfall* waterfall = 0;
  if(waterFallInterval){
    waterfall = new NGMWaterfall("WATERFALL","WATERFALL");
    waterfall->SetDisplayInterval(waterFallInterval);
    waterfall->SetNumberOfPointsDisplayed(waterFallPoints);
//    waterfall->getParticleIdent()->AddCut(1,500.,1e10);
//    waterfall->getParticleIdent()->AddCut(2,-1.,0.);
//    waterfall->getParticleIdent()->AddCut(3,2500.0,1E10);
//    waterfall->getParticleIdent()->AddCut(4,-1.,0.);
//    waterfall->getParticleIdent()->AddCut(5,-1.,0.);
//    waterfall->getParticleIdent()->AddCut(6,-1.,0.);
//    waterfall->getParticleIdent()->AddCut(7,-1.,0.);
    waterfall->getParticleIdent()->AddCut(8,-1.,0.);
//    waterfall->getParticleIdent()->AddCut(9,-1.,0.);
  }

  if (simulation == 1) {
    fin->Add(ana);
  } else {
    fin->Add(mCal);
    if(mAnaGui){
      mCal->Add(mAnaGui);
      mAnaGui->Add(mMerge);
    }else{
      mCal->Add(mMerge);
    }
    mMerge->Add(ana);
  }
  
  if (simulation == 0) {
    // Select file to use for neutron-gamma cuts
//  mCal->SetROIFileName("ROIs.root"); //"/usr/gapps/ngm/data/run7/ROIs5sigLS.root");
    mCal->SetROIFileName("/p/lscratcha/verbeke2/ROIs2007Dec20.root");
//  mCal->SetROIFileName("/usr/gapps/ngm/data/run7/ROIs5sigLS.root");
    // Assume no packets cross boundary
    // Not a perfect assumption, could use a different mode of merge
    // that would set required number of packets based on active HV???
    mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
    mMerge->setVerbosity(1);
  }

  if(deadTime){
    ana->Add(deadTime);
    if (mTime) deadTime->Add(mTime);
    if (countdist) deadTime->Add(countdist);
    if (mBurst) deadTime->Add(mBurst);
    if (waterfall) deadTime->Add(waterfall);
    deadTime->Add(ascii);
  } else {
     if (mTime) ana->Add(mTime);
     if (countdist) ana->Add(countdist);
     if (mBurst) ana->Add(mBurst);
     if (waterFallInterval) ana->Add(waterfall);
   }

  new TBrowser;

  gROOT->Add(fin);
  fin->initModules();
  fin->openInputFile(fname);
  long spills = 0;
  if(maxSpills==0) {
    while(fin->readNextSpill()) {spills++;}
  } else {
    while(spills<maxSpills && fin->readNextSpill()) {
      if(spills%50 == 0) gSystem->ProcessEvents();
      cout<<"At spill "<<spills<<endl;
      spills++;
    }
  }

  // Send end of analysis signals to all modules.
  TObjString endRunFlush("EndRunFlush");
  TObjString endRunSave("EndRunSave");
  fin->push(*((const TObject*)&endRunFlush));
  fin->push(*((const TObject*)&endRunSave));
}
