#ifndef __NCALUTMAKER_H__
#define __NCALUTMAKER_H__

#include "TH1.h"
#include "TH2.h"
#include "TF2.h"
#include "TObject.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TMarker.h"
#include "TMath.h"
#include "TBox.h"
#include "TEllipse.h"
#include "TLatex.h"
#include "TLine.h"

namespace nca {

class LUTmaker : public TObject
{
public:
    LUTmaker(TH2I *hF, const Int_t numPixels,const Int_t tpixelsPerRow = -1);
    ~LUTmaker();
             
	TH2I * GetFlood() {return hFlood;}                 //returns original flood image
	TH2I * GetModifiedFlood() {return hFloodModified;} //returns modified (rebinned/smoothed) flood image
	TH2I * GetLUTHistogram() {return hLUT;}            //returns LUT histogram
	TH2I * GetHitPattern() {return hHitPattern;}       //return the pixel hit pattern

	Int_t GetNumberPixels() {return nPixels;}          //returns number of pixels in the detector
	
	TCanvas * GetCanvas() {return c1;}                 //return the canvas
    
    //Set and Get various user controlled variables
    void SetSearchSpacing(Double_t dValue);          //set spacing between pixels in units of histogram bins
    Double_t GetSearchSpacing() {return spacing;}    //return spacing between pixels in units of histograms bins
    void SetRebin(Bool_t bValue = true);             //set whether or not to rebin the flood
    Bool_t GetRebinStatus() {return bRebin;}         //return bool which determines whether the flood is rebinned
    void SetRebinGrouping(Int_t iValue);             //set the number of bins to group for rebinning
    Int_t GetRebinGrouping() {return nrebin;}        //return the value for the number of bins to group for rebinning
    void SetSmooth(Bool_t bValue = true);            //set whether or not to smooth the flood
    Bool_t GetSmoothStatus() {return bSmooth;}       //return bool which determines whether or not the flood is smoothed

    void SetFitRange(Double_t dValue);                            //set the range (half width of box) over which to fit a pixel
    Double_t GetFitRange() {return fitRange;}                     //return the range (half width of box) over which to fit a pixel
    void SetNominalWidth(Double_t dValue);                        //set the seed value for the pixel width
    Double_t GetNominalWidth() {return nomWidth;}                 //return the seed value for the pixel width
    void SetWidthMaximum(Double_t dValue);                        //set the upper limit on what the pixel width fit may return
    Double_t GetWidthMaximum() {return widthMax;}                 //return the upper limit on what the pixel width fit may return
    void SetWidthMinimum(Double_t dValue);                        //set the lower limit on what the pixel width fit may return
    Double_t GetWidthMinimum() {return widthMin;}                 //return the lower limit on what the pixel width fit may return
    void SetWidthRange(Double_t dValueLow, Double_t dValueHigh);  //set the (lower,upper) limits on the pixel width for the fits
    void SetHighlightGaus(Bool_t bValue = true);                  //set flag for highlighting the fitted gaussian for each pixel
    Bool_t GetHighlightGaus() {return bHighlightGaus;}            //return flag for highlighting the fitted gaussian for each pixel
    void SetHighlightFitRegion(Bool_t bValue = true);             //set flag for drawing a box that highlights the fit region for each pixel
    Bool_t GetHighlightFitRegion() {return bHighlightFitRegion;}  //return flag for drawing a box that highlights the fit region for each pixel

    void SetPixelsPerRow(Int_t iValue){pixelsPerRow = iValue;}                           //set the number of pixels per row
    Int_t GetPixelsPerRow() {return pixelsPerRow;}                //return the value for the number of pixels per row

    Bool_t GetLUTstatus() {return bLUTstatus;}                    // return whether or not the LUT has been created

    Double_t* GetFinalX() { return xPix; }
    Double_t* GetFinalY() { return yPix; }
    
    //Zeroing values
    void ZeroSeeds(void);                                      // zero the seed values for pixel centers
    void ZeroFitPix(void);                                     // zero the fit values for pixel centers
    void ZeroFinalPix(void);                                   // zero the final values for pixel centers

    //Operations
    void PixelSearch(Bool_t bGather=true);                     // search for the pixel centers for seed values
    void GatherSeedValues(Bool_t bDump=false);                 // gather the seed values from the canvas
    void GatherFitValues(Bool_t bDump=false);                  // gather the fitted values from the canvas
    void FitPixels(Bool_t bGather=true, Bool_t bPause=false);  // fit the pixel centers with a 2D gaussian
    void SortPixels(void);                                     // sort the pixels by row
    void CreateLUT(void);                                      // create the LUT histogram
    void CreateHitPattern(void);                               // create hit pattern histogram
    void Help(void);                                           // method that simply dumps the standard procedure for creating a LUT
    
    //Drawing
    void OverlayLUT(void);                                           //overlay the LUT on the current plot
    void DrawFinalMarkers(void);                                     //draw the final markers on the current plot
    void DrawPixelNumbers(void);                                     //draw the pixel numbers on the current plot
    void DrawFlood(Bool_t bLabel=false, TString opt="col");          //Draw original flood
    void DrawModifiedFlood(Bool_t bLabel=false, TString opt="col");  //Draw modified (rebinned/smoothed) flood
	void DrawLUThistogram(Bool_t bLabel=false, TString opt="col");   //Draw LUT in histogram form

private:
	
	const Int_t nPixels;
    
	TH2I  *hFlood;               //original flood image
	TH2I *hFloodModified;       //modified flood image (rebinned or smoothed)
	TH2I *hLUT;                 //final LUT histogram
	TH2I *hHitPattern;          //pixel hit pattern histogram
    
	TCanvas *c1;                //canvas on which everything is drawn
	
	Double_t *xSeed, *ySeed;    //x and y seed values of pixel locations
	Double_t *xFit, *yFit;      //x and y values of fit pixel locations
    Double_t *xPix, *yPix;      //x and y values of final pixel locations

    Bool_t bSearchPerformed;    //used to tell whether pixel search has been performed yet
    Bool_t bSeedsDefined;       //used to tell whether seed values have been defined yet
	Bool_t bFitsPerformed;      //used to tell whether fits have been performed
    Bool_t bFitPointsDefined;   //used to tell whether fit values have been determined yet
    Bool_t bPixSorted;          //used to tell whether pixels have been sorted
    Bool_t bLUTstatus;          //used to tell whether or not LUT has been created
    
    TMarker **pixelMarkerSeed;  //markers holding the x & y values of seed locations
    TMarker **pixelMarker;      //markers holding the x & y values of fit locations
	
    Double_t spacing;           // minimum spacing between pixel centers in units of bins
	Bool_t bRebin;              // true=rebin flood, false=don't rebin
	Int_t nrebin;               // grouping factor for rebinning
	Bool_t bSmooth;             // true=smooth histogram, false=don't smooth

    Double_t fitRange;          // range over which gaus fit will occur
	Double_t nomWidth;          // seed value for gaussian width in both directions
	Double_t widthMin;          // minimum limit of gaussian width in fit
	Double_t widthMax;          // maximum limit of gaussian width in fit
	Bool_t bHighlightGaus;      // highlight the 1 sigma value of the gaussian fit for the pixels
	Bool_t bHighlightFitRegion; // highlight the region used in the gaussian fit

    Int_t pixelsPerRow;         // number of pixels per row
    
    void PrepareFlood(void);    //smooth or rebin the flood
    
    ClassDef(LUTmaker,2)
};

}//    namespace nca
#endif // __NCALUTMAKER_H__

