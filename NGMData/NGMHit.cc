#include "NGMHit.h"
#include "TTimeStamp.h"
#include "NGMWaveform.h"
#include <iostream>
#include "TROOT.h"
#include "TH1.h"
#include "NGMLogger.h"
#include "TMath.h"
#include <cmath>
#include "TArrayD.h"
#include "TGraph.h"
#include <cmath>
#define ngmpicosecspersecond 1000000000000LL

ClassImp(NGMTimeStamp)

NGMTimeStamp::NGMTimeStamp()
{
  _picosecs = 0;
}

NGMTimeStamp::NGMTimeStamp(time_t t, Int_t nsec, Int_t picosecs)
: TTimeStamp(t,nsec)
{

  _picosecs = picosecs;
  NormalizePicoSecs();
}

NGMTimeStamp::NGMTimeStamp(TTimeStamp ts)
: TTimeStamp(ts)
{
  _picosecs = 0;
}


void NGMTimeStamp::SetPicoSecs(const int picosecs)
{
  _picosecs = picosecs;
  NormalizePicoSecs();
}

int NGMTimeStamp::GetPicoSecs() const
{
  return _picosecs;
}

void NGMTimeStamp::NormalizePicoSecs()
{
  // ensure that _picosecs is in range [0,999]
  const Int_t kPsPerNs = 1000;
  while(_picosecs < 0)
  {
    _picosecs += kPsPerNs;
    NGMTimeStamp ts(GetSec(),GetNanoSec() - 1, _picosecs);
    (*this) = ts;
    //std::cout<<"LT "<<GetSec()<<" : "<<GetNanoSec()<<" : "<<GetPicoSecs()<<std::endl;
  }

  while(_picosecs >= kPsPerNs)
  {
    _picosecs -= kPsPerNs;
    NGMTimeStamp ts(GetSec(),GetNanoSec() + 1, _picosecs);
    (*this) = ts;
    //std::cout<<"GT "<<GetSec()<<" : "<<GetNanoSec()<<" : "<<GetPicoSecs()<<std::endl;
  }
  if(GetNanoSec()<0 || GetNanoSec()>1000000000LL)
  {
     NGMTimeStamp tsfinal(GetSec(),GetNanoSec(),_picosecs);
     (*this) = tsfinal;
      //std::cout<<"GLNS "<<GetSec()<<" : "<<GetNanoSec()<<" : "<<GetPicoSecs()<<std::endl;
  }
}

double NGMTimeStamp::TimeDiffNanoSec(const NGMTimeStamp& baseTime) const
{
   
   double baseTimeDiff = 0.0;
   Long64_t timeDiffns = ((Long64_t)(GetSec() - baseTime.GetSec()))*1000000000LL;
   timeDiffns+=(Long64_t)(GetNanoSec()-baseTime.GetNanoSec());
   
   
   baseTimeDiff = 1E-3*((double)(GetPicoSecs() - baseTime.GetPicoSecs())) + (double)(timeDiffns);
   return baseTimeDiff;
}

void NGMTimeStamp::IncrementNs(double time_ns)
{

   Long64_t tmpPicos = GetPicoSecs()+floor(time_ns*1E3);
   Long64_t newPicos = tmpPicos%1000LL;
   Long64_t tmpNanos = (tmpPicos-newPicos)/1000LL+GetNanoSec();
   Long64_t newNanos = tmpNanos%1000000000;
   Long64_t tmpSecs = GetSec() + (tmpNanos-newNanos)/1000000000;
   
   //(*this) = NGMTimeStamp(tmpSecs,newNanos,newPicos);
    SetSec(tmpSecs);
    SetNanoSec(newNanos);
    SetPicoSecs(newPicos);
   
}

 Bool_t operator==(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs)
   { return lhs.GetSec() == rhs.GetSec() &&
            lhs.GetNanoSec() == rhs.GetNanoSec() &&
            lhs.GetPicoSecs() == lhs.GetPicoSecs(); }

 Bool_t operator!=(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs)
   { return lhs.GetSec() != rhs.GetSec() ||
            lhs.GetNanoSec() != rhs.GetNanoSec() || 
            lhs.GetPicoSecs() != rhs.GetPicoSecs(); }

 Bool_t operator<(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs)
   {
      if(lhs.GetSec() < rhs.GetSec()) return true;
      if(lhs.GetSec() > rhs.GetSec()) return false;

      // So then secs are equal
      if(lhs.GetNanoSec() < rhs.GetNanoSec()) return true;
      if(lhs.GetNanoSec() > rhs.GetNanoSec()) return false;
      
      // So then nanosecs are equal
      if(lhs.GetPicoSecs() < rhs.GetPicoSecs()) return true;
      if(lhs.GetPicoSecs() > rhs.GetPicoSecs()) return false;

      // So then picosecs are equal
      return false;
}

Bool_t operator<=(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs)
{
      if(lhs<rhs || lhs == rhs) return true;
      return false;
}

Bool_t operator>(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs)
{
      if(lhs.GetSec() > rhs.GetSec()) return true;
      if(lhs.GetSec() < rhs.GetSec()) return false;

      // So then secs are equal
      if(lhs.GetNanoSec() > rhs.GetNanoSec()) return true;
      if(lhs.GetNanoSec() < rhs.GetNanoSec()) return false;
      
      // So then nanosecs are equal
      if(lhs.GetPicoSecs() > rhs.GetPicoSecs()) return true;
      if(lhs.GetPicoSecs() < rhs.GetPicoSecs()) return false;

      // So then picosecs are equal
      return false;

}

Bool_t operator>=(const NGMTimeStamp &lhs, const NGMTimeStamp &rhs)
{
  if(lhs>rhs || lhs == rhs) return true;
  return false;
}

ClassImp(NGMHit)

const TTimeStamp& NGMHit::GetTimeStamp() const 
{
	static TTimeStamp ts(0,0);
	return ts;
}

const NGMTimeStamp& NGMHit::GetRawTime() const 
{
	static NGMTimeStamp ts;
	ts = NGMTimeStamp(GetTimeStamp());
	return ts;
}

  
void NGMHit::CopyHit(const NGMHit* hit){
  if(!hit) return;
  SetSlot(hit->GetSlot());
  SetChannel(hit->GetChannel());
  SetPulseHeight(hit->GetPulseHeight());
  SetTimeStamp(hit->GetTimeStamp());
  SetPileUpCounter(hit->GetPileUpCounter());
  SetCFDWord(hit->GetCFDWord());
  SetRawClock(hit->GetRawClock());

  for(int id = 0; id < TMath::Min(hit->GetGateCount(),GetGateCount()); id++) SetGate(id, hit->GetGate(id));
  if(hit->GetNSamples()>0){
	  if(hit->GetWaveform()){
		const NGMWaveform* oldWave = hit->GetWaveform();
		if(oldWave){
		  NGMWaveform* newWave = CreateWaveform();
		  if(newWave) newWave->CopyWaveform(oldWave);
	      
		}
	  }else{
		  SetNSamples(hit->GetNSamples());
		  for(int isample = 0; isample < hit->GetNSamples(); isample++)
		  {
			  SetSample(isample,hit->GetSample(isample));
		  }
	  }// Check if waveform is dynamically allocated
  }// Check if Waveform is present
  //Now Calibrated NGMHitv3,4,5 data
  SetRawTime(hit->GetRawTime());
  SetCalibratedTime(hit->GetCalibratedTime());
  SetEnergy(hit->GetEnergy());
  SetNeutronId(hit->GetNeutronId());
  SetGammaId(hit->GetGammaId());
  SetBlockX(GetBlockX());
  SetBlockY(GetBlockY());
};

