void splitDataFile(const char* fname, int spillsPerFile = 100)
{
  NGMPacketBufferIO* fin = new NGMPacketBufferIO("fin","fin");
  NGMPacketOutputFile* fout = new NGMPacketOutputFile("RawSplit","RawSplit");
  fin->Add(fout);
  fout->setSpillsPerFile(spillsPerFile);
  fout->setBasePathVariable("");
  fout->setBasePath("./");
  fout->setSplitRuns();
  fin->initModules();
  fin->openInputFile(fname);
  while(fin->readNextSpill()) {}
  // Send end of analysis signals to all modules.
	TObjString endRunFlush("EndRunFlush");
	TObjString endRunSave("EndRunSave");
	fin->push(*((const TObject*)&endRunFlush));
	fin->push(*((const TObject*)&endRunSave));
  
}
