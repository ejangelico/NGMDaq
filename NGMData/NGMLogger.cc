//---------------------------------------------------
/*! \class NGMLogger
\brief A nice interface to the NGMLogger system

This NGMLogger proveds an easy to work with interface similar to cerr.
*/
//-----------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "Riostream.h"
#include "TEnv.h"
#include "TSystem.h"
#include "TVirtualMutex.h"

#include "NGMLogger.h"

// Logger local tracing
// Set LTRACING to 'true' to turn Logger local tracing on, false for off
#define LTRACING   false
#define LTRACE_MODULE "NGMLogger"
#define LTRACE_CMD cerr<<LTRACE_MODULE<<"::"<<__FUNCTION__<<": "
#define LTRACE     if(LTRACING) LTRACE_CMD


using namespace std;

NGMLogger* NGMLogger::ptr_instance=0;

NGMLogger* NGMLogger::instance(){
  LTRACE<<endl; // debug
  if(ptr_instance==0){
    ptr_instance = new NGMLogger();
  }
  return ptr_instance;
}

/*!
This is the default constructer and when it is called the output is sent to the standard place, cerr.
*/
NGMLogger::NGMLogger()
  :_queue()
//,_sout()
{
  LTRACE<<endl;
  _outputSelector=stdError;
  _file=NULL;
  _cout = 0;
  _color = false;
  _oldgPrintViaErrorHandler = false;
  _oldgErrorHandler = 0;
}


bool NGMLogger::openFile(const char* filename)
{
  LTRACE<<endl;
  _file = new ofstream;
  _file->open(filename,std::ios_base::app);
  if(_file) {
	setOutputSelector(diskFile);
    return true;
  }
  return false;
}

/*! These are the operator overloads that allow you to use this class like cerr*/
NGMLogger& NGMLogger::operator<<(const char* str)
{
  LTRACE<<"str: "<<str<<endl; // debug
  _queue+=str;
  return *this;
}

NGMLogger& NGMLogger::operator<<(int in)
{
  LTRACE<<"int: "<<in<<endl; // debug
  char tmp[100];
  if(_hex) {
    sprintf(tmp,"%x",in);
  }else{
    sprintf(tmp,"%i",in);
  }
  _queue+=tmp;
  
  return *this;
}

NGMLogger& NGMLogger::operator<<(long in)
{
  LTRACE<<"long: "<<in<<endl; // debug
  char tmp[100];
  if(_hex) {
    sprintf(tmp,"%lx",in);
  }else{
    sprintf(tmp,"%li",in);
  }
  _queue+=tmp;
  return *this;
}

NGMLogger& NGMLogger::operator<<(unsigned long long in)
{
    LTRACE<<"unsigned long long: "<<in<<endl; // debug
    char tmp[100];
    if(_hex) {
        sprintf(tmp,"%llx",in);
    }else{
        sprintf(tmp,"%llu",in);
    }
    _queue+=tmp;
    return *this;
}

NGMLogger& NGMLogger::operator<<(long long in)
{
  LTRACE<<"long long: "<<in<<endl; // debug
  char tmp[100];
  if(_hex) {
    sprintf(tmp,"%llx",in);
  }else{
    sprintf(tmp,"%lli",in);
  }
  _queue+=tmp;
  return *this;
}

NGMLogger& NGMLogger::operator<<(double in)
{
  LTRACE<<"double: "<<in<<endl; // debug
  /*
  char tmp[100];
  sprintf(tmp,"%f",in);
  _queue+=tmp;
  */
  _queue += in;
  return *this;
}

#define NGMLoggerError(s) (cerr<<"ERROR: NGMLogger internal error: " \
						       << s <<LOGLOC<<endl)


NGMLogger& NGMLogger::operator<<(const wchar_t *op)
{
  // hack for CINT

  //LTRACE<<"wchar_t:"<<*op<<endl; // debug

#undef  LOGLOC
#define LOGLOC " [interpreted]"

  if      (*op == 'B') { LOG<<__ENDM_BASE__  ; }
  else if (*op == 'I') { LOG<<  ENDM_INFO    ; }
  else if (*op == 'W') { LOG<<  ENDM_WARN    ; }
  else if (*op == 'F') { LOG<<  ENDM_FATAL   ; }
  else if (*op == 'R') { LOG<<  ENDM_RESET   ; }
  else if (*op == 'L') { LOG<<  BEGM_LOGROOT ; }
  else if (*op == 'E') { LOG<<  ENDM_LOGROOT ; }
  else if (*op == 'O') { LOG<<  ENDM_ROOTOUT ; }

  else if (*op == 'x') { LOG<<  hex  ; }
  else if (*op == 'd') { LOG<<  dec  ; }
  else if (*op == 'c') { LOG<<  color; }
  else if (*op == 'b') { LOG<<  bandw; }
  
  else if (*op == 'f') { LOG<<  flush; }
  //  else if (*op == '') { LOG<<  ; }

#undef  LOGLOC
#define LOGLOC LOGLOC_BASE
  
  else NGMLoggerError("unrecognized wchar_t opcode");

  return *this;
}

