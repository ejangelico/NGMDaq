#include "NGMZoomGui.h"
#include "TGLabel.h"
#include "TGScrollBar.h"
//#include "TGMdiMainFrame.h"
#include "TGCanvas.h"
#include "TGFrame.h"
#include "TRootEmbeddedCanvas.h"
#include "TCanvas.h"
#include "NGMCanvas.h"
//#include "Riostream.h" //NEED TO CHECK
#include "TAxis.h"
#include "TROOT.h"
#include "TGClient.h"
#include <iostream>

using namespace std;

void NGMZoomGui::setAxis(TAxis* axis){
  _axList.Clear();
  _axList.AddLast(axis);
  int bins = axis->GetNbins();
  zoomScrollBar->SetRange((Int_t)(1.1*bins),(Int_t)(0.1*bins));
  zoomScrollBar->SetPosition(bins);
  slideScrollBar->SetRange((Int_t)(1.1*bins),(Int_t)(0.1*bins));
  slideScrollBar->SetPosition(bins/2);
}

void NGMZoomGui::AddAxis(TAxis* axis){
  if (_axList.GetSize() == 0) setAxis(axis);
  else _axList.AddLast(axis);
}

void NGMZoomGui::doSlider1(Int_t pos){
	if(_axList.GetSize() == 0) return;
	for (int iax=0; iax<_axList.GetSize(); iax++){
		TAxis* _axis = (TAxis*)_axList.At(iax);
		int d = (int)(_axis->GetNbins()*pos/(float)(zoomScrollBar->GetRange()-zoomScrollBar->GetPageSize())/2+0.5);
		int m = (_axis->GetLast()+_axis->GetFirst())/2;
		_axis->SetRange(m-d,m+d);
	}
	myCanvas->Modified();
	myCanvas->Update();
	myCanvas->Draw();
}

void NGMZoomGui::doSlider2(Int_t pos){
	if(_axList.GetSize() == 0) return;
	for (int iax=0; iax<_axList.GetSize(); iax++){
		TAxis* _axis = (TAxis*)_axList.At(iax);
		int d = _axis->GetLast() - _axis->GetFirst();
		int m = (int)(pos/(float)(slideScrollBar->GetRange()-slideScrollBar->GetPageSize())*(_axis->GetNbins()-d));  
		//  if(m==0) return;
		_axis->SetRange(m+1,m+d+1);
	}
  	myCanvas->Modified();
	myCanvas->Update();
	myCanvas->Draw();
}

void NGMZoomGui::CloseWindow(){
  delete this;
}

