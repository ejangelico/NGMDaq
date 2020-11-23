
#include "ImageMaker.h"
#include "TStopwatch.h"
using namespace std;

Double_t fgaus(Double_t *x, Double_t *par);

ClassImp(ImagerRunDescription)

ImagerRunDescription::ImagerRunDescription(){
    
}

ImagerRunDescription::~ImagerRunDescription(){
}

ClassImp(ImageMaker)

ImageMaker::ImageMaker(Int_t maskRank, Double_t maskDetectorDistance,
                       Double_t sourceMaskDistance, Double_t maskElementSize,
                       Int_t numberPixelsX, Int_t numberPixelsY,
					   Double_t detectorPixelSize)
{
        
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

	Initialize(maskRank);
    
	//mask information
    mRank = maskRank;
    maskElSize = maskElementSize;

	//detector information
	numPixX = numberPixelsX;
    numPixY = numberPixelsY;
	detPixelSize = detectorPixelSize;
			
	//setup information
    sourceMaskDist = sourceMaskDistance;
    maskDetDist = maskDetectorDistance;
	
    GetDecoder();
    
    TH1::AddDirectory(bPrevAddDirectory);

}

ImageMaker::ImageMaker(Int_t maskRank, Double_t maskElementSize,
		   Double_t maskDetectorDistance, Double_t sourceMaskDistance,
		   const Char_t *imagerName){
	
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);
	
	Initialize(maskRank);
    
	//mask information
    mRank = maskRank;
    maskElSize = maskElementSize;
	    
	//detector information
	if(!strcmp(imagerName,"L40")){
		numPixX = 40;
		numPixY = 40;
		detPixelSize = 4.25*2.54/10.0;
	}
	else if(!strcmp(imagerName,"P40")){
		numPixX = 40;
		numPixY = 40;
		detPixelSize = 11.2/10.0;
	}
	else if(!strcmp(imagerName,"P24")){
		numPixX = 24;
		numPixY = 24;
		detPixelSize = 11.2/8.0;
	}
	else{
		printf("\nError in ImageMaker::ImageMaker() - imager %s is not defined.\n\n",imagerName);
		exit(0);
	}
	
	//setup information
    sourceMaskDist = sourceMaskDistance;
    maskDetDist = maskDetectorDistance;
	
    GetDecoder();
    
    TH1::AddDirectory(bPrevAddDirectory);
	
}

ImageMaker::~ImageMaker(){
    delete hImage;
    delete hImage2;
    delete hImage3;
    delete hDecoder;
    delete hDecM;
    delete hDecA;
    delete hDecD;
    delete hExpAnti;
    delete hExpMask;
    delete hExpDiff;
    delete hMaskScaled;
    delete hAntiScaled;
    delete hDiffScaled;
    delete hMaskPattern;
    delete hRebinnedDet;
    delete hRebinnedDetAdd;
    delete hDetAbsAxis;
    delete hOffsetHist;
    delete hImage3D;
    delete hData3D;
}

Double_t fgaus(Double_t *x, Double_t *par){
    
    return par[0]*TMath::Gaus(x[0],par[1],par[2]);
    
}

void ImageMaker::Initialize(Int_t maskRank){
	
	Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

	hDHPm = 0;
    hDHPa = 0;
    hDHP = 0;
    hMaskPattern = 0;
    hDecoder = 0;
    hDecM = 0;
    hDecA = 0;
    hDecD = 0;
    hImage = 0;
    hImage2 = 0;
    hImage3 = 0;
    hImage3D = 0;
    hData3D = 0;
	
    hMaskScaled = 0;
    hAntiScaled = 0;
    hDiffScaled = 0;
    hExpAnti = 0;
    hExpMask = 0;
    hExpDiff = 0;
    hDetAbsAxis = 0;
    hRebinnedDet = 0;
    hRebinnedDetAdd = 0;
    hOffsetHist = 0;
    avgPixelWeights = 0;
    
    totalDetectorCounts = 0;
    totalDetectorCountsAdd = 0;

	hImage = 0;
    GetMaskPattern(maskRank);
	MakeCorrectionCurve();

    TH1::AddDirectory(bPrevAddDirectory);
	
}

void ImageMaker::SetDistances(Double_t newMaskDetDist, Double_t newMaskSrcDist){
    
    sourceMaskDist = newMaskSrcDist;
    maskDetDist = newMaskDetDist;
	
    GetDecoder();
    
}


