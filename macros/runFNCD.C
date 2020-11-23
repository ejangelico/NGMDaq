void runFNCD(const char* fname)
{
  gStyle->SetPalette(1); // Rainbow palette
  gROOT->SetStyle("Plain");

  //Standardish values
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

  bool globalUpdate = false;
  //Verify that we have not already created an analysis tree
  NGMMultiFormatReader* fin = (NGMMultiFormatReader*) NGMSystem::getSystem();
  if(!fin)
  {
  cout<<"Creating analysis tree "<<std::endl;
  fin = new NGMMultiFormatReader;
  fin->SetPassName("QuickTest2");

  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  //For now simulations do not need a calibration file to be specified
  //Calibration Files must be located in the current directory or NGMCALDIR environment variable
  //// Uncomment the line below if you want to determine the calibration file from the database
  //mCal->SetAllowDatabaseReads(); 
  mCal->SetCalFileName("NGMCAL_B334May2009_1.root");
  
  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("mMerge","mMerge");
  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(0);
  ////Uncomment the line below if you want to exit the analysis after N seconds of live time
  //mMerge->SetMaxLiveTime(10.0);
  //// Tell the analysis how often to send the PlotUpdate message in seconds
  mMerge->SetPlotFrequency(30.0);
  //// Uncomment the line below if you want the analysis to save the entire analysis tree
  //// at the PlotFrequency
  //mMerge->SetSaveOnPlotUpdate(true);
      
    
///*********************************************************///    
  //// Below are analysis modules that may be enabled by 
  //// Uncommenting the heap allocation " = new Module ..."

  //// The module below forms channel by channel timing differences
  //// but it is very cpu intensive
  bool doTimeMon = false;  
  NGMTimeMonitor* mTime = 0;
  if(doTimeMon){
    mTime = new NGMTimeMonitor("TIME","TIME");
    mTime->getParticleIdent()->AddCut(1,-1,0);
  }
  
  //// The module below forms spectra and neutron PSD Plots for real data
  //// may not work properly for large (>120) channel counts like the full prototype
  NGMNeutronMonitor* nmon = 0;
  //nmon = new NGMNeutronMonitor("NMON","NMON");
  if(nmon)
  {
    //nmon->SetAllowDatabaseReads();
    //nmon->SetUpdateDB(globalUpdate);
  }
    
  //// NGMRateMonitor displays per channel rates and per particle type rates
  NGMRateMonitor* rmon = 0;
  rmon = new NGMRateMonitor("RATEMON","RATEMON");
  if(rmon)
  {
    //rmon->SetAllowDatabaseReads();
    //// Uncomment line below to update the database with run statistics for a new run
    //rmon->SetUpdateDB(globalUpdate);
    //// Uncomment line below to overwrite any existing run statistics
    //rmon->SetForceDBUpdate();
  }
  
  NGMCountDistR* countdist = 0;
  //countdist = new NGMCountDistR("CDistFN","CDistFN");
  if(countdist){
    countdist->setGateInterval(10.0);
    countdist->setMaxTime(1000.0);
    setNGates(60);
    countdist->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.,0.);
    countdist->SetDisplayInterval(1E9);
    countdist->setPromptStepForY2FandZ2F(true);
    countdist->setAllGraphs(false);
    //// Uncomment the line below when analyzing gammas from n,nprime interactions
    //countdist->useFastNeutronEquation(false);
    countdist->SetAllowDatabaseReads();
    /// Uncomment the line below to save Y2F fits into the database
    /// DB is update rates are determined by mMerge->SetPlotFrequency
    /// Indepedent of mMerge->SetPlotFrequency fits are saved at the end of the run
    //countdist->SetUpdateDB(globalUpdate);
  }
    

    NGMBurstMonitor* burst = 0;
    burst = new NGMBurstMonitor("burst","burst");
    if(burst)
    {
      burst->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.0,0.0);
      burst->setBurstWindowNS(100.0);
      burst->setGraphWindowNS(200.0);
      burst->setHitsInBurst(1);
      burst->setDrawGraphics(false);
      burst->AddRequirement(NGMSimpleParticleIdent::lsneutron, 1, 1E9);
    }
    
    NGMRandomCD* cdrand = 0;
    cdrand = new NGMRandomCD("FNCDNew","FNCDNew");
    if(cdrand){
       cdrand->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.,0.);
      //// Be sure to update both with the timeGates and countdistributions with the same
      //// Number of Gates
      cdrand->_cd.SetNGates(9);
      cdrand->_timeGates.ParametricTimeGates(10.0, 9, 2.15443, "LOG") ;
      //cdrand->SetUpdateDB(globalUpdate);
      cdrand->SetAllowDatabaseReads();
    }      

    fin->Add(mCal);
    mCal->Add(mMerge);

    
    NGMModule* anaMother = mMerge;    
    //NGMModule* anaMother = mBufHit;
    //NGMModule* anaMother = mTHit;
   
    if (mTime) anaMother->Add(mTime);
    if (countdist) anaMother->Add(countdist);
    if(cdistSN) anaMother->Add(cdistSN);
   if (cdist2) anaMother->Add(cdist2);
   if (nmon) anaMother->Add(nmon);
   if (rmon) anaMother->Add(rmon);
    if(burst) anaMother->Add(burst);
    if(cdrand) anaMother->Add(cdrand);
    if(sncdrand) anaMother->Add(sncdrand);
    
  fin->initModules();

  gROOT->Add(fin->GetParentFolder());
  }
  fin->OpenInputFile(fname);
  fin->StartAcquisition();
  fin->SaveAnaTree();
  
}
