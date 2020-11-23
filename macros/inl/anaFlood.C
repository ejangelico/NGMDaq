void anaFlood(const char* filename = "")
{

  TStopwatch *timer = new TStopwatch();
  timer->Start();

  NGMMultiFormatReader* fin = NGMSystem::getSystem();
  
  // Only setup the analysis tree once per root session
  if(!fin)
  {
   fin = new NGMMultiFormatReader();
    gStyle->SetPalette(1);
    gStyle->SetOptStat(0);

    NGMBlockDetectorCalibrator* mCal = new NGMBlockDetectorCalibrator("cal","cal");
    NGMPacketMergeSort* mSort = new NGMPacketMergeSort("mSort","mSort");
    NGMHitOutputFile* mOut = 0;
    //mOut = new NGMHitOutputFile("HitOut","HitOut");
    NGMHitFilter* mFilt = new NGMHitFilter("mFilt","mFilt");
    	  
	  NGMBlockFlood* mFlood = 0;
    mFlood = new NGMBlockFlood("Flood","Flood");
    NGMBlockArrayMonitor* blockMon = 0;
    //blockMon = new NGMBlockArrayMonitor("Hausladen","Hausladen");
    NGMPixelADCMonitor* pixADC = 0;
    //pixADC = new NGMPixelADCMonitor("pixADC","pixADC");
    //NGMPacketOutputFile* pOut = new NGMPacketOutputFile("pOut","pOut");
    //fin->Add(pOut);
    mSort->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
    //mSort->setVerbosity(11);
    fin->Add(mCal);
    mCal->Add(mSort);
    mSort->Add(mFilt);
    
    if(blockMon) mFilt->Add(blockMon);
    if(mOut) mFilt->Add(mOut);
    if(mFlood) mFilt->Add(mFlood);
    if(pixADC) mFilt->Add(pixADC);
    
    //Good cuts for run 20110629180428
    // AmBe first prod detector
    //mFilt->_baselinelow[0] = 1600.0;
    //mFilt->_baselinehigh[0] = 1700.0;

    for(int ibl = 0; ibl <64; ibl++)
    {
      mFilt->SetAccept(ibl,1);
      mFilt->_baselinelow[ibl] = 0;
      mFilt->_baselinehigh[ibl] = 3000.0;    
      mFilt->_energylow[ibl] = 500;
      mFilt->_energyhigh[ibl] = 4000;
//      mFilt->_neutroncutlow[ibl] = -2.0;
//      mFilt->_neutroncuthigh[ibl] = 2.0;//0.992;
//      mFilt->_gammacutlow[ibl] = -1e9;
//      mFilt->_gammacuthigh[ibl] = -4.0;//0.992;
      mFilt->_neutroncutlow[ibl] = -2.0;
      mFilt->_neutroncuthigh[ibl] = 2.0;//0.992;
      mFilt->_gammacutlow[ibl] = -4.0;
      mFilt->_gammacuthigh[ibl] = -1e9;//0.992;
    }
    
    //mSort->Add(mWf);
    mSort->SetPlotFrequency(100.0);
    fin->initModules();
    //fin->LaunchSpyServ();
    gROOT->Add(NGMSystem::getSystem()->GetParentFolder());

    TCanvas *Flood_Canvas = new TCanvas("Flood_Canvas", "Flood_Canvas",244,92,1313,715);
    Flood_Canvas->ToggleEventStatus();
    Flood_Canvas->SetFillColor(0);
    Flood_Canvas->SetFrameFillColor(0);

  }
  
  
  TString fname(filename);
  if(fname == "")
  {
    std::string fstr;
    gROOT->ProcessLine(".! ls -rt P16*.root | tail -n1>.lastfile");
    ifstream if(".lastfile");
    if>>fstr;
    //    fstr = "data/" + fstr;
    fname=fstr.c_str();
  }
  std::cout<<fname.Data()<<std::endl;
  fin->OpenInputFile(fname);
  fin->StartAcquisition();
  
  printf("\nReplay took %.0f sec\n\n",timer->RealTime());


}


