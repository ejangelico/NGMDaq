void runNeutronMonFromHits(const char* infilename)
{
  NGMHitIO* infile = new NGMHitIO("input","input");
NGMAnalysisInput* ana = new NGMAnalysisInput("ana","ana");
NGMNeutronMonitor* mon = new NGMNeutronMonitor("mon","mon");
//NGMTimeMonitor* time = new NGMTimeMonitor("time","time");
//NGMHitOutputFile* hitout = new NGMHitOutputFile("HitOutput","HitOutput");
//NGMComptonEdgeFinder* cpf = new NGMComptonEdgeFinder("CPF","CPF");
infile->Add(ana);
ana->Add(mon);

infile->initModules();

infile->openInputFile(infilename);

 mon->ReadNeutronCuts("ROIs10sig.root","NMON");
 mon->setGeneratorIndex(96);
new TBrowser;
 gStyle->SetPalette(1);
 int bufcnt = 0;
 while(infile->readNextBuffer()){
   bufcnt++;
   if(bufcnt%100000 == 0)
   {
     std::cout<<"Buffer "<<bufcnt<<std::endl;
     gSystem->ProcessEvents();
   }
 //  if(bufcnt==55) break;
 }

 infile->finishModules();
 NGMAnalysisOutput::instance()->closeOutputFile();
 double neutrons10sig = 0;
 double neutrons5sig = 0;
 mon->UpdateTotalNeutrons();
 TH1* NMON_TotalNeutrons = (TH1*) (gROOT->FindObject("mon_TotalNeutrons"));
 if(NMON_TotalNeutrons)
 {
	neutrons10sig = NMON_TotalNeutrons->Integral(1, NMON_TotalNeutrons->GetNbinsX());
 }
 
// mon->ReadNeutronCuts("ROIs5sig.root","NMON"); 
// mon->UpdateTotalNeutrons();
// if(NMON_TotalNeutrons)
// {
//	neutrons5sig = NMON_TotalNeutrons->Integral(1, NMON_TotalNeutrons->GetNbinsX());
// }
 
 std::cout<<"Total Neutrons -- 10sig( "<<neutrons10sig<<") "<<std::endl;
 std::cout<<"Total Neutrons -- 5sig( "<<neutrons5sig<<") "<<std::endl;
}
