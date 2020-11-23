void createRunStats(const char* fname=0)
{

  gROOT->SetStyle("Plain");
  //Standardish values
  //deadTimeNS=4000
  //burstSize=10
  //gateInterval=1000
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

  NGMMultiFormatReader* fin = new NGMMultiFormatReader;
  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("mMerge","mMerge");
  NGMRateMonitor* rmon = new NGMRateMonitor("RATEMON","RATEMON");



  mMerge->SetPlotFrequency(1E9);
  rmon->SetUpdateDB();

  fin->Add(mCal);
  fin->SetPassName("RunStats");
  mCal->Add(mMerge);
  mMerge->Add(rmon);

  fin->initModules();
  fin->OpenInputFile(fname);
  fin->StartAcquisition();

    // Send end of analysis signals to all modules.
    TObjString endRunFlush("EndRunFlush");
    TObjString endRunSave("EndRunSave");
    fin->push(*((const TObject*)&endRunFlush));
    fin->push(*((const TObject*)&endRunSave));
}
