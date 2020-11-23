#!/Volumes/NGML1/ngm/NGMinstall/bin/NGMBatch

void parseModuleConfigurationFile(const char* fname, const char* parentName);  // defined later in file

void runAnaLive(const char* fname = "")
{
	TObject::SetObjectStat(0);
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
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
	NGMLogger::instance()->setOutputSelector(NGMLogger::diskFile);
	NGMLogger::instance()->openFile("ana.log");
	
	double nupper = 3.0;
	double nlow = -3.0;
	double gupcut = 5.0;
	double tailtestlow = 1.0;
	double tailtesthigh = 4.0;
	bool useThreads = false;
  bool globalUpdate = false;
  NGMMultiFormatReader* fin = (NGMMultiFormatReader*) NGMSystem::getSystem();
  // We check if it the analysis tree is already created as would be the case for
  // running the script in multiple times in the same root session
  if(!fin)
  {
    
    cout<<"Creating analysis tree "<<std::endl;
    fin = new NGMMultiFormatReader;
    fin->SetPassName("LIVE");

  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  //  mCal->SetAllowDatabaseReads();
  mCal->SetCalFileName("NGMCal_B334Nov2009_newPSD_V2.root");

  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("mMerge","mMerge");
  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(0);
  //mMerge->SetMaxLiveTime(10);
  mMerge->SetPlotFrequency(10);
    NGMBufferHits* hitBuffer = 0;
		NGMThreadedHitAnalysis* threadAna = 0;
		if(useThreads)
		{
			hitBuffer = new NGMBufferHits("hitBuffer","hitBuffer");
			threadAna= new NGMThreadedHitAnalysis("threadAna","threadAna");
		}

		NGMCountDistF* rcount = 0;
		rcount = new NGMCountDistF("rcount","rcount");
		if(rcount){
			rcount->getParticleIdent()->setNeutronIdParams(nlow,nupper,gupcut);
			rcount->getParticleIdent()->setTailTest(tailtestlow, tailtesthigh);
			rcount->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.,0.);
			//rcount->SetDisplayInterval(10);
			rcount->setGateInterval(0.25);
			rcount->setAllGraphs(true);
			rcount->selectGapProbability(1E-120);
		}
		
		NGMCountDistF* rsncount = 0;
		rsncount = new NGMCountDistF("rsncount","rsncount");
		if(rsncount){
			rsncount->getParticleIdent()->setNeutronIdParams(nlow,nupper,gupcut);
			rsncount->getParticleIdent()->setTailTest(tailtestlow, tailtesthigh);
			rsncount->getParticleIdent()->AddCut(NGMSimpleParticleIdent::hettlid,-1.,0.);
			//rcount->SetDisplayInterval(10);
			rsncount->setGateInterval(1000);
			rsncount->setAllGraphs(true);
			rsncount->selectGapProbability(1E-120);
		}

	NGMRateMonitor* rmon = 0;
  rmon = new NGMRateMonitor("RATEMON","RATEMON");
  if(rmon)
  {
    rmon->SetAllowDatabaseReads();
		rmon->getParticleIdent()->setNeutronIdParams(nlow,nupper,gupcut);
		rmon->getParticleIdent()->setTailTest(tailtestlow, tailtesthigh);
    rmon->SetForceDBUpdate(globalUpdate);
    rmon->SetUpdateDB(globalUpdate);
  }

  NGMNeutronMonitor* nmon = 0;
  nmon = new NGMNeutronMonitor("NMON","NMON");
  if(nmon)
  {
		nmon->getParticleIdent()->setNeutronIdParams(nlow,nupper,gupcut);
		nmon->getParticleIdent()->setTailTest(tailtestlow, tailtesthigh);
    //nmon->SetAllowDatabaseReads();
    //nmon->SetUpdateDB(globalUpdate);
  }

  //NGMPacketOutputFile* pOut = new NGMPacketOutputFile("CalOut","CalOut");
  //pOut->setBasePathVariable("");
  //pOut->setBasePath("./");

  NGMRandomCD* fncdrand = 0;
  fncdrand = new NGMRandomCD("FNCD","FNCD");
  if(fncdrand){
    fncdrand->_cd.SetNGates(10);
    fncdrand->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron,-1.,0.);
		fncdrand->getParticleIdent()->setNeutronIdParams(nlow,nupper,gupcut);
		fncdrand->getParticleIdent()->setTailTest(tailtestlow, tailtesthigh);

    fncdrand->_timeGates.ParametricTimeGates(10, 10, 10, "LIN") ;
    //fncdrand->SetUpdateDB(globalUpdate);
    fncdrand->SetAllowDatabaseReads();

    NGMMomentFit* fitter = fncdrand->CreateFitter();
    fitter->FixParam(NGMFitParameter::R02F);
    fitter->FixParam(NGMFitParameter::R03F);
  }      
    NGMRandomCD* sncdrand = 0;
    sncdrand = new NGMRandomCD("SNCD","SNCD");
    if(sncdrand){
      sncdrand->getParticleIdent()->AddCut(NGMSimpleParticleIdent::hettlid,-1.,0.);
      sncdrand->_cd.SetNGates(50);
      sncdrand->_timeGates.ParametricTimeGates(10000.0, 50, 10000.0, "LIN") ;      
      sncdrand->SetAllowDatabaseReads(true);
			sncdrand->getParticleIdent()->setNeutronIdParams(nlow,nupper,gupcut);
			sncdrand->getParticleIdent()->setTailTest(tailtestlow, tailtesthigh);
      //sncdrand->SetUpdateDB(globalUpdate);
      NGMMomentFit* fitter = sncdrand->CreateFitter();
      fitter->FixParam(NGMFitParameter::R02F);
      fitter->FixParam(NGMFitParameter::R03F);
    }
		
    NGMBurstMonitor* fnburst = 0;
		NGMBurstWaterfall* burstWaterfall = 0;
		fnburst = new NGMBurstMonitor("fnburst","fnburst");
		if(fnburst){
			fnburst->setHitsInBurst(2);
			fnburst->setBurstWindowNS(100.0);
			fnburst->setGraphWindowNS(5000.0);
			//fnburst->AddRequirement(NGMSimpleParticleIdent::lsgamma, 1, 1E9);
			fnburst->AddRequirement(NGMSimpleParticleIdent::lsneutron, 2, 1E9);
			fnburst->getParticleIdent()->setNeutronIdParams(nlow,nupper,gupcut);
			fnburst->getParticleIdent()->setTailTest(tailtestlow, tailtesthigh);
			fnburst->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsgamma, 250.0, 5000.0);
			fnburst->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron, 250.0, 5000.0);
			fnburst->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsmuon, 0.0, 1E9);
			fnburst->setPushFullWindow(true);
