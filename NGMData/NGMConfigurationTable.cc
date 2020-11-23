#include "NGMConfigurationTable.h"
#include "NGMConfigurationParameter.h"
#include "NGMLogger.h"
#include "TDirectory.h"
#include <vector>
#include <string.h>
#include <sstream>
#include <cstdlib>

ClassImp(NGMConfigurationTable)

NGMConfigurationTable::NGMConfigurationTable(){
}


NGMConfigurationTable* NGMConfigurationTable::CreateCurrentVersionTable(int nentries){
  return new NGMConfigurationTablev1(nentries,false);
}

NGMConfigurationTable* NGMConfigurationTable::CreateCurrentVersionList(){
  return new NGMConfigurationTablev1(1,true);
}

ClassImp(NGMConfigurationTablev1)

NGMConfigurationTablev1::NGMConfigurationTablev1(){
  _isList = false;
  _nentries = 0;
  _numparameters=0;
  _parTable=0;
  _quiet = false;
}

NGMConfigurationTablev1::NGMConfigurationTablev1(int nentries, bool isList){
  _isList = isList;
  _numparameters=0;
  _nentries = nentries;
  _parTable= 0;
  _quiet = false;
}

NGMConfigurationTablev1::~NGMConfigurationTablev1(){
  if(_parTable){
    for(int ipar = 0; ipar <= _parTable->GetLast(); ipar++){
      NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar));
      if(!par) continue;
      delete par;
    }
    delete _parTable;
  }
}

void NGMConfigurationTablev1::RemoveParameter(const char* parameterName)
{
  if(!_parTable)
  {
    _parTable = new TObjArray;
    return ;
  }
  NGMConfigurationParameter* par = GetColumn(parameterName);
  if(par)
  {
    LOG<<"Remove Parameter "<< parameterName << ENDM_INFO;
    _parTable->Remove(par);
    // Remove empty entry in array and shift cells up
    _parTable->Compress();
    // Update total number of parameters
    _numparameters = _parTable->GetLast() +1;
  }

}

void NGMConfigurationTablev1::RemoveParameter(int parIndex)
{
    NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(parIndex));
    if(par)
    {
        LOG<<"Remove Parameter at index "<< parIndex << ENDM_INFO;
        _parTable->Remove(par);
        // Remove empty entry in array and shift cells up
        _parTable->Compress();
        // Update total number of parameters
        _numparameters = _parTable->GetLast() +1;
    }
    
}

const NGMConfigurationParameter* NGMConfigurationTablev1::GetColumn(const char* parName) const
{
  int ipar = GetParIndex(parName);
    
  // needs error logging
  if(!(ipar >= 0 && ipar < _numparameters)){
    LOG<<" Error parameter "<<parName<<" not found!"<<ENDM_WARN;
    return 0;
  }
  
  return (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar))); 	
}

NGMConfigurationParameter* NGMConfigurationTablev1::GetColumn(const char* parName)
{
  int ipar = GetParIndex(parName);
    
  // needs error logging
  if(!(ipar >= 0 && ipar < _numparameters)){
    LOG<<" Error parameter "<<parName<<" not found!"<<ENDM_WARN;
    return 0;
  }
  
  return (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)));
}


