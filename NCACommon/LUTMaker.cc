
#include "LUTmaker.h"
#include "TVirtualPad.h"
#include "TSystem.h"
#include "TString.h"

using namespace std;

ClassImp(nca::LUTmaker)

namespace nca {

Double_t fgaus2(Double_t *x, Double_t *par);
    

LUTmaker::LUTmaker(TH2I *hF, const Int_t numPixels,const Int_t tpixelsPerRow)
: nPixels(numPixels)
{
	
  hFlood = hF;
  hFloodModified = 0;
  TString hname(hF->GetName());
  hname+="_LUTMaker";
  hLUT = (TH2I*)hFlood->Clone(hname);
  hLUT->SetDirectory(0);
  hLUT->Reset();
  if(!gPad)
    c1 = new TCanvas("c1", "c1",519,497,718,640);
  gPad->SetFillColor(0);
  gPad->SetFrameFillColor(0);
  gPad->GetCanvas()->ToggleEventStatus();

  gStyle->SetPalette(1);
  gStyle->SetNumberContours(99);

  DrawFlood();
  
  spacing = 22.0;  
	bRebin = false; 
	nrebin = 2; 
	bSmooth = true; 
    
  PrepareFlood();

  xSeed = new Double_t[nPixels];
  ySeed = new Double_t[nPixels];
  xFit = new Double_t[nPixels];
  yFit = new Double_t[nPixels];
  xPix = new Double_t[nPixels];
  yPix = new Double_t[nPixels];
  pixelMarker = new TMarker*[nPixels];
  pixelMarkerSeed = new TMarker*[nPixels];
  for(int ipix = 0; ipix< nPixels; ipix++)
  {
    pixelMarker[ipix] = 0;
    pixelMarkerSeed[ipix]=0;
  }
  
  fitRange = 0.016;
	nomWidth = 0.02;
	widthMin = 0.004;
	widthMax = 0.03;
	bHighlightGaus = true;
	bHighlightFitRegion = false;

  if(tpixelsPerRow>0){
    pixelsPerRow = tpixelsPerRow;
  }else{
    pixelsPerRow = int(sqrt(nPixels));
    printf("Using pixels %d pixels per row\n",pixelsPerRow);
  }

  hname = hF->GetName();
  hname+="_HitPattern";
  hHitPattern = new TH2I(hname,hname,pixelsPerRow,0,pixelsPerRow,
			 pixelsPerRow,0,pixelsPerRow);
  
  bSeedsDefined = false;
  bPixSorted = false;
  bSearchPerformed = false;
  bFitPointsDefined = false;
  bFitsPerformed = false;
  bLUTstatus = false;

  Help();
    
}

LUTmaker::~LUTmaker(){
   
//    printf("\t\tDestructing the LUTmaker class...\n");
    
    //Most likely hFlood is owned by someone else...
    //so don't // delete hFlood;
  
    delete hFloodModified;
    delete hLUT;

    delete [] xSeed;
    delete [] ySeed;
    delete [] xFit;
    delete [] yFit;
    delete [] xPix;
    delete [] yPix;
    
	for(Int_t ipix=0; ipix<nPixels; ipix++){
        delete pixelMarkerSeed[ipix];
        delete pixelMarker[ipix];
    }
    delete [] pixelMarkerSeed;
    delete [] pixelMarker;

//    printf("\t\t...done\n");

}

void LUTmaker::PrepareFlood(void){

    printf("\n\t\tPreparing the histogram...\n");
	if(!bRebin)
		hFloodModified = (TH2I*)hFlood->Clone(TString(hFlood->GetName())+"_Modified");
	else{
		TH2I *hs0 = (TH2I*)hFlood->Clone(TString(hFlood->GetName())+"hs0");
		TH2I *hs1 = (TH2I*)hs0->RebinX(nrebin,"hs1");
		hFloodModified = (TH2I*)hs1->RebinY(nrebin,"hFloodModified");
		printf("\t\t\t(Histogram rebinned by %d in both dimensions)\n",nrebin);
	}
	
	if(bSmooth){
		hFloodModified->Smooth(1,"k5b");
		printf("\t\t\t(Histogram smoothed)\n");
	}
    hFloodModified->SetDirectory(0);
    DrawModifiedFlood();
    
	printf("\t\t...done\n");

    
}

void LUTmaker::PixelSearch(Bool_t bGather){
 
    TH2I *hSearch = (TH2I*)hFloodModified->Clone(TString(hFlood->GetName())+"_Search");
    DrawModifiedFlood();

	//Initialize variables for finding pixel seed locations
    ZeroSeeds();
	
	printf("\t\tFinding peak locations...\n");
    
	for(Int_t ipix=0; ipix<nPixels; ipix++){
		
		Int_t maxBin = hSearch->GetMaximumBin();
		Int_t binX, binY, binZ; 
		hSearch->GetBinXYZ(maxBin,binX,binY,binZ);
		
		xFit[ipix] = hSearch->GetXaxis()->GetBinCenter(binX);
		yFit[ipix] = hSearch->GetYaxis()->GetBinCenter(binY);
		
		//once a peak has been found, zero out all the pixels around it in a radius defined by 'spacing'
		for(Int_t ix=binX-spacing-1; ix<=binX+spacing+1; ix++){
			if(ix<1 || ix>hSearch->GetNbinsX()) continue;
			for(Int_t iy=binY-spacing-1; iy<=binY+spacing+1; iy++){
				if(iy<1 || iy>hSearch->GetNbinsY()) continue;
                
				Double_t xBinCenter = hSearch->GetXaxis()->GetBinCenter(ix);
				Double_t yBinCenter = hSearch->GetYaxis()->GetBinCenter(iy);
				Double_t diff = TMath::Sqrt( TMath::Power(xBinCenter-xFit[ipix],2.0) + TMath::Power(yBinCenter-yFit[ipix],2.0) );
				if(diff < spacing) hSearch->SetBinContent(ix,iy,0.0);
                
			} // loop over y bins
		} // loop over x bins
        
		pixelMarkerSeed[ipix] = new TMarker(xFit[ipix],yFit[ipix],8);
		pixelMarkerSeed[ipix]->Draw();
        
		gPad->Update();
		
	} // loop over number of pixels
	    
    bSearchPerformed = true;

    if(bGather) GatherSeedValues();

    printf("\t\t...done\n");

}

void LUTmaker::GatherSeedValues(Bool_t bDump){
    
    if(!bSearchPerformed){
        printf("\t\tGatherSeedValues: Pixel search has not yet been performed.  Run PixelSearch() first.\n");
        return;
    }

    for(Int_t ipix=0; ipix<nPixels; ipix++){
        xSeed[ipix] = pixelMarkerSeed[ipix]->GetX();
        ySeed[ipix] = pixelMarkerSeed[ipix]->GetY();
        if(bDump) printf("\tPixel %3d:  %5.3f   %5.3f\n",ipix,xSeed[ipix],ySeed[ipix]);
    }
    bSeedsDefined = true;
    
}

void LUTmaker::GatherFitValues(Bool_t bDump){
    
    if(!bFitsPerformed){
        printf("\t\tGatherFitValues: Fits have not yet been performed.  Run FitPixels() first.\n");
        return;
    }

    for(Int_t ipix=0; ipix<nPixels; ipix++){
        xFit[ipix] = pixelMarker[ipix]->GetX();
        yFit[ipix] = pixelMarker[ipix]->GetY();
        if(bDump) printf("\tPixel %3d:  %5.3f   %5.3f\n",ipix,xFit[ipix],yFit[ipix]);
    }
    bFitPointsDefined = true;
    
}

void LUTmaker::FitPixels(Bool_t bGather, Bool_t bPause){
   
    if(!bSeedsDefined){
        printf("\t\tFitPixels: Seed values for fits are not defined yet.  Run PixelSearch() first.\n");
        return;
    }

    printf("\t\tGetting the final pixel locations...\n");

    ZeroFitPix();
    
    TH2I *hPlot = (TH2I*)hFloodModified->Clone(TString(hFlood->GetName())+"_Plot");
    TH2I *hFit = (TH2I*)hFloodModified->Clone(TString(hFlood->GetName())+"_Fit");
	    
	for(Int_t ipix=0; ipix<nPixels; ipix++){
        
		TF2 *fitFcn = new TF2("fitFcn",fgaus2,xSeed[ipix]-fitRange,xSeed[ipix]+fitRange,ySeed[ipix]-fitRange,ySeed[ipix]+fitRange,5);
		fitFcn->SetParameter(0,hFit->GetMaximum()/2.0);
		fitFcn->SetParameter(1,xSeed[ipix]);
		fitFcn->SetParLimits(1,xSeed[ipix]-fitRange,xSeed[ipix]+fitRange);
		fitFcn->SetParameter(2,nomWidth);
		fitFcn->SetParLimits(2,widthMin,widthMax);
		fitFcn->SetParameter(3,ySeed[ipix]);
		fitFcn->SetParLimits(3,ySeed[ipix]-fitRange,ySeed[ipix]+fitRange);
		fitFcn->SetParameter(4,nomWidth);
		fitFcn->SetParLimits(4,widthMin,widthMax);
		hFit->Draw("colz");
		hFit->Fit("fitFcn","qr");
        
		Double_t par[5];
		fitFcn->GetParameters(&par[0]);
        
		xFit[ipix] = par[1];
		yFit[ipix] = par[3];
		
		Double_t viewLoX = xSeed[ipix]-5*fitRange;
		if(viewLoX < hFit->GetXaxis()->GetXmin()) viewLoX = hFit->GetXaxis()->GetXmin();
		Double_t viewLoY = ySeed[ipix]-5*fitRange;
		if(viewLoY < hFit->GetYaxis()->GetXmin()) viewLoY = hFit->GetYaxis()->GetXmin();
		
		Double_t viewHiX = xSeed[ipix]+5*fitRange;
		if(viewHiX > hFit->GetXaxis()->GetXmax()) viewHiX = hFit->GetXaxis()->GetXmax();
		Double_t viewHiY = ySeed[ipix]+5*fitRange;
		if(viewHiY > hFit->GetYaxis()->GetXmax())	viewHiY = hFit->GetYaxis()->GetXmax();
        
		hPlot->SetAxisRange(viewLoX,viewHiX,"x");
		hPlot->SetAxisRange(viewLoY,viewHiY,"y");
        
		hPlot->Draw("col");
		pixelMarker[ipix] = new TMarker(par[1],par[3],8);
		pixelMarker[ipix]->Draw();
		
		if(bHighlightFitRegion){
			TBox *box = new TBox(xSeed[ipix]-fitRange,ySeed[ipix]-fitRange,xSeed[ipix]+fitRange,ySeed[ipix]+fitRange);
			box->SetFillStyle(0);
			box->SetLineStyle(2);
			box->Draw();
		}
		if(bHighlightGaus){
			TEllipse *pixel = new TEllipse(xFit[ipix],yFit[ipix],par[2],par[4]);
			pixel->SetFillStyle(0);
			pixel->Draw();
		}
		
		gPad->Update();
        if(bPause) cin.ignore();
        
        delete fitFcn;
       
	}
	//end of fits

    DrawModifiedFlood();
    for(Int_t ipix=0; ipix<nPixels; ipix++) pixelMarker[ipix]->Draw();
        
    hPlot = 0;
    hFit = 0;
    bFitsPerformed = true;

    if(bGather) GatherFitValues();

    printf("\t\t...done\n");

}

void LUTmaker::SortPixels(void){
   
    if(!bFitPointsDefined){
        printf("\t\tSortPixels: Final pixel center values are not defined yet.  Run FitPixels() first.\n");
        return;
    }

    ZeroFinalPix();
	
	Int_t isortedIndex[nPixels];
	TMath::Sort(nPixels,yFit,isortedIndex,false);
	
	Double_t *xRow = new Double_t[pixelsPerRow];
	Double_t *yRow = new Double_t[pixelsPerRow];
    Int_t numRows = TMath::CeilNint( double(nPixels)/double(pixelsPerRow) );
	for(Int_t irow=0; irow<numRows; irow++){
		for(Int_t ix=0; ix<pixelsPerRow; ix++){
			xRow[ix] = 0.0;
			yRow[ix] = 0.0;
			if(ix+irow*pixelsPerRow >= nPixels)	continue;
			xRow[ix] = xFit[isortedIndex[ix+irow*pixelsPerRow]];
			yRow[ix] = yFit[isortedIndex[ix+irow*pixelsPerRow]];
		}
		Int_t *isortedRow = new Int_t[pixelsPerRow];
        Int_t numToSort = pixelsPerRow;
        if(irow==numRows-1 && nPixels%pixelsPerRow != 0) numToSort = nPixels%pixelsPerRow;
		TMath::Sort(numToSort,xRow,isortedRow,false);
		for(Int_t ix=0; ix<numToSort; ix++){
			if(ix+irow*pixelsPerRow >= nPixels) continue;
			xPix[ix+irow*pixelsPerRow] = xRow[isortedRow[ix]];
			yPix[ix+irow*pixelsPerRow] = yRow[isortedRow[ix]];
		}
        delete [] isortedRow;
	}
    
    hFloodModified->SetMinimum(-0.1);
    DrawModifiedFlood();
	
    bPixSorted = true;

    DrawFinalMarkers();
    DrawPixelNumbers();
    
    delete [] xRow;
    delete [] yRow;
    
}

void LUTmaker::CreateHitPattern(void){

    if(!bLUTstatus){
        printf("\t\tOverlayLUT: LUT has not been created yet.  Run CreateLUT() first.\n");
        return;
    }

    for(Int_t ix=1; ix<=hFlood->GetNbinsX(); ix++){
      for(Int_t iy=1; iy<=hFlood->GetNbinsY(); iy++){
	Int_t pixelNum = hLUT->GetBinContent(ix,iy);
	Int_t floodContent = hFlood->GetBinContent(ix,iy);

	Int_t pixelX = pixelNum%pixelsPerRow + 1;
	Int_t pixelY = pixelNum/pixelsPerRow + 1;

	hHitPattern->SetBinContent(pixelX,pixelY,hHitPattern->GetBinContent(pixelX,pixelY)+floodContent);
      }
    }

}

void LUTmaker::CreateLUT(void){
   
    if(!bPixSorted && bFitPointsDefined) SortPixels();
    else if(!bFitPointsDefined){
        printf("\t\tCreateLUT: Final pixel center values are not defined yet.  Run FitPixels() first.\n");
        return;
    }

	hLUT->Reset();
	for(Int_t ix=1; ix<=hLUT->GetNbinsX(); ix++){
		for(Int_t iy=1; iy<=hLUT->GetNbinsY(); iy++){
            
			Double_t xVal = hLUT->GetXaxis()->GetBinCenter(ix);
			Double_t yVal = hLUT->GetYaxis()->GetBinCenter(iy);
            
			Double_t minDist=100000;
			for(Int_t ipix=0; ipix<nPixels; ipix++){
				if( TMath::Abs(xVal-xPix[ipix]) < 0.2 && TMath::Abs(yVal-yPix[ipix]) < 0.2 ){
                    
					Double_t pixDist = pow( pow(xPix[ipix]-xVal,2.0) 
                                           + pow(yPix[ipix]-yVal,2.0), 0.5);
					if(pixDist < minDist){
						minDist = pixDist;
						hLUT->SetBinContent(ix,iy,ipix);
					}
				}
			}
		}
	}

    bLUTstatus = true;

    DrawModifiedFlood(true,"col");
            
}

void LUTmaker::OverlayLUT(void){
 
    if(!bLUTstatus){
        printf("\t\tOverlayLUT: LUT has not been created yet.  Run CreateLUT() first.\n");
        return;
    }

    TLine *line0;
	Double_t prevBinC = -1;
	Double_t binWx = hLUT->GetXaxis()->GetBinWidth(1);
	Double_t binWy = hLUT->GetYaxis()->GetBinWidth(1);
	for(Int_t j=1; j<=hLUT->GetNbinsX(); j++){
		for(Int_t k=1; k<=hLUT->GetNbinsY(); k++){
			Double_t binC = hLUT->GetBinContent(j,k);
			if(binC != prevBinC){
				Double_t xL = hLUT->GetXaxis()->GetBinLowEdge(j);
				Double_t yL = hLUT->GetYaxis()->GetBinLowEdge(k);
				line0 = new TLine(xL,yL,xL+binWx,yL);
				line0->Draw();
			}
			prevBinC = binC;
		}
	}
	prevBinC = -1;
	for(Int_t j=1; j<=hLUT->GetNbinsY(); j++){
		for(Int_t k=1; k<=hLUT->GetNbinsX(); k++){
			Double_t binC = hLUT->GetBinContent(k,j);
			if(binC != prevBinC){
				Double_t xL = hLUT->GetXaxis()->GetBinLowEdge(k);
				Double_t yL = hLUT->GetYaxis()->GetBinLowEdge(j);
				line0 = new TLine(xL,yL,xL,yL+binWy);
				line0->Draw();
			}
			prevBinC = binC;
		}
	}

    
}

void LUTmaker::DrawFinalMarkers(){
    	
    if(!bPixSorted){
        printf("\t\tDrawFinalMarkers: Final pixel values have not been determined yet.  Run SortPixels() first.\n");
        return;
    }

	for(Int_t ipix=0; ipix<nPixels; ipix++){
		TMarker *mark = new TMarker(xPix[ipix],yPix[ipix],8);
		mark->Draw();
	}

}

void LUTmaker::DrawPixelNumbers(){

    if(!bPixSorted){
        printf("\t\tDrawPixelNumbers: Final pixel values have not been determined yet.  Run SortPixels() first.\n");
        return;
    }

    Char_t cPixNum[5];
	TLatex tex;
	tex.SetTextSize(0.025);
	tex.SetLineWidth(2);
	for(Int_t ipix=0; ipix<nPixels; ipix++){
		sprintf(cPixNum,"%d",ipix);		
		tex.DrawLatex(xPix[ipix],yPix[ipix]+5.0*hFlood->GetYaxis()->GetBinWidth(1),cPixNum);
	}

}

void LUTmaker::SetSearchSpacing(Double_t dValue){
    printf("\t\tSpacing required between pixels for pixel search has been updated from %.1f bins to ",spacing);
    spacing = dValue;
    printf("%.1f bins.  This will take effect when pixel search is performed.\n",spacing);
}

void LUTmaker::SetRebin(Bool_t bValue){
    bRebin = bValue;
    if(bRebin) printf("\t\tRebinning turned on with rebing grouping value of %d.\n",nrebin);
    else printf("\t\tRebinning turned off.\n");
    PrepareFlood();
}

void LUTmaker::SetRebinGrouping(Int_t iValue){
    printf("\t\tRebin grouping value has been updated from %d to ",nrebin);
    nrebin = iValue;
    printf("%d. ",nrebin);
    if(!bRebin) printf("However, rebinning has not been selected.\n");
    else{
        printf("\n");
        PrepareFlood();
    }
}

void LUTmaker::SetSmooth(Bool_t bValue){
    bSmooth = bValue;
    if(bSmooth) printf("\t\tSmoothing turned on.\n");
    else printf("\t\tSmoothing turned off.\n");
    PrepareFlood();
}

void LUTmaker::SetFitRange(Double_t dValue){
    printf("\t\tFit region value has been updated from %.4f to ",fitRange);
    fitRange = dValue;   
    printf("%.4f.\n",fitRange);
}

void LUTmaker::SetNominalWidth(Double_t dValue){
    printf("\t\tNominal pixel width has been updated from %.4f to ",nomWidth);
    nomWidth = dValue;
    printf("%.4f.\n",nomWidth);
}

void LUTmaker::SetWidthMaximum(Double_t dValue){
    printf("\t\tMaximum pixel width has been updated from %.4f to ",widthMax);
    widthMax = dValue;
    printf("%.4f.\n",widthMax);
}

void LUTmaker::SetWidthMinimum(Double_t dValue){
    printf("\t\tMinimum pixel width has been updated from %.4f to ",widthMin);
    widthMin = dValue;
    printf("%.4f.\n",widthMin);
}

void LUTmaker::SetWidthRange(Double_t dValueLow, Double_t dValueHigh){
    SetWidthMinimum(dValueLow);
    SetWidthMaximum(dValueHigh);
}

void LUTmaker::SetHighlightGaus(Bool_t bValue){
    bHighlightGaus = bValue;
    if(bHighlightGaus) printf("\t\tFit result highlighting turned on.\n");
    else printf("\t\tFit result highlighting turned off.\n");
}

void LUTmaker::SetHighlightFitRegion(Bool_t bValue){
    bHighlightFitRegion = bValue;
    if(bHighlightFitRegion) printf("\t\tFit region highlighting turned on.\n");
    else printf("\t\tFit region highlighting turned off.\n");
}

void LUTmaker::DrawFlood(Bool_t bLabel, TString opt){
    
    gPad->Modified();
    gPad->Update();
    gSystem->ProcessEvents();
    if(opt=="colz") gPad->cd()->SetRightMargin(0.15);
    else gPad->cd()->SetRightMargin(0.1);
    hFlood->Draw(opt);
    
    if(bLabel){
        OverlayLUT();
        DrawFinalMarkers();
        DrawPixelNumbers();
    }
    
    gPad->Update();
    
}

void LUTmaker::DrawModifiedFlood(Bool_t bLabel, TString opt){
    
    if(opt=="colz") gPad->cd()->SetRightMargin(0.15);
    else gPad->cd()->SetRightMargin(0.1);
    hFloodModified->Draw(opt);
    gPad->Update();
    
    if(bLabel){
        OverlayLUT();
        DrawFinalMarkers();
        DrawPixelNumbers();
    }
    
}

void LUTmaker::DrawLUThistogram(Bool_t bLabel, TString opt){
    
    if(opt=="colz") gPad->cd()->SetRightMargin(0.15);
    else gPad->cd()->SetRightMargin(0.1);
    hLUT->Draw(opt);
    gPad->Update();
    
    if(bLabel){
        OverlayLUT();
        DrawFinalMarkers();
        DrawPixelNumbers();
    }

    
}

void LUTmaker::ZeroSeeds(void){
 
    bSeedsDefined = false;
    bPixSorted = false;
    bSearchPerformed = false;
    bFitPointsDefined = false;
    bFitsPerformed = false;
    bLUTstatus = false;
    
    for(Int_t i=0; i<nPixels; i++){
        xSeed[i] = 0.0;
        ySeed[i] = 0.0;
    }
    
}

void LUTmaker::ZeroFitPix(void){

    bPixSorted = false;
    bFitPointsDefined = false;
    bFitsPerformed = false;
    bLUTstatus = false;
    
    for(Int_t i=0; i<nPixels; i++){
        xFit[i] = 0.0;
        yFit[i] = 0.0;
    }
    
}

void LUTmaker::ZeroFinalPix(void){
    
    for(Int_t i=0; i<nPixels; i++){
        xPix[i] = 0.0;
        yPix[i] = 0.0;
    }
    
}

void LUTmaker::Help(void){
 
    printf("\nNormal procedures:\n");
    printf("\t1. Search for pixel center seed values using PixelSearch().\n");
    printf("\t2. Modify the seed values on the canvas if desired.\n");
    printf("\t3. Register the new seed values using GatherSeedValues().  PixelSearch does this\n\t\t by default (unless specified not to) but any modifications using the GUI\n\t\t require the values to be re-registered.\n");
    printf("\t4. Fit the pixels using FitPixels().\n");
    printf("\t5. Modify the fit values on the canvas if desired.\n");
    printf("\t6. Register the fit values using GatherFitValues().  FitPixels does this\n\t\t by default (unless specified not to) but any modifications using the GUI\n\t\t requires the value to be re-registered.\n");
    printf("\t7. Create the LUT using CreateLUT().\n\n");

}

Double_t fgaus2(Double_t *x, Double_t *par){
    
    return par[0]*TMath::Gaus(x[0],par[1],par[2])*TMath::Gaus(x[1],par[3],par[4]);
	
}

}

