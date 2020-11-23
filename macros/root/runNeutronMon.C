void runNeutronMon(const char* infilename)
{
NGMPacketBufferIO* infile = new NGMPacketBufferIO("input","input");
NGMAnalysisInput* ana = new NGMAnalysisInput("ana","ana");
NGMNeutronMonitor* mon = new NGMNeutronMonitor("NMON","NMON");
NGMTimeMonitor* time = new NGMTimeMonitor("time","time");
//NGMComptonEdgeFinder* cpf = new NGMComptonEdgeFinder("CPF","CPF");
infile->Add(ana);
//ana->Add(cpf);
ana->Add(mon);
ana->Add(time);
infile->initModules();

gROOT->Add(infile);

infile->openInputFile(infilename);
new TBrowser;
 gStyle->SetPalette(1);
 int bufcnt = 0;
 while(infile->readNextBuffer()){
	 std::cout<<"Buffer "<<bufcnt++<<std::endl;
	gSystem->ProcessEvents();
   //if(bufcnt>=72) break;
	}
// for(int ibuf = 0; ibuf < 112; ibuf++){
//   infile->readBuffer(ibuf);
// }
 infile->finishModules();
 NGMAnalysisOutput::instance()->closeOutputFile();

 mon->ReadNeutronCuts("ROIs10sig.root");
 mon->UpdateTotalNeutrons();
 TH1* hNeutrons = (TH1*)(gROOT->FindObject("NMON_TotalNeutrons"));
 double totalNeutrons10sig = hNeutrons->Integral();
 mon->ReadNeutronCuts("ROIs5sig.root");
 mon->UpdateTotalNeutrons();
 double totalNeutrons5sig = hNeutrons->Integral();
 std::cout<<"Total Neutrons 10sig gamma cut: "<<totalNeutrons10sig<<std::endl;
 std::cout<<"Total Neutrons 5sig gamma cut: "<<totalNeutrons5sig<<std::endl;

}
