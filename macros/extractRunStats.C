void extractRunStats(const char* flist)
{
   char fname[1024];
   ifstream infilelist(flist);
   ofstream outtable("output.txt");
   // The following fields are extracted from the analysis files:
   outtable<<"Run Number"<<"\t";
   outtable<<"Start time"<<"\t";
   outtable<<"Raw Waveforms"<<"\t";
   outtable<<"Double/Single Buffering"<<"\t";
   outtable<<"BufferFraction"<<"\t";
   outtable<<"Livetime%"<<"\t";
   outtable<<"Elapsed Time"<<"\t";
   outtable<<"Detector Configuration"<<"\t";
   outtable<<"Source"<<"\t";
   outtable<<"Shielding"<<"\t";
   outtable<<"Thresholds GS"<<"\t";
   outtable<<"Thresholds GL"<<"\t";
   outtable<<"Thresholds LS"<<"\t";
   outtable<<"Thresholds MU"<<"\t";
   outtable<<"Rates System"<<"\t";
   outtable<<"Rates GS"<<"\t";
   outtable<<"Rates GL"<<"\t";
   outtable<<"Rates LS"<<"\t";
   outtable<<"Rates MU"<<"\t";
   outtable<<"Rates HE"<<"\t";
   outtable<<"Runtime Comment"<<"\t";
   outtable<<"Comment"<<std::endl;
   
   
   
   while(infilelist>>fname)
   {
      TFile* aFile = TFile::Open(fname);
      if(!aFile) continue;
      // Lets get the configuration Object
      NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*)( aFile->Get("NGMSystemConfiguration"));
      
      TString runNumber;
      runNumber+=sysConf->getRunNumber();
      TString startTime;
      startTime = sysConf->GetTimeStamp().AsString("cl"); // compact local time (PST)
      startTime = TString(startTime(0,startTime.Index(".",0)));

      TString saveWaveforms;
      if(sysConf->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 0)
      {
         saveWaveforms = "Never";
      }else if(sysConf->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 1)
      {
         saveWaveforms = "Always";
      }else if(sysConf->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 2)
      {
         saveWaveforms = "BeginOfBuffer";
      }else if(sysConf->GetSlotParameters()->GetParValueI("RawDataSampleMode",0) == 3)
      {
         saveWaveforms = "OnPileup";
      }
      
      TString sdBuffering;
      if(sysConf->GetSystemParameters()->GetParValueI("RunReadoutMode",0) == 1)
      {
         sdBuffering = "Single";
      }else if(sysConf->GetSystemParameters()->GetParValueI("RunReadoutMode",0) == 2)
      {
         sdBuffering = "Double";
      }      
      
      double bufferFillFraction = sysConf->GetSystemParameters()->GetParValueD("BufferFractionPerSpill",0);
      
      TH1* hRunStat = (TH1*)(aFile->Get("NMON_RunStat"));
      TString selapsedTime;
      TString sliveTimeFraction;
      double elapsedTime = hRunStat->GetBinContent(2);
      double liveTime = hRunStat->GetBinContent(1);
      selapsedTime=TString::Format("%.1f",elapsedTime);
      sliveTimeFraction=TString::Format("%.2f",liveTime/elapsedTime*100.0);
      
      // Hardware thresholds
      TString sGSThreshold = sysConf->GetChannelParameters()->LookupParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","GS01");
      TString sGLThreshold = sysConf->GetChannelParameters()->LookupParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","GL01");
      TString sLSThreshold = sysConf->GetChannelParameters()->LookupParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","LS01");
      TString sMUThreshold = sysConf->GetChannelParameters()->LookupParValueAsString("FirTriggerThresholdValue_Conf","DetectorName","MU01");
      
      // Lets examine rates
      double gsrates = 0.0;
      double glrates = 0.0;
      double lsrates = 0.0;
      double herates = 0.0;
      double murates = 0.0;
      TH1* hTotalCounts = (TH1*)(aFile->Get("NMON_totalCounts"));
      for(int ichan = 0; ichan < sysConf->GetChannelParameters()->GetEntries(); ichan++)
      {
         TString chanName = sysConf->GetChannelParameters()->GetParValueS("DetectorName",ichan);
         if(chanName.BeginsWith("GS"))
         {
            gsrates+=hTotalCounts->GetBinContent(ichan+1);
         }else if(chanName.BeginsWith("GL"))
         {
            glrates+=hTotalCounts->GetBinContent(ichan+1);
         }else if(chanName.BeginsWith("LS"))
         {
            lsrates+=hTotalCounts->GetBinContent(ichan+1);
         }else if(chanName.BeginsWith("HE"))
         {
            herates+=hTotalCounts->GetBinContent(ichan+1);
         }else if(chanName.BeginsWith("MU"))
         {
            murates+=hTotalCounts->GetBinContent(ichan+1);
         }
      }
      gsrates*=1.0/liveTime;
      glrates*=1.0/liveTime;
      lsrates*=1.0/liveTime;
      herates*=1.0/liveTime;
      murates*=1.0/liveTime;
      double systemRates = gsrates + glrates + lsrates + herates + murates;
      
      TString runTimeComment = sysConf->GetSystemParameters()->GetParValueS("Comment",0);
      
      // Write this record
      outtable<<  runNumber.Data() <<"\t"; //"Run Number";
      outtable<< startTime.Data() <<"\t"; //"Start time";
      outtable<< saveWaveforms.Data() <<"\t"; //"Raw Waveforms";
      outtable<< sdBuffering.Data() <<"\t"; //"Double/Single Buffering";
      outtable<< TString::Format("%.0f",bufferFillFraction*100.0).Data() <<"\t"; // Buffer Fill Fraction
      outtable<< sliveTimeFraction.Data() <<"\t"; //"Livetime%";
      outtable<< selapsedTime.Data() <<"\t"; //"Elapsed Time";
      outtable<< "" <<"\t"; //"Detector Configuration";
      outtable<< "" <<"\t"; //"Source";
      outtable<< "" <<"\t"; //"Shielding";
      outtable<< sGSThreshold.Data() <<"\t"; //"Thresholds GS";
      outtable<< sGLThreshold.Data() <<"\t"; //"Thresholds GL";
      outtable<< sLSThreshold.Data() <<"\t"; //"Thresholds LS";
      outtable<< sMUThreshold.Data() <<"\t"; //"Thresholds MU";
      outtable<< TString::Format("%.2f",systemRates).Data() <<"\t"; //"Rates System";
      outtable<< TString::Format("%.2f",gsrates).Data() <<"\t"; //"Rates GS";
      outtable<< TString::Format("%.2f",glrates).Data() <<"\t"; //"Rates GL";
      outtable<< TString::Format("%.2f",lsrates).Data() <<"\t"; //"Rates LS";
      outtable<< TString::Format("%.2f",murates).Data() <<"\t"; //"Rates MU";
      outtable<< TString::Format("%.2f",herates).Data() <<"\t"; //"Rates HE";
      outtable<< runTimeComment.Data() <<"\t"; //"Runtime Comment";
      outtable<< "" <<"\t"; //"Comment";
      outtable<<  std::endl;
      
      delete aFile;
      
   }
   outtable.close();
}