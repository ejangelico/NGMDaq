void updateRunStats(const char* flist)
{
   char fname[1024];
   ifstream infilelist(flist);
   
   while(infilelist>>fname)
   {
      TFile* aFile = TFile::Open(fname);
      if(!aFile) continue;
      // Lets get the configuration Object
      NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*)( aFile->Get("NGMSystemConfiguration"));
      
	  NGMNeutronMonitor::UpdateDatabaseRunHeader(sysConf,aFile,gSystem->Getenv("NGMDBPW"));
      delete aFile;
      
   }
}