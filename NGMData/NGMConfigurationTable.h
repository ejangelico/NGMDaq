#ifndef __NGMCONFIGURATIONTABLE_H__
#define __NGMCONFIGURATIONTABLE_H__

/// \brief NGMConfigurationTable provides the interface to a table of parameters.
///

#include <iostream>
#include "TObject.h"
#include "TObjArray.h"
#include "Rtypes.h"
#include "NGMConfigurationParameter.h"
#include <limits>

class NGMConfigurationTable : public TObject
{

public:

  /// \brief Default Constructor
  NGMConfigurationTable();
  /// \brief Default Constructor
  /// @param nparameters is the number of hardware parameters to be managed
  /// @param iList determines whether the object should be treated as table or list
  NGMConfigurationTable(int nentries, bool isList = false){}
  static const char* CurrentVersionString(){return "NGMConfigurationTablev1";}
  /// \brief Constructor of Versioned Object
  /// @param nparameters is the number of hardware parameters to be managed
  static NGMConfigurationTable* CreateCurrentVersionTable(int nentries);
  /// \brief Constructor of Versioned Object for list
  static NGMConfigurationTable* CreateCurrentVersionList();
  /// \brief Destructor
  virtual ~NGMConfigurationTable(){}
  /// \brief AddParameter Double
  /// @param parameterName
  /// @param defaultVal
  /// @param minVal
  /// @param maxVal
    virtual void AddParameterD(const char* parameterName, double defaultVal = 0.0,
                               double minVal = std::numeric_limits<double>::min(),
                               double maxVal = std::numeric_limits<double>::max(),
                               int nentries = 1) = 0;
  /// \brief AddParameter Integer Parameter
  /// @param parameterName
  /// @param defaultVal
  /// @param minVal
  /// @param maxVal
  virtual void AddParameterI(const char* parameterName, int defaultVal = 0,
                             int minVal = std::numeric_limits<int>::min(),
                             int maxVal = std::numeric_limits<int>::max(),
                             int nentries = 1) = 0;
  /// \brief AddParameterS StringParameter
  /// @param parameterName
  /// @param defaultVal
  virtual void AddParameterS(const char* parameterName,
                             const char* defaultVal = "",
                             int nentries = 1) = 0;
  /// \brief AddParameterO ObjectParameter
  /// @param parameterName
  /// @param defaultVal
  virtual void AddParameterO(const char* parameterName,
                             TObject* defaultVal = 0,
                             int nentries = 1) = 0;
  /// \brief RemoveParameter
  /// @param parameterName
  virtual void RemoveParameter(const char* parameterName) = 0;
    
  /// \brief RemoveParameter
  /// @param parIndex
  virtual void RemoveParameter(int parIndex) = 0;