void ImageMaker::GetDecoder(){
	
    detRangeX = detPixelSize*numPixX/2.0;
    detRangeY = detPixelSize*numPixY/2.0;
	
    Double_t mag = (sourceMaskDist+maskDetDist)/sourceMaskDist;
    maskRangeX = maskElSize*mRank*mag;
    maskRangeY = maskElSize*mRank*mag;
    
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);
    
    if(!hMaskScaled)
        hMaskScaled = new TH2D("hMaskScaled","Mask Scaled to Detector Plane",
                               hMaskPattern->GetNbinsX(),-maskRangeX,maskRangeX,
                               hMaskPattern->GetNbinsY(),-maskRangeY,maskRangeY);
    else
        hMaskScaled->SetBins(hMaskPattern->GetNbinsX(),-maskRangeX,maskRangeX,
                             hMaskPattern->GetNbinsY(),-maskRangeY,maskRangeY);
    
    if(!hAntiScaled)
        hAntiScaled = new TH2D("hAntiScaled","Antimask Scaled to Detector Plane",
                               hMaskPattern->GetNbinsX(),-maskRangeX,maskRangeX,
                               hMaskPattern->GetNbinsY(),-maskRangeY,maskRangeY);
    else
        hAntiScaled->SetBins(hMaskPattern->GetNbinsX(),-maskRangeX,maskRangeX,
                             hMaskPattern->GetNbinsY(),-maskRangeY,maskRangeY);
	
    if(!hDiffScaled)
        hDiffScaled = new TH2D("hDiffScaled","Mask-Anti Scaled to Detector Plane",
                               hMaskPattern->GetNbinsX(),-maskRangeX,maskRangeX,
                               hMaskPattern->GetNbinsY(),-maskRangeY,maskRangeY);
    else
        hDiffScaled->SetBins(hMaskPattern->GetNbinsX(),-maskRangeX,maskRangeX,
                             hMaskPattern->GetNbinsY(),-maskRangeY,maskRangeY);
    
    if(!hExpMask) hExpMask = new TH2D("hExpMask","Expected Mask",numPixX,0,numPixX,numPixY,0,numPixY);
    else hExpMask->Reset();
    if(!hExpAnti) hExpAnti = new TH2D("hExpAnti","Expected Anti",numPixX,0,numPixX,numPixY,0,numPixY);
    else hExpAnti->Reset();
    if(!hExpDiff) hExpDiff = new TH2D("hExpDiff","Expected Diff",numPixX,0,numPixX,numPixY,0,numPixY);
    else hExpDiff->Reset();
    
    for(Int_t i=1; i<=hMaskPattern->GetNbinsX(); i++){
        for(Int_t j=1; j<=hMaskPattern->GetNbinsY(); j++){
            hDiffScaled->SetBinContent(i,j,hMaskPattern->GetBinContent(i,j));
            hMaskScaled->SetBinContent(i,j,hMaskPattern->GetBinContent(i,j));
            hAntiScaled->SetBinContent(i,j,-hMaskPattern->GetBinContent(i,j));
            if(hMaskPattern->GetBinContent(i,j) == 0.0){
                hMaskScaled->SetBinContent(i,j,1.0);
                hAntiScaled->SetBinContent(i,j,1.0);
            }
        }
    }
    
    if(!hDecM)
        hDecM = new TH2D("hDecM","Mask Decoder",
                         numPixX*2,-detRangeX*2.0,detRangeX*2.0,
                         numPixY*2,-detRangeY*2.0,detRangeY*2.0);
    else
        hDecM->SetBins(numPixX*2,-detRangeX*2.0,detRangeX*2.0,
                       numPixY*2,-detRangeY*2.0,detRangeY*2.0);
    
    if(!hDecA)
        hDecA = new TH2D("hDecA","Antimask Decoder",
                         numPixX*2,-detRangeX*2.0,detRangeX*2.0,
                         numPixY*2,-detRangeY*2.0,detRangeY*2.0);
    else
        hDecA->SetBins(numPixX*2,-detRangeX*2.0,detRangeX*2.0,
                       numPixY*2,-detRangeY*2.0,detRangeY*2.0);
    
    if(!hDecD)
        hDecD = new TH2D("hDecD","Mask-Antimask Decoder",
                         numPixX*2,-detRangeX*2.0,detRangeX*2.0,
                         numPixY*2,-detRangeY*2.0,detRangeY*2.0);
    else
        hDecD->SetBins(numPixX*2,-detRangeX*2.0,detRangeX*2.0,
                       numPixY*2,-detRangeY*2.0,detRangeY*2.0);
    
    hDecM->Reset();
    hDecA->Reset();
    hDecD->Reset();
    
    Int_t numDivisions = 10;  //Divisions per pixel
    Double_t stepSize = detPixelSize/(double)numDivisions;
    Int_t numStepsX = 2*numDivisions*numPixX;
    Int_t numStepsY = 2*numDivisions*numPixY;
    
    for(Int_t i=0; i<numStepsX; i++){
        Double_t stepX = -2.0*detRangeX + stepSize*(i+0.5);
        Double_t wrappedStepX = stepX;
        if(wrappedStepX < -maskRangeX) wrappedStepX += maskRangeX*2.0;
        if(wrappedStepX >= maskRangeX) wrappedStepX -= maskRangeX*2.0;
        Int_t binX = hMaskScaled->GetXaxis()->FindBin(wrappedStepX);
        for(Int_t j=0; j<numStepsY; j++){
            Double_t stepY = -2.0*detRangeY + stepSize*(j+0.5);
            Double_t wrappedStepY = stepY;
            if(wrappedStepY < -maskRangeY) wrappedStepY += maskRangeY*2.0;
            if(wrappedStepY >= maskRangeY) wrappedStepY -= maskRangeY*2.0;
            Int_t binY = hMaskScaled->GetYaxis()->FindBin(wrappedStepY);
            
            Double_t binVal = hMaskScaled->GetBinContent(binX,binY);
            hDecM->Fill(stepY,stepX,binVal);
            binVal = hAntiScaled->GetBinContent(binX,binY);
            hDecA->Fill(stepY,stepX,binVal);
            binVal = hDiffScaled->GetBinContent(binX,binY);
            hDecD->Fill(stepY,stepX,binVal);
            
            if( TMath::Abs(stepY) <= detRangeX && TMath::Abs(stepX) <= detRangeY ){
                Double_t vX = numPixX*(stepY+detRangeX)/(2.0*detRangeX);
                Double_t vY = numPixY*(stepX+detRangeY)/(2.0*detRangeY);
                if(binVal > 0.0) hExpMask->Fill(vX,vY,binVal);
                if(binVal < 0.0) hExpAnti->Fill(vX,vY,-binVal);
                hExpDiff->Fill(vX,vY,binVal);
            }
            
        }
    }
    
    Double_t totSteps = numDivisions;
    hDecM->Scale(1/pow(totSteps,2.0));
    hDecA->Scale(1/pow(totSteps,2.0));
    hDecD->Scale(1/pow(totSteps,2.0));
    
    hExpAnti->Scale(1/pow(totSteps,2.0));
    hExpMask->Scale(1/pow(totSteps,2.0));
    hExpDiff->Scale(1/pow(totSteps,2.0));
    
    TH1::AddDirectory(bPrevAddDirectory);

}

TH2D *ImageMaker::GetExpectedHitPatternMask(Double_t imX, Double_t imY){
    return GetExpectedHitPattern(imX,imY,1);
}

TH2D *ImageMaker::GetExpectedHitPatternAnti(Double_t imX, Double_t imY){
    return GetExpectedHitPattern(imX,imY,-1);
}

TH2D *ImageMaker::GetExpectedHitPatternDiff(Double_t imX, Double_t imY){
    return GetExpectedHitPattern(imX,imY,0);
}

TH2D *ImageMaker::GetExpectedHitPattern(Double_t imX, Double_t imY, Int_t iType){
    
    Double_t basisRange = (double(mRank)-1.0)/2.0;
    if(imX > basisRange || imX < -basisRange || imY > basisRange || imY < -basisRange ){
        printf("WARNING in ImageMaker::GetExpectedHitPattern() -- source position requested is outside FOV.\n");
        printf("\t%f, %f requested. FOV ranges are +/- %f in each dimension. Image will have wrap-around.\n",imX,imY,basisRange);
    }
    if(iType==1) hExpMask->Reset();
    else if(iType==-1) hExpAnti->Reset();
    else hExpDiff->Reset();
    
    Int_t numDivisions = 10;  //Divisions per pixel
    Double_t stepSize = detPixelSize/(double)numDivisions;
    Int_t numStepsX = 2*numDivisions*numPixX;
    Int_t numStepsY = 2*numDivisions*numPixY;
    
    Double_t imPixSize = maskRangeX/mRank;
    Double_t xOffset = -imX*imPixSize;
    Double_t yOffset = imY*imPixSize;
    
    for(Int_t i=0; i<numStepsX; i++){
        Double_t stepX = -2.0*detRangeX + stepSize*(i+0.5);
        Double_t wrappedStepX = stepX + yOffset;
        if(wrappedStepX < -maskRangeX) wrappedStepX += maskRangeX*2.0;
        if(wrappedStepX >= maskRangeX) wrappedStepX -= maskRangeX*2.0;
        Int_t binX = hMaskScaled->GetXaxis()->FindBin(wrappedStepX);
        for(Int_t j=0; j<numStepsY; j++){
            Double_t stepY = -2.0*detRangeY + stepSize*(j+0.5);
            Double_t wrappedStepY = stepY + xOffset;
            if(wrappedStepY < -maskRangeY) wrappedStepY += maskRangeY*2.0;
            if(wrappedStepY >= maskRangeY) wrappedStepY -= maskRangeY*2.0;
            Int_t binY = hMaskScaled->GetYaxis()->FindBin(wrappedStepY);
            
            Double_t binVal = hDiffScaled->GetBinContent(binX,binY);
            
            if( TMath::Abs(stepY) <= detRangeX && TMath::Abs(stepX) <= detRangeY ){
                Double_t vX = numPixX*(stepY+detRangeX)/(2.0*detRangeX);
                Double_t vY = numPixY*(stepX+detRangeY)/(2.0*detRangeY);
                if(iType==1 && binVal > 0.0) hExpMask->Fill(vX,vY,binVal);
                if(iType==-1 && binVal < 0.0) hExpAnti->Fill(vX,vY,-binVal);
                if(iType==0) hExpDiff->Fill(vX,vY,binVal);
            }
            
        }
    }
    
    Double_t totSteps = numDivisions;
    
    if(iType==1){
        hExpMask->Scale(1/pow(totSteps,2.0));
        return hExpMask;
    }
    else if(iType==-1){
        hExpAnti->Scale(1/pow(totSteps,2.0));
        return hExpAnti;
    }
    else{
        hExpDiff->Scale(1/pow(totSteps,2.0));
        return hExpDiff;
    }
}

