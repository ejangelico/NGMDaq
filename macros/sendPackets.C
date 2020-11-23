void sendPackets(const char* fname)
{
  
  NGMPacketBufferIO* pin = new NGMPacketBufferIO("pin","pin");
  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("mMerge","mMerge");
  NGMPacketSocketOutput* pout = new NGMPacketSocketOutput("pout","pout");
  pin->Add(mCal);
  mCal->Add(mMerge);
  mMerge->Add(pout);

  // Select file to use for neutron-gamma cuts
  mCal->SetROIFileName("/usr/gapps/ngm/data/run7/ROIs5sigLS.root");

  // Assume no packets cross boundary
  // Not a perfect assumption, could use a different mode of merge
  // that would set required number of packets based on active HV???
  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(1);

  pin->initModules();
  pout->openSocket("localhost");
  pin->openInputFile(fname);

//  pin->run();
  for(int ibuffer = 0; ibuffer < 10000; ibuffer++) pin->readNextBuffer();

  // Send end of analysis signals to all modules.
  TObjString endRunFlush("EndRunFlush");
  TObjString endRunSave("EndRunSave");
  pin->push(*((const TObject*)&endRunFlush));
  pin->push(*((const TObject*)&endRunSave));
  return;
}
