#include <TMath.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TMatrixD.h>
#include <TVectorD.h>
#include <TDecompSVD.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

TH1F*    NormalizeHistogram(TH1F* Histogram);
TH1F*    TakeDerivative(TH1F* Histogram, int deltaX);
double   Find_Rising_Zero(TH1F* Histogram, int deltaX);
TH1F*    SmoothBoxcar(TH1F* Histogram, double widfrac);
TH1F*    TimesBinNumber(TH1F* Histogram);
double   Find2614(const char* FileName, const char* Detector);
double   HVGainSlope(int measurements, double* voltages, vector<string> &FileList, const char* Detector);
double   HVGainConstant(int measurements, double* voltages, vector<string> &FileList, const char* Detector);
TCanvas* PlotSpectra(int measurements, double* voltages, vector<string> &FileList, const char* Detector);
TGraph*  PlotGainVsVoltage(int measurements, double* voltages, vector<string> &FileList, const char* Detector);
TGraph*  PlotGainVsVoltage(int measurements, double* voltages, double* Bin2614);
TH2F*    SetupGainPlot(int measurements, double* x, double* y, const char* title, char* xaxis, char* yaxis);
TCanvas* MakeGainPlots(int measurements, int detectors, double* voltages, vector<string> &FileList, vector<string> DetectorList);
double   FindTargetHV(int measurements, double* voltages, vector<string> &FileList, const char* Detector, double Target_ADC_Counts_of_2614_Peak);


