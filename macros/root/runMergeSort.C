void runMergeSort(const char* filename)
{

	NGMPacketBufferIO* inputFile = new NGMPacketBufferIO("input","input");
	NGMPacketFilter* pfilter = new NGMPacketFilter("Run5Filter","Run5Filter");
	NGMPacketMergeSort* ngmsort = new NGMPacketMergeSort("ngmsort","ngmsort");
	NGMAnalysisInput* ana = new NGMAnalysisInput("ana","ana");
	NGMTimeMonitor* time = new NGMTimeMonitor("time","time");
  NGMHitOutputFile* outFile = new NGMHitOutputFile("Run6","Run6");
  
	//Chain our analysis modules
	inputFile->Add(pfilter);
	//inputFile->Add(ngmsort);
	pfilter->Add(ngmsort);
	ngmsort->Add(outFile);
  ngmsort->Add(ana);
  ana->Add(time);
	
	// Initialze module and daughter modules
	inputFile->initModules();
	ngmsort->setVerbosity(11);
	
	// Add a list of channels to skip
	// Bad Channel
	pfilter->addSkipChannel(0,-1);
	pfilter->addSkipChannel(1,-1);
	pfilter->addSkipChannel(2,-1);
	pfilter->addSkipChannel(3,-1);
	pfilter->addSkipChannel(4,-1);
	pfilter->addSkipChannel(5,-1);
	pfilter->addSkipChannel(6,-1);
	pfilter->addSkipChannel(7,-1);
  pfilter->addSkipChannel(9,-1);
  pfilter->addSkipChannel(10,-1);
  pfilter->addSkipChannel(11,-1);
  pfilter->addSkipChannel(12,-1);
  //pfilter->addSkipChannel(10,4);
  //pfilter->addSkipChannel(10,5);
  //pfilter->addSkipChannel(10,6);
  //pfilter->addSkipChannel(10,7);
  //pfilter->addSkipChannel(11,1);
	//pfilter->addSkipChannel(8,-1);
	//pfilter->addSkipChannel(9,-1);
	//pfilter->addSkipChannel(10,-1);
	//pfilter->addSkipChannel(11,-1);
	//pfilter->addSkipChannel(12,-1);
	// Not Connected
//	pfilter->addSkipChannel(10,0);
//	pfilter->addSkipChannel(10,1);
//	pfilter->addSkipChannel(10,2);
	// Unused Channel
//	pfilter->addSkipChannel(12,1);
//	pfilter->addSkipChannel(12,2);
//	pfilter->addSkipChannel(12,3);
//	pfilter->addSkipChannel(12,4);
//	pfilter->addSkipChannel(12,6);
//	pfilter->addSkipChannel(12,7);


	inputFile->openInputFile(filename);
	ngmsort->setRequiredSlots(8);
	int ievent = 0;
	while(inputFile->readBuffer(ievent++)){std::cout<<ievent<<std::endl; if(ievent>416) break;}
	
	inputFile->finishModules();
	
	outFile->closeOutputFile();

}
