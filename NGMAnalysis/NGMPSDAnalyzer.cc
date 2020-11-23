#include "NGMPSDAnalyzer.h"
#include "NGMHit.h"
#include <iostream>

ClassImp(NGMPSDAnalyzer)

NGMPSDAnalyzer::NGMPSDAnalyzer()
{
  nWaveform = 0;
  gWaveform = 0;
  gGattiWeights= 0;
  normalization = 0.0;
  baseline = 0.0;
}

NGMPSDAnalyzer::NGMPSDAnalyzer(const char* name, const char* title)
: TNamed(name,title)
{
  nWaveform = 0;
  gWaveform = 0;
  gGattiWeights= 0;
  normalization = 0.0;
  baseline = 0.0;
}

void NGMPSDAnalyzer::SetNGates(int ngates){
  gbegin.Set(ngates);
  glength.Set(ngates);
  _gate.Set(ngates);
}

void NGMPSDAnalyzer::SetGates(int ngates, int *begin, int *length){
  SetNGates(ngates);
  for(int igate = 0; igate < ngates; igate++)
  {
    gbegin[igate] = begin[igate];
    glength[igate] = length[igate];
  }
}

void NGMPSDAnalyzer::SetGate(int igate, double begin_ns, double length_ns){
  gbegin[igate] = begin_ns/nsPerSample;
  glength[igate] = length_ns/nsPerSample;
  if(glength[igate]<1) glength[igate] = 1;
}

void NGMPSDAnalyzer::Print(Option_t* option)
{
  std::cout<<"PSDAnalyzer: "<<GetName()<<" "<<GetTitle()<<std::endl;
  std::cout<<"nsPerSample: "<<nsPerSample<<std::endl;
  std::cout<<"gbegin[samples]\tglength[samples]\tgbegin[ns]\tglength[ns]"<<std::endl;
  for(int igate=0; igate < gbegin.GetSize(); igate++)
  {
    std::cout<<gbegin[igate]<<"\t"<<glength[igate]
        <<"\t"<<gbegin[igate]*nsPerSample<<"\t"<<glength[igate]*nsPerSample<<std::endl;
  }
}

void NGMPSDAnalyzer::SetWaveformParams(double tnsPerSample,
                                       double baselineBegin,
                                       double baselineEnd)
{
  nsPerSample = tnsPerSample;
  TString titleString = GetName();
  int nbins = (int)(baselineEnd - baselineBegin);
  titleString+="_N";
  if(!nWaveform){ nWaveform = new TProfile(titleString,titleString,
                                          nbins,baselineBegin,baselineEnd);
  }else{
    nWaveform->SetNameTitle(titleString,titleString);
    nWaveform->SetBins(nbins,baselineBegin,baselineEnd);
  }

  titleString=GetName();
  titleString+="_G";
  if(!gWaveform){
    gWaveform = new TProfile(titleString,titleString,
                                          nbins,baselineBegin,baselineEnd);
  }else{
    gWaveform->SetNameTitle(titleString,titleString);
    gWaveform->SetBins(nbins,baselineBegin,baselineEnd);
  }

  titleString=GetName();
  titleString+="_GattiWeights";
  if(!gGattiWeights){
    gGattiWeights = new TProfile(titleString,titleString,
                             nbins,baselineBegin,baselineEnd);
  }else{
    gGattiWeights->SetNameTitle(titleString,titleString);
    gGattiWeights->SetBins(nbins,baselineBegin,baselineEnd);
  }
  
}

void NGMPSDAnalyzer::AddNeutron(const NGMHit* hit, double baseline, double normalization)
{
  for(int isample = 0; isample < hit->GetNSamples();isample++)
  {
    nWaveform->Fill(((double)isample+hit->GetCFD())*nsPerSample,
                    (hit->GetSample(isample)-baseline)/normalization);
  }
}

