#ifdef __ROOTCLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclass    ;  // see http://root.cern.ch/viewvc/trunk/cint/doc/ref.txt
#pragma link C++ nestedtypedef  ;

#pragma link C++ defined_in "EXOUtilities/EXOErrorLogger.hh";
#pragma link C++ class EXOErrorLogger-!;
#pragma link C++ class EXOErrorLogger::LoggedError-!;

#pragma link C++ class EXOTemplWaveform<Double_t>+;
#pragma link C++ class EXOTemplWaveform<Float_t>+;
#pragma link C++ class EXOTemplWaveform<Int_t>+;
#pragma link C++ class EXOTemplWaveform<complex<double> >+;
#pragma link C++ class EXOTemplWaveform<unsigned long>+;
#pragma link C++ class EXOTemplWaveform<unsigned int>+;
#pragma link C++ class EXOTemplWaveform<Char_t>+;

//#pragma link C++ class EXOVWaveformTransformer+;

#pragma link C++ class EXOBaselineRemover+;
#pragma link C++ class EXOSmoother+;
#pragma link C++ class EXOPoleZeroCorrection+;
#pragma link C++ class EXORisetimeCalculation+;
#pragma link C++ class EXOExtremumFinder+;
#pragma link C++ class EXOTrapezoidalFilter+;
#pragma link C++ class EXODecayTimeFit+;
#pragma link C++ class EXOWaveformFT+;
#pragma link C++ class EXOMatchedFilter+;
#pragma link C++ class EXOFastFourierTransformFFTW+;

#endif
