void plotNGMComptonEdgePars()
{
 
  const char* filelist[] = {
    "/Users/Shared/ngm/B231AllCs137VScan/NGMComptonEdgeFinderParameters1150V.root",
    "/Users/Shared/ngm/B231AllCs137VScan/NGMComptonEdgeFinderParameters1100V.root",
    "/Users/Shared/ngm/B231AllCs137VScan/NGMComptonEdgeFinderParameters1050V.root",
    "/Users/Shared/ngm/B231AllCs137VScan/NGMComptonEdgeFinderParameters1000V.root"
  };
  const double hvsetting[]= {1150.0,1100.0,1050.0,1000.0};
  const int hvcolor[]={kBlack,kBlue,kRed,kGreen};
  const int nfiles = 4;
  const int nchannels = 96;
  TH1* hComptonList[4];
  
  TLegend* tl = new TLegend(0.8,0.7,0.9,0.9);
  TH2* h2DGainHist = new TH2F("h2DGainHist","h2DGainHist",nchannels,0,nchannels,nfiles,0,nfiles);
  h2DGainHist->SetDirectory(gROOT);
  for(int ifile = 0; ifile < nfiles; ifile++)
  {
    TFile* tf = TFile::Open(filelist[ifile]);
    NGMConfigurationTable* results = (NGMConfigurationTable*) tf->Get("NGMComptonEdgeFinderParameters");
    char cbuff[1024];
    sprintf(cbuff,"hComptonEdge_%d",hvsetting[ifile]);
    TH1* hComptonEdge = new TH1F(cbuff,cbuff,nchannels,0,nchannels);
    hComptonList[ifile] = hComptonEdge;
    hComptonEdge->SetDirectory(gROOT);
    for(int ichan = 0; ichan < nchannels; ichan++)
    {
      hComptonEdge->SetBinContent(ichan+1, results->GetParValueD("comptonEdge",ichan));
      h2DGainHist->Fill(ichan , nfiles - 1 - ifile, results->GetParValueD("comptonEdge",ichan));
    }
    hComptonEdge->SetLineWidth(2);
    hComptonEdge->SetLineColor(hvcolor[ifile]);
    hComptonEdge->SetFillColor(hvcolor[ifile]);
    sprintf(cbuff,"%d Volts",hvsetting[ifile]);
    tl->AddEntry(hComptonEdge,cbuff,"L");
    
    if(ifile == 0)
      hComptonEdge->Draw("B");
    else
      hComptonEdge->Draw("BSAME");
    
  }
  tl->Draw();
 
  double target = 2000.0;
  
  ofstream fout("targetVoltages.txt");
  
  for(int ichan = 0; ichan < nchannels; ichan++)
  {
    double currentdiff = 10000.0;
    double currenthvset = 0;
    int ifilematch = 0;
    for(int ifile = 0; ifile < nfiles; ifile++)
    {
      if(abs(hComptonList[ifile]->GetBinContent(ichan+1) - target)<currentdiff)
      {
        currentdiff = abs(hComptonList[ifile]->GetBinContent(ichan+1) - target);
        currenthvset = hvsetting[ifile];
        ifilematch = ifile;
      }
      
    }
    
    char gflag = ' ';
    
    double ratio = hComptonList[ifilematch]->GetBinContent(ichan+1)/target;
    
    if(ratio>1.5)
      gflag = 'o';
    else if(ratio<0.5)
      gflag = 'u';
    else
      gflag = '_';
    
    fout<<ichan<< "\t"<<gflag<<"\t"<< currenthvset <<"\t"<< hComptonList[ifilematch]->GetBinContent(ichan+1)
                    << "\t" << ratio<<std::endl;
  }
  fout.close();
}