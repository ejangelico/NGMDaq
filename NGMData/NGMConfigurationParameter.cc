#include "NGMConfigurationParameter.h"
#include "TObjArray.h"
#include "TArrayD.h"
#include "TArrayI.h"
#include "TObjString.h"
#include "NGMLogger.h"
#include <cstdlib>

ClassImp(NGMConfigurationParameter)

ClassImp(NGMConfigurationParameterv1)

void NGMConfigurationParameter::SetToDefault()
{
  //Call the Set Default appropriate for this object
  for(int ich = 0; ich < GetEntries(); ich++)
  {
    if(GetParameterType() == ParDouble)
      SetValue(ich, GetDefault());
    else if(GetParameterType() == ParInteger)
      SetValue(ich, GetDefaultI());
    else if(GetParameterType() == ParString)
      SetValue(ich, GetDefaultS());
  }
}

const char* NGMConfigurationParameter::GetValueAsString(int index) const
{
	static char cbuff[1024];

	if(GetParameterType() == ParDouble)
      sprintf(cbuff,"%f",GetValue(index));
    else if(GetParameterType() == ParInteger)
      sprintf(cbuff,"%d",GetValueI(index));
    else if(GetParameterType() == ParString)
      sprintf(cbuff,"%s",GetValueS(index));
    else {
	  //  sprintf(cbuff,"");
    }
	return cbuff;
}

void NGMConfigurationParameter::SetValueFromString(int index, const char* newVal)
{
	if(GetParameterType() == ParDouble)
	  SetValue(index, atof(newVal));
    else if(GetParameterType() == ParInteger)
      SetValue(index, atoi(newVal));
    else if(GetParameterType() == ParString)
      SetValue(index, newVal);
}

const char* NGMConfigurationParameter::GetDefaultAsString() const
{
	static char cbuff[1024];

	if(GetParameterType() == ParDouble)
      sprintf(cbuff,"%f",GetDefault());
    else if(GetParameterType() == ParInteger)
      sprintf(cbuff,"%d",GetDefaultI());
    else if(GetParameterType() == ParString)
      sprintf(cbuff,"%s",GetDefaultS());
	else
	  sprintf(cbuff,"NA");
	return cbuff;
}
const char* NGMConfigurationParameter::GetMinimumAsString() const
{
	static char cbuff[1024];

	if(GetParameterType() == ParDouble)
      sprintf(cbuff,"%f",GetMinimum());
    else if(GetParameterType() == ParInteger)
      sprintf(cbuff,"%s","NA");
    else if(GetParameterType() == ParString)
      sprintf(cbuff,"%s","NA");
	else
	  sprintf(cbuff,"NA");
	return cbuff;
}
const char* NGMConfigurationParameter::GetMaximumAsString() const
{
	static char cbuff[1024];

	if(GetParameterType() == ParDouble)
      sprintf(cbuff,"%f",GetMaximum());
    else if(GetParameterType() == ParInteger)
      sprintf(cbuff,"%s","NA");
    else if(GetParameterType() == ParString)
      sprintf(cbuff,"%s","NA");
	else
	  sprintf(cbuff,"NA");
	return cbuff;
}

const char* NGMConfigurationParameter::GetOperation() const {return "NA"; }
const char* NGMConfigurationParameter::GetDescription() const {return "NA";}
const char* NGMConfigurationParameter::GetFormat() const {return "NA";}

void NGMConfigurationParameter::SetOperation(const char* newVal){}
void NGMConfigurationParameter::SetDescription(const char* newVal){}
void NGMConfigurationParameter::SetFormat(const char* newVal){}


NGMConfigurationParameterv1::NGMConfigurationParameterv1()
{
  _parval = 0;
}

NGMConfigurationParameterv1::NGMConfigurationParameterv1(const char* parameterName, double defaultVal,
                                    double minVal, double maxVal,int nentries)
{
  _parameterName = parameterName;
  _defaultVal = defaultVal;
  _minimumVal = minVal;
  _maximumVal = maxVal;
  _nentries = nentries;
  if(_nentries<1) _nentries = 1;
  _parval = new TArrayD(_nentries);
}

///Constructor
NGMConfigurationParameterv1::NGMConfigurationParameterv1(int nentries){
  _nentries = nentries;
  if(_nentries<1) _nentries = 1;
    _parval = new TArrayD(_nentries);
}

NGMConfigurationParameterv1::~NGMConfigurationParameterv1(){
  if(_parval) delete _parval;
}

NGMConfigurationParameter* NGMConfigurationParameter::CreateCurrentVersion(int nentries, DataType wtype)
{
  if(wtype==ParDouble)
    return new NGMConfigurationParameterv1(nentries);
  if(wtype==ParInteger)
    return new NGMConfigurationParameterIv1(nentries);
  if(wtype==ParString)
    return new NGMConfigurationParameterSv1(nentries);
  if(wtype==ParObject)
    return new NGMConfigurationParameterOv1(nentries);
  else
    return 0;
}

void NGMConfigurationParameterv1::SetNRows(int newVal)
{
  _nentries = newVal;
  _parval->Set(newVal);
}

