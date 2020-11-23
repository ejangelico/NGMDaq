#ifndef NGMMARKER
#define NGMMARKER
#include "TMarker.h"
#include "TPad.h"
#include "TText.h" 
#include <iostream>

class NGMMarker : public TMarker {
 public:
  NGMMarker():TMarker(){
    _pad=0;
    _obj=0;
    _active=kFALSE;
    _old_color = 0;
    _oldSize = 0;
	_label = "";
	_text = new TText;
 }

  ~NGMMarker(){}

  void Paint(Option_t *);
  void ExecuteEvent(Int_t event, Int_t px, Int_t py);
  void SetAssociated(TPad* pad, TObject* obj){
    _pad=pad;
    _obj=obj;
  }
  
  void SetActive(Bool_t active = kTRUE){
    _active=active;
  }
  
  void SetLabel(const char* label){
	_label = label;
  }

const char* GetLabel(){return _label.Data();}

 private:
  TPad* _pad;
  TObject* _obj;
  Bool_t _active;
  int _old_color;
  Size_t _oldSize;
  TText* _text;
  TString _label;

  ClassDef(NGMMarker,1)
};

#endif