NGMSystemConfiguration* CalculateGainParameters()
{
   const int    NumberOfGainMeasurements = 4;
   
   double*       voltage = new double[NumberOfGainMeasurements];
   //voltage[0] = 0.9;
   voltage[0] = 1.0;
   voltage[1] = 1.1;
   voltage[2] = 1.2;
   voltage[3] = 1.3;

   vector<string> Filelist;
   //Filelist.push_back("HVScan900V20070824181458ngm.root");
   Filelist.push_back("HVScan1000V20070824183600ngm.root");
   Filelist.push_back("HVScan1100V20070824185707ngm.root");
   Filelist.push_back("HVScan1200V20070824191816ngm.root");
   Filelist.push_back("HVScan1299V20070824193923ngm.root");

//    Filelist.push_back("NGMAna20070515161915ngm.root");
//    Filelist.push_back("NGMAna20070515164927ngm.root");
//    Filelist.push_back("NGMAna20070515171938ngm.root");
//    Filelist.push_back("NGMAna20070515174950ngm.root");
//    Filelist.push_back("NGMAna20070515182001ngm.root");

   int detectors = 0;
   vector<string> Detectorlist;
//   Detectorlist.push_back("GL01");   detectors++;
//   Detectorlist.push_back("GL02");   detectors++;
//   Detectorlist.push_back("GL03");   detectors++;
//   Detectorlist.push_back("GL04");   detectors++;
//   Detectorlist.push_back("GL05");   detectors++;
//   Detectorlist.push_back("GL06");   detectors++;
//   Detectorlist.push_back("GL07");   detectors++;
//   Detectorlist.push_back("GL08");   detectors++;
//   Detectorlist.push_back("GL09");   detectors++;
//   //Detectorlist.push_back("GL10");   detectors++;
//   Detectorlist.push_back("GL11");   detectors++;
//   Detectorlist.push_back("GL12");   detectors++;
//   Detectorlist.push_back("GS01");   detectors++;
//   Detectorlist.push_back("GS02");   detectors++;
//   Detectorlist.push_back("GS03");   detectors++;
//   Detectorlist.push_back("GS04");   detectors++;
//   Detectorlist.push_back("GS05");   detectors++;
//   Detectorlist.push_back("GS06");   detectors++;
//   Detectorlist.push_back("GS07");   detectors++;
//   Detectorlist.push_back("GS08");   detectors++;
//   Detectorlist.push_back("GS09");   detectors++;
//   Detectorlist.push_back("GS10");   detectors++;
//   Detectorlist.push_back("GS11");   detectors++;
//   Detectorlist.push_back("GS12");   detectors++;
    Detectorlist.push_back("MU01");   detectors++;
    Detectorlist.push_back("MU02");   detectors++;
    Detectorlist.push_back("MU03");   detectors++;
    Detectorlist.push_back("MU04");   detectors++;
    Detectorlist.push_back("MU05");   detectors++;
    Detectorlist.push_back("MU06");   detectors++;
    Detectorlist.push_back("MU07");   detectors++;
    Detectorlist.push_back("MU08");   detectors++;
    Detectorlist.push_back("MU09");   detectors++;
    Detectorlist.push_back("MU10");   detectors++;
    Detectorlist.push_back("MU11");   detectors++;
    Detectorlist.push_back("MU12");   detectors++;
    Detectorlist.push_back("MU13");   detectors++;
    Detectorlist.push_back("MU14");   detectors++;
    Detectorlist.push_back("MU15");   detectors++;
    Detectorlist.push_back("MU16");   detectors++;
    Detectorlist.push_back("MU17");   detectors++;
    Detectorlist.push_back("MU18");   detectors++;
    Detectorlist.push_back("LS01");   detectors++;
    Detectorlist.push_back("LS02");   detectors++;
    Detectorlist.push_back("LS03");   detectors++;
    Detectorlist.push_back("LS04");   detectors++;
    Detectorlist.push_back("LS05");   detectors++;
    Detectorlist.push_back("LS06");   detectors++;
    Detectorlist.push_back("LS07");   detectors++;
    Detectorlist.push_back("LS08");   detectors++;
    Detectorlist.push_back("LS09");   detectors++;
    Detectorlist.push_back("LS10");   detectors++;
    Detectorlist.push_back("LS11");   detectors++;
    Detectorlist.push_back("LS12");   detectors++;
    Detectorlist.push_back("LS13");   detectors++;
    Detectorlist.push_back("LS14");   detectors++;
    Detectorlist.push_back("LS15");   detectors++;
    Detectorlist.push_back("LS16");   detectors++;
    Detectorlist.push_back("LS17");   detectors++;
    Detectorlist.push_back("LS18");   detectors++;
    Detectorlist.push_back("LS19");   detectors++;
    Detectorlist.push_back("LS20");   detectors++;
    Detectorlist.push_back("LS21");   detectors++;
    Detectorlist.push_back("LS23");   detectors++;
    Detectorlist.push_back("LS24");   detectors++;
    Detectorlist.push_back("LS25");   detectors++;
    Detectorlist.push_back("LS26");   detectors++;
    Detectorlist.push_back("LS27");   detectors++;
    Detectorlist.push_back("LS28");   detectors++;
    Detectorlist.push_back("LS29");   detectors++;
    Detectorlist.push_back("LS30");   detectors++;   
    Detectorlist.push_back("LS31");   detectors++;
    Detectorlist.push_back("LS32");   detectors++;
    Detectorlist.push_back("LS33");   detectors++;
    Detectorlist.push_back("LS34");   detectors++;
    Detectorlist.push_back("LS35");   detectors++;
    Detectorlist.push_back("LS36");   detectors++;
    Detectorlist.push_back("LS37");   detectors++;
    Detectorlist.push_back("LS38");   detectors++;
    Detectorlist.push_back("LS39");   detectors++;
    Detectorlist.push_back("LS40");   detectors++;
    Detectorlist.push_back("LS41");   detectors++;
    Detectorlist.push_back("LS42");   detectors++;
    Detectorlist.push_back("LS43");   detectors++;
    Detectorlist.push_back("LS44");   detectors++;
    Detectorlist.push_back("LS45");   detectors++;
    Detectorlist.push_back("LS46");   detectors++;
    Detectorlist.push_back("LS47");   detectors++;
    Detectorlist.push_back("LS48");   detectors++;
   
//    for ( int D = 0; D < detectors; D++ )
//    {
//       TString Detector = Detectorlist[D].c_str();
//       double      slope    = HVGainSlope(NumberOfGainMeasurements, voltage, Filelist, Detector);
//       double      constant = HVGainConstant(NumberOfGainMeasurements, voltage, Filelist, Detector);
//       cout << Detector.Data() << ": constant = " << constant << ",  slope = " << slope << endl;
//    }

//   TCanvas* Plots     = PlotSpectra(NumberOfGainMeasurements, voltage, Filelist, "GL01");
//   Plots->Draw();
//   TCanvas* GainPlots = MakeGainPlots(NumberOfGainMeasurements, detectors, voltage, Filelist, Detectorlist);
//   GainPlots->Draw();

   // Lets get a copy of the Configuration Object
   TFile* firstFile = TFile::Open(Filelist[0].c_str());
   NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*)(firstFile->Get("NGMSystemConfiguration"));
   // Close the file
   delete firstFile;
   
   	if(!sysConf) return;
	
	NGMConfigurationTable* cChan = sysConf->GetChannelParameters();
	NGMConfigurationTable* cDetector = sysConf->GetDetectorParameters();
	
	cDetector->PrintRow(0);

	if(cDetector->GetParIndex("HVSlope")<0)
	{
		cDetector->AddParameterD("HVSlope",-1.0,0.0,1.0);
		cDetector->SetParameterToDefault("HVSlope");
	}
	
	if(cDetector->GetParIndex("HVConstant")<0)
	{
		cDetector->AddParameterD("HVConstant",-1.0,0.0,1.0);
		cDetector->SetParameterToDefault("HVConstant");
	}
	
	if(cDetector->GetParIndex("TargetHV")<0)
	{
		cDetector->AddParameterD("TargetHV",-1.0,0.0,1.0);
		cDetector->SetParameterToDefault("TargetHV");
	}

   // Lets loop over all Gamma Detectors
   
   //This is actually the bin number which is binned in units of 10 ADCs
   double Target_ADC_Counts_of_2614_KeV = 3090.0;  // Good for gammas g2prime = 3090.0
   for ( int D = 0; D < detectors; D++ )
	{
       TString Detector = Detectorlist[D].c_str();
	   if(!Detector.BeginsWith("MU") && !Detector.BeginsWith("LS")) continue;
       double      slope    = HVGainSlope(NumberOfGainMeasurements, voltage, Filelist, Detector);
       double      constant = HVGainConstant(NumberOfGainMeasurements, voltage, Filelist, Detector);
       double      TargetHV = FindTargetHV(NumberOfGainMeasurements, voltage, Filelist, Detector, Target_ADC_Counts_of_2614_KeV);
	
		cDetector->SetParameterAsStringThatBeginsWith("HVSlope",TString()+=slope,"DetectorName",Detector.Data());
		cDetector->SetParameterAsStringThatBeginsWith("HVConstant",TString()+=constant,"DetectorName",Detector.Data());
		cDetector->SetParameterAsStringThatBeginsWith("TargetHV",TString()+=TargetHV,"DetectorName",Detector.Data());
       cout << Detector.Data() << ": constant = " << constant << ",  slope = " << slope << ", TargetHV = "<<TargetHV<<endl;
	   
    }
	
	return sysConf;
	

}


