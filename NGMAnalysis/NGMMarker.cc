#include "NGMMarker.h"
#include "TList.h" //stupid kluge

ClassImp(NGMMarker)

void NGMMarker::Paint(Option_t *)
{
  // Paint this marker with its current attributes.

  if (TestBit(kMarkerNDC)) {
    Double_t u = gPad->GetX1() + fX*(gPad->GetX2()-gPad->GetX1());
    Double_t v = gPad->GetY1() + fY*(gPad->GetY2()-gPad->GetY1());
    PaintMarker(u,v);
  } else {
    if((fX < gPad->PadtoX(gPad->GetUxmin())) ||
       (fX > gPad->PadtoX(gPad->GetUxmax())) ||
       (fY < gPad->PadtoY(gPad->GetUymin())) ||
       (fY > gPad->PadtoY(gPad->GetUymax()))) return;
    PaintMarker(gPad->XtoPad(fX),gPad->YtoPad(fY));
  }
}

void NGMMarker::ExecuteEvent(Int_t event, Int_t px, Int_t py)
{
  if (_label != ""	){
  	if(event==kMouseEnter){
	_text->SetNDC();
	_text->SetX(0.75);
	_text->SetY(0.92);
	_text->SetTitle(_label.Data());
	//_text = new TText(fX*1.1,fY*1.1,_label.Data());
	_text->Draw();
	//_text->PaintText(fX,fY,_label.Data());
	gPad->Modified();
	gPad->Update();
	gPad->Draw();
	} else if(event == kMouseLeave){
	gPad->RecursiveRemove(_text);
	gPad->Modified();
	gPad->Update();
	gPad->Draw();
	//delete _text;
	//_text = 0;
	}
  }

  if(!_active) return;

  if(event==kMouseEnter){
    _oldSize=GetMarkerSize();
    SetMarkerSize(2.0*_oldSize);
    _old_color=GetMarkerColor();
    SetMarkerColor(kRed);
    Draw();
    gPad->Update();
    gPad->Draw();
    if(_pad && _obj){ //Stupid kludge
      if(_pad->GetPad(255) != _obj){
	_pad->GetListOfPrimitives()->Clear("nodelete");
	_pad->GetPad(0)->Clear();
        _pad->cd();
	_obj->Draw();
      }
      _pad->cd();
      _pad->ResizePad();
      _pad->Update();
    }

  } else if(event==kMouseLeave){
    SetMarkerSize(_oldSize);
    SetMarkerColor(_old_color);
    Draw();
    gPad->Update();
    gPad->Draw();
  }else if(event == kButton1Up){
    //      std::cout<<"Drawing Associated"<<std::endl;
  }
}
