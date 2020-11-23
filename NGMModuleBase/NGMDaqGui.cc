#include <TGClient.h> 
#include <TCanvas.h>  
#include <TF1.h> 
#include <TRandom.h> 
#include <TGButton.h> 
#include <TGFrame.h> 
#include <TRootEmbeddedCanvas.h> 
#include <RQ_OBJECT.h> 
#include <TApplication.h>
#include "NGMDaqGui.h"
#include "TGTab.h"
#include "TList.h"
#include "TGLabel.h"
#include "TGLayout.h"
#include "TGTableLayout.h"
#include "NGMSystem.h"
#include "TSystem.h"


typedef NGMTextEntry* txtentry_ptr;

ClassImp(NGMTextEntry)

NGMTextEntry::NGMTextEntry(const TGWindow* parent, const char* text, Int_t id)
: TGTextEntry(parent,text,id)
{
}


ClassImp(NGMTableGui)

NGMTableGui::NGMTableGui(const TGWindow *p,UInt_t w,UInt_t h)
: TGCanvas(p, w, h)
{

  _treatAsList = false;
  _numrows = 100;
  fTable = new TGHorizontalFrame(GetViewPort(),800,800);
  SetContainer(fTable);  
  for(int icol = 0; icol < 1; icol++)
  {
    TGVerticalFrame *fCol = new TGVerticalFrame(fTable, dColumnWidth,dColumnHeight );
    for(int irow = 0; irow < _numrows; irow++){
      TString textValue;
      if(irow == 0) textValue = "Row#";
      else textValue+=irow;
      NGMTextEntry* tEntry = new NGMTextEntry(fCol,(const char*)(textValue));
      fCol->AddFrame(tEntry,
                     new TGLayoutHints(
                                       kLHintsExpandX | kLHintsCenterY
                                       | kLHintsShrinkX
                                       ));
    }
    //fTable->AddFrame(fCol, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY|kLHintsFillX|kLHintsFillY));
    fTable->AddFrame(fCol, new TGLayoutHints(kLHintsLeft|kLHintsFillX|kLHintsFillY));
    
  }
  
  MapSubwindows();

}

int NGMTableGui::SyncCell(int index)
{
  if(!_ngmtable) return -1;
  
  int col = index/_numrows;
  int row = index%_numrows;

  if(_ngmtable->GetEntries()>1)
  {
    _ngmtable->SetParameterFromString(_ngmtable->GetParName(col-headerColumns),
                                      row-headerRows,
                                      GetCellText(col,row));
  }else{
    _ngmtable->SetParameterFromString(_ngmtable->GetParName(row-headerRows),
                                      col-headerColumns-1,GetCellText(col,row));
  }
  return 0;
}

int NGMTableGui::GetColumns() const
{
  return fTable->GetList()->LastIndex() + 1;
}

void NGMTableGui::RefreshGUITable()
{
  
  if(!_ngmtable) return ;
  
  if(_ngmtable->GetEntries()>1)
  {
    for(int col = headerColumns; col < GetColumns(); col++)
      for(int row = headerRows; row < _numrows; row++)
    SetCellText(col,row,_ngmtable->GetParameterAsString(_ngmtable->GetParName(col-headerColumns),
                                      row-headerRows));
  }else{
      int col = headerColumns+1;
      for(int row = headerRows; row < _numrows; row++)
        SetCellText(col,row,_ngmtable->GetParameterAsString(_ngmtable->GetParName(row-headerRows),
                                      col-headerColumns-1));
  }

}

void NGMTableGui::SetNRows(int newVal)
{
  if(newVal==_numrows) return;
  
  // iterate over each column and throw away extra entries
  else if(newVal < _numrows)
  {
    TList* fcolList = fTable->GetList();
    for(int icol = 0; icol <= fcolList->LastIndex(); icol++)
    {
      TGFrameElement *colel = (TGFrameElement*)( fTable->GetList()->At(icol));
      if(!colel) continue;
      TGCompositeFrame* colFrame = (TGCompositeFrame*)(colel->fFrame);
      if(!colFrame) continue;
      while(colFrame->GetList()->LastIndex() >= newVal)
      {
        TGFrameElement *rowel = (TGFrameElement*)(colFrame->GetList()->Last());
        if(!rowel) continue;
        TGTextEntry *rowFrame = (TGTextEntry*)(rowel->fFrame);
        if(!rowFrame) continue;
        colFrame->RemoveFrame(rowFrame);
        delete rowFrame;      
        
      }
    }
  }

  // iterate over each column and add empty entries
  else if(newVal > _numrows)
  {
    TList* fcolList = fTable->GetList();
    for(int icol = 0; icol <= fcolList->LastIndex(); icol++)
    {
      TGFrameElement *colel = (TGFrameElement*) (fTable->GetList()->At(icol));
      if(!colel) continue;
      TGCompositeFrame* colFrame = (TGCompositeFrame*)(colel->fFrame);
      if(!colFrame) continue;
      for(int irow = _numrows; irow < newVal; irow++)
      {
        TString textValue;
        if(icol == 0)
          textValue += irow;
        else
          textValue = "";
        NGMTextEntry* tEntry = new NGMTextEntry(colFrame,(const char*)(textValue));
        colFrame->AddFrame(tEntry,
                       new TGLayoutHints(
                                         kLHintsExpandX | kLHintsCenterY
                                         ));
        
      }
    }
  }
  _numrows = newVal;
  MapSubwindows();
}

