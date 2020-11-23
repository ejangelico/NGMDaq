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
#pragma   link C++ class     CoincidenceVariables+;
#pragma   link C++ class     CoincidentEvent+;
#pragma   link C++ class     CoincidenceMaker+;
#pragma   link C++ class     CoincidenceImageCut+;
#pragma   link C++ class     CoincidenceImager+;
#pragma   link C++ class     CoincToTuple+;
#pragma   link C++ class     ImagerRunDescription+;
#pragma   link C++ class     ImageMaker+;
#pragma   link C++ class     NGMBlockAliveMonitor+;
#pragma   link C++ class     NGMBlockArrayMonitor+;
#pragma   link C++ class     NGMBlockArrayMonitorDisplay+;
#pragma   link C++ class     NGMBlockBaseline+;
#pragma   link C++ class     NGMBlockDetectorCalibrator+;
#pragma   link C++ class     NGMBlockFlood+;
#pragma   link C++ class     NGMBlockFloodDisplay+;
#pragma   link C++ class     NGMBlockMapping+;
#pragma   link C++ class     NGMBlockPSD+;
#pragma   link C++ class     NGMBlockPSDMaker+;
#pragma   link C++ class     NGMPixelADCMonitor+;
#pragma   link C++ class     NGMPixelADCMonitorDisplay+;
#pragma   link C++ class     NGMBlockPicoDST+;
#pragma   link C++ class     InteractiveDecoder+;
#pragma   link C++ class     PDSTImager+;
#pragma   link C++ class     PDSTImagerSelector+;
#pragma   link C++ class     NGMSISCalibrator+;
#pragma   link C++ class     LinearScanAnalysis+;
#pragma   link C++ class     DepthScan+;
#pragma   link C++ class     MyArray2d+;
#pragma   link C++ class     NGMBlockMultipleScattering+;
#pragma   link C++ class     NGMBlockMultipleScatteringModule+;
#pragma   link C++ class     NGMBlockFNWellCounter_tuple_st+;
#pragma   link C++ class     NGMBlockFNWellCounter+;
#endif
