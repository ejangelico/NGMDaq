#ifndef EXOErrorLogger_hh 
#define EXOErrorLogger_hh
#include <map>
#include <string>

#define LogEXOMsg(errmsg, severity) \
 EXOErrorLogger::GetLogger().LogError(__PRETTY_FUNCTION__,__FILE__, __LINE__,errmsg, severity)
#define LogEXOMsgShort(errmsg, severity) \
 EXOErrorLogger::GetLogger().LogError(__FUNCTION__,__FILE__, __LINE__,errmsg, severity)

enum EXOErrorLevel {
  EEOk = 0,   // all fine
  EEDebug,    // debug-level
  //EEInfo,     // informational
  EENotice,   // normal but significant
  EEWarning,  // warning condition
  EEError,    // error condition
  EECritical, // critical condition
  EEAlert,    // still can shutdown (calls exit)
  EEPanic     // and now we crash (calls abort)
};

class EXOErrorLogger
{
private:

  class LoggedError {
    public:  
      LoggedError( const std::string& cls, const std::string& afile, int aline,
                   const std::string& msg, EXOErrorLevel level );
      std::string fClassName;
      std::string fFilename;
      int fLine;
      std::string fMsg;
      EXOErrorLevel fSeverity;
      bool operator== (const LoggedError& rhs) const;
      bool operator< (const LoggedError& rhs) const;
      bool operator> (const LoggedError& rhs) const;
  };

  int fPrintMax;            // Maximum number of times a msg may be printed
  EXOErrorLevel fThreshold; // Minimum threshold to be printed
  bool          fPrintFunc; // Print Function information, default false 

  typedef std::map<LoggedError, int> ErrSet;
  ErrSet fErrors;           // Map holding msgs 

  void PrintError( const LoggedError& err ) const;
  
  // Make private to make it a singleton
  EXOErrorLogger();
  EXOErrorLogger(const EXOErrorLogger&);
  EXOErrorLogger& operator=(const EXOErrorLogger&);
  ~EXOErrorLogger();

  std::streambuf* fCoutBuf;
  std::streambuf* fCerrBuf;
  
public:

  static EXOErrorLogger& GetLogger() 
  { 
    static EXOErrorLogger gfLogger; return gfLogger; 
  } 

  void LogError(std::string classfuncname, std::string filename, int aline,
                std::string msg, EXOErrorLevel severity );

  std::string GetSummary(bool Suppress = true) const;

  // Sets the maximum times an error can be printed
  void SetPrintMaximum( int value ) { fPrintMax = value; }

  // Sets the output lower limit threshold, that is, 
  void SetOutputThreshold( EXOErrorLevel value ) { fThreshold = value; }
  void SetOutputThreshold( std::string threshold );
  EXOErrorLevel GetOutputThreshold() const { return fThreshold; }
  void SetPrintFunction(bool aval = true) { fPrintFunc = aval; }

  static std::string ErrorLevel(EXOErrorLevel e);

  static void RedirectAllOutputTo(std::ostream& out);
  static void ResetRedirection();
  static void RedirectAllOutputToDevNull();
  static void ClearOutputRecord() { GetLogger().fErrors.clear(); }
};

#endif