void	NGMHit::Print(Option_t* option) const {
  std::cout<<"NGMHit: Slot("<<GetSlot()
           <<")\tChannel( " <<GetChannel()
           <<")\tPulseHeight( "<<GetPulseHeight()
           <<")\tTime( "<<GetTimeStamp().GetSec()
           <<" "<<GetTimeStamp().GetNanoSec()
           <<")"<<std::endl;
}

int NGMHit::CalcPileup(int peaktime, int gaptime, double threshold, int begin, int end) const
{

  if(GetNSamples()<=0) return 0;
  
  int ntriggers = 0;
  bool aboveT = false;

  if(end == -1)
  {
    end = GetNSamples();
  }
  
  for(int ibin = 2*peaktime+gaptime+begin; ibin <= end; ibin++)
  {
    int sum1 = 0;
    int sum2 = 0;
    for(int ib1 = ibin-(2*peaktime+gaptime); ib1 < (ibin-(peaktime+gaptime)); ib1++)
    {
      sum1+=GetSample(ib1);
    }
    for(int ib1 = ibin-peaktime; ib1 < (ibin); ib1++)
    {
      sum2+=GetSample(ib1);
    }
    if( (sum2-sum1) > threshold)
    {
      if(!aboveT) ntriggers++;
      aboveT = true;
    }else{
      aboveT = false;
    }
  }
    
  return ntriggers;
}


int NGMHit::CountLETriggers(int baselineBegin, int baselineLength, double threshold, int begin, int end, int smooth) const
{
  
  if(GetNSamples()<=0) return 0;
  
  int ntriggers = 0;
  bool aboveT = false;
  
  double baseline = ComputeSum(baselineBegin,baselineLength)/(double)baselineLength;
  
  if(end == -1)
  {
    end = GetNSamples();
  }
  
  int sum = ComputeSum(begin,1+smooth);
  double tsign = 1.0;
  if(threshold<0)
  {
    tsign = -1.0;
  }
  threshold = fabs(threshold);
  
  for(int ibin = begin+1+smooth; ibin < end; ibin++)
  {
    if( (tsign*(sum-baseline*(smooth+1.0))) > threshold)
    {
      if(!aboveT) ntriggers++;
      aboveT = true;
    }else{
      aboveT = false;
    }
    sum-=GetSample(ibin-smooth-1);
    sum+=GetSample(ibin);
  }
  
  return ntriggers;
}

int NGMHit::CountLETriggersList(int baselineBegin, int baselineLength, double threshold, int begin, int end, int smooth, TArrayD& ph, TArrayD& ts) const
{
  double tmpArray[10000];
  double tmpArrayT[10000];
  
  if(GetNSamples()<=0) return 0;
  
  int ntriggers = 0;
  bool aboveT = false;
  
  double baseline = ComputeSum(baselineBegin,baselineLength)/(double)baselineLength;
  
  if(end == -1)
  {
    end = GetNSamples();
  }
  
  int sum = ComputeSum(begin,1+smooth);
  double tsign = 1.0;
  if(threshold<0)
  {
    tsign = -1.0;
  }
  threshold = fabs(threshold);
  int prevSum=sum;
  int aboveThreshCrossing = 0;
  
  for(int ibin = begin+1+smooth; ibin < end; ibin++)
  {
    if( (tsign*(sum-baseline*(smooth+1.0))) > threshold)
    {
      if(!aboveT)
      {
        aboveThreshCrossing = ibin - 1 - smooth;
        ntriggers++;
      }
      aboveT = true;
    }else{
      if(aboveT){
        tmpArray[ntriggers-1]=tsign*(ComputeSum(aboveThreshCrossing,(ibin-1)-aboveThreshCrossing)-baseline*((ibin-1)-aboveThreshCrossing));
        tmpArrayT[ntriggers-1]=ibin-1-(smooth+1)/2.0;
      }
      aboveT = false;
    }
    prevSum=sum;
    sum-=GetSample(ibin-smooth-1);
    sum+=GetSample(ibin);
  }
  ph.Set(ntriggers,tmpArray);
  ts.Set(ntriggers,tmpArrayT);
  return ntriggers;
}

void NGMHit::DisplayWaveform(int option, int option2, Option_t* chopt) const
{
	if(GetNSamples()>0)
	{
		char cbuff[1024];
		sprintf(cbuff,"hWaveform_%d_%u%010u",
			GetSlot()*8+GetChannel(),
			(unsigned int)(GetRawClock()>>32),
			(unsigned int)(GetRawClock()&0xFFFFFFFF));
        TGraph* gr = new TGraph();
        int icolor = kBlack;
		double baseline = 0.0;
        if(option & 0x2 || option & 0x4)
        {
            // Iterprete as 4 block detectors of GetNSamples()/option2 length
            TArrayI x(GetNSamples()/option2);
            TArrayI y(GetNSamples()/option2);
            TArrayI sum(GetNSamples()/option2);
            
            TString opt("AL");
            TString gOpt=chopt;
            if(gOpt!="")
                opt=gOpt;
            for(int ipmt = 0; ipmt<option2; ipmt++)
            {
                if(option & 0x1){
                    baseline = ComputeSum(ipmt*GetNSamples()/option2,10)/10;
                }
                gr->SetLineColor(icolor);
                for(int isample=0;isample<GetNSamples()/option2;isample++)
                {
                    x[isample]=isample;
                    y[isample]=GetSample(ipmt*GetNSamples()/option2+isample)-baseline;
                    sum[isample]+=GetSample(ipmt*GetNSamples()/option2+isample)-baseline;
                    //printf("%d %d %d %d\n",ipmt,isample,ipmt*GetNSamples()/option2+isample,GetSample(ipmt*GetNSamples()/option2+isample));
                }
                if(option & 0x2){
                    gr->DrawGraph(GetNSamples()/option2,x.GetArray(),y.GetArray(),opt.Data());
                    opt="L";
                    icolor++;
                }
            }
            if(option & 0x4 && option2>1)
            {
                gr->SetLineColor(icolor);
                gr->DrawGraph(GetNSamples()/option2,x.GetArray(),sum.GetArray(),opt.Data());
            }
        }else{
            if(option & 0x1){
                baseline = ComputeSum(0,10)/10;
            }
            TArrayI x(GetNSamples());
            TArrayI y(GetNSamples());
            for(int isample = 0; isample < GetNSamples(); isample++)
            {
                x[isample]=isample;
                y[isample]=GetSample(isample)-baseline;
                gr->DrawGraph(GetNSamples(),x.GetArray(),y.GetArray(),"L");
            }
        }
    delete gr;
	}
}