TH2D *ImageMaker::GetImageHistogram(){
    
    if(hImage) return hImage;
    else{
        printf("\tImage not created yet.  Pass mask and/or antimask data to GetImage() method.\n");
        return 0;
    }
	
}


void ImageMaker::SetData(TH2D *hMaskDetectorHitPattern, TH2D *hAntiDetectorHitPattern){
    totalDetectorCounts = 0;
    totalDetectorCountsAdd = 0;
    if(hMaskDetectorHitPattern && hAntiDetectorHitPattern) iMask = -1;
    else if(hMaskDetectorHitPattern && !hAntiDetectorHitPattern) iMask = 0;
    else if(!hMaskDetectorHitPattern && hAntiDetectorHitPattern) iMask = 1;
    else{
        printf("Error: Neither mask nor antimask histogram passed to ImageMaker.\n");
        return;
    }
    
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

    if(hDHPa) hDHPa->Reset();
    if(hDHPm) hDHPm->Reset();
    if(hDHP) hDHP->Reset();
    
    TH2D *hDecS = 0;
    
    if(iMask<=0){
        hDHPm = (TH2D*)hMaskDetectorHitPattern->Clone("hDHPm");
        hDHP = (TH2D*)hMaskDetectorHitPattern->Clone("hDHP");
        hDecoder = (TH2D*)hDecM->Clone("hDecoder");
        hDecS = (TH2D*)hMaskScaled->Clone("hDecS");
        if(iMask<0){
            hDHPa = (TH2D*)hAntiDetectorHitPattern->Clone("hDHPa");
            hDHP->Add(hDHPm,hDHPa,1,-1);
            hDecoder = (TH2D*)hDecD->Clone("hDecoder");
            hDecS = (TH2D*)hDiffScaled->Clone("hDecS");
        }
    }
    else if(iMask==1){
        hDHPa = (TH2D*)hAntiDetectorHitPattern->Clone("hDHPa");
        hDHP = (TH2D*)hAntiDetectorHitPattern->Clone("hDHP");
        hDecoder = (TH2D*)hDecA->Clone("hDecoder");
        hDecS = (TH2D*)hAntiScaled->Clone("hDecS");
    }
    
    hDecoderStandard = (TH2D*)hDecS->Clone("hDecoderStandard");
    for(Int_t i=1; i<=hDecS->GetNbinsX(); i++){
        for(Int_t j=1; j<=hDecS->GetNbinsY(); j++){
            hDecoderStandard->SetBinContent(i,j,hDecS->GetBinContent(j,i));
        }
    }
    
    if(!hRebinnedDet)
        hRebinnedDet = new TH2D("hRebinnedDet","Rebinned data",
                                mRank*2,-maskRangeX/2.0,maskRangeX/2.0,
                                mRank*2,-maskRangeY/2.0,maskRangeY/2.0);
    else{
        hRebinnedDet->SetBins(mRank*2,-maskRangeX/2.0,maskRangeX/2.0,
                              mRank*2,-maskRangeY/2.0,maskRangeY/2.0);
        hRebinnedDet->Reset();
    }
    
    if(!hDetAbsAxis)
        hDetAbsAxis = new TH2D("hDetAbsAxis","Detector - absolute axis",numPixX,-detRangeX,detRangeX,
                               numPixY,-detRangeY,detRangeY);
    else
        hDetAbsAxis->SetBins(numPixX,-detRangeX,detRangeX,
                             numPixY,-detRangeY,detRangeY);
    
    if(!hRebinnedDetAdd)
        hRebinnedDetAdd = new TH2D("hRebinnedDetAdd","Rebinned data - Add edges",
                                   mRank*2,-maskRangeX/2.0,maskRangeX/2.0,
                                   mRank*2,-maskRangeY/2.0,maskRangeY/2.0);
    else{
        hRebinnedDetAdd->SetBins(mRank*2,-maskRangeX/2.0,maskRangeX/2.0,
                                 mRank*2,-maskRangeY/2.0,maskRangeY/2.0);
        hRebinnedDetAdd->Reset();
    }
    
    for(Int_t i=1; i<=hDHP->GetNbinsX(); i++){
        for(Int_t j=1; j<=hDHP->GetNbinsY(); j++){
            hDetAbsAxis->SetBinContent(i,j,hDHP->GetBinContent(i,j));
        }
    }
    
    Rehistogram(hDetAbsAxis,hRebinnedDet,hRebinnedDetAdd);
    if(hDHPm){
        TH2D *hDetAbsAxisM = (TH2D*)hDetAbsAxis->Clone("hDetAbsAxisM");
        hDetAbsAxisM->Reset();
        for(Int_t i=1; i<=hDHPm->GetNbinsX(); i++){
            for(Int_t j=1; j<=hDHPm->GetNbinsY(); j++){
                hDetAbsAxisM->SetBinContent(i,j,hDHPm->GetBinContent(i,j));
            }
        }
        TH2D *hRebinnedDetM = (TH2D*)hRebinnedDet->Clone("hRebinnedDetM");
        TH2D *hRebinnedDetAddM = (TH2D*)hRebinnedDetAdd->Clone("hRebinnedDetAddM");
        Rehistogram(hDetAbsAxisM,hRebinnedDetM,hRebinnedDetAddM);
        totalDetectorCounts += hRebinnedDetM->Integral();
        totalDetectorCountsAdd += hDetAbsAxisM->Integral();
    }
    
    if(hDHPa){
        TH2D *hDetAbsAxisA = (TH2D*)hDetAbsAxis->Clone("hDetAbsAxisA");
        hDetAbsAxisA->Reset();
        for(Int_t i=1; i<=hDHPa->GetNbinsX(); i++){
            for(Int_t j=1; j<=hDHPa->GetNbinsY(); j++){
                hDetAbsAxisA->SetBinContent(i,j,hDHPa->GetBinContent(i,j));
            }
        }
        TH2D *hRebinnedDetA = (TH2D*)hRebinnedDet->Clone("hRebinnedDetA");
        TH2D *hRebinnedDetAddA = (TH2D*)hRebinnedDetAdd->Clone("hRebinnedDetAddA");
        Rehistogram(hDetAbsAxisA,hRebinnedDetA,hRebinnedDetAddA);
        totalDetectorCounts += hRebinnedDetA->Integral();
        totalDetectorCountsAdd += hDetAbsAxisA->Integral();
    }
    
    TH1::AddDirectory(bPrevAddDirectory);

}