NGMConfigurationParameter::DataType NGMConfigurationTablev1::GetParType(const char* parName) const
{
  int ipar = GetParIndex(parName);
    
  // needs error logging
  if(!(ipar >= 0 && ipar < _numparameters)){
    LOG<<" Error parameter "<<parName<<" not found!"<<ENDM_WARN;
    return NGMConfigurationParameter::ParDouble;
  }
  
  return (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->GetParameterType();
  
}
void NGMConfigurationTablev1::AddParameterD(const char* parameterName, double defaultVal,
                                      double minVal, double maxVal, int nentries){
  if(GetParIndex(parameterName)>=0) return;
  _numparameters++;
  if(!_isList) nentries = _nentries;
  NGMConfigurationParameter* tPar = NGMConfigurationParameter::CreateCurrentVersion(nentries,
                                                                                    NGMConfigurationParameter::ParDouble);
  if(!_parTable)
  {
    _parTable = new TObjArray;
  }
  _parTable->Add(tPar);
  tPar->SetName(parameterName);
  tPar->SetDefault(defaultVal);
  tPar->SetMinimum(minVal);
  tPar->SetMaximum(maxVal);
}
void NGMConfigurationTablev1::AddParameterI(const char* parameterName, int defaultVal,
                                            int minVal, int maxVal, int nentries){
  if(GetParIndex(parameterName)>=0) return;
  _numparameters++;
  if(!_isList) nentries = _nentries;
  NGMConfigurationParameter* tPar = NGMConfigurationParameter::CreateCurrentVersion(nentries,NGMConfigurationParameter::ParInteger);
  if(!_parTable)
  {
    _parTable = new TObjArray;
  }
  _parTable->Add(tPar);
  tPar->SetName(parameterName);
  tPar->SetDefault(defaultVal);
  tPar->SetMinimum(minVal);
  tPar->SetMaximum(maxVal);
}

void NGMConfigurationTablev1::AddParameterS(const char* parameterName, const char* defaultVal, int nentries){
  if(GetParIndex(parameterName)>=0) return;
  _numparameters++;
  if(!_isList) nentries = _nentries;
  NGMConfigurationParameter* tPar = NGMConfigurationParameter::CreateCurrentVersion(nentries,NGMConfigurationParameter::ParString);
  if(!_parTable)
  {
    _parTable = new TObjArray;
  }
  _parTable->Add(tPar);
  tPar->SetName(parameterName);
  tPar->SetDefault(defaultVal);
}

void NGMConfigurationTablev1::AddParameterO(const char* parameterName, TObject* defaultVal, int nentries){
  if(GetParIndex(parameterName)>=0) return;
  _numparameters++;
  if(!_isList) nentries = _nentries;
  NGMConfigurationParameter* tPar = NGMConfigurationParameter::CreateCurrentVersion(nentries,NGMConfigurationParameter::ParObject);
  if(!_parTable)
  {
    _parTable = new TObjArray;
  }
  _parTable->Add(tPar);
  tPar->SetName(parameterName);
  tPar->SetDefault(defaultVal);
}

void NGMConfigurationTablev1::SetParameterDefaultD(const char *parName, double newVal)
{
  // Sanity check
  if(!_parTable) return;
  TString tsName = parName;
  for(int ipar = 0; ipar <= _parTable->GetLast(); ipar++)
  {
    NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar));
    if(!par) continue;
    // Should generate fatal error here!!!
    if(par->GetName() == tsName)
    {
		if(par->GetParameterType() == NGMConfigurationParameter::ParDouble){
			par->SetDefault(newVal);
		}
      break;
    }
  }


}

void NGMConfigurationTablev1::SetParameterToDefault(const char* parName)
{
  // Sanity check
  if(!_parTable) return;
  TString tsName = parName;
  for(int ipar = 0; ipar <= _parTable->GetLast(); ipar++)
  {
    NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar));
    if(!par) continue;
    // Should generate fatal error here!!!
    if(par->GetName() == tsName)
    {
      par->SetToDefault();
      break;
    }
  }
  
}

void NGMConfigurationTablev1::SetAllParametersToDefault(){
  // Sanity check
  if(!_parTable) return;
  for(int ipar = 0; ipar <= _parTable->GetLast(); ipar++)
  {
    NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar));
    if(!par) continue;
    // Should generate fatal error here!!!
    par->SetToDefault();
  }
}

int NGMConfigurationTablev1::GetParIndex(const char* parName) const {
  if(!_parTable) return -1;
  TString tsName = parName;
  for(int ipar = 0; ipar <= _parTable->GetLast(); ipar++)
  {
    NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar));
    if(!par) continue;
    if(par->GetName() == tsName)
    {
      return ipar;
    }
  }
  //if(!GetQuiet()) LOG<<"Parameter \""<<parName<<" not found"<<ENDM_WARN;
  return -1;
}

