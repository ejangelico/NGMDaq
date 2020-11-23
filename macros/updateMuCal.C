void updateMuCal(const char* fname)
{
	
	// Find Timing Calibrations
	TFile* fTiming = TFile::Open("TimingCal.root");
	NGMSystemConfiguration* timeConf = 0;
	if(fTiming)
	{
		timingConf = (NGMSystemConfiguration*)( fTiming->Get("NGMSystemConfiguration") );
	}
	
	bool updateTiming  = false;
	if(timingConf)
	{
		if(timingConf->GetDetectorParameters()->GetParIndex("CalTimingOffset")>=0) updateTiming = true;
	}
	TFile* modFile = TFile::Open(fname,"UPDATE");
	if(!modFile) return;
	
	NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*)(modFile->Get("NGMSystemConfiguration"));
	if(!sysConf){ std::cerr<<"Configuration not found!\n"; return; }
	
	NGMConfigurationTable* cDetector = sysConf->GetDetectorParameters();
	cDetector->AddParameterD("MuonThreshold", 10000.0, -1.0, 100000.0);
	cDetector->SetParameterToDefault("MuonThreshold");
	cDetector->SetParameterAsStringThatBeginsWith("calSourceEnergy","10000.0","DetectorName","MU");
	cDetector->SetParameterAsStringThatBeginsWith("calSourceComptonEdge","1000.0","DetectorName","MU");	
	
	if(updateTiming)
	{
		if( cDetector->GetParIndex("CalTimingOffset")<0 )
		{
			cDetector->AddParameterD("CalTimingOffset",0.0,-1E10,1E10);
		}
		
		timingConf->GetDetectorParameters()->CopyParValuesMatching("CalTimingOffset","CalTimingOffset", "DetectorName",
																		"", cDetector);

	}
	
	modFile->WriteTObject(sysConf,"NGMSystemConfiguration");
	modFile->Close();
}