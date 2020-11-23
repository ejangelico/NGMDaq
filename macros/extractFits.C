void extractFits(const char* flist, bool printflag = true,
                 const char* smodname="CountDist", const char* spassname = "v528a")
{
  bool updateDB = true;

  TString modname(smodname);
  TString passname(spassname);
  ifstream inlist1(flist);
//  std::vector<std::string> fnamelist;
//  std::string fname;
  char fname[1024];
  TList* fnamelist = new TList();
  while(inlist1>>fname)
  {
    //fnamelist.push_back(fname);
    fnamelist->AddLast(new TObjString(fname));
  }
  inlist1.close();

  TSQLServer* db = 0;
  TSQLResult* res = 0;
  
  //  if(updateDB)
  {
    std::string password;
    ifstream pfile(TString(gSystem->HomeDirectory())+"/.ngmdbpw");
    pfile>>password;
    pfile.close();
  
  
    db = TSQLServer::Connect("mysql://hip.llnl.gov/ngmrunlog","ngmdaq", password.c_str());
    if(!db)
    {
      std::cerr<<"Error accessing database"<<std::endl;
      return;
    }
  }

  // Declare all data locals
  Long64_t runnumber = 0;
  double r1 = 0.0;
  double duration = 0.0;
  double timebinwidth = 0.0;
  double r2f = 0.0;
  double r2fe = 0.0;
  double lambda = 0.0;
  double lambdae = 0.0;
  double c2 = 0;
  double c2e = 0;
  double r3f = 0.0;
  double r3fe = 0.0;
  double r2fp = 0.0;
  double r2fpe = 0.0;
  double lambdap = 0.0;
  double lambdape = 0.0;
  double c2p = 0.0;
  double c2pe = 0.0;
  double r3fp = 0.0;
  double r3fpe = 0.0;
  double y2fr_chisquare = 0.0;
  double y2fr_ndf = 0.0;
  double y2ft_chisquare = 0.0;
  double y2ft_ndf = 0.0;
  double y2ftime[60];
  double y2fval[60];
  double y2ferr[60];
  char source[1024];
  char experiment[1024];
  char shielding[1024];
  
  TString ofname;
  ofname.Form("CountDist%s%s.root",modname.Data(),passname.Data());
  TFile* tfout = TFile::Open(ofname.Data(),"RECREATE");
  TTree* ttree = new TTree("CountDistScan","CountDistScan");
  ttree->Branch("runnumber",&runnumber,"runnumber/L");
  ttree->Branch("r1",&r1,"r1/D"); // r1
  ttree->Branch("duration",&duration,"duration/D");
  ttree->Branch("timebinwidth",&timebinwidth,"timebinwidth/D"); // r1
  ttree->Branch("r2f",&r2f,"r2f/D");// r2f
  ttree->Branch("r2fe",&r2fe,"r2fe/D"); // r2fe
  ttree->Branch("lambda",&lambda,"lambda/D"); // lambda
  ttree->Branch("lambdae",&lambdae,"lambdae/D"); // lambdae
  ttree->Branch("c2",&c2,"c2/D"); // c2
  ttree->Branch("c2e",&c2e,"c2e/D"); // c2e
  ttree->Branch("r3f",&r3f,"r3f/D"); // r3f
  ttree->Branch("r3fe",&r3fe,"r3fe/D"); // r3fe
  ttree->Branch("y2fr_chisquare",&y2fr_chisquare,"y2fr_chisquare/D"); // 
  ttree->Branch("y2fr_ndf",&y2fr_ndf,"y2fr_ndf/D"); // 
  ttree->Branch("r2fp",&r2fp,"r2fp/D"); // r2fp
  ttree->Branch("r2fpe",&r2fpe,"r2fpe/D"); // r2fpe
  ttree->Branch("lambdap",&lambdap,"lambdap/D"); // lambdap
  ttree->Branch("lambdape",&lambdape,"lambdape/D"); // lambdape
  ttree->Branch("c2p",&c2p,"c2p/D"); // c2p
  ttree->Branch("c2pe",&c2pe,"c2pe/D"); // c2pe
  ttree->Branch("r3fp",&r3fp,"r3fp/D"); // r3fp
  ttree->Branch("r3fpe",&r3fpe,"r3fpe/D"); // r3fpe
  ttree->Branch("y2ft_chisquare",&y2ft_chisquare,"y2ft_chisquare/D"); // 
  ttree->Branch("y2ft_ndf",&y2ft_ndf,"y2ft_ndf/D"); // 
  ttree->Branch("y2ftime",&y2ftime,"y2ftime[60]/D");
  ttree->Branch("y2fval",&y2fval,"y2fval[60]/D");
  ttree->Branch("y2ferr",&y2ferr,"y2ferr[60]/D");
  ttree->Branch("source",&source,"source/C");
  ttree->Branch("shielding",&shielding,"shielding/C");
  ttree->Branch("experiment",&experiment,"experiment/C");
  //  ttree->Branch("",&,"");

  
  //Loop over files
  TFile* tmpfile = 0;
//  std::vector<std::string>::iterator fitr = fnamelist.begin();
//  for( ;fitr < fnamelist.end(); fitr++)
  TListIter fitr(fnamelist);
  TObjString* fnameo = 0;
  while((fnameo = (TObjString*)(fitr())))
  {
    
    tmpfile = TFile::Open(fnameo->GetString().Data());
    if(! tmpfile){ std::cerr<<"Error opening file: "<<fnameo->GetString().Data()<<std::endl; }
    else{ std::cout<<" Analyzing "<<fnameo->GetString().Data()<<std::endl;}
    if(1)
    {
    NGMSystemConfiguration* sysConf = (NGMSystemConfiguration*)(tmpfile->Get("NGMSystemConfiguration"));
    NGMAnalysisInput* ana = (NGMAnalysisInput*)(tmpfile->Get("NGMAna"));

    if(!ana) continue;
    
    NGMCountDistR* cntd = (NGMCountDistR*)(ana->FindModule(modname.Data()));
    if(!cntd) continue;
    TGraphErrors* randomTG = cntd->getY2F();//(TGraphErrors*)(tmpfile->Get((modname+"_Y2F").Data()));
    TGraphErrors* trigTG = cntd->getZ2F();//(TGraphErrors*)(tmpfile->Get((modname+"_Z2F").Data()));
    TGraphErrors* randomTG3 = cntd->getY3F();//(TGraphErrors*)(tmpfile->Get((modname+"_Y2F").Data()));
    TGraphErrors* trigTG3 = cntd->getZ3F();//(TGraphErrors*)(tmpfile->Get((modname+"_Z2F").Data()));

    if(!randomTG || !trigTG || !sysConf) continue;
    
    runnumber = sysConf->getRunNumber();
    std::cout << runnumber << std::endl;
    r1 = cntd->GetRandomRate();
    duration = cntd->GetDataTime();
    timebinwidth = cntd->getGateInterval();
    r2f = randomTG->GetFunction("Y2F_fit")->GetParameter(0);
    r2fe = randomTG->GetFunction("Y2F_fit")->GetParError(0);
    lambda = randomTG->GetFunction("Y2F_fit")->GetParameter(1);
    lambdae = randomTG->GetFunction("Y2F_fit")->GetParError(1);
    c2 = randomTG->GetFunction("Y2F_fit")->GetParameter(2);
    c2e = randomTG->GetFunction("Y2F_fit")->GetParError(2);
    r3f = randomTG3->GetFunction("Y3F_fit")->GetParameter(0);
    r3fe = randomTG3->GetFunction("Y3F_fit")->GetParError(0);
    y2fr_chisquare = randomTG->GetFunction("Y2F_fit")->GetChisquare();
    y2fr_ndf = randomTG->GetFunction("Y2F_fit")->GetNDF();
//     r2fp = trigTG->GetFunction("Z2F_fit")->GetParameter(0);
//     r2fpe = trigTG->GetFunction("Z2F_fit")->GetParError(0);
//     lambdap = trigTG->GetFunction("Z2F_fit")->GetParameter(1);
//     lambdape = trigTG->GetFunction("Z2F_fit")->GetParError(1);
//     c2p = trigTG->GetFunction("Z2F_fit")->GetParameter(2);
//     c2pe = trigTG->GetFunction("Z2F_fit")->GetParError(2);
//     r3fp = trigTG3->GetFunction("Z3F_fit")->GetParameter(0);
//     r3fpe = trigTG3->GetFunction("Z3F_fit")->GetParError(0);
//     y2ft_chisquare = trigTG->GetFunction("Z2F_fit")->GetChisquare();
//     y2ft_ndf = trigTG->GetFunction("Z2F_fit")->GetNDF();
    
    for(int itb = 0; itb < TMath::Min(60,randomTG->GetN()); itb++)
    {
      y2ftime[itb] = (randomTG->GetX())[itb];
      y2fval[itb] = (randomTG->GetY())[itb];
      y2ferr[itb] = randomTG->GetErrorY(itb);
    }

    bool runinfofound = false;
    if(db)
    {
      //First test if this entry is already in database
      TString sqlRunInfo("select runnumber,experimentname,source,shielding from filelog where runnumber=\"");
      sqlRunInfo+=runnumber;
      sqlRunInfo+="\"";
      
      res = db->Query(sqlRunInfo);
      if(res->GetRowCount()==1)
      {
	TSQLRow* row = res->Next();
	runinfofound = true;
	sprintf(experiment,"%s",row->GetField(1));
	sprintf(source,"%s",row->GetField(2));
	sprintf(shielding,"%s",row->GetField(3));

      }


      delete res;

    }
    if(!runinfofound)
    {
      sprintf(experiment,"");
      sprintf(source,"");
      sprintf(shielding,"");
    }

    ttree->Fill();

    TCanvas* countDisplay = (TCanvas*)(tmpfile->FindObjectAny(modname+"_Canvas"));
    TString printName  ((TString("plots/"+passname)+modname)+=runnumber);

      if(countDisplay && printflag)
      {
        countDisplay->Print(printName+".eps" );
        countDisplay->Print(printName+".svg" );
	
      }
      if(updateDB)
      {
      // Convert Varibles to strings
      TString srunnumber; srunnumber+=runnumber;
      TString sr1; sr1.Form("%f",r1);
      TString sduration; sduration.Form("%f",duration);
      TString stimebinwidth; stimebinwidth.Form("%f", timebinwidth);
      TString sr2f; sr2f.Form("%e", r2f);
      TString sr2fe; sr2fe.Form("%e", r2fe);
      TString slambda; slambda.Form("%e",lambda );
      TString slambdae; slambdae.Form("%e", lambdae);
      TString sc2; sc2.Form("%e", c2);
      TString sc2e; sc2e.Form("%e", c2e);
      TString sr3f; sr3f.Form("%e", r3f);
      TString sr3fe; sr3fe.Form("%e", r3fe);
      TString sr2fp; sr2fp.Form("%e", r2fp);
      TString sr2fpe; sr2fpe.Form("%e", r2fpe);
      TString slambdap; slambdap.Form("%e", lambdap);
      TString slambdape; slambdape.Form("%e", lambdape);
      TString sc2p; sc2p.Form("%e", c2p);
      TString sc2pe; sc2pe.Form("%e", c2pe);
      TString sr3fp; sr3fp.Form("%e", r3fp);
      TString sr3fpe; sr3fpe.Form("%e", r3fpe);
      TString sy2fr_chisquare; sy2fr_chisquare.Form("%e", y2fr_chisquare);
      TString sy2fr_ndf; sy2fr_ndf.Form("%f", y2fr_ndf);
      TString sy2ft_chisquare; sy2ft_chisquare.Form("%e", y2ft_chisquare);
      TString sy2ft_ndf; sy2ft_ndf.Form("%f", y2ft_ndf);
      
      //First test if this entry is already in database
      TString sqlStatement("select runnumber from countdist where runnumber=\"");
      sqlStatement+=runnumber;
      sqlStatement+="\" AND pass=\"";
      sqlStatement+=passname;
      sqlStatement+="\" AND modulename=\"";
      sqlStatement+=modname;
      sqlStatement+="\"";
      
      std::cout<<sqlStatement.Data()<<std::endl;
      res = db->Query(sqlStatement);
      std::cout<<" Number of rows: "<<res->GetRowCount()<<std::endl;
      bool entryfound = false;
      if(res->GetRowCount()>0)
        entryfound = true;
      delete res;
      
      // Update Database Entry
      TString updateStatement;
      if(entryfound){
        updateStatement="UPDATE countdist SET";
      }else{
        updateStatement="INSERT INTO countdist SET";
        updateStatement+=" runnumber=\""; updateStatement+=srunnumber; updateStatement+="\", ";
        updateStatement+=" pass=\""; updateStatement+=passname; updateStatement+="\", ";
        updateStatement+=" modulename=\""; updateStatement+=modname; updateStatement+="\",";
      }

      updateStatement+=" runduration=\""; updateStatement+=sduration; updateStatement+="\", ";
      updateStatement+=" timebinwidth=\""; updateStatement+=stimebinwidth;updateStatement+="\", ";
      updateStatement+=" r1=\""; updateStatement+=sr1;updateStatement+="\", ";
      updateStatement+=" r2f=\""; updateStatement+=sr2f;updateStatement+="\", ";
      updateStatement+=" r2fe=\""; updateStatement+=sr2fe;updateStatement+="\", ";
      updateStatement+=" lambda=\""; updateStatement+=slambda;updateStatement+="\", ";
      updateStatement+=" lambdae=\""; updateStatement+=slambdae;updateStatement+="\", ";
      updateStatement+=" c2=\""; updateStatement+=sc2;updateStatement+="\", ";
      updateStatement+=" c2e=\""; updateStatement+=sc2e;updateStatement+="\", ";
      updateStatement+=" r3f=\""; updateStatement+=sr3f;updateStatement+="\", ";
      updateStatement+=" r3fe=\""; updateStatement+=sr3fe;updateStatement+="\", ";
      updateStatement+=" r2fp=\""; updateStatement+=sr2fp;updateStatement+="\", ";
      updateStatement+=" r2fpe=\""; updateStatement+=sr2fpe;updateStatement+="\", ";
      updateStatement+=" lambdap=\""; updateStatement+=slambdap;updateStatement+="\", ";
      updateStatement+=" lambdape=\""; updateStatement+=slambdape;updateStatement+="\", ";
      updateStatement+=" c2p=\""; updateStatement+=sc2p;updateStatement+="\", ";
      updateStatement+=" c2pe=\""; updateStatement+=sc2pe;updateStatement+="\", ";
      updateStatement+=" r3fp=\""; updateStatement+=sr3fp;updateStatement+="\", ";
      updateStatement+=" r3fpe=\""; updateStatement+=sr3fpe;updateStatement+="\", ";
      updateStatement+=" y2fr_chisquare=\""; updateStatement+=sy2fr_chisquare;updateStatement+="\", ";
      updateStatement+=" y2fr_ndof=\""; updateStatement+=sy2fr_ndf;updateStatement+="\", ";
      updateStatement+=" y2ft_chisquare=\""; updateStatement+=sy2ft_chisquare;updateStatement+="\", ";
      updateStatement+=" y2ft_ndof=\""; updateStatement+=sy2ft_ndf;updateStatement+="\" ";
 
      if(entryfound){
        updateStatement+="WHERE runnumber=\"";
        updateStatement+=srunnumber;
        updateStatement+="\" AND pass=\"";
        updateStatement+=passname;
        updateStatement+="\" AND modulename=\"";
        updateStatement+=modname;
        updateStatement+="\"";        
      }
      
      
      // Execute insert
      cout<<updateStatement.Data()<<endl;
      res = db->Query(updateStatement.Data());
      delete res;


      // Open the svg file printed above and save in the database
      ifstream svgfile;
      svgfile.open(printName+".svg");
      if(svgfile.good())
      {
	// Read svg file into string buffer
	TString svgstring;
	svgstring.ReadFile(svgfile);
	TString stmtstr("UPDATE countdist SET plotsvg=? ");
        stmtstr+="WHERE runnumber=\"";
        stmtstr+=srunnumber;
        stmtstr+="\" AND pass=\"";
        stmtstr+=passname;
        stmtstr+="\" AND modulename=\"";
        stmtstr+=modname;
        stmtstr+="\"";        

	TSQLStatement* stmt =db->Statement(stmtstr.Data()); 
	stmt->NextIteration();
	stmt->SetString(0,svgstring.Data());
	stmt->Process();
	delete stmt;

      }
      svgfile.close();
      }
      
      delete ana;
      delete sysConf;
    }
    delete tmpfile;
    TCollection::EmptyGarbageCollection();
  }
  if(db) db->Close();
  tfout->Write();
  tfout->Close();
}
