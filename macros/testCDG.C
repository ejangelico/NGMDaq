void testCDG()
{
   NGMCountDistGenerator* cdg = new NGMCountDistGenerator;
   int ngates = 1000000;
   cdg->setNgates(ngates);
   cdg->setCountRate(2000);
   cdg->setLambda_inv_usecs(300);
   // Get Functional form for random time distribution
   // fcdf is forever tied to this instance of cdg.
   TF1* fcdf = cdg->getTF1("MyCountDist");
   fcdf->SetRange(-0.5,19.5);
   fcdf->SetNpx(20);
   fcdf->Draw();
   // Use the functional form to generate a histogram
   TH1* hRandDist = new TH1F("hRandDist","hRandDist",20,-0.4,19.6);
   for(int ibin = 1; ibin <= hRandDist->GetNbinsX(); ibin++)
   {
      hRandDist->SetBinContent(ibin,fcdf->Eval( hRandDist->GetXaxis()->GetBinCenter(ibin) ));
      hRandDist->SetBinError(ibin,sqrt(hRandDist->GetBinContent(ibin)));                         
   }

   // Sample from the hRandDist histogram ngate times.
   TCanvas* cFit = new TCanvas("cTestFit","TestFit");
   TH1* hTestDist = new TH1F("hTestDist","hTestDist",20,-0.4,19.6);
   hTestDist->FillRandom(hRandDist,ngates);
   hTestDist->SetMarkerStyle(20);
   hTestDist->SetMarkerColor(kRed);
   hTestDist->Draw("E");
   
   //Now lets attempt to fit the histogram randomly sampled from the parent distribution
   
   //fcdf->SetParLimits(0,1.0,10.0); // M
   fcdf->FixParameter(0,fcdf->GetParameter(0)); // M
   //fcdf->SetParLimits(1,0.001,0.5); // eff
   fcdf->SetParLimits(1,fcdf->GetParameter(1)*0.9,fcdf->GetParameter(1)*1.1); // eff
   //fcdf->FixParameter(1,fcdf->GetParameter(1)); // eff
   // // Must use FixParameter instead of xmin=xmax if xmax == 0.0
   fcdf->SetParLimits(2,0.0,0.0); // A
   fcdf->FixParameter(2,0.0); // A
   fcdf->SetParLimits(3,0.0,0.0); // Rext
   fcdf->FixParameter(3,0.0); // Rext
   fcdf->FixParameter(4,fcdf->GetParameter(4)); //sorc
   fcdf->FixParameter(5,fcdf->GetParameter(5)); //ind
   fcdf->SetParLimits(6,fcdf->GetParameter(6)*0.8,fcdf->GetParameter(6)*1.2); // countrate
   fcdf->FixParameter(6,fcdf->GetParameter(6)); // countrate
   fcdf->SetParLimits(7,fcdf->GetParameter(7)*0.9,fcdf->GetParameter(7)*1.1); // lambda
   //fcdf->FixParameter(7,fcdf->GetParameter(7)); // lambda    
   fcdf->FixParameter(8,fcdf->GetParameter(8)); // gatelength 
   fcdf->FixParameter(9,fcdf->GetParameter(9)); //nepsb 
   fcdf->FixParameter(10,0.0); 
   fcdf->FixParameter(11,0.0); 
   fcdf->FixParameter(12,0.0); 
   fcdf->FixParameter(13,0.0); 
   fcdf->FixParameter(14,0.0); 
   fcdf->FixParameter(15,0.0); 
   
   fcdf->SetParameter(1,fcdf->GetParameter(1)*1.1);
   fcdf->SetParameter(7,fcdf->GetParameter(7)*0.9);
   
   hTestDist->Fit(fcdf,"","E");
   cFit->SetLogy();
}
