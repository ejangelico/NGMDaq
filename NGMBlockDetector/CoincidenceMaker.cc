
#include "CoincidenceMaker.h"

#include "TFile.h"
#include "TH2F.h"
#include "TMath.h"
#include "TFolder.h"
#include "NGMSystemConfiguration.h"

using namespace std;
#include <iostream>

ClassImp(CoincidenceMaker)

CoincidenceMaker::CoincidenceMaker()
{
   
    //Not allowed for default constructor
    chain = 0;//new TChain("HitTree");

    coincTree = 0;

    theHit = 0;
    hit0 = 0;
    
    fout = 0;
    
    dtSlot = -1;
    dtChan = -1;
    liqSlot = -1;
    
    //Not allowed for default constructor
    processedEvent = 0;//new CoincidentEvent();
    
    bFirstPrint = false;
    bFirstDT = false;
    currentDTindex = 0;
}

CoincidenceMaker::~CoincidenceMaker(){
    
    if(fout) delete fout;
    
}

void CoincidenceMaker::AddFileToChain(TString fileName){
    if(!chain) chain = new TChain("HitTree");
    chain->AddFile(fileName.Data());
    printf("%s added to chain\n",fileName.Data());

}

void CoincidenceMaker::SetOutputFileName(TString ofname){
 
    fout = TFile::Open(ofname,"recreate");
    coincTree = new TTree("coincTree","coincidence tree");
    if(!processedEvent) processedEvent = new CoincidentEvent();
    coincTree->Branch("coincData","CoincidentEvent",&processedEvent);

    bFirstPrint = false;
    bFirstDT = false;
    currentDTindex = 0;

}

void CoincidenceMaker::SetDTSlotChannel(Int_t theSlot, Int_t theChan){
    
    dtSlot = theSlot;
    dtChan = theChan;
    
}

void CoincidenceMaker::SetLiqSlot(Int_t theSlot, Int_t nLiquids){
 
    liqSlot = theSlot;
    numLiq = nLiquids;
    
}

void CoincidenceMaker::ConstructCoincidences(){

    SetupHistograms();

    if(chain->GetEntries() > 0){
        printf("\nChain has %lld entries\n",chain->GetEntries());
        chain->SetBranchAddress("HitTree",&theHit);
        if(!coincTree){
            coincTree = new TTree("coincTree","coincidence tree");
            if(!processedEvent) processedEvent = new CoincidentEvent();
            coincTree->Branch("coincData","CoincidentEvent",&processedEvent);
        }
        DoOrientation();
        
        if(fout) coincTree->AutoSave();

        SaveHistograms();
    }
    

}

void CoincidenceMaker::SaveHistograms(){

    if(fout){
        printf("Output file: %s\n\n",fout->GetName());
        coincTree->AutoSave();
        fout->cd();
        
        for(Int_t i=0; i<numLiq; i++){
            hLiqImTimeDiff[i]->Write(Form("liq%02d",i));
            delete hLiqImTimeDiff[i];
        }
        delete [] hLiqImTimeDiff;
        
        hDTvsImTime[0]->Write("nTimeSinceDT");
        hDTvsImTime[1]->Write("tnTimeSinceDT");
        hDTvsImTime[2]->Write("gTimeSinceDT");
        hDTvsImTime[3]->Write("otherTimeSinceDT");
        
        for(Int_t i=0; i<PARTICLE_TYPES; i++) delete hDTvsImTime[i];
        
    }

}

void CoincidenceMaker::SetupHistograms(TFolder* saveFolder){
   
    hLiqImTimeDiff = new TH2F*[numLiq];
    hDTvsImTime[0] = new TH1F("hDTvsImTime[0]","Time after DT - Imager n Counts",10000,0,10000000);
    hDTvsImTime[1] = new TH1F("hDTvsImTime[1]","Time after DT - Imager th n Counts",10000,0,10000000);
    hDTvsImTime[2] = new TH1F("hDTvsImTime[2]","Time after DT - Imager g Counts",10000,0,10000000);
    hDTvsImTime[3] = new TH1F("hDTvsImTime[3]","Time after DT - Imager other Counts",10000,0,10000000);
    if(saveFolder){
        for(int i =0; i<4; i++){
            hDTvsImTime[i]->SetDirectory(0);
            saveFolder->Add(hDTvsImTime[i]);
        }
    }
 
    for(Int_t i=0; i<numLiq && fout; i++){
        hLiqImTimeDiff[i] = new TH2F(Form("hLiqImTimeDiff[%d]",i),Form("Liq - Im time difference - %d",i),
                                     TMath::Nint(2.0*halfWindowNanoSec),-halfWindowNanoSec,halfWindowNanoSec,4,0,4);
        if(saveFolder){
            hLiqImTimeDiff[i]->SetDirectory(0);
            saveFolder->Add(hLiqImTimeDiff[i]);
        }
    }

}

