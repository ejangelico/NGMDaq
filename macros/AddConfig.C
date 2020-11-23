void AddConfig(const char* fname)
{
  // This routine adds the latest calibration values to the 
  // input file
  // fname is the name of the input file

  // Lets update the input file with the latest calibration values
  TFile* tf = TFile::Open(fname,"UPDATE");
  if(!tf) printf("Error opening input file!\n");
  NGMSystemConfiguration* conf = (NGMSystemConfiguration*)(tf->Get("NGMSystemConfiguration"));
  if(!conf) printf("Error: Configuration object was not found!\n");
  conf->GetDetectorParameters()->ImportFromTextFile("/usr/gapps/ngm/data/run7/DetectorParsPUCal.txt");
  tf->WriteTObject(conf,"NGMSystemConfiguration");
  tf->Close();
  delete tf;
}