TH1* NGMHit::DisplayNormalizedIntegral(int baselinestart, int baselinelength, int integralstart, int integrallength) const
{
  
  if(GetNSamples() < (integralstart+integrallength) ) return 0;
  
  char cbuff[1024];
  sprintf(cbuff,"hWaveformIntegral_%d_%u%010u",
	  GetSlot()*8+GetChannel(),
	  (unsigned int)(GetRawClock()>>32),
	  (unsigned int)(GetRawClock()&0xFFFFFFFF));
  TH1* th = new TH1F(cbuff,cbuff,integrallength,-0.5,integrallength-0.5);
  double baseline = ((double)ComputeSum(baselinestart,baselinelength))/baselinelength;
  double integralT = ((double)ComputeSum(integralstart,integrallength))-baseline*integrallength;
  double integral = 0.0;
  for(int isample = integralstart; isample < integralstart+integrallength; isample++)
  {
    double newsample = GetSample(isample) - baseline;
    integral+=newsample;
    th->SetBinContent(isample-integralstart+1,integral/integralT);
  }
  th->Draw();
  th->SetDirectory(gROOT);
  return th;

}

void NGMHit::getPeakPosition( int &peakPos, int &peakVal) const
{
  
  peakPos = 0;
  peakVal = 0;
 
  for(int isample = 0; isample < GetNSamples(); isample++)
  {
    if(GetSample(isample)>peakVal)
    {
      
      peakVal = GetSample(isample);
      peakPos = isample;
    }
  }
}

void NGMHit::simpleQuadFit(double* x, double* y, double &xmax, double& ymax) const
{
	double y1 = y[0];
	double y2 = y[1];
	double y3 = y[2];
	double x1 = x[0];
	double x2 = x[1];
	double x3 = x[2];
  
	double cA = ((y2-y1)*(x1-x3) + (y3-y1)*(x2-x1))/((x1-x3)*(x2*x2-x1*x1) + (x2-x1)*(x3*x3-x1*x1));
	double cB = ((y2 - y1) - cA*(x2*x2 - x1*x1)) / (x2-x1);
	double cC = y1 - cA*x1*x1 - cB*x1;
  
	xmax = -cB/2.0/cA;
	ymax = cA*xmax*xmax + cB*xmax + cC;
	return;
}

double NGMHit::getQuadTiming( int peakPos, double baseline, double& quadpeak) const
{
  
  double quadx[3];
  double quady[3];
  
  double qtiming;
  double qpeak;
  
  quadx[0]=peakPos-1;
  quadx[1]=peakPos;
  quadx[2]=peakPos+1;
  quady[0]=GetSample(peakPos-1);
  quady[1]=GetSample(peakPos);
  quady[2]=GetSample(peakPos+1);
  
  if(quady[0]==quady[1] && quady[1]==quady[2] ) return peakPos;
  
  simpleQuadFit(quadx,quady,qtiming,qpeak);
  quadpeak = qpeak;
  
  if( fabs(qtiming - quadx[1])>1.0 ) return peakPos;
  return qtiming;
  
}

double NGMHit::calcTimeToFraction( double tfrac, int baselineLength, int pulseLength,
                                  int prePeakSamples /*= 20*/, int baselineStart /*= 0*/) const
{
  
  double baseline = ComputeSum(baselineStart, baselineLength)/baselineLength;
  int peakSamplePos;
  int peakSampleHeight;
  getPeakPosition(peakSamplePos, peakSampleHeight);
  
  // Interpolated values
  double peakHeight = 0.0;
  double cfdtiming = getQuadTiming(peakSamplePos, baseline, peakHeight);
  
  // Sanity check
  if( (peakSamplePos+ pulseLength - prePeakSamples) > GetNSamples()) return 0.0;
  double nspersample = 1.0;
  double integral = ComputeSum(peakSamplePos-prePeakSamples,pulseLength)-baseline*pulseLength;
  double tint = 0.0;
  double tintprev = 0.0;
  for(int isample = peakSamplePos - prePeakSamples; isample < (peakSamplePos + pulseLength - prePeakSamples); isample++)
  {
    //cout << "Ciao" << endl;
    double sampletime = isample - cfdtiming;
    double samplevalue = (GetSample(isample)-baseline);
    tint+=samplevalue;
    if((1.0-tint/integral)<tfrac)
    {
      return (( tfrac - tintprev )/((1.0-tint/integral) - tintprev) + sampletime)*nspersample;
      
    }
    tintprev=1.0-tint/integral;
  }
  
  return 0.0;
}

double NGMHit::TimeDiffNanoSec(const TTimeStamp& baseTime) const
{
  TTimeStamp ts(GetTimeStamp().GetSec() - baseTime.GetSec(),
                GetTimeStamp().GetNanoSec() - baseTime.GetNanoSec());
  
  return 1E9*(double)ts;
}

double NGMHit::TimeDiffNanoSec(const NGMTimeStamp& baseTime) const
{

  NGMTimeStamp ts = GetNGMTime();

  double baseTimeDiff = 0.0;
  Long64_t timeDiffns = ((Long64_t)(ts.GetSec() - baseTime.GetSec()))*1000000000LL;
  timeDiffns+=(Long64_t)(ts.GetNanoSec()-baseTime.GetNanoSec());
  
  
  baseTimeDiff = 1E-3*((double)(ts.GetPicoSecs() - baseTime.GetPicoSecs())) + (double)(timeDiffns);
  return baseTimeDiff;
}

/// \brief Compute the sum of from the the waveform
int NGMHit::ComputeSum(int sumbegin, int sumlength) const{
    int sum = 0;
    int lastinsum = sumbegin+sumlength - 1;
    if(lastinsum >= GetNSamples()) return 0;
    if(sumbegin < 0) return 0;
    for(int isample = sumbegin; isample <= lastinsum; isample++){
      sum+=GetSample(isample);
    }
    return sum;
}

NGMHit* NGMHit::FastFilter( int risetime, int flattop, double cfdweight) const
{
  const Long64_t offset = 0x7FFF;
  NGMHit* outHit = DuplicateHit();
  Long64_t plus = 0;
  Long64_t sub = 0;
  int isample = 0;
  
  if(risetime<=0)
    return 0;
  if(risetime+flattop >= GetNSamples())
    return 0;
  
  double cfdfactor=1.0/pow(2.0,cfdweight);

  for (; isample < risetime; isample++) {
    plus+=GetSample(isample+risetime+flattop);
    sub+= GetSample(isample);
  }

  for(int outsample = 0; outsample < risetime; outsample++)
  {
    outHit->SetSample(outsample,plus-sub*cfdfactor+offset);
  }

  for(isample = risetime; isample < (GetNSamples()-risetime-flattop); isample++)
  {
    plus+=GetSample(isample+risetime+flattop);
    plus-=GetSample(isample+flattop);
    sub+= GetSample(isample);
    sub-= GetSample(isample-risetime);
    outHit->SetSample(isample,plus-sub*cfdfactor+offset);
  }
  
  for (; isample<GetNSamples(); isample++) {
    outHit->SetSample(isample,plus-sub*cfdfactor+offset);
  }
  
  return outHit;
}