double HVGainSlope(int measurements, double* voltages, vector<string> &FileList, const char* Detector)
{
//    double Bin2614[measurements];
//    int    goodmeasurements = 1;
//    for ( int M = 0; M < measurements - 1; M++ )
//    {
//       const char* filename     = FileList[M].c_str();
//       const char* nextfilename = FileList[M+1].c_str();
//       Bin2614[M]               = Find2614(filename    , Detector);
//       Bin2614[M+1]             = Find2614(nextfilename, Detector);
//       if (Bin2614[M] < Bin2614[M+1] && voltages[M] < voltages[M+1])
//       {
//          goodmeasurements++;
//       }
//       if (Bin2614[M] > Bin2614[M+1] && voltages[M] < voltages[M+1])
//       {
//          goodmeasurements = 1;
//       }
//    }
   int    goodmeasurements = measurements;
   double Slope = -999;
   int N = 0;
   if (goodmeasurements > 1)
   {
      TVectorD Bin2614Vector( goodmeasurements );
      TMatrixD VoltageMatrix( goodmeasurements, 2 );
      for ( int M = measurements - goodmeasurements; M < measurements; M++ )
      {
         const char* filename  = FileList[M].c_str();
         VoltageMatrix( N, 0 ) = 1;
         VoltageMatrix( N, 1 ) = log(voltages[M]);
         Bin2614Vector( N )    = log(Find2614(filename, Detector));
         N++;
      }
      Bool_t ok;
      TDecompSVD GainSVD( VoltageMatrix );
      const TVectorD GainParameters = GainSVD.Solve( Bin2614Vector, ok );   
      Slope = GainParameters(1);
   }
   return Slope;
}


