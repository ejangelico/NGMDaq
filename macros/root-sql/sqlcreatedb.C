void sqlcreatedb()
{
   // Create a runcatalog table in a MySQL test database.
   
   // read in runcatalog table definition
   FILE *fp = fopen("runcatalog.sql", "r");
   const char sql[4096];
   fread(sql, 1, 4096, fp);
   fclose(fp);
   
   // open connection to MySQL server on localhost
   //   TSQLServer *db = TSQLServer::Connect("mysql://localhost/test", "nobody", "");
  TSQLServer* db = dBConnect();
   
   TSQLResult *res;

   // create new table (delete old one first if exists)
   res = db->Query("DROP TABLE runcatalog");
   delete res;
   
   res = db->Query(sql);
   delete res;
   
   delete db;
}

TSQLServer* dBConnect() {
  // Get db server connection
  char host[]="mysql://hip.llnl.gov/test";
  
  cout<<"Connecting to db at "<<host<<"..."<<std::flush;
  TSQLServer* dbServ =
	TSQLServer::Connect(host,"ngmdaq","NGMp3daq");

  if ((dbServ!=0) && dbServ->IsConnected()) {
	cout<<"succeeded."<<endl;
  } else {
	cout<<"failed"<<endl;
	cerr<<"Connection to db failed: ";
	cerr<<"(code:"<<dbServ->GetErrorCode()<<")";
	cerr<<", msg:"<<dbServ->GetErrorMsg()<<endl;
	// should exit here!
  }

  return dbServ ;
}

