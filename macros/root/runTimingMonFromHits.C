void runTimingMonFromHits(const char* infilename)
{
NGMAnalysisInput* ana = new NGMAnalysisInput("ana","ana");
NGMCalibrator* calM = new NGMCalibrator("calM","calM");
NGMTimeMonitor* time = new NGMTimeMonitor("time","time");
NGMHitIO* infile = new NGMHitIO("hitinput","hitinput");
//NGMComptonEdgeFinder* cpf = new NGMComptonEdgeFinder("CPF","CPF");
infile->Add(calM);
calM->Add(ana);
//infile->Add(ana);
ana->Add(time);

infile->initModules();
calM->setDoSISTimeFix(true);
calM->ReadNeutronCuts("Run5NeutronCuts.root","nmon");
time->setMaxNanoSecInList(1E3);
infile->openInputFile(infilename);
new TBrowser;
 gStyle->SetPalette(1);
 int bufcnt = 0;
 while(infile->readNextBuffer()){
	bufcnt++;
	if(bufcnt%10000 == 0){
		gSystem->ProcessEvents();
		std::cout<<"Buffer "<<bufcnt<<std::endl;
	}
   if(bufcnt>=450000) break;
 }
 //for(int ibuf = 0; ibuf < 112; ibuf++){
 //  infile->readBuffer(ibuf);
 //}
 infile->finishModules();
 NGMAnalysisOutput::instance()->closeOutputFile();
}