NGMHit* NGMHit::FastFilterXIA(int risetime, int flattop, int CFDdelay, int cfdweight, int nwaveforms) const
{
  NGMHit* outHit = 0;
  NGMHit* ffilt = 0;
  int nsamples = GetNSamples()/nwaveforms;
  if(risetime<=0)
    return 0;
  if(risetime+flattop >= nsamples)
    return 0;
  // Convert block detectors to single waveform
  if(nwaveforms!=1){
    NGMHit* input = DuplicateHit();
    for(int isample = 0; isample< nsamples; isample++){
      int sum = 0;
      for(int iwf = 0; iwf<nwaveforms; iwf++) sum+=GetSample(isample+nsamples*iwf);
      input->SetSample(isample, sum);
    }
    input->SetNSamples(nsamples);
    ffilt = input->FastFilter(risetime,flattop);
    delete input;
  }else{
    ffilt = FastFilter(risetime,flattop);
  }
  outHit = ffilt->DuplicateHit();
  for(int isample = CFDdelay; isample<nsamples; isample++)
  {
    outHit->SetSample(isample, ffilt->GetSample(isample) - ((ffilt->GetSample(isample-CFDdelay))>>(1+cfdweight)) );
  }
  for(int isample = 0; isample<CFDdelay; isample++)
  {
    outHit->SetSample(isample,outHit->GetSample(CFDdelay));
  }
  return outHit;
}

int NGMHit::FastFilterZCPList( int risetime, int flattop,int threshold,int begin,int end, TArrayD& pt) const
{
  Long64_t plus = 0;
  Long64_t sub = 0;
  Long64_t filter = 0;
  int isample = begin-risetime;
  int ntriggers = 0;
  bool aboveT = false;
  double tmpArrayT[10000];


  if(risetime<=0)
    return 0;
  if(risetime+flattop >= GetNSamples())
    return 0;
  
  for (; isample < risetime+begin; isample++) {
    plus+=GetSample(isample+risetime+flattop);
    sub+= GetSample(isample);
  }
  int prevFilter = plus-sub;
  if(end > GetNSamples()-risetime-flattop) end = GetNSamples()-risetime-flattop;
  for(; isample < end; isample++)
  {
    plus+=GetSample(isample+risetime+flattop);
    plus-=GetSample(isample+flattop);
    sub+= GetSample(isample);
    sub-= GetSample(isample-risetime);
    filter = plus - sub;
    if( filter > threshold)
    {
      aboveT = true;
    }
    //Check to see if we have crossed zero
    if(aboveT && filter < 0){
      tmpArrayT[ntriggers]=isample+filter/(double)(prevFilter-filter);
      ntriggers++;
      aboveT = false;
    }
    prevFilter = filter;
  }
  pt.Set(ntriggers,tmpArrayT);
  return ntriggers;
}

NGMHit* NGMHit::FastFilter2( int risetime, int flattop) const
{
  const Long64_t offset = 0x7FFF;
  NGMHit* outHit = DuplicateHit();
  Long64_t plus = 0;
  Long64_t sub = 0;
  Long64_t sub2 = 0;
  
  int isample = 0;
  int outsample = 0;
  
  if(risetime<=0)
    return 0;
  if(3*risetime+2*flattop >= GetNSamples())
    return 0;
  
  for (; isample < risetime; isample++) {
    plus+=GetSample(isample+risetime+flattop);
    sub+= GetSample(isample);
    sub2+= GetSample(isample+2*risetime+2*flattop);
  }
  
  for(outsample = 0; outsample < (risetime + risetime/2); outsample++)
  {
    outHit->SetSample(outsample,sub + sub2 - 2*plus + offset);
  }
  
  for(; isample < (GetNSamples()-2*risetime-2*flattop); isample++)
  {
    plus+=GetSample(isample+risetime+flattop);
    plus-=GetSample(isample+flattop);
    sub+= GetSample(isample);
    sub-= GetSample(isample-risetime);
    sub2+= GetSample(isample+2*risetime+2*flattop);
    sub2-= GetSample(isample+risetime+flattop);
    outHit->SetSample(outsample++,sub + sub2 - 2*plus + offset);
  }
  
  for (; outsample<GetNSamples(); outsample++) {
    outHit->SetSample(outsample,sub + sub2 - 2*plus + offset);
  }
  
  return outHit;
}

Float_t NGMHit::ZCPBipolarF(double fraction) const
{
    const Long64_t offset = 0x7FFF;
    Int_t result = 0;
    //Find peak
    int peakpos = 0;
    int peakvalue = 0;
    getPeakPosition(peakpos, peakvalue);
    if(peakvalue< offset) return result;
    // now step until we find peak
    int isample = peakpos + 1;
    double targetThresh = fraction*(peakvalue-offset)+offset;
    while(isample < GetNSamples() && GetSample(isample)>targetThresh) isample++;
    if (isample == GetNSamples()) {
        return result;
    }
    result = (isample-1)<<8; // shift to the upper bits;
    
    double fractionalClock = (targetThresh - (double)GetSample(isample-1))/((double)GetSample(isample) - (double)GetSample(isample-1));
    result = result | ((UShort_t)(256.0*fractionalClock));
    
    return result/256.0;
}

Float_t NGMHit::ZCPBipolar() const
{
  const Long64_t offset = 0x7FFF;
  Int_t result = 0;
  //Find peak
  int peakpos = 0;
  int peakvalue = 0;
  getPeakPosition(peakpos, peakvalue);
  if(peakvalue< offset) return result;
  // now step until we find peak
  int isample = peakpos + 1;
  
  while(isample < GetNSamples() && GetSample(isample)>offset) isample++;
  if (isample == GetNSamples()) {
    return result;
  }
  result = (isample-1)<<8; // shift to the upper bits;
  
  double fractionalClock = (offset - (double)GetSample(isample-1))/((double)GetSample(isample) - (double)GetSample(isample-1));
  result = result | ((UShort_t)(256.0*fractionalClock));
  
  return result/256.0;
}

Float_t NGMHit::ZCPBipolarFF(int risetime,int flattop) const
{
  NGMHit* tmp = FastFilter(risetime,flattop);
  Float_t tval = tmp->ZCPBipolar();
  delete tmp;
  return tval;
}


NGMHit* NGMHit::correctTailDecay(int g0begin, int g0length, double decaytime, int sbegin, int send) const
{
  
  NGMHit* nhit = DuplicateHit();
  
  if (send==-1) {
    send= GetNSamples();
  }
  
  TArrayD thit(GetNSamples());
  double baseline = nhit->ComputeSum(g0begin,g0length)/(double)(g0length);
  for (int isample = 0; isample<GetNSamples(); isample++) {
    thit[isample]=GetSample(isample)-baseline;
  }
  
  //  for (int isample = sbegin; isample<hit->GetNSamples(); isample++) {
  //    double thisVal = thit[isample];
  //    for(int csample = isample+1;csample < hit->GetNSamples(); csample++){
  //      thit[csample]+=thisVal*(1.0-exp(-(csample-isample)*10.0/decaytime));
  //    }
  //  }
  
  for (int isample = sbegin; isample<send; isample++) {
    double thisVal = thit[isample]-thit[isample-1];
    for(int csample = isample+1;csample < GetNSamples(); csample++){
      thit[csample]+=thisVal*(1.0-exp(-(csample-isample)*10.0/decaytime));
    }
  }
  
  for (int isample = 0; isample<GetNSamples(); isample++) {
    nhit->SetSample(isample,(int)(thit[isample]));
  }
  
  return nhit;
  
}

