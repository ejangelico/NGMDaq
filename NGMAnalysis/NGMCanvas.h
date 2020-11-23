#ifndef NGMCANVAS
#define NGMCANVAS

#include <iostream>
#include "TBox.h"
#include "TCanvas.h"

class NGMCanvas : public TCanvas {
public:

	NGMCanvas(Bool_t build = kTRUE):TCanvas(build){
		fSelector=0;
		_box=0;
	}
	NGMCanvas(const char* name, const char* title = "", Int_t form = 1):TCanvas(name,title,form){
		fSelector=0;
		_box=0;
	}
	NGMCanvas(const char* name, const char* title, Int_t ww, Int_t wh):TCanvas(name,title,ww,wh){
		fSelector=0;	
		_box=0;
	}
	NGMCanvas(const char* name, const char* title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh):TCanvas(name,title,wtopx,wtopy,ww,wh){
		fSelector=0;
		_box=0;
	}
	NGMCanvas(const char* name, Int_t ww, Int_t wh, Int_t winid):TCanvas(name,ww,wh,winid){
		fSelector=0;
		_box=0;
	}
	
	virtual ~NGMCanvas(){}
	virtual void	HandleInput(EEventType button, Int_t x, Int_t y);
    virtual void SetSelector(Int_t value = 1); //*TOGGLE*
	virtual Int_t GetSelector() const {return fSelector;}
	Int_t fSelector;
	
private:
	int pxorig;
	int pyorig;
	TBox* _box;
	ClassDef(NGMCanvas,1)
		
};
#endif