void NGMConfigurationTablev1::SetParameterD(const char* parName, int index, double newVal){
  int ipar = GetParIndex(parName);
  if(ipar<0) return;
  if(ipar >= 0 && ipar < _numparameters){
    if(!GetQuiet())
		LOG<<"SetParameter "<<parName
		<<" from "<<(dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->GetValue(index)
		<<" to "<<newVal<< ENDM_INFO;
    (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->SetValue(index, newVal);
  }
}

void NGMConfigurationTablev1::SetParameterI(const char* parName, int index, int newVal){
  int ipar = GetParIndex(parName);
  if(ipar<0) return;
  if(ipar >= 0 && ipar < _numparameters){
    if(!GetQuiet())
		LOG<<"SetParameter "<<parName
		<<" from "<<(dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->GetValueI(index)
		<<" to "<<newVal<< ENDM_INFO;
    (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->SetValue(index, newVal);
  }
}

void NGMConfigurationTablev1::SetParameterS(const char* parName, int index, const char* newVal){
  int ipar = GetParIndex(parName);
    if(ipar<0){
        LOG<<"Parameter "<<parName<<" does not exist "<<ENDM_WARN;
        return;
    }
  
  if(ipar >= 0 && ipar < _numparameters){
    if(!GetQuiet())
		LOG<<"SetParameter "<<parName
		<<" from "<<(dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->GetValueS(index)
		<<" to "<<newVal<< ENDM_INFO;
    (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->SetValue(index, newVal);
  }
}

void NGMConfigurationTablev1::SetParameterO(const char* parName, int index, TObject* newVal){
  int ipar = GetParIndex(parName);
  if(ipar<0) return;
  
  if(ipar >= 0 && ipar < _numparameters){
    if(!GetQuiet())
      LOG<<"Update Parameter "<<parName<<ENDM_INFO;

    (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->SetValue(index, newVal);
  }
}

double NGMConfigurationTablev1::GetParValueD(const char* parName, int index) const {
  int ipar = GetParIndex(parName);
  if(ipar<0) return 0.0;
  NGMConfigurationParameter* tPar = (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)));
  
  // needs error logging
  if(!(ipar >= 0 && ipar < _numparameters)){
    return 0.0;
  }
  return tPar->GetValue(index);
}

int NGMConfigurationTablev1::GetParValueI(const char* parName, int index) const {
  int ipar = GetParIndex(parName);
  if(ipar<0) return 0;
  
  // needs error logging
  if(!(ipar >= 0 && ipar < _numparameters)){
    return 0;
  }
  return (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->GetValueI(index);
}

const char* NGMConfigurationTablev1::GetParValueS(const char* parName, int index) const {
  int ipar = GetParIndex(parName);
  if(ipar<0) return "";
  
  // needs error logging
  if(!(ipar >= 0 && ipar < _numparameters)){
    return "";
  }
  return (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->GetValueS(index);
}

const TObject* NGMConfigurationTablev1::GetParValueO(const char* parName, int index) const {
  int ipar = GetParIndex(parName);
  if(ipar<0) return 0;
  
  // needs error logging
  if(!(ipar >= 0 && ipar < _numparameters)){
    return 0;
  }
  return (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->GetValueO(index);
}

TObject* NGMConfigurationTablev1::GetParValueO(const char* parName, int index) {
    int ipar = GetParIndex(parName);
    if(ipar<0) return 0;
    
    // needs error logging
    if(!(ipar >= 0 && ipar < _numparameters)){
        return 0;
    }
    return (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->GetValueO(index);
}


const char* NGMConfigurationTablev1::GetParName(int parindex) const{
  // Sanity check
  // Needs error logging
  if(!_parTable) return "";
 
  if(parindex<0 || parindex >= _numparameters){
    LOG<<"Parameter index "<<parindex<<" out of range [0,"<<_numparameters-1<<"]"<<ENDM_WARN;
  }

  NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(parindex));
  //Needs error logging
  if(!par){
    LOG<<"Parameter index "<<parindex<<" contains null pointer "<<ENDM_WARN;
    return "";
  }
  return par->GetName();
}

int NGMConfigurationTablev1::GetEntries(const char* parName) const
{
  if(_isList){
    int ipar = GetParIndex(parName);
    if(ipar<0) return 0;
    return (dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar)))->GetEntries();
  }else{
    return _nentries;
  }
  return 0;
}

void NGMConfigurationTablev1::PrintTable(std::ostream &ostr)
{
  // Sanity check
  if(!_parTable) return;

  for(int ipar = 0; ipar <= _parTable->GetLast(); ipar++)
  {
    NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar));
    if(!par) continue;
    LOG<<par->GetName()<<"\t";
  }
  LOG<<ENDM_INFO;
  for(int ientry = 0; ientry < _nentries; ientry++){
    for(int ipar = 0; ipar <= _parTable->GetLast(); ipar++)
    {
      NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar));
      if(!par) continue;
      if(par->GetParameterType() == NGMConfigurationParameter::ParDouble)
        LOG<<par->GetValue(ientry)<<"\t";
      else if(par->GetParameterType() == NGMConfigurationParameter::ParInteger)
        LOG<<par->GetValueI(ientry)<<"\t";
      else if(par->GetParameterType() == NGMConfigurationParameter::ParString)
        LOG<<par->GetValueS(ientry)<<"\t";
    }
    LOG<<ENDM_INFO;
  }
  return;
}

void NGMConfigurationTablev1::PrintRow(int ientry, std::ostream &ostr)
{
	 // Sanity check
  if(!_parTable) return;
  LOG<<"\n";
  for(int ipar = 0; ipar <= _parTable->GetLast(); ipar++)
  {
    NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar));
    if(!par) continue;
    LOG<<par->GetName()<<"\t";
	if(par->GetParameterType() == NGMConfigurationParameter::ParDouble)
        LOG<<par->GetValue(ientry)<<"\n";
	else if(par->GetParameterType() == NGMConfigurationParameter::ParInteger)
        LOG<<par->GetValueI(ientry)<<"\n";
    else if(par->GetParameterType() == NGMConfigurationParameter::ParString)
        LOG<<par->GetValueS(ientry)<<"\n";
    else if(par->GetParameterType() == NGMConfigurationParameter::ParObject)
      if(par->GetValueO(ientry)) LOG<<par->GetValueO(ientry)->GetName()<<"\n";
  }
  LOG<<"\n";
  LOG<<ENDM_INFO;
}

