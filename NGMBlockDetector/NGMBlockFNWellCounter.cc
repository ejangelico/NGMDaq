#include "NGMBlockFNWellCounter.h"
#include "TH2.h"
#include "TH3.h"
#include "NGMLogger.h"
#include "NGMSimpleParticleIdent.h"
#include "TFolder.h"
#include <cmath>
#include "TFile.h"
#include "TObjString.h"
#include "NGMSystemConfiguration.h"
#include "TParameter.h"

ClassImp(NGMBlockFNWellCounter_tuple_st)

ClassImp(NGMBlockFNWellCounter)

void NGMBlockFNWellCounter::InitCommon()
{
  _hDoublesVsDistance = 0;
  _hDoublesVsDistanceVsEmin = 0;
  _hTimingDiffPerColumn = 0;
  _hNEnergyvsTof = 0;
  _hNEnergyvsTofPrevious = 0;
  _energy = 0;
  _duration = 0;
  _totalSingles = 0;
  _runBeginTime = 0;
  _maxtimediff_ns=100.0;
  _tupleOut = 0;
}

NGMBlockFNWellCounter::NGMBlockFNWellCounter()
{
  InitCommon();
}

NGMBlockFNWellCounter::NGMBlockFNWellCounter(const char* name, const char* title)
: NGMModule(name,title)
{
  InitCommon();
  partID = new NGMSimpleParticleIdent();
}

NGMBlockFNWellCounter::~NGMBlockFNWellCounter()
{
  NGMHit* thit = 0;
  while( (thit = nextHit(-1.0))){
    delete thit;
  }
  CloseTuple();
}

void NGMBlockFNWellCounter::CloseTuple(){
  if(_tupleOut){
    _tupleOut->AutoSave();
    TFile* outFile = _tupleOut->GetCurrentFile();
    //Write out value of parameters
    TParameter<double> pDuration;
    TParameter<double> pTotalSingles;
    pDuration.SetVal(_duration);
    pTotalSingles.SetVal(_totalSingles);
    outFile->WriteTObject(&pDuration,"Duration");
    outFile->WriteTObject(&pTotalSingles,"TotalSingles");
    delete _tupleOut->GetCurrentFile();
    _tupleOut = 0;
  }
}

  
void NGMBlockFNWellCounter::setMaxNanoSecInList(double newVal)
{
  _maxtimediff_ns = newVal;
}

bool  NGMBlockFNWellCounter::processConfiguration(const NGMSystemConfiguration* conf)
{
  _bmap.Init(conf);
  partID->Init(conf);
  clear();
  push(*((const TObject*)(conf)));
  delete _runBeginTime;
  _runBeginTime = 0;
  _totalSingles = 0;
  _duration = 0.0;
  
  if(!_hDoublesVsDistance)
  {
    _hDoublesVsDistance = new TH2D(Form("%s_DoublesDistribution",GetName()),
                                   Form("%s_DoublesDistribution; Time [ns]; Distance [pixels]",GetName()),
                                   1000,0,1000,60,0,60);
    _hDoublesVsDistance->SetDirectory(0);
    GetParentFolder()->Add(_hDoublesVsDistance);
  }
  
  _hDoublesVsDistance->Reset();
  
  if(!_hDoublesVsDistanceVsEmin)
  {
    _hDoublesVsDistanceVsEmin = new TH3D(Form("%s_DoublesDistribution3d",GetName()),
                                       Form("%s_DoublesDistribution3d; Time [ns]; Distance [pixels]; Min Energy[keV]",GetName()),
                                       100,0,1000,60,0,60,100,0,1000);
        _hDoublesVsDistanceVsEmin->SetDirectory(0);
        GetParentFolder()->Add(_hDoublesVsDistanceVsEmin);
    }
    
    _hDoublesVsDistanceVsEmin->Reset();
    
  
  if(!_hTimingDiffPerColumn)
  {
    _hTimingDiffPerColumn = new TH2D(Form("%s_TimingDiffPerColumn",GetName()),
                                   Form("%s_TimingDiffPerColumn; Time [ns]; Column ",GetName()),
                                   40,-20,20,4,0,4);
    _hTimingDiffPerColumn->SetDirectory(0);
    GetParentFolder()->Add(_hTimingDiffPerColumn);
  }
  
  _hTimingDiffPerColumn->Reset();
  
  if(!_energy)
  {
    _energy = new TH1D(Form("%s_Energy",GetName()),
                                     Form("%s_Energy; Energy [keV] ",GetName()),
                                     1000,0,8000);
    _energy->SetDirectory(0);
    GetParentFolder()->Add(_energy);
  }
  
  _energy->Reset();
    
    if(!_hNEnergyvsTof)
    {
        _hNEnergyvsTof = new TH2D(Form("%s_hNEnergyvsTof",GetName()),
                           Form("%s_hNEnergyvsTof; NTof [ns]; Neutron Energy [keV] ",GetName()),
                           2000,-1000,1000,1000,0,4000);
        _hNEnergyvsTof->SetDirectory(0);
        GetParentFolder()->Add(_hNEnergyvsTof);
    }

    _hNEnergyvsTof->Reset();
    
    ;
    if(!_hNEnergyvsTofPrevious)
    {
        _hNEnergyvsTofPrevious = new TH2D(Form("%s_h_hNEnergyvsTofPrevious",GetName()),
                                  Form("%s_hNEnergyvsTofPrevious; NTof [ns]; Neutron Energy [keV] ",GetName()),
                                  2000,-1000,1000,1000,0,4000);
        _hNEnergyvsTofPrevious->SetDirectory(0);
        GetParentFolder()->Add(_hNEnergyvsTofPrevious);
    }
    
    _hNEnergyvsTofPrevious->Reset();
  
  CloseTuple();
  // Create the output tuple
  TDirectory* thisDirectory = gDirectory;
  TString foutName =Form("%s_%lld_ntuple.root",
                         GetName(),
                         conf->getRunNumber());
  TFile* fout = TFile::Open(foutName.Data(),
                            "RECREATE");
  _tupleOut = new TTree("pairtuple","pair tuple");
  _tupleOut->Branch("p",&_tbuf);
  if(thisDirectory) thisDirectory->cd();
  
  return true;
}

