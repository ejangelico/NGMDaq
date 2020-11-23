void jnCountDistPlotTest(int flag=0) {

  // testing jntestr4.v657
  // Set Style
  TStyle *mystyle = gROOT->GetStyle("Default");
  mystyle->SetOptStat(0);
  mystyle->SetPadTickX(1);
  mystyle->SetPadTickY(1);
  mystyle->SetNdivisions(505,  "axis=X");
  mystyle->SetNdivisions(505,  "axis=Y");
  mystyle->SetHistFillColor(10);
  mystyle->SetHistFillStyle(0);
  mystyle->SetCanvasColor(10);
  //  mystyle->SetLineWidth(3);
  mystyle->SetOptLogx();
  mystyle->SetOptLogy();

  // Fetch info from database first for ntsdaf runs
  TSQLServer* db = TSQLServer::Connect("mysql://hip.llnl.gov/ngmrunlog","ngmdaq","NGMp3daq");
  TTreeSQL* nt = new TTreeSQL(db,"ngmrunlog","jntestr4"); 
  int num;

  if ((flag==0)||(flag==1)) {


    num = nt->Draw("lambda:r2f:r2fe","(strstr(experimentname,\"1.\"))||(strstr(experimentname,\"3.\"))","goff");
    TGraphErrors* gHEUr2f_lam = new TGraphErrors(num,nt->GetV1(),nt->GetV2(),0,nt->GetV3());
    gHEUr2f_lam->SetMarkerStyle(20);
    gHEUr2f_lam->SetMarkerColor(2);

    num = nt->Draw("lambda:r2f:r2fe","(strstr(experimentname,\"2.\"))||(strstr(experimentname,\"4.\"))","goff");
    TGraphErrors* gPUr2f_lam = new TGraphErrors(num,nt->GetV1(),nt->GetV2(),0,nt->GetV3());
    gPUr2f_lam->SetMarkerStyle(20);
    gPUr2f_lam->SetMarkerColor(4);

    num = nt->Draw("lambda:r2f:r2fe","(strstr(experimentname,\"CAR\"))","goff");
    TGraphErrors* gCARr2f_lam = new TGraphErrors(num,nt->GetV1(),nt->GetV2(),0,nt->GetV3());
    gCARr2f_lam->SetMarkerStyle(21);
    gCARr2f_lam->SetMarkerColor(1);


    TCanvas* ctest_r2f = new TCanvas("ctest_r2f","testObject.v641 Pass R2F",150,100,700,500);
    TH2F *h1 = new TH2F("h1","",10,1e-10,20.,10,0.001,50.0);
    h1->SetXTitle("Mult");
    h1->SetYTitle("R2F");  
    h1->Draw();
    gCARr2f_lam->Draw("P");
    gPUr2f_lam->Draw("P");
    gHEUr2f_lam->Draw("P");

    TLatex *t1 = new TLatex(0.5,0.95,"jntest4.v657 Pass - R2F vs. lambda 1-#sigma cut");
    t1->SetNDC();
    t1->Draw("same");
    t1->SetTextAlign(22);

    TLatex *t2 = new TLatex(0.98,0.98,"RAS 14-Nov-2008");
    t2->SetTextSize(0.02);
    t2->SetNDC();
    t2->Draw("same");
    t2->SetTextAlign(32);

    TLegend *leg = new TLegend(0.70,0.15,0.90,0.30);
    leg->SetTextSize(.035);
    leg->SetFillColor(0);
    leg->AddEntry(gCARr2f_lam, "CAR Study", "p");
    leg->AddEntry(gPUr2f_lam, "NTS Pu Exp", "p");
    leg->AddEntry(gHEUr2f_lam, "NTS HEU.", "p");
    leg->Draw("same");

  } // end flag==1 for R2F vs. lambda for SN plots



  db->Delete();

}

