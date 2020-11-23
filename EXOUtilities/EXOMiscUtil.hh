// $ Header:$
#ifndef EXOMiscUtil_hh
#define EXOMiscUtil_hh

#include "Rtypes.h"
#include <string>
#include <vector>
#include <map>
#include <exception>
#include <complex>
#include <stdexcept>
class EXOTransferFunction;
class EXOControlRecordList;
class TObject;
class TChain;
class TCanvas;
class TTimeStamp;
class TApplication;
namespace ROOT {
  namespace Math { template <class A, unsigned int B> class SVector; }
}
template<typename _Tp> class EXOTemplWaveform;
typedef EXOTemplWaveform<Double_t> EXODoubleWaveform;

/** @file EXOMiscUtil.h
    @author J. Bogart

    This file declares the class Util for basic static utilities 
    and an associated exception class.
    
*/
/// This class provides a home for utility functions with no need for
/// any context (hence static)

/// Exception class used by expandEnvVar
class EXOExceptUntranslatable : public std::exception {
public:
  EXOExceptUntranslatable(const std::string& toTrans = "") : 
    std::exception(), m_badVar(toTrans) {}
  virtual ~EXOExceptUntranslatable() throw() {}
  std::string m_badVar;
  virtual const char* what() const throw() {
    return m_badVar.c_str();
  }
};

/// Exception class used when converting from string to numeric type

class EXOExceptWrongType : public std::exception {
public:
  EXOExceptWrongType(const std::string& toConvert="", 
                     const std::string& typeName="") : 
    std::exception(), m_toConvert(toConvert), m_typeName(typeName) {
    m_msg = std::string("facilities::WrongType. Cannot convert '") +  
      m_toConvert;
    m_msg += std::string("'to type ");
    m_msg += m_typeName;
  }
  virtual ~EXOExceptWrongType() throw() {}
  std::string getMsg() const {
    return m_msg;
  }
  virtual const char* what() const throw() { 
    return (m_msg.c_str());
  }
private:
  std::string m_toConvert;
  std::string m_typeName;
  std::string m_msg;
};

typedef ROOT::Math::SVector<double,4> EXOSVec;
namespace EXOMiscUtil 
{

  typedef std::map<std::string,double> ParameterMap;
  typedef std::map<int,ParameterMap> ChannelInfoMap;

  // Exception class used by talk-to manager.
  // Follows exactly the model from runtime_error.
  // I create a new type of command mainly to ensure that we catch exactly what we ourselves throw, no more.
  // (So, if something else throws, we shouldn't catch it in EXOTalkToManager, even if it's a runtime error.)
  class EXOBadCommand : public std::runtime_error {
  public:
    explicit EXOBadCommand(const std::string& badCommand) : std::runtime_error(badCommand) {fWhat = badCommand;}
    explicit EXOBadCommand(const char* badCommand) : std::runtime_error(badCommand) {fWhat = badCommand;}
    virtual ~EXOBadCommand() throw() {};
    void append(const std::string& toAppend) {fWhat.append("\n" + toAppend);}
    virtual const char* what() const throw() {return fWhat.c_str();}
  private:
    std::string fWhat;
  };


  /** Given input string @a toExpand expand references to environment
      variables, by default of the form $(varname) and put the 
      expanded version back into the original string.  Alternate
      delimiters for the @a varname may optionally be specified
      @param   toExpand string for which expansion is to be done
      @param   openDel opening delimiter (defaults to "$(")
      @param   closeDel closing delimiter (defaults to ")")
      
      @return  -1 if attempt at expansion failed at least once,
      else number of successful expansions.
      
      TODO:  Perhaps add optional arguments to specify alternate
      delimiters.
  */
  int expandEnvVar(std::string* toExpand, 
                          const std::string& openDel = std::string("$("),
                          const std::string& closeDel = std::string(")"));

  /**  
       Expand environment variables in the string, using standard conventions
       of current OS.
       For Windows, use delimiters %    %
       For Linux or other unix, use ${    }
  */
  int expandEnvVarOS(std::string* toExpand);
  
  /** Given an input vector of strings, each of which may contain an
      env variable, we construct a new vector of strings, where each
      env variable has been expanded.
      @param toExample
      @param result will be modified by this method
      @param delimiters is a string which defaults to ",".  This is the list
      of delimiters to use when tokenizing one of the strings in the vector
  */
  int expandEnvVarList(const std::vector<std::string>& toExpand,
                              std::vector<std::string> &result,
                              const std::string &delimiters=",");

