//______________________________________________________________________________
//                                                                        
// EXOErrorLogger                                                           
// Singleton class that handles message logging for the EXOAnalysis
// package.  Generally most users will not use this directly, instead
// using the function:        
//
//   LogEXOMsg( "My Error", severity)
//
// to log information.  This function automatically logs the function and class
// in which the logging is requested so that the user doesn't need to
// explicitly give this information.  If the severity is either EEAlert or
// EEPanic, EXOErrorLogger will call exit(0) or abort(), respectively.
//
// Explicit requests can of course be made by the following:
//
//   EXOErrorLogger::GetLogger()
//
// which returns a reference to the logger.  For example, to print out a
// summary of the logger, do:
//
//   cout << EXOErrorLogger::GetLogger().GetSummary();
//
//______________________________________________________________________________

#include "EXOErrorLogger.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>
using namespace std;

static char const * const EXOErrorLevelNames[] = { 
  "Ok", 
  "Debug", /*"EEInfo",*/ 
  "Notice", 
  "Warning", 
  "Error", 
  "Critical", 
  "Alert", 
  "Panic" };
//______________________________________________________________________________
EXOErrorLogger::LoggedError::LoggedError(const std::string& cls, const std::string& afile, int aline,
                                         const std::string& msg, EXOErrorLevel level ) :
fClassName(cls),
fFilename(afile),
fLine(aline),
fMsg(msg),
fSeverity(level){} 

//______________________________________________________________________________
bool EXOErrorLogger::LoggedError::operator==(const EXOErrorLogger::LoggedError& rhs) const
{
  return (fClassName == rhs.fClassName) and (fFilename == rhs.fFilename) and (fLine == rhs.fLine) and
         (fMsg == rhs.fMsg) and (fSeverity == rhs.fSeverity);
}

//______________________________________________________________________________
bool EXOErrorLogger::LoggedError::operator<(const EXOErrorLogger::LoggedError& rhs) const
{
  return (fClassName < rhs.fClassName) or ((fClassName == rhs.fClassName) and   // 1 "("
         ((fFilename < rhs.fFilename) or ((fFilename == rhs.fFilename) and      // 2 "("
         ((fLine < rhs.fLine) or ((fLine == rhs.fLine) and                      // 2 "("
         ((fMsg < rhs.fMsg) or ((fMsg == rhs.fMsg) and                          // 2 "("
         (fSeverity < rhs.fSeverity) )) )) )) );
}

//______________________________________________________________________________
bool EXOErrorLogger::LoggedError::operator>(const EXOErrorLogger::LoggedError& rhs) const
{
  return (rhs < *this);
}

//______________________________________________________________________________
EXOErrorLogger::EXOErrorLogger() : 
  fPrintMax(10), 
  fThreshold(EENotice), 
  fPrintFunc(false),
  fCoutBuf(std::cout.rdbuf()),
  fCerrBuf(std::cerr.rdbuf())
{
}
//______________________________________________________________________________
EXOErrorLogger::~EXOErrorLogger()
{
  std::cout << GetSummary();
}
//______________________________________________________________________________
void EXOErrorLogger::LogError(std::string CLASSNAME, std::string filename, int aline,
			      std::string MESSAGE, EXOErrorLevel SEVERITY )
{
  // Log a message.  Users should consider instead using the macro:
  //
  // LogEXOMsg( MESSAGE, SEVERITY ) 
  // 
  // instead of calling this function directly as the macro automatically fills
  // in the CLASSNAME and filename and line number. 

  std::string tempFile = filename;
  size_t tempLoc = tempFile.find_last_of('/');
  if (tempLoc != std::string::npos) tempFile = tempFile.substr(tempLoc + 1); 

  LoggedError err(CLASSNAME, tempFile, aline, MESSAGE, SEVERITY);
  ErrSet::iterator iter = fErrors.find(err);
  
  if ( iter != fErrors.end() ) {
    if(iter->second == std::numeric_limits<int>::max()) LogEXOMsg("OVERFLOW in EXOErrorLogger.", EEAlert);
    if (iter->second < fPrintMax && iter->first.fSeverity >= fThreshold) {
      PrintError(iter->first);
      if ( iter->second == fPrintMax - 1 ) {
        std::cout << "Suppressing further output of this error" << std::endl;
      }
    }
    iter->second++;
    return;
  }

  // print this error for the first time

  if (err.fSeverity >= fThreshold) PrintError(err);

  // Add this error to the list

  fErrors[err] = 1;

}
//______________________________________________________________________________
std::string EXOErrorLogger::GetSummary(bool Suppress) const
{
  // Return the summary of logging messaged received until now.
  // Returned string starts with a header, and ends with a dashed line and newline.
  std::ostringstream Stream;
  const size_t outputWidth = 80;
  const size_t levelWidth = 9;
  const size_t descWidth = 12;
  const std::string errSum = "EXOErrorLogger Summary";
  Stream << setfill('-') << setw(outputWidth/2 - errSum.size()/2) << "-" 
         << setw(0) << errSum 
         << setfill('-') << setw(outputWidth/2 - errSum.size()/2) << "-" << std::endl; 
  Stream << left << setfill(' ')
         << setw(levelWidth) << "Level"
         << "Message information" << std::endl;
  Stream << setfill('-') << setw(outputWidth) << "-" << std::endl;
  
  bool didPrintOut = false;
  for (int intlevel = (int)(Suppress ? fThreshold : EEOk); intlevel <= (int)EEPanic; intlevel++ ) {
    EXOErrorLevel level = (EXOErrorLevel)intlevel;
    bool firstTime = true;
    for (ErrSet::const_iterator iter = fErrors.begin() ; iter != fErrors.end(); iter++ ) {
      const LoggedError& err = iter->first;
      if ( err.fSeverity != level ) continue;
      didPrintOut = true;
      if (firstTime) {
          Stream << setfill(' ') << setw(levelWidth) 
                 << left << ErrorLevel(err.fSeverity) << std::endl;
          firstTime = false;
      }
      
      std::string className = err.fClassName;
      Stream << setw(levelWidth+descWidth) << right 
             << "Calls: " << iter->second << std::endl 
             << setw(levelWidth+descWidth) << right 
             << "Function: " << err.fClassName << std::endl 
             << setw(levelWidth+descWidth) << right 
             << "Location: " << err.fFilename << ":" << err.fLine << std::endl 
             << setw(levelWidth+descWidth) << right 
             << "Message: " << err.fMsg << std::endl; 
      Stream << setfill(' ') << setw(outputWidth/2 - 1) << " " 
             << "--" << std::endl;
    }
  }

  if ( fErrors.size() == 0 ) Stream << "No errors or messages reported" << std::endl;
  else if ( not didPrintOut ) Stream << "Some suppressed errors/messages seen" << std::endl;

  Stream << setfill('-') << setw(outputWidth) << "-" << std::endl;
  return Stream.str();
}