void NGMConfigurationTablev1::SetNRows(int newVal)
{

  _nentries = newVal;

  // Sanity check
  if(!_parTable) return;

  for(int ipar = 0; ipar <= _parTable->GetLast(); ipar++)
  {
    NGMConfigurationParameter* par = dynamic_cast<NGMConfigurationParameter*>(_parTable->At(ipar));
    if(!par) continue;
    par->SetNRows(newVal);
  }
}


void NGMConfigurationTablev1::SetParameterFromString(const char* parName, int index, const char* newVal)
{
	NGMConfigurationParameter* tPar = GetColumn(parName);
	if(tPar)
		tPar->SetValueFromString(index, newVal);
	else
		LOG<<"Parameter "<<parName<<" not found"<<ENDM_WARN;
}

const char* NGMConfigurationTablev1::GetParameterAsString(const char* parName, int index) const
{
	const NGMConfigurationParameter* tPar = GetColumn(parName);
	if(tPar)
		return tPar->GetValueAsString(index);

	LOG<<"Parameter "<<parName<<" not found"<<ENDM_WARN;
	return "";
}

void NGMConfigurationTablev1::SetParameterAsStringThatBeginsWith(const char* parName,
    const char* newVal, const char* matchingfield, const char* matchingVal)
{
  int rowsmodified = 0;
  if(GetParIndex(parName)>=0 && GetParIndex(matchingfield)>=0)
  {
    for(int irow = 0; irow < GetEntries(); irow++)
    {
      if(TString(GetParameterAsString(matchingfield,irow)).BeginsWith(matchingVal))
      {
        SetParameterFromString(parName,irow,newVal);
        rowsmodified++;
      }
    }
  }
  if(!GetQuiet()) 
    LOG<<rowsmodified<<" rows modified."<<ENDM_INFO;

  return;
}

void NGMConfigurationTablev1::SetParameterAsStringThatContains(const char* parName,
    const char* newVal, const char* matchingfield, const char* matchingVal)
{
  int rowsmodified = 0;
  if(GetParIndex(parName)>=0 && GetParIndex(matchingfield)>=0)
  {
    for(int irow = 0; irow < GetEntries(); irow++)
    {
      if(TString(GetParameterAsString(matchingfield,irow)).Contains(matchingVal))
      {
        SetParameterFromString(parName,irow,newVal);
        rowsmodified++;
      }
    }
  }
  if(!GetQuiet()) 
    LOG<<rowsmodified<<" rows modified."<<ENDM_INFO;

  return;
}

