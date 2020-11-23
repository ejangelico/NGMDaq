// From Jason Newby, July 2016

/*
  notes about inheritance and file paths:

  TTask
    NGMModuleBase/NGMModule
      NGMModuleBase/NGMSystem
        NGMModuleCore/NGMMultiFormatReader 
        NGMModuleCore/NGMMultiFormatReader _reader = NGMSIS3316RawReader (public NGMReaderBase)
  
  NGMModuleCore/NGMReaderBase
    NGMModuleCore/NGMSIS3316RawReader


  NGMModuleBase/NGMModule
    NGMModuleCore/NGMHitIO
      NGMModuleCore/NGMHitOutputFile (in NGMHitIO.{h, cc})

NGMModuleCore/NGMSIS3316RawReader.cc fills out the NGMHit object
FIXME -- change NGMHitv6 _waveform to an array/vector? -- maybe make this v8?

*/

void toRoot(const char* fname = "")
{

  NGMMultiFormatReader* fin = (NGMMultiFormatReader*) NGMSystem::getSystem();
  // We check if it the analysis tree is already created as would be the case for
  // running the script in multiple times in the same root session
  if(!fin)
  {
    cout<<"Creating analysis tree "<<std::endl;
    fin = new NGMMultiFormatReader;
    fin->SetPassName("LIVE");
    NGMHitOutputFile* mHitOut = new NGMHitOutputFile("HitOut","HitOut");
    mHitOut->setBasePath("./");
    mHitOut->setBasePathVariable("");
    fin->Add(mHitOut);
    fin->initModules();
  }

  // If input filename is null assume we want to examine the most recent file
  TString sfname(fname);
  if(sfname == "")
  {
   // example name: SIS3316Raw_20160712204526_1.bin
   gSystem->Exec("ls -rt ./SIS3316Raw*_1.bin | tail -n1 > .lastsisfile.tmp");
    ifstream tmpf(".lastsisfile.tmp");
    sfname.ReadLine(tmpf);
    sfname = TString(sfname(0,sfname.Index("_1.bin"))).Data();
  }


  fin->OpenInputFile(sfname.Data());
    TStopwatch ts;
    ts.Start();
  fin->StartAcquisition(); 
  // calls NGMSIS3316RawReader::ReadAll(), where GetParent()->push() calls NGMNodule::process()
    ts.Print();
}