  /** Given an input integer @a val to convert and an output string @a outStr
      converts val into a std::string.
      This method duplicates the stdlib.h method itoa, except that it returns
      std::string rather than char*.
      @param   val
      @param   outStr will be modified by this method
      
      @return  const char* based on the contents of outStr.c_str()
  */
  const char* itoa(int val, std::string &outStr);
  
  /**
     Given unsigned, convert to output string. If optional  @arg base
     is 16, format as 8 hex digits; else output decimal.
  */
  const char* utoa(unsigned int val, std::string &outStr,
                          int base=10);
  
  /// converts an std::string to an integer
  int atoi(const std::string& InStr);
  
  
  /// converts a std::string to a double.  If string contents are not
  /// of proper form, throws facilities::WrongType
  double stringToDouble(const std::string& InStr);
  
  /// converts a std::string to an int.  If string contents are not
  /// of proper form, throws facilities::WrongType
  int stringToInt(const std::string& InStr);
  
  /// converts a std::string to an unsigned int.  If string contents are not
  /// of proper form, throws facilities::WrongType
  unsigned int stringToUnsigned(const std::string& InStr);
  
  /// converts a std::string to an unsigned long long.  If string contents are
  /// not of the proper form, throws facilities::WrongType
  unsigned long long stringToUll(const std::string& InStr);
  
  
  /** This routine breaks down a string into tokens, based on the
      characters appearing in the string @a delimiters.
      @param input       string to be tokenized
      @param delimiters  string containing one or more delimiter characters
      @param tokens      vector of strings to hold resulting tokens
      @param clear       if true (default) @a tokens will be cleared
      at the start of processing
  */
  void stringTokenize(std::string input, const std::string &delimiters,
                             std::vector<std::string> &tokens,
                             bool clear = true);
  
  /** This routine breaks down a string into key/value token pairs and
      stores them in the user-supplied map. , based on the
      characters appearing in the string @a delimiters and the value
      of @a pairDelimiter.  In a typical example, @a input could be
      "key1=val1,key2=val2,key3=val3".  In this case invoke with
      delimiters=std::string(",") and pairDelimiter=std::string("=")
      (or omit pairDelimiter since it has the default value) 
      @param input         string to be tokenized
      @param delimiters    string containing one or more delimiter 
      characters
      @param tokenMap      map of strings to hold resulting tokens
      @param pairDelimiter string separating key and value; defaults
      to "=" 
      @param clear         if true (default) @a tokens will be cleared
      at the start of processing
  */
  void keyValueTokenize(std::string input, 
                               const std::string &delimiters,
                               std::map<std::string,std::string> &tokenMap,
			       const std::string& pairDelimiter = 
                               std::string("="),
                               bool clear = true);
  
  /** return the "non-directory" part of a (supposed) file identifier, 
      @a path.  Environment variable translation should be done before 
      calling @a basename.
      @sa { Util::expandEnvVar }
      @param path        string assumed to be a file identifier. 
  */
  std::string basename(const std::string &path);
  
  /**
     Trim leading white space characters from the supplied string.
     White space characters for this purpose are blank, carriage return,
     line feed and form feed.  Return # of characters trimmed.
  */
  unsigned trimLeading(std::string* toTrim);
  
  /**
     Trim trailing white space characters from the supplied string.
     White space characters for this purpose are blank, carriage return,
     line feed and form feed.  Return # of characters trimmed.
  */
  unsigned trimTrailing(std::string* toTrim);
  
  /**
     Trim leading+trailing white space characters from the supplied string.
     White space characters for this purpose are blank, carriage return,
     line feed and form feed.  Return # of characters trimmed.
  */
  unsigned trim(std::string* toTrim);
  
  /**
     Wrapper handles different system sleep interfaces for Windows,
     unix/Linux
  */
  void gsleep(unsigned milli);

  /**
    Serializes a TObject and returns the object as a std::string.
    A zero-length string is returned if a failure occurs.
    This also includes the StreamerInfo information by default.
  */
  std::string SerializeTObject( TObject* object, 
                                bool useBase64 = true,
                                bool includeStreamerInfo = true);

  /**
    Unserializes a TObject serialized in a std::string using
    SerializeTObject.  Returns a pointer (owned by the caller)  
    to a TObject, NULL if the function fails.   
  */
  TObject* UnserializeString( const std::string& bufferstr );