//NGMLogger& NGMLogger::level     (levelE lev)  { return (*this)<<lev; }
NGMLogger& NGMLogger::operator<<(levelE lev)
{
  LTRACE<<"setting level:"<<lev<<endl; // debug
  _level = lev;
  return *this;
}


//NGMLogger& NGMLogger::control   (controlE test)  { return (*this)<<test; }
NGMLogger& NGMLogger::operator<<(controlE test)
{
  LTRACE<<"received control:"<<(int)test<<endl; // debug
  switch(test)
    {
    case hex:
      _hex=true;
      break;
	  
    case dec:
      _hex=false;
      break;
	  
    case color:
      _color=true;
      break;
	  
    case bandw:
      _color=false;
      break;
	  
    case endm:
	  LTRACE<<"received endm"<<endl;
	  Label_queue();
      if(_outputSelector==stdError){
		cerr    <<_queue<<endl;
      }else if(_outputSelector==diskFile){
		(*_file)<<_queue<<endl;	
      }else{
		NGMLoggerError("Unrecognized output selector");
      }
	  
      _queue = "";
      break;

    case flush:
	  LTRACE<<"received flush"<<endl;
      if(_outputSelector==stdError){
		TString savedQ(_queue);
		LOG<<info;
		Label_queue();
		cerr<<_queue<<"\r";
		_queue = savedQ;
      }
      break;

	case reset:
	  LTRACE<<"received reset"<<endl;
      _queue = "";
      break;

	case logroot:
//	  // first tell root to use our error handler for everything
//	  LTRACE<<"received logroot"<<endl;
//	  _oldgPrintViaErrorHandler = gPrintViaErrorHandler;
//	  gPrintViaErrorHandler     = true;
//	  _oldgErrorHandler         = SetErrorHandler(RootErrorHandler);
//
//	  // now, divert cout, used by, e.g., TFitterMinuit
//	  _cout = std::cout.rdbuf();
//	  std::cout.rdbuf(&_sout);
//
//	  // and start with a clean slate
//	  _rootQ = "";
	  break;

	case endroot:
//	  // tell root to go back to the default error handler and printer
//	  LTRACE<<"received endroot"<<endl;
//	  gPrintViaErrorHandler     = _oldgPrintViaErrorHandler;
//	  _oldgErrorHandler         = SetErrorHandler(_oldgErrorHandler);
//
//	  // send cout back where it was
//	  _cout = std::cout.rdbuf(_cout);
//	  if ( _cout != &_sout) {
//		NGMLoggerError("Restored cout, unexpected result");
//	  }
//	  _cout = 0;
//
//	  // and save to _rootQ
//	  _rootQ += _sout.str();
	  break;

	case rootout:
	  LTRACE<<"received rootout"<<endl;
	  // usually need to strip trailing endl
	  _rootQ.Remove(TString::kTrailing,'\n');
	  _queue += _rootQ;
	  _rootQ = "";
	  break;

    default:
	  NGMLoggerError("Unknown control operator");

    }
  return *this;
}

//______________________________________________________________________________
void
NGMLogger::Label_queue()
{
  // prepend severity label, and colorize

  const char *BLACK = "\x1b[30m";
  const char *RED   = "\x1b[31m";
  const char *GREEN = "\x1b[32m";
  TString label;

  switch (_level) {
  case info   :  label += "INFO:: ";            break;
  case warning:  label += "ERROR{warning}:: ";  break;
  case fatal:    label += "ERROR{fatal}:: ";    break;
  default:       NGMLoggerError("Unknown label");
  }

  if (_color) {
	switch (_level) {
	case info:     label.Prepend(GREEN);  label += BLACK;  break;
	case warning:  label.Prepend(RED);    label += BLACK;  break;
	case fatal:    label.Prepend(RED);                     break;
	default:       NGMLoggerError("Unknown label");
	}
  }

  _queue.Prepend(label);

  if (_level == fatal && _color) {
	// need to turn back to black at end of line
	if (_queue.First("\n") != kNPOS) {
	  _queue.Insert(_queue.First("\n"), BLACK);
	} else {
	  _queue += BLACK;
	}
	// only colorize label on extra lines
	label += BLACK;
  }

  
  TString rnl("\r\n");   TString labrnl(rnl);   labrnl += label;
  TString r  ("\r");     TString labr  (r);     labr   += label;
  TString nl ("\n");     TString labnl (nl);    labnl  += label;
  _queue.ReplaceAll(rnl, labrnl);
  _queue.ReplaceAll(r,   labr);
  _queue.ReplaceAll(nl,  labnl);
}

