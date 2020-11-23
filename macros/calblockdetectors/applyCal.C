void applyCal(const char* fname)
{
  TFile* filein = TFile::Open(fname);
  NGMSystemConfiguration* conf = (NGMSystemConfiguration*)(filein->FindObjectAny("NGMSystemConfiguration"));
  NGMConfigurationTable* sys = conf->GetSystemParameters();
  sys->AddParameterI("BLOCKNROWS",1,0,4);
  sys->AddParameterI("BLOCKNCOLS",3,0,6);
  sys->SetParameterToDefault("BLOCKNROWS");
  sys->SetParameterToDefault("BLOCKNCOLS");
  NGMConfigurationTable* chan = conf->GetChannelParameters();
  chan->AddParameterS("DetectorName","NA");
  chan->SetParameterS("DetectorName",0,"LSBLOCKA_00");
  chan->SetParameterS("DetectorName",4,"LSBLOCKA_01");
  chan->SetParameterS("DetectorName",8,"LSBLOCKA_02");
  NGMConfigurationTable* det = conf->GetDetectorParameters();
  det->AddParameterS("DetectorName","NA");
  det->AddParameterO("FLOOD_LUT",0);
  det->AddParameterI("BLOCK_ROW",0,0,1000);
  det->AddParameterI("BLOCK_COL",0,0,1000);
  
  det->SetParameterS("DetectorName",0,"LSBLOCKA_00");
  det->SetParameterI("BLOCK_ROW",0,0);
  det->SetParameterI("BLOCK_COL",0,0);

  det->SetParameterS("DetectorName",1,"LSBLOCKA_01");
  det->SetParameterI("BLOCK_ROW",1,0);
  det->SetParameterI("BLOCK_COL",1,1);
  
  det->SetParameterS("DetectorName",2,"LSBLOCKA_02");
  det->SetParameterI("BLOCK_ROW",2,0);
  det->SetParameterI("BLOCK_COL",2,2);
  
  
  TFile* tf = TFile::Open("OnebythreeFlood.root");
  TH2I* Flood_LUT_00 = (TH2I*)(tf->FindObjectAny("Flood_LUT_00"));
  TH2I* Flood_LUT_01 = (TH2I*)(tf->FindObjectAny("Flood_LUT_01"));
  TH2I* Flood_LUT_02 = (TH2I*)(tf->FindObjectAny("Flood_LUT_02"));
  Flood_LUT_00->SetDirectory(0);
  Flood_LUT_01->SetDirectory(0);
  Flood_LUT_02->SetDirectory(0);
  tf->Close();
  
  det->SetParameterO("FLOOD_LUT",0,Flood_LUT_00);
  det->SetParameterO("FLOOD_LUT",1,Flood_LUT_01);
  det->SetParameterO("FLOOD_LUT",2,Flood_LUT_02);
  
  TString outName(gSystem->BaseName(fname));
  outName.ReplaceAll(".root","-cal.root");
  TFile* fout = TFile::Open(outName.Data(),"NEW");
  fout->WriteTObject(conf,"NGMSystemConfiguration");
  fout->Close();

}