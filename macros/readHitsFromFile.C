void readHitsFromFile(const char* fname)
{
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
  
  // Read a File stream of sorted hits and produce analysis histograms

  NGMHitIO* fin = new NGMHitIO("fin","fin");
  NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAnaTiming","NGMAnaTiming");
  NGMTimeMonitor* timeMon = new NGMTimeMonitor("NGMTiming","NGMTiming");
  NGMBurstMonitor* burst = new NGMBurstMonitor("burst","burst");
  NGMCountDist* countdist = new NGMCountDist("CountDist","CountDist");
  NGMSimpleParticleIdent* partID = countdist->getParticleIdent();
  
//  partID->AddCut(1,-1.,0.); // gbgamma
//  partID->AddCut(2,-1.,0.); // gbmuon
//  partID->AddCut(3,-1.,0.); // mbgamma
//  partID->AddCut(4,-1.,0.); // mbmuon
//  partID->AddCut(5,-1.,0.); // lsgamma
	partID->AddCut(6,-1.,0.); // lsneutron
//  partID->AddCut(7,-1.,0.); // lsmuon
//  partID->AddCut(8,-1.,0.); // hettlid
//  partID->AddCut(9,-1.,0.); // heid

  NGMCountDist* countdistlong = new NGMCountDist("CountDistLong","CountDistLong");
  NGMSimpleParticleIdent* partIDLong = countdistlong->getParticleIdent();
  partIDLong->AddCut(8,-1.,0.); // hettlid

  countdistlong->setGateInterval(1000.0);
  countdist->setGateInterval(10.0);
  burst->setHitsInBurst(2);

  burst->setBurstWindowNS(1E5);
  burst->setGraphWindowNS(1E6);

  fin->Add(ana);
  //ana->Add(timeMon);
  timeMon->setMaxNanoSecInList(1E3);
  //ana->Add(burst);
  ana->Add(countdist);
  ana->Add(countdistlong);
  
  gROOT->Add(fin);
  new TBrowser;


  fin->initModules();
  fin->openInputFile(fname);
  Long64_t nbuffers = 0;
   while(fin->readNextBuffer())
   {
     if(nbuffers%100000 == 0)
       gSystem->ProcessEvents();
     nbuffers++;
	 //if(nbuffers>5E4) break;
   }
//  fin->run();
  TObjString endRunFlush("EndRunFlush");
  TObjString endRunSave("EndRunSave");
  fin->push(*((const TObject*)&endRunFlush));
  fin->push(*((const TObject*)&endRunSave));

}
