void runCosmicRejection(const char* fname)
{
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
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
  
  //Cosmic cut parameters [KEV not MEV]
  double gammaslope = 2000.0; // keV per multiplicity
  double gammaconst = 25000.0; // keV per multiplicity
  double neutronslope = 800.0; // keV per multiplicity
  double neutronconst = 5000.0; // keV per multiplicity
  
  NGMMultiFormatReader* fin = (NGMMultiFormatReader*) NGMSystem::getSystem();
  // We check if it the analysis tree is already created as would be the case for
  // running the script in multiple times in the same root session
  if(!fin)
  {
    cout<<"Creating analysis tree "<<std::endl;
    fin = new NGMMultiFormatReader;
    fin->SetPassName("TEST");
    
    NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
    
    NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("mMerge","mMerge");
    mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
    mMerge->setVerbosity(0);
    mMerge->SetMaxLiveTime(600.0);
    mMerge->SetPlotFrequency(3000);
    //mMerge->SetSaveOnPlotUpdate(true);
    
    
    // 
    NGMMultipleScattering* mMult = 0;
    //mMult = new NGMMultipleScattering("mMult","mMult");
    if(mMult)
    {
      mMult->enableNearestNeighbourRejection(true);
      //mMult->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsgamma,200.0,0.0);
      mMult->enable(NGMSimpleParticleIdent::lsneutron);
      mMult->SetMaximumDistanceForInitFromGeometry(30.0);
      //mMult->SetMaximumDistanceForInitFromGeometry(150.0);
      mMult->SetUseGeometry();
      mMult->SetAllowDatabaseReads();
    }
    
//      NGMBufferHits* mBufHit = new NGMBufferHits("mBufHit","mBufHit");
//      mMerge->Add(mBufHit);
//    
//      NGMThreadedHitAnalysis* mTHit = new NGMThreadedHitAnalysis("mTHit","mTHit");
//      mBufHit->Add(mTHit);
    
    double deadTimeNS = 0.0;
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
    //mTime = new NGMTimeMonitor("TIME","TIME");
    if(mTime)
    {
      mTime->SetFillAllChannelByChannel(false);
    }
    NGMNeutronMonitor* nmon = 0;
    //nmon = new NGMNeutronMonitor("NMON","NMON");
    if(nmon)
    {
      //nmon->SetAllowDatabaseReads();
      //nmon->SetUpdateDB();
    }
    
    NGMRateMonitor* rmon = 0;
    rmon = new NGMRateMonitor("RATEMON","RATEMON");
    if(rmon)
    {
      rmon->SetAllowDatabaseReads();
      //rmon->SetUpdateDB(globalUpdate);
    }
            
    NGMBurstMonitor* burst = 0;
    //burst = new NGMBurstMonitor("FNburst","FNburst");
    if(burst)
    {
      burst->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.0,0.0);
      burst->setBurstWindowNS(1000.0);
      burst->setGraphWindowNS(2000.0);
      burst->setHitsInBurst(1);
      burst->setDrawGraphics(false);
      burst->AddRequirement(NGMSimpleParticleIdent::lsneutron, 1, 1E9);
    }
    
    NGMBurstMonitor* burstCR = 0;
    //burstCR = new NGMBurstMonitor("FNburstCR","FNburstCR");
    if(burstCR)
    {
      burstCR->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.0,0.0);
      burstCR->setBurstWindowNS(1000.0);
      burstCR->setGraphWindowNS(2000.0);
      burstCR->setHitsInBurst(1);
      burstCR->setDrawGraphics(false);
      burstCR->AddRequirement(NGMSimpleParticleIdent::lsneutron, 1, 1E9);
    }
    
    NGMBurstMonitor* cosmicBurstFN = 0;
    //cosmicBurstFN = new NGMBurstMonitor("cosmicBurstFN","cosmicBurstFN");
    if(cosmicBurstFN)
    {
      cosmicBurstFN->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.0,0.0);
      cosmicBurstFN->setBurstWindowNS(1000.0);
      cosmicBurstFN->setGraphWindowNS(2000.0);
      cosmicBurstFN->setPushFullWindow(true);
      cosmicBurstFN->setHitsInBurst(1);
      cosmicBurstFN->setDrawGraphics(false);
      
      //cosmicBurstFN->AddRequirement(NGMSimpleParticleIdent::lsneutron, 1E8, 1E9);
      cosmicBurstFN->doMultEnergyCut = true;
      
      cosmicBurstFN->lsnCutSlope = neutronslope;
      cosmicBurstFN->lsnCutConstant = neutronconst;
      cosmicBurstFN->lsgCutSlope = gammaslope;
      cosmicBurstFN->lsgCutConstant = gammaconst;
      cosmicBurstFN->gbCutSlope = 0.0;
      cosmicBurstFN->gbCutConstant = 1E9;
    }
    
    NGMBurstMonitor* cosmicBurstSN = 0;
    cosmicBurstSN = new NGMBurstMonitor("cosmicBurstSN","cosmicBurstSN");
    if(cosmicBurstSN)
    {
      //cosmicBurstSN->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.0,0.0);
      cosmicBurstSN->setBurstWindowNS(10000.0);
      cosmicBurstSN->setGraphWindowNS(1000000.0);
      cosmicBurstSN->setPushFullWindow(true);
      cosmicBurstSN->setHitsInBurst(1);
      cosmicBurstSN->setDrawGraphics(false);
      //cosmicBurstSN->AddRequirement(NGMSimpleParticleIdent::lsneutron, 1, 1E9);
      cosmicBurstSN->doMultEnergyCut = true;
      
      cosmicBurstSN->lsnCutSlope = 0.0;
      cosmicBurstSN->lsnCutConstant = 1E9;
      cosmicBurstSN->lsgCutSlope = 0.0;
      cosmicBurstSN->lsgCutConstant = 1E9;
      cosmicBurstSN->gbCutSlope = gammaslope;
      cosmicBurstSN->gbCutConstant = gammaconst;
    }
    
    NGMBurstMonitor* burstSN = 0;
    burstSN = new NGMBurstMonitor("burstSN","burstSN");
    if(burstSN)
    {
      //burstSN->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.0,0.0);
      burstSN->setBurstWindowNS(10000.0);
      burstSN->setGraphWindowNS(1000000.0);
      burstSN->setPushFullWindow(true);
      burstSN->setHitsInBurst(1);
      burstSN->setDrawGraphics(false);
      //burstSN->AddRequirement(NGMSimpleParticleIdent::lsneutron, 1, 1E9);
      burstSN->doMultEnergyCut = true;
      
      burstSN->lsnCutSlope = 0.0;
      burstSN->lsnCutConstant = 1E9;
      burstSN->lsgCutSlope = 0.0;
      burstSN->lsgCutConstant = 1E9;
      burstSN->gbCutSlope = gammaslope;
      burstSN->gbCutConstant = gammaconst;
    }
    
    NGMRateMonitor* cosmicrmon = 0;
    cosmicrmon = new NGMRateMonitor("CosmicFilterRateMon","CosmicFilterRateMon");
    if(cosmicrmon)
    {
      rmon->SetAllowDatabaseReads();
      //rmon->SetUpdateDB(globalUpdate);
    }
    
    
    NGMRandomCD* cdrand = 0;
    //cdrand = new NGMRandomCD("FNCD","FNCD");
    if(cdrand){
      cdrand->_cd.SetNGates(5);
      cdrand->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.,0.);
      cdrand->_timeGates.ParametricTimeGates(10.0, 5, 4.0, "LOG") ;
      cdrand->SetUpdateDB(globalUpdate);
      cdrand->SetAllowDatabaseReads();
      NGMMomentFit* fitter = cdrand->CreateFitter();
      fitter->FixParam(NGMFitParameter::R02F);
    }      
    NGMRandomCD* cdrandcr = 0;
    //cdrandcr = new NGMRandomCD("FNCDCR","FNCDCR");
    if(cdrandcr){
      cdrandcr->_cd.SetNGates(5);
      cdrandcr->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.,0.);
      cdrandcr->_timeGates.ParametricTimeGates(10.0, 5, 4.0, "LOG") ;
      cdrandcr->SetUpdateDB(globalUpdate);
      cdrandcr->SetAllowDatabaseReads();
      NGMMomentFit* fitter = cdrandcr->CreateFitter();
      fitter->FixParam(NGMFitParameter::R02F);
    }      
    NGMRandomCD* sncdrand = 0;
    sncdrand = new NGMRandomCD("SNCD","SNCD");
    if(sncdrand){
      sncdrand->getParticleIdent()->AddCut(NGMSimpleParticleIdent::hettlid,-1.,0.);
      sncdrand->_cd.SetNGates(5);
      sncdrand->_timeGates.ParametricTimeGates(4000.0, 5, 4.0, "LOG") ;      
      sncdrand->SetAllowDatabaseReads(true);
      sncdrand->SetUpdateDB(globalUpdate);
      NGMMomentFit* fitter = sncdrand->CreateFitter();
      fitter->FixParam(NGMFitParameter::R02F);
    }      
    
    NGMRandomCD* sncdrandcr = 0;
    sncdrandcr = new NGMRandomCD("SNCDCR","SNCDCR");
    if(sncdrandcr){
      sncdrandcr->getParticleIdent()->AddCut(NGMSimpleParticleIdent::hettlid,-1.,0.);
      sncdrandcr->_cd.SetNGates(5);
      sncdrandcr->_timeGates.ParametricTimeGates(4000.0, 5, 4.0, "LOG") ;      
      sncdrandcr->SetAllowDatabaseReads(true);
      sncdrandcr->SetUpdateDB(globalUpdate);
      NGMMomentFit* fitter = sncdrandcr->CreateFitter();
      fitter->FixParam(NGMFitParameter::R02F);
    }      
    
    NGMWaterfall* cWater = 0;
    //cWater = new NGMWaterfall("FNWaterfall","FNWaterfall");
    if(cWater)
    {
      cWater->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.0,0.);
      cWater->SetDisplayInterval(1E9);
    }
    NGMWaterfall* cWaterEcut = 0;
    //cWaterEcut = new NGMWaterfall("FNWaterfallEcut","FNWaterfallEcut");
    if(cWaterEcut){
      cWaterEcut->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,500.0,3000.0);
      cWaterEcut->SetDisplayInterval(1E9);
    }
    
    NGMHitOutputASCIIFile* bWriter = 0;
    //bWriter = new NGMHitOutputASCIIFile("Writer","Writer");
    if(bWriter)
    {
      bWriter->setBinary();
    }
    
    fin->Add(mCal);
    mCal->Add(mMerge);
    //mMerge->Add(mMult);
    
    NGMModule* anaMother = mMerge;    
    //NGMModule* anaMother = mBufHit;
    //NGMModule* anaMother = mTHit;
    
    if (mTime) mMult->Add(mTime);
    if (nmon) anaMother->Add(nmon);
    if (rmon) anaMother->Add(rmon);
    if(burst) anaMother->Add(burst);
    if(cosmicBurstFN){
      anaMother->Add(cosmicBurstFN);
      if(burstCR) cosmicBurstFN->Add(burstCR);
      if(cosmicrmon) cosmicBurstFN->Add(cosmicrmon);
      if(cdrandcr) cosmicBurstFN->Add(cdrandcr);
    }
    if(cosmicBurstSN){
      anaMother->Add(cosmicBurstSN);
      if(sncdrandcr) cosmicBurstSN->Add(sncdrandcr);
      if(burstSN) cosmicBurstSN->Add(burstSN);
    }
    if(cdrand) anaMother->Add(cdrand);
    if(sncdrand) anaMother->Add(sncdrand);
    if(cWater) anaMother->Add(cWater);
    if(cWaterEcut) anaMother->Add(cWaterEcut);
    if(bWriter) anaMother->Add(bWriter);
      
    fin->initModules();
    
    gROOT->Add(fin->GetParentFolder());
    
    if(0){
      int defwidth = 700;
      int defheight = 500;
      int screenTop = 0;
      TString cName;
      if(cdrand)
      {
        cName.Form("%s_Canvas",cdrand->GetName());
        TCanvas* tc = new TCanvas(cName,cName,400,screenTop,defwidth,defheight);
        cdrand->GetParentFolder()->Add(tc);
      }
      if(cdrandcr)
      {
        cName.Form("%s_Canvas",cdrandcr->GetName());
        TCanvas* tc = new TCanvas(cName,cName,400,screenTop+defheight+20,defwidth,defheight);
        cdrandcr->GetParentFolder()->Add(tc);
      }
      if(sncdrand)
      {
        cName.Form("%s_Canvas",sncdrand->GetName());
        TCanvas* tc = new TCanvas(cName,cName,400+defwidth,screenTop,defwidth,defheight);
        sncdrand->GetParentFolder()->Add(tc);
      }
      if(sncdrandcr)
      {
        cName.Form("%s_Canvas",sncdrandcr->GetName());
        TCanvas* tc = new TCanvas(cName,cName,400+defwidth,screenTop+defheight+20,defwidth,defheight);
        sncdrandcr->GetParentFolder()->Add(tc);
      }
    }
    
  }
  fin->OpenInputFile(fname);
  
  fin->StartAcquisition();
  fin->SaveAnaTree();
  
}