NGMZoomGui::NGMZoomGui()
{
   gROOT->SetStyle("Plain");
   // main frame
   //zoomerMainFrame = new TGMainFrame(0,10,10,kMainFrame | kVerticalFrame);
   zoomerMainFrame = new TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame);
   zoomerMainFrame->SetCleanup(kDeepCleanup);//ROOT warns this may be dangerous

   // vertical frame
   myVerticalFrame = new TGVerticalFrame(zoomerMainFrame,692,483,kVerticalFrame);

   // embedded canvas
   zoomerEmbeddedCanvas = new TRootEmbeddedCanvas(0,myVerticalFrame,652,323);
   UInt_t w = zoomerEmbeddedCanvas->GetCanvas()->GetWindowWidth();
   UInt_t h = zoomerEmbeddedCanvas->GetCanvas()->GetWindowHeight();
   Int_t winId = zoomerEmbeddedCanvas->GetCanvasWindowId();
   TCanvas* oldCanvas = zoomerEmbeddedCanvas->GetCanvas();
   myCanvas = new NGMCanvas(Form("%s_canvas", zoomerEmbeddedCanvas->GetName()), w, h, winId);
   myCanvas->EmbedInto(winId,w, h);
   zoomerEmbeddedCanvas->AdoptCanvas(myCanvas);
   delete oldCanvas;
   //myCanvas = zoomerEmbeddedCanvas->GetCanvas();
   myVerticalFrame->AddFrame(zoomerEmbeddedCanvas, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY,20,20,20,20));

   // horizontal frame
   slideHorizFrame = new TGHorizontalFrame(myVerticalFrame,688,56,kHorizontalFrame);
   slideScrollBar = new TGHScrollBar(slideHorizFrame,564,16,kHorizontalFrame | kOwnBackground);
   slideScrollBar->SetRange(220,20);
   slideScrollBar->SetPosition(100);
   slideHorizFrame->AddFrame(slideScrollBar, new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsCenterY | kLHintsExpandX,20,20,10,10));
   slideLabel = new TGLabel(slideHorizFrame,"Slide");
   slideLabel->SetTextJustify(36);
   slideHorizFrame->AddFrame(slideLabel, new TGLayoutHints(kLHintsTop | kLHintsCenterY,20,5,2,2));

   myVerticalFrame->AddFrame(slideHorizFrame, new TGLayoutHints(kLHintsLeft | kLHintsBottom | kLHintsExpandX,2,2,2,22));

   // horizontal frame
   zoomHorizFrame = new TGHorizontalFrame(myVerticalFrame,688,56,kHorizontalFrame);
   zoomLabel = new TGLabel(zoomHorizFrame,"Zoom");
   zoomLabel->SetTextJustify(36);
   zoomHorizFrame->AddFrame(zoomLabel, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY,20,2,2,2));
   zoomScrollBar = new TGHScrollBar(zoomHorizFrame,564,16,kHorizontalFrame | kOwnBackground);
   zoomScrollBar->SetRange(220,20);
   zoomScrollBar->SetPosition(200);
   zoomHorizFrame->AddFrame(zoomScrollBar, new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsCenterY | kLHintsExpandX,20,20,10,10));

   myVerticalFrame->AddFrame(zoomHorizFrame, new TGLayoutHints(kLHintsLeft | kLHintsBottom | kLHintsExpandX,2,2,2,2));

   zoomerMainFrame->AddFrame(myVerticalFrame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

   zoomerMainFrame->SetWindowName("NGMZoomer");
   zoomerMainFrame->SetIconName("NGMZoomer");
   //   zoomerMainFrame->SetIconPixmap("bld_rgb.xpm");
   zoomerMainFrame->SetClassHints("NGMZoomGui","NGMZoomer");
   //zoomerMainFrame->SetMWMHints(kMWMDecorAll,
   //                     kMWMFuncAll,
   //                     kMWMInputFullApplicationModal);
   zoomerMainFrame->SetWMSize(692,483);
   zoomerMainFrame->SetWMSizeHints(692,483,10000,10000,0,0);
   zoomerMainFrame->MapSubwindows();

   zoomerMainFrame->Resize(zoomerMainFrame->GetDefaultSize());
   zoomerMainFrame->MapWindow();
   zoomerMainFrame->Resize(692,483);

   zoomScrollBar->Connect("PositionChanged(Int_t)","NGMZoomGui",this,"doSlider1(Int_t)");
   slideScrollBar->Connect("PositionChanged(Int_t)","NGMZoomGui",this,"doSlider2(Int_t)");
   zoomerMainFrame->Connect("CloseWindow()","NGMZoomGui",this,"CloseWindow()");
   zoomerMainFrame->MapRaised();
} 

NGMZoomGui::~NGMZoomGui(){
	//SetCleanup(kDeepCleanup) seems to make these unnecessary 
  //myVerticalFrame->Cleanup();  
  //zoomerMainFrame->Cleanup();
  delete zoomerMainFrame;
}


TGScrollBar* 
NGMZoomGui::AddScrollBar(const char* label){
  TGHorizontalFrame* tempHorizontalFrame = new TGHorizontalFrame(myVerticalFrame,688,56,kHorizontalFrame);
  TGLabel* tempLabel = new TGLabel(tempHorizontalFrame,label);
  tempLabel->SetTextJustify(36);
  tempHorizontalFrame->AddFrame(tempLabel, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY,20,8,2,2));
  TGHScrollBar* tempHScrollBar = new TGHScrollBar(tempHorizontalFrame,564,16,kHorizontalFrame | kOwnBackground);
  tempHScrollBar->SetRange(220,20);
  tempHScrollBar->SetPosition(200);
  tempHorizontalFrame->AddFrame(tempHScrollBar, new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsCenterY | kLHintsExpandX,20,20,10,10));
  
  myVerticalFrame->AddFrame(tempHorizontalFrame, new TGLayoutHints(kLHintsLeft | kLHintsBottom | kLHintsExpandX,2,2,2,2));
  zoomerMainFrame->MapSubwindows();
  
  zoomerMainFrame->Resize(zoomerMainFrame->GetDefaultSize());
  zoomerMainFrame->MapWindow();
  return tempHScrollBar;
}