void ImageMaker::Rehistogram(TH2D *hOrig, TH2D *hNew, TH2D *hNewAdd){
	
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

    Double_t lowerX = hOrig->GetXaxis()->GetBinLowEdge(1);
    Double_t upperX = hOrig->GetXaxis()->GetBinLowEdge(hOrig->GetNbinsX()+1);
    Double_t lowerY = hOrig->GetYaxis()->GetBinLowEdge(1);
    Double_t upperY = hOrig->GetYaxis()->GetBinLowEdge(hOrig->GetNbinsY()+1);
	
    Double_t binWx = hOrig->GetXaxis()->GetBinWidth(1);
    Double_t binWy = hOrig->GetYaxis()->GetBinWidth(1);
    Double_t binArea = binWx*binWy;
    
    for(Int_t i=1; i<=hNew->GetNbinsX(); i++){
        Double_t xlo = hNew->GetXaxis()->GetBinLowEdge(i);
        if(xlo<lowerX) xlo = lowerX;
        Double_t xhi = hNew->GetXaxis()->GetBinLowEdge(i+1);
        if(xhi>upperX) xhi = upperX;
        Int_t xbinLo = hOrig->GetXaxis()->FindBin(xlo);
        Int_t xbinHi = hOrig->GetXaxis()->FindBin(xhi);
        for(Int_t j=1; j<=hNew->GetNbinsY(); j++){
            Double_t ylo = hNew->GetYaxis()->GetBinLowEdge(j);
            if(ylo<lowerY) ylo = lowerY;
            Double_t yhi = hNew->GetYaxis()->GetBinLowEdge(j+1);
            if(yhi>upperY) yhi = upperY;
            Int_t ybinLo = hOrig->GetYaxis()->FindBin(ylo);
            Int_t ybinHi = hOrig->GetYaxis()->FindBin(yhi);
			
            Double_t newVal = 0.0;
            for(Int_t ii=xbinLo; ii<=xbinHi; ii++){
				Double_t xOrigLo = hOrig->GetXaxis()->GetBinLowEdge(ii);
				Double_t xOrigHi = hOrig->GetXaxis()->GetBinLowEdge(ii+1);
				if(xlo > xOrigLo) xOrigLo = xlo;
				if(xhi < xOrigHi) xOrigHi = xhi;
				for(Int_t jj=ybinLo; jj<=ybinHi; jj++){
					Double_t yOrigLo = hOrig->GetYaxis()->GetBinLowEdge(jj);
					Double_t yOrigHi = hOrig->GetYaxis()->GetBinLowEdge(jj+1);
					if(ylo > yOrigLo) yOrigLo = ylo;
					if(yhi < yOrigHi) yOrigHi = yhi;
					
					Double_t binC = hOrig->GetBinContent(ii,jj);
					Double_t fracArea = (yOrigHi-yOrigLo)*(xOrigHi-xOrigLo)/binArea;
					newVal += binC*fracArea;
				}
            }
            hNew->SetBinContent(i,j,newVal);
        }
    }
    
    Double_t remainder = detRangeX-maskRangeX/2.0;
    Int_t extraBins = TMath::Floor(remainder/hNew->GetXaxis()->GetBinWidth(1));
    Double_t extraRange = extraBins*hNew->GetXaxis()->GetBinWidth(1);
    
    TH2D *hNewScale = (TH2D*)hNew->Clone("hNewScale");
    hNewScale->Reset();
    for(Int_t i=1; i<=hNewScale->GetNbinsX(); i++){
        for(Int_t j=1; j<=hNewScale->GetNbinsY(); j++){
            hNewScale->SetBinContent(i,j,1.0);
        }
    }
    
    TH2D *hNewTemp = new TH2D("hNewTemp","Temp",
                              (mRank+extraBins)*2,-maskRangeX/2.0-extraRange,maskRangeX/2.0+extraRange,
                              (mRank+extraBins)*2,-maskRangeY/2.0-extraRange,maskRangeY/2.0+extraRange);
    
    for(Int_t i=1; i<=hNewTemp->GetNbinsX(); i++){
        Double_t xlo = hNewTemp->GetXaxis()->GetBinLowEdge(i);
        if(xlo<lowerX) xlo = lowerX;
        Double_t xhi = hNewTemp->GetXaxis()->GetBinLowEdge(i+1);
        if(xhi>upperX) xhi = upperX;
        Int_t xbinLo = hOrig->GetXaxis()->FindBin(xlo);
        Int_t xbinHi = hOrig->GetXaxis()->FindBin(xhi);
        
        Double_t binCenterX = hNewTemp->GetXaxis()->GetBinCenter(i);
        Bool_t bEdgeX = false;
        if(binCenterX<-maskRangeX/2.0){
            binCenterX += maskRangeX;
            bEdgeX = true;
        }
        if(binCenterX> maskRangeX/2.0){
            binCenterX -= maskRangeX;
            bEdgeX = true;
        }
        Int_t i2 = hNewAdd->GetXaxis()->FindBin(binCenterX);
        
        for(Int_t j=1; j<=hNewTemp->GetNbinsY(); j++){
            Double_t ylo = hNewTemp->GetYaxis()->GetBinLowEdge(j);
            if(ylo<lowerY) ylo = lowerY;
            Double_t yhi = hNewTemp->GetYaxis()->GetBinLowEdge(j+1);
            if(yhi>upperY) yhi = upperY;
            Int_t ybinLo = hOrig->GetYaxis()->FindBin(ylo);
            Int_t ybinHi = hOrig->GetYaxis()->FindBin(yhi);
			
            Double_t binCenterY = hNewTemp->GetYaxis()->GetBinCenter(j);
            Bool_t bEdgeY = false;
            if(binCenterY<-maskRangeY/2.0){
                binCenterY += maskRangeY;
                bEdgeY = true;
            }
            if(binCenterY> maskRangeY/2.0){
                binCenterY -= maskRangeY;
                bEdgeY = true;
            }
            Int_t j2 = hNewAdd->GetYaxis()->FindBin(binCenterY);
            
            Double_t newVal = 0.0;
            for(Int_t ii=xbinLo; ii<=xbinHi; ii++){
				Double_t xOrigLo = hOrig->GetXaxis()->GetBinLowEdge(ii);
				Double_t xOrigHi = hOrig->GetXaxis()->GetBinLowEdge(ii+1);
				if(xlo > xOrigLo) xOrigLo = xlo;
				if(xhi < xOrigHi) xOrigHi = xhi;
				for(Int_t jj=ybinLo; jj<=ybinHi; jj++){
					Double_t yOrigLo = hOrig->GetYaxis()->GetBinLowEdge(jj);
					Double_t yOrigHi = hOrig->GetYaxis()->GetBinLowEdge(jj+1);
					if(ylo > yOrigLo) yOrigLo = ylo;
					if(yhi < yOrigHi) yOrigHi = yhi;
					
					Double_t binC = hOrig->GetBinContent(ii,jj);
					Double_t fracArea = (yOrigHi-yOrigLo)*(xOrigHi-xOrigLo)/binArea;
					newVal += binC*fracArea;
				}
            }
            
            hNewAdd->SetBinContent(i2,j2, hNewAdd->GetBinContent(i2,j2) + newVal);
            if(bEdgeX || bEdgeY)
                hNewScale->SetBinContent(i2,j2, hNewScale->GetBinContent(i2,j2) + 1.0);
        }
    }
    
    hNewAdd->Divide(hNewAdd,hNewScale,1,1);
    
    delete hNewTemp;
    
    TH1::AddDirectory(bPrevAddDirectory);
    
}

