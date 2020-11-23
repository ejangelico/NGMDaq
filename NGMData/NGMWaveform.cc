#include "NGMWaveform.h"

ClassImp(NGMArrayS)

ClassImp(NGMArraySFixed)

ClassImp(NGMWaveform)

ClassImp(NGMWaveformv1)

NGMArraySFixed::NGMArraySFixed()
{
  _cnt =0;
}

int NGMArraySFixed::GetSize() const {
	return _cnt;
}
void NGMArraySFixed::Set(int newCnt){
	_cnt = newCnt;
}

void NGMArraySFixed::Set(int idx,short newVal){
	if(idx>0 && idx < _cnt)
		_array[idx] = newVal;
}

short NGMArraySFixed::At(int idx) const
{
	if(idx>0 && idx < _cnt)
		return _array[idx];
	return 0;
}


NGMWaveformv1::NGMWaveformv1()
{
  _t0 = 0;
  _dt = 0.0;
  _trigsample = 0;
  _sampleArray = 0;
}

NGMWaveformv1::~NGMWaveformv1()
{
  if(_sampleArray) delete _sampleArray;
}


int NGMWaveformv1::GetNSamples() const {
  if(!_sampleArray) return 0;
  return _sampleArray->GetSize();
}

void NGMWaveformv1::SetNSamples(int newVal){
  if(!_sampleArray) _sampleArray = new TArrayS(newVal);
  _sampleArray->Set(newVal);
}

short NGMWaveformv1::GetSample(int isample) const {
  if(!_sampleArray) return 0;
  return _sampleArray->At(isample);
}

void NGMWaveformv1::SetSample(int isample, short newVal) {
  if(!_sampleArray) return;
  (*_sampleArray)[isample]=newVal;

}

NGMWaveform* NGMWaveformv1::DuplicateWaveform(){
  NGMWaveformv1* tmpWave = new NGMWaveformv1;
  tmpWave->CopyWaveform(this);
  return tmpWave;
}

void NGMWaveform::CopyWaveform(const NGMWaveform* oldWave){
  SetT0(oldWave->GetT0());
  SetDt(oldWave->GetDt());
  SetTrig(oldWave->GetTrig());
  SetNSamples(oldWave->GetNSamples());
  for(int isample = 0; isample < GetNSamples(); isample++){
    SetSample(isample,oldWave->GetSample(isample));
  }
  return;
}
