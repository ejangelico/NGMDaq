void mergePacketsFromFileRun7(const char* fname, const char* outprefix="Run7Hit")
{
  // This routine produces a merge sorted output stream of NGMHits
  // fname is the name of the input file
  // outprefix is the prefix to be prepended to the output files.

  // Lets update the input file with the latest calibration values
  TFile* tf = TFile::Open(fname,"UPDATE");
  if(!tf) printf("Error opening input file!\n");
  NGMSystemConfiguration* conf = (NGMSystemConfiguration*)(tf->Get("NGMSystemConfiguration"));
  if(!conf) printf("Error: Configuration object was not found!\n");
  conf->GetDetectorParameters()->ImportFromTextFile("/usr/gapps/ngm/data/run7/DetectorParsPUCal.txt");
  tf->WriteTObject(conf,"NGMSystemConfiguration");
  tf->Close();
  delete tf;
  
  //
  NGMPacketBufferIO* fin = new NGMPacketBufferIO("fin","fin");
  //NGMPacketSocketInput* pin = new NGMPacketSocketInput("pin","pin");
  NGMPacketFilter* pfilt = new NGMPacketFilter("pfilt","pfilt");
  NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAnaOffline","NGMAnaOffline");
  NGMNeutronMonitor* nmon = new NGMNeutronMonitor("NMON","NMON");

  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("merge","merge");
  NGMTimeMonitor* mTime = new NGMTimeMonitor("TIME","TIME");
  NGMHitOutputFile* mHitOut = new NGMHitOutputFile(outprefix,"HitOut");

  // Assume no packets cross boundary
  // Not a perfect assumption, could use a different mode of merge
  // that would set required number of packets based on active HV???
  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(1); // Keep things quiet

  mHitOut->setBasePathVariable("");
  mHitOut->setBasePath("");
  // Select file to use for neutron-gamma cuts
  mCal->SetROIFileName("/usr/gapps/ngm/data/run7/ROIs5sigLS.root");
  
  // This channel seems to have timestamp problem 
  //  pfilt->addSkipChannel(5,5);

  //  fin->Add(pfilt);
  //  pfilt->Add(ana);
  fin->Add(ana);
  ana->Add(mCal);
  mCal->Add(nmon);
  mCal->Add(mMerge);
  //mMerge->Add(mTime);
  mMerge->Add(mHitOut);
  
  fin->initModules();
  fin->openInputFile(fname);
  Long64_t nbuffers = 0;
//   // Uncomment while loop for a looking at a subset of data
//   while(nbuffers++<10000)
//   {
//     fin->readNextBuffer();
//   }

  fin->run();
  
  // Send end of analysis signals to all modules.
  TObjString endRunFlush("EndRunFlush");
  TObjString endRunSave("EndRunSave");
  fin->push(*((const TObject*)&endRunFlush));
  fin->push(*((const TObject*)&endRunSave));

}