void NGMHit::SetCalibratedTime(NGMTimeStamp ts){ return ;}
NGMTimeStamp NGMHit::GetCalibratedTime() const {return NGMTimeStamp(0,0,0);}
NGMTimeStamp NGMHit::GetNGMTime() const {return NGMTimeStamp(0,0,0);}
void NGMHit::SetEnergy(float newVal) {return;}
float NGMHit::GetEnergy() const { return -9999.0; }
float NGMHit::GetNeutronId() const { return -9999.0; }
void NGMHit::SetNeutronId(float newVal) { return; }
float NGMHit::GetGammaId() const { return -9999.0; }
void NGMHit::SetGammaId(float newVal) { return; }
float NGMHit::GetBlockX() const { return -9999.0; }
void NGMHit::SetBlockX(float newVal){ return;}
float NGMHit::GetBlockY() const { return -9999.0; }
void NGMHit::SetBlockY(float newVal){ return;}

TGraph* NGMHit::GetGraph() const
{
    TGraph* tg = new TGraph(GetNSamples());
    for(int isample = 0; isample<GetNSamples(); isample++)
    {
        tg->SetPoint(isample,isample,GetSample(isample));
    }
    return tg;
}
#ifdef NGMHITV1
ClassImp(NGMHitv1)

NGMHitv1::NGMHitv1()
{
  _slot = -1;
  _channel = -1;
  _pulseHeight = 0.0;
  _triggerTime.Set();
  _pileupcounter = -1;
  _waveform = 0;
  for(int id = 0; id < 8; id++) _gate[id] = 0;
}

NGMHitv1::~NGMHitv1()
{
  if(_waveform) delete _waveform;
  _waveform = 0;
}


void NGMHitv1::SetTimeStamp(TTimeStamp newVal)
{
  _triggerTime = newVal;
}

NGMWaveform* NGMHitv1::CreateWaveform()
{
  if(_waveform) return _waveform;
  _waveform = new NGMWaveformv1;
  return _waveform;
}

void NGMHitv1::DeleteWaveform()
{
  if(!_waveform) return;
  delete _waveform;
  _waveform = 0;
}


NGMHit* NGMHitv1::DuplicateHit() const
{
  NGMHit* tHit = new NGMHitv1;
  tHit->CopyHit(this);
  return tHit;
}

Int_t NGMHitv1::Compare(const TObject* other) const{
  // Compare to another object -1 lt, 0 eq, 1 gt
  cerr << "NGMHitv1::Compare other = " << other << endl;
  const NGMHit& otherHit = *((const NGMHit*) other);
  if(_triggerTime < otherHit.GetTimeStamp() ) return -1;
  if(_triggerTime > otherHit.GetTimeStamp() ) return 1;
  
  return 0;
}

int NGMHitv1::GetGate(int index) const {
  if(index>-1 && index < 8)
    return _gate[index];
  return 0;
}

void NGMHitv1::SetGate(int index, int newVal){
  if(index>-1 && index < 8)
     _gate[index] = newVal;
  return;
}

int NGMHitv1::GetNSamples() const
{
	if(_waveform)
	{
		return _waveform->GetNSamples();
	};
	return 0;
}
void NGMHitv1::SetNSamples(int newVal)
{
	if(_waveform)
		_waveform->SetNSamples(newVal);
}
  
int NGMHitv1::GetSample(int isample) const
{
	if(_waveform)
		return _waveform->GetSample(isample);
	return 0;
}
void NGMHitv1::SetSample(int isample, int newVal)
{
	if(_waveform)
		_waveform->SetSample(isample,newVal);
}

void NGMHitv1::SetPileUpCounter(int newVal){
	_pileupcounter = newVal;
}

int NGMHitv1::GetPileUpCounter() const {
	return _pileupcounter;
}

NGMTimeStamp NGMHitv1::GetNGMTime() const {return NGMTimeStamp(GetTimeStamp());}

#endif
#ifdef NGMHITV2
// NGMHitv2 Implementation
ClassImp(NGMHitv2)

NGMHitv2::NGMHitv2()
: _waveform()
{
  _slot = -1;
  _channel = -1;
  _pulseHeight = 0.0;
  _triggerTime.Set();
  _pileupcounter = 0;
  _cfdtiming = 0;

  for(int id = 0; id < 8; id++) _gate[id] = 0;
}

NGMHitv2::~NGMHitv2()
{
}


void NGMHitv2::SetTimeStamp(TTimeStamp newVal)
{
  _triggerTime = newVal;
}



NGMHit* NGMHitv2::DuplicateHit() const
{
  NGMHit* tHit = new NGMHitv2(*this);
  return tHit;
}

Int_t NGMHitv2::Compare(const TObject* other) const{
  // Compare to another object -1 lt, 0 eq, 1 gt
  cerr << "NGMHitv2::Compare other = " << other << endl;
  const NGMHit& otherHit = *((const NGMHit*) other);
  if(_triggerTime < otherHit.GetTimeStamp() ) return -1;
  if(_triggerTime > otherHit.GetTimeStamp() ) return 1;
  
  return 0;
}
NGMTimeStamp NGMHitv2::GetNGMTime() const {
  // Later we mayt modify this to include CFDWord information
  return NGMTimeStamp(GetTimeStamp());
}

int NGMHitv2::GetGate(int index) const {
  if(index>-1 && index < 8)
    return _gate[index];
  return 0;
}

void NGMHitv2::SetGate(int index, int newVal){
  if(index>-1 && index < 8)
     _gate[index] = newVal;
  return;
}

int NGMHitv2::GetNSamples() const
{
	return _waveform.GetSize();
}
void NGMHitv2::SetNSamples(int newVal)
{
	_waveform.Set(newVal);
}
  
int NGMHitv2::GetSample(int isample) const
{
	return _waveform[isample];
}
void NGMHitv2::SetSample(int isample, int newVal)
{
	_waveform[isample] = newVal;
}

void NGMHitv2::SetPileUpCounter(int newVal){
	_pileupcounter = newVal;
}

int NGMHitv2::GetPileUpCounter() const {
	return _pileupcounter;
}

void NGMHitv2::CopyHit(const NGMHit* hit){
  
  static TClass* tNGMHitv2Type = gROOT->GetClass("NGMHitv2"); 

  // Cannot do anything with an null pointer
  if(hit == 0) return;
  
  if(hit->InheritsFrom(tNGMHitv2Type)){
    // Use the optimized assignment operator
    *this = *(const NGMHitv2*)hit;
  }else{
    // Use the brute force copy
    NGMHit::CopyHit(hit);
  }
}
#endif
#ifdef NGMHITV3
ClassImp(NGMHitv3)

