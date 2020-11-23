void runTimingMon(const char* infilename)
{
NGMPacketBufferIO* infile = new NGMPacketBufferIO("input","input");
  NGMPacketMergeSort* merge = new NGMPacketMergeSort("Merge","Merge");
NGMAnalysisInput* ana = new NGMAnalysisInput("ana","ana");
NGMNeutronMonitor* mon = new NGMNeutronMonitor("mon","mon");
NGMTimeMonitor* time = new NGMTimeMonitor("time","time");
NGMHitOutputFile* hitout = new NGMHitOutputFile("HitOutput","HitOutput");
//NGMComptonEdgeFinder* cpf = new NGMComptonEdgeFinder("CPF","CPF");
infile->Add(merge);
merge->Add(ana);
merge->Add(hitout);
merge->setVerbosity(0);
//ana->Add(cpf);
ana->Add(time);
infile->initModules();

infile->openInputFile(infilename);
merge->setRequiredSlots(6);
new TBrowser;
 gStyle->SetPalette(1);
 int bufcnt = 0;
 while(infile->readNextBuffer()){
   std::cout<<"Buffer "<<bufcnt++<<std::endl;
   gSystem->ProcessEvents();
 //  if(bufcnt==55) break;
 }
 //for(int ibuf = 0; ibuf < 112; ibuf++){
 //  infile->readBuffer(ibuf);
 //}
 infile->finishModules();
 NGMAnalysisOutput::instance()->closeOutputFile();
}
