// $Id: $

#ifndef _LOGGER_
#define _LOGGER_

#include <map>       // std::map hash table, for tracing
#include <sstream>   // std:: stream objects

#include "TError.h"  // ErrorHandlerFunc_t

// Seems silly to use root for string buffer but both std::string
// and std:strstream behave badly under vc++
#include "TString.h"
#include <Riostream.h>

//__________________________________________________________________________
//
// The class is used in the following way:
//
//  LOG <<"This is a message"<<some_number<<"more message"<<ENDM_INFO;
//  LOGF<<"Another message"  <<some_number<<"more message"<<ENDM_INFO;
//
// There are 3 severity levels, selected with the closing
// key word, e.g. INFO in the example above.
// Valid keys are                         LOG<<ENDM_INFO;
//                                        LOG<<ENDM_WARN;
//                                        LOG<<ENDM_FATAL;
// (Note:  FATAL does not terminate the program...)
//
// You can set the number format to hex:  LOG<<hex;  LOG<<LOG_HEX;  (equivalent)
// And back to decimal:                   LOG<<dec;  LOG<<LOG_DEC
//
// Output is buffered until the
// closing ENDM_xxxx.
//
// You can discard the buffered output:   LOG<<ENDM_RESET;
// (before you've sent an ENDM_xxxx)
//
// You can colorize the output:           LOG<<color;  LOG<<LOG_COLOR
// And switch back to black:              LOG<<bandw;  LOG<<LOG_BANDW 
//
// You can capture root output or cout:   LOG<<BEGM_LOGROOT;  // start capture
//                                        <do some stuff>
//                                        LOG<<ENDM_LOGROOT;  // end capture
//                                        if (<you want the output>) {
//                                          LOG<<"Root said:"<<ENDM_WARN
//                                          LOG<<ENDM_ROOTOUT<<ENDM_WARN;
//                                        }
// 	 
// There is no reset for ROOTOUT;
// the next BEGM_LOGROOT does that.
// 
// The logger works in root interactive macros:
// First, at the root prompt, load
// NGMLogger.h by hand:                   .L NGMLogger.h
// Then, use the logger as normal in your
// macro, e.g.,                           #include "NGMLogger.h"
//                                        {
//                                          LOG<<"Hello, world."<<ENDM_INFO;
//                                        }
// The only big difference is in LOGF:    LOGF()<<"message"...
// (LOGF is a function, #defines are
// erratic in CINT)
//
// Minor differences:
// Macros:             Compiled Code
// Use constants:      Instead of controls:
//   LOG<<LOG_HEX      LOG<<hex
//   LOG<<LOG_DEC      LOG<<dec
//   LOG<<LOG_COLOR    LOG<<color
//   LOG<<LOG_BANDW    LOG<<bandw
// The constants are also defined in compiled code,
// so you won't have to make any changes.
//
//
// Finally, there is also a tracing interface.
//
// In the top of your code:               #define TRACE_MODULE "MyClassFileName"
//                                        #include "NGMLogger.h"
// This registers the module name, with
// default tracing level 1.
//
// Now, on entry to your functions:       void MyClass::foo(int bar) {
//                                          TRACE<<ENDM_INFO;
// You can also add other output:
//                                        TRACE<<"bar = "<<bar<<ENDM_INFO;
//
// For higher level messages, use         TRACE2<<
// These will print at tracing level      TRACE3<<
// 2 and above, or 3 and above.
//
// You can change the tracing level       LOG::Tracing("MyClassFileName", 2);
// elsewhere, in code, or even at the
// root prompt.

#if !defined(__CINT__)

// shorthands
#define LOG (*NGMLogger::instance())
#define LOGF LOG<<__FUNCTION__<<"("<<__LINE__<<")\t"

// controls
#define ENDM_INFO  NGMLogger::info   <<__ENDM_BASE__
#define ENDM_WARN  NGMLogger::warning<<__ENDM_BASE__
#define ENDM_FATAL NGMLogger::fatal  <<__ENDM_BASE__
#define ENDM_RESET NGMLogger::reset

#define BEGM_LOGROOT NGMLogger::logroot
#define ENDM_LOGROOT NGMLogger::endroot
#define ENDM_ROOTOUT NGMLogger::rootout

// modifiers
#define LOG_HEX   NGMLogger::hex
#define LOG_DEC   NGMLogger::dec
#define LOG_COLOR NGMLogger::color
#define LOG_BANDW NGMLogger::bandw
#define LOG_FLUSH NGMLogger::flush

// Set up tracing interface
#ifndef TRACE_MODULE
#define TRACE_MODULE __FILE__
#endif
#ifndef TRACE_LEVEL
#define TRACE_LEVEL 1
#endif
#define TRACE_REGISTER LOG.RegisterModule(TRACE_MODULE, TRACE_LEVEL)

#define TRACING(level) LOG.Tracing(TRACE_MODULE,level,TRACE_LEVEL)
//#define TRACING(level) ( (level) <= TRACE_LEVEL ? true : false )

#define TRACE  TRACEL(1)
#define TRACE2 TRACEL(2)
#define TRACE3 TRACEL(3)
#define TRACEL(level) if (TRACING(level)) \
	LOG<<TRACE_MODULE<<"::"<<__FUNCTION__<<"("<<__LINE__<<"):\t"

#define LOGCONCAT(a,b) LOGCONCAT_(a,b)
#define LOGCONCAT_(a,b) a ## b

// special conditional terminator
#define TRACE_ENDM(level,endm)                   \
  if (TRACING(level)) LOG<< LOGCONCAT(ENDM_,endm) ; \
  else LOG<<ENDM_RESET


