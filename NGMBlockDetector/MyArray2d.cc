
#include "MyArray2d.h"

using namespace std;

ClassImp(MyArray2d)

MyArray2d::MyArray2d(Int_t nX, Int_t nY){

    theArray = 0;
    numX = 0;
    numY = 0;
    
    Reset(nX, nY);

}

MyArray2d::MyArray2d(Int_t nX, Int_t nY,
                     Double_t lowX, Double_t hiX,
                     Double_t lowY, Double_t hiY){
    
    theArray = 0;
    numX = 0;
    numY = 0;
    
    Reset(nX, nY, lowX, hiX, lowY, hiY);

}

MyArray2d::~MyArray2d(){
    delete [] theArray;
    theArray = NULL;
}

MyArray2d * MyArray2d::Clone(Bool_t bZeroValues){
    
    MyArray2d * returnArray = new MyArray2d(numX,numY);
    
    if(bBinX) returnArray->SetRangeX(xRange[0],xRange[1]);
    if(bBinY) returnArray->SetRangeY(yRange[0],yRange[1]);
    
    if(!bZeroValues) returnArray->CopyValues(this);
    
    return returnArray;
}

void MyArray2d::Reset(Int_t nX, Int_t nY,
                      Double_t lowX, Double_t hiX,
                      Double_t lowY, Double_t hiY){

    Reset(nX,nY);
    
    SetRangeX(lowX, hiX);
    SetRangeY(lowY, hiY); 
    
}

void MyArray2d::Reset(Int_t nX, Int_t nY){
    
    numX = nX;
    numY = nY;
    num = numX*numY;
    
    Double_t *resetArray = new Double_t[num];
    if(theArray) delete [] theArray;
    theArray = resetArray;
    
    bRandomSetup = false;
    
    bBinX = false;
    bBinY = false;
    binSizeX = -1.0;
    binSizeY = -1.0;
    for(Int_t i=0; i<2; i++){
        xRange[i] = -1.0;
        yRange[i] = -1.0;
    }
    
    ClearValues();
}

void MyArray2d::SetRange(Double_t lowLimit, Double_t hiLimit, Bool_t bAxis){
    
    if(lowLimit < hiLimit){
        if(bAxis){
            xRange[0] = lowLimit;
            xRange[1] = hiLimit;
            binSizeX = (xRange[1]-xRange[0])/double(numX);
            bBinX = true;
        }
        else{
            yRange[0] = lowLimit;
            yRange[1] = hiLimit;
            binSizeY = (yRange[1]-yRange[0])/double(numY);
            bBinY = true;
        }
    }
    else printf("Error in MyArray2d::SetRange() - lower limit (%f) larger than higher limit (%f)\n",lowLimit,hiLimit);
}

void MyArray2d::SetBinSize(Double_t sizeVal, Bool_t bAxis, Double_t lowLimit){
    
    if(sizeVal > 0.0){
        if(bAxis){
            binSizeX = sizeVal;
            xRange[0] = lowLimit;
            xRange[1] = xRange[0] + binSizeX*numX;
            bBinX = true;
        }
        else{
            binSizeY = sizeVal;
            yRange[0] = lowLimit;
            yRange[1] = yRange[0] + binSizeY*numY;
            bBinY = true;
        }
    }
    else printf("Error in MyArray2d::SetBinSize() - bin size requested is negative (%f)\n",sizeVal);

}

Int_t MyArray2d::globalIndex(Int_t ix, Int_t iy){
    Int_t index = iy*numX + ix;
    if(index<0 || index >= num || ix > numX || ix<0 || iy > numY || iy<0){
        printf("ERROR in MyArray2d::globalIndex(): Array index out of bounds: %d, %d (%d). Upper value is %d (%d x %d).\n\n",
               ix,iy,index,num-1,numX-1,numY-1);
        exit(0);
    }
    
    return index;
}

void MyArray2d::IndexXY(Int_t index, Int_t &binX, Int_t &binY){
    //    Int_t index = iy*numX + ix;
    binX = index%numX;
    binY = (index-binX)/numX;
}

Double_t MyArray2d::val(Int_t ix, Int_t iy){
    return theArray[globalIndex(ix,iy)];
}

Double_t MyArray2d::CycledVal(Int_t ix, Int_t iy){
    
    return val(GetCycledIndexX(ix),GetCycledIndexY(iy));
}

Int_t MyArray2d::GetCycledIndex(Int_t i0, Bool_t bAxis){
    
    //add a multiple of 100 cycles to be able to handle negative numbers properly
    if(bAxis) return (i0 + 100*GetLengthX())%(GetLengthX());
    else return (i0 + 100*GetLengthY())%(GetLengthY());

}

