void convertSim(const char* fname = "MPI_List_timeout_n_p_np_lsg.200keV_truncated")
{
  // This routine reads a simulation file fin. It needs some initialization
  // parameters in file 'MPI_List_timeout_n_p_np.init'
  //
  // fin: the name of the simulation file
  //

  NGMCOGSim* fin = new NGMCOGSim("simulation","simulation");
  NGMHitOutputFile* foutput = new NGMHitOutputFile("simulation", "simulation");

  fin->Add(foutput);

  gROOT->Add(fin);
  fin->initModules();
  fin->openInputFile(fname);
  int i = 0;
  while (i < 100) {
    fin->readEvents(2000);
    i++;
  }
  // Send end of analysis signals to all modules.
  TObjString endRunFlush("EndRunFlush");
  fin->push(*((const TObject*)&endRunFlush));
  TObjString endRunSave("EndRunSave");
  fin->push(*((const TObject*)&endRunSave));
}