double HVGainConstant(int measurements, double* voltages, vector<string> &FileList, const char* Detector)
{
//    double Bin2614[measurements];
//    int    goodmeasurements = 1;
//    for ( int M = 0; M < measurements - 1; M++ )
//    {
//       const char* filename     = FileList[M].c_str();
//       const char* nextfilename = FileList[M+1].c_str();
//       Bin2614[M]               = Find2614(filename    , Detector);
//       Bin2614[M+1]             = Find2614(nextfilename, Detector);
//       if (Bin2614[M] < Bin2614[M+1] && voltages[M] < voltages[M+1])
//       {
//          goodmeasurements++;
//       }
//       if (Bin2614[M] > Bin2614[M+1] && voltages[M] < voltages[M+1])
//       {
//          goodmeasurements = 1;
//       }
//    }
   int    goodmeasurements = measurements;
   double Constant = -999;
   int N = 0;
   if (goodmeasurements > 1)
   {
      TVectorD Bin2614Vector( goodmeasurements );
      TMatrixD VoltageMatrix( goodmeasurements, 2 );
      for ( int M = measurements - goodmeasurements; M < measurements; M++ )
      {
         const char* filename  = FileList[M].c_str();
         VoltageMatrix( N, 0 ) = 1;
         VoltageMatrix( N, 1 ) = log(voltages[M]);
         Bin2614Vector( N )    = log(Find2614(filename, Detector));
         N++;
      }
      Bool_t ok;
      TDecompSVD GainSVD( VoltageMatrix );
      const TVectorD GainParameters = GainSVD.Solve( Bin2614Vector, ok );   
      Constant = GainParameters(0);
   }
   return Constant;
}


double Find2614(const char* FileName, const char* Detector)
{
   TFile* file = TFile::Open(FileName);
   char   HistogramName[20] = "NMON_g2prime_";
   strcat(HistogramName, Detector);
   TH1F*  hist = (TH1F*)file->Get(HistogramName);
   TH1F*  hist1 = SmoothBoxcar(hist, 0.2);
   TH1F*  hist2 = TimesBinNumber(hist1);
   TH1F*  hist3 = TakeDerivative(hist2,10);
   double LocalMinimum = Find_Rising_Zero(hist3, 1);
   file -> Close();
   return LocalMinimum;
}


TH1F* NormalizeHistogram(TH1F* Histogram)
{
   double scale = 1/Histogram->Integral();       // Normalize
   Histogram -> Scale(scale);
   return Histogram;
}


TH1F* TakeDerivative(TH1F* Histogram, int deltaX)
{
   int    Bins               = Histogram -> GetNbinsX();     // Number of bins in spectrum
   double BinWidth           = Histogram -> GetBinWidth(1);  // Bin width (in ADC Counts) of spectrum
   TH1F* HistogramDerivative = new TH1F("HistogramDerivative" , "Derivative of Histogram", Bins-deltaX, 0, BinWidth*(Bins-deltaX));
   for ( int B = 1; B < Bins; B++ )                          // Derivative
   {
      HistogramDerivative -> SetBinContent(B, (Histogram->GetBinContent(B+deltaX) - Histogram->GetBinContent(B))/(deltaX*BinWidth));
   }
   return HistogramDerivative;
}