Int_t MyArray2d::GetCycledIndex(Double_t d0, Bool_t bAxis, Double_t &cyclesMoved){
    
    Int_t rbin;
    Double_t sumNeeded = 0.0;
    cyclesMoved = 0;
    if(bAxis){
        Double_t thisRange = double(numX);
        if(bBinX) thisRange = GetHighX()-GetLowX();
        if(d0 < GetLowX()){
            cyclesMoved = TMath::Ceil((GetLowX()-d0)/thisRange);
            sumNeeded = thisRange*double(cyclesMoved);
        }
        else if(d0 > GetHighX()){
            cyclesMoved = -TMath::Ceil((d0-GetHighX())/thisRange);
            sumNeeded = thisRange*double(cyclesMoved);
        }
 
        Bool_t bBin = GetArrayIndexX(d0+sumNeeded,rbin);
        if(bBin) return rbin;        
    }
    else{
        Double_t thisRange = double(numY);
        if(bBinY) thisRange = GetHighY()-GetLowY();
        if(d0 < GetLowY()){
            cyclesMoved = TMath::Ceil((GetLowY()-d0)/thisRange);
            sumNeeded = thisRange*double(cyclesMoved);
        }
        else if(d0 > GetHighY()){
            cyclesMoved = -TMath::Ceil((d0-GetHighY())/thisRange);
            sumNeeded = thisRange*double(cyclesMoved);
        }

        Bool_t bBin = GetArrayIndexY(d0+sumNeeded,rbin);
        if(bBin) return rbin;
    }
    
    return -99999;
}

void MyArray2d::SetSumValue(Double_t newVal){
    arraySum = newVal;
    bRandomSetup = false;
}

void MyArray2d::IncrementSumValue(Double_t incVal){
    arraySum += incVal;
    bRandomSetup = false;
}

void MyArray2d::SetVal(Int_t ix, Int_t iy, Double_t dVal){
    Double_t prevVal = theArray[globalIndex(ix, iy)];
    theArray[globalIndex(ix,iy)] = dVal;
    IncrementSumValue(dVal-prevVal);
}

void MyArray2d::AddVal(Int_t ix, Int_t iy, Double_t dVal){
    theArray[globalIndex(ix,iy)] += dVal;
    IncrementSumValue(dVal);
}

void MyArray2d::ClearValues(){
    for(Int_t i=0; i<num && theArray; i++){
        theArray[i] = 0.0;
    }      
    SetSumValue(0.0);
}

void MyArray2d::SetUniform(Double_t newVal){
    for(Int_t i=0; i<num && theArray; i++){
        theArray[i] = newVal;
        IncrementSumValue(newVal);
    }
}

void MyArray2d::CopyValues(MyArray2d *arrayToCopy){
    
    if(theArray){
        for(Int_t i=0; i<numX; i++){
            for(Int_t j=0; j<numY; j++){
                theArray[globalIndex(i,j)] = arrayToCopy->val(i, j);
            }
        }
        SetSumValue(arrayToCopy->Integral());
    }
    else{
        printf("ERROR in MyArray2d::CopyValues(): Trying to copy array values without instantiating array first\n");
    }
}

void MyArray2d::AddArrays(MyArray2d *array1, MyArray2d *array2, Double_t scale1, Double_t scale2){
    
    if(array1 && array2){
        if(array1->GetLengthX()==array2->GetLengthX() && array1->GetLengthY()==array2->GetLengthY()){
            SetSumValue(0.0);
            for(Int_t i=0; i<numX; i++){
                for(Int_t j=0; j<numY; j++){
                    Double_t sumVal = scale1*array1->val(i, j) + scale2*array2->val(i,j);
                    theArray[globalIndex(i,j)] = sumVal;
                    IncrementSumValue(sumVal);
                }
            }
        }
        else printf("ERROR in MyArray2d::AddArrays(): Arrays are not the same length.\n");
        
    }
    else printf("ERROR in MyArray2d::AddArrays(): Trying to add arrays but at least one pointer is null\n");
    
}

void MyArray2d::DivideArrays(MyArray2d *array1, MyArray2d *array2){
    
    if(array1 && array2){
        if(array1->GetLengthX()==array2->GetLengthX() && array1->GetLengthY()==array2->GetLengthY()){
            SetSumValue(0.0);
            for(Int_t i=0; i<numX; i++){
                for(Int_t j=0; j<numY; j++){
                    Double_t sumVal = 0.0;
                    if(TMath::Abs(array2->val(i,j)) > 1e-10)
                        sumVal = array1->val(i, j)/array2->val(i,j);
                    theArray[globalIndex(i,j)] = sumVal;
                    IncrementSumValue(sumVal);
                }
            }
        }
        else printf("ERROR in MyArray2d::DivideArrays(): Arrays are not the same length.\n");
        
    }
    else printf("ERROR in MyArray2d::DivideArrays(): Trying to add arrays but at least one pointer is null\n");
    
}

