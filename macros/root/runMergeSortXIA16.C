void runMergeSortXIA16(const char* filename)
{

	NGMPacketBufferIO* inputFile = new NGMPacketBufferIO("input","input");
	NGMPacketFilter* pfilter = new NGMPacketFilter("Run5Filter","Run5Filter");
	NGMPacketMergeSort* ngmsort = new NGMPacketMergeSort("ngmsort","ngmsort");
	NGMHitOutputFile* outFile = new NGMHitOutputFile("XIA","XIA");
	//Chain our analysis modules
	//inputFile->Add(pfilter);
	inputFile->Add(ngmsort);
	//pfilter->Add(ngmsort);
	ngmsort->Add(outFile);
	
	// Initialze module and daughter modules
	inputFile->initModules();
	ngmsort->setVerbosity(11);

	inputFile->openInputFile(filename);
	ngmsort->setRequiredSlots(6);
	int ievent = 0;
	while(inputFile->readBuffer(ievent++)){std::cout<<ievent<<std::endl;}
	
	inputFile->finishModules();
	
	outFile->closeOutputFile();

}
