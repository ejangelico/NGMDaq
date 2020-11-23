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

#include <vector>

/// \brief NGMSystem provides the API from which to derive all Hardware specific loadable modules.
///

class NGMSystem
{

public:

   ///Constructor
   NGMSystem() {}

   ///Destructor
   ~NGMSystem();
   
   /// \brief Register an LCLCutter for analysis.
   /// @param LCLCutter an analysis dependent inplementation derived from the LCLCutter base class
   ///
   void registerSystem(NGMSystem* newSystem);
   
   /// \brief Configure this acquisition system using an archived configuration file
   /// @param configname provides the location of configuration file 
   ///
   int readConfigFile(const char* configname, const std::string overrideFilename = "");
   
   /// \breif Request a list of System wide parameters
   /// \breif Request a list of Slot specific parameters
   /// \breif Request a list of Channel specific parameters
   
private:
   /// The list of pointers to NGMSystems for this DAQ Configuration
   /// the NGMSystems owns these objects and is responsible for deallocating
   /// their memory.
   static std::vector<NGMSystem*> _ngmsystems;
};

#endif //__NGMSYSTEM_H__