void MyArray2d::MultiplyArrays(MyArray2d *array1, MyArray2d *array2, Double_t scale1, Double_t scale2){
//void MyArray2d::MultiplyArrays(MyArray2d *array1, MyArray2d *array2, Double_t scale1, Double_t scale2){
    
    if(array1 && array2){
        if(array1->GetLengthX()==array2->GetLengthX() && array1->GetLengthY()==array2->GetLengthY()){
            SetSumValue(0.0);
            for(Int_t i=0; i<numX; i++){
                for(Int_t j=0; j<numY; j++){
                    theArray[globalIndex(i,j)] = scale1*array1->val(i, j)*scale2*array2->val(i,j);
                    IncrementSumValue(theArray[globalIndex(i,j)]);
                }
            }
        }
        else printf("ERROR in MyArray2d::DivideArrays(): Arrays are not the same length.\n");
        
    }
    else printf("ERROR in MyArray2d::DivideArrays(): Trying to add arrays but at least one pointer is null\n");
    
}

void MyArray2d::ScaleArrayValues(Double_t scaleFactor){
 
    SetSumValue(0.0);
    for(Int_t i=0; i<num && theArray; i++){
        theArray[i] *= scaleFactor;
        IncrementSumValue(theArray[i]);
    }
    
}

void MyArray2d::Fill(Double_t xVal, Double_t yVal, Double_t incVal){
 
    if(!bBinX || !bBinY){
        printf("\tWarning in MyArray2d::Fill() - array does not have limits defined.\n");
        return;
    }
    
    Int_t xBin, yBin;
    Bool_t bX = GetArrayIndexX(xVal,xBin);
    Bool_t bY = GetArrayIndexY(yVal,yBin);
    
    if(bX && bY) AddVal(xBin, yBin, incVal);
    
}

TH2F * MyArray2d::GetArrayHistogram(const Char_t *histName, Bool_t bAbs){
 
    if(gROOT->FindObject(histName)){
        delete gROOT->FindObject(histName);
    }
    
    TH2F *hist = 0;
    if(bAbs && bBinX && bBinY)
        hist = new TH2F(histName,histName,numX,xRange[0],xRange[1],numY,yRange[0],yRange[1]);
    else hist = new TH2F(histName,histName,numX,-0.5,numX-0.5,numY,-0.5,numY-0.5);
    
    for(Int_t ix=0; ix<numX; ix++){
        for(Int_t iy=0; iy<numY; iy++){
            hist->SetBinContent(ix+1,iy+1, theArray[globalIndex(ix, iy)]);
        }
    }
    
    return hist;
    
}

void MyArray2d::GetMatrixFromArray(TMatrixD *theMatrix){
    
    theMatrix->SetMatrixArray(theArray,"F");
    
}

MyArray2d * MyArray2d::GetArray(TH2 *histogram){
 
    MyArray2d *returnArray = 0;
    
    if(histogram){
        returnArray = new MyArray2d(histogram->GetNbinsX(),histogram->GetNbinsY(),
                                    histogram->GetXaxis()->GetXmin(),histogram->GetXaxis()->GetXmax(),
                                    histogram->GetYaxis()->GetXmin(),histogram->GetYaxis()->GetXmax());
        
        for(Int_t ix=1; ix<=histogram->GetNbinsX(); ix++){
            for(Int_t iy=1; iy<=histogram->GetNbinsY(); iy++){
                returnArray->SetVal(ix-1, iy-1, histogram->GetBinContent(ix,iy));
            }
        }
    }
    
    return returnArray;
    
}

MyArray2d * MyArray2d::GetContentArray(Int_t bins, Double_t xLo, Double_t xHi){
    
    MyArray2d *returnArray = new MyArray2d(bins,1,xLo,xHi,-0.5,0.5);
    
    for(Int_t ix=0; ix<GetLengthX(); ix++){
        for(Int_t iy=0; iy<GetLengthY(); iy++){
            Int_t ibin;
            returnArray->GetArrayIndexX(val(ix,iy),ibin);
            returnArray->AddVal(ibin, 0);
        }
    }
    
    return returnArray;
    
}

MyArray2d * MyArray2d::GetContentArray(Int_t bins, Double_t axisPaddingFraction){

    Double_t xLo, xHi;
    Double_t zmin = GetMinimum();
    Double_t zmax = GetMaximum();
    if( TMath::Abs(zmax-zmin) < 1e5 ){
        xLo = zmin - 1.0;
        xHi = zmax + 1.0;
    }
    else{
        xLo = zmin - axisPaddingFraction*(zmax-zmin);
        xHi = zmax + axisPaddingFraction*(zmax-zmin);
    }
    
    return GetContentArray(bins, xLo, xHi);
    
}


