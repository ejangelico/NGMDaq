void printCalibrationValues(const char* fname = "NGMComptonEdgeFinderParameters20061213235753.root")
{
	TFile* calFile = TFile::Open(fname);
	NGMConfigurationTable* calTable = (NGMConfigurationTable*)(calFile->Get("NGMComptonEdgeFinderParameters"));
	
	std::ofstream outfile("cal.csv");
	for(int ichan = 0; ichan < 32; ichan++)
	{
		outfile << ichan <<", "
			<< calTable->GetParValueD("comptonEdge",ichan)
			<<std::endl;
	}
	outfile.close();
	return;
}