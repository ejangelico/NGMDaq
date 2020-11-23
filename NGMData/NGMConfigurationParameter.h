#ifndef __NGMCONFIGURATIONPARAMETER_H__
#define __NGMCONFIGURATIONPARAMETER_H__

/// \brief NGMConfigurationParameter provides the interface to a single configuration parameter.
///

#include "TString.h"
#include "Rtypes.h"
#include "TObject.h"
class TArrayD; //Forward Declaration
class TObjArray; //Forward Declaration
class TArrayI; //Forward Declaration

class NGMConfigurationParameter : public TObject
{

public:

  enum DataType { ParDouble, ParInteger, ParString, ParObject };
  
   ///Default Constructor
   NGMConfigurationParameter(){}
  ///Constructor
  NGMConfigurationParameter(const char* parameterName, double defaultVal,
                                    double minVal, double maxVal,int nentries){}
  ///Constructor
  NGMConfigurationParameter(int nentries){}
  
  ///Destructor
  virtual ~NGMConfigurationParameter(){}
  
  virtual DataType GetParameterType() const = 0;
   
  static const char* CurrentVersionString(){return "v1";}

  static NGMConfigurationParameter* CreateCurrentVersion(int nentries, DataType wtype = ParDouble);
  
  /// Set Parameter Name
  virtual void SetName(const char* newName) {}
  /// Set Parameter Default
  virtual void SetDefault(const double newVal) {}
  /// Set Parameter Default
  virtual void SetDefault(const char* newVal) {}
  /// Set Parameter Default
  virtual void SetDefault(const int newVal) {}
  /// Set Parameter Default
  virtual void SetDefault(TObject* newVal) {}
  /// Set Parameter Minimum
  virtual void SetMinimum(const double newVal) {}
  /// Set Parameter Maximum
  virtual void SetMaximum(const double newVal) {}
  /// Set Parameter Minimum
  virtual void SetMinimum(const int newVal) {}
  /// Set Parameter Maximum
  virtual void SetMaximum(const int newVal) {}
  /// Set Parameter Value
  virtual void SetValue(int index, double newVal) {}
  /// Set Parameter Value
  virtual void SetValue(int index, const char* newVal) {}
  /// Set Parameter Value
  virtual void SetValue(int index, TObject* newVal) {}
  /// Set Parameter Value
  virtual void SetValue(int index, int newVal) {}
  /// Set Parameter Value from String
  virtual void SetValueFromString(int index, const char* newVal);
  /// Set Parameter Operation
  virtual void SetOperation(const char*);
  /// Set Parameter Description
  virtual void SetDescription(const char*);
  /// Set Parameter Format
  virtual void SetFormat(const char*);
  /// Set All Channels to Default Value
  virtual void SetToDefault();
  /// Set Number of Entries
  virtual void SetNRows(int newVal) {}


  /// Get Parameter Name
  virtual const char* GetName() const {return "";}
  /// Get Parameter Default
  virtual double GetDefault() const {return 0.0;}
  /// Get Parameter Default
  virtual const char* GetDefaultS() const {return "";}
  /// Get Parameter Default
  virtual int GetDefaultI() const {return 0;}
  /// Get Parameter Default
  virtual TObject* GetDefaultO() const {return 0;}
  /// Get Parameter Minimum
  virtual double GetMinimum() const {return 0.0;}
  /// Get Parameter Maximum
  virtual double GetMaximum() const {return 0.0;}
  /// Get Parameter Value
  virtual double GetValue(int index) const {return 0.0;}
  /// Get Parameter Value
  virtual const char* GetValueS(int index) const {return "";}
  /// Get Parameter Value
  virtual const TObject* GetValueO(int index) const {return 0;}
  /// Get Parameter Value Modifiable
  virtual TObject* GetValueO(int index) {return 0;}
  /// Get Parameter Value
  virtual int GetValueI(int index) const {return 0;}
  /// Get Parameter Value as String
  virtual const char* GetValueAsString(int index) const;
  /// Get Default Value as String
  virtual const char* GetDefaultAsString() const;
  /// Get Minimum Value as String
  virtual const char* GetMinimumAsString() const;
  /// Get Maximum Value as String
  virtual const char* GetMaximumAsString() const;
  /// Get Parameter Operation
  virtual const char* GetOperation() const;
  /// Get Parameter Description
  virtual const char* GetDescription() const;
  /// Get Parameter Format
  virtual const char* GetFormat() const;

