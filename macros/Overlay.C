void Overlay()
{
  const char* fname1 = "NGMAna20071114203335ngm.root";
  const char* fname2 = "NGMAna20071025000000ngm.root";
  //usr/gapps/ngm/data/Nov07/e3.3/NGMAna20071126202542ngm.root";
  //  const char* fname2 = "/usr/gapps/ngm/simu/Nov07/e3.3/mcnpx/data/spfiss/NGMAna20071025000000ngm.root";

  TFile* tf1 = TFile::Open(fname1);
  TCanvas* tc1 = (TCanvas*)(tf1->Get("COUNTSNSLOW_Canvas"));
  tc1->SetName("CounDist1_Canvas");
  tc1->Draw();

  TFile* tf2 = TFile::Open(fname2);
  TCanvas* tc2 = (TCanvas*)(tf2->Get("CountDist_Canvas"));
  tc2->Draw();

  TList* tc1l = tc1->GetListOfPrimitives();
  TList* tc2l = tc2->GetListOfPrimitives();

  for(int il = 0; il < tc1l->GetEntries(); il++)
  {
    if ((il!=0)&&(il!=5)) {continue;}
    // If the member is a pad then overlay from subpad
    if(tc1l->At(il)->IsA()->InheritsFrom("TPad"))
    {
      TVirtualPad* tPad1 = (TVirtualPad*)(tc1l->At(il));
      tPad1->cd();
      TVirtualPad* tPad2 = (TVirtualPad*)(tc2l->At(il));
      TListIter* tp2l = tPad2->GetListOfPrimitives()->MakeIterator();
      TObject* tp2o = 0;
      while((tp2o = (*tp2l)()))
      {
	if(tp2o->IsA()->InheritsFrom("TH1") || tp2o->IsA()->InheritsFrom("TGraph") )
	  {
	    TH1* thist = (TH1*) tp2o;
	    cout<<"Drawing "<<tp2o->GetName()<<endl;
	    if(TString(tp2l->GetOption()).Contains("p"))
	      thist->SetMarkerStyle(thist->GetMarkerStyle()+4);
	    thist->SetLineColor(thist->GetLineColor()+10);
	    TList* funcList =  thist->GetListOfFunctions();
	    if(0){//funcList){
	      TIterator* tfunc =funcList->MakeIterator();
	      TObject* funco = 0;
	      while((funco = (*tfunc)()))
	      {
		if(funco->IsA()->InheritsFrom("TF1"))
		{
		  TF1* funcp = (TF1*)funco;
		  cout<<"Found function "<<funcp->GetName()<<endl;
		  funcp->SetLineColor(funcp->GetLineColor()+10);
		}
	      }
	    }
	    tp2o->Draw(TString(tp2l->GetOption())+="SAME");

	  }else if(tp2o->IsA()->InheritsFrom("TGraph") )
	  {
	    TGraph* tg = (TGraph*) tp2o;
	    cout<<"Drawing "<<tp2o->GetName()<<endl;
	    if(TString(tp2l->GetOption()).Contains("p"))
	      tg->SetMarkerStyle(tg->GetMarkerStyle()+4);
	    tg->SetLineColor(tg->GetLineColor()+10);
	    tp2o->Draw(TString(tp2l->GetOption())+="SAME");
	  }
      }
    }
  }

}
