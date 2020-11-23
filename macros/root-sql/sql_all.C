#ifndef __CINT__
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#endif
   
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

//
// Example of TSQL functions
//
// Adapted from root/tutorials/sql/*
//
// Usage:
//
//  root[] .L <this file>  # load this file
//  sqlcreatedb()          # create table 'runcatalog' in dB test
//  sqlfilldb()            # fill the table with fake data
//  sqlselect()            # print server, dB, and table info
//                         # print rows from a select on the table
//  sqlttree()             # get rows into a TTree

void sqlcreatedb()
{
   // Create a runcatalog table in a MySQL test database.
   
   // read in runcatalog table definition
  char fileName[]="macros/root-sql/runcatalog.sql";
  ifstream fin(fileName, ifstream::in);

  // read the lines
  TString *sql = new TString();
  sql->ReadFile(fin);
   
   // open connection to MySQL server on localhost
   //   TSQLServer *db = TSQLServer::Connect("mysql://localhost/test", "nobody", "");
  TSQLServer* db = dBConnect();
   
   TSQLResult *res;

   // create new table (delete old one first if exists)
   res = db->Query("DROP TABLE IF EXISTS runcatalog");
   delete res;

   printSQL("Create table query", sql->Data());

   cout<<"Creating table..."<<std::flush;
   res = db->Query(sql->Data());
   cout<<"done."<<endl;
   
   delete res;
   delete db;
   delete sql;
}

void sqlfilldb(int nfiles = 1000)
{
   // Fill run catalog with nfiles entries
  cout<<"Filling runcatalog."<<endl;
   
   const char *ins = "INSERT INTO runcatalog VALUES ('%s', %d,"
      " %d, %d, %d, %10.2f, '%s', '%s', '1997-01-15 20:16:28',"
      " '1999-01-15 20:16:28', '%s', '%s')";
   
   char sql[4096];
   char dataset[32];
   char rawfile[128];
   int  tag, evt = 0;
   
   // open connection to MySQL server on localhost
   //   TSQLServer *db = TSQLServer::Connect("mysql://localhost/test", "nobody", "");
  TSQLServer* db = dBConnect();
   TSQLResult *res;
   
   // first clean table of old entries
   cout<<"Deleting any existing rows..."<<std::flush;
   res = db->Query("DELETE FROM runcatalog");
   delete res;
   cout<<"done."<<endl;

   // start timer
   TStopwatch timer;
   timer.Start();
   
   // fill run catalog
   for (int i = 0; i < nfiles; i++) {
      sprintf(dataset, "testrun_%d", i);
      sprintf(rawfile, "/v1/data/lead/test/run_%d.root", i);
      tag = int(gRandom->Rndm()*10.);
      sprintf(sql,
	      ins,
	      dataset,
	      i,
	      evt,
	      evt+10000,
	      tag,
	      25+rand()%5,
	      "test",
	      "lead",
              rawfile,
	      "test run dummy data");
      evt += 10000;

      if (i == 0) {
	printSQL("Fill table query", sql);
	cout<<"Starting fill: "<<std::flush;
      }

      res = db->Query(sql);
      delete res;
      //	  printf("%s\n", sql);
      if (i % 100 == 0 ) {
	cout<<i<<std::flush;
      } else if (i % 20 == 0) {
	cout<<"."<<std::flush;
      }
   }
   cout<<endl;
   
   delete db;

   // stop timer and print results
   timer.Stop();
   Double_t rtime = timer.RealTime();
   Double_t ctime = timer.CpuTime();

   cout<<"Fill completed, ";

   cout<<nfiles<<" files in run catalog."<<endl;
   cout<<"RealTime= "<<rtime<<" seconds";
   cout<<", CpuTime= "<<ctime<<" seconds"<<endl;
}

void coutRes(TSQLResult * res)
{
   TSQLRow *row;

   while ((row = res->Next())) {
     cout<<"\t'"<<row->GetField(0)<<"'\t("<<row->GetFieldLength(0)<<")"<<endl;;
      delete row;
   }
   delete res;

}

