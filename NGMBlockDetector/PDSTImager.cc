#include "PDSTImager.h"
#include "TStopwatch.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "NGMSystemConfiguration.h"
#include "NGMHit.h"
#include <iostream>
#include "NGMBlockMapping.h"
#include "NGMBlockPSDMaker.h"
#include "TFile.h"
#include "TList.h"

ClassImp(PDSTImager)
//////////////// PDSTIMager Implementation ////////////////
PDSTImager::PDSTImager(Int_t maskRank, Double_t maskDetectorDistance, Double_t sourceMaskDistance, Double_t maskElementSize,
                   Int_t numberPixelsX, Int_t numberPixelsY, TTree* maskTree, TTree* antiTree, TTree* voidTree)
:_imaker(maskRank,maskDetectorDistance,sourceMaskDistance,maskElementSize,numberPixelsX,numberPixelsY),
_maskTree(maskTree),
_antiTree(antiTree),
_voidTree(voidTree),
histList(0)
{
    
}

PDSTImager::PDSTImager(Int_t maskRank, Double_t maskElementSize, Double_t maskDetectorDistance, Double_t sourceMaskDistance,const char* imagerName, TTree* maskTree, TTree* antiTree, TTree* voidTree)
:_imaker(maskRank,maskElementSize,maskDetectorDistance,sourceMaskDistance,imagerName),
_maskTree(maskTree),
_antiTree(antiTree),
_voidTree(voidTree),
histList(0)
{
    
}
PDSTImager::PDSTImager()
:
//_imaker(19,100.0,100.0,1.4,40,40),
_imaker(19,1.2,100.0,115.5,"P40"),
_maskTree(0),
_antiTree(0),
_voidTree(0),
histList(0)
{

}

void PDSTImager::ScaleHistos(TList* histList, TH2* hScale)
{
    TIter itr(histList);
    TObject* obj;
    
    while( ( obj=itr() ) )
    {
        TH2* hTmp = dynamic_cast<TH2*>(obj);
        hTmp->Multiply(hScale);
    }
}

TH2* PDSTImager::CreateDetectorLiveTimeScaling(std::vector<double> livetime, double maxlivetime)
{
    TH2* hScale = 0;
    int nPixelsX=40,nPixelsY=40;
    hScale = new TH2D("hScale","hScale",
                      nPixelsX,0.0,nPixelsX,nPixelsY,0.0,nPixelsY);
    
    hScale->SetDirectory(0);
    
    NGMBlockMapping bmap;
    bmap.Init(_conf);
    int detSeq, localRow,localCol;
    for(int gpix = 0; gpix<nPixelsX*nPixelsY; gpix++)
    {
        bmap.GlobalPixelToLocalPixel(gpix, detSeq,localRow,localCol);
        hScale->SetBinContent(gpix%nPixelsX+1,gpix/nPixelsY+1,
                              maxlivetime/livetime[detSeq]
                              );
    }
    
    return hScale;
}

TH2* PDSTImager::CreateVoidFromMaskAnti(TH2* mask, TH2* anti)
{
    TH2* hVoid = 0;
    if(!mask || !anti)
    {
        return hVoid;
    }
    
    hVoid =dynamic_cast<TH2*>(mask->Clone(TString(mask->GetName())+"_void"));
    hVoid->Add(anti);
    hVoid->Scale((hVoid->GetNbinsX()*hVoid->GetNbinsY())/(hVoid->Integral()));
    mask->Divide(hVoid);
    anti->Divide(hVoid);
    
    return hVoid;
}