void NGMTableGui::DisplayTable(NGMConfigurationTable* ngmtable)
{
  _ngmtable = ngmtable;
  
  // Should empty all but the headerColumn before proceeding
  
  // Check for Table or List
  
  if(_ngmtable->IsList()|| _ngmtable->GetEntries() == 1)
  {
    _treatAsList = true;
    SetNRows(_ngmtable->GetParCount() + headerRows);
    AddColumn("Name");
    AddColumn("Value");
    for(int irow = 0; irow < _ngmtable->GetParCount(); irow++)
    {
      SetCellText(headerColumns,irow + headerRows, _ngmtable->GetParName(irow));
      SetCellText(headerColumns+1, irow + headerRows,
              _ngmtable->GetParameterAsString(_ngmtable->GetParName(irow),0));
      
    }
  }else{
    //Table
    // Set Number of Rows Table entries plus one header row
    _treatAsList = false;
    SetNRows(_ngmtable->GetEntries() + headerRows);
    for(int icol = 0; icol < _ngmtable->GetParCount(); icol++)
    {
      AddColumn(_ngmtable->GetParName(icol));
      for(int irow = 0; irow < _numrows - headerRows; irow++)
      {
        SetCellText(icol+ headerColumns, irow + headerRows,
                ngmtable->GetParameterAsString(_ngmtable->GetParName(icol),irow));
      }
    }
  }
}

void NGMTableGui::AddColumn(const char* colName)
{
  TGVerticalFrame *fCol = new TGVerticalFrame(fTable,dColumnWidth,dColumnHeight);
  int indexOfThisColumn = fTable->GetList()->LastIndex()+1;
  char cbuf[1024];
  for(int irow = 0; irow < _numrows; irow++){
    TString textVal;
    if(irow ==0) textVal = colName;
    else textVal="";
    TGTextEntry* tEntry = new NGMTextEntry(fCol,(const char*)(textVal));
    sprintf(cbuf,"SyncCell(=%d)",indexOfThisColumn*_numrows+irow);
    tEntry->Connect("ReturnPressed()","NGMTableGui",this,cbuf);
    //    tEntry->Connect("TextChanged(char*)","NGMTableGui",this,cbuf);
    fCol->AddFrame(tEntry,
                   new TGLayoutHints(
                                     kLHintsExpandX | kLHintsCenterY
                                     ));
  }
  fTable->AddFrame(fCol, new TGLayoutHints(kLHintsLeft|kLHintsFillX|kLHintsFillY));
  MapSubwindows();
}

void NGMTableGui::RemoveColumn(int index)
{
  TGFrameElement *el = (TGFrameElement*)(fTable->GetList()->At(index));
  if(!el) return;
  fTable->RemoveFrame(el->fFrame);
  delete el->fFrame;
  MapSubwindows();
}

TGTextEntry* NGMTableGui::GetCell(int column, int row)
{
  TGFrameElement* colel = (TGFrameElement*) (fTable->GetList()->At(column));
  if(!colel) return 0;
  
  TGCompositeFrame* colFrame = (TGCompositeFrame*) colel->fFrame;
  if(!colFrame) return 0;
  
  TGFrameElement* rowel = (TGFrameElement*) (colFrame->GetList()->At(row));
  if(!rowel) return 0;
  
  TGTextEntry* tEntry = (TGTextEntry*) rowel->fFrame; 
  if(!tEntry) return 0;
  
  return tEntry;
  
}


const char* NGMTableGui::GetCellText(int column, int row) const
{
  TGFrameElement* colel = (TGFrameElement*) (fTable->GetList()->At(column));
  if(!colel) return "";
  
  TGCompositeFrame* colFrame = (TGCompositeFrame*) colel->fFrame;
  if(!colFrame) return "";
  
  TGFrameElement* rowel = (TGFrameElement*) (colFrame->GetList()->At(row));
  if(!rowel) return "";
  
  TGTextEntry* tEntry = (TGTextEntry*) rowel->fFrame; 
  if(!tEntry) return "";
  
  return  tEntry->GetText(); 
}