TH2D *ImageMaker::GetImage(TH2D *hMaskDetectorHitPattern, TH2D *hAntiDetectorHitPattern, Bool_t bOffset){
    
    printf("\nWARNING: The use of this method is not recommended. Use GetImage3.\n\n");
    
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

    SetData(hMaskDetectorHitPattern,hAntiDetectorHitPattern);
	
    //Make image
    TH2D *hDataMove = (TH2D*)hDecoder->Clone("hDataMove");
    hDataMove->Reset();
	
    TH2D *hProd = (TH2D*)hDecoder->Clone("hProd");
    hProd->Reset();
    
    if(!hImage)
        hImage = new TH2D("hImage","Coded Aperture Image",
                          numPixX,0,numPixX,numPixY,0,numPixY);
    else
        hImage->SetBins(numPixX,0,numPixX,numPixY,0,numPixY);
	
    
    for(Int_t i=1; i<=hImage->GetNbinsX(); i++){
        for(Int_t j=1; j<=hImage->GetNbinsY(); j++){
            hDataMove->Reset();
            for(Int_t i2=1; i2<=hDHP->GetNbinsX(); i2++){
                for(Int_t j2=1; j2<=hDHP->GetNbinsY(); j2++){
                    //	  hDataMove->SetBinContent(i2+i-1,j2+j-1,hDHP->GetBinContent(i2,j2));
                    hDataMove->SetBinContent(i2+i,j2+j,hDHP->GetBinContent(i2,j2));
                }
            }
			
            //Full image
            hProd->Reset();
            hProd->Multiply(hDecoder,hDataMove,1,1);
            //            hImage->SetBinContent(i,j,hProd->Integral());
            hImage->SetBinContent(hImage->GetNbinsX()-i+1,j,hProd->Integral());
			
        }
    }
    
    if(bOffset && iMask<0){
        TH1F *hVal = new TH1F("hVal","Values",100,2.5*hImage->GetMinimum(),-2.5*hImage->GetMinimum());
        for(Int_t i=1; i<=hImage->GetNbinsX(); i++){
            for(Int_t j=1; j<=hImage->GetNbinsY(); j++){
                hVal->Fill(hImage->GetBinContent(i,j));
            }
        }
        
        TF1 *fg = new TF1("fg",fgaus,2.5*hImage->GetMinimum(),-2.*hImage->GetMinimum(),3);
        fg->SetParameter(0,hVal->GetMaximum());
        fg->SetParameter(1,hVal->GetMean());
        fg->SetParameter(2,hVal->GetRMS());
        fg->SetParLimits(0,0.0,hVal->GetMaximum()*4.0);
        fg->SetParLimits(1,2.5*hImage->GetMinimum(),-2.5*hImage->GetMinimum());
        fg->SetParLimits(2,0.0,-5.0*hImage->GetMinimum());
        hVal->Fit("fg","nrq");
        Double_t offset = fg->GetParameter(1);
        
        for(Int_t i=1; i<=hImage->GetNbinsX(); i++){
            for(Int_t j=1; j<=hImage->GetNbinsY(); j++){
                hImage->SetBinContent(i,j,hImage->GetBinContent(i,j)-offset);
            }
        }
        
        delete hVal;
    }
    
    TH1::AddDirectory(bPrevAddDirectory);

    return hImage;
	
}

TH2D *ImageMaker::GetImage2(TH2D *hMaskDetectorHitPattern, TH2D *hAntiDetectorHitPattern, Bool_t bOffset){
    
    printf("\nWARNING: The use of this method is not recommended. Use GetImage3.\n\n");
    
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

    SetData(hMaskDetectorHitPattern,hAntiDetectorHitPattern);
	
    //Make image
    TH2D *hDataMove = (TH2D*)hDecoder->Clone("hDataMove");
    hDataMove->Reset();
	
    TH2D *hProd = (TH2D*)hDecoder->Clone("hProd");
    hProd->Reset();
    
    Int_t minBin = hDecM->GetXaxis()->FindBin( hMaskScaled->GetXaxis()->GetXmin()+0.001 )+1;
    Int_t maxBin = hDecM->GetXaxis()->FindBin( hMaskScaled->GetXaxis()->GetXmax()-0.001 )-1;
    Int_t numBins = (maxBin-minBin+1)/2;
    
    Int_t detBinRange = (numBins+1)/2;
    
    Int_t minDetBin = numPixX/2 - detBinRange + 1;
    Int_t maxDetBin = numPixX/2 + detBinRange;
    Int_t numDetBins = maxDetBin-minDetBin+1;
    
    if(numBins%2==0){
        minBin += 1;
        maxBin -= 1;
        numBins = (maxBin-minBin+1)/2;
    }
    
    Double_t factor = 2.0*numDetBins*detPixelSize/(hMaskScaled->GetXaxis()->GetXmax()-hMaskScaled->GetXaxis()->GetXmin());
    Double_t FOV = factor*mRank*maskElSize*(sourceMaskDist+maskDetDist)/maskDetDist;
    
    if(!hImage2)
        hImage2 = new TH2D("hImage2","Coded Aperture Image",
                           numBins,-FOV/2.0,FOV/2.0,numBins,-FOV/2.0,FOV/2.0);
    else
        hImage2->SetBins(numBins,-FOV/2.0,FOV/2.0,numBins,-FOV/2.0,FOV/2.0);
    
    for(Int_t i=1; i<=hImage2->GetNbinsX(); i++){
        for(Int_t j=1; j<=hImage2->GetNbinsY(); j++){
            
            hDataMove->Reset();
            for(Int_t i2=0; i2<numDetBins; i2++){
                for(Int_t j2=0; j2<numDetBins; j2++){
                    hDataMove->SetBinContent(minBin+i2+i-1,minBin+j2+j-1,hDHP->GetBinContent(i2+minDetBin,j2+minDetBin));
                }
            }
            
            //Full image
            hProd->Reset();
            hProd->Multiply(hDecoder,hDataMove,1,1);
            //            hImage2->SetBinContent(i,j,hProd->Integral());
            hImage2->SetBinContent(hImage2->GetNbinsX()-i+1,j,hProd->Integral());
            
        }
    }
	
    if(bOffset && iMask<0){
        TH1F *hVal = new TH1F("hVal","Values",100,2.5*hImage2->GetMinimum(),-2.5*hImage2->GetMinimum());
        for(Int_t i=1; i<=hImage2->GetNbinsX(); i++){
            for(Int_t j=1; j<=hImage2->GetNbinsY(); j++){
                hVal->Fill(hImage2->GetBinContent(i,j));
            }
        }
        
        TF1 *fg = new TF1("fg",fgaus,2.5*hImage2->GetMinimum(),-2.5*hImage2->GetMinimum(),3);
        fg->SetParameter(0,hVal->GetMaximum());
        fg->SetParameter(1,hVal->GetMean());
        fg->SetParameter(2,hVal->GetRMS());
        fg->SetParLimits(0,0.0,hVal->GetMaximum()*4.0);
        fg->SetParLimits(1,2.5*hImage2->GetMinimum(),-2.5*hImage2->GetMinimum());
        fg->SetParLimits(2,0.0,-5.0*hImage2->GetMinimum());
        hVal->Fit("fg","nrq");
        Double_t offset = fg->GetParameter(1);
        
        for(Int_t i=1; i<=hImage2->GetNbinsX(); i++){
            for(Int_t j=1; j<=hImage2->GetNbinsY(); j++){
                hImage2->SetBinContent(i,j,hImage2->GetBinContent(i,j)-offset);
            }
        }
        
        delete hVal;
    }
    
    TH1::AddDirectory(bPrevAddDirectory);

    return hImage2;
	
}