//			NGMPacketOutputFile* burstOut = new NGMPacketOutputFile("BurstOut","BurstOut");
//			burstOut->setBasePathVariable("");
//			burstOut->setBasePath("./");
//			fnburst->Add(burstOut);
			burstWaterfall = new NGMBurstWaterfall("burstWaterfall","burstWaterfall");
			fnburst->Add(burstWaterfall);
			
		}
    
	  NGMWaterfall* mWaterfall = 0;
		//mWaterfall = new NGMWaterfall("mWaterfall","mWaterfall");
		if(mWaterfall){
			mWaterfall->SetDisplayInterval(1E9);
			mWaterfall->SetNumberOfPointsDisplayed(100000);
			mWaterfall->getParticleIdent()->setNeutronIdParams(nlow,nupper,gupcut);
			mWaterfall->getParticleIdent()->setTailTest(tailtestlow, tailtesthigh);

	  }
		
	  NGMTimeMonitor* mTime = 0;
		//mTime = new NGMTimeMonitor("mTime","mTime");
	  
	  
  fin->Add(mCal);
		
		NGMAnaGui* gui = new NGMAnaGui("AnaGui","AnaGui");
		mCal->Add(gui);

  //  if(nmon) mCal->Add(nmon);
		
  gui->Add(mMerge);
		NGMModule* anaMother = mMerge;
	if(useThreads)
	{
		anaMother = threadAna;
		mMerge->Add(hitBuffer);
		hitBuffer->Add(threadAna);
	}
		
  if (fncdrand) anaMother->Add(fncdrand);
  if (rmon) anaMother->Add(rmon);
    if(sncdrand) anaMother->Add(sncdrand);
  if (nmon) anaMother->Add(nmon);
	  if(mWaterfall) anaMother->Add(mWaterfall);
	  if (mTime) anaMother->Add(mTime);
		if (fnburst) anaMother->Add(fnburst);
		if(rcount) anaMother->Add(rcount);
    if(rsncount) anaMother->Add(rsncount);
		
  fin->initModules();

  gROOT->Add(fin->GetParentFolder());
    
    if(1){
		int screenWidth = 1920;
		int screenHeight = 1180;
		int leftedge = 0;
      int defwidth = 650;
      int defheight = 580;
      int screenTop = 0;
      TString cName;

		if(mTime)
		{
			cName.Form("%s_Canvas",mTime->GetName());
			TCanvas* tc = new TCanvas(cName,cName,leftedge,screenTop,defwidth,defheight);
			mTime->GetParentFolder()->Add(tc);
		}
		if(nmon)
      {
        cName.Form("%s_Canvas",nmon->GetName());
        TCanvas* tc = new TCanvas(cName,cName,leftedge+screenWidth*2,screenTop,screenWidth,screenHeight);
        nmon->GetParentFolder()->Add(tc);
		  tc->Divide(3,4,0.001,0.001);
      }
      if(rmon)
      {
        cName.Form("%s_Canvas",rmon->GetName());
        TCanvas* tc = new TCanvas(cName,cName,leftedge,screenTop+defheight+20,defwidth,defheight);
        rmon->GetParentFolder()->Add(tc);
      }
      if(fncdrand)
      {
        cName.Form("%s_Canvas",fncdrand->GetName());
        TCanvas* tc = new TCanvas(cName,cName,leftedge+defwidth,screenTop,defwidth,defheight);
        fncdrand->GetParentFolder()->Add(tc);
      }
      if(sncdrand)
      {
        cName.Form("%s_Canvas",sncdrand->GetName());
        TCanvas* tc = new TCanvas(cName,cName,leftedge+defwidth,screenTop+defheight+20,defwidth,defheight);
        sncdrand->GetParentFolder()->Add(tc);
      }
			if(rcount)
			{
				cName.Form("%s_Canvas",rcount->GetName());
				TCanvas* tc = new TCanvas(cName,cName,leftedge+2.0*defwidth,screenTop,defwidth,defheight);
				rcount->GetParentFolder()->Add(tc);
				tc->Divide(3,3);
				
			}
			
			if(rsncount)
			{
				cName.Form("%s_Canvas",rsncount->GetName());
				TCanvas* tc = new TCanvas(cName,cName,leftedge+2.0*defwidth,screenTop,defwidth,defheight);
				rsncount->GetParentFolder()->Add(tc);
				tc->Divide(3,3);
				
			}

			if(mWaterfall)
		{
			cName.Form("%s_Canvas",mWaterfall->GetName());
			TCanvas* tc = new TCanvas(cName,cName,leftedge+screenWidth,screenTop,screenWidth,screenHeight/2);
			mWaterfall->GetParentFolder()->Add(tc);
			tc->Divide(1,2);
		}
			if(fnburst)
			{
				cName.Form("%s_Canvas",fnburst->GetName());
				TCanvas* tc = new TCanvas(cName,cName,leftedge+screenWidth,screenHeight/2,screenWidth,screenHeight/2);
				fnburst->GetParentFolder()->Add(tc);
				tc->Divide(1,2);
			}
			if(burstWaterfall)
			{
				cName.Form("%s_Canvas",burstWaterfall->GetName());
				TCanvas* tc = new TCanvas(cName,cName,leftedge+screenWidth,screenTop,screenWidth,screenHeight/2);
				burstWaterfall->GetParentFolder()->Add(tc);
				tc->Divide(1,2);
			}
			
    }
    
    
    
  }
  // If input filename is null assume we want to examine the most recent file
  TString sfname(fname);
  if(sfname == "")
  {
   gSystem->Exec("ls -rt ../data/SISRaw*_1.bin | tail -n1 > .lastsisfile.tmp");
    ifstream tmpf(".lastsisfile.tmp");
    sfname.ReadLine(tmpf);
    sfname = TString(sfname(0,sfname.Index("_1.bin"))).Data();
  }

  fin->OpenInputFile(sfname.Data());
  fin->StartAcquisition();
  fin->SaveAnaTree();

}