NGMHitv3::NGMHitv3()
: _calTime(0,0,0)
{
  _energy = 0.0;
  _neutronSigma = 0.0;
  _gammaSigma = 0.0;
}

Int_t NGMHitv3::Compare(const TObject* other) const{
  cerr << "NGMHitv3::Compare other = " << other << endl;
  const NGMHit* otherHit = (const NGMHit*) other;
  // Compare to another object -1 lt, 0 eq, 1 gt
  if(GetNGMTime() < otherHit->GetNGMTime() ) return -1;
  if(GetNGMTime() > otherHit->GetNGMTime() ) return 1;
  return 0;
}

NGMTimeStamp NGMHitv3::GetNGMTime() const {
  // Check for empty calibrated time
  // if so return raw time.
  if(GetCalibratedTime().GetSec() == 0)
    return NGMTimeStamp(GetTimeStamp());
  return GetCalibratedTime();
}

void NGMHitv3::SetCalibratedTime(NGMTimeStamp ts)
{
  _calTime = ts;
  return ;
}

NGMTimeStamp NGMHitv3::GetCalibratedTime() const
{
  return _calTime;
}

void NGMHitv3::SetEnergy(float newVal)
{
  _energy = newVal;
}

float NGMHitv3::GetEnergy() const {
  return _energy;
}

float NGMHitv3::GetNeutronId() const {
  return _neutronSigma;
}

void NGMHitv3::SetNeutronId(float newVal) {
  _neutronSigma = newVal;
}
float NGMHitv3::GetGammaId() const {
  return _gammaSigma;
}
void NGMHitv3::SetGammaId(float newVal) {
  _gammaSigma = newVal;
}

NGMHit* NGMHitv3::DuplicateHit() const
{
  NGMHit* tHit = new NGMHitv3(*this);
  return tHit;
}

void NGMHitv3::CopyHit(const NGMHit* hit){
  
  //unused:  static TClass* tNGMHitv2Type = gROOT->GetClass("NGMHitv2"); 
  static TClass* tNGMHitv3Type = gROOT->GetClass("NGMHitv3"); 
  
  // Cannot do anything with an null pointer
  if(hit == 0) return;
  
  if( hit->IsA() == tNGMHitv3Type){
    // Use the optimized assignment operator
    *this = *(const NGMHitv3*)hit;
  }else{
    // Use the brute force copy
    NGMHit::CopyHit(hit);
  }
}
#endif
// Begin version NGMHitv4 and NGMHitv5
// NGMHitv4 Implementation
ClassImp(NGMHitv4)

NGMHitv4::NGMHitv4()
: _waveform()
{
  _slot = -1;
  _channel = -1;
  _pulseHeight = 0.0;
  _triggerTime.Set();
  _pileupcounter = 0;
  _cfdtiming = 0;

  for(int id = 0; id < 8; id++) _gate[id] = 0;
}

NGMHitv4::~NGMHitv4()
{
}


void NGMHitv4::SetTimeStamp(TTimeStamp newVal)
{
  _triggerTime = NGMTimeStamp(newVal);
}

void NGMHitv4::SetRawTime(NGMTimeStamp newVal)
{
  _triggerTime = newVal;
}


NGMHit* NGMHitv4::DuplicateHit() const
{
  NGMHit* tHit = new NGMHitv4(*this);
  return tHit;
}

Int_t NGMHitv4::Compare(const TObject* other) const{
  // Compare to another object -1 lt, 0 eq, 1 gt
  const NGMHit& otherHit = *((const NGMHit*) other);
  if(_triggerTime < otherHit.GetNGMTime() ) return -1;
  if(_triggerTime > otherHit.GetNGMTime() ) return 1;
  
  return 0;
}
NGMTimeStamp NGMHitv4::GetNGMTime() const {
  // Later we mayt modify this to include CFDWord information
  return _triggerTime;
}

NGMTimeStamp& NGMHitv4::GetNGMTime() {
    // Later we mayt modify this to include CFDWord information
    return _triggerTime;
}

int NGMHitv4::GetGate(int index) const {
  if(index>-1 && index < 8)
    return _gate[index];
  return 0;
}

void NGMHitv4::SetGate(int index, int newVal){
  if(index>-1 && index < 8)
     _gate[index] = newVal;
  return;
}

int NGMHitv4::GetNSamples() const
{
	return _waveform.GetSize();
}
void NGMHitv4::SetNSamples(int newVal)
{
	_waveform.Set(newVal);
}
  
int NGMHitv4::GetSample(int isample) const
{
	return _waveform[isample];
}
void NGMHitv4::SetSample(int isample, int newVal)
{
	_waveform[isample] = newVal;
}

void NGMHitv4::SetPileUpCounter(int newVal){
	_pileupcounter = newVal;
}

int NGMHitv4::GetPileUpCounter() const {
	return _pileupcounter;
}

void NGMHitv4::CopyHit(const NGMHit* hit){
  
  static TClass* tNGMHitv4Type = gROOT->GetClass("NGMHitv4"); 

  // Cannot do anything with an null pointer
  if(hit == 0) return;
  
  if(hit->InheritsFrom(tNGMHitv4Type)){
    // Use the optimized assignment operator
    *this = *(const NGMHitv4*)hit;
  }else{
    // Use the brute force copy
    NGMHit::CopyHit(hit);
  }
}

int NGMHitv4::getMaxValue() const 
{
  return TMath::MaxElement(_waveform.GetSize(),_waveform.GetArray());
}

ClassImp(NGMHitv5)

NGMHitv5::NGMHitv5()
: _calTime(0,0,0)
{
  _energy = 0.0;
  _neutronSigma = 0.0;
  _gammaSigma = 0.0;
}

Int_t NGMHitv5::Compare(const TObject* other) const{
  const NGMHit* otherHit = (const NGMHit*) other;
  // Compare to another object -1 lt, 0 eq, 1 gt
  if(_calTime < otherHit->GetNGMTime() ) return -1;
  if(_calTime > otherHit->GetNGMTime() ) return 1;
  return 0;
}

NGMTimeStamp NGMHitv5::GetNGMTime() const {
  // Check for empty calibrated time
  // if so return raw time.
  if(GetCalibratedTime().GetSec() == 0 )
    return GetRawTime();
  return GetCalibratedTime();
}

NGMTimeStamp& NGMHitv5::GetNGMTime() {
    return _calTime;
}

void NGMHitv5::SetCalibratedTime(NGMTimeStamp ts)
{
  _calTime = ts;
  return ;
}

NGMTimeStamp NGMHitv5::GetCalibratedTime() const
{
  return _calTime;
}

void NGMHitv5::SetEnergy(float newVal)
{
  _energy = newVal;
}

float NGMHitv5::GetEnergy() const {
  return _energy;
}

float NGMHitv5::GetNeutronId() const {
  return _neutronSigma;
}

void NGMHitv5::SetNeutronId(float newVal) {
  _neutronSigma = newVal;
}
float NGMHitv5::GetGammaId() const {
  return _gammaSigma;
}
void NGMHitv5::SetGammaId(float newVal) {
  _gammaSigma = newVal;
}

