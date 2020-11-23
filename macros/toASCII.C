void toASCII(const char* fname, int maxSpills = 0)
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

  NGMPacketBufferIO* fin = new NGMPacketBufferIO("fin","fin");
  NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAna","NGMAna");
  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("mMerge","mMerge");
  NGMHitOutputASCIIFile* fout = new NGMHitOutputASCIIFile("ASCIIData","ASCIIData");
 // NGMTimeMonitor* timeMon = new NGMTimeMonitor("timeMonitor","timeMonitor");

  fin->Add(ana);
  ana->Add(mCal);
  mCal->Add(mMerge);
  mMerge->Add(fout);
//  mMerge->Add(timeMon);

  // Select file to use for neutron-gamma cuts
  mCal->SetROIFileName("ROIs.root");

  // Assume no packets cross boundary
  // Not a perfect assumption, could use a different mode of merge
  // that would set required number of packets based on active HV???
  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(1);

// set which particles to write out
// default: dumps everything unless a single line is commented in
  NGMSimpleParticleIdent* partID = fout->getParticleIdent();
// (particle type, min energy =-1 is everything, max energy)
//  partID->AddCut(1,-1.,0.); // gbgamma
//  partID->AddCut(2,-1.,0.); // gbmuon
//  partID->AddCut(3,-1.,0.); // mbgamma
//  partID->AddCut(4,-1.,0.); // mbmuon
//  partID->AddCut(5,-1.,0.); // lsgamma
//  partID->AddCut(6,-1.,0.); // lsneutron
//  partID->AddCut(7,-1.,0.); // lsmuon
//  partID->AddCut(8,-1.,0.); // hettlid
//  partID->AddCut(9,-1.,0.); // heid

  fout->setBasePathVariable("");
  fout->setBasePath("/g/g22/verbeke2/root/");
  fout->setBinary(true);
//  fout->setOutputFileName("RonWurtz");

  fin->initModules();
  gROOT->Add(fin);
  // new TBrowser;

  fin->openInputFile(fname);
  // fin->run();
  int nSpills = 0;
  while(fin->readNextSpill() && nSpills<maxSpills)
  {
    // if(nSpills%50 == 0) gSystem->ProcessEvents();
    cout<<"At spill "<<nSpills<<endl;
    nSpills++;
  }

//       fin->run();


  fout->closeOutputFile();

  // Send end of analysis signals to all modules.
  TObjString endRunFlush("EndRunFlush");
  TObjString endRunSave("EndRunSave");
  fin->push(*((const TObject*)&endRunFlush));
  fin->push(*((const TObject*)&endRunSave));
}
