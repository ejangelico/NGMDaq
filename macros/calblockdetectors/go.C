void go()
{
	gSystem->Load("libPixie16Daq.so");
	gStyle->SetPalette(1);
	gStyle->SetOptStat(0);
	
	double gate[8];
	Pixie16Daq* p16 = new Pixie16Daq;
	NGMBlockDetectorCalibrator* mCal = new NGMBlockDetectorCalibrator("cal","cal");
	NGMPacketMergeSort* mSort = new NGMPacketMergeSort("mSort","mSort");
	NGMBlockFlood* mFlood = new NGMBlockFlood("Flood","Flood");
	
	NGMPacketOutputFile* pOut = new NGMPacketOutputFile("P16Out","P16Out");
	// NGMWaterfall* mWf = new NGMWaterfall("waterfall","waterfall");

	p16->Add(mCal);
	mCal->Add(mSort);
	mSort->Add(mFlood);
	// mSort->Add(mWf);
	mSort->SetPlotFrequency(10.0);
	mSort->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);


	p16->Add(pOut);
	p16->initModules();
	p16->InitializeSystem();
	p16->GetConfiguration()->GetSystemParameters()->SetParameterD("MaxDuration",0,60);

	p16->AdjustOffsets(0);
	//p16->AdjustOffsets(1);
	//p16->AdjustOffsets(2);
	//p16->AdjustOffsets(3);

	p16->SetDataFormat(0x1);
	for(int islot = 0; islot < 4; islot++)
	{
	  for(int ichan=0; ichan<16; ichan++)
	  {
	    // Anode Pulse???
	     // p16->WriteSglChanPar("QDCLen0",0.750,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen1",0.080,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen2",0.010,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen3",0.080,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen4",0.010,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen5",0.080,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen6",0.010,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen7",0.010,islot,ichan);
	     // Fast Anode Pulse
	     // p16->WriteSglChanPar("QDCLen0",0.74,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen1",0.12,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen2",0.08,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen3",0.02,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen4",0.40,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen5",0.60,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen6",0.010,islot,ichan);
	     // p16->WriteSglChanPar("QDCLen7",0.010,islot,ichan);
	     // // Long Tail Pulse
	     p16->WriteSglChanPar("QDCLen0",0.70,islot,ichan);
	     p16->WriteSglChanPar("QDCLen1",0.10,islot,ichan);
	     p16->WriteSglChanPar("QDCLen2",0.04,islot,ichan);
	     p16->WriteSglChanPar("QDCLen3",0.08,islot,ichan);
	     p16->WriteSglChanPar("QDCLen4",0.01,islot,ichan);
	     p16->WriteSglChanPar("QDCLen5",0.08,islot,ichan);
	     p16->WriteSglChanPar("QDCLen6",0.01,islot,ichan);
	     p16->WriteSglChanPar("QDCLen7",0.01,islot,ichan);
	     //Dont Save Traces
	     //p16->WriteSglChanPar("CHANNEL_CSRA",0x6a4,islot,ichan);
	     //Save Traces
	     p16->WriteSglChanPar("CHANNEL_CSRA",0x7a4,islot,ichan);

          }
	}


	p16->WriteGlobalToTable();
	gROOT->Add(NGMSystem::getSystem()->GetParentFolder());
	p16->LaunchGui();
	gSystem->Exec("xterm -e \"tail -f PIXIE16.log\" &");
	//p16->StartAcquisition();

	TCanvas *Flood_Canvas = new TCanvas("Flood_Canvas", "Flood_Canvas",866,73,700,520);
	Flood_Canvas->ToggleEventStatus();
	Flood_Canvas->SetFillColor(0);
	Flood_Canvas->SetFrameFillColor(0);

	TCanvas *cPixie16DaqBufferLevels = new TCanvas("cPixie16DaqBufferLevels", "cPixie16DaqBufferLevels",4,846,257,175);
	

}


