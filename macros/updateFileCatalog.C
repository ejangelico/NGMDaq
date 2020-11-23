void updateFileCatalog(const char* fname)
{

  std::string password;
  ifstream pfile(TString(gSystem->HomeDirectory())+"/.ngmdbpw");
  pfile>>password;
  pfile.close();
	
	TSQLServer* db = TSQLServer::Connect("mysql://ngmgrp.llnl.gov/ngmrunlog","ngmdaq",password.c_str());
	if(!db)
	{
		cout<<"Unable to connect to run database"<<endl;
		return;
	}
	
	TSQLResult* res;
	
	ifstream infile(fname);
	char pathname[1024];
	char countname[1024];
	while(infile>>pathname)
	{
		TString pName(pathname);
		TString runnumber;
		// skip all but first segment
		if(pName.Contains("SISRaw_"))
		{
		  runnumber=pName(pName.Index("SISRaw_")+7,14);
		}else{
		  if(!pName.Contains("ngm.root")) continue;
		  runnumber = pName(pName.Index("-ngm.root")-14,14);
		}
		
		ifstream countfile(fname);
		int numsegments = 0;
		while(countfile>>countname)
		{
			TString teststring(countname);
			if(teststring.Contains(runnumber)) numsegments++;
		}
		countfile.close();
		
		
		cout<<runnumber.Data()<<" "<<numsegments<<" "<<pName.Data()<<endl;
		//First test if this runnumber is already in database
		TString sqlStatement("select runnumber from filelog where runnumber=\"");
		sqlStatement+=runnumber;
		sqlStatement+="\"";
		std::cout<<sqlStatement.Data()<<std::endl;
		
		res = db->Query(sqlStatement);
		std::cout<<" Number of rows: "<<res->GetRowCount()<<std::endl;
		bool runNumberFound = false;
		if(res->GetRowCount()>0)
			runNumberFound = true;
		
		delete res;
		
		// Form Update or insert statement
		TString updateStatement;
		if(runNumberFound){
			updateStatement="UPDATE filelog SET";
		}else{
			updateStatement="INSERT INTO filelog SET";
			updateStatement+=" runnumber=\""; updateStatement+=runnumber; updateStatement+="\", ";
		}
		
		updateStatement+=" filename=\""; updateStatement+=pName.Data(); updateStatement+="\", ";
		updateStatement+=" rawfilesegments="; updateStatement+=numsegments; updateStatement+=" ";
		
		if(runNumberFound){
			updateStatement+="WHERE runnumber=\"";
			updateStatement+=runnumber;
			updateStatement+="\"";
		}
		
		
		// Execute the db statement
		cout<<updateStatement.Data()<<endl;
		res = db->Query(updateStatement.Data());
		delete res;

		
	}
	infile.close();
}
