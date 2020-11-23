// $Id: makeDbView.C 710 2009-01-26 23:16:40Z barnes $
// Author: Peter Barnes 2008-10

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// TTree * SNTree = DbView("pdbSNTree", "CDistSN");

TTree * DbView(TString table,     // <table> parameter
	       TString module     // <module> parameter 
	       )
{
  
  // set up the dB connection
  TSQLServer* dbServ = dBConnect();

  // read parametrized sql command file
  TString *cvf = new TString("macros/createView-fl-cd-cw.sql");
  cout<<"Reading sql file "<<cvf->Data()<<endl;
  TString *stmt = slurpFile(cvf) ;

  // open sql command file to create the view 'pdbtest'
  createView(dbServ,stmt,&table,&module);

  // get the ntuple
  cout<<"Getting "<<table.Data()<<" ntuple..."<<std::flush;
  TTreeSQL* nt = new TTreeSQL(dbServ,"ngmrunlog",table.Data());
  cout<<"done."<<endl;

  nt->Draw("r1");

  TTreeViewer *tt = new TTreeViewer(nt);

  //  dropView(dbServ,&table);  // drop the view
  dBClose(dbServ);        // close the dB connection

  //delete dbServ;
  delete cvf;
  delete stmt;

  return nt;
}

TSQLServer* dBConnect() {
  // Get db server connection
  char host[]="mysql://hip.llnl.gov/ngmrunlog";
  
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

void dBClose(TSQLServer* dbServ) {
  cout<<"Closing db connection."<<endl;
  dbServ->Delete();
}
  
TString * slurpFile( TString *fileName)
{
  ifstream fin(fileName->Data(), ifstream::in);

  cout<<"Slurping file "<<fileName->Data()<<"..."<<std::flush;
  if (! fin.good()) {
	cout<<"Failed!"<<endl;
	return;
  } else {
	cout<<"done."<<endl;
  }

  // read the lines
  TString *s = new TString();
  s->ReadFile(fin);

  cout<<"Read statement <<EOF"<<endl;
  cout<<s->Data()<<endl;
  cout<<"EOF"<<endl;

  return s;
}

void createView(TSQLServer* dbServ,  // the server
		TString * stmt,      // parametrized SQL
		TString * table,     // <table> parameter
		TString * module     // <module> parameter
		)
{
  // replace the parameters
  TString s = *stmt;
  s.ReplaceAll("<table>",  table->Data());
  s.ReplaceAll("<module>", module->Data());

  cout<<"Read statement <<EOF"<<endl;
  cout<<s.Data()<<endl;
  cout<<"EOF"<<endl;
  s.ReplaceAll("\n", " ");

  // create and process the Statement object
  cout<<"Processing view query..."<<std::flush;
  TSQLResult *qr = dbServ->Query(s.Data());
  cout<<"done. ("<<qr->GetRowCount()<<" rows)"<<endl;

  delete qr;
}

void dropView(TSQLServer* dbServ, TString * view)
{
  TString * stmt = new TString("DROP VIEW ngmrunlog.");
  *stmt += *view;
  
  cout<<"Drop statement <<EOF"<<endl;
  cout<<stmt->Data()<<endl;
  cout<<"EOF"<<endl;

  cout<<"Dropping view "<<view->Data()<<"..."<<std::flush;
  TSQLResult *qr = dbServ->Query(stmt->Data());
  cout<<" ("<<qr->GetRowCount()<<" rows)"<<endl;

  delete stmt;
}

void printRows(TSQLResult *res)
{
  TSQLRow *row;
  
  while ((row = res->Next())) {
	printf("%s\n", row->GetField(0));
	delete row;
  }
}

void dumpRows(TSQLResult *res)
{
  TSQLRow *row;

  int nfields = res->GetFieldCount();
  int nrows = res->GetRowCount();


  for (int i = 0; i < nfields; i++) {
	if (i > 0) cout<<"  ";
	cout.width(18);
	cout<< res->GetFieldName(i);
  }
  cout<<endl;

  for (int i = 0; i < nfields; i++)
	cout<<"==================  ";
  cout<<endl;

  
  for (int i = 0; i < nrows; i++) {
	row = res->Next();
	for (int j = 0; j < nfields; j++) {
	  if (j > 0) cout<<"  ";
	  cout.width(18);
	  // if (j > 0) cout<<"\t";
	  cout<<row->GetField(j);
	}
	cout<<endl;
	delete row;
  }
}