double NGMConfigurationParameterv1::GetValue(int index) const
{
  if(_parval&&index<_nentries&&index>=0)
    return _parval->At(index);
  LOG<<"Parameter "<<GetName()<<" index"<<index<< " out of range [0,"<<_nentries<<"]"<<ENDM_WARN;
  return 0.0;
}

void NGMConfigurationParameterv1::SetValue(int index, double newVal)
{
  if(_parval&&index<_nentries&&index>=0)
    (*_parval)[index] = newVal;
  else
    LOG<<"Parameter "<<GetName()<<" index"<<index<< " out of range [0,"<<_nentries<<"]"<<ENDM_WARN;
}

const char* NGMConfigurationParameterv1::GetOperation() const {return _operation; }
const char* NGMConfigurationParameterv1::GetDescription() const {return _description;}
const char* NGMConfigurationParameterv1::GetFormat() const {return _format;}

void NGMConfigurationParameterv1::SetOperation(const char* newVal){ _operation = newVal; }
void NGMConfigurationParameterv1::SetDescription(const char* newVal){ _description = newVal; }
void NGMConfigurationParameterv1::SetFormat(const char*newVal){ _format = newVal; }


ClassImp(NGMConfigurationParameterSv1)

NGMConfigurationParameterSv1::NGMConfigurationParameterSv1()
{
  _nentries = 0;
  _parval = 0;
}

void NGMConfigurationParameterSv1::SetToDefault()
{
  for(int ich = 0; ich < GetEntries(); ich++)
  {
    SetValue(ich, GetDefaultS());
  }
}


NGMConfigurationParameterSv1::NGMConfigurationParameterSv1(const char* parameterName, const char* defaultVal,
                                                           int nentries)
{
  _parameterName = parameterName;
  _defaultVal = defaultVal;
  _nentries = nentries;
  if(_nentries<1) _nentries = 1;
  _parval = new TObjArray(_nentries);
  _parval->SetLast(_nentries-1);
}

///Constructor
NGMConfigurationParameterSv1::NGMConfigurationParameterSv1(int nentries){
  _nentries = nentries;
  if(_nentries<1) _nentries = 1;
  _parval = new TObjArray(_nentries);
  _parval->SetLast(_nentries -1 );
  for(int itr = 0; itr < _nentries; itr++){
    (*_parval)[itr] = new TObjString("");
  }
}

NGMConfigurationParameterSv1::~NGMConfigurationParameterSv1(){
  if(_parval) delete _parval;
}

void NGMConfigurationParameterSv1::SetNRows(int newVal)
{
  _nentries = newVal;
  _parval->Expand(newVal);
  _parval->SetLast(_nentries -1);
  for(int irow = 0; irow < _nentries; irow++)
  {
    if(! _parval->At(irow))
      (*_parval)[irow] = new TObjString("");
  }
}

const char* NGMConfigurationParameterSv1::GetValueS(int index) const
{
  if(_parval&&index<_nentries&&index>=0){
    if(_parval->At(index))
      return _parval->At(index)->GetName();
    else
      return "";
  }
  if(_parval)
    LOG<<"Parameter "<<GetName()<<" index "<<index<< " out of range [0,"<<_nentries<<"]"<<ENDM_WARN;
  return "";
}

void NGMConfigurationParameterSv1::SetValue(int index, const char* newVal)
{
  if(_parval&&index<_nentries&&index>=0){
    TObjString* tStr = ((TObjString*)((*_parval)[index]));
    if(!tStr){
      tStr = new TObjString;
      (*_parval)[index] = tStr;
    }
    tStr->SetString(newVal);
  }else{
    LOG<<"Parameter "<<GetName()<<" index"<<index<< " out of range [0,"<<_nentries<<"]"<<ENDM_WARN;
  }
}

const char* NGMConfigurationParameterSv1::GetOperation() const {return _operation; }
const char* NGMConfigurationParameterSv1::GetDescription() const {return _description;}
const char* NGMConfigurationParameterSv1::GetFormat() const {return _format;}

void NGMConfigurationParameterSv1::SetOperation(const char* newVal){ _operation = newVal; }
void NGMConfigurationParameterSv1::SetDescription(const char* newVal){ _description = newVal; }
void NGMConfigurationParameterSv1::SetFormat(const char*newVal){ _format = newVal; }


ClassImp(NGMConfigurationParameterIv1)

void NGMConfigurationParameterIv1::SetToDefault()
{
  for(int ich = 0; ich < GetEntries(); ich++)
  {
    SetValue(ich, GetDefaultI());
  }
}


NGMConfigurationParameterIv1::NGMConfigurationParameterIv1()
{
  _nentries = 0;
  _parval = 0;
}


NGMConfigurationParameterIv1::NGMConfigurationParameterIv1(const char* parameterName, int defaultVal,
                                                           int minVal, int maxVal,int nentries)
{
  _parameterName = parameterName;
  _defaultVal = defaultVal;
  _nentries = nentries;
  if(_nentries<1) _nentries = 1;
  _parval = new TArrayI(_nentries);
}