Bool_t MyArray2d::GetArrayIndex(Double_t testVal, Bool_t bAxis, Int_t &rBin){
    
    Int_t numBins;
    Double_t rangeLow, binSize;
    if(bAxis){
        if(!bBinX) return false;
        numBins = numX;
        rangeLow = xRange[0];
        binSize = binSizeX;
    }
    else{
        if(!bBinY) return false;
        numBins = numY;
        rangeLow = yRange[0];
        binSize = binSizeY;
    }
    
    for(Int_t ibin=0; ibin<numBins; ibin++){
        if(testVal >= rangeLow + double(ibin)*binSize && testVal <= rangeLow + double(ibin+1)*binSize){
            rBin = ibin;
            return true;
        }
    }
    
    return false;
    
}

Double_t MyArray2d::GetBinPosition(Int_t iBin, Bool_t bAxis, Bool_t bAllowOutsideBounds){
    
    if(bAxis){
        if(!bAllowOutsideBounds && (iBin<0 || iBin>=numX)) return 0.0;
        else return xRange[0] + (double(iBin)+0.5)*binSizeX;
    }
    else{
        if(!bAllowOutsideBounds && (iBin<0 || iBin>=numY)) return 0.0;
        else return yRange[0] + (double(iBin)+0.5)*binSizeY;
    }
}

Double_t MyArray2d::GetQuantity(Int_t iCase, Double_t relVal){
    
    Double_t max = -9e9;
    Double_t min = 9e9;
    Double_t mean = 0.0;
    Double_t stdDev = 0.0;
    for(Int_t i=0; i<num && theArray; i++){
        if(theArray[i] > max) max = theArray[i];
        if(theArray[i] < min) min = theArray[i];
        mean += theArray[i];
        stdDev += pow(theArray[i]-relVal,2.0);
    }
    
    if(num>0){
        mean /= double(num);
        stdDev = TMath::Sqrt(stdDev/double(num));
    }
    
    if(iCase==0) return max;
    else if(iCase==1) return min;
    else if(iCase==2) return mean;
    else if(iCase==3){
        if(TMath::Abs(mean-relVal) > 1e-9){
            printf("\tWarning in MyArray2d::GetQuantity(3,%f) - Std. Dev. calculated using a mean of %f, but calculated mean is %f\n",relVal,relVal,mean);
        }
        return stdDev;
    }
    else return 0.0;
    
}

Double_t MyArray2d::GetStdDev(){

    Double_t mean = GetMean();
    return GetQuantity(3,mean);
    
}

Double_t MyArray2d::AbsIntegral(){
    
    Double_t sum = 0.0;
    for(Int_t i=0; i<num && theArray; i++){
        sum += TMath::Abs(theArray[i]);
    }
    
    return sum;
}

void MyArray2d::GetMaximumIndexXY(Int_t &maxIndexX, Int_t &maxIndexY){
    
    Double_t max = -99999.0;
    for(Int_t ix=0; ix<numX; ix++){
        for(Int_t iy=0; iy<numY; iy++){
            if(val(ix,iy) > max){
                max = val(ix,iy);
                maxIndexX = ix;
                maxIndexY = iy;
            }
        }
    }
    
}

