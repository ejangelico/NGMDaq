void anaOnline(const char* filename = "")
{
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
    //mFlood = new NGMBlockFlood("Flood","Flood");
    NGMBlockArrayMonitor* mBlockArray = 0;
    mBlockArray = new NGMBlockArrayMonitor("BlockArray","BlockArray");
    NGMPixelADCMonitor* mPixelADC = 0;
    //mPixelADC = new NGMPixelADCMonitor("PixelADC","PixelADC");
    //NGMWaterfall* mWf = new NGMWaterfall("waterfall","waterfall");
    mSort->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
    fin->Add(mCal);
    mCal->Add(mSort);
    mSort->Add(mFilt);
    if(mOut) mSort->Add(mOut);
    if(mBlockArray) mFilt->Add(mBlockArray);
    if(mPixelADC)
    {
      mPixelADC->SetMaxADC(1000);
      mFilt->Add(mPixelADC);
    }
    if(mFlood) mFilt->Add(mFlood);
    
    //Good cuts for run 20110629180428
    // AmBe first prod detector
    //mFilt->_baselinelow[0] = 1600.0;
    //mFilt->_baselinehigh[0] = 1700.0;

    
    mFilt->SetAccept(0,1);
    mFilt->SetAccept(4,1);
    mFilt->SetAccept(8,1);
    
    for(int ibl = 0; ibl <3*4; ibl++)
    {
      mFilt->_baselinelow[ibl] = 0;
      mFilt->_baselinehigh[ibl] = 3000.0;    
      mFilt->_energylow[ibl] = 150;
      mFilt->_energyhigh[ibl] = 3000;
      mFilt->_neutroncutlow[ibl] = 0.0;
      mFilt->_neutroncuthigh[ibl] = 1.0;
    }
    
    //mSort->Add(mWf);
    mSort->SetPlotFrequency(1000.0);
    fin->initModules();
    fin->LaunchSpyServ();
    gROOT->Add(NGMSystem::getSystem()->GetParentFolder());
  }
  
  
  TString fname(filename);
  if(fname == "")
  {
    std::string fstr;
    gROOT->ProcessLine(".! ls -rt P16*.root | tail -n1>.lastfile");
    ifstream if(".lastfile");
    if>>fstr;
    fname=fstr.c_str();
  }
	std::cout<<fname.Data()<<std::endl;
	fin->OpenInputFile(fname);
	fin->StartAcquisition();
}