///Constructor
NGMConfigurationParameterIv1::NGMConfigurationParameterIv1(int nentries){
  _nentries = nentries;
  if(_nentries<1) _nentries = 1;
  _parval = new TArrayI(_nentries);
}

NGMConfigurationParameterIv1::~NGMConfigurationParameterIv1(){
  if(_parval) delete _parval;
}

void NGMConfigurationParameterIv1::SetNRows(int newVal)
{
  _nentries = newVal;
  _parval->Set(newVal);
}

int NGMConfigurationParameterIv1::GetValueI(int index) const
{
  if(_parval&&index<_nentries&&index>=0)
    return _parval->At(index);
  
  LOG<<"Parameter "<<GetName()<<" index "<<index<< " out of range [0,"<<_nentries<<"]"<<ENDM_WARN;  
  return 0;
}

void NGMConfigurationParameterIv1::SetValue(int index, int newVal)
{
  if(_parval&&index<_nentries&&index>=0)
    (*_parval)[index] = newVal;
  else
    LOG<<"Parameter "<<GetName()<<" index"<<index<< " out of range [0,"<<_nentries<<"]"<<ENDM_WARN;
}

const char* NGMConfigurationParameterIv1::GetOperation() const {return _operation; }
const char* NGMConfigurationParameterIv1::GetDescription() const {return _description;}
const char* NGMConfigurationParameterIv1::GetFormat() const {return _format;}

void NGMConfigurationParameterIv1::SetOperation(const char* newVal){ _operation = newVal; }
void NGMConfigurationParameterIv1::SetDescription(const char* newVal){ _description = newVal; }
void NGMConfigurationParameterIv1::SetFormat(const char*newVal){ _format = newVal; }


ClassImp(NGMConfigurationParameterOv1)

NGMConfigurationParameterOv1::NGMConfigurationParameterOv1()
{
  _defaultVal = 0;
  _nentries = 0;
  _parval = 0;
}

void NGMConfigurationParameterOv1::SetToDefault()
{
  for(int ich = 0; ich < GetEntries(); ich++)
  {
    SetValue(ich, GetDefaultO());
  }
}


NGMConfigurationParameterOv1::NGMConfigurationParameterOv1(const char* parameterName,
                                                           TObject* defaultVal,
                                                           int nentries)
{
  _parameterName = parameterName;
  _defaultVal = defaultVal;
  _nentries = nentries;
  if(_nentries<1) _nentries = 1;
  _parval = new TObjArray(_nentries);
  _parval->SetLast(_nentries-1);
}

///Constructor
NGMConfigurationParameterOv1::NGMConfigurationParameterOv1(int nentries){
  _nentries = nentries;
  if(_nentries<1) _nentries = 1;
  _parval = new TObjArray(_nentries);
  _parval->SetLast(_nentries -1 );
  //for(int itr = 0; itr < _nentries; itr++){
  //  (*_parval)[itr] = new TObjString("");
  //}
}

NGMConfigurationParameterOv1::~NGMConfigurationParameterOv1(){
  if(_parval) delete _parval;
  // We have to think about how to clean this up
}

void NGMConfigurationParameterOv1::SetNRows(int newVal)
{
  _nentries = newVal;
  _parval->Expand(newVal);
  _parval->SetLast(_nentries -1);
//  for(int irow = 0; irow < _nentries; irow++)
//  {
////    if(! _parval->At(irow))
////      (*_parval)[irow] = new TObjString("");
//  }
}

const TObject* NGMConfigurationParameterOv1::GetValueO(int index) const
{
  if(_parval&&index<_nentries&&index>=0){
    if(_parval->At(index))
      return _parval->At(index);
    else
      return 0;
  }
  if(_parval)
    LOG<<"Parameter "<<GetName()<<" index "<<index<< " out of range [0,"<<_nentries<<"]"<<ENDM_WARN;
  return 0;
}


TObject* NGMConfigurationParameterOv1::GetValueO(int index)
{
    if(_parval&&index<_nentries&&index>=0){
        if(_parval->At(index))
            return _parval->At(index);
        else
            return 0;
    }
    if(_parval)
        LOG<<"Parameter "<<GetName()<<" index "<<index<< " out of range [0,"<<_nentries<<"]"<<ENDM_WARN;
    return 0;
}


void NGMConfigurationParameterOv1::SetValue(int index, TObject* newVal)
{
  if(_parval&&index<_nentries&&index>=0){
    // Need to think about how to clean up the old value
    (*_parval)[index] = newVal;
  }else{
    LOG<<"Parameter "<<GetName()<<" index"<<index<< " out of range [0,"<<_nentries<<"]"<<ENDM_WARN;
  }
}

const char* NGMConfigurationParameterOv1::GetOperation() const {return _operation; }
const char* NGMConfigurationParameterOv1::GetDescription() const {return _description;}
const char* NGMConfigurationParameterOv1::GetFormat() const {return _format;}

void NGMConfigurationParameterOv1::SetOperation(const char* newVal){ _operation = newVal; }
void NGMConfigurationParameterOv1::SetDescription(const char* newVal){ _description = newVal; }
void NGMConfigurationParameterOv1::SetFormat(const char*newVal){ _format = newVal; }

