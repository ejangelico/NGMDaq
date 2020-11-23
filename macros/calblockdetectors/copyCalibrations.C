void copyCalibrations(const char* fin, const char* fout)
{
  TFile* tfout = TFile::Open(fout,"UPDATE");
  if(!tfout)
  {
    std::cerr<<"Unable to open output file "<<fout<<std::endl;
  }
  std::cout<<"Fetching configuration from "<<fout<<std::endl;
  NGMSystemConfiguration* confout = (NGMSystemConfiguration*)(tfout->FindObjectAny("NGMSystemConfiguration"));

  TFile* tfin = TFile::Open(fin);
  if(!tfin)
  {
    std::cerr<<"Unable to open input file "<<fin<<std::endl;
  }
  std::cout<<"Fetching configuration from "<<fin<<std::endl;
  NGMSystemConfiguration* confin = (NGMSystemConfiguration*)(tfin->FindObjectAny("NGMSystemConfiguration"));
  
  
  if(confout->GetDetectorParameters()->GetParIndex("BlockEnergyCal")<0)
    confout->GetDetectorParameters()->AddParameterO("BlockEnergyCal",0);
  for(int idet=0; idet < confout->GetDetectorParameters()->GetEntries(); idet++)
  {
    TString detName(confout->GetDetectorParameters()->GetParValueS("DetectorName",idet));
    int sourceIndex = confin->GetDetectorParameters()->FindFirstRowMatching("DetectorName",detName);
    if(sourceIndex<0) continue;
    confout->GetDetectorParameters()->SetParameterO("BlockEnergyCal",idet,
                                                    confin->GetDetectorParameters()->GetParValueO("BlockEnergyCal",sourceIndex));
  }
  tfout->WriteTObject(confout,"NGMSystemConfiguration");
}