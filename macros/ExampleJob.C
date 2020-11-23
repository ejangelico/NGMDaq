// $Id: ExampleJob.C 600 2008-11-03 22:44:22Z barnes $
// $Author: barnes $

void ExampleJob(const char* fname)
{

//______________________________________________________________________________
//______________________________________________________________________________
//
//    Example analysis job
//
//    Run this like
//
//    NGMBatch [] .L macros/ExampleJob.C 
//
//    NGMBatch [] ExampleJob("Test/ExampleJob.root")
//
//______________________________________________________________________________

  // The Analyzer
  NGMAnalysisInput* ana = new NGMAnalysisInput("ExampleJob","ExampleJob");
  
//______________________________________________________________________________
//
//    Define and configure the analysis modules

  void commonCountDistR_config(NGMCountDistR *cd)
  {
	// some common defaults
	cd->SetAllowDatabaseReads();
	cd->SetDisplayInterval(1E10);
	cd->setAllGraphs(true);
	cd->setGateInterval(1);
	cd->setPromptStepForY2FandZ2F(true);
	cd->useFastNeutronEquation(false);
  }
  
  //  Module:  MultipleScattering
  NGMMultipleScattering* mult = new NGMMultipleScattering("mult","mult");

  // Module:  CountDistR for Liquid Gammas
  NGMCountDistR* countDistG = new NGMCountDistR("CDISTGAMMA","CDISTGAMMA");
  commonCountDistR_config(countDistG);
  countDistG->getParticleIdent()
	->AddCut(NGMSimpleParticleIdent::lsgamma, -1, 0.0);
  
  // Module:  CountDistR for Liquid Neutrons
  NGMCountDistR* countDistN = new NGMCountDistR("CDISTFN","CDISTFN");
  commonCountDistR_config(countDistN);
  countDistN->getParticleIdent()
	->AddCut(NGMSimpleParticleIdent::lsneutron, -1, 0.0);
  countDistN->useFastNeutronEquation(true);
  
  // Module:  CountDistR for He-3 Neutrons
  NGMCountDistR* countDistH = new NGMCountDistR("CDISTHe","CDISTHe");
  commonCountDistR_config(countDistH);
  countDistH->getParticleIdent()
	->AddCut(NGMSimpleParticleIdent::hettlid, -1, 0.0);
  countDistH->setGateInterval(10);
  countDistH->setPromptStepForY2FandZ2F(false);
  
//______________________________________________________________________________
//
//    Define the analysis chain

  // Start of chain:  data source
  //    Going to read from a file
  NGMHitIO* fin = new NGMHitIO("fin","fin");

  fin->Add(ana);          // Add the analyzer to the chain
  ana->Add(mult);         // add module in series
  mult->Add(countDistG);  // add multiple modules (feed data to multiple modules)
  mult->Add(countDistN);  
  mult->Add(countDistH);  

  
//______________________________________________________________________________
//
//    And away we go

  fin->initModules();
  fin->openInputFile(fname);
  
  fin->run();

  fin->sendEndRunFlushAndSave();
  fin->closeInputFile();
  
  printf("\07\n");  // beep the terminal to wake you up again
}