void NGMPSDAnalyzer::AddGamma(const NGMHit* hit, double baseline, double normalization)
{
  for(int isample = 0; isample < hit->GetNSamples();isample++)
  {
    gWaveform->Fill(((double)isample+hit->GetCFD())*nsPerSample,
                    (hit->GetSample(isample)-baseline)/normalization);
  }
}

void NGMPSDAnalyzer::ComputeGattiWeights()
{
  
  if(!nWaveform || ! gWaveform) return;
  
  TString titleString = GetName();
  titleString+="_GattiWeights";
  if(!gGattiWeights){
    gGattiWeights = new TProfile(titleString,titleString,
                                 nWaveform->GetNbinsX(),
                                 nWaveform->GetXaxis()->GetXmin(),
                                 nWaveform->GetXaxis()->GetXmax());
  }else{
    gGattiWeights->SetNameTitle(titleString,titleString);
    gGattiWeights->SetBins(nWaveform->GetNbinsX(),
                           nWaveform->GetXaxis()->GetXmin(),
                           nWaveform->GetXaxis()->GetXmax());
  }
  gGattiWeights->Reset();
  
  for(int ibin = 1; ibin <= gGattiWeights->GetNbinsX(); ibin++)
  {
    double nval = nWaveform->GetBinContent(ibin);
    double gval = gWaveform->GetBinContent(ibin);
    if(nval ==0.0 && gval == 0.0) gGattiWeights->SetBinContent(ibin, 0.0);
    else{
      double xval = gGattiWeights->GetXaxis()->GetBinLowEdge(ibin);
      gGattiWeights->Fill(xval,(nval-gval)/(nval+gval));
    }
  }
}

ClassImp(NGMPSDFIS)

NGMPSDFIS::NGMPSDFIS()
{
}

NGMPSDFIS::NGMPSDFIS(const char* name, const char* title)
: NGMPSDAnalyzer(name,title)
{}



double NGMPSDFIS::ComputePSD(const NGMHit* hit)
{
  
  int zerocrossingSample = int(hit->GetCFD());
  
  double fractionalClock = hit->GetCFD() - zerocrossingSample;
  
  for(int igate = 0; igate < _gate.GetSize(); igate++)
    _gate[igate] = hit->ComputeSum(gbegin[igate]+zerocrossingSample,glength[igate]);

  baseline = _gate[0]/(double)glength[0];
  normalization = hit->ComputeSum(hit->GetNSamples()-11,10)/10.0 - baseline;

  double energy = _gate[2]-fractionalClock*hit->GetSample(gbegin[2]+zerocrossingSample)+(fractionalClock)*hit->GetSample(gbegin[2]+glength[2]+zerocrossingSample)
                 + _gate[1]-fractionalClock*hit->GetSample(gbegin[1]+zerocrossingSample)+(fractionalClock)*hit->GetSample(gbegin[1]+glength[1]+zerocrossingSample)
                 - _gate[0]*((double)(glength[1] + glength[2]))/glength[0];
  
  return (_gate[2]-fractionalClock*hit->GetSample(gbegin[2]+zerocrossingSample)+(fractionalClock)*hit->GetSample(gbegin[2]+glength[2]+zerocrossingSample)
                     - _gate[0]*((double)(glength[2]))/glength[0]) / energy;
}

ClassImp(NGMPSDDelayedGate)

NGMPSDDelayedGate::NGMPSDDelayedGate()
{
}

NGMPSDDelayedGate::NGMPSDDelayedGate(const char* name, const char* title)
: NGMPSDAnalyzer(name,title)
{}

double NGMPSDDelayedGate::ComputePSD(const NGMHit* hit)
{
  
  int zerocrossingSample = int(hit->GetCFD());
  
  double fractionalClock = hit->GetCFD() - zerocrossingSample;
  
  for(int igate = 0; igate < _gate.GetSize(); igate++)
    _gate[igate] = hit->ComputeSum(gbegin[igate]+zerocrossingSample,glength[igate]);
  
  baseline = _gate[0]/(double)glength[0];
  normalization = _gate[1] - baseline*glength[1];
  
  double energy = (double)_gate[1] - baseline*glength[1];
  
  return  ((double)_gate[2] - (double)_gate[0]*glength[2]/(double)glength[0])/ energy;

}