double Find_Rising_Zero(TH1F* Histogram, int deltaX)
{
   double BinWidth   = Histogram -> GetBinWidth(1);  // Bin width (in ADC Counts) of spectrum
   bool   KeepGoing  = true;
   double RisingZero = 0;
   int    B          = 0;
   while ( KeepGoing )
   {
      if ( Histogram->GetBinContent(B) < 0 && Histogram->GetBinContent(B+deltaX) > 0 )
      {
         KeepGoing  = false;
         RisingZero = (B + deltaX/2)*BinWidth;
      }
      B++;
   }
   return RisingZero;
}


TH1F* TimesBinNumber(TH1F* Histogram)
{
   int    Bins               = Histogram -> GetNbinsX();     // Number of bins in spectrum
   double BinWidth           = Histogram -> GetBinWidth(1);  // Bin width (in ADC Counts) of spectrum
   TH1F*  TimesBinNumberHist = new TH1F("TimesBinNumberHist" , "Multiply by bin number", Bins, 0, BinWidth*Bins);
   for ( int B = 1; B <= Bins; B++ )                        // Multiply bin content by bin number
   {
      TimesBinNumberHist -> SetBinContent(B, B*Histogram -> GetBinContent(B));
   }
   return TimesBinNumberHist;
}


TH1F* SmoothBoxcar(TH1F* Histogram, double widfrac)
{
   int    Bins               = Histogram -> GetNbinsX();     // Number of bins in spectrum
   double BinWidth           = Histogram -> GetBinWidth(1);  // Bin width (in ADC Counts) of spectrum
   TH1F*  BoxcarSmoothed = new TH1F("BoxcarSmoothed" , "Boxcar Smoothed", Bins, 0, BinWidth*Bins);
   for ( int B = 1; B <= Bins; B++ )                        // Average bins over an interval
   {
      // each bin's box width is x-dependent but do the ends right
      int TempArray[2];
      TempArray[0] = B - 1;
      TempArray[1] = Bins - 1 - B - 1;
      int limitwid = TMath::MinElement( 2, TempArray );
      TempArray[0] = (int)(widfrac*B)/2;
      TempArray[1] = limitwid;
      int boxwidup = TMath::MinElement( 2, TempArray );      
      // this is giving trouble at the top end. that explains the two -1s in limitwid
      BoxcarSmoothed -> SetBinContent(B, Histogram->Integral(B-boxwidup,B+boxwidup)/(2*boxwidup+1));
   }
   return BoxcarSmoothed;
}


TCanvas* PlotSpectra(int measurements, double* voltages, vector<string> &FileList, const char* Detector)
{
   TCanvas* SpectraPlots = new TCanvas("SpectraPlots" , Detector, 1275, 755);
   SpectraPlots -> Divide(measurements,3);
   for ( int M = 0; M < measurements; M++ )
   {
      const char* filename          = FileList[M].c_str();
      TFile*      file              = TFile::Open(filename);
      char        HistogramName[20] = "NMON_g2prime_";
      strcat(HistogramName, Detector);
      TH1F*       hist0             = (TH1F*)file->Get(HistogramName);
      TH1F*       hist1             = SmoothBoxcar(hist0, 0.2);
      TH1F*       hist2             = TimesBinNumber(hist1);   
      TH1F*       hist3             = TakeDerivative(hist2,10);
      SpectraPlots -> cd(M + 1                 ) -> SetLogx();
      SpectraPlots -> cd(M + 1                 ) -> SetLogy();
      SpectraPlots -> cd(M + 1                 );                hist0 -> DrawCopy();   
      SpectraPlots -> cd(M + 1 + measurements  ) -> SetLogx();
      SpectraPlots -> cd(M + 1 + measurements  );                hist2 -> DrawCopy();   
      SpectraPlots -> cd(M + 1 + 2*measurements) -> SetLogx();
      SpectraPlots -> cd(M + 1 + 2*measurements);                hist3 -> DrawCopy();   
      //SpectraPlots -> cd(0);
   }
   return SpectraPlots;
}


