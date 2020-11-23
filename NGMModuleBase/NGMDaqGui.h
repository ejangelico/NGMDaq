#include "RQ_OBJECT.h"
#include "TGCanvas.h"
#include "NGMConfigurationTable.h"
#include "NGMSystemConfiguration.h"
#include "TGTextEntry.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"
#include "TRootEmbeddedCanvas.h"
#include "TTimer.h"

class NGMTextEntry : public TGTextEntry
{
public:
   NGMTextEntry(const TGWindow* parent = 0, const char* text = 0, Int_t id = -1);
   ~NGMTextEntry(){}
   void TestMenu(){} // *MENU*
   
   ClassDef(NGMTextEntry,0)
};


class NGMTableGui : public TGCanvas
{
  RQ_OBJECT("NGMTableGui")
public:

  ClassDef(NGMTableGui,0)

  NGMTableGui(const TGWindow *p,UInt_t w,UInt_t h); 
  void DisplayTable(NGMConfigurationTable* ngmtable);
  void AddColumn(const char* colName);
  void RemoveColumnByName(const char* colName){}
  void RemoveColumn(int index);
  const char* GetCellText(int column, int row) const;
  void SetCellText(int column, int row, const char* newVal);
  void SetNRows(int newVal);
  int GetColumns() const;
  int GetRows() const { return _numrows; }
  int SyncCell(int index);
  
  TGTextEntry* GetCell(int column, int row);
  void RefreshGUITable();

  private:
  bool _treatAsList;
  NGMConfigurationTable* _ngmtable;
  enum defaults {dColumnWidth=400, dColumnHeight=100, headerRows =1, headerColumns =1};
  TGHorizontalFrame *fTable;
  int _numrows;


};


class NGMDaqGui {
  RQ_OBJECT("NGMDaqGui") 
private: 
  TGMainFrame         *fMain; 
  TRootEmbeddedCanvas *fEcanvas; 
public: 
    NGMDaqGui(const TGWindow *p,UInt_t w,UInt_t h);
  NGMTableGui* getChannelTableGui(){ return chanPar;}
  void DisplayConfiguration(NGMSystemConfiguration* config);
  virtual ~NGMDaqGui(); 
  void UpdateRunStats();
  void RefreshTables();

  enum controlid {bSTART,bSTOP};
  void DoDraw(); 
  TGTextEntry ** textTable;
  NGMTableGui* sysPar;
  NGMTableGui* slotPar;
  NGMTableGui* chanPar;
  NGMTableGui* hvPar;
  NGMTableGui* detPar;

  TGTextButton* bStart;
  TGTextButton* bStop;
  TGTextButton* bConfigure;
  
  TGNumberEntry* tbNumEvents;
  TGNumberEntry* tbLiveTime;
  TGNumberEntry* tbRunDuration;
  TGNumberEntry* tbLiveFraction;
  TGLabel* tbRunNumL;
  TTimer _runstatTimer;
  int nrows;
  int ncols;
};