  /// Get Number of Entries
  virtual int GetEntries() const { return 0; }
    
  ClassDef(NGMConfigurationParameter,1)
};

class NGMConfigurationParameterv1 : public NGMConfigurationParameter
{
  
public:
  
  ///Default Constructor
  NGMConfigurationParameterv1();
  ///Constructor
  NGMConfigurationParameterv1(const char* parameterName, double defaultVal,
                                    double minVal, double maxVal,int nentries);
  ///Constructor
  NGMConfigurationParameterv1(int nentries);
  
  ///Destructor
  virtual ~NGMConfigurationParameterv1();
  /// Get Parameter Type : Double
  virtual DataType GetParameterType() const { return ParDouble; }
  /// Set Parameter Name
  virtual void SetName(const char* newName) {_parameterName=newName;}
  /// Set Parameter Default
  virtual void SetDefault(const double newVal) {_defaultVal=newVal;}
  /// Set Parameter Minimum
  virtual void SetMinimum(const double newVal) {_minimumVal=newVal;}
  /// Set Parameter Maximum
  virtual void SetMaximum(const double newVal) {_maximumVal=newVal;}
  /// Set Parameter Value
  virtual void SetValue(int index, double newVal);
  /// Set Number of Entries
  virtual void SetNRows(int newVal);
  /// Set Parameter Operation
  virtual void SetOperation(const char*);
  /// Set Parameter Description
  virtual void SetDescription(const char*);
  /// Set Parameter Format
  virtual void SetFormat(const char*);


  /// Get Parameter Name
  virtual const char* GetName() const {return (const char*)_parameterName;}
  /// Get Parameter Default
  virtual double GetDefault() const {return _defaultVal;}
  /// Get Parameter Minimum
  virtual double GetMinimum() const {return _minimumVal;}
  /// Get Parameter Maximum
  virtual double GetMaximum() const {return _maximumVal;}
  /// Get Parameter Value
  virtual double GetValue(int index) const;
  /// Get Number of Entries
  virtual int GetEntries() const { return _nentries; }
  /// Get Parameter Operation
  virtual const char* GetOperation() const;
  /// Get Parameter Description
  virtual const char* GetDescription() const;
  /// Get Parameter Format
  virtual const char* GetFormat() const;

private:
  
  TString   _parameterName;
  Double_t  _defaultVal;
  Double_t  _minimumVal;
  Double_t  _maximumVal;
  Int_t     _nentries;
  TArrayD* _parval;
  TString _operation;
  TString _description;
  TString _format;
  
public:
  ClassDef(NGMConfigurationParameterv1,2)
};


class NGMConfigurationParameterSv1 : public NGMConfigurationParameter
{
  
public:
  
  ///Default Constructor
  NGMConfigurationParameterSv1();
  ///Constructor
  NGMConfigurationParameterSv1(const char* parameterName, const char* defaultVal,
                               int nentries);
  ///Constructor
  NGMConfigurationParameterSv1(int nentries);
  
  ///Destructor
  virtual ~NGMConfigurationParameterSv1();
  /// Get Parameter Type : String
  virtual DataType GetParameterType() const { return ParString; }
  /// Set Parameter Name
  virtual void SetName(const char* newName) {_parameterName=newName;}
  /// Set Parameter Default
  virtual void SetDefault(const char* newVal) {_defaultVal=newVal;}
  /// Set Parameter Value
  virtual void SetValue(int index, const char* newVal);
  /// Set All Channels to Default Value
  virtual void SetToDefault();
   /// Set Number of Entries
  virtual void SetNRows(int newVal);
  /// Set Parameter Operation
  virtual void SetOperation(const char*);
  /// Set Parameter Description
  virtual void SetDescription(const char*);
  /// Set Parameter Format
  virtual void SetFormat(const char*);

  /// Get Parameter Name
  virtual const char* GetName() const {return (const char*)_parameterName;}
  /// Get Parameter Default
  virtual const char* GetDefaultS() const {return _defaultVal;}
  /// Get Parameter Value
  virtual const char* GetValueS(int index) const;
  /// Get Number of Entries
  virtual int GetEntries() const { return _nentries; }
    /// Get Parameter Operation
  virtual const char* GetOperation() const;
  /// Get Parameter Description
  virtual const char* GetDescription() const;
  /// Get Parameter Format
  virtual const char* GetFormat() const;

private:
    