//______________________________________________________________________________
void
NGMLogger::RootErrorHandler(Int_t level, Bool_t abort_bool,
							const char *location, const char *msg)
{
  // Capture root output.

  // Based on DefaultErrorHandler in TString.cxx

  // Note this is declared static, so we don't have
  // a 'this' pointer to the logger, nor can we see
  // any member variables.  The LOG macro gets us
  // the logger instance.  LOG.RootQ() gets us the
  // the root message queue.

  if (gErrorIgnoreLevel == kUnset) {
	R__LOCKGUARD2(gGlobalMutex);
	
	gErrorIgnoreLevel = 0;
	if (gEnv) {
	  TString slevel = gEnv->GetValue("Root.ErrorIgnoreLevel", "Print");
	  if (!slevel.CompareTo("Print", TString::kIgnoreCase))
		gErrorIgnoreLevel = kPrint;
	  else if (!slevel.CompareTo("Info", TString::kIgnoreCase))
		gErrorIgnoreLevel = kInfo;
	  else if (!slevel.CompareTo("Warning", TString::kIgnoreCase))
		gErrorIgnoreLevel = kWarning;
	  else if (!slevel.CompareTo("Error", TString::kIgnoreCase))
		gErrorIgnoreLevel = kError;
	  else if (!slevel.CompareTo("Break", TString::kIgnoreCase))
		gErrorIgnoreLevel = kBreak;
	  else if (!slevel.CompareTo("SysError", TString::kIgnoreCase))
		gErrorIgnoreLevel = kSysError;
	  else if (!slevel.CompareTo("Fatal", TString::kIgnoreCase))
		gErrorIgnoreLevel = kFatal;
	}
  }
  
  if (level < gErrorIgnoreLevel)
	return;
  
  const char *type = 0;
  
  if (level >= kInfo)
	type = "Info";
  if (level >= kWarning)
	type = "Warning";
  if (level >= kError)
	type = "Error";
  if (level >= kBreak)
	type = "\n *** Break ***";
  if (level >= kSysError)
	type = "SysError";
  if (level >= kFatal)
	type = "Fatal";
  
  if (level >= kPrint && level < kInfo) {
	// DebugPrint("%s\n", msg);
	LOG.RootQ() += TString::Format("%s\n", msg);
  } else if (level >= kBreak && level < kSysError) {
	// DebugPrint("%s %s\n", type, msg);
	LOG.RootQ() += TString::Format("%s %s\n", type, msg);
  } else if (!location || strlen(location) == 0) {
	// DebugPrint("%s: %s\n", type, msg);
	LOG.RootQ() += TString::Format("%s: %s\n", type, msg);
  } else {
	// DebugPrint("%s in <%s>: %s\n", type, location, msg);
	LOG.RootQ() += TString::Format("%s in <%s>: %s\n", type, location, msg);
  }
  
  // fflush(stderr);   
  if (abort_bool) {
	
	// DebugPrint("aborting\n");
	LOG.RootQ() += "aborting\n";
	// fflush(stderr);
	LOG<<ENDM_ROOTOUT<<ENDM_FATAL;
	if (gSystem) {
	  gSystem->StackTrace();
	  gSystem->Abort();
	} else
	  abort();
  }
}


//______________________________________________________________________________
void
NGMLogger::RegisterModule(const char *module, const int defaultLevel)
{
  // Register module, with default tracing level
  LTRACE<<"mod: "<<module<<", lev "<<defaultLevel<<endl;
  _trace[module] = defaultLevel;

}

//______________________________________________________________________________
int
NGMLogger::SetLevel(const char *module, const int newLevel)
{
  // Change the default tracing level, returns old level
  LTRACE<<"mod: "<<module<<", lev "<<newLevel<<endl;

  int oldLevel = _trace[module];
  RegisterModule(module, newLevel);
  return oldLevel;
}

//______________________________________________________________________________
bool
NGMLogger::Tracing(const char *module, const int level, const int deflevel)
{
  // Are we tracing module at level?
  LTRACE<<"mod: "<<module<<", lev "<<level<<endl;

  bool t = false;
  
  if ( _trace.find(module) == _trace.end() ) {
	RegisterModule(module, deflevel);
  }
  if ( _trace[module] >= level ) {
	t = true;
  }

  return t;
}

//______________________________________________________________________________
void
NGMLogger::PrintTracingLevels()
{
  // Print the tracing level for each registered module
  LTRACE<<endl;

  LOG<<LTRACE_MODULE<<" Registerd Modules:\t"<<ENDM_INFO;
  LOG<<"Level\tModule\t\t"<<ENDM_INFO;
  
  std::map<const char *, int>::iterator it;
  for ( it =  _trace.begin();
		it != _trace.end();
		it++)
	{
	  LOG<<it->second<<"\t"<<it->first<<"\t\t"<<ENDM_INFO;
	}
}
