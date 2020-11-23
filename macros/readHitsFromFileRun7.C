void readHitsFromFileRun7(const char* fname)
{
  // Read a File stream of sorted hits and produce analysis histograms

  NGMHitIO* fin = new NGMHitIO("fin","fin");
  NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAnaTiming","NGMAnaTiming");
  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  NGMNeutronMonitor* nmon = new NGMNeutronMonitor("NMON","NMON");
  NGMTimeMonitor* mTime = new NGMTimeMonitor("TIME","TIME");

  fin->Add(ana);
  ana->Add(nmon);
  ana->Add(mTime);
  
  gROOT->Add(fin);
  new TBrowser;


  fin->initModules();
  fin->openInputFile(fname);
  Long64_t nbuffers = 0;
//   while(fin->readNextBuffer())
//   {
//     if(nbuffers%10000 == 0)
//       gSystem->ProcessEvents();
//     nbuffers++;
//   }
  fin->run();
  TObjString endRunFlush("EndRunFlush");
  TObjString endRunSave("EndRunSave");
  fin->push(*((const TObject*)&endRunFlush));
  fin->push(*((const TObject*)&endRunSave));

}
