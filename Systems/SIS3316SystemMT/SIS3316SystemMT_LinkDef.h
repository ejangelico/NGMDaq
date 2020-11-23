#ifdef __ROOTCLING__

//______________________________________________________________________________
//
// Root/Cint flags to generate streamer and dictionary functions
//
// These are sorted alphabetically by defining module name, e.g.
//
//   #pragma link C++ class     NGMCountDistribution+;
//   #pragma link C++ enum      NGMMomentId;   // NGMCountDistributions.h
//   #pragma link C++ typedef   NGMMomentId;   // NGMCountDistributions.h
//
// The enum and typedef immediately follow the class which defines them
//
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclass    ;  // see http://root.cern.ch/viewvc/trunk/cint/doc/ref.txt
#pragma link C++ nestedtypedef  ;
#pragma   link C++ class     SIS3316SystemMT+;
#endif
