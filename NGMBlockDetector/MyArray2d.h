#ifndef __MyArray2d_H__
#define __MyArray2d_H__

#include "TObject.h"
#include "TROOT.h"
#include "TH2.h"
#include "TMath.h"
#include "TRandom.h"
#include "TMatrixD.h"

#include <iostream>

#define MAX_REBIN_MAP_PIX 10

class MyArray2d : public TObject
{
public:
    MyArray2d(Int_t nX, Int_t nY);
    MyArray2d(Int_t nX, Int_t nY,
              Double_t lowX, Double_t hiX,
              Double_t lowY, Double_t hiY);
    virtual ~MyArray2d();
        
    void Reset(Int_t nX, Int_t nY);
    void Reset(Int_t nX, Int_t nY,
               Double_t lowX, Double_t hiX,
               Double_t lowY, Double_t hiY);
    
    Double_t val(Int_t ix, Int_t iy);
    Double_t CycledVal(Int_t ix, Int_t iy);
    void SetVal(Int_t ix, Int_t iy, Double_t dVal);
    void AddVal(Int_t ix, Int_t iy, Double_t dVal=1.0);
    inline Int_t GetLengthX() {return numX;}
    inline Int_t GetLengthY() {return numY;}
    Double_t Integral() {return arraySum;}
    void ClearValues();
    void SetUniform(Double_t newVal=1.0);
    void CopyValues(MyArray2d *arrayToCopy);
    void AddArrays(MyArray2d *array1, MyArray2d *array2, Double_t scale1=1.0, Double_t scale2=1.0);
    void DivideArrays(MyArray2d *array1, MyArray2d *array2);
    void MultiplyArrays(MyArray2d *array1, MyArray2d *array2, Double_t scale1=1.0, Double_t scale2=1.0);
    void ScaleArrayValues(Double_t scaleFactor);
    void Fill(Double_t xVal, Double_t yVal, Double_t incVal=1.0);
    
    Double_t GetBinSizeX() {return binSizeX;}
    Double_t GetBinSizeY() {return binSizeY;}
    Double_t GetLowX() { return xRange[0];}
    Double_t GetLowY() { return yRange[0];}
    Double_t GetHighX() { return xRange[1];}
    Double_t GetHighY() { return yRange[1];}
    
    Bool_t GetBinningStatus() {return bBinX*bBinY;}
    
    void SetRange(Double_t lowLimit, Double_t hiLimit, Bool_t bAxis);
    void SetRangeX(Double_t lowLimit, Double_t hiLimit){ SetRange(lowLimit,hiLimit,true); }
    void SetRangeY(Double_t lowLimit, Double_t hiLimit){ SetRange(lowLimit,hiLimit,false); }
    
    void SetBinSize(Double_t sizeVal, Bool_t bAxis, Double_t lowLimit);
    void SetBinSizeX(Double_t sizeVal, Double_t lowLimit){ SetBinSize(sizeVal, true, lowLimit);}
    void SetBinSizeY(Double_t sizeVal, Double_t lowLimit){ SetBinSize(sizeVal, false, lowLimit);}
    
    Bool_t GetArrayIndex(Double_t testVal, Bool_t bAxis, Int_t &rBin);
    Bool_t GetArrayIndexX(Double_t testVal, Int_t &rBin) {return GetArrayIndex(testVal, true, rBin);}
    Bool_t GetArrayIndexY(Double_t testVal, Int_t &rBin) {return GetArrayIndex(testVal, false, rBin);}

    Int_t GetCycledIndex(Int_t i0, Bool_t bAxis);
    Int_t GetCycledIndexX(Int_t i0) {return GetCycledIndex(i0, true);}
    Int_t GetCycledIndexY(Int_t i0) {return GetCycledIndex(i0, false);}
    Int_t GetCycledIndex(Double_t d0, Bool_t bAxis, Double_t &cyclesMoved);
    Int_t GetCycledIndexX(Double_t d0, Double_t &cyclesMoved) {return GetCycledIndex(d0, true, cyclesMoved);}
    Int_t GetCycledIndexY(Double_t d0, Double_t &cyclesMoved) {return GetCycledIndex(d0, false, cyclesMoved);}
    
    Double_t GetBinPosition(Int_t iBin, Bool_t bAxis, Bool_t bAllowOutsideBounds=false);
    Double_t GetBinPositionX(Int_t iBin, Bool_t bAllowOutsideBounds=false) {return GetBinPosition(iBin, true, bAllowOutsideBounds);}
    Double_t GetBinPositionY(Int_t iBin, Bool_t bAllowOutsideBounds=false) {return GetBinPosition(iBin, false, bAllowOutsideBounds);}
    Double_t GetQuantity(Int_t iCase, Double_t relVal=0.0);
    Double_t GetMaximum() {return GetQuantity(0);}
    Double_t GetMinimum() {return GetQuantity(1);}
    Double_t GetMean() {return GetQuantity(2);}
    Double_t GetStdDev();
    void GetMaximumIndexXY(Int_t &maxIndexX, Int_t &maxIndexY);
    Bool_t GetRandom(Double_t &xRand, Double_t &yRand);
    
    Double_t AbsIntegral();
    Bool_t RehistogramArrayByArea(MyArray2d *arrayOld, Bool_t bCyclic=false, Double_t acceptanceRadius=-1.0);
    Bool_t RehistogramArrayByInterpolation(MyArray2d *arrayOld, Double_t acceptanceRadius=-1.0);
    Int_t GetRebinnedContribution(Int_t theXbin, Int_t theYbin, MyArray2d *arrayOld,
                                  Int_t *binNumsX, Int_t *binNumsY, Double_t *binWeights,
                                  Bool_t bCyclic=false, Double_t acceptanceRadius=-1.0);
    Double_t GetFractionalCoverage(Int_t iThisX, Int_t iThisY, MyArray2d *arrayOld, Int_t iOldX, Int_t iOldY);
    
    TH2F * GetArrayHistogram(const Char_t *histName, Bool_t bAbs=true);
    MyArray2d * GetContentArray(Int_t bins, Double_t xLo, Double_t xHi);
    MyArray2d * GetContentArray(Int_t bins, Double_t axisPaddingFraction=0.1);
    static MyArray2d * GetArray(TH2 *histogram);
    virtual MyArray2d * Clone(Bool_t bZeroValues=false);
    void GetMatrixFromArray(TMatrixD *theMatrix);
    
private:
	
    Int_t num,numX,numY;
    Int_t globalIndex(Int_t ix, Int_t iy);
    void IndexXY(Int_t index, Int_t &binX, Int_t &binY);
    Double_t *theArray;
    Double_t arraySum;
    
    Double_t xRange[2], yRange[2];
    Double_t binSizeX, binSizeY;
    Bool_t bBinX, bBinY, bRandomSetup;
    
    Double_t *randomBoundaries;
    void GetRandomBoundaries();
    void SetSumValue(Double_t newVal);
    void IncrementSumValue(Double_t incVal);
    
    ClassDef(MyArray2d,1);
    
}; //END of class MyArray2D


#endif