void NGMConfigurationTablev1::SetParameterAsStringMatching(const char* parName,
    const char* newVal, const char* matchingfield, const char* matchingVal)
{
  int rowsmodified = 0;
  if(GetParIndex(parName)>=0 && GetParIndex(matchingfield)>=0)
  {
    for(int irow = 0; irow < GetEntries(); irow++)
    {
      if(TString(GetParameterAsString(matchingfield,irow))== matchingVal)
      {
        SetParameterFromString(parName,irow,newVal);
        rowsmodified++;
      }
    }
  }
  if(!GetQuiet()) 
    LOG<<rowsmodified<<" rows modified."<<ENDM_INFO;

  return;
}

void NGMConfigurationTablev1::SetParameterMatching(const char* parName,
    const TObject* newVal, const char* matchingfield, const char* matchingVal)
{
  int rowsmodified = 0;
  if(GetParIndex(parName)>=0 && GetParIndex(matchingfield)>=0)
  {
    for(int irow = 0; irow < GetEntries(); irow++)
    {
      if(TString(GetParameterAsString(matchingfield,irow))== matchingVal)
      {
	if(gDirectory&&newVal) 
	  SetParameterO(parName,irow,gDirectory->CloneObject(newVal,false));
	else
	  SetParameterO(parName,irow,0);
        rowsmodified++;
      }
    }
  }
  if(!GetQuiet()) 
    LOG<<rowsmodified<<" rows modified."<<ENDM_INFO;

  return;
}

const char* NGMConfigurationTablev1::LookupParValueAsString(
     const char* parName,
     const char* matchingfield, const char* matchingVal) const
{
  if(GetParIndex(parName)>=0 && GetParIndex(matchingfield)>=0)
  {
    for(int irow = 0; irow < GetEntries(); irow++)
    {
      if(TString(GetParameterAsString(matchingfield,irow)) == matchingVal)
      {
        return GetParameterAsString(parName,irow);
      }
    }
  }
  return "";
}

int NGMConfigurationTable::FindFirstRowMatching(const char* parName,
                                                const char* beginsWith) const
{
  if(GetParIndex(parName)>=0)
  {
    for(int irow = 0; irow < GetEntries(); irow++)
    {
      if(TString(GetParameterAsString(parName,irow)) == beginsWith )
      {
        return irow;
      }
    }
  }
  return -1;
}

const char* NGMConfigurationTable::FindFirstParValueAsString(
                                                            const char* parName,
                                                            const char* matchingfield, const char* beginsWith) const
{
   if(GetParIndex(parName)>=0 && GetParIndex(matchingfield)>=0)
   {
      for(int irow = 0; irow < GetEntries(); irow++)
      {
         if(TString(GetParameterAsString(matchingfield,irow)) == beginsWith)
         {
            return GetParameterAsString(parName,irow);
         }
      }
   }
   return "";
}


const TObject* NGMConfigurationTable::FindFirstParValueAsObject(
                                                             const char* parName,
                                                             const char* matchingfield, const char* beginsWith) const
{
  if(GetParIndex(parName)>=0 && GetParIndex(matchingfield)>=0)
  {
    for(int irow = 0; irow < GetEntries(); irow++)
    {
      if(TString(GetParameterAsString(matchingfield,irow)) == beginsWith)
      {
        return GetParValueO(parName,irow);
      }
    }
  }
  return 0;
}

TObject* NGMConfigurationTable::FindFirstParValueAsObject(
                                                                const char* parName,
                                                                const char* matchingfield, const char* beginsWith)
{
    if(GetParIndex(parName)>=0 && GetParIndex(matchingfield)>=0)
    {
        for(int irow = 0; irow < GetEntries(); irow++)
        {
            if(TString(GetParameterAsString(matchingfield,irow)) == beginsWith)
            {
                return GetParValueO(parName,irow);
            }
        }
    }
    return 0;
}