#endif // ! __CINT__ 

#if !defined(__CINT__) || defined(__MAKECINT__) 

//The NGMLogger is a singleton so the output can be assigned to one output.
class NGMLogger
{

 public:
  enum levelE   {info,warning,fatal};  //The severity levels
  enum controlE {hex,dec,  //You can set the format to hex or dec just as one does for cout.
				 endm,     //Ignore endm, that is used to end a message
				 flush,    //Like std::flush
				 reset,    //Discard the queue without output
				 logroot,  //Capture root output to a separate queue
				 endroot,  //End root capture
				 rootout,  //Print root queue on next ENDM_xxxx
				 color,    //Colorize output
				 bandw     //Black and white output
  };


  static NGMLogger* instance();  //singleton

  //The following list is an enumeration of all of the types that the NGMLogger
  // can read.
  NGMLogger& operator<< (const char*);
  NGMLogger& operator<< (int);
  NGMLogger& operator<< (unsigned int tmp){*this<<(int)tmp; return *this;};
  NGMLogger& operator<< (long);
  NGMLogger& operator<< (long long);
  NGMLogger& operator<< (unsigned long long);
  NGMLogger& operator<< (double);

  NGMLogger& operator<< (controlE);
  NGMLogger& operator<< (levelE);
  NGMLogger& operator<< (const wchar_t*); // hack for CINT

  //returns true if file was opened.
  bool openFile(const char*);
  //select where the output goes.
  enum output {stdError,diskFile};
  void setOutputSelector(output o) {_outputSelector = o;};

  // capture root output
  static void RootErrorHandler(int level, Bool_t abort, const char *location,
                               const char *msg);

  // register module, with default tracing level
  void RegisterModule(const char *module, const int defaultLevel);
  // change the default tracing level, returns old level
  int SetLevel(const char *module, const int newLevel);
  // are we tracing this module at this level?
  bool Tracing       (const char *module, const int level, const int deflevel);
  // print the tracing level for each registered module
  void PrintTracingLevels();

					  
 protected:
  NGMLogger();

 private:

  static NGMLogger* ptr_instance; // singleton
  TString        _queue;          // queue messages until ENDM_xxx
  levelE         _level;          // severity of current queue
  output         _outputSelector; // Where the output is logged
  std::ofstream* _file;           // if _outputSelector is "file" then this is the file pointer.
  bool           _hex;            // are the numbers in hex mode? default dec
  bool           _color;          // colorize output?

  TString        _rootQ;          // queue root messages until ENDM_logroot
  bool           _oldgPrintViaErrorHandler ; // save old state while capturing
  ErrorHandlerFunc_t _oldgErrorHandler;      // save old eh while capturing
  std::streambuf *_cout;          // saved cout
  //std::stringbuf  _sout;          // divert cout to here
  std::map<const char *, int>
	              _trace;         // tracing levels

  // member functions
  void           Label_queue();   // prepend severity label, and colorize
  TString       &RootQ() { return _rootQ ; }  // provide access to RootErrorHandler

};


#endif // ! __CINT__ || __MAKECINT__

// Implementation details ______________

#if !defined( __CINT__ )

// some private defines to facilitate CINT hack
#define LOGLOC3(f,l,n) " [file:"<<f<<" line:"<<l<<" function:"<<n<<"]"
#define LOGLOC_BASE LOGLOC3(__FILE__,__LINE__,__FUNCTION__)
#define LOGLOC LOGLOC_BASE

#define __ENDM_BASE__ NGMLogger::dec<<LOGLOC<<NGMLogger::endm

#else

//  CINT implementation ________________

NGMLogger &LOG = *NGMLogger::instance();

#include <stdlib.h>
#include <string.h>

wchar_t ENDM_BASE_w    = 'B';  wchar_t *ENDM_BASE    = &ENDM_BASE_w   ;

wchar_t ENDM_INFO_w    = 'I';  wchar_t *ENDM_INFO    = &ENDM_INFO_w   ;
wchar_t ENDM_WARN_w    = 'W';  wchar_t *ENDM_WARN    = &ENDM_WARN_w   ;
wchar_t ENDM_FATAL_w   = 'F';  wchar_t *ENDM_FATAL   = &ENDM_FATAL_w  ;
wchar_t ENDM_RESET_w   = 'R';  wchar_t *ENDM_RESET   = &ENDM_RESET_w  ;

wchar_t BEGM_LOGROOT_w = 'L';  wchar_t *BEGM_LOGROOT = &BEGM_LOGROOT_w;
wchar_t ENDM_LOGROOT_w = 'E';  wchar_t *ENDM_LOGROOT = &ENDM_LOGROOT_w;
wchar_t ENDM_ROOTOUT_w = 'O';  wchar_t *ENDM_ROOTOUT = &ENDM_ROOTOUT_w;

wchar_t LOG_FLUSH_w    = 'f';  wchar_t *LOG_FLUSH    = &LOG_FLUSH_w  ;
wchar_t LOG_HEX_w      = 'x';  wchar_t *LOG_HEX      = &LOG_HEX_w   ; 
wchar_t LOG_DEC_w      = 'd';  wchar_t *LOG_DEC      = &LOG_DEC_w   ; 
wchar_t LOG_COLOR_w    = 'c';  wchar_t *LOG_COLOR    = &LOG_COLOR_w   ; 
wchar_t LOG_BANDW_w    = 'b';  wchar_t *LOG_BANDW    = &LOG_BANDW_w   ; 

NGMLogger &LOGF()         { LOG<<"[interpreted]\t";  return LOG; }

#endif // !__CINT__ 

#endif // _LOGGER_
