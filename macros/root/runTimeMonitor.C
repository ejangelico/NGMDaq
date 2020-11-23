void runTimeMonitor(const char* filename)
{

	NGMHitIO* input = new NGMHitIO("input","input");
	NGMAnalysisInput* ana = new NGMAnalysisInput("NGMAna","NGMAna");
	input->Add(ana);
	NGMTimeMonitor* tmon = new NGMTimeMonitor("tmon","tmon");
	ana->Add(tmon);
  NGMNeutronMonitor* nmon = new NGMNeutronMonitor("nmon","nmon");
  ana->Add(nmon);
	
	input->initModules();
	
	input->openInputFile(filename);
	nmon->setGeneratorIndex(-1);
  
	gROOT->Add(input);

	new TBrowser;

	int ievent = 0;
	while(	  input->readNextBuffer();){
	  if(ievent%10000 ==0 ){ std::cout<<ievent<<std::endl; gSystem->ProcessEvents(); }

    ievent++;
	}
	
  input->finishModules();
	
  NGMAnalysisOutput::instance()->closeOutputFile();
}