TH2D *ImageMaker::GetImage3(TH2D *hMaskDetectorHitPattern, TH2D *hAntiDetectorHitPattern, Bool_t bOffset, Bool_t bAdd, Bool_t bDoubleSample){
    
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

    SetData(hMaskDetectorHitPattern,hAntiDetectorHitPattern);
    
    TH2D *hRebinnedDetAddSS = 0;
    TH2D *hRebinnedDetSS = 0;
    
    if(!bDoubleSample){
        TH2D *hRebinnedDetAddSS_0 = (TH2D*)hRebinnedDetAdd->Clone("hRebinnedDetAddSS_0");
        TH2D *hRebinnedDetSS_0 = (TH2D*)hRebinnedDet->Clone("hRebinnedDetSS_0");
        hRebinnedDetAddSS = (TH2D*)hRebinnedDetAddSS_0->Rebin2D(2,2,"hRebinnedDetAddSS");
        hRebinnedDetSS = (TH2D*)hRebinnedDetSS_0->Rebin2D(2,2,"hRebinnedDetSS");
    }
    
    TH2D *hUseData = 0;
    TH2D *hDecoderSS = 0;
    if(bDoubleSample){
        if(bAdd) hUseData = (TH2D*)hRebinnedDetAdd->Clone("hUseData");
        else hUseData = (TH2D*)hRebinnedDet->Clone("hUseData");
	}
    else{
        if(bAdd) hUseData = (TH2D*)hRebinnedDetAddSS->Clone("hUseData");
        else hUseData = (TH2D*)hRebinnedDetSS->Clone("hUseData");

        hDecoderSS = new TH2D("hDecoderSS","Single sample decoder",2*mRank-1,0,2*mRank-1,2*mRank-1,0,2*mRank-1);
        
        for(Int_t ix=0; ix<hDecoderStandard->GetNbinsX(); ix++){
            for(Int_t iy=0; iy<hDecoderStandard->GetNbinsY(); iy++){
                
                if( (ix)%2==0 && (iy)%2==0 && (ix+1)/2 <= hDecoderSS->GetNbinsX() && (iy+1)/2 <= hDecoderSS->GetNbinsY() ){
                    hDecoderSS->SetBinContent( (ix+2)/2, (iy+2)/2, hDecoderStandard->GetBinContent( ix+1, iy+1 ) );
                    
                }
            }
        }
    }

    TH2D *hDecoderToUse = 0;
    if(bDoubleSample) hDecoderToUse = hDecoderStandard;
    else hDecoderToUse = hDecoderSS;
    
    //Make image
    TH2D *hDataMove = (TH2D*)hDecoderToUse->Clone("hDataMove");
    hDataMove->Reset();
	
    TH2D *hProd = (TH2D*)hDecoderToUse->Clone("hProd");
    hProd->Reset();
    
    Double_t FOV = mRank*maskElSize*(sourceMaskDist+maskDetDist)/maskDetDist;
    Int_t numBins = 2*mRank-1;
    if(bDoubleSample) FOV *= double(numBins)/double(numBins+1);
    else numBins = mRank;
    
    if(!hImage3)
        hImage3 = new TH2D("hImage3","Coded Aperture Image",
                           numBins,-FOV/2.0,FOV/2.0,numBins,-FOV/2.0,FOV/2.0);
    else
        hImage3->SetBins(numBins,-FOV/2.0,FOV/2.0,numBins,-FOV/2.0,FOV/2.0);
    
    for(Int_t i=1; i<=hImage3->GetNbinsX(); i++){
        for(Int_t j=1; j<=hImage3->GetNbinsY(); j++){
            
            hDataMove->Reset();
            for(Int_t i2=1; i2<=hUseData->GetNbinsX(); i2++){
                for(Int_t j2=1; j2<=hUseData->GetNbinsY(); j2++){
                    hDataMove->SetBinContent(i+i2,j+j2,hUseData->GetBinContent(i2,j2));
                }
            }
            
            //Full image
            hProd->Reset();
            hProd->Multiply(hDecoderToUse,hDataMove,1,1);
            //            hImage3->SetBinContent(i,j,hProd->Integral());
            hImage3->SetBinContent(hImage3->GetNbinsX()-i+1,j,hProd->Integral());
			
        }
    }
		
    if(bOffset && iMask<0){
        if(hOffsetHist) delete hOffsetHist;
        
        hOffsetHist = new TH1F("hVal","Values",100,2.5*hImage3->GetMinimum(),-2.5*hImage3->GetMinimum());
        hOffsetHist->SetDirectory(0);
        for(Int_t i=1; i<=hImage3->GetNbinsX(); i++){
            for(Int_t j=1; j<=hImage3->GetNbinsY(); j++){
                hOffsetHist->Fill(hImage3->GetBinContent(i,j));
            }
        }
        
        TF1 *fg = new TF1("fg",fgaus,2.5*hImage3->GetMinimum(),-2.5*hImage3->GetMinimum(),3);
        fg->SetParameter(0,hOffsetHist->GetMaximum());
        fg->SetParameter(1,hOffsetHist->GetMean());
        fg->SetParameter(2,hOffsetHist->GetRMS());
        fg->SetParLimits(0,0.0,hOffsetHist->GetMaximum()*4.0);
        fg->SetParLimits(1,2.5*hImage3->GetMinimum(),-2.5*hImage3->GetMinimum());
        fg->SetParLimits(2,0.0,-5.0*hImage3->GetMinimum());
        hOffsetHist->Fit("fg","nrq");
        Double_t offset = fg->GetParameter(1);
        offsetPar.Set(4);
        offsetParE.Set(4);
        for(int ipar = 0; ipar < 3; ipar++){
            offsetPar[ipar]=fg->GetParameter(ipar);
            offsetParE[ipar]=fg->GetParError(ipar);
        }
        offsetPar[3]=hOffsetHist->GetRMS();
        
        for(Int_t i=1; i<=hImage3->GetNbinsX(); i++){
            for(Int_t j=1; j<=hImage3->GetNbinsY(); j++){
                hImage3->SetBinContent(i,j,hImage3->GetBinContent(i,j)-offset);
            }
        }
        
        delete fg;
    }
    
    delete hUseData;
    
    TH1::AddDirectory(bPrevAddDirectory);

    return hImage3;
	
}

TH2D *ImageMaker::GetCurrentMaskPattern(){
	
	
    if(hMaskPattern) return hMaskPattern;
    else{
        return GetMaskPattern(mRank);
    }
}

TH2D *ImageMaker::GetMaskPattern(Int_t base){
    
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

    if(!hMaskPattern)
        hMaskPattern = new TH2D("hMaskPattern","Mask Pattern",4*base,0,4*base,4*base,0,4*base);
    else
        hMaskPattern->SetBins(4*base,0,4*base,4*base,0,4*base);
	
    for(Int_t i=1; i<=2*base; i++){
        for(Int_t j=1; j<=2*base; j++){
			
            Int_t ii = i%base;
            Int_t jj = j%base;
			
            Bool_t bElement = false;
            if(ii==0)	bElement = true;
            else if(jj==0) bElement = false;
            else{
                Bool_t bCp = false;
                Int_t Cp = 0;
                for(Int_t k=1; k<base; k++)
                    if( Int_t(pow((double)k,(double)2))%base == ii ) bCp = true;
                if(bCp) Cp = 1;
                else Cp = -1;
				
                Bool_t bCq = false;
                Int_t Cq = 0;
                for(Int_t k=1; k<base; k++)
                    if( Int_t(pow((double)k,(double)2))%base == jj ) bCq = true;
                if(bCq) Cq = 1;
                else Cq = -1;
				
                if(Cp*Cq == 1) bElement = false;
                else bElement = true;
            }
			
            if(bElement){
                hMaskPattern->SetBinContent((2*j-1)%hMaskPattern->GetNbinsX()+1,(2*i-1)%hMaskPattern->GetNbinsY()+1,-1);
                hMaskPattern->SetBinContent((2*j)%hMaskPattern->GetNbinsX()+1,(2*i-1)%hMaskPattern->GetNbinsY()+1,-1);
                hMaskPattern->SetBinContent((2*j)%hMaskPattern->GetNbinsX()+1,(2*i)%hMaskPattern->GetNbinsY()+1,-1);
                hMaskPattern->SetBinContent((2*j-1)%hMaskPattern->GetNbinsX()+1,(2*i)%hMaskPattern->GetNbinsY()+1,-1);
            }
            else{
                hMaskPattern->SetBinContent((2*j-1)%hMaskPattern->GetNbinsX()+1,(2*i-1)%hMaskPattern->GetNbinsY()+1,1);
                hMaskPattern->SetBinContent((2*j)%hMaskPattern->GetNbinsX()+1,(2*i-1)%hMaskPattern->GetNbinsY()+1,1);
                hMaskPattern->SetBinContent((2*j)%hMaskPattern->GetNbinsX()+1,(2*i)%hMaskPattern->GetNbinsY()+1,1);
                hMaskPattern->SetBinContent((2*j-1)%hMaskPattern->GetNbinsX()+1,(2*i)%hMaskPattern->GetNbinsY()+1,1);
            }
        }
    }
	
    for(Int_t ii=-1; ii<=0; ii++){
        for(Int_t jj=-1; jj<=0; jj++){
            hMaskPattern->SetBinContent((2*base+ii)%hMaskPattern->GetNbinsX()+1,(2*base+jj)%hMaskPattern->GetNbinsY()+1,0.0);
            hMaskPattern->SetBinContent((4*base+ii)%hMaskPattern->GetNbinsX()+1,(2*base+jj)%hMaskPattern->GetNbinsY()+1,0.0);
            hMaskPattern->SetBinContent((4*base+ii)%hMaskPattern->GetNbinsX()+1,(4*base+jj)%hMaskPattern->GetNbinsY()+1,0.0);
            hMaskPattern->SetBinContent((2*base+ii)%hMaskPattern->GetNbinsX()+1,(4*base+jj)%hMaskPattern->GetNbinsY()+1,0.0);
        }
    }
    
    TH1::AddDirectory(bPrevAddDirectory);

    return hMaskPattern;
	
}