void NGMTableGui::SetCellText(int column, int row, const char* newVal)
{
  TGFrameElement* colel = (TGFrameElement*) (fTable->GetList()->At(column));
  if(!colel) return;
  
  TGCompositeFrame* colFrame = (TGCompositeFrame*) colel->fFrame;
  if(!colFrame) return;
  
  TGFrameElement* rowel = (TGFrameElement*) (colFrame->GetList()->At(row));
  if(!rowel) return;
  
  TGTextEntry* tEntry = (TGTextEntry*) rowel->fFrame; 
  if(!tEntry) return;
  
  tEntry->SetText(newVal); 
}

NGMDaqGui::NGMDaqGui(const TGWindow *p,UInt_t w,UInt_t h) { 

  Pixel_t colorGreen;
  Pixel_t colorRed;
  Pixel_t colorBlue;
  Pixel_t colorOrange;  
  gClient->GetColorByName("red",colorRed);
  gClient->GetColorByName("green",colorGreen);
  gClient->GetColorByName("blue",colorBlue);
  gClient->GetColorByName("orange",colorOrange);
  
  // Create a main frame 
  fMain = new TGMainFrame(p,w,h);
  
  fMain->DontCallClose();
  fMain->SetCleanup(kDeepCleanup);
  
  TGHorizontalFrame *fHoriz1 = new TGHorizontalFrame(fMain,w,h);
  fMain->AddFrame(fHoriz1, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY
                                            |kLHintsShrinkX|kLHintsShrinkY,2,2,2,2));
  TGVerticalFrame *controlFrame = new TGVerticalFrame(fHoriz1, 300,h-4);
  fHoriz1->AddFrame(controlFrame, new TGLayoutHints(kLHintsNormal|kLHintsExpandY
                                                    |kLHintsShrinkY,2,2,2,2));

  
  bStart = new TGTextButton(controlFrame,"&Start","NGMSystem::getSystem()->StartAcquisition()");
  bStart->SetBackgroundColor(colorGreen);
  controlFrame->AddFrame(bStart, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  bStop = new TGTextButton(controlFrame,"&End","NGMSystem::getSystem()->RequestAcquisitionStop()");
  bStop->SetBackgroundColor(colorRed);
  controlFrame->AddFrame(bStop, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  bConfigure = new TGTextButton(controlFrame,"&Configure","NGMSystem::getSystem()->ConfigureSystem()");
  bConfigure->SetBackgroundColor(colorBlue);
  controlFrame->AddFrame(bConfigure, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  TGTextButton* bSaveAna = new TGTextButton(controlFrame,"SaveAna","NGMSystem::getSystem()->SaveAnaTree()");
  bSaveAna->SetBackgroundColor(colorOrange);
  controlFrame->AddFrame(bSaveAna,new TGLayoutHints(kLHintsNormal,2,2,2,2));
  // Run Number
  tbRunNumL = new TGLabel(controlFrame,"Run 12345678112233");
  controlFrame->AddFrame(tbRunNumL, new TGLayoutHints(kLHintsNormal,2,2,2,2)); 
  // Number of Events
  TGLabel* tbNumEventsL = new TGLabel(controlFrame,"Total Events");
  controlFrame->AddFrame(tbNumEventsL, new TGLayoutHints(kLHintsNormal,2,2,2,2)); 
  tbNumEvents = new TGNumberEntry(controlFrame,0,10);
  controlFrame->AddFrame(tbNumEvents, new TGLayoutHints(kLHintsNormal,2,2,2,2));  
  // Live Time
  TGLabel* tbLiveTimeL = new TGLabel(controlFrame,"Live Time");
  controlFrame->AddFrame(tbLiveTimeL, new TGLayoutHints(kLHintsNormal,2,2,2,2)); 
  tbLiveTime = new TGNumberEntry(controlFrame,0,10);
  controlFrame->AddFrame(tbLiveTime, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  // Run Duration
  TGLabel* tbRunDurationL = new TGLabel(controlFrame,"Run Duration");
  controlFrame->AddFrame(tbRunDurationL, new TGLayoutHints(kLHintsNormal,2,2,2,2)); 
  tbRunDuration = new TGNumberEntry(controlFrame,0,10);
  controlFrame->AddFrame(tbRunDuration, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  // Live Fraction
  TGLabel* tbLiveFractionL = new TGLabel(controlFrame,"Live Time Fraction");
  controlFrame->AddFrame(tbLiveFractionL, new TGLayoutHints(kLHintsNormal,2,2,2,2)); 
  tbLiveFraction = new TGNumberEntry(controlFrame,0,10);
  controlFrame->AddFrame(tbLiveFraction, new TGLayoutHints(kLHintsNormal,2,2,2,2));

  TGTextButton* bTableRefresh = new TGTextButton(controlFrame,"&Refresh GUI");
  bTableRefresh->SetBackgroundColor(colorGreen);
  controlFrame->AddFrame(bTableRefresh, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  bTableRefresh->Connect("Clicked()","NGMDaqGui",this,"RefreshTables()");

  //SetHV Parameters
  TGTextButton* bHVOFF = new TGTextButton(controlFrame,"HVOFF","NGMSystem::getSystem()->GetConfiguration()->GetHVParameters()->SetParameterAsStringThatBeginsWith(\"Voltage\",\"0.0\",\"DetectorName\",\"\")");
  bHVOFF->SetBackgroundColor(colorGreen);
  controlFrame->AddFrame(bHVOFF, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  TGTextButton* bHVGL = new TGTextButton(controlFrame,"GL HV","NGMSystem::getSystem()->GetConfiguration()->GetDetectorParameters()->CopyParValuesMatching(\"TargetHV\",\"Voltage\",\"DetectorName\",\"GL\",NGMSystem::getSystem()->GetConfiguration()->GetHVParameters())");
  bHVGL->SetBackgroundColor(colorRed);
  controlFrame->AddFrame(bHVGL, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  TGTextButton* bHVGS = new TGTextButton(controlFrame,"GS HV","NGMSystem::getSystem()->GetConfiguration()->GetDetectorParameters()->CopyParValuesMatching(\"TargetHV\",\"Voltage\",\"DetectorName\",\"GS\",NGMSystem::getSystem()->GetConfiguration()->GetHVParameters())");
  bHVGS->SetBackgroundColor(colorRed);
  controlFrame->AddFrame(bHVGS, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  TGTextButton* bHVMU = new TGTextButton(controlFrame,"MU HV","NGMSystem::getSystem()->GetConfiguration()->GetDetectorParameters()->CopyParValuesMatching(\"TargetHV\",\"Voltage\",\"DetectorName\",\"MU\",NGMSystem::getSystem()->GetConfiguration()->GetHVParameters())");
  bHVMU->SetBackgroundColor(colorRed);
  controlFrame->AddFrame(bHVMU, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  TGTextButton* bHVHE = new TGTextButton(controlFrame,"HE HV","NGMSystem::getSystem()->GetConfiguration()->GetDetectorParameters()->CopyParValuesMatching(\"TargetHV\",\"Voltage\",\"DetectorName\",\"HETTL\",NGMSystem::getSystem()->GetConfiguration()->GetHVParameters())");
  bHVHE->SetBackgroundColor(colorRed);
  controlFrame->AddFrame(bHVHE, new TGLayoutHints(kLHintsNormal,2,2,2,2));
  TGTextButton* bHVLS = new TGTextButton(controlFrame,"LS HV","NGMSystem::getSystem()->GetConfiguration()->GetDetectorParameters()->CopyParValuesMatching(\"TargetHV\",\"Voltage\",\"DetectorName\",\"LS\",NGMSystem::getSystem()->GetConfiguration()->GetHVParameters())");
  bHVLS->SetBackgroundColor(colorRed);
  controlFrame->AddFrame(bHVLS, new TGLayoutHints(kLHintsNormal,2,2,2,2));

    TGTab* mainTab = new TGTab(fHoriz1,w-300,h-4);
    TGCompositeFrame* RatesTab = mainTab->AddTab("Rates");
    TGHorizontalFrame *fRatesHoriz = new TGHorizontalFrame(RatesTab);
    RatesTab->AddFrame(fRatesHoriz, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,kLHintsShrinkX|kLHintsShrinkY));
    
    
    TGCompositeFrame* confTab = mainTab->AddTab("Expert");
    


  TGTab* parTabs = new TGTab(confTab,w - 300,h-4);
  confTab->AddFrame(parTabs,new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,2,2,2,2));
  TGCompositeFrame* sysParTab = parTabs->AddTab("System");
  sysPar = new NGMTableGui(sysParTab,600,600);
  sysParTab->AddFrame(sysPar, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,2,2,2,2)); 
  TGCompositeFrame* slotParTab = parTabs->AddTab("Slot");
  slotPar = new NGMTableGui(slotParTab,600,600);
  slotParTab->AddFrame(slotPar, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,2,2,2,2)); 
  TGCompositeFrame* chanParTab = parTabs->AddTab("Channel");
  chanPar = new NGMTableGui(chanParTab,600,600);
  chanParTab->AddFrame(chanPar, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,2,2,2,2)); 
  TGCompositeFrame* hvParTab = parTabs->AddTab("HV");
  hvPar = new NGMTableGui(hvParTab,600,600);
  hvParTab->AddFrame(hvPar, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,2,2,2,2)); 
  TGCompositeFrame* detParTab = parTabs->AddTab("Detector");
  detPar = new NGMTableGui(detParTab,600,600);
  detParTab->AddFrame(detPar, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,2,2,2,2)); 
  
  fHoriz1->AddFrame(mainTab, new TGLayoutHints(kLHintsShrinkX|kLHintsShrinkY|kLHintsExpandX|kLHintsExpandY,2,2,2,2));
  
  // Set a name to the main frame 
  fMain->SetWindowName("DAQ Control GUI"); 
  
  // Map all subwindows of main frame 
  fMain->MapSubwindows(); 
  
  // Initialize the layout algorithm 
  fMain->Resize(fMain->GetDefaultSize()); 
  
  // Map main frame 
  fMain->MapWindow(); 

  gSystem->AddTimer(&_runstatTimer);

  _runstatTimer.Connect("Timeout()","NGMDaqGui",this,"UpdateRunStats()");
  _runstatTimer.Start(1000);

}

void NGMDaqGui::RefreshTables()
{
  sysPar->RefreshGUITable();
  slotPar->RefreshGUITable();
  chanPar->RefreshGUITable();
  hvPar->RefreshGUITable();
  detPar->RefreshGUITable();

}

void NGMDaqGui::UpdateRunStats(){
  if(NGMSystem::getSystem())
  {
    tbNumEvents->SetNumber(NGMSystem::getSystem()->GetTotalEventsThisRun());
    tbLiveTime->SetNumber(NGMSystem::getSystem()->GetLiveTime());
    tbRunDuration->SetNumber(NGMSystem::getSystem()->GetRunDuration());
    if(NGMSystem::getSystem()->GetRunDuration()>0.0)
      tbLiveFraction->SetNumber(NGMSystem::getSystem()->GetLiveTime()
                                 /NGMSystem::getSystem()->GetRunDuration());
    TString runText("Run ");
    runText+= NGMSystem::getSystem()->GetConfiguration()->getRunNumber();
    tbRunNumL->ChangeText(runText.Data());
    if(NGMSystem::getSystem()->GetRunStatus())
    {
      bStart->SetEnabled(false);
      bConfigure->SetEnabled(false);
      bStop->SetEnabled(true);
    }else{
      bStart->SetEnabled(true);
      bConfigure->SetEnabled(true);
      bStop->SetEnabled(false);      
    }
  }else{
     bStart->SetEnabled(false);
     bStop->SetEnabled(false);
     bConfigure->SetEnabled(false);     
  }
}

void NGMDaqGui::DoDraw() { 
  // Draws function graphics in randomly choosen interval 
  TF1 *f1 = new TF1("f1","sin(x)/x",0,gRandom->Rndm()*10); 
  f1->SetFillColor(19); 
  f1->SetFillStyle(1); 
  f1->SetLineWidth(3); 
  f1->Draw(); 
  TCanvas *fCanvas = fEcanvas->GetCanvas();  
  fCanvas->cd(); 
  fCanvas->Update(); 
} 
NGMDaqGui::~NGMDaqGui() { 
  // Clean up used widgets: frames, buttons, layouthints 
  fMain->Cleanup(); 
  delete fMain; 
}

void NGMDaqGui::DisplayConfiguration(NGMSystemConfiguration* config)
{
  if(!config) return;
  if(config->GetSystemParameters())
    sysPar->DisplayTable(config->GetSystemParameters());
  if(config->GetSlotParameters())
    slotPar->DisplayTable(config->GetSlotParameters());
  if(config->GetChannelParameters())
    chanPar->DisplayTable(config->GetChannelParameters());
  if(config->GetHVParameters())
    hvPar->DisplayTable(config->GetHVParameters());
  if(config->GetDetectorParameters())
    detPar->DisplayTable(config->GetDetectorParameters());
  TString runText("Run ");
  runText+= config->getRunNumber();
  tbRunNumL->ChangeText(runText.Data());
  
}

void example() { 
  // Popup the GUI... 
//  new MyMainFrame(gClient->GetRoot(),600,600); 
} 