NGMHit* NGMHitv5::DuplicateHit() const
{
  NGMHit* tHit = new NGMHitv5(*this);
  return tHit;
}

void NGMHitv5::CopyHit(const NGMHit* hit){
  
  //unused:  static TClass* tNGMHitv4Type = gROOT->GetClass("NGMHitv4"); 
  static TClass* tNGMHitv5Type = gROOT->GetClass("NGMHitv5"); 
  
  // Cannot do anything with an null pointer
  if(hit == 0) return;
  
  if( hit->IsA() == tNGMHitv5Type){
    // Use the optimized assignment operator
    *this = *(const NGMHitv5*)hit;
  }else{
    // Use the brute force copy
    NGMHit::CopyHit(hit);
  }
}

ClassImp(NGMHitv6)

NGMHitv6::NGMHitv6()
: _calTime(0,0,0)
{
  _slot = -1;
  _channel = -1;
  _rawclock = 0;
  _cfdtiming = 0.0;
  _pulseHeight = 0.0;
  _energy = 0.0;
  _neutronSigma = 0.0;
  _gammaSigma = 0.0;
  _x = 0;
  _y = 0;
  _pileupcounter = 0;
  _pixel = -1;
}

NGMHitv6::NGMHitv6(int ngates)
: _calTime(0,0,0),_gate(ngates)
{
  _slot = -1;
  _channel = -1;
  _rawclock = 0;
  _cfdtiming = 0.0;
  _pulseHeight = 0.0;
  _energy = 0.0;
  _neutronSigma = 0.0;
  _gammaSigma = 0.0;
  _x = 0;
  _y = 0;
  _pileupcounter = 0;
  _pixel = -1;
}

Int_t NGMHitv6::Compare(const TObject* other) const{
  const NGMHit* otherHit = (const NGMHit*) other;
  // Compare to another object -1 lt, 0 eq, 1 gt
  if(_calTime < otherHit->GetNGMTime() ) return -1;
  if(_calTime > otherHit->GetNGMTime() ) return 1;
  return 0;
}

NGMTimeStamp NGMHitv6::GetNGMTime() const {
  // Check for empty calibrated time
  // if so return raw time.
  //if(GetCalibratedTime().GetSec() == 0 && )
  //  return GetRawTime();
  return GetCalibratedTime();
}

NGMTimeStamp& NGMHitv6::GetNGMTime() {
    // Check for empty calibrated time
    // if so return raw time.
    //if(GetCalibratedTime().GetSec() == 0 && )
    //  return GetRawTime();
    return _calTime;
}


void NGMHitv6::SetCalibratedTime(NGMTimeStamp ts)
{
  _calTime = ts;
  return ;
}

NGMTimeStamp NGMHitv6::GetCalibratedTime() const
{
  return _calTime;
}

int NGMHitv6::GetGate(int index) const {
  if(index>-1 && index < _gate.GetSize())
    return _gate[index];
  return 0;
}

void NGMHitv6::SetGate(int index, int newVal){
  
  if (_gate.GetSize()==0 || _gate.GetSize() <= index)
  {
    _gate.Set(index+1);
  }
  
  _gate[index] = newVal;

}

int NGMHitv6::GetNSamples() const
{
	return _waveform.GetSize();
}
void NGMHitv6::SetNSamples(int newVal)
{
	_waveform.Set(newVal);
}

int NGMHitv6::GetSample(int isample) const
{
	return _waveform[isample];
}
void NGMHitv6::SetSample(int isample, int newVal)
{
	_waveform[isample] = newVal;
}

void NGMHitv6::SetPileUpCounter(int newVal){
	_pileupcounter = newVal;
}

int NGMHitv6::GetPileUpCounter() const {
	return _pileupcounter;
}

void NGMHitv6::SetEnergy(float newVal)
{
  _energy = newVal;
}

float NGMHitv6::GetEnergy() const {
  return _energy;
}

float NGMHitv6::GetNeutronId() const {
  return _neutronSigma;
}

void NGMHitv6::SetNeutronId(float newVal) {
  _neutronSigma = newVal;
}
float NGMHitv6::GetGammaId() const {
  return _gammaSigma;
}
void NGMHitv6::SetGammaId(float newVal) {
  _gammaSigma = newVal;
}
float NGMHitv6::GetBlockX() const {
  return _x;
}
void NGMHitv6::SetBlockX(float newVal) {
  _x = newVal;
}
float NGMHitv6::GetBlockY() const {
  return _y;
}
void NGMHitv6::SetBlockY(float newVal) {
  _y = newVal;
}
NGMHit* NGMHitv6::DuplicateHit() const
{
  NGMHit* tHit = new NGMHitv6(*this);
  return tHit;
}

int NGMHitv6::GetQuadGate(int index) const
{
  int ngatesPerPMT=_gate.GetSize()/4;
  return GetGate(index)
  + GetGate(index+ngatesPerPMT)
  + GetGate(index+2*ngatesPerPMT)
  + GetGate(index+3*ngatesPerPMT);
}

int NGMHitv6::getMaxValue() const
{
  return TMath::MaxElement(_waveform.GetSize(),_waveform.GetArray());
}

void NGMHitv6::CopyHit(const NGMHit* hit){
  
  static TClass* tNGMHitv6Type = gROOT->GetClass("NGMHitv6"); 
  
  // Cannot do anything with an null pointer
  if(hit == 0) return;
  
  if( hit->IsA() == tNGMHitv6Type){
    // Use the optimized assignment operator
    *this = *(const NGMHitv6*)hit;
  }else{
    // Use the brute force copy
    NGMHit::CopyHit(hit);
  }
}

//Class NGMHitv7

ClassImp(NGMHitv7)

NGMHitv7::NGMHitv7()
:  _timestamp(0,0,0)
{
    _energy = 0.0;
    _neutronSigma = 0.0;
    _gammaSigma = 0.0;
    _pileupcounter = 0;
    _pixel = -1;

}

NGMTimeStamp NGMHitv7::GetNGMTime() const
{
    return _timestamp;
}

NGMTimeStamp& NGMHitv7::GetNGMTime()
{
    return _timestamp;
}

Int_t NGMHitv7::Compare(const TObject* other) const{
    const NGMHit* otherHit = (const NGMHit*) other;
    // Compare to another object -1 lt, 0 eq, 1 gt
    if(_timestamp < otherHit->GetNGMTime() ) return -1;
    if(_timestamp > otherHit->GetNGMTime() ) return 1;
    return 0;
}

void NGMHitv7::SetPileUpCounter(int newVal){
	_pileupcounter = newVal;
}

int NGMHitv7::GetPileUpCounter() const {
	return _pileupcounter;
}

void NGMHitv7::SetEnergy(float newVal)
{
    _energy = newVal;
}

float NGMHitv7::GetEnergy() const {
    return _energy;
}

float NGMHitv7::GetNeutronId() const {
    return _neutronSigma;
}

void NGMHitv7::SetNeutronId(float newVal) {
    _neutronSigma = newVal;
}
float NGMHitv7::GetGammaId() const {
    return _gammaSigma;
}
void NGMHitv7::SetGammaId(float newVal) {
    _gammaSigma = newVal;
}