Bool_t MyArray2d::RehistogramArrayByInterpolation(MyArray2d *arrayOld, Double_t acceptanceRadius){
    
    if(!arrayOld->GetBinningStatus()){
        printf("Error in MyArray2d::RehistogramArrayByArea() - original array does not have absolute bin limits set up\n");
        return false;
    }
    
    Bool_t bFixRange = false;
    if(!GetBinningStatus()){
        bFixRange = true;
        Double_t workingDx = arrayOld->GetBinPositionX(arrayOld->GetLengthX()-1) - arrayOld->GetBinPositionX(0);
        SetRangeX(arrayOld->GetBinPositionX(0)-0.5*workingDx/double(numX-1),
                  arrayOld->GetBinPositionX(arrayOld->GetLengthX()-1)+0.5*workingDx/double(numX-1));
        Double_t workingDy = arrayOld->GetBinPositionY(arrayOld->GetLengthY()-1) - arrayOld->GetBinPositionY(0);
        SetRangeY(arrayOld->GetBinPositionY(0)-0.5*workingDy/double(numY-1),
                  arrayOld->GetBinPositionY(arrayOld->GetLengthY()-1)+0.5*workingDy/double(numY-1));
    }
    
    Bool_t bRadius = false;
    if(acceptanceRadius > 0.0){
        if( (GetLowX() + GetHighX() > 0.001) || (GetLowY() + GetHighY() > 0.001) ){
            printf("Warning in MyArray2d::RehistogramArrayByArea() - radial cut requested, but method\n");
            printf("\tnot set up to handle circle not centered at (0,0). Request ignored.\n");
        }
        else bRadius = true;
    }

    Double_t origBinSizeX = arrayOld->GetBinSizeX();
    Double_t origBinSizeY = arrayOld->GetBinSizeY();
    
    for(Int_t ix=0; ix<GetLengthX(); ix++){
        
        Double_t xLoc = GetBinPositionX(ix);
        
        Double_t dx0 = xLoc - (arrayOld->GetLowX() + origBinSizeX/2.0);
        Int_t binDX[2] = { TMath::FloorNint( dx0/origBinSizeX ), TMath::CeilNint( dx0/origBinSizeX ) };
        Bool_t bX[2] = {false,false};
        
        for(Int_t ii=0; ii<2; ii++)
            if(binDX[ii] >= 0 && binDX[ii] < arrayOld->GetLengthX()) bX[ii] = true;
        
        for(Int_t iy=0; iy<GetLengthY(); iy++){
    
            Double_t yLoc = GetBinPositionY(iy);

            Double_t dy0 = yLoc - (arrayOld->GetLowY() + origBinSizeY/2.0);
            Int_t binDY[2] = { TMath::FloorNint( dy0/origBinSizeY ), TMath::CeilNint( dy0/origBinSizeY ) };

            Bool_t bY[2] = {false,false};
            for(Int_t ii=0; ii<2; ii++)
                if(binDY[ii] >= 0 && binDY[ii] < arrayOld->GetLengthY()) bY[ii] = true;
           

            Double_t val0[2][2];
            Double_t x0[2],y0[2];
            for(Int_t ii=0; ii<2; ii++){
                x0[ii] = (arrayOld->GetLowX() + (binDX[ii]+0.5)*origBinSizeX);
                for(Int_t jj=0; jj<2; jj++){
                    val0[ii][jj] = 0.0;
                    if(bX[ii] && bY[jj]){
                        val0[ii][jj] = arrayOld->val(binDX[ii],binDY[jj]);
                    }
                    
                    if(ii==0) y0[jj] = (arrayOld->GetLowY() + (binDY[jj]+0.5)*origBinSizeY);

                    if(bRadius){
                        Double_t rad = TMath::Sqrt( pow(x0[ii],2) + pow(y0[jj],2) );
                        if(rad>acceptanceRadius) val0[ii][jj] = 0.0;
                    }
                }
            }

            Double_t xWeightedVal[2] = {0,0};
            for(Int_t jj=0; jj<2; jj++){
                if(binDX[0]!=binDX[1]){
                    if(bX[0] && bX[1])
                        xWeightedVal[jj] = ((x0[1]-xLoc)*val0[0][jj] + (xLoc-x0[0])*val0[1][jj])/origBinSizeX;
                    else if(bX[0] && !bX[1])
                        xWeightedVal[jj] = val0[0][jj];
                    else
                        xWeightedVal[jj] = val0[1][jj];
                }
                else xWeightedVal[jj] = val0[0][jj];
            }
            
            Double_t newVal = 0.0;
            if(binDY[0]!=binDY[1]){
                if(bY[0] && bY[1])
                    newVal = ((y0[1]-yLoc)*xWeightedVal[0] + (yLoc-y0[0])*xWeightedVal[1])/origBinSizeY;
                else if(bY[0] && !bY[1])
                    newVal = xWeightedVal[0];
                else
                    newVal = xWeightedVal[1];
            }
            else newVal = xWeightedVal[0];
            
            SetVal(ix, iy, newVal);
        }
    }

    if(bFixRange){
        SetRangeX(arrayOld->GetLowX(),arrayOld->GetHighX());
        SetRangeY(arrayOld->GetLowY(),arrayOld->GetHighY());
    }
    
    return true;

    
}

