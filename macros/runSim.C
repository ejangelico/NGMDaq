NGMWaterfall* waterFall = 0;
void runSim(const char* fname=0, int maxBuffers = 0,
	    int deadTimeNS=0, int burstSize=0, int gateInterval=0, int doTimeMon=0,
	    float waterFallInterval=0, int pulseHight=3000, int burstWindow=100, int burstGraphWindow=0, int countPID=1, int waterFallPoints=1000)
{
  //Lets print some info
  if(!fname ||(deadTimeNS==0 && burstSize==0 && gateInterval==0 && doTimeMon==0 && waterFallInterval==0)){
    cout<<"The arguments are:"<<endl;
    cout<<"fname : The RAW data file to be analyzed."<<endl<<endl;
    cout<<"maxBuffers : The number of buffers to be looped over. Beware the partial spill effect."<<endl;
    cout<<"             (0 means loop over entire file.)"<<endl<<endl;
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
    return;
}

  if(burstGraphWindow==0) burstGraphWindow = 10*burstWindow;
  if(maxBuffers==0) maxBuffers = 1E9;

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

  NGMHitIO* fin = new NGMHitIO("fin","fin");
  NGMAnaGui* mAnaGui = new NGMAnaGui("mAnaGui","mAnaGui");
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
    countdist = new NGMCountDist("CountDist","CountDist");
    countdist->setGateInterval(gateInterval);
    countdist->setAllGraphs(true);
    countdist->getParticleIdent()->AddCut(6,-1.,0.);
    countdist->getParticleIdent()->AddCut(8,-1.,0.);
//    countdist->getParticleIdent()->AddCut(countPID,4000.,5500.);
  }

  NGMHitOutputASCIIFile* ascii = new NGMHitOutputASCIIFile("ascii","ascii");
  ascii->getParticleIdent()->AddCut(6,-1.,0.); // lsneutron
  ascii->getParticleIdent()->AddCut(8,-1.,0.); // hettlid

  NGMBurstMonitor* mBurst = 0;
  if(burstSize){
    mBurst = new NGMBurstMonitor("BURST","BURST");
    mBurst->setHitsInBurst(burstSize);
    mBurst->setBurstWindowNS(burstWindow);
    mBurst->setGraphWindowNS(burstGraphWindow);
//    mBurst->getParticleIdent()->AddCut(1,0.,5500.);
//     mBurst->getParticleIdent()->AddCut(2,-1.,0.);
//     mBurst->getParticleIdent()->AddCut(3,3000.0,1E10);
//     mBurst->getParticleIdent()->AddCut(4,-1.,0.);
//     mBurst->getParticleIdent()->AddCut(5,3000.0,1E10);
     mBurst->getParticleIdent()->AddCut(6,-1.,0.);
//     mBurst->getParticleIdent()->AddCut(7,-1.,0.);
     mBurst->getParticleIdent()->AddCut(8,-1.,0.);
//     mBurst->getParticleIdent()->AddCut(9,-1.,0.);
//     mBurst->AddRequirement(3,-1);
//     mBurst->AddRequirement(8,2);
//     mBurst->AddRequirement(6,3);
  }

  //NGMWaterfall* waterFall = 0;
  if(waterFallInterval){
    waterFall = new NGMWaterfall("WATERFALL","WATERFALL");
    waterFall->SetDisplayInterval(waterFallInterval);
    waterFall->SetNumberOfPointsDisplayed(waterFallPoints);
//     waterFall->getParticleIdent()->AddCut(1,4000.,5500.);
//     waterFall->getParticleIdent()->AddCut(2,-1.,0.);
//     waterFall->getParticleIdent()->AddCut(3,2500.0,1E10);
//     waterFall->getParticleIdent()->AddCut(4,-1.,0.);
//     waterFall->getParticleIdent()->AddCut(5,2500.0,1E10);
     waterFall->getParticleIdent()->AddCut(6,-1.,0.);
//     waterFall->getParticleIdent()->AddCut(7,-1.,0.);
     waterFall->getParticleIdent()->AddCut(8,-1.,0.);
//     waterFall->getParticleIdent()->AddCut(9,-1.,0.);
  }

  fin->Add(mAnaGui);
  mAnaGui->Add(ana);
  
  if(deadTime){
    ana->Add(deadTime);
    if (mTime) deadTime->Add(mTime);
    if (countdist) deadTime->Add(countdist);
    if (mBurst) deadTime->Add(mBurst);
    if (waterFall) deadTime->Add(waterFall);
    deadTime->Add(ascii);
  } else {
     if (mTime) ana->Add(mTime);
     if (countdist) ana->Add(countdist);
     if (mBurst) ana->Add(mBurst);
     if (waterFall) ana->Add(waterFall);
   }

  new TBrowser;

  gROOT->Add(fin);
  fin->initModules();
  fin->openInputFile(fname);
  int nbuffers = 0;
  while(fin->readNextBuffer() && nbuffers<maxBuffers)
//  while(fin->readNextSpill() && nbuffers<maxBuffers)
    {
      if(nbuffers%50 == 0) gSystem->ProcessEvents();
      cout<<"At buffer "<<nbuffers<<endl;
      nbuffers++;
    }

//       fin->run();

  //fin->closeInputFile();
  //fin->openInputFile("NGMRaw20070901011614-ngm_1.root");
//   nbuffers = 0;
//   while(fin->readNextBuffer() && nbuffers<48)
//     {
//       if(nbuffers%50 == 0) gSystem->ProcessEvents();
//       cout<<"At buffer "<<i<<endl;
//       nbuffers++;
//     }
  //     fin->run();


    // Send end of analysis signals to all modules.
    TObjString endRunFlush("EndRunFlush");
    TObjString endRunSave("EndRunSave");
    fin->push(*((const TObject*)&endRunFlush));
    fin->push(*((const TObject*)&endRunSave));
}