TGraph* PlotGainVsVoltage(int measurements, double* voltages, vector<string> &FileList, const char* Detector)
{
   double Bin2614[measurements];
   for ( int M = 0; M < measurements; M++ )
   {
      const char* filename = FileList[M].c_str();
      Bin2614[M]           = Find2614(filename, Detector);
   }
   TGraph* Plot = new TGraph(measurements, voltages, Bin2614);
   return Plot;
}


TGraph* PlotGainVsVoltage(int measurements, double* voltages, double* Bin2614)
{
   TGraph* Plot = new TGraph(measurements, voltages, Bin2614);
   return Plot;
}


TH2F* SetupGainPlot(int measurements, double* x, double* y, const char* title, char* xaxis, char* yaxis)
{   
   double LowestX     = TMath::MinElement( measurements, x );
   double HighestX    = TMath::MaxElement( measurements, x );
   double ExtraRange  = (HighestX - LowestX)*0.1;
   double LowestY     = TMath::MinElement( measurements, y );
   double HighestY    = TMath::MaxElement( measurements, y );
   double ExtraDomain = (HighestY - LowestY)*0.1;
   TH2F*  Plot        = new TH2F("Plot",
                                 title,
                                 10,
                                 LowestX  - ExtraRange,
                                 HighestX + ExtraRange,
                                 100,
                                 LowestY  - ExtraDomain,
                                 HighestY + ExtraDomain );
   Plot -> GetYaxis()->SetTitle(xaxis);
   Plot -> GetYaxis()->SetTitleOffset(1.3);
   Plot -> GetXaxis()->SetTitle(yaxis);
   Plot -> GetXaxis()->CenterTitle(1);
   Plot -> GetYaxis()->CenterTitle(1);
   Plot -> SetStats(false);
   return Plot;
}


TCanvas* MakeGainPlots(int measurements, int detectors, double* voltages, vector<string> &FileList, vector<string> DetectorList)
{
   int         plotsbyplots = (int)sqrt(detectors);
   TCanvas*    GainPlots    = new TCanvas("GainPlots" , "2614 KeV Integrated ADC Counts vs. HV Setting", 1275, 755);
   if (plotsbyplots < sqrt(detectors))   GainPlots -> Divide(plotsbyplots+1,plotsbyplots);
   if ((float)plotsbyplots == sqrt(detectors))  GainPlots -> Divide(plotsbyplots,plotsbyplots);
   for ( int D = 0; D < detectors; D++ )
   {
      const char* Detector = DetectorList[D].c_str();
      double      Bin2614[measurements];
      for ( int M = 0; M < measurements; M++ )
      {
         const char* filename = FileList[M].c_str();
         Bin2614[M]           = Find2614(filename, Detector);
      }
      TH2F*       GainHist   = SetupGainPlot(measurements, voltages, Bin2614, Detector, "2614 KeV Integrated Counts", "HV Setting");
      TGraph*     ActualPlot = PlotGainVsVoltage(measurements, voltages, Bin2614);
      GainPlots  -> cd(D + 1) -> SetLogy();
      GainPlots  -> cd(D + 1) -> SetLogx();
      GainPlots  -> cd(D + 1);
      GainHist   -> DrawCopy();
      ActualPlot -> Draw("lp");
      ActualPlot -> SetMarkerSize(1);
      ActualPlot -> SetMarkerStyle(kFullCircle);
   }
   return GainPlots;
}