void sqlselect()
{
  //   TSQLServer *db = TSQLServer::Connect("mysql://localhost/test","nobody", "");
  TSQLServer* db = dBConnect();

  cout<<"Server info: "<<db->ServerInfo()<<endl;;
   
   TSQLResult *res;
   
   // list databases available on server
   cout<<"\nList all databases on server "<<db->GetHost()<<endl;;
   res = db->GetDataBases();
   coutRes(res);

   // list tables in database "test" (the permission tables)
   cout<<"\nList all tables in database \"test\" on server ";
   cout<<db->GetHost()<<endl;
   res = db->GetTables("test");
   coutRes(res);
   
   // list columns in table "runcatalog" in database "mysql"
   cout<<"\nList all columns in table \"runcatalog\" in database \"test\" on server ";
   cout<<db->GetHost()<<endl;;
   res = db->GetColumns("test", "runcatalog");
   coutRes(res);

   // start timer
   TStopwatch timer;
   timer.Start();

   // query database and print results
   const char *sql = "select tag,dataset,rawfilepath from test.runcatalog "
	 "WHERE (tag<3) AND (run<50)";
//   const char *sql = "select count(*) from test.runcatalog "
//                     "WHERE tag&(1<<2)";
   printSQL("Select query", sql);

   res = db->Query(sql);

   int nrows = res->GetRowCount();
   cout<<"\nGot "<<nrows<<" rows in result"<<endl;
   
   int nfields = res->GetFieldCount();
   const int width[3] = {4, 20, 50};
   for (int i = 0; i < nfields; i++) {
	 cout.width(width[i]);
	 cout<<res->GetFieldName(i);
	 cout<<"  ";
   }
   cout<<endl;;
   for (int i = 0; i < nfields; i++) {
	 for (int j = 0; j < width[i]; j++) {
      cout<<"=";
	 }
	 cout<<"  ";
   }
   cout<<endl;;
   
   for (int i = 0; i < nrows; i++) {
      row = res->Next();
      for (int j = 0; j < nfields; j++) {
		cout.width(width[j]);
		cout<<row->GetField(j);
		cout<<"  ";
      }
      cout<<endl;
      delete row;
   }
   
   delete res;
   delete db;

   // stop timer and print results
   timer.Stop();
   Double_t rtime = timer.RealTime();
   Double_t ctime = timer.CpuTime();

   cout<<"\nRealTime="<<rtime<<" seconds, ";
   cout<<"CpuTime="<<ctime<<" seconds"<<endl;
}

TTreeSQL * sqlttreesql()
{
  //   TSQLServer *db = TSQLServer::Connect("mysql://localhost/test","nobody", "");
  TSQLServer* db = dBConnect();

  TTreeSQL* nt = new TTreeSQL(db,"test","runcatalog");

  nt->Draw("energy");

  //gSystem->Load("TTreeViewer");
  TTreeViewer *tv = new TTreeViewer(nt);
  return nt;  
}

struct RunRecord {
  char * dataset;
  int    run;
  int    firstevent;
  int    events;
  int    tag;
  double energy;
  char * runtype;
  char * target;
  char * timef;
  char * timel;
  char * rawfilepath;
  char * comments;
};

void printRun (RunRecord r)
{
  cout<<"\tDataset:\t"      << r.dataset;
  cout<<"\trun:\t"          << r.run;
  cout<<"\tfirstevent:\t"   << r.firstevent;
  cout<<"\tevents:\t"       << r.events;
  cout<<"\ttag:\t"          << r.tag;
  cout<<"\tenergy:\t"       << r.energy;
  cout<<"\truntype:\t"      << r.runtype;
  cout<<"\ttarget:\t"       << r.target;
  cout<<"\ttimef:\t"        << r.timef;
  cout<<"\ttimel:\t"        << r.timel;
  cout<<"\trawfilepath:\t"  << r.rawfilepath;
  cout<<"\tcomments:\t"     << r.comments;
  cout<<endl;
}