Double_t ImageMaker::CalcImageCounts(Int_t& pixelsUsed, Double_t zeroSupThresh, Int_t imageIdx) const
{
    
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

    Double_t sum=0.0;
    pixelsUsed=0;
    
    TH2* im = 0;
    if (imageIdx == 3)
        im = hImage3;
    else if(imageIdx == 2)
        im = hImage2;
    else if(imageIdx ==1 )
        im = hImage;
    
    if(!im) return 0.0;
    
    for(int xbin = 1; xbin<=im->GetNbinsX(); xbin++)
        for(int ybin = 1; ybin<=im->GetNbinsX(); ybin++)
        {
            Double_t val = im->GetBinContent(xbin,ybin);
            if(val>=zeroSupThresh)
            {
                pixelsUsed++;
                sum+=val;
            }
        }
    return sum;
    
    TH1::AddDirectory(bPrevAddDirectory);

}

Double_t ImageMaker::GetCorrection() const
{
    if(mRank!=19)
    {
        std::cerr<<"Correction only implemented for rank 19 "<<std::endl;
        return 1.0;
    }
    Double_t Dn = (maskDetDist + sourceMaskDist)/sourceMaskDist*mRank*maskElSize/detPixelSize;
    //printf("FocalLength(%f) sourceMaskDistance(%f) MaskElSize(%f) DetectorPixelSize(%f) DetectorPixels(%f) Correction(%f)\n",maskDetDist,sourceMaskDist,maskElSize,detPixelSize,Dn,avgPixelWeights->Eval(Dn));
    return avgPixelWeights->Eval(Dn);
}

TH2D* ImageMaker::makeFloat(TH2* h)
{
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);
    
    TString hname;
    hname.Form("%s_F",h->GetName());
    
    TH2D* h2f = new TH2D(hname,h->GetTitle(),
                    h->GetNbinsX(),h->GetXaxis()->GetXmin(),h->GetXaxis()->GetXmax(),
                         h->GetNbinsY(),h->GetYaxis()->GetXmin(),h->GetYaxis()->GetXmax());
    h2f->SetDirectory(0);
    for(int ix =1; ix<=h->GetNbinsX(); ix++){
        for(int iy =1; iy<=h->GetNbinsY(); iy++){
            h2f->SetBinContent(ix,iy,h->GetBinContent(ix,iy));
        }
    }
    
    TH1::AddDirectory(bPrevAddDirectory);

    return h2f;
}

TH2D *ImageMaker::GetImage3(TH2I *hMaskDetectorHitPattern, TH2I *hAntiDetectorHitPattern, Bool_t bOffset, Bool_t bAdd, Bool_t bDoubleSample)
{
    
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);
    
    TH2D* hMaskD = makeFloat(hMaskDetectorHitPattern);
    TH2D* hAntiD = makeFloat(hAntiDetectorHitPattern);
    
    TH1::AddDirectory(bPrevAddDirectory);

    return GetImage3(hMaskD,hAntiD,bOffset,bAdd,bDoubleSample);
}


TH2D *ImageMaker::GetImage3(TH2 *hMaskDetectorHitPattern, TH2 *hAntiDetectorHitPattern, Bool_t bOffset, Bool_t bAdd, Bool_t bDoubleSample)
{
    return GetImage3(dynamic_cast<TH2D*>(hMaskDetectorHitPattern),dynamic_cast<TH2D*>(hAntiDetectorHitPattern),bOffset,bAdd, bDoubleSample);
}

TH3D *ImageMaker::GetImage3D(TH3* hMask, TH3 *hAnti, TH3* hVoid,Bool_t bOffset, Bool_t bAdd)
{
 
    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

    delete hImage3D;
    hImage3D = 0;
    delete hData3D;
    hData3D = dynamic_cast<TH3D*>(hMask->Clone("hData3D"));
    hData3D->SetDirectory(0);
    
    for (int iz = 1; iz<=hMask->GetNbinsZ();iz++){
        hVoid->GetZaxis()->SetRange(iz,iz);
        hMask->GetZaxis()->SetRange(iz,iz);
        hAnti->GetZaxis()->SetRange(iz,iz);
        TH2* hVoidT = dynamic_cast<TH2*>(hVoid->Project3D("yx"));
        TH2* hMaskT = dynamic_cast<TH2*>(hMask->Project3D("yx"));
        TH2* hAntiT = dynamic_cast<TH2*>(hAnti->Project3D("yx"));
        double scale = hVoidT->GetNbinsX()*hVoidT->GetNbinsY()/float(hVoidT->Integral());
        hVoidT->Scale(scale);
        hMaskT->Divide(hVoidT);
        hAntiT->Divide(hVoidT);
        TH2* hIm = GetImage3(hMaskT,hAntiT,bOffset,bAdd);
        TH2* hData = GetData();
        if(!hImage3D){
            hImage3D = new TH3D("hImage3D","hImage3D",
                                hIm->GetNbinsX(),hIm->GetXaxis()->GetXmin(),hIm->GetXaxis()->GetXmax(),
                                hIm->GetNbinsY(),hIm->GetYaxis()->GetXmin(),hIm->GetYaxis()->GetXmax(),
                                hMask->GetNbinsZ(),hMask->GetZaxis()->GetXmin(),hMask->GetZaxis()->GetXmax());
            hImage3D->SetDirectory(0);
        }
        for (int ix = 1; ix<=hImage3D->GetNbinsX();ix++){
            for (int iy = 1; iy<=hImage3D->GetNbinsY();iy++){
                hImage3D->SetBinContent(ix,iy,iz,hIm->GetBinContent(ix,iy));
            }
        }
        for (int ix = 1; ix<=hData3D->GetNbinsX();ix++){
            for (int iy = 1; iy<=hData3D->GetNbinsY();iy++){
                hData3D->SetBinContent(ix,iy,iz,hData->GetBinContent(ix,iy));
            }
        }
        
    }
    
    TH1::AddDirectory(bPrevAddDirectory);

    return hImage3D;
}


