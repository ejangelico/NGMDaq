#ifndef __NGMZOOMGUI__
#define __NGMZOOMGUI__

#include "TGLabel.h"
#include "TGScrollBar.h"
#include "TGCanvas.h"
#include "TRootEmbeddedCanvas.h"
#include "TCanvas.h"
#include "TList.h"

class TAxis;

class NGMZoomGui{
 public:
  NGMZoomGui();
  //NGMZoomGui(const char* name = "myCanvas");
  ~NGMZoomGui();
  void doSlider1(Int_t pos);
  void doSlider2(Int_t pos);
  void CloseWindow();
  TCanvas* getCanvas(){return myCanvas;}
  TGScrollBar* AddScrollBar(const char* label="Label");
  void setAxis(TAxis* axis);
  void AddAxis(TAxis* axis);


 private:
  TList _axList;
  TCanvas *myCanvas;
  TGMainFrame *zoomerMainFrame;
  TGVerticalFrame *myVerticalFrame;
  TRootEmbeddedCanvas *zoomerEmbeddedCanvas;
  TGHorizontalFrame *slideHorizFrame;
  TGHScrollBar *slideScrollBar;
  TGLabel *slideLabel;
  TGHorizontalFrame *zoomHorizFrame;
  TGLabel *zoomLabel;
  TGHScrollBar *zoomScrollBar;
};

#endif