NGMHit* NGMHitv7::DuplicateHit() const
{
    NGMHit* tHit = new NGMHitv7(*this);
    return tHit;
}

void NGMHitv7::CopyHit(const NGMHit* hit){
    
    // Cannot do anything with an null pointer
    if(hit == 0) return;
    SetPileUpCounter(hit->GetPileUpCounter());
    SetEnergy(hit->GetEnergy());
    SetNeutronId(hit->GetNeutronId());
    SetGammaId(hit->GetGammaId());
    SetPSD(hit->GetPSD());
    SetPixel(hit->GetPixel());
    GetNGMTime()=hit->GetNGMTime();
}



//Class NGMHitv8 -- for Stanford charge-readout setup
ClassImp(NGMHitv8)

NGMHitv8::NGMHitv8()
: _calTime(0,0,0)
{
  _slot = -1;
  _channel = -1;
  _rawclock = 0;
  _cfdtiming = 0.0;
  _pulseHeight = 0.0;
  _energy = 0.0;
  _neutronSigma = 0.0;
  _gammaSigma = 0.0;
  _x = 0;
  _y = 0;
  _pileupcounter = 0;
  _pixel = -1;
  _waveform.reserve(1000); // AGS
}

NGMHitv8::NGMHitv8(int ngates)
: _calTime(0,0,0),_gate(ngates)
{
  _slot = -1;
  _channel = -1;
  _rawclock = 0;
  _cfdtiming = 0.0;
  _pulseHeight = 0.0;
  _energy = 0.0;
  _neutronSigma = 0.0;
  _gammaSigma = 0.0;
  _x = 0;
  _y = 0;
  _pileupcounter = 0;
  _pixel = -1;
  _waveform.reserve(1000); // AGS
}

Int_t NGMHitv8::Compare(const TObject* other) const{
  const NGMHit* otherHit = (const NGMHit*) other;
  // Compare to another object -1 lt, 0 eq, 1 gt
  if(_calTime < otherHit->GetNGMTime() ) return -1;
  if(_calTime > otherHit->GetNGMTime() ) return 1;
  return 0;
}

NGMTimeStamp NGMHitv8::GetNGMTime() const {
  // Check for empty calibrated time
  // if so return raw time.
  //if(GetCalibratedTime().GetSec() == 0 && )
  //  return GetRawTime();
  return GetCalibratedTime();
}

NGMTimeStamp& NGMHitv8::GetNGMTime() {
    // Check for empty calibrated time
    // if so return raw time.
    //if(GetCalibratedTime().GetSec() == 0 && )
    //  return GetRawTime();
    return _calTime;
}


void NGMHitv8::SetCalibratedTime(NGMTimeStamp ts)
{
  _calTime = ts;
  return ;
}

NGMTimeStamp NGMHitv8::GetCalibratedTime() const
{
  return _calTime;
}

int NGMHitv8::GetGate(int index) const {
  if(index>-1 && index < _gate.GetSize())
    return _gate[index];
  return 0;
}

void NGMHitv8::SetGate(int index, int newVal){
  
  if (_gate.GetSize()==0 || _gate.GetSize() <= index)
  {
    _gate.Set(index+1);
  }
  
  _gate[index] = newVal;

}

int NGMHitv8::GetNSamples() const
{
	return _waveform.size();
}
void NGMHitv8::SetNSamples(int newVal)
{
	_waveform.resize(newVal);
}

int NGMHitv8::GetSample(int isample) const
{
	return _waveform[isample];
}
void NGMHitv8::SetSample(int isample, int newVal)
{
	_waveform[isample] = newVal;
}

void NGMHitv8::SetPileUpCounter(int newVal){
	_pileupcounter = newVal;
}

int NGMHitv8::GetPileUpCounter() const {
	return _pileupcounter;
}

void NGMHitv8::SetEnergy(float newVal)
{
  _energy = newVal;
}

float NGMHitv8::GetEnergy() const {
  return _energy;
}

float NGMHitv8::GetNeutronId() const {
  return _neutronSigma;
}

void NGMHitv8::SetNeutronId(float newVal) {
  _neutronSigma = newVal;
}
float NGMHitv8::GetGammaId() const {
  return _gammaSigma;
}
void NGMHitv8::SetGammaId(float newVal) {
  _gammaSigma = newVal;
}
float NGMHitv8::GetBlockX() const {
  return _x;
}
void NGMHitv8::SetBlockX(float newVal) {
  _x = newVal;
}
float NGMHitv8::GetBlockY() const {
  return _y;
}
void NGMHitv8::SetBlockY(float newVal) {
  _y = newVal;
}
NGMHit* NGMHitv8::DuplicateHit() const
{
  NGMHit* tHit = new NGMHitv8(*this);
  return tHit;
}

int NGMHitv8::GetQuadGate(int index) const
{
  int ngatesPerPMT=_gate.GetSize()/4;
  return GetGate(index)
  + GetGate(index+ngatesPerPMT)
  + GetGate(index+2*ngatesPerPMT)
  + GetGate(index+3*ngatesPerPMT);
}

int NGMHitv8::getMaxValue() const
{
  return TMath::MaxElement(_waveform.size(), &(_waveform[0]));
}

void NGMHitv8::CopyHit(const NGMHit* hit){
  
  static TClass* tNGMHitv8Type = gROOT->GetClass("NGMHitv8"); 
  
  // Cannot do anything with an null pointer
  if(hit == 0) return;
  
  if( hit->IsA() == tNGMHitv8Type){
    // Use the optimized assignment operator
    *this = *(const NGMHitv8*)hit;
  }else{
    // Use the brute force copy
    NGMHit::CopyHit(hit);
  }
}


ClassImp(NGMHitPoolv6)
NGMHitPoolv6::NGMHitPoolv6()
{
}

NGMHitPoolv6::~NGMHitPoolv6()
{
    for(std::list<NGMHit*>::iterator itr = _hitreservoir.begin(); itr!=_hitreservoir.end(); ++itr)
    {
        delete *itr;
    }
}

NGMHit* NGMHitPoolv6::GetHit()
{
    NGMHit* retVal = 0;
    if(_hitreservoir.empty())
    {
        retVal = new NGMHitv6();
    }else{
        retVal = _hitreservoir.front();
        _hitreservoir.pop_front();
    }
    return retVal;
}
void NGMHitPoolv6::ReturnHit(NGMHit* hit)
{
    _hitreservoir.push_back(hit);
}

ClassImp(NGMHitPoolv5)

NGMHitPoolv5* NGMHitPoolv5::_ptr = 0;

NGMHitPoolv5::NGMHitPoolv5()
: a("NGMHitv5",1000000), cnt(1000000)
{
  for(int i = 0; i < a.GetSize();i++)
  {
    new (a[i]) NGMHitv5;
    cnt[i] = 0;
  }
}

NGMHitPoolv5* NGMHitPoolv5::instance()
{
  if(!_ptr)
  {
    _ptr = new NGMHitPoolv5();
  }
  return _ptr;
}
ClassImp(NGMHitRefv5)
