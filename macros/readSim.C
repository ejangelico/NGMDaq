void readSim(const char* fname = "MPI_List_timeout_n_p_np_lsg.200keV_truncated")
{
  // This routine reads a simulation file fname. It needs some initialization
  // parameters in file 'MPI_List_timeout_n_p_np.init'
  //
  // fname: the name of the simulation file
  //

  NGMCOGSim* sim = new NGMCOGSim("sim","sim");
  NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAna","NGMAna");
  NGMCountDistR* countdist = new NGMCountDistR("CountDist","CountDist");
//  NGMWaterfall* waterfall = new NGMWaterfall("Waterfall","Waterfall");
//  NGMBurstMonitor* mBurst = new NGMBurstMonitor("BURST","BURST");

  sim->Add(ana);
  ana->Add(countdist);
//  ana->Add(waterfall);
//  ana->Add(mBurst);

//  waterfall->getParticleIdent()->AddCut(1,-1.,0.);
//  waterfall->getParticleIdent()->AddCut(5,-1.,0.);
//  waterfall->getParticleIdent()->AddCut(8,-1.,0.);
//  waterfall->SetNumberOfPointsDisplayed(100000);
//  waterfall->SetDisplayInterval(100);

  countdist->getParticleIdent()->AddCut(8,-1.,0.);
  countdist->setGateInterval(1.0);
  countdist->setAllGraphs(false);
  countdist->SetDisplayInterval(1E10);
//  sim->getParticleIdent()->AddCut(6,-1.,0.);


/*
    burstFall = new NGMBurstWaterfall("BURSTFALL","BURSTFALL");
    mBurst->setPushFullWindow(false);
    mBurst->Add(burstFall);
    mBurst->setNumberZoomedGraphs(50);
    mBurst->setHitsInBurst(10);
    mBurst->setBurstWindowNS(10000);
    mBurst->setGraphWindowNS(100000);
    mBurst->getParticleIdent()->AddCut(1,-1.,0);
    mBurst->getParticleIdent()->AddCut(5,-1.,0);
*/
//     mBurst->AddRequirement(6,3);


//  1 gbgamma
//  2 gbmuon
//  3 mbgamma
//  4 mbmuon
//  5 lsgamma
//  6 lsneutron
//  7 lsmuon
//  8 hettlid
//  9 heid


//  new TBrowser;

  gROOT->Add(sim);
  sim->initModules();
  sim->openInputFile(fname);
  int i = 0;
  while (i < 100) {
    sim->readEvents(2000);
    i++;
  }
  // Send end of analysis signals to all modules.
  TObjString endRunFlush("EndRunFlush");
  sim->push(*((const TObject*)&endRunFlush));
  TObjString endRunSave("EndRunSave");
  sim->push(*((const TObject*)&endRunSave));
}