bool  NGMBlockFNWellCounter::processHit(const NGMHit* tHit)
{
  if(!_runBeginTime) _runBeginTime = new NGMTimeStamp(tHit->GetNGMTime());
  _duration = tHit->TimeDiffNanoSec(*_runBeginTime);
  
  if( ! (partID->IsSelected(tHit)) ) return false;
  _totalSingles++;
  
  NGMHit* hit = _hitPool.GetHit();
  hit->CopyHit(tHit);
  if(!pushHit(hit))
    _hitPool.ReturnHit(hit);
  NGMHit* nexthit = 0;
  while( (nexthit =nextHit()) )
  {
    _hitPool.ReturnHit( nexthit);
  }
  
  // Form all pairwise combinations
  NGMHit* hitToAnalyze = _hitbuffer.back();
  int detSequence, localRow, localCol;
  _bmap.GlobalPixelToLocalPixel(hitToAnalyze->GetPixel(),  detSequence, localRow, localCol);
  int col1 = _bmap.GetBlockColumn(detSequence);

  if(partID->GetType(hit) == NGMSimpleParticleIdent::lsgamma){
      _preceedingGamma = hit->GetNGMTime();
  }
  if(partID->GetType(hit) == NGMSimpleParticleIdent::lsneutron){
      _hNEnergyvsTofPrevious->Fill(hit->TimeDiffNanoSec(_preceedingGamma),hit->GetEnergy());
        ;
  }
    
  std::list<NGMHit*>::iterator itr = _hitbuffer.begin();
  for(;(*itr)!=_hitbuffer.back(); itr++){
    
    double distanceInPixels = _bmap.GetDistanceInPixels((*itr)->GetPixel(),hit->GetPixel());
    double timeDiff = hitToAnalyze->TimeDiffNanoSec((*itr)->GetNGMTime());
    _energy->Fill(hitToAnalyze->GetEnergy());
    _energy->Fill((*itr)->GetEnergy());

    _hDoublesVsDistance->Fill(timeDiff,distanceInPixels);
    _hDoublesVsDistanceVsEmin->Fill(timeDiff,distanceInPixels,std::min(hitToAnalyze->GetEnergy(),(*itr)->GetEnergy()));
    _tbuf.t = timeDiff;
    _tbuf.d = distanceInPixels;
    _tbuf.e1 = hitToAnalyze->GetEnergy();
    _tbuf.e2 = (*itr)->GetEnergy();
    _tupleOut->Fill();
    
    _bmap.GlobalPixelToLocalPixel((*itr)->GetPixel(),  detSequence, localRow, localCol);
    int col2 = _bmap.GetBlockColumn(detSequence);
    if( (col2-col1) == 1){
      _hTimingDiffPerColumn->Fill(timeDiff,col2);
    }else if( (col2-col1) == -1){
      _hTimingDiffPerColumn->Fill(-timeDiff,col1);
    }
      NGMHit* nhit = 0;
      NGMHit* ghit = 0;
      if(partID->GetType((*itr)) == NGMSimpleParticleIdent::lsneutron
         && partID->GetType(hit) == NGMSimpleParticleIdent::lsgamma)
      {
          nhit = (*itr);
          ghit = hit;
      }else if (partID->GetType((*itr)) == NGMSimpleParticleIdent::lsgamma
                && partID->GetType(hit) == NGMSimpleParticleIdent::lsneutron){
          ghit = (*itr);
          nhit = hit;
      }
      if(nhit&&ghit){
          _hNEnergyvsTof->Fill(nhit->TimeDiffNanoSec(ghit->GetNGMTime()),nhit->GetEnergy());
      }
  }

  
  return true;
}