Bool_t MyArray2d::RehistogramArrayByArea(MyArray2d *arrayOld, Bool_t bCyclic, Double_t acceptanceRadius){
        
    if(!arrayOld->GetBinningStatus()){
        printf("Error in MyArray2d::RehistogramArrayByArea() - original array does not have absolute bin limits set up\n");
        return false;
    }

    if(!GetBinningStatus()){
        printf("Error in MyArray2d::RehistogramArrayByArea() - new array does not have absolute bin limits set up\n");
        return false;
    }

    //new array values
    Double_t lowerX = GetLowX();
    Double_t lowerY = GetLowY();
	
    Double_t binWx = GetBinSizeX();
    Double_t binWy = GetBinSizeY();
    
    //old array values
    Double_t origBinWx = arrayOld->GetBinSizeX();
    Double_t origBinWy = arrayOld->GetBinSizeY();
    Double_t binArea = origBinWx*origBinWy;
    
    if(binArea<=0.0) return false;
        
    Double_t remainder[4], extraRange[4], binW[4] = {binWx,binWx,binWy,binWy}; 
    Int_t extraBins[4];
        
    remainder[0] = GetLowX()-arrayOld->GetLowX();
    remainder[1] = arrayOld->GetHighX()-GetHighX();
    remainder[2] = GetLowY()-arrayOld->GetLowY();
    remainder[3] = arrayOld->GetHighY()-GetHighY();
    
    for(Int_t ii=0; ii<4; ii++){
        extraBins[ii] = TMath::FloorNint(remainder[ii]/binW[ii]);
        if(remainder[ii] < 0.0 || !bCyclic){
            extraBins[ii] = 0;
            extraRange[ii] = 0.0;
        }
        else extraRange[ii] = extraBins[ii]*binW[ii];        
    }
    
    MyArray2d indexScaleFactors = MyArray2d(numX,numY);
    
    Bool_t bRadius = false;
    if(acceptanceRadius > 0.0){
        if( (GetLowX() + GetHighX() > 0.001) || (GetLowY() + GetHighY() > 0.001) ){
            printf("Warning in MyArray2d::RehistogramArrayByArea() - radial cut requested, but method\n");
            printf("\tnot set up to handle circle not centered at (0,0). Request ignored.\n");
        }
        else bRadius = true;
    }
    
    for(Int_t iix=0; iix<numX+extraBins[0]+extraBins[1]; iix++){
        
        Bool_t bEdgeX = false;
        
        Double_t xlo = lowerX - extraRange[0] + binWx*iix;
        if(xlo < lowerX) bEdgeX = true;
        Double_t xhi = xlo + binWx;
        if(xhi > GetHighX()) bEdgeX = true;
        
        if(xlo < arrayOld->GetLowX()) xlo = arrayOld->GetLowX();
        if(xhi > arrayOld->GetHighX()) xhi = arrayOld->GetHighX();
        
        if(xlo > xhi) continue;
        
        Int_t loBinX, hiBinX;
        Bool_t bLoX = arrayOld->GetArrayIndex(xlo, true, loBinX);
        Bool_t bHiX = arrayOld->GetArrayIndex(xhi, true, hiBinX);
        
        if(loBinX > hiBinX) continue;
        
        if(!bLoX || !bHiX){
            printf("Warning in MyArray2d::RehistogramArrayByArea() - x bin not found\n");
            continue;
        }
        
        
        for(Int_t iiy=0; iiy<numY+extraBins[2]+extraBins[3]; iiy++){
            
            Bool_t bEdgeY = false;
            
            Double_t ylo = lowerY - extraRange[2] + binWy*iiy;
            if(ylo < lowerY) bEdgeY = true;
            Double_t yhi = ylo + binWy;
            if(yhi > GetHighY()) bEdgeY = true;
            
            if(ylo < arrayOld->GetLowY()) ylo = arrayOld->GetLowY();
            if(yhi > arrayOld->GetHighY()) yhi = arrayOld->GetHighY();
            
            if(ylo > yhi) continue;
            
            Int_t loBinY, hiBinY;
            Bool_t bLoY = arrayOld->GetArrayIndex(ylo, false, loBinY);
            Bool_t bHiY = arrayOld->GetArrayIndex(yhi, false, hiBinY);
            
            if(loBinY > hiBinY) continue;
            
            if(!bLoY || !bHiY){
                printf("Warning in MyArray2d::RehistogramArrayByArea() - y bin not found\n");
                continue;
            }
                        
            //radial requirement here
            if(bRadius){
                Double_t rad = TMath::Sqrt( pow(xlo,2) + pow(ylo,2) );
                if(rad>acceptanceRadius) continue;
                rad = TMath::Sqrt( pow(xlo,2) + pow(yhi,2) );
                if(rad>acceptanceRadius) continue;
                rad = TMath::Sqrt( pow(xhi,2) + pow(ylo,2) );
                if(rad>acceptanceRadius) continue;
                rad = TMath::Sqrt( pow(xhi,2) + pow(yhi,2) );
                if(rad>acceptanceRadius) continue;
            }
                    
            //assign the new value
            Double_t newVal = 0.0;
            for(Int_t ii=loBinX; ii<=hiBinX; ii++){
                Double_t xOrigLo = arrayOld->GetLowX() + ii*arrayOld->GetBinSizeX();
                if(ii==loBinX) xOrigLo = xlo;
                Double_t xOrigHi = arrayOld->GetLowX() + (ii+1)*arrayOld->GetBinSizeX();
                if(ii==hiBinX) xOrigHi = xhi;
                
                for(Int_t jj=loBinY; jj<=hiBinY; jj++){
                    Double_t yOrigLo = arrayOld->GetLowY() + jj*arrayOld->GetBinSizeY();
                    if(jj==loBinY) yOrigLo = ylo;
                    Double_t yOrigHi = arrayOld->GetLowY() + (jj+1)*arrayOld->GetBinSizeY();
                    if(jj==hiBinY) yOrigHi = yhi;
                    

                    Double_t binC = arrayOld->val(ii, jj);
                    Double_t fracArea = (yOrigHi-yOrigLo)*(xOrigHi-xOrigLo)/binArea;
                    newVal += binC*fracArea;

                }
            }
            
            Int_t xBinToInc = iix-extraBins[0];
            if(xBinToInc < 0) xBinToInc += numX;
            else if(xBinToInc >= numX) xBinToInc -= numX;
            Int_t yBinToInc = iiy-extraBins[2];
            if(yBinToInc < 0) yBinToInc += numY;
            else if(yBinToInc >= numY) yBinToInc -= numY;
            
            if(xBinToInc < 0 || xBinToInc >= numX || yBinToInc < 0 || yBinToInc >= numY){
                printf("Warning in MyArray2d::RehistogramArrayByArea() - cyclic index value not determined correctly.\n");
                printf("\tx index: %d (max is %d), y index: %d (max is %d)\n",xBinToInc,numX,yBinToInc,numY);
                continue;
            }
            
            AddVal(xBinToInc, yBinToInc, newVal);
            indexScaleFactors.AddVal(xBinToInc, yBinToInc);

        }
    }
    
    for(Int_t ix=0; ix<numX; ix++){
        for(Int_t iy=0; iy<numY; iy++){
            if(indexScaleFactors.val(ix, iy) > 0.0)
                SetVal(ix, iy, val(ix, iy)/indexScaleFactors.val(ix, iy));
            else
                SetVal(ix, iy, 0.0);
        }
    }
    
    return true;
}