void CoincidenceMaker::processHit(const NGMHit* hit){
    if(hit0==0) hit0 = hit->DuplicateHit();
    if(hit->GetSlot() == dtSlot && hit->GetChannel() == dtChan){
        mostRecentDTtime = hit->GetNGMTime();
        dtTimes[currentDTindex%DT_PULSES_TO_SAVE] = mostRecentDTtime;
        currentDTindex = (currentDTindex+1)%DT_PULSES_TO_SAVE;
        bFirstDT = true;
    }
    else if(hit->GetSlot() == liqSlot) AddLiquidHit(hit);
    else{
        Int_t thePartStatus = GetParticleStatus(hit);
        hDTvsImTime[thePartStatus]->Fill( hit->TimeDiffNanoSec(mostRecentDTtime) );
        AddImagerHit(hit);
    }
    
    //if this condition is true, we have some liquid hits that can be erased
    if(_imagerHits.empty() && !_liqHits.empty() && hit->TimeDiffNanoSec( ((NGMHit*)_liqHits.front())->GetNGMTime() ) > halfWindowNanoSec){
        
        //go through liquid hits to see what is out of time window
        std::list<NGMHit*>::iterator it = _liqHits.begin();
        while (it != _liqHits.end())
        {
            Double_t tDiff = ((NGMHit*)_liqHits.back())->TimeDiffNanoSec(((NGMHit*)*it)->GetNGMTime());
            
            if(tDiff > halfWindowNanoSec){ delete (*it); _liqHits.erase(it++);}
            else break;
        }
    }
    
    //look for whether we're past the time window for the earliest imager hit still stored
    if(!_imagerHits.empty() && hit->TimeDiffNanoSec( ((NGMHit*)_imagerHits.front())->GetNGMTime() ) > halfWindowNanoSec ){
        
        NGMHit *imHit = (NGMHit*)_imagerHits.front();
        processedEvent->ClearData();
        
        Int_t iImagerPartID = GetParticleStatus(imHit);
        
        //go through liquid hits and compare to time for earliest imager hit
        std::list<NGMHit*>::iterator it = _liqHits.begin();
        if(iImagerPartID<3){
            
            processedEvent->ImagerHit->particleType = iImagerPartID;
            processedEvent->ImagerHit->pixelX = imHit->GetPixel()%numDetPix;
            processedEvent->ImagerHit->pixelY = imHit->GetPixel()/numDetPix;
            for(Int_t iDT=currentDTindex-1; iDT>currentDTindex-1-DT_PULSES_TO_SAVE; iDT--){
                Int_t iDTc = iDT;
                if(iDT<0) iDTc += DT_PULSES_TO_SAVE;
                Double_t tempDiff = imHit->TimeDiffNanoSec(dtTimes[iDTc]);
                if(tempDiff > 0){
                    processedEvent->ImagerHit->relativeTime = tempDiff;
                    break;
                }
            }
            processedEvent->ImagerHit->energy = imHit->GetEnergy();
            
            while (it != _liqHits.end() )
            {
                NGMHit *liqHit = (NGMHit*)*it;
                
                Double_t tDiff = liqHit->TimeDiffNanoSec(imHit->GetNGMTime());
                Int_t iLiquidPartID = GetParticleStatus(liqHit);
                
                //if liquid hit is before the earliest's imager hit window, we don't need it anymore
                //so remove it
                if(iLiquidPartID==3 || tDiff < -halfWindowNanoSec) { delete (*it); _liqHits.erase(it++);}
                else if( TMath::Abs(tDiff) <= halfWindowNanoSec){
                    
                    Int_t liqChn = liqHit->GetChannel();
                    if(liqChn >= 0 && liqChn < numLiq){
                        
                        processedEvent->AddLiqCoinc(iLiquidPartID, tDiff, liqHit->GetEnergy());
                        
                        if(iImagerPartID==0) processedEvent->numLiqNeutron++;
                        if(iImagerPartID==2) processedEvent->numLiqGamma++;
                        
                        if(iImagerPartID==0 && iLiquidPartID==0) hLiqImTimeDiff[liqChn]->Fill( tDiff, 0 ); //n-n
                        else if(iImagerPartID==0 && iLiquidPartID==2) hLiqImTimeDiff[liqChn]->Fill( tDiff, 1 ); //n-g
                        else if(iImagerPartID==2 && iLiquidPartID==0) hLiqImTimeDiff[liqChn]->Fill( tDiff, 2 ); //g-n
                        else if(iImagerPartID==2 && iLiquidPartID==2) hLiqImTimeDiff[liqChn]->Fill( tDiff, 3 ); //g-g
                    }
                    ++it;
                }
                else break;
            }
            
            coincTree->Fill();
            
        }
        
        delete _imagerHits.front();
        _imagerHits.pop_front();
        
    }
    
}