//______________________________________________________________________________
void EXOErrorLogger::PrintError( const LoggedError& err ) const
{
  // Internal function to print an error

  ostream* output = &std::cout;
  // Redirect to std::cerr when severity higher than a Warning.
  std::string precursor = "(EXO " + std::string(ErrorLevel(err.fSeverity)) 
                          + "): ";
  if ( err.fSeverity >= EEWarning ) output = &std::cerr; 
  *output << precursor; 
  if (fPrintFunc) {
    *output << err.fClassName << std::endl 
            << std::setw(precursor.size()) << " "; 
  }
  *output << "@" << err.fFilename << ":" << err.fLine << ":" << std::endl
          << std::setw(precursor.size()) << " " 
          << "'" << err.fMsg << "'" << std::endl;

  if(err.fSeverity >= EEPanic) abort();
  if(err.fSeverity >= EEAlert) exit(1);
}

//______________________________________________________________________________
string EXOErrorLogger::ErrorLevel(EXOErrorLevel e)
{ 
  // Convert the severity enum to a string
  if (e < EEOk || e >  EEPanic) return "";
  return EXOErrorLevelNames[(int)e]; 
}
//______________________________________________________________________________
void EXOErrorLogger::SetOutputThreshold(std::string thresh)
{
  // overloaded function to change the output threshold using a string

  // transform to lowercase 
  std::transform( thresh.begin(), thresh.end(), 
                  thresh.begin(), ::tolower );
  for (int intlevel = (int)EEOk; intlevel <= (int)EEPanic; intlevel++ ) {
    EXOErrorLevel level = (EXOErrorLevel)intlevel;
    std::string temp = ErrorLevel(level);
    std::transform( temp.begin(), temp.end(), 
                    temp.begin(), ::tolower );
    if (temp == thresh) {
      SetOutputThreshold(level);
      return;
    }
  }
  LogEXOMsg("Level: " + thresh + " not found", EEWarning);
  
}

//______________________________________________________________________________
void EXOErrorLogger::ResetRedirection()
{
  EXOErrorLogger& me = GetLogger();
  std::cout.rdbuf(me.fCoutBuf);
  std::cerr.rdbuf(me.fCerrBuf);
}

//______________________________________________________________________________
void EXOErrorLogger::RedirectAllOutputTo(ostream& out)
{
  std::cout.rdbuf(out.rdbuf());
  //std::cerr.rdbuf(out.rdbuf());
}

class DevNull : public std::streambuf
{
  protected:
    int overflow( int c ) { return c; }
};

//______________________________________________________________________________
void EXOErrorLogger::RedirectAllOutputToDevNull()
{
  static DevNull gdevNull; 
  static std::ostream gDevNullOstream(&gdevNull);
  EXOErrorLogger::RedirectAllOutputTo(gDevNullOstream);
}