void NGMConfigurationTablev1::CopyParValuesMatching(const char* FromParName,
    const char* ToParName, const char* matchingfield, const char* matchingPattern,
    NGMConfigurationTable* ToTable) const
{
  if(!ToTable) return;
  int rowscopied = 0;
  if(GetParIndex(FromParName)>=0 && GetParIndex(matchingfield)>=0)
  {
    for(int irow = 0; irow < GetEntries(); irow++)
    {
      TString thisVal = GetParameterAsString(matchingfield,irow);
      if(thisVal.BeginsWith(matchingPattern))
      {
	if(GetParType(FromParName)==NGMConfigurationParameter::ParObject){
	  ToTable->SetParameterMatching(ToParName,GetParValueO(FromParName,irow),matchingfield,thisVal.Data());
	}else{
	  TString newVal =  GetParameterAsString(FromParName,irow);
	  ToTable->SetParameterAsStringMatching(ToParName,
						newVal.Data(),matchingfield,thisVal.Data());
	}
        rowscopied++;
      }
    }
  }
  if(!GetQuiet()) 
    LOG<<rowscopied<<" rows copied."<<ENDM_INFO;

  return;
}

void NGMConfigurationTablev1::ExportToTextFile(const char* filename) const
{
	std::ofstream outfile(filename);
	char delm = '\t';
	int nrows = GetEntries();
	int ncols = GetParCount();

	// Write parameterNames
  outfile<<"ParName"<<delm<<delm;
	for(int icol = 0; icol < ncols; icol++)
	{
    if(GetParType(GetParName(icol)) == NGMConfigurationParameter::ParObject) continue;
		outfile<<GetParName(icol)<<delm;
	}
	outfile<<std::endl;

	//Write parameter types
  outfile<<"ParType"<<delm<<delm;
	for(int icol = 0; icol < ncols; icol++)
	{
    switch(GetParType(GetParName(icol)))
    {
    case NGMConfigurationParameter::ParDouble :
		    outfile<<"Real"<<delm;
        break;
    case NGMConfigurationParameter::ParInteger :
		    outfile<<"Integer"<<delm;
        break;
    case NGMConfigurationParameter::ParObject :
        break;
    case NGMConfigurationParameter::ParString :
      ;
    default:
		    outfile<<"String"<<delm;
    }
	}
	outfile<<std::endl;
	//Write parameter value defaults
	// need to implement
  outfile<<"ParDefault"<<delm<<delm;
	for(int icol = 0; icol < ncols; icol++)
	{
    if(GetParType(GetParName(icol)) == NGMConfigurationParameter::ParObject) continue;
		outfile<<GetColumn(GetParName(icol))->GetDefaultAsString()<<delm;
	}
	outfile<<std::endl;

  //Write parameter value minimums
	// need to implement
  outfile<<"ParMinimum"<<delm<<delm;
	for(int icol = 0; icol < ncols; icol++)
	{
    if(GetParType(GetParName(icol)) == NGMConfigurationParameter::ParObject) continue;
		outfile<<GetColumn(GetParName(icol))->GetMinimumAsString()<<delm;
	}
	outfile<<std::endl;
	//Write parameter value maximums
	// need to implement
  outfile<<"ParMaximum"<<delm<<delm;
	for(int icol = 0; icol < ncols; icol++)
	{
    if(GetParType(GetParName(icol)) == NGMConfigurationParameter::ParObject) continue;
		outfile<<GetColumn(GetParName(icol))->GetMaximumAsString()<<delm;
	}
	outfile<<std::endl;
  //Write cell entries
  for(int irow = 0; irow < nrows; irow++)
	{
    outfile<<irow<<delm<<delm;
  	for(int icol = 0; icol < ncols; icol++)
	  {
      if(GetParType(GetParName(icol)) == NGMConfigurationParameter::ParObject) continue;
		  outfile<<GetColumn(GetParName(icol))->GetValueAsString(irow)<<delm;
    }
    outfile<<std::endl;
	}
  outfile.close();
}

