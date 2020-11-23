void parseModuleConfigurationFile(const char* fname, const char* parentName);  // defined later in file

void runAnaBatch(const char* fname=0, double maxCPUTime = 0.0, int maxSpills = 0)
{

  int maxSpills = 5;
   int deadTimeNS=0;
   int burstSize=3;
   int gateInterval=0;
   int doTimeMon=1;
   int doCountDist = 1;
	float waterFallInterval=0;
   
   int pulseHight=3000;
   int burstWindow=10;
   int burstGraphWindow=10000;
   int countPID=1;
   int waterFallPoints=1000;
   int simulation=0;
   int GrandCanyonAnalysis=0;
   
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
	
	if(simulation == 1) {
		NGMHitIO* fin = new NGMHitIO("fin","fin");
	} else {
		NGMPacketBufferIO* fin = new NGMPacketBufferIO("fin","fin");
		NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
    mCal->SetAllowDatabaseReads();
		NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("mMerge","mMerge");
	}
  NGMDeadTimeModule* dead = new NGMDeadTimeModule("dead","dead");
	NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAna","NGMAna");
	gROOT->Add(ana);
   		
   NGMNeutronMonitor* basicMon = new NGMNeutronMonitor("NMON","NMON");

	if (simulation == 1) {
		fin->Add(ana);
	} else {
		fin->Add(mCal);
      if(basicMon) mCal->Add(basicMon);
		mCal->Add(mMerge);
		//    mMerge->Add(dead);
		//    dead->Add(ana);
    mMerge->Add(ana);
	}
      
	if (simulation == 0) {
		// Select file to use for neutron-gamma cuts
		//  mCal->SetROIFileName("ROIs.root"); //"/usr/gapps/ngm/data/run7/ROIs5sigLS.root");
		// mCal->SetROIFileName("/p/lscratcha/verbeke2/ROIs2007Dec20.root");
		//  mCal->SetROIFileName("/usr/gapps/ngm/data/run7/ROIs5sigLS.root");
		// Assume no packets cross boundary
		// Not a perfect assumption, could use a different mode of merge
		// that would set required number of packets based on active HV???
		mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
		mMerge->setVerbosity(1);
	}
	
   parseModuleConfigurationFile("batch.txt","NGMAna");
	
	gROOT->Add(fin);
	fin->initModules();
	fin->openInputFile(fname);
	int spills = 0;
	TStopwatch cpuTimer;
	cpuTimer.Start();
	double tcpu = 0.0;
	while(fin->readNextSpill()) {
	  spills++;
	  if(maxSpills>0 && spills > maxSpills) break;
	  tcpu = cpuTimer.CpuTime();
	  cpuTimer.Continue();
	  cout<<"At spill "<<spills<<" cpu time(s) "<<tcpu<<endl;
	  if(tcpu > maxCPUTime && maxCPUTime > 0.0) break;
	}
	
	
	// Send end of analysis signals to all modules.
	TObjString endRunFlush("EndRunFlush");
	TObjString endRunSave("EndRunSave");
	fin->push(*((const TObject*)&endRunFlush));
	fin->push(*((const TObject*)&endRunSave));
}

void parseModuleConfigurationFile(const char* fname, const char* parentName)
{
   ifstream cFile(fname);
   char cbuf[4096];
   char cmdbuf[4096];
   int buflength = 4096;
   bool moduletablefound = false;
   TString moduleType;
   TString moduleName;
   TString moduleConf;
   NGMModule* tModule = 0;
   gROOT->cd();
   NGMModule* parModule = (NGMModule*)(gROOT->FindObjectAny(parentName));
   if(parModule)
      std::cout<<"Found parent module "<<parModule->GetName()<<std::endl;
   int nlines = 0;
   gROOT->cd();
   while(!cFile.eof() && nlines <1000)
   {
      nlines++;
      cFile.getline(cbuf,buflength);
      //std::cout<<cbuf<<std::endl;
      TString cline(cbuf);
      
      if(!moduletablefound)
      if(cline == "|+Analysis Summary Table")
      {
         moduletablefound = true;
         // Skip next two lines
         cFile.getline(cbuf,buflength);
         cFile.getline(cbuf,buflength);
      }
      
      if(!moduletablefound) continue;
      
      // Test for next module definition
      if(cline.BeginsWith("|-"))
      {
         moduleType = "";
         moduleName = "";
         moduleConf = "";
         tModule = 0;
      }
      // Test for module type
      if(cline.BeginsWith("! "))
      {
         moduleType = cline(2,cline.Length()-2);
         std::cout<<"Mod Type: "<<moduleType<<std::endl;
      }
      // Test for module name
      if(cline.BeginsWith("| ") && cline.Length()>3)
      {
         moduleName = cline(2,cline.Length()-2);
         std::cout<<"Mod Name: "<<moduleType<<std::endl;
         std::cout<<"createNewModule "<<moduleType<<" "<<moduleName<<std::endl;
         sprintf(cmdbuf,"%s* %s = new %s(\"%s\",\"%s\");",
                 moduleType.Data(),moduleName.Data(),moduleType.Data(),
                 moduleName.Data(),moduleName.Data(),moduleName.Data());
         std::cout<<cmdbuf<<std::endl;
         gROOT->ProcessLine(cmdbuf);
         if(parModule)
         {
            sprintf(cmdbuf,"%s->Add(%s);", parentName, moduleName.Data());
            gROOT->ProcessLine(cmdbuf);            
         }
         
      }
      // Test for module name
      if(cline.BeginsWith("* ") && cline.Length()>3)
      {
         moduleConf = cline(2,cline.Length()-2);
         std::cout<<"Mod Conf: "<<moduleConf<<std::endl;
         std::cout<<"configModule "<<moduleType<<" "<<moduleName<<" "<<moduleConf<<std::endl;
         sprintf(cmdbuf,"%s->%s;",
                 moduleName.Data(),moduleConf.Data());
         std::cout<<cmdbuf<<std::endl;
         gROOT->ProcessLine(cmdbuf);
         
      }
      
      if(cline == "|}")
      {
         // Were at the end of the table
         return;
      }
   }
   cFile.close();
}

void extractFiguresFromAnalysisFile(const char* fName)
{
   
}
