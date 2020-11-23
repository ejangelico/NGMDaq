void extractPSD(const char* fname)
{
  TFile *fin = TFile::Open(fname);
  TFolder* MultiFormatReaderFolder = (TFolder*)(fin->Get("MultiFormatReaderFolder"));
  NGMSystemConfiguration* sysConf = NGMSystem::getSystem()->GetConfiguration();
  NGMNeutronMonitor* nmon = MultiFormatReaderFolder->FindObjectAny("NMON");
  nmon->fitNeutronGammaBands("SBA","NMON",sysConf,"NGMCAL_SBA_PSD.root");

}