void CoincidenceMaker::DoOrientation(){
    

    if(!fout){
        printf("\nError in CoincidenceMaker::ConstructCoincidences(%f) - No output file name specified.\n\t Use CoincidenceMaker::SetOutputFileName(TString)\n\n",halfWindowNanoSec);
        exit(0);
    }
    
    bFirstPrint = false;
    bFirstDT = false;
    currentDTindex = 0;

    for(Int_t iEntry=0; iEntry<chain->GetEntries(); iEntry++){
    
        if(iEntry%100000==0){
            if(bFirstPrint) fflush(stdout);
            printf("\tTree entry %d of %lld    (%.3f)\r",iEntry,chain->GetEntries(),double(iEntry+1)/double(chain->GetEntries()));
            bFirstPrint = true;
        }
        
        chain->GetEntry(iEntry);
        
        if(iEntry==0) hit0 = theHit->DuplicateHit();
                
        if(theHit->GetSlot() == dtSlot && theHit->GetChannel() == dtChan){
            mostRecentDTtime = theHit->GetNGMTime();
            dtTimes[currentDTindex%DT_PULSES_TO_SAVE] = mostRecentDTtime;
            currentDTindex = (currentDTindex+1)%DT_PULSES_TO_SAVE;
            bFirstDT = true;
        }
        else if(theHit->GetSlot() == liqSlot) AddLiquidHit(theHit);
        else{
            Int_t thePartStatus = GetParticleStatus(theHit);
            hDTvsImTime[thePartStatus]->Fill( theHit->TimeDiffNanoSec(mostRecentDTtime) );
            AddImagerHit(theHit);
        }
        
        //if this condition is true, we have some liquid hits that can be erased
        if(_imagerHits.empty() && !_liqHits.empty() && theHit->TimeDiffNanoSec( ((NGMHit*)_liqHits.front())->GetNGMTime() ) > halfWindowNanoSec){
            
            //go through liquid hits to see what is out of time window
            std::list<NGMHit*>::iterator it = _liqHits.begin();
            while (it != _liqHits.end())
            {
                Double_t tDiff = ((NGMHit*)_liqHits.back())->TimeDiffNanoSec(((NGMHit*)*it)->GetNGMTime());
                
                if(tDiff > halfWindowNanoSec) { delete (*it); _liqHits.erase(it++);}
                else break;
            }
        }
        
        //look for whether we're past the time window for the earliest imager hit still stored
        if(!_imagerHits.empty() && theHit->TimeDiffNanoSec( ((NGMHit*)_imagerHits.front())->GetNGMTime() ) > halfWindowNanoSec ){
            
            NGMHit *imHit = (NGMHit*)_imagerHits.front();
            processedEvent->ClearData();
            
            Int_t iImagerPartID = GetParticleStatus(imHit);
            
            //go through liquid hits and compare to time for earliest imager hit
            std::list<NGMHit*>::iterator it = _liqHits.begin();
            if(iImagerPartID<3){
                
                processedEvent->ImagerHit->particleType = iImagerPartID;
                processedEvent->ImagerHit->pixelX = imHit->GetPixel()%numDetPix;
                processedEvent->ImagerHit->pixelY = imHit->GetPixel()/numDetPix;
                for(Int_t iDT=currentDTindex-1; iDT>currentDTindex-1-DT_PULSES_TO_SAVE; iDT--){
                    Int_t iDTc = iDT;
                    if(iDT<0) iDTc += DT_PULSES_TO_SAVE;
                    Double_t tempDiff = imHit->TimeDiffNanoSec(dtTimes[iDTc]);
                    if(tempDiff > 0){
                        processedEvent->ImagerHit->relativeTime = tempDiff;
                        break;
                    }
                }
                processedEvent->ImagerHit->energy = imHit->GetEnergy();

                while (it != _liqHits.end() )
                {
                    NGMHit *liqHit = (NGMHit*)*it;
                    
                    Double_t tDiff = liqHit->TimeDiffNanoSec(imHit->GetNGMTime());
                    Int_t iLiquidPartID = GetParticleStatus(liqHit);
                    
                    //if liquid hit is before the earliest's imager hit window, we don't need it anymore
                    //so remove it
                    if(iLiquidPartID==3 || tDiff < -halfWindowNanoSec) {  delete (*it);_liqHits.erase(it++); }
                    else if( TMath::Abs(tDiff) <= halfWindowNanoSec){
                        
                        Int_t liqChn = liqHit->GetChannel();
                        if(liqChn >= 0 && liqChn < numLiq){
                        
                            processedEvent->AddLiqCoinc(iLiquidPartID, tDiff, liqHit->GetEnergy());

                            if(iImagerPartID==0) processedEvent->numLiqNeutron++;
                            if(iImagerPartID==2) processedEvent->numLiqGamma++;
                            
                            if(iImagerPartID==0 && iLiquidPartID==0) hLiqImTimeDiff[liqChn]->Fill( tDiff, 0 ); //n-n
                            else if(iImagerPartID==0 && iLiquidPartID==2) hLiqImTimeDiff[liqChn]->Fill( tDiff, 1 ); //n-g
                            else if(iImagerPartID==2 && iLiquidPartID==0) hLiqImTimeDiff[liqChn]->Fill( tDiff, 2 ); //g-n
                            else if(iImagerPartID==2 && iLiquidPartID==2) hLiqImTimeDiff[liqChn]->Fill( tDiff, 3 ); //g-g
                        }
                        ++it;
                    }
                    else break;
                }
                
                coincTree->Fill();
                
            }
            delete _imagerHits.front();
            _imagerHits.pop_front();
            
        }
        
    }
    
    printf("\n");
    
    
}