bool  NGMBlockFNWellCounter::processMessage(const TObjString* mess){
  if(mess->GetString()=="EndRunSave"){
    CloseTuple();
  }
  push(*((const TObject*)(mess)));
  return true;
}

void NGMBlockFNWellCounter::clear(){
  while(!_hitbuffer.empty()){
    NGMHit* retVal = _hitbuffer.front();
    _hitbuffer.pop_front();
    delete retVal;
  }
}

bool NGMBlockFNWellCounter::pushHit(NGMHit* hit)
{
  // A few simple but likely cases
  if(_hitbuffer.empty())
  {
    _hitbuffer.push_back(hit);
    return true;
  }
  
  if(hit->TimeDiffNanoSec(_hitbuffer.back()->GetNGMTime())>=0.0)
  {
    _hitbuffer.push_back(hit);
  }else{
    _hitbuffer.back()->Print();
    hit->Print();
    LOG<<" DATA NOT SORTED "<<ENDM_FATAL;
    exit(-1);
  }
  return true;
}

NGMHit* NGMBlockFNWellCounter::nextHit(double maxtimediff_ns)
{
  NGMHit* retVal = 0;
  if(!_hitbuffer.empty()
     &&(maxtimediff_ns<0.0
        || _hitbuffer.back()->TimeDiffNanoSec(_hitbuffer.front()->GetNGMTime())>maxtimediff_ns)
     )
  {
    retVal = _hitbuffer.front();
    _hitbuffer.pop_front();
  }
  return retVal;
}

NGMHit* NGMBlockFNWellCounter::nextHit()
{
  return nextHit(_maxtimediff_ns);
}

bool NGMBlockFNWellCounter::empty(double maxtimediff_ns)
{
  bool retVal = true;
  if(!_hitbuffer.empty()
     &&(maxtimediff_ns<0.0
        || _hitbuffer.back()->TimeDiffNanoSec(_hitbuffer.front()->GetNGMTime())>maxtimediff_ns)
     )
  {
    retVal = false;
  }
  return retVal;
}

bool NGMBlockFNWellCounter::empty()
{
  return empty(_maxtimediff_ns);
}

void NGMBlockFNWellCounter::Print(Option_t* option) const
{
  NGMModule::Print();
}

int NGMBlockFNWellCounter::getTotalDoubles(const double maxtime_ns, const double mindistance_pix ) const {
  if(!_hDoublesVsDistance) return -1;
  int maxtimebin = _hDoublesVsDistance->GetXaxis()->FindBin(maxtime_ns);
  int minpixbin = _hDoublesVsDistance->GetYaxis()->FindBin(mindistance_pix);
  int maxpixbin = _hDoublesVsDistance->GetYaxis()->GetNbins()+1; //Include overflow

  return _hDoublesVsDistance->Integral(1,maxtimebin,minpixbin,maxpixbin);
}

int NGMBlockFNWellCounter::getTotalDoubles3d(const double maxtime_ns, const double mindistance_pix, const double minenergy) const {
    if(!_hDoublesVsDistanceVsEmin) return -1;
    int maxtimebin = _hDoublesVsDistanceVsEmin->GetXaxis()->FindBin(maxtime_ns);
    
    int minpixbin = _hDoublesVsDistanceVsEmin->GetYaxis()->FindBin(mindistance_pix);
    int maxpixbin = _hDoublesVsDistanceVsEmin->GetYaxis()->GetNbins()+1; //Include overflow
    
    int minebin = _hDoublesVsDistanceVsEmin->GetZaxis()->FindBin(minenergy);
    int maxebin = _hDoublesVsDistanceVsEmin->GetZaxis()->GetNbins()+1; //Include overflow
    
    return _hDoublesVsDistanceVsEmin->Integral(1,maxtimebin,minpixbin,maxpixbin,minebin,maxebin);
}