  /// \brief GetColumn accesses the parameter column via parameterName
  /// @param parameterName
  virtual const NGMConfigurationParameter* GetColumn(const char* parameterName) const = 0;
  /// \brief GetColumn accesses the parameter column via parameterName
  /// @param parameterName
  virtual NGMConfigurationParameter* GetColumn(const char* parameterName) = 0;
  /// \brief GetParName accesses the name of configuration parameter by index
  /// @param parameterName
  virtual const char* GetParName(int parindex) const = 0;
  /// \brief GetParCount returns the number of configuration parameters
  virtual int GetParCount() const = 0;
  /// \brief GetParEntries returns the number of entries for all configuration parameter
  virtual int GetEntries(const char* parName = "") const = 0;
  /// \brief GetParameterValue accesses a single channel
  /// @param parameterName
  /// @param index of channel
  virtual double GetParValueD(const char* parName, int index) const = 0;
  /// \brief GetParameterValue accesses a single channel
  /// @param parameterName
  /// @param index of channel
  virtual int GetParValueI(const char* parName, int index) const = 0;
  /// \brief GetParameterValue accesses a single channel
  /// @param parameterName
  /// @param index of channel
  virtual const char* GetParValueS(const char* parName, int index) const = 0;
  /// \brief GetParameterValue accesses a single channel
  /// @param parameterName
  /// @param index of channel
  virtual const TObject* GetParValueO(const char* parName, int index) const = 0;
  /// \brief GetParameterValue accesses a single channel
  /// @param parameterName
  /// @param index of channel
  virtual TObject* GetParValueO(const char* parName, int index) = 0;
  /// \brief GetParameterType returns the type for the specified channel ParString,ParDouble,ParInteger
  /// @param parameterName
  virtual NGMConfigurationParameter::DataType GetParType(const char* parName) const = 0;
  /// \brief SetParameter modifies single channel
  /// @param parameterName
  /// @param index of channel
  /// @param newValue
  virtual void SetParameterD(const char* parName, int index, double newVal) = 0;
  /// \brief SetParameter modifies single channel
  /// @param parameterName
  /// @param index of channel
  /// @param newValue
  virtual void SetParameterI(const char* parName, int index, int newVal) = 0;
  /// \brief SetParameter modifies single channel
  /// @param parameterName
  /// @param index of channel
  /// @param newValue
  virtual void SetParameterS(const char* parName, int index, const char* newVal) = 0;
  /// \brief SetParameter modifies single channel
  /// @param parameterName
  /// @param index of channel
  /// @param newValue
  virtual void SetParameterO(const char* parName, int index, TObject* newVal) = 0;
  /// \brief SetParameterDefaultD modifies default of a parameter
  /// @param parameterName
  /// @param newValue
  virtual void SetParameterDefaultD(const char* parName, double newVal) = 0;
  /// \brief SetParameterToDefault modifies single channel resetting to the parameter default
  /// @param parameterName
  virtual void SetParameterToDefault(const char* parName) = 0;
  /// \brief SetAllParametersToDefault modifies each channel resetting to the parameter default
  virtual void SetAllParametersToDefault() = 0;
  /// \brief Expand or Shrink Table Preserving existing entries
  /// @param newVal number of rows to expand or shrink table
  virtual void SetNRows(int newVal) = 0;
  /// \brief SetQuiet 
  /// @param bool do we LOG every update of every parameter
  virtual void SetQuiet(bool quietval){}
  /// \brief GetQuiet do we LOG every update of every parameter
  virtual bool GetQuiet() const { return false;}
  /// \brief PrintTable dumps the table to the ouput buffer
  /// @param ostr Output stream is std::cout by default
  virtual void PrintTable(std::ostream &ostr = std::cout){}
  /// \brief PrintRow dumps the values of each channel to the ouput buffer
  /// @param ostr Output stream is std::cout by default
  virtual void PrintRow(int ientry, std::ostream &ostr = std::cout){}
  /// \brief IsList Is this a list of arrays of variable length (return true) or a table (return false)
  virtual bool IsList() const = 0;
  /// \brief SetIsList Is determines a list of arrays of variable length (return true) or a table (return false)
  virtual void SetIsList(bool isList=true) = 0;
  /// \breif SetParameterFromString
  /// @param parName: name of parameter
  /// @param index: row number of entry
  /// @param newVal: value to write into table
  virtual void SetParameterFromString(const char* parName, int index, const char* newVal) = 0;
 /// \breif SetParameterFromString
  /// @param parName: name of parameter
  /// @param index: row number of entry
  /// @param newVal: value to write into table
  virtual const char* GetParameterAsString(const char* parName, int index) const = 0;
 /// \breif SetParameterAsStringThatBeginsWith
  /// @param parName: name of parameter
  /// @param newVal: value to write into table
  /// @param matchingfield: field to use when matching row
  /// @param matchingvalue: value to use when matching row
  virtual void SetParameterAsStringThatBeginsWith(const char* parName,
    const char* newVal, const char* matchingfield, const char* matchingVal) = 0;
 /// \breif SetParameterAsStringThatContains
  /// @param parName: name of parameter
  /// @param newVal: value to write into table
  /// @param matchingfield: field to use when matching row
  /// @param matchingvalue: value to use when matching row
  virtual void SetParameterAsStringThatContains(const char* parName,
    const char* newVal, const char* matchingfield, const char* matchingVal) = 0;
  /// \breif SetParameterAsStringMatching
  /// @param parName: name of parameter
  /// @param newVal: value to write into table
  /// @param matchingfield: field to use when matching row
  /// @param matchingvalue: value to use when matching row
virtual void SetParameterAsStringMatching(const char* parName,
    const char* newVal, const char* matchingfield, const char* matchingVal) = 0;
  /// \breif SetParameterMatching
  /// @param parName: name of parameter
  /// @param newVal: object value to write into table
  /// @param matchingfield: field to use when matching row
  /// @param matchingvalue: value to use when matching row
  virtual void SetParameterMatching(const char* parName,
				  const TObject* newVal,
				  const char* matchingfield,
				  const char* matchingVal){}
  /// \breif LookupParValueAsString
  /// @param parName: name of parameter
  /// @param matchingfield: field to use when matching row
  /// @param matchingvalue: value to use when matching row
  virtual const char* LookupParValueAsString(const char* parName,
     const char* matchingfield, const char* matchingVal) const = 0;
  /// \breif FindFirstParValueAsString
  /// @param parName: name of parameter
  /// @param matchingfield: field to use when matching row
  /// @param beginsWith: value to use when matching row
  virtual const char* FindFirstParValueAsString(const char* parName,
                                                const char* matchingfield,
                                                const char* beginsWith) const;
  /// \breif FindFirstParValueAsObject
  /// @param parName: name of parameter
  /// @param matchingfield: field to use when matching row
  /// @param beginsWith: value to use when matching row
  virtual const TObject* FindFirstParValueAsObject(const char* parName,
                                                const char* matchingfield,
                                                const char* beginsWith) const;     
    /// \breif FindFirstParValueAsObject
    /// @param parName: name of parameter
    /// @param matchingfield: field to use when matching row
    /// @param beginsWith: value to use when matching row
    virtual TObject* FindFirstParValueAsObject(const char* parName,
                                                     const char* matchingfield,
                                                     const char* beginsWith);
  /// \breif FindFirstRowMatching
  /// @param parName: name of parameter
  /// @param matchingfield: field to use when matching row
  /// @param beginsWith: value to use when matching row
  virtual int FindFirstRowMatching(const char* parName,
                                   const char* beginsWith) const;
 /// \breif SetParameterAsStringThatContains
  /// @param FromParName: name of parameter to copy
  /// @param ToParName: name of parameter to copy to
  /// @param matchingfield: field to use when matching row
  /// @param matchingPattern: value to use to select subset of rows
  /// @param ToTable: target table of copy
  virtual void CopyParValuesMatching(const char* FromParName,
    const char* ToParName, const char* matchingfield, const char* matchingPattern,
    NGMConfigurationTable* ToTable) const = 0; 
  /// \breif ExportToTextFile
  /// @param filename: pathname of file to export table
  virtual void ExportToTextFile(const char* filename) const = 0;
  /// \breif ImportFromTextFile
  /// @param filename: pathname of file to import to table
  virtual void ImportFromTextFile(const char* filename) = 0;
  /// \breif Copy from source table deleting existing data
  virtual void CopyTable(const NGMConfigurationTable* tableToCopy);

  
  virtual int GetParIndex(const char* parName) const = 0;
public:
  ClassDef(NGMConfigurationTable,1)
};

