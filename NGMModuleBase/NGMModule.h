#ifndef __MODULE__
#define __MODULE__

#include "TTask.h"
#include "TH1F.h"
#include "TObjString.h"

 // forward declarations
class TFolder;
class NGMParticleIdent;
class TSQLServer;
class NGMSystemConfiguration;
class NGMHit;
class NGMBufferedPacket;
//class TObjString;

class NGMModule: public TTask{
 public:
  NGMModule();
  NGMModule(const char*, const char*);
  virtual ~NGMModule();

  //These 3 functions are overloaded by the user.
  virtual bool init() = 0;
  virtual bool process(const TObject &);
  virtual bool finish() = 0;

  //operations that call the functions of this NGMModule and the daughters
  void initModules(); // *MENU*
  void push(const TObject&);
  void finishModules();// *MENU*
  void sendEndRunFlushAndSave() ; // send EndRunFlush and EndRunSave messages
  
  void setDebug(){_debug=true;}
  void clearDebug(){_debug=false;}
  bool getDebug(){return _debug;}
  // Print analysis summary to eps files
   void Print(Option_t* option = "") const {}
   void LaunchDisplayCanvas() {} // *MENU*
  // returns the Particle Identification object to add cuts, etc.
  virtual NGMParticleIdent* getParticleIdent();
  NGMModule* FindModule(const char* name) const;
  virtual void SetAllowDatabaseReads(bool newVal = true);
  virtual bool GetAllowDatabaseReads() const;
  static TString GetDBPassword();
  static TString GetDBUser();
  static TSQLServer* GetDBConnection();
  void  SetUpdateDB(bool updateDB = true) {_updateDB = updateDB;} // *MENU*
  bool  GetUpdateDB() {return _updateDB;}
  TFolder* GetParentFolder() { return _parentFolder; }
  virtual void SetRunNumber(Long64_t runnumber) { _runnumber = runnumber; }
  virtual void ResetData() {}
  static void  UpdateDatabaseRunHeader(NGMSystemConfiguration* sysConfig, TSQLServer* db = 0, bool forceUpdate = false);
  
    void RecursivelyRemoveChildren(); //Used only by destructor
  virtual void setNeutronIdParams(double ,double, double);
  virtual void AddCut(int tpartid, double tminenergy, double tmaxenergy);

 protected:
  virtual bool  processConfiguration(const NGMSystemConfiguration* conf){return true;}
  virtual bool  processHit(const NGMHit* tHit){return true;}
  virtual bool  processPacket(const NGMBufferedPacket* packet){return true;}
  virtual bool  processMessage(const TObjString* mess) {return true;}
  
  NGMParticleIdent* partID;

  bool GetRunInfoFromDatabase(const char* runnumber);
  
  Long64_t _runnumber;
  TString _facility;
  TString _series;
  TString _experimentname;
  TString _geoMacroName;
  TString _calibFileName;
  TString _testobjectid;
  virtual void InitCommon();
  
 private:
  TH1F* makeHistogram(const char* name,const char* description,int Nbins,int lowBin,int highBin);
  bool _initialized;
  bool _debug;  //show debug messages
  bool _allowDatabaseReads; // allow this module to read from the database
  bool _updateDB;
  TFolder* _parentFolder;
  
  ClassDef(NGMModule,6);
  
};
#endif