Int_t MyArray2d::GetRebinnedContribution(Int_t theXbin, Int_t theYbin, MyArray2d *arrayOld,
                                         Int_t *binNumsX, Int_t *binNumsY, Double_t *binWeights,
                                         Bool_t bCyclic, Double_t acceptanceRadius){
    
    if(!arrayOld->GetBinningStatus()){
        printf("Error in MyArray2d::RehistogramArrayByArea() - original array does not have absolute bin limits set up\n");
        return false;
    }
    
    if(!GetBinningStatus()){
        printf("Error in MyArray2d::RehistogramArrayByArea() - new array does not have absolute bin limits set up\n");
        return false;
    }

    Double_t binPosX0 = arrayOld->GetBinPositionX(theXbin) - arrayOld->GetBinSizeX()/2.0;
    Double_t binPosX1 = binPosX0 + arrayOld->GetBinSizeX();
    Double_t binPosY0 = arrayOld->GetBinPositionY(theYbin) - arrayOld->GetBinSizeY()/2.0;
    Double_t binPosY1 = binPosY0 + arrayOld->GetBinSizeY();
 
    Bool_t bRadius = false;
    if(acceptanceRadius > 0.0){
        if( (GetLowX() + GetHighX() > 0.001) || (GetLowY() + GetHighY() > 0.001) ){
            printf("Warning in MyArray2d::RehistogramArrayByArea() - radial cut requested, but method\n");
            printf("\tnot set up to handle circle not centered at (0,0). Request ignored.\n");
        }
        else bRadius = true;
    }

    //radial requirement here
    if(bRadius){
        Double_t rad = TMath::Sqrt( pow(binPosX0,2) + pow(binPosY0,2) );
        if(rad>acceptanceRadius) return 0;
        rad = TMath::Sqrt( pow(binPosX0,2) + pow(binPosY1,2) );
        if(rad>acceptanceRadius) return 0;
        rad = TMath::Sqrt( pow(binPosX1,2) + pow(binPosY0,2) );
        if(rad>acceptanceRadius) return 0;
        rad = TMath::Sqrt( pow(binPosX1,2) + pow(binPosY1,2) );
        if(rad>acceptanceRadius) return 0;
    }
    
    Int_t totPixContributing = 0;
    
    Double_t cyclesLoX, cyclesHiX, cyclesLoY, cyclesHiY;
    Int_t binLoX = GetCycledIndexX(binPosX0,cyclesLoX);
    Int_t binHiX = GetCycledIndexX(binPosX1,cyclesHiX);
    Int_t binLoY = GetCycledIndexX(binPosY0,cyclesLoY);
    Int_t binHiY = GetCycledIndexX(binPosY1,cyclesHiY);
    
    for(Int_t ix=binLoX-cyclesLoX*numX; ix<=binHiX-cyclesHiX*numX; ix++){
        for(Int_t iy=binLoY-cyclesLoY*numY; iy<=binHiY-cyclesHiY*numY; iy++){
            
            Double_t frac = GetFractionalCoverage(ix, iy, arrayOld, theXbin, theYbin);
            if(totPixContributing<MAX_REBIN_MAP_PIX){
                binNumsX[totPixContributing] = GetCycledIndexX(ix);
                binNumsY[totPixContributing] = GetCycledIndexY(iy);
                binWeights[totPixContributing] = frac;
                totPixContributing++;
            }
            else{
                printf("\tError in MyArray2d::GetRebinnedContribution() - bin %d, %d of original array overlaps with too many rebinned bins\n",theXbin,theYbin);
                printf("\t\tMaximum number of bins to be stored is MAX_REBIN_MAP_PIX = %d\n\n",MAX_REBIN_MAP_PIX);
            }
        }
    }
    
    return totPixContributing;
}