  /* delta_compression takes the int array data and converts into the unsigned short array qdata.
  Memory for qdata should be allocated by the function that calls delta_compression, and
  the length of the array should be passed as maxqdata. delta_compression returns the 
  number of elements of qdata which were used. delta_uncompression does the same but
  in reverse. If the available array space is not large enough, both functions return -1.
  */
  int delta_compression( int *data, int nsample, unsigned short *qdata, int maxqdata, int nbit  );
  int delta_uncompression( unsigned short *qdata, int qlength, int *data, int maxdata, int nbit );

  /*
   Identify trees in a root file.

   When adding a tree, it is important for auto-documentation that you follow the format here:
   Get***TreeName()
   Get***BranchName()
   Get***TreeDescription()
   Get***ClassName()
   */
  inline std::string GetEventTreeName() { return "tree"; } 
  inline std::string GetEventBranchName() { return "EventBranch"; }
  inline std::string GetEventTreeDescription() { return "Data from EXO Runs"; }
  inline std::string GetEventClassName() { return "EXOEventData"; }
  inline std::string GetWaveformBranchName() { return "fWaveformData"; } 

  inline std::string GetGlitchTreeName() { return "glitch"; }
  inline std::string GetGlitchBranchName() { return "GlitchBranch"; }
  inline std::string GetGlitchTreeDescription() { return "HV glitches extracted from the veto records"; }
  inline std::string GetGlitchClassName() { return "EXOGlitchRecord"; }

  inline std::string GetVetoTreeName() { return "veto"; }
  inline std::string GetVetoBranchName() { return "VetoBranch"; }
  inline std::string GetVetoTreeDescription() { return "Veto events"; }
  inline std::string GetVetoClassName() { return "EXOVetoEventHeader"; }

  inline std::string GetStatisticsTreeName() { return "stats"; }
  inline std::string GetStatisticsTreeDescription() { return "Statistics of EXOAnalysis"; }

  inline std::string GetMuonTreeName() { return "muon"; }
  inline std::string GetMuonBranchName() { return "MuonBranch"; }
  inline std::string GetMuonTreeDescription() { return "Information for events tagged as muons"; }
  inline std::string GetMuonClassName() { return "EXOMuonData"; }

  inline std::string GetMCTrackTreeName() { return "mctrack"; }
  inline std::string GetMCTrackBranchName() { return "MCTrackBranch"; }
  inline std::string GetMCTrackTreeDescription() { return "Information about particle tracks processes by MC"; }
  inline std::string GetMCTrackClassName() { return "EXOMCTrackInfo"; }
	
  inline int GetTreeCompressionLevel() { return 4; } 

  // This function calculates the wire signals from the LXe hit info.
  // EXOt0 is the global time of the first hit to consider. Signals 
  // are calculated for a time nsample*SAMPLE_TIME - trigger_time after
  // EXOt0.

  // This function calculates the APD signals from the APD hit info.
  // EXOt0 is the global time of the first hit to consider. Signals 
  // are calculated for a time nsample*SAMPLE_TIME - trigger_time after
  // EXOt0.
  void RC_shaper( Double_t *signal_in, Double_t *signal_out, Int_t n, 
		  Double_t tratio );

  void CR_shaper( Double_t *signal_in, Double_t *signal_out, Int_t n, 
		  Double_t tratio, Double_t baseline );
 
  typedef std::complex<double> CmplNum;
  CmplNum H_omega_RC( Double_t omega_tau );

  CmplNum H_omega_CR( Double_t omega_tau );
  void realft(float data[], unsigned long n, int isign);

  void four1(float data[], unsigned long nn, int isign);

  enum ECoordinateSystem {
    kUVCoordinates,
    kXYCoordinates
  };
  void UVToXYCoords(double u, double v, double& x, double& y, double z); 
  void XYToUVCoords(double& u, double& v, double x, double y, double z); 

  enum ETPCSide {
    kNorth=0,
    kSouth=1
  };

  enum EChannelType {
    kUWire,
    kVWire,
    kAPDGang,
    kAPDSumOfGangs,
    kUWireInduction,
    kOtherTag
  };
  inline bool ChannelIsWire(EChannelType type) {return (type == kUWire or type == kVWire);}
  inline bool ChannelIsUWire(EChannelType type) {return (type == kUWire);}
  inline bool ChannelIsVWire(EChannelType type) {return (type == kVWire);}
  inline bool ChannelIsAPD(EChannelType type) {return (type == kAPDGang or type == kAPDSumOfGangs);}
  inline bool ChannelIsAPDGang(EChannelType type) {return (type == kAPDGang);}
  inline bool ChannelIsAPDSumOfGangs(EChannelType type) {return (type == kAPDSumOfGangs);}
  inline bool ChannelIsUWireInduction(EChannelType type) {return (type == kUWireInduction);}
  EChannelType TypeOfChannel(int SoftwareChannelNumber);
  int GetClosestUChannel(double uPosition, ETPCSide side);
  int GetClosestVChannel(double vPosition, ETPCSide side);