Int_t CoincidenceMaker::GetParticleStatus(const NGMHit *theHit){
    
    Int_t iPartID = 3;
    
    if( TMath::Abs(theHit->GetNeutronId()) < 2.0 && theHit->GetGammaId() < -5.0 && theHit->GetEnergy() > 100.0) iPartID = 0; //neutron
    else if( theHit->GetPSD() >0.1 && theHit->GetPSD() < 0.55  && theHit->GetEnergy() > 100.0 ) iPartID = 1; //thermal
    else if( TMath::Abs(theHit->GetGammaId()) < 2.0 && theHit->GetEnergy() > 100.0) iPartID = 2; //gamma
    
    return iPartID;
    
}

void CoincidenceMaker::AddImagerHit(const NGMHit *theImagerHit){
    
    _imagerHits.push_back(theImagerHit->DuplicateHit());
    
}

void CoincidenceMaker::AddLiquidHit(const NGMHit *theLiquidHit){
    
    _liqHits.push_back(theLiquidHit->DuplicateHit());
   
}

ClassImp(CoincidenceVariables)

void CoincidenceVariables::ClearData(){
    
    pixelX = -1;
    pixelY = -1;
    relativeTime = -1e9;
    particleType = -1;
    energy = -1;

}

ClassImp(CoincidentEvent)

TClonesArray *CoincidentEvent::sLiqCoinc = 0;

CoincidentEvent::CoincidentEvent(){
    
    if(!sLiqCoinc) sLiqCoinc = new TClonesArray("CoincidenceVariables",5);
    sLiqCoinc->SetOwner(true);
    liqCoinc = sLiqCoinc;

    ImagerHit = new CoincidenceVariables();
    
    ClearData();
}

CoincidentEvent::~CoincidentEvent(){

    if(liqCoinc) liqCoinc->Clear("C");

}

void CoincidentEvent::AddLiqCoinc(Int_t iPartID, Double_t tDiff, Double_t en){
 
    CoincidenceVariables *theHit = (CoincidenceVariables*)liqCoinc->ConstructedAt(liqCoinc->GetEntries());
    theHit->particleType = iPartID;
    theHit->relativeTime = tDiff;
    theHit->energy = en;
    
}

void CoincidentEvent::ClearData(){
    
    ImagerHit->ClearData();
    
    numLiqGamma = 0;
    numLiqNeutron = 0;
    
    liqCoinc->Clear();
    
}

CoincToTuple::CoincToTuple(){
    InitCommon();
}
CoincToTuple::CoincToTuple(const char* name, const char* title){
    InitCommon();
    coim = new CoincidenceMaker();
}
CoincToTuple::~CoincToTuple(){
    
}

bool  CoincToTuple::finish()
{
    coim->SaveHistograms();
    return true;
}

void  CoincToTuple::LaunchDisplayCanvas()
{
    return;
}


void  CoincToTuple::ResetHistograms()
{

}


bool CoincToTuple::init()
{
    return true;
}

bool CoincToTuple::processConfiguration(const NGMSystemConfiguration* conf)
{
    coim->SetOutputFileName(Form("Coinc%lld.root",conf->getRunNumber()));
    coim->SetupHistograms();
    return true;
}

void CoincToTuple::InitCommon()
{
    coim = 0;
}

bool CoincToTuple::processHit(const NGMHit* tHit)
{
    coim->processHit(tHit);
    return true;
}