  TString  _parameterName;
  TString  _defaultVal;
  Int_t    _nentries;
  TObjArray* _parval;
  TString _operation;
  TString _description;
  TString _format;

  
public:
    ClassDef(NGMConfigurationParameterSv1,2)
};

class NGMConfigurationParameterIv1 : public NGMConfigurationParameter
{
  
public:
  
  ///Default Constructor
  NGMConfigurationParameterIv1();
  ///Constructor
  NGMConfigurationParameterIv1(const char* parameterName, int defaultVal,
                               int minVal, int maxVal,int nentries);
  ///Constructor
  NGMConfigurationParameterIv1(int nentries);
  
  ///Destructor
  virtual ~NGMConfigurationParameterIv1();
  /// Get Parameter Type : Integer
  virtual DataType GetParameterType() const { return ParInteger; }
  /// Set Parameter Name
  virtual void SetName(const char* newName) {_parameterName=newName;}
  /// Set Parameter Default
  virtual void SetDefault(const int newVal) {_defaultVal=newVal;}
  /// Set Parameter Value
  virtual void SetValue(int index, int newVal);
  /// Set All Channels to Default Value
  virtual void SetToDefault();
   /// Set Number of Entries
  virtual void SetNRows(int newVal);
  /// Set Parameter Operation
  virtual void SetOperation(const char*);
  /// Set Parameter Description
  virtual void SetDescription(const char*);
  /// Set Parameter Format
  virtual void SetFormat(const char*);

  /// Get Parameter Name
  virtual const char* GetName() const {return (const char*)_parameterName;}
  /// Get Parameter Default
  virtual int GetDefaultI() const {return _defaultVal;}
  /// Get Parameter Value
  virtual int GetValueI(int index) const;
  /// Get Number of Entries
  virtual int GetEntries() const { return _nentries; }
  /// Get Parameter Operation
  virtual const char* GetOperation() const;
  /// Get Parameter Description
  virtual const char* GetDescription() const;
  /// Get Parameter Format
  virtual const char* GetFormat() const;

private:
    
  TString  _parameterName;
  int      _defaultVal;
  Int_t    _nentries;
  TArrayI* _parval;
  TString _operation;
  TString _description;
  TString _format;

public:
    ClassDef(NGMConfigurationParameterIv1,2)
};


class NGMConfigurationParameterOv1 : public NGMConfigurationParameter
{
  
public:
  
  ///Default Constructor
  NGMConfigurationParameterOv1();
  ///Constructor
  NGMConfigurationParameterOv1(const char* parameterName, TObject* defaultO,
                               int nentries);
  ///Constructor
  NGMConfigurationParameterOv1(int nentries);
  
  ///Destructor
  virtual ~NGMConfigurationParameterOv1();
  /// Get Parameter Type : Object
  virtual DataType GetParameterType() const { return ParObject; }
  /// Set Parameter Name
  virtual void SetName(const char* newName) {_parameterName=newName;}
  /// Set Parameter Default
  virtual void SetDefault(TObject* newVal) {_defaultVal = newVal;}
  /// Set Parameter Value
  virtual void SetValue(int index, TObject*);
  /// Set All Channels to Default Value
  virtual void SetToDefault();
  /// Set Number of Entries
  virtual void SetNRows(int newVal);
  /// Set Parameter Operation
  virtual void SetOperation(const char*);
  /// Set Parameter Description
  virtual void SetDescription(const char*);
  /// Set Parameter Format
  virtual void SetFormat(const char*);
  
  /// Get Parameter Name
  virtual const char* GetName() const {return (const char*)_parameterName;}
  /// Get Parameter Default
  virtual TObject* GetDefaultO() const {return _defaultVal;}
  /// Get Parameter Value
  virtual const TObject* GetValueO(int index) const;
  /// Get Parameter Value Modifiable
  virtual TObject* GetValueO(int index);
  /// Get Number of Entries
  virtual int GetEntries() const { return _nentries; }
  /// Get Parameter Operation
  virtual const char* GetOperation() const;
  /// Get Parameter Description
  virtual const char* GetDescription() const;
  /// Get Parameter Format
  virtual const char* GetFormat() const;
  
private:
  
  TString  _parameterName;
  TObject*  _defaultVal;
  Int_t    _nentries;
  TObjArray* _parval;
  TString _operation;
  TString _description;
  TString _format;
  
  
public:
  ClassDef(NGMConfigurationParameterOv1,2)
};

#endif //__NGMCONFIGURATIONPARAMETER_H__