void PDSTImager::go()
{
    
    if(!_maskTree||!_antiTree) return;
    
    PDSTImagerSelector* maskSelector = new PDSTImagerSelector();
    PDSTImagerSelector* antiSelector = new PDSTImagerSelector();

    if(!histList)  histList = new TList();
    //histList->Clear();
    
    TList* maskList = new TList();
    maskList->Add(_conf);
    maskList->Add( new TNamed("MaskAntiVoid","Mask"));
    maskSelector->SetInputList(maskList);
    
    TList* antiList = new TList();
    antiList->Add(_conf);
    antiList->Add( new TNamed("MaskAntiVoid","Anti"));
    antiSelector->SetInputList(antiList);
    
    _maskTree->Process(maskSelector);
    maskSelector->GetOutputList()->Print();
    _antiTree->Process(antiSelector);
    antiSelector->GetOutputList()->Print();
    
    std::vector<double>::iterator biggestMaskLivetime = std::max_element(masklivetime.begin(), masklivetime.end());
    std::vector<double>::iterator biggestAntiLivetime = std::max_element(antilivetime.begin(), antilivetime.end());
    double maxlivetime = TMath::Max(*biggestMaskLivetime,*biggestAntiLivetime);
    Info("go","MaxLiveTimes %f %f",*biggestMaskLivetime,*biggestAntiLivetime);
    TH2* hMaskScale = CreateDetectorLiveTimeScaling(masklivetime, maxlivetime);
    TH2* hAntiScale = CreateDetectorLiveTimeScaling(antilivetime, maxlivetime);
    new TCanvas();
    hMaskScale->DrawCopy("TEXT");
    new TCanvas();
    hAntiScale->DrawCopy("TEXT");
    //Lets add our histograms to our output list
    ScaleHistos(maskSelector->GetOutputList(), hMaskScale);
    ScaleHistos(antiSelector->GetOutputList(), hAntiScale);
    
    histList->AddAll(maskSelector->GetOutputList());
    
    histList->AddAll(antiSelector->GetOutputList());
    
    //delete maskSelector;
    //delete antiSelector;
}

void PDSTImager::SetSourceDistance(Double_t newSourceDist)
{
    _imaker.SetDistances(_imaker.GetDetectorMaskDistance(),newSourceDist);

}

ClassImp(PDSTImagerSelector)
//////////////// PDSTIMager Implementation ////////////////

Int_t   PDSTImagerSelector::GetEntry(Long64_t entry, Int_t getall/* = 0*/)
{
    return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0;
}

void PDSTImagerSelector::Init(TTree *tree)
{
    // The Init() function is called when the selector needs to initialize
    // a new tree or chain. Typically here the branch addresses and branch
    // pointers of the tree will be set.
    // It is normally not necessary to make changes to the generated
    // code, but the routine can be extended by the user if needed.
    // Init() will be called many times when running on PROOF
    // (once per file to be processed).
    
    // Set branch addresses and branch pointers
    if (!tree) return;
    fChain = tree;
    //fChain->SetMakeClass(1);
    fChain->SetBranchAddress("HitTree",&hit,&hit_br);
}

Bool_t PDSTImagerSelector::Notify()
{
    // The Notify() function is called when a new file is pened. This
    // can be either for a new TTree in a TChain or when when a new TTree
    // is started when using PROOF. It is normally not necessary to make changes
    // to the generated code, but the routine can be extended by the
    // user if needed. The return value is currently not used.
    
    return kTRUE;
}

void PDSTImagerSelector::Begin(TTree * /*tree*/)
{
    // The Begin() function is called at the start of the query.
    // When running with PROOF Begin() is only called on the client.
    // The tree argument is deprecated (on PROOF 0 is passed).
    
    TString option = GetOption();
    
}

void PDSTImagerSelector::SlaveBegin(TTree * /*tree*/)
{
    // The SlaveBegin() function is called after the Begin() function.
    // When running with PROOF SlaveBegin() is called on each slave server.
    // The tree argument is deprecated (on PROOF 0 is passed).
    
    TString option = GetOption();
    int nPixelsX=40,nPixelsY=40;
    if(GetInputList())
    {
        conf=dynamic_cast<NGMSystemConfiguration*>(GetInputList()->FindObject("PIXIE16"));
        if(!conf)
            Fatal(__FUNCTION__,"Cannot Find NGMSystemConfiguration !");
        maskAntiVoid = (dynamic_cast<TNamed*>(GetInputList()->FindObject("MaskAntiVoid")))->GetTitle();
        
    }
    bmap = new NGMBlockMapping();
    Info("SlaveBegin","Initializing Pixel Map");
    bmap->Init(conf);
    bpsd.clear();
    
    for(int idet = 0; idet < bmap->GetNBlocks(); idet++)
    {
        Info(__FUNCTION__,"Searching for %s \n",bmap->GetDetectorName(idet).c_str());
        NGMBlockPSD* tbpsd = dynamic_cast<NGMBlockPSD*>(conf->GetDetectorParameters()->FindFirstParValueAsObject("BLOCK_PSD", "DetectorName", bmap->GetDetectorName(idet).c_str()));
        if(!tbpsd)
            Fatal(__FUNCTION__,"Cannot Find %s !",bmap->GetDetectorName(idet).c_str());
        bpsd.push_back(tbpsd);
        Info("SlaveBegin","Found %s \n",tbpsd->GetName());
    }
    
    // enable bin errors:
    // Add to output list (needed for PROOF)
    
    fDetImage_Neutron = new TH2D("DetImage_Neutron_"+maskAntiVoid,
                                 "DetImage_Neutron_"+maskAntiVoid,
                                 nPixelsX,0.0,nPixelsX,nPixelsY,0.0,nPixelsY);
    fDetImage_Neutron_Tight = new TH2D("DetImage_Neutron_Tight_"+maskAntiVoid,
                                       "DetImage_Neutron_Tight_"+maskAntiVoid,
                                       nPixelsX,0.0,nPixelsX,nPixelsY,0.0,nPixelsY);
    fDetImage_Gamma = new TH2D("DetImage_Gamma_"+maskAntiVoid,
                               "DetImage_Gamma_"+maskAntiVoid,
                               nPixelsX,0.0,nPixelsX,nPixelsY,0.0,nPixelsY);
    
    GetOutputList()->Add(fDetImage_Neutron);
    GetOutputList()->Add(fDetImage_Neutron_Tight);
    GetOutputList()->Add(fDetImage_Gamma);
    
}