/*
Class RunRecord {
  TString dataset;
  Int_t   run;
  Int_t   firstevent;
  Int_t   events;
  Int_t   tag;
  Double_t energy;
  TString runtype;
  TString target;
  TString timef;
  TString timel;
  TString rawfilepath;
  TString comments;
  
  RunRecord(TString da,
	    Int_t   ru,
	    Int_t   fi,
	    Int_t   ev,
	    Int_t   ta,
	    Double_t en,
	    TString rt,
	    TString tr,
	    TString tf,
	    TString tl,
	    TString ra,
	    TString co)
    {
      dataset    =da;
      run        =ru;
      firstevent =fi;
      events     =ev;
      tag        =ta;
      energy     =en;
      runtype    =rt;
      target     =tr;
      timef      =tf;
      timel      =tl;
      rawfilepath=ra;
      comments   =co;
    }
  ~RunRecord(){};
}
*/

TString field(TSQLRow *row, int i )
{
  //return TString(row->GetField(i),row->GetFieldLength(i)) ; }
  return TString(row->GetField(i)) ;
}

TTree * sqlttree()
{
  // Read the table into a tree

  // Create the ttree
  TTree *tree = new TTree("runcatalog","Test TTree from TSQL select query.");
  Int_t split = 0;
  Int_t bsize = 64000;

  // define the fields
  RunRecord runR ;
//   TString dataset;
//   Int_t   run;
//   Int_t   firstevent;
//   Int_t   events;
//   Int_t   tag;
//   Double_t energy;
//   TString runtype;
//   TString target;
//   TString timef;
//   TString timel;
//   TString rawfilepath;
//   TString comments;
  tree->Branch("dataset",     &(runR.dataset),     "dataset/C");
  tree->Branch("run",         &(runR.run),         "run/I");
  tree->Branch("firstevent",  &(runR.firstevent),  "firstevent/I");
  tree->Branch("events",      &(runR.events),      "events/I");
  tree->Branch("tag",         &(runR.tag),         "tag/I");
  tree->Branch("energy",      &(runR.energy),      "energy/D");
  tree->Branch("runtype",     &(runR.runtype),     "runtype/C");
  tree->Branch("target",      &(runR.target),      "target/C");
  tree->Branch("timef",       &(runR.timef),       "timef/C");
  tree->Branch("timel",       &(runR.timel),       "timel/C");
  tree->Branch("rawfilepath", &(runR.rawfilepath), "rawfilepath/C");
  tree->Branch("comments",    &(runR.comments),    "comments/C");

  //char keyname[16];

  //   TSQLServer *db = TSQLServer::Connect("mysql://localhost/test","nobody", "");
  TSQLServer* db = dBConnect();
  TSQLResult *res;
  // query database and print results
  const char *sql = "SELECT * FROM test.runcatalog";
  printSQL("Select query", sql);
  
  res = db->Query(sql);

  int nrows = res->GetRowCount();
  int nfields = res->GetFieldCount();
  cout<<"\nGot "<<nrows<<" rows of "<<nfields<<" fields in result."<<endl;
  
  TSQLRow *row;

  for (int i = 0; i < nrows; i++) {
    row = res->Next();
    
    runR.dataset     = field(row, 0).Data();
    runR.run         = field(row, 1).Atoi();
    runR.firstevent  = field(row, 2).Atoi();
    runR.events      = field(row, 3).Atoi();
    runR.tag         = field(row, 4).Atoi();
    runR.energy      = field(row, 5).Atof();
    runR.runtype     = field(row, 6).Data();
    runR.target      = field(row, 7).Data();
    runR.timef       = field(row, 8).Data();
    runR.timel       = field(row, 9).Data();
    runR.rawfilepath = field(row,10).Data();
    runR.comments    = field(row,11).Data();

    tree->Fill();

    // busy dots
    if (i % 100 == 0 ) {
      cout<<i<<std::flush;
    } else if (i % 20 == 0) {
      cout<<"."<<std::flush;
    } else if (i < 20 ) {
      printRun(runR);
    }
    
    delete row;
   }
  cout<<endl; // finish busy dots
   
   delete res;
   delete db;

   return tree;
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

void printSQL(char * hdr, char * sql)
{
  cout<<hdr<<" <<EOF"<<endl;
  cout<<"\t"<<sql<<endl;
  cout<<"EOF"<<endl;
}
	      