NGMPSDGatti::NGMPSDGatti()
{
}

NGMPSDGatti::NGMPSDGatti(const char* name, const char* title)
: NGMPSDAnalyzer(name,title)
{}

void NGMPSDGatti::SetGattiWeights( double beginTime, double endTime, TProfile* gWeights)
{
  if(gGattiWeights != gWeights  && gWeights != 0) gWeights->Copy(*gGattiWeights);
  gattiBeginTime = beginTime;
  gattiEndTime = endTime;
}

double NGMPSDGatti::ComputePSD(const NGMHit* hit)
{
  
  double CFD = -hit->GetCFD();
  int FirstBin = (gattiBeginTime/nsPerSample+CFD);
  int LastBin = (gattiEndTime/nsPerSample+CFD);
  if(LastBin>=hit->GetNSamples()) LastBin=hit->GetNSamples()-1;
  
  //int zerocrossingSample = int(hit->GetCFD());
  
  //for(int igate = 0; igate < _gate.GetSize(); igate++)
  //  _gate[igate] = hit->ComputeSum(gbegin[igate]+zerocrossingSample,glength[igate]);

  //Set these prior to call
  //baseline = _gate[0]/(double)glength[0];
  //normalization = _gate[1] - baseline*glength[1];

  double nsum,dsum;
  nsum=dsum=0.0;
  for (int isample = FirstBin; isample<=LastBin; isample++) {
    double time = ((double)isample+hit->GetCFD())*nsPerSample;
    double gWeight = gGattiWeights->GetBinContent(gGattiWeights->GetXaxis()->FindBin(time));
      nsum+=gWeight*((hit->GetSample(isample)-baseline));///normalization);
    dsum+=gWeight;
  }
  return nsum/dsum;
}

NGMPSDGattiTail::NGMPSDGattiTail()
{
}

NGMPSDGattiTail::NGMPSDGattiTail(const char* name, const char* title)
: NGMPSDAnalyzer(name,title)
{}

void NGMPSDGattiTail::SetGattiWeights( double beginTime, double endTime, TProfile* gWeights)
{
  if(gGattiWeights != gWeights  && gWeights != 0) gWeights->Copy(*gGattiWeights);
  gattiBeginTime = beginTime;
  gattiEndTime = endTime;
}

double NGMPSDGattiTail::ComputePSD(const NGMHit* hit)
{
  //This should be modified for tail weights
  double CFD = hit->GetCFD();
  int FirstBin = (gattiBeginTime/nsPerSample+CFD);
  int LastBin = (gattiEndTime/nsPerSample+CFD);
  if(LastBin>=hit->GetNSamples()) LastBin=hit->GetNSamples()-1;
  
  int zerocrossingSample = int(hit->GetCFD());
  
  for(int igate = 0; igate < _gate.GetSize(); igate++)
    _gate[igate] = hit->ComputeSum(gbegin[igate]+zerocrossingSample,glength[igate]);
  
  baseline = _gate[0]/(double)glength[0];
  normalization = _gate[1] - baseline*glength[1];
  
  double nsum,dsum;
  nsum=dsum=0.0;
  for (int isample = FirstBin; isample<=LastBin-1; isample++) {
    double time = ((double)isample-hit->GetCFD())*nsPerSample;
    double gWeight = gGattiWeights->GetBinContent(gGattiWeights->GetXaxis()->FindBin(time));
    nsum+=gWeight*((hit->GetSample(isample)-baseline)/normalization);
    dsum+=gWeight;
  }
  return nsum/dsum;
}