TH3D* ImageMaker::make3DFromSerial(TH2* h, int nCols)
{

    Bool_t bPrevAddDirectory = TH1::AddDirectoryStatus();
    TH1::AddDirectory(false);

    TString hname;
    hname.Form("%s_2D",h->GetName());
    int nRows=h->GetNbinsX()/nCols;
    
    TH3D* h3f = new TH3D(hname,h->GetTitle(),
                    nCols,0.0,double(nCols),
                    nRows,0.0,double(nRows),
                    h->GetNbinsY(),h->GetYaxis()->GetXmin(),h->GetYaxis()->GetXmax());
    h3f->SetDirectory(0);
    for(int ix =1; ix<=h->GetNbinsX(); ix++){
        for(int iy =1; iy<=h->GetNbinsY(); iy++){
            int ixnew = ix%40;
            int iynew = int(ix/40);
            h3f->SetBinContent(ixnew,iynew,iy,h->GetBinContent(ix,iy));
        }
    }

    TH1::AddDirectory(bPrevAddDirectory);

    return h3f;
}

void ImageMaker::MakeCorrectionCurve()
{
    Double_t pixeldim[] = {
        25.000000,25.100000,25.200000,25.300000,25.400000,25.500000,25.600000,25.700000,25.800000,25.900000,26.000000,26.100000,26.200000,26.300000,26.400000,26.500000,26.600000,26.700000,26.800000,26.900000,27.000000,27.100000,27.200000,27.300000,27.400000,27.500000,27.600000,27.700000,27.800000,27.900000,28.000000,28.100000,28.200000,28.300000,28.400000,28.500000,28.600000,28.700000,28.800000,28.900000,29.000000,29.100000,29.200000,29.300000,29.400000,29.500000,29.600000,29.700000,29.800000,29.900000,30.000000,30.100000,30.200000,30.300000,30.400000,30.500000,30.600000,30.700000,30.800000,30.900000,31.000000,31.100000,31.200000,31.300000,31.400000,31.500000,31.600000,31.700000,31.800000,31.900000,32.000000,32.100000,32.200000,32.300000,32.400000,32.500000,32.600000,32.700000,32.800000,32.900000,33.000000,33.100000,33.200000,33.300000,33.400000,33.500000,33.600000,33.700000,33.800000,33.900000,34.000000,34.100000,34.200000,34.300000,34.400000,34.500000,34.600000,34.700000,34.800000,34.900000,35.000000,35.100000,35.200000,35.300000,35.400000,35.500000,35.600000,35.700000,35.800000,35.900000,36.000000,36.100000,36.200000,36.300000,36.400000,36.500000,36.600000,36.700000,36.800000,36.900000,37.000000,37.100000,37.200000,37.300000,37.400000,37.500000,37.600000,37.700000,37.800000,37.900000,38.000000,38.100000,38.200000,38.300000,38.400000,38.500000,38.600000,38.700000,38.800000,38.900000,39.000000,39.100000,39.200000,39.300000,39.400000,39.500000,39.600000,39.700000,39.800000,39.900000,40.000000,40.100000,40.200000,40.300000,40.400000,40.500000,40.600000,40.700000,40.800000,40.900000,41.000000,41.100000,41.200000,41.300000,41.400000,41.500000,41.600000,41.700000,41.800000,41.900000,42.000000,42.100000,42.200000,42.300000,42.400000,42.500000,42.600000,42.700000,42.800000,42.900000,43.000000,43.100000,43.200000,43.300000,43.400000,43.500000,43.600000,43.700000,43.800000,43.900000,44.000000,44.100000,44.200000,44.300000,44.400000,44.500000,44.600000,44.700000,44.800000,44.900000,45.000000,45.100000,45.200000,45.300000,45.400000,45.500000,45.600000,45.700000,45.800000,45.900000,46.000000,46.100000,46.200000,46.300000,46.400000,46.500000,46.600000,46.700000,46.800000,46.900000,47.000000,47.100000,47.200000,47.300000,47.400000,47.500000,47.600000,47.700000,47.800000,47.900000,48.000000,48.100000,48.200000,48.300000,48.400000,48.500000,48.600000,48.700000,48.800000,48.900000,49.000000,49.100000,49.200000,49.300000,49.400000,49.500000,49.600000,49.700000,49.800000,49.900000,50.000000};
    Double_t avg[]={
     0.597132,0.609183,0.599932,0.640077,0.632883,0.607894,0.603798,0.605031,0.604540,0.625038,0.592900,0.591277,0.584173,0.585477,0.601054,0.616497,0.622788,0.626530,0.664171,0.682915,0.665583,0.665175,0.646573,0.646364,0.627861,0.626848,0.632982,0.635550,0.666569,0.673020,0.637259,0.620981,0.605308,0.584067,0.574541,0.547801,0.551019,0.545895,0.558269,0.561544,0.584276,0.572846,0.554237,0.552499,0.537765,0.545536,0.545414,0.542367,0.553995,0.559466,0.572152,0.580101,0.581465,0.601590,0.606316,0.596976,0.580103,0.581784,0.577021,0.578600,0.594651,0.589784,0.614526,0.628513,0.625159,0.614549,0.606913,0.597920,0.618163,0.617974,0.628802,0.623704,0.620583,0.635976,0.656252,0.660397,0.680466,0.686014,0.692452,0.700059,0.709022,0.689228,0.701560,0.697139,0.681162,0.704958,0.698327,0.677167,0.674234,0.656027,0.649604,0.626224,0.615910,0.607013,0.598025,0.586992,0.590770,0.591590,0.595458,0.616914,0.628527,0.652800,0.659714,0.688077,0.714325,0.747943,0.733043,0.759234,0.765097,0.733481,0.733719,0.720282,0.711655,0.702479,0.680037,0.674446,0.665576,0.671347,0.670483,0.675282,0.681766,0.688817,0.714679,0.726279,0.769885,0.795732,0.814475,0.861711,0.903540,0.974989,0.997230,0.965808,0.904496,0.863732,0.818019,0.802743,0.776337,0.740425,0.725056,0.701376,0.695964,0.691141,0.687969,0.690217,0.686161,0.695971,0.702623,0.723781,0.734795,0.748880,0.758539,0.759467,0.789206,0.783709,0.762488,0.770824,0.748061,0.720821,0.701947,0.689612,0.677066,0.668260,0.650781,0.653655,0.649818,0.644502,0.659341,0.668714,0.677929,0.687913,0.709670,0.716474,0.733126,0.736848,0.755669,0.762310,0.743895,0.763202,0.762980,0.754126,0.771285,0.765207,0.760260,0.748758,0.753054,0.738298,0.736269,0.728271,0.712023,0.722367,0.721200,0.714113,0.715635,0.701303,0.709693,0.711620,0.726421,0.732807,0.721032,0.703637,0.708754,0.698035,0.698300,0.703337,0.703518,0.722736,0.725737,0.729583,0.710251,0.710670,0.706282,0.698793,0.695930,0.687919,0.692664,0.689229,0.690135,0.702222,0.704949,0.720123,0.728854,0.718094,0.713471,0.708724,0.711420,0.713095,0.731875,0.739620,0.754968,0.766468,0.777983,0.802343,0.798981,0.780444,0.780072,0.777423,0.779225,0.787428,0.792846,0.805306,0.806344,0.821588,0.807426,0.787762,0.784452,0.781837,0.773687,0.765511,0.766021,0.771108,0.773357};
    if(avgPixelWeights) delete avgPixelWeights;
    avgPixelWeights = new TGraph(251,pixeldim,avg);
}