class NGMConfigurationTablev1 : public NGMConfigurationTable
{
  
public:
  
  ///Default Constructor
  NGMConfigurationTablev1();
  ///Constructor
  NGMConfigurationTablev1(int nentries, bool isList = false);
  
  ///Destructor
  virtual ~NGMConfigurationTablev1();
  
  virtual void AddParameterD(const char* parameterName, double defaultVal = 0.0,
                             double minVal = std::numeric_limits<double>::min(),
                             double maxVal = std::numeric_limits<double>::max(),
                             int nentries = 1);
  virtual void AddParameterI(const char* parameterName, int defaultVal = 0,
                             int minVal = std::numeric_limits<int>::min(),
                             int maxVal = std::numeric_limits<int>::max(),
                             int nentries = 1);
  virtual void AddParameterS(const char* parameterName,
                             const char* defaultVal = "", int nentries = 1);
  virtual void AddParameterO(const char* parameterName,
                             TObject* defaultVal = 0, int nentries = 1);
  virtual void RemoveParameter(const char* parameterName);
  virtual void RemoveParameter(int parIndex);

  virtual void PrintTable(std::ostream &ostr = std::cout);
  virtual void PrintRow(int ientry, std::ostream &ostr = std::cout);
  
  virtual NGMConfigurationParameter::DataType GetParType(const char* parName) const;
  virtual void SetParameterDefaultD(const char* parName, double newVal);
  virtual void SetParameterToDefault(const char* parName);
  virtual void SetAllParametersToDefault();
  virtual int GetParIndex(const char* parName) const;
  virtual void SetParameterD(const char* parName, int index, double newVal);
  virtual void SetParameterI(const char* parName, int index, int newVal);
  virtual void SetParameterS(const char* parName, int index, const char* newVal);
  virtual void SetParameterO(const char* parName, int index, TObject* newVal);
  virtual const char* GetParName(int parindex) const;
  virtual const NGMConfigurationParameter* GetColumn(const char* parameterName) const;
  virtual NGMConfigurationParameter* GetColumn(const char* parameterName);
  virtual void SetParameterFromString(const char* parName, int index, const char* newVal);
  virtual const char* GetParameterAsString(const char* parName, int index) const;
  virtual void SetParameterAsStringThatBeginsWith(const char* parName,
    const char* newVal, const char* matchingfield, const char* matchingVal);
  virtual void SetParameterAsStringThatContains(const char* parName,
    const char* newVal, const char* matchingfield, const char* matchingVal);
  virtual void SetNRows(int newVal);