void NGMConfigurationTablev1::ImportFromTextFile(const char* filename)
{
	std::ifstream infile1(filename);
  const int nheaderrows = 5;
  int ncols = 0;
  int readcols = 0;
  int totalcols = 0;
  char delim = '\t';

  std::vector<std::string> parNames;
  std::vector<int> parType;
  std::vector<std::string> parDefault;
  std::vector<std::string> parMinimum;
  std::vector<std::string> parMaximum;
  std::vector<std::string> parDescription;
  std::vector<std::string> parFormat;
  std::vector<std::string> parOperation;
  
	std::string linestr;
	std::string fieldstr;
	int totalrows = 0;
	while(std::getline(infile1,linestr))
	{
		totalrows++;
	}
	std::cout<<"total rows: "<<totalrows<<std::endl;
	infile1.close();

  std::ifstream infile(filename);
	
  SetNRows(  totalrows - nheaderrows ) ;

  // Read parameterNames
  std::getline(infile,linestr);
  {
    LOG<<linestr.c_str()<<ENDM_INFO;
    readcols = 0;
    std::istringstream iss(linestr,std::istringstream::in);
		while(std::getline(iss,fieldstr,delim))
    {
      //Skip first 2 columns
      if(readcols++ <2) continue;
      parNames.push_back(fieldstr);
      ncols++;
    }
	}
  totalcols = ncols;
  LOG<<"Found "<<totalcols<<" columns"<<ENDM_INFO;

	//Read parameter types
  std::getline(infile,linestr);
  {
    readcols = 0;
    ncols = 0;
    std::istringstream iss(linestr,std::istringstream::in);
		while(std::getline(iss,fieldstr,delim))
    {
      //Skip first 2 columns
      if(readcols++ <2) continue;

      if(fieldstr == "Real")
       parType.push_back(0);
      else if(fieldstr == "Integer")
        parType.push_back(1);
      else
        parType.push_back(2);
      
      parDefault.push_back("0");
      parMinimum.push_back("0");
      parMaximum.push_back("1");
      ncols++;
    }
    if(ncols != totalcols)
    {
      LOG<<"Mismatch in number of columns"<<ENDM_FATAL;
      return;
    }
	}
  
	//Read parameter index
//  std::getline(infile,linestr);
	//Read parameter description
//  std::getline(infile,linestr);
  if(false)
  {
    readcols = 0;
    ncols = 0;
    std::istringstream iss(linestr,std::istringstream::in);
		while(std::getline(iss,fieldstr,delim))
    {
      //Skip first 2 columns
      if(readcols++ <2) continue;
      parDescription.push_back(fieldstr);
      ncols++;
    }
    if(ncols != totalcols)
    {
      LOG<<"Mismatch in number of columns"<<ENDM_FATAL;
      return;
    }

	}

	//Read parameter format
  //std::getline(infile,linestr);
  if(false)
  {
    readcols = 0;
    ncols = 0;
    std::istringstream iss(linestr,std::istringstream::in);
		while(std::getline(iss,fieldstr,delim))
    {
      //Skip first 2 columns
      if(readcols++ <2) continue;
      parFormat.push_back(fieldstr);
      ncols++;
    }
    if(ncols != totalcols)
    {
      LOG<<"Mismatch in number of columns"<<ENDM_FATAL;
      return;
    }
	}

  //Read parameter operation
  //std::getline(infile,linestr);
  if(false)
  {
    readcols = 0;
    ncols = 0;
    std::istringstream iss(linestr,std::istringstream::in);
		while(std::getline(iss,fieldstr,delim))
    {
      //Skip first 2 columns
      if(readcols++ <2) continue;
      parOperation.push_back(fieldstr);
      ncols++;
    }
    if(ncols != totalcols)
    {
      LOG<<"Mismatch in number of columns"<<ENDM_FATAL;
      return;
    }
	}
	
  //Read ParDefault
  std::getline(infile,linestr);
  //Read ParMinimum
  std::getline(infile,linestr);
  //Read ParMaximum
  std::getline(infile,linestr);

  // Add Columns to the ngm table
	for(int icol = 0; icol < totalcols; icol++)
	{
    std::cout<<"Adding parameter "<<parNames[icol].c_str()<<std::endl;
    if(GetParIndex(parNames[icol].c_str())<0)
    {
    if(parType[icol] == NGMConfigurationParameter::ParDouble)
    {
      AddParameterD(parNames[icol].c_str(),
        atof(parDefault[icol].c_str()),
        atof(parMinimum[icol].c_str()),
        atof(parMaximum[icol].c_str()));
    }else if(parType[icol] == NGMConfigurationParameter::ParInteger){
      AddParameterI(parNames[icol].c_str(),
        atoi(parDefault[icol].c_str()),
        atoi(parMinimum[icol].c_str()),
        atoi(parMaximum[icol].c_str()));
    }else{
      AddParameterS(parNames[icol].c_str(),
        parDefault[icol].c_str());
    }
    }
    //GetColumn(parNames[icol].c_str())->SetOperation(parOperation[icol].c_str());
    //GetColumn(parNames[icol].c_str())->SetDescription(parDescription[icol].c_str());
    //GetColumn(parNames[icol].c_str())->SetFormat(parFormat[icol].c_str());

	}
  
  //Write cell entries
	int irow = 0;
	while(std::getline(infile,linestr))
	{
    readcols = 0;
		int icol = 0;
		std::istringstream iss(linestr,std::istringstream::in);
    while(std::getline(iss,fieldstr,delim))
		{
      //Skip first 2 columns
      if(readcols++ <2) continue;
      if(icol>=totalcols)
      {
        LOG<<"Unexpected number of columns"<<ENDM_FATAL;
        return;
      }
      GetColumn(parNames[icol].c_str())->SetValueFromString(irow,fieldstr.c_str());
			icol++;
		}
		irow++;
	}
  
  infile.close();

}