Double_t MyArray2d::GetFractionalCoverage(Int_t iThisX, Int_t iThisY, MyArray2d *arrayOld, Int_t iOldX, Int_t iOldY){
 
    Double_t frac = 0.0;
    
    Double_t oBinPosX0 = arrayOld->GetBinPositionX(iOldX,true) - arrayOld->GetBinSizeX()/2.0;
    Double_t oBinPosX1 = oBinPosX0 + arrayOld->GetBinSizeX();
    Double_t oBinPosY0 = arrayOld->GetBinPositionY(iOldY,true) - arrayOld->GetBinSizeY()/2.0;
    Double_t oBinPosY1 = oBinPosY0 + arrayOld->GetBinSizeY();
    
    Double_t binPosX0 = GetBinPositionX(iThisX,true) - GetBinSizeX()/2.0;
    Double_t binPosX1 = binPosX0 + GetBinSizeX();
    Double_t binPosY0 = GetBinPositionY(iThisY,true) - GetBinSizeY()/2.0;
    Double_t binPosY1 = binPosY0 + GetBinSizeY();

    if(oBinPosX0 < binPosX0) oBinPosX0 = binPosX0;
    if(oBinPosX1 > binPosX1) oBinPosX1 = binPosX1;
    if(oBinPosY0 < binPosY0) oBinPosY0 = binPosY0;
    if(oBinPosY1 > binPosY1) oBinPosY1 = binPosY1;

    frac = (oBinPosX1-oBinPosX0)*(oBinPosY1-oBinPosY0)/(arrayOld->GetBinSizeX()*arrayOld->GetBinSizeY());
    
    return frac;
    
}

void MyArray2d::GetRandomBoundaries(){
    
    Double_t absInt = AbsIntegral();
    if(absInt < 1e-9){
        printf("\tWarning in MyArray2d::GetRandomBoundaries() - cannot set up random boundaries for empty array\n");
        bRandomSetup = false;
        return;
    }
    
    randomBoundaries = new Double_t[num];
   
    Double_t runningSum = 0.0;
    for(Int_t i=0; i<num; i++){
        randomBoundaries[i] = runningSum/absInt;
        runningSum += TMath::Abs(theArray[i]);
    }
    
    bRandomSetup = true;
    
}

Bool_t MyArray2d::GetRandom(Double_t &xRand, Double_t &yRand){
    
    if(!bRandomSetup) GetRandomBoundaries();
    
    if(!bRandomSetup){
        printf("\tWarning in MyArray2d::GetRandom() - problem setting up random boundaries\n");
        return false;
    }
    
    Double_t r1 = gRandom->Rndm();
    Int_t ibin = TMath::BinarySearch(num,randomBoundaries,r1);

    Bool_t bSign = true;
    if(theArray[ibin] < 0.0) bSign = false;
    
    Int_t binX, binY;
    IndexXY(ibin, binX, binY);
    
    Double_t binUpperEdge = 1.0;
    if(binX < numX-1) binUpperEdge = randomBoundaries[ibin+1];
    xRand = GetBinPositionX(binX) + GetBinSizeX()*((r1-randomBoundaries[ibin])/(binUpperEdge-randomBoundaries[ibin])-0.5);
    yRand = GetBinPositionY(binY) + GetBinSizeY()*(gRandom->Rndm()-0.5);
        
    return bSign;
    
}