  virtual int GetParCount() const {return _numparameters;}
  virtual int GetEntries(const char* parName = "") const;
  virtual double GetParValueD(const char* parName, int index) const;
  virtual int GetParValueI(const char* parName, int index) const;
  virtual const char* GetParValueS(const char* parName, int index) const;
  virtual const TObject* GetParValueO(const char* parName, int index) const;    
  virtual TObject* GetParValueO(const char* parName, int index);
  virtual bool IsList() const {return _isList;}
  virtual void SetQuiet(bool quietval){_quiet = quietval;}
  virtual bool GetQuiet() const {return _quiet; }
  virtual void ExportToTextFile(const char* filename) const;
  virtual void ImportFromTextFile(const char* filename);
  virtual void SetParameterMatching(const char* parName,
				    const TObject* newVal,
				    const char* matchingfield,
				    const char* matchingVal);
 virtual void SetParameterAsStringMatching(const char* parName,
    const char* newVal, const char* matchingfield, const char* matchingVal);
 virtual const char* LookupParValueAsString(const char* parName,
     const char* matchingfield, const char* matchingVal) const;
  virtual void CopyParValuesMatching(const char* FromParName,
    const char* ToParName, const char* matchingfield, const char* matchingPattern,
    NGMConfigurationTable* ToTable) const;
  virtual void SetIsList(bool isList=true) {_isList = isList;}
private:
    bool _isList;
    int _nentries;
    int _numparameters;
    TObjArray* _parTable;
	bool _quiet; //! Dont save this to disk
public:
  ClassDef(NGMConfigurationTablev1,2)
};

#endif //__NGMCONFIGURATIONPARAMETER_H__