void NGMConfigurationTable::CopyTable(const NGMConfigurationTable* tableToCopy)
{
   SetQuiet(true);

   // Lets start by deleting all columns in the corrent table
   while(GetParCount()>0)
   {
      RemoveParameter(GetParName(0));
   }
   
   if(tableToCopy->IsList())
   {
      SetIsList(true);
   }else{
      SetIsList(false);
      SetNRows(tableToCopy->GetEntries());
   }
   
   // Add Columns to the ngm table
   int totalcols = tableToCopy->GetParCount();
   
	for(int icol = 0; icol < totalcols; icol++)
	{
      //LOG<<"Adding parameter "<<tableToCopy->GetParName(icol)<<ENDM_INFO;
      int newEntries = tableToCopy->GetEntries();
      if(tableToCopy->IsList()) newEntries = tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetEntries();
      if(tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetParameterType() == NGMConfigurationParameter::ParDouble)
      {
         AddParameterD(tableToCopy->GetParName(icol),
                       atof(tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetDefaultAsString()),
                       atof(tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetMinimumAsString()),
                       atof(tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetMaximumAsString()),
                       newEntries);
         for(int irow = 0; irow < newEntries; irow++)
            SetParameterD(GetParName(icol),irow,tableToCopy->GetParValueD(GetParName(icol),irow));
      }else if(tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetParameterType() == NGMConfigurationParameter::ParInteger){
         AddParameterI(tableToCopy->GetParName(icol),
                       atoi(tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetDefaultAsString()),
                       atoi(tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetMinimumAsString()),
                       atoi(tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetMaximumAsString()),
                       newEntries);
         for(int irow = 0; irow < newEntries; irow++)
            SetParameterI(GetParName(icol),irow,tableToCopy->GetParValueI(GetParName(icol),irow));
      }else if(tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetParameterType() == NGMConfigurationParameter::ParObject){
        AddParameterO(tableToCopy->GetParName(icol),
                      tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetDefaultO(),
                      newEntries);
        for(int irow = 0; irow < newEntries; irow++)
          if(tableToCopy->GetParValueO(GetParName(icol),irow))
          SetParameterO(GetParName(icol),irow,
                        tableToCopy->GetParValueO(GetParName(icol),irow)->Clone());
      }else{
         AddParameterS(tableToCopy->GetParName(icol),
                       tableToCopy->GetColumn(tableToCopy->GetParName(icol))->GetDefaultAsString(),
                       newEntries);
         for(int irow = 0; irow < newEntries; irow++)
            SetParameterS(GetParName(icol),irow,tableToCopy->GetParValueS(GetParName(icol),irow));

      }
   
	}
	
	SetQuiet(false);

}
