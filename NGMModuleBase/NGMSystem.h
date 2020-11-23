#ifndef __NGMSYSTEM_H__
#define __NGMSYSTEM_H__

/// \mainpage NGMDaq is a toolkit for acquiring and analyzing neutron and gamma signals.
/// Any hardware controlled or readout by a PC can be integrated by a Hardware specific implementation
/// derived from the NGMSystem API.  The UI controls manage hardware configuration, initialization,
/// and data retrieval.  The data are analyzed and displayed in real time from the same user interface.
///
/// The framework itself has two explicit dependencies:
/// - root <a href="root.cern.ch">root.cern.ch</a> installed in $OFFLINE_MAIN
/// - posix threads: standard on *nix systems and part of Windows Services for UNIX for MS systems
///
/// A working example is provided to get things started. These are instructions for
/// building and running the example program:

#include "Rtypes.h"
#include "TH1.h"
#include "NGMModule.h"
#include "TString.h"
#include "TArrayI.h"
#include <vector>
class NGMSystemConfiguration; // Forward Declaration
class TThread; // Forward Declaration
class NGMSpyServ; // Forward Declaration
class TSignalHandler; //Forward Declaration
//#include "TThread.h"

/// \brief NGMSystem provides the API from which to derive all Hardware specific loadable modules.
///

class NGMSystem : public NGMModule
{

public:

   ///Constructor
   NGMSystem();

   ///Destructor
   virtual ~NGMSystem();
   
   /// \brief Register an NGMSystem provided by a loadable module
   /// @param NGMSystem*
   ///
   static void registerSystem(NGMSystem* newSystem);
   
   /// \brief Register an NGMSystem provided by a loadable module
   /// @param NGMSystem*
   ///
   static NGMSystem* getSystem(int nsystem = 0);
   
   /// \brief Remove a subsystem provided by a loadable module
   /// @param NGMSystem*
   ///
   static void unregisterSystem(NGMSystem* oldSystem){}
   
   /// \brief Load a hardware module by name and library file
   /// @param systemName
   /// @param modulePathName
   static void loadHardwareSystem(const char* systemName, const char* modulePathName) {}

   /// \brief Configure this acquisition system using an archived configuration file
   /// @param configfile provides the path of configuration file 
   ///
   virtual int readConfigFile(const char* configfile); // *MENU*

   /// \brief Read the configuration from the Hardware
   ///
   virtual int readConfigurationFromHardware(){ return 1; }

   /// \brief Save the current configuration to file
   /// @param configfile provides the path of configuration file 
   ///
   virtual int saveConfigFile(const char* configfile); // *MENU*

   /// \brief Generate a default configuration for this hardware
   /// @param configname
   virtual int CreateDefaultConfig(const char* configname) { return 0; } // *MENU*

   /// \brief Get the current configuration for this hardware
   virtual NGMSystemConfiguration* GetConfiguration(){return _config;};

   /// \brief Initialize the Hardware drivers and let this process claim the hardware
   /// 
   virtual int InitializeSystem() {return 0;} // *MENU*
   
   /// \brief Configure the Hardware with the current configuration
   /// 
   virtual int ConfigureSystem() {return 0;} // *MENU*
   
   /// \brief Start the hardware acquisition
   /// 
   virtual int StartAcquisition() {return 0;} // *MENU*

   /// \brief Stop the hardware acquisition 
   /// 
   virtual int StopAcquisition() {return 0;} // *MENU*
   
   /// \brief Set the Acquisition Stop Request
   ///
   virtual int RequestAcquisitionStop() {return 0;} // *MENU*

   /// \brief Get the LiveTime in seconds for this run
   ///
   virtual double GetLiveTime() const {return 0.0; }
   
   /// \brief Get the time since beginning of run in seconds
   ///
   virtual double GetRunDuration() const { return 0.0; }
 
   /// \brief Get the number of events for this run
   ///
   virtual int GetTotalEventsThisRun() const { return 0; }

   /// \brief Get Run Status
   ///
   /// Returns 0 for not running, 1 for running
   virtual int GetRunStatus() const { return 0; }

   /// \brief Get Run Status
   ///
   ///
   virtual void GetStatus(TString& status);

    /// \brief Open the input file
  ///
  virtual int OpenInputFile(const char*) {return 0;} // *MENU*
  
  /// \brief Close the input file
  virtual int CloseInputFile() {return 0;} // *MENU*

  /// \brief Open the output buffer
   ///
   virtual int OpenOutputFile(const char*) {return 0;}
   
   /// \brief Close the output buffer
   virtual int CloseOutputFile() {return 0;}
   
   /// \brief Get a snapshot of the current Monitor plots
   ///
   //virtual std::vector<TH1*> GetMonitorHistograms() = 0;
   
   /// \brief For external monitoring a separate buffer
   /// decoupled from the main DAQ thread will provide access
   /// to all or some prescaled fraction of the waveforms
   ///
   //virtual std::vector<TH1*> GetNextWaveforms() =0;

   /// \brief For external monitoring a separate buffer
   /// decoupled from the main DAQ thread will provide access
   /// to all or some prescaled fraction of the pulses
   ///
   //virtual std::vector<TH1*> GetNextPulses() = 0;
   
   /// \breif Release the hardware back to the operating system
   /// for use by another process.
   virtual void ReleaseSystem() {}
   
   /// \brief Get Name Identifying this pass
   virtual TString GetPassName() const { return TString(""); }
  
    /// \brief Get Name Identifying this pass
   virtual void SetPassName(TString passname) {}
  
   /// \brief Save Analysis Tree to file with name
   /// derived from runnumber and passname
   virtual int SaveAnaTree() ; // *MENU*

   /// \brief Request Update of all plots
   virtual void RequestPlotUpdate(){} //*MENU*
  
  /// \brief Request Update of all plots
  virtual TArrayI GetBufferLevelArray(){ return TArrayI(10);} //*MENU*
  
  /// \brief Get Bytes Written this run
  virtual Long64_t GetBytesWritten(){ return 0; } //*MENU*
  
   // methods that must be implemented from base class NGMModule
   virtual bool init();
   virtual bool process(const TObject &);
   virtual bool finish();
   virtual void LaunchGui(); // *MENU*
   virtual void LaunchSpyServ(int port = 9090);
   virtual void ProcessSpyServ();
   virtual void SetStopDaqOnCtlC();
   virtual void ResetCtlC();
   // Methods for Threaded acquisition.
   void LaunchAcquisitionStartThread(); // *MENU*
   static void* StartThreadedAcquisitionMethod(void * arg);

private:
   /// The list of pointers to NGMSystems for this DAQ Configuration
   /// the NGMSystems owns these objects and is responsible for deallocating
   /// their memory.
     static std::vector<NGMSystem*> _ngmsystems;
protected:
   NGMSystemConfiguration* _config;
   TThread* acqThread; //!
  int _treeSaveSequence;
  NGMSpyServ* spySrv; //!
  TSignalHandler* stopDaqOnCtlC;//!
  
public:
   ClassDef(NGMSystem, 4)
};


#endif //__NGMSYSTEM_H__
