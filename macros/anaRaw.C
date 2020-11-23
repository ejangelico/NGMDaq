void anaRaw(const char* fname)
{
  SISRawRead* fin = new SISRawRead();
  NGMCalibrator* cal = new NGMCalibrator("cal","cal");
  NGMPacketMergeSort* merge = new NGMPacketMergeSort("merge","merge");
  NGMAnalysisInput* ana = new NGMAnalysisInput("AnaNeutronVeto","AnaNeutronVeto");
  NGMNeutronMonitor* nmon = new NGMNeutronMonitor("NMON","NMON");
  NGMMultipleScattering* mult = new NGMMultipleScattering("mult","mult");
  NGMTimeMonitor* mTime = new NGMTimeMonitor("TIME","TIME");
  mult->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron, -1, 0);
  mult->setMaxTime(400);
  mTime->setMaxNanoSecInList(200);
  mTime->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron, -1, 0);

  NGMCountDistR* countDistG = new NGMCountDistR("CDISTGAMMA","CDISTGAMMA");
  countDistG->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsgamma, -1, 0.0);
  countDistG->SetDisplayInterval(1E10);
  countDistG->setAllGraphs(true);
  countDistG->setGateInterval(1);
  countDistG->useFastNeutronEquation(false);
  countDistG->setPromptStepForY2FandZ2F(true);
  countDistG->SetAllowDatabaseReads(true);
  
  NGMCountDistR* countDistN = new NGMCountDistR("CDISTFN","CDISTFN");
  countDistN->getParticleIdent()->AddCut(NGMSimpleParticleIdent::lsneutron, -1, 0.0);
  countDistN->SetDisplayInterval(1E10);
  countDistN->setAllGraphs(true);
  countDistN->setGateInterval(1);
  countDistN->useFastNeutronEquation(true);
  countDistN->setPromptStepForY2FandZ2F(true);
  countDistN->SetAllowDatabaseReads(true);
  
  cal->SetCalFileName("CalB3342008July.root");
  merge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  
  
  fin->Add(ana);
  ana->Add(cal);
  cal->Add(merge);
  cal->Add(nmon);
  
  //merge->Add(countDistG);
  //merge->Add(countDistN);  
  merge->Add(mult);
  mult->Add(mTime);
  
  merge->setVerbosity(10);

  fin->initModules();
  NGMDisplay::Instance()->InitFromMacro("/Users/jnewby/Public/NGMDaq/macros/drawLSHexHenge.C");
  mult->InitFromGeometry(0);
  fin->InitializeSystem();
  
  fin->OpenInputFile(fname);
  int nbuffs = 0;

  //while(fin->ReadNextBufferFromFile()){}//{if((nbuffs++)>=(96-3)) break;}
  fin->ReadListModeGeneral();
  
  // Send end of analysis signals to all modules.
  TObjString endRunFlush("EndRunFlush");
  TObjString endRunSave("EndRunSave");
  fin->push(*((const TObject*)&endRunFlush));
  fin->push(*((const TObject*)&endRunSave));
  
  
}