void PDSTImagerSelector::DoConf(const char* fname)
{
    TFile* fin = TFile::Open(fname);
    conf = dynamic_cast<NGMSystemConfiguration*>(fin->Get("NGMSystemConfiguration"));
    TList* inputList = new TList();
    inputList->Add(conf);
    SetInputList(inputList);
    fin->Close();
}

Bool_t PDSTImagerSelector::Process(Long64_t entry)
{
    // The Process() function is called for each entry in the tree (or possibly
    // keyed object in the case of PROOF) to be processed. The entry argument
    // specifies which entry in the currently loaded tree is to be processed.
    // It can be passed to either ImageSelector::GetEntry() or TBranch::GetEntry()
    // to read either all or the required parts of the data. When processing
    // keyed objects with PROOF, the object is already loaded and is available
    // via the fObject pointer.
    //
    // This function should contain the "body" of the analysis. It can contain
    // simple or elaborate selection criteria, run algorithms on the data
    // of the event and typically fill histograms.
    //
    // The processing can be stopped by calling Abort().
    //
    // Use fStatus to set the return value of TTree::Process().
    //
    // The return value is currently not used.
    GetEntry(entry);
    int gPix = hit->GetPixel();
    int pixelCol = gPix%40;
    int pixelRow = gPix/40;
    //int detCol = pixelCol/10;
    //int detRow = pixelRow/10;
    int detSeq = -1;//detCol+4*detRow;
    int localRow=0,localCol=0;
    detSeq = 0;
    bmap->GlobalPixelToLocalPixel(gPix,detSeq,localRow,localCol);
    double gsig = bpsd[detSeq]->GetGSigma(hit);
    double nsig = bpsd[detSeq]->GetNSigma(hit);
    // Gamma Image
    if( /*fabs(gsig) < 2.0 && */ hit->GetEnergy()>200.0 && hit->GetEnergy()<3000.0){
        fDetImage_Gamma->Fill(pixelCol,pixelRow);
    }
    
    if(gsig < -5.0 && fabs(nsig) < 2.0 ){
        fDetImage_Neutron->Fill(pixelCol,pixelRow);
    }
    
    if(gsig < -6.0 && fabs(nsig) < 2.0 ){
        fDetImage_Neutron_Tight->Fill(pixelCol,pixelRow);
    }
    
    return kTRUE;
}

void PDSTImagerSelector::SlaveTerminate()
{
    // The SlaveTerminate() function is called after all entries or objects
    // have been processed. When running with PROOF SlaveTerminate() is called
    // on each slave server.
    
}

void PDSTImagerSelector::Terminate()
{
    // The Terminate() function is the last function to be called during
    // a query. It always runs on the client, it can be used to present
    // the results graphically or save the results to file.
//    TString foutname="CfPSD.root";
//    if(conf) foutname.Form("Image%llu.root",conf->getRunNumber());
//    TFile* fout=TFile::Open(foutname.Data(),"RECREATE");
//    //fPSD_E->Draw();
//    fout->WriteTObject(fDetImage_Neutron,"fDetImage_Neutron");
//    fout->WriteTObject(conf,"NGMSystemConfiguration");
//    fout->Close();
}