  TApplication &GetApplication();

  TCanvas &GetDebugCanvas();
  // Will display a histogram (or anything that has a Draw command) in a program, useful for debugging purposes.
  void DisplayInProgram(std::vector<TObject*>& vecofhists, const std::string& message = "", const std::string& drawOpts = "");

  void DisplayInProgram(TObject& hist, const std::string& message = "", const std::string& drawOpts = "");

  void DumpHistToFile(std::vector<TObject*>& vecofhists, const std::string& filename = "output.png", const std::string& message = "", const std::string& drawOpts = "");

  void DumpHistToFile(TObject& hist, const std::string& filename = "output.png", const std::string& message = "", const std::string& drawOpts = "");

  // Shows a debug prompt and blocks until user exits the prompt.
  void ShowDebugPrompt();

  void CR_unshaper( Double_t *signal_in, const Double_t *signal_out, Int_t n, 
          	  Double_t tratio, Double_t baseline );
  
  // signal_in[0] should have already been set as an unknown.
  // parameter 0 will be used as the correction to baseline_guess.
  void CR_unshaper( std::vector < EXOSVec > &signal_in,
                    std::vector < EXOSVec > &signal_out, 
                    Int_t n, Double_t tratio, Double_t baseline_guess );
  
  void RC_unshaper( Double_t *signal_in, const Double_t *signal_out, Int_t n, 
          	  Double_t tratio );
  
  
  // signal_in[0] should have already been set as an unknown
  void RC_unshaper( std::vector < EXOSVec > &signal_in, 
                    std::vector < EXOSVec > &signal_out,
                    Int_t n, Double_t tratio );
  Double_t* unshape_with_parameters_impl ( const Int_t* shaped_waveform, Int_t& length, Int_t& increment_forward,
                const EXOTransferFunction& transfer, Double_t baseline );
  
  // The following function is implemented for portability; faster would be a bitwise operation, but we never need to do this quickly.
  bool is_power_of_two(Int_t n);
  
  const EXODoubleWaveform& unshaped_signal(); 
  const EXODoubleWaveform& unshaped_ind_signal(); 
  bool CanWeUse_noise_sq_mag_apd(const EXOTransferFunction& transfer);
  const EXODoubleWaveform& noise_sq_mag_apd();
  bool CanWeUse_noise_sq_mag_wire(const EXOTransferFunction& transfer);
  const EXODoubleWaveform& noise_sq_mag_wire();
  Double_t GetDigitizerRescaling(const Int_t chan);

  std::string SearchForFile(const std::string& filename);
  std::vector<std::string> SearchForMultipleFiles(const std::string& filename); 
  inline std::string GetDefaultPath() { return ".:${EXOLIB}"; }
  TTimeStamp& GetStartTimeOfProcess();

  ETPCSide GetTPCSide(unsigned int channel);
  bool OnSameDetectorHalf(int channel1, int channel2);

  double GetMeanUorVPositionFromChannel(int channel);

  double GetGaussVar();

  std::map<Int_t, EXOControlRecordList> ExtractControlRecords(TChain& chain,
    std::vector<TClass*> FilterRecords = std::vector<TClass*>());

  const int kUWireIndOffset = -500;
}

inline bool EXOMiscUtil::is_power_of_two(Int_t n)
{
  if ( n<=0 ) {
    return false;
  }
  while( n > 1 ) {
    if ( n%2 != 0 ) {
      return false;
    }
    n /= 2;
  }
  return true;
}

// This calculates the transfer function for an RC filter

inline EXOMiscUtil::CmplNum EXOMiscUtil::H_omega_RC( Double_t omega_tau )
{

  Double_t ot2 = omega_tau*omega_tau;

  return CmplNum(1.0/(1+ot2), -1.0*omega_tau/(1+ot2));
  
}

// This calculates the transfer function for an CR filter

inline EXOMiscUtil::CmplNum EXOMiscUtil::H_omega_CR( Double_t omega_tau ) 
{

  Double_t ot2 = omega_tau*omega_tau;

  CmplNum H = CmplNum(ot2/(1+ot2), 0.0);

  if ( omega_tau != 0.0 ) H += CmplNum(0.0, H.real()/omega_tau);
  return H;

}

#endif
