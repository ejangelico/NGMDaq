#include <iostream>
#include "NGMCanvas.h"

#include "TList.h"
#include "NGMMarker.h"
#include "TMath.h"
#include "NGMDisplay.h"

ClassImp(NGMCanvas)

void
NGMCanvas::HandleInput(EEventType button, Int_t x, Int_t y){
	TVirtualPad* pad = gPad;
	if(pad != this && !FindObject(pad)){ TCanvas::HandleInput(button,x,y); return;}
	//std::cout<<x<<" "<<y<<" pad "<<GetName()<<std::endl;
	if(button == kButton1Double) SetSelector(fSelector==0?1:0);
	if(fSelector){
		if(button == kButton1Down){
			if(_box){ delete _box; _box=0;}
			pxorig = x;
			pyorig = y;
			return;
		}
		if(button == kButton1Up && _box){
			int Nselected=0;
			for(int iobj=0; iobj<pad->GetListOfPrimitives()->GetSize();iobj++){
				if(pad->GetListOfPrimitives()->At(iobj)->InheritsFrom("NGMMarker")){
					NGMMarker* mark = (NGMMarker*)pad->GetListOfPrimitives()->At(iobj);
					if(mark->GetX() >= TMath::Min(_box->GetX1(),_box->GetX2()) && 
					   mark->GetX() <= TMath::Max(_box->GetX1(),_box->GetX2()) &&
					   mark->GetY() >= TMath::Min(_box->GetY1(),_box->GetY2()) && 
					   mark->GetY() <= TMath::Max(_box->GetY1(),_box->GetY2())){
						Nselected++;
						if(Nselected == 1){
							NGMDisplay::Instance()->Reset();
						}
						NGMDisplay::Instance()->AddHit(mark->GetLabel(),mark->GetMarkerColor());
						//std::cout<<"Selected marker at "<<mark->GetX()<<" "<<mark->GetY()<<std::endl;
					}
				}
			}
			if(Nselected>0)NGMDisplay::Instance()->Update();
			std::cout<<"Selected "<<Nselected<<" NGM Markers"<<std::endl;
			pad->cd();
			SetSelector(0);
			return;
		}
		if (button == kButton1Motion) {
			if(!_box){
				_box = new TBox;
				_box->SetLineColor(kBlack);
				_box->SetLineWidth(1);
				_box->SetFillStyle(0);
				_box->SetLineStyle(2);
			}
			_box->SetY1(pad->PadtoY(pad->AbsPixeltoY(pyorig)));
			_box->SetY2(pad->PadtoY(pad->AbsPixeltoY(y)));
			_box->SetX1(pad->PadtoX(pad->AbsPixeltoX(pxorig)));
			_box->SetX2(pad->PadtoX(pad->AbsPixeltoX(x)));
			
			if(!pad->FindObject(_box))_box->Draw();
			Modified();
			Update();
			Draw();
			return;
		}
	}
	TCanvas::HandleInput(button,x,y);
}

void 
NGMCanvas::SetSelector(Int_t value){
	fSelector=value; 
	delete _box; 
	_box=0;
	SetCrosshair(value);
}
