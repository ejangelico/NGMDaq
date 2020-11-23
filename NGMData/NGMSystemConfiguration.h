#ifndef __NGMSYSTEMCONFIGURATION_H__
#define __NGMSYSTEMCONFIGURATION_H__

/// \brief NGMSystemConfiguration defines the configuration format for all hardware platforms
///
/// The NGM Configuration provides parameter sets for
/// - System Wide Parameters
/// - Slot Specific Parameters
/// - Channel Specific Parameters
/// Each Slot and Channel can also be assigned to a NGMConfigurationGroup.  This is a logical
/// collection of slots and channels that might have similar characteristics such as might be
/// the case for all channels reading out the same detector type, scintillator, He3, or muon
/// paddles.  The NGMConfigurationGroup will facilitate SetAll methods for slots and channels.

#include "TNamed.h"
#include "NGMConfigurationTable.h"
#include <iostream>
#include "TTimeStamp.h"

class NGMSystemConfiguration : public TNamed
{

public:

   ///Constructor
   NGMSystemConfiguration() {}

  ///Constructor
  NGMSystemConfiguration(const char* confName, int nslots, int nchannels) : TNamed(confName,"") {}

  ///Destructor
  virtual ~NGMSystemConfiguration(){}
   
  /// \brief Get the Configuration Name
  virtual const char* GetConfigurationName() const {return "Not Implemented";}
  /// \brief Copy from source configuration
  virtual void CopyConfiguration(const NGMSystemConfiguration* sysToCopy);
  /// \brief Create duplicate of this configuration
  virtual NGMSystemConfiguration* DuplicateConfiguration() const;
  /// \brief Set the TimeStamp Associated with this Configuration
  virtual void SetTimeStampNow() {}
  /// \brief Set the Configuration Name
  virtual void SetConfigurationName(const char* confName) { SetName(confName);}
  /// \brief Get the System Parameter Table
  virtual NGMConfigurationTable* GetSystemParameters() { return 0; }
  /// \brief Get the Slot Parameter Table
  virtual NGMConfigurationTable* GetSlotParameters() { return 0; }
  /// \brief Get the Channel Parameter Table
  virtual NGMConfigurationTable* GetChannelParameters() { return 0; }  
  /// \brief Get the Detector Parameter Table
  virtual NGMConfigurationTable* GetDetectorParameters() { return 0; }  
  /// \brief Get the Channel Parameter Table
  virtual NGMConfigurationTable* GetHVParameters() { return 0; }  
  /// \brief Get the System Parameter Table
  virtual const NGMConfigurationTable* GetSystemParameters() const { return 0; }
  /// \brief Get the Slot Parameter Table
  virtual const NGMConfigurationTable* GetSlotParameters() const { return 0; }
  /// \brief Get the Channel Parameter Table
  virtual const NGMConfigurationTable* GetChannelParameters() const { return 0; }  
  /// \brief Get the Detector Parameter Table
  virtual const NGMConfigurationTable* GetDetectorParameters() const { return 0; }  
  /// \brief Get the High Voltage Parameter Table
  virtual const NGMConfigurationTable* GetHVParameters() const { return 0; }  
  /// \brief Print the Parameter Tables
  virtual void PrintTables(std::ostream &ostr = std::cout) const {}
  /// \brief Get the unique run number for this configuration
  virtual long long getRunNumber() const {return 0;}
  /// \brief Set the unique run number for this configuration
  virtual void setRunNumber(long long newVal) {}
  /// \brief Get the TimeStamp for this configuration  
  virtual TTimeStamp GetTimeStamp() const { return TTimeStamp((time_t) 0,0);}
  /// \brief Set the TimeStamp for this configuration  
  virtual void SetTimeStamp(TTimeStamp newVal) {}
  /// \brief Launch GUI  
  virtual void LaunchGUI(); // *MENU*

   public:
     ClassDef(NGMSystemConfiguration,1)
};


class NGMSystemConfigurationv1 : public NGMSystemConfiguration
{
  
public:
  
  ///Constructor
  NGMSystemConfigurationv1();
  ///Constructor
  NGMSystemConfigurationv1(const char* confName, int nslots, int nchannels);
  
  ///Destructor
  virtual ~NGMSystemConfigurationv1();
  virtual void SetTimeStampNow();

  /// \brief Get the System Parameter Table
  NGMConfigurationTable* GetSystemParameters(){ return _fSystemPars; }
  /// \brief Get the Slot Parameter Table
  NGMConfigurationTable* GetSlotParameters() { return _fSlots; }
  /// \brief Get the Channel Parameter Table
  NGMConfigurationTable* GetChannelParameters() { return _fChannels; }
  /// \brief Get the Detector Parameter Table
  NGMConfigurationTable* GetDetectorParameters() { return _fDetectors; }  
  /// \brief Get the High Voltage Parameter Table
  NGMConfigurationTable* GetHVParameters() { return _fHVControl; }  
  /// \brief Get the System Parameter Table
  const NGMConfigurationTable* GetSystemParameters() const { return _fSystemPars; }
  /// \brief Get the Slot Parameter Table
  const NGMConfigurationTable* GetSlotParameters() const { return _fSlots; }
  /// \brief Get the Channel Parameter Table
  const NGMConfigurationTable* GetChannelParameters() const { return _fChannels; }
  /// \brief Get the Detector Parameter Table
  const NGMConfigurationTable* GetDetectorParameters() const { return _fDetectors; }  
  /// \brief Get the High Voltage Parameter Table
  const NGMConfigurationTable* GetHVParameters() const { return _fHVControl; }  
  /// \brief Print the Channel Parameter Tables
  virtual void PrintTables(std::ostream &ostr = std::cout) const;
  /// \brief Create duplicate of this configuration
  virtual NGMSystemConfiguration* DuplicateConfiguration() const;
  
  long long getRunNumber() const;
  
  void setRunNumber(long long newVal) { _runNumber = newVal; } 
  
  virtual TTimeStamp GetTimeStamp() const { return _timestamp; }
  /// \brief Set the TimeStamp for this configuration  
  virtual void SetTimeStamp(TTimeStamp newVal) { _timestamp = newVal; }
  
private:
  Long64_t _runNumber;
  TTimeStamp _timestamp;
  NGMConfigurationTable* _fSystemPars;
  NGMConfigurationTable* _fSlots;
  NGMConfigurationTable* _fChannels;
  NGMConfigurationTable* _fDetectors;
  NGMConfigurationTable* _fHVControl;
  //NGMConfigurationGroup* fGroups;
public:
  ClassDef(NGMSystemConfigurationv1,3)

};

#endif //__NGMSYSTEMCONFIGURATION_H__
