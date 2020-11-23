void CountDist(const int fromFile = 0, const char* fname)
{
  // This routine generates count distributions with their associated
  // moments Cbar, Y2F, Y3F.
  // fromFile = 0 : receiving packets from a socket
  //            1 : reading packets from a file
  // fname is the name of the input file

  NGMPacketBufferIO* fin = new NGMPacketBufferIO("fin","fin");
  NGMPacketSocketInput* pin = new NGMPacketSocketInput("pin","pin");
  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("mMerge","mMerge");
  NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAna","NGMAna");
  NGMCountDist* countdist = new NGMCountDist("CountDist","CountDist");
  NGMHitOutputASCIIFile* ascii = new NGMHitOutputASCIIFile("ascii","ascii");
//  NGMBurstMonitor* burst = new NGMBurstMonitor("burst","burst");

  if (fromFile) {
    fin->Add(mCal);
    mCal->Add(mMerge);
    mMerge->Add(ana);
  } else {
    pin->Add(ana);
  }

  if (fromFile) {
    // Select file to use for neutron-gamma cuts
    mCal->SetROIFileName("/usr/gapps/ngm/data/run7/ROIs5sigLS.root");
    // Assume no packets cross boundary
    // Not a perfect assumption, could use a different mode of merge
    // that would set required number of packets based on active HV???
    mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
    mMerge->setVerbosity(1);
  }

  ana->Add(countdist);
  countdist->Add(ascii);
//  countdist->Add(burst);

  NGMSimpleParticleIdent* partID = countdist->getParticleIdent();
//  partID->AddCut(1,-1.,0.); // gbgamma
//  partID->AddCut(2,-1.,0.); // gbmuon
//  partID->AddCut(3,-1.,0.); // mbgamma
//  partID->AddCut(4,-1.,0.); // mbmuon
//  partID->AddCut(5,-1.,0.); // lsgamma
//  partID->AddCut(6,-1.,0.); // lsneutron
//  partID->AddCut(7,-1.,0.); // lsmuon
  partID->AddCut(8,-1.,0.); // hettlid
  partID->AddCut(9,-1.,0.); // heid

  NGMSimpleParticleIdent* partID2 = ascii->getParticleIdent();
//  partID2->AddCut(1,-1.,0.); // gbgamma
//  partID2->AddCut(2,-1.,0.); // gbmuon
//  partID2->AddCut(3,-1.,0.); // mbgamma
//  partID2->AddCut(4,-1.,0.); // mbmuon
//  partID2->AddCut(5,-1.,0.); // lsgamma
//  partID2->AddCut(6,-1.,0.); // lsneutron
//  partID2->AddCut(7,-1.,0.); // lsmuon
  partID2->AddCut(8,-1.,0.); // hettlid
  partID2->AddCut(9,-1.,0.); // heid

  //  display count rate, Y2F, Y3F
//  countdist->LaunchDisplayMoments();


//  new TBrowser;

  if(!fromFile) {
    gROOT->Add(pin);
    pin->initModules();
    pin->openSocket();
    pin->LaunchReceiveLoopThread();
  } else {
    gROOT->Add(fin);
    fin->initModules();
    fin->openInputFile(fname);
//    int nbuffers = 0;
//    while(fin->readNextBuffer())
//    {
//      if(nbuffers%100 == 0)
//         gSystem->ProcessEvents();
//      nbuffers++;
//    }
//     fin->run();
    int i = 0;
//    while (i < 21) {
    while (i < 21550) {
      fin->readNextBuffer();
      i++;
    }
    // Send end of analysis signals to all modules.
    TObjString endRunFlush("EndRunFlush");
    TObjString endRunSave("EndRunSave");
    fin->push(*((const TObject*)&endRunFlush));
    fin->push(*((const TObject*)&endRunSave));
  }
}