double FindTargetHV(int measurements, double* voltages, vector<string> &FileList, const char* Detector, double Target_ADC_Counts_of_2614_KeV)
{
   double slope         = HVGainSlope(measurements, voltages, FileList, Detector);
   double constant      = HVGainConstant(measurements, voltages, FileList, Detector);
   double log_target_HV = (log(Target_ADC_Counts_of_2614_KeV) - constant)/slope;
   double TargetHV      = exp(log_target_HV);
   return TargetHV;
}

double Find2614G2(const char* Detector, TDirectory* curDir)
{
   //TFile* file = TFile::Open(FileName);
   char   HistogramName[20] = "NMON_g2prime_";
   strcat(HistogramName, Detector);
   TH1F*  hist = (TH1F*)curDir->FindObjectAny(HistogramName);
   if(!hist) std::cerr<<"Cannot find histogram "<<HistogramName<<std::endl;
   TH1F*  hist1 = SmoothBoxcar(hist, 0.2);
   TH1F*  hist2 = TimesBinNumber(hist1);
   TH1F*  hist3 = TakeDerivative(hist2,10);
   double LocalMinimum = Find_Rising_Zero(hist3, 1);
   double g2Value = hist->GetXaxis()->GetBinCenter(LocalMinimum);
   
   //file -> Close();
   delete hist1;
   delete hist2;
   delete hist3;
   
   return g2Value;
}

NGMSystemConfiguration* Tabulate2614G2()
{
	TDirectory* curDir = gDirectory;
	if(!curDir) return;
	
	NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*) curDir->FindObjectAny("NGMSystemConfiguration");
	if(!sysConf) return;
	
	NGMConfigurationTable* cChan = sysConf->GetChannelParameters();
	NGMConfigurationTable* cDetector = sysConf->GetDetectorParameters();
	std::cout<<cDetector<<std::endl;
	if(cDetector->GetParIndex("LowThreshold_g2prime")<0)
	{
		cDetector->AddParameterD("LowThreshold_g2prime",-1.0,0.0,1.0);
		cDetector->SetParameterToDefault("LowThreshold_g2prime");
	}
	
	for(int ichan = 0; ichan < cChan->GetEntries(); ichan++)
	{
		// Only examine channels active in the existing run
		TString detName = cChan->GetParValueS("DetectorName",ichan);
		
		// Skip all but GS GL MU
		if(!detName.BeginsWith("LS")) continue;
		TString HistogramName("NMON_g2prime_");
		HistogramName+=detName;
		
		TH1*  hist = (TH1*)curDir->FindObjectAny(HistogramName);
		   
		double g2Val = Find2614G2(detName,curDir);
		TString calibrationEnergy("2614.0");
		TString calibrationG2(""); calibrationG2+=g2Val;
		
		hist->GetXaxis()->SetRange(); // Reset range to ensure mean is accurate
		TString calibrationMeanADC(""); calibrationMeanADC+=hist->GetMean()

		double lowThresholdG2 = hist->GetXaxis()->GetBinCenter(hist->GetMaximumBin());
		TString calibrationMeanADC(""); calibrationMeanADC+=hist->GetMean()

		if(g2Val>0.0)
		{
			std::cout<<detName.Data()<<" "<<g2Val<<std::endl;
			cDetector->SetParameterAsStringThatBeginsWith("calSourceName","Natural2614","DetectorName",detName.Data());
			cDetector->SetParameterAsStringThatBeginsWith("calSourceEnergy",calibrationEnergy.Data(),"DetectorName",detName.Data());
			cDetector->SetParameterAsStringThatBeginsWith("calSourceMeanADC",calibrationMeanADC.Data(),"DetectorName",detName.Data());			
			cDetector->SetParameterAsStringThatBeginsWith("calSourceComptonEdge",calibrationG2.Data(),"DetectorName",detName.Data());
			cDetector->SetParameterAsStringThatBeginsWith("LowThreshold_g2prime",calibrationG2.Data(),"DetectorName",detName.Data());
		}
	}
	
	return sysConf;
	
}