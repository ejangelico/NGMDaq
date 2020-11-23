#ifndef __NGMPACKETBUFFERIO_H__
#define __NGMPACKETBUFFERIO_H__

#include "NGMModule.h"
#include "TStopwatch.h"

// Forward declarations
class TFile;
class TTree;
class NGMBufferedPacket;
class TBranch;
class TServerSocket;
class TSocket;
class TThread;
class NGMSystemConfiguration;

/*! \class NGMPacketBufferIO NGMPacketBufferIO "NGMPacketBufferIO.h"
 *  \brief IO Interface for the buffered data from a packet
 *
 *  This class is responsible for implementing connections between
 *  the NGMSystem and the output file.  Or reading in a file to the rest of
 *  the analysis chain.
 */

class NGMPacketBufferIO : public NGMModule
{
public:
  NGMPacketBufferIO();
  NGMPacketBufferIO(const char* name, const char* title);
  virtual ~NGMPacketBufferIO();
  virtual int pushPacket(NGMBufferedPacket*);
  virtual int openInputFile(const char* fileName, bool readConfig = true); // *MENU*
  virtual int closeInputFile(); // *MENU*
  virtual bool FindNextFile();
  virtual int openInputSocket(const char*){return 0;}
  virtual int readBuffer(Long64_t eventToRead);
  virtual int readNextBuffer(); // *MENU*
  virtual int run();// *MENU*
  virtual Long64_t readNextSpill(); // *MENU*
  virtual void setVerbosity(int newVal){ _verbosity = newVal; }
  virtual int getVerbosity(){ return _verbosity; }
  virtual bool init(){return true;}
  virtual bool process(const TObject &);
  virtual bool finish(){return true;}
  virtual NGMBufferedPacket* getLocalBuffer(){ return _localBuffer; }
  virtual void setLocalBuffer(NGMBufferedPacket* newVal){ _localBuffer = newVal; }
  virtual void setCompressionLevel(int newVal) { _compressionlevel = newVal; }
  virtual int getCompressionLevel() { return _compressionlevel; }
  virtual void setMaxFileSize(Long64_t newVal) { _maxfilesize = newVal; }
  virtual Long64_t getMaxFileSize() { return _maxfilesize; }
  virtual void setBytesWritten(Long64_t newVal) { _byteswritten = newVal; }
  virtual Long64_t getBytesWritten() { return _byteswritten; }
  virtual void setBytesRead(Long64_t newVal) { _bytesread = newVal; }
  virtual Long64_t getBytesRead() { return _bytesread; }
  virtual void setBasePath(const char* basepath) { _basepath = basepath; }
  virtual void setBasePathVariable(const char* basepathvar) { _basepathvar = basepathvar; }

private:    
  TFile* _inputFile; //! inputFile object
  TTree* _inputBuffer;//! inputStream within inputFile object
  Long64_t _currEventNumber; //! index of current event
  Int_t _verbosity;
                                  
protected:
  Int_t _compressionlevel;
  Long64_t _maxfilesize;
  Long64_t _byteswritten;
  Long64_t _bytesread;
  TString _basepath;
  TString _basepathvar;
  Long64_t _numberOfSlots;
  Long64_t _numberOfHWChannels;
  Long64_t _channelsperSlot;
  Long64_t _prevBufferIndex;
  Long64_t _spillCounter;

  NGMBufferedPacket* _localBuffer; //! local buffer used for IO
  NGMSystemConfiguration* _sysConf; //! local buffer used for IO
  
  TStopwatch _timer;
public:  
  ClassDef(NGMPacketBufferIO,1)
};

class NGMPacketOutputFile : public NGMPacketBufferIO
{
public:
  NGMPacketOutputFile();
  NGMPacketOutputFile(const char* name, const char* title);
  virtual ~NGMPacketOutputFile();
  virtual void setSpillsPerFile(int spillsPerFile = 0) {_spillsPerFile = spillsPerFile;} // *MENU*
  virtual void setSplitRuns(bool splitRuns  = true) {_splitRuns = splitRuns;} // *MENU*
  virtual int openOutputFile(const char* outputfilepath); // *MENU*
  virtual int closeOutputFile(); // *MENU*
  virtual bool process(const TObject &);
  virtual void setSelfSavePeriod(double newVal) {_selfSavePeriod = newVal;}

private:
  TFile* _outputFile;  //! outputFile object
  TTree* _outputBuffer; //!inputStream within inputFile object
  TBranch* _localBranch; //!branch on output tree
  bool _passThrough;
  Long64_t _spillsThisOutputFile;
  Long64_t _spillsPerFile;
  bool _newFileNextSpill;
  Long64_t _fileCounter;
  bool _splitRuns;
  double _timeOfLastSelfSave;
  double _selfSavePeriod;
  
public:
  ClassDef(NGMPacketOutputFile,4)
};

// Simple module to filter out packets based on
// slotid and for SIS channel number of first hit...
class NGMPacketFilter : public NGMModule
{
public:
  NGMPacketFilter();
  NGMPacketFilter(const char* name, const char* title);
  virtual ~NGMPacketFilter();
  virtual bool process(const TObject &);
  virtual bool init(){return true;}
  virtual bool finish(){return true;}
  virtual bool addSkipChannel(int slotid, int channelid);

private:
  enum _localconsts {maxskips = 120};

  int _slotsToSkip[maxskips];
  int _channelsToSkip[maxskips];
  int _numskips;

public:
  ClassDef(NGMPacketFilter,1)
};

class NGMPacketSocketInput : public NGMPacketBufferIO
{
public:
  NGMPacketSocketInput();
  NGMPacketSocketInput(const char* name, const char* title);
  virtual ~NGMPacketSocketInput();
  virtual int openSocket(); // *MENU*
  virtual bool init();
  virtual bool process(const TObject &);
  virtual bool finish();
  virtual bool ReceiveLoop();
  virtual void setExitLoop(bool newVal = true); // *MENU*
  static void* ReceiveLoopThread(void *arg);
  void LaunchReceiveLoopThread(); // *MENU*
  
private:
  TServerSocket *serverSocket;
  TServerSocket *signalServerSocket;
  TSocket* socket;
  TSocket* signalSocket;
  TThread* _receiveThread;
  bool _exitLoop;

public:  
        ClassDef(NGMPacketSocketInput,1)
};

class NGMPacketSocketOutput : public NGMPacketBufferIO
{
public:
  NGMPacketSocketOutput();
  NGMPacketSocketOutput(const char* name, const char* title);
  virtual ~NGMPacketSocketOutput();
  virtual int openSocket(const char* hostname = "localhost"); // *MENU*
  virtual bool init();
  virtual bool process(const TObject &);
  virtual bool finish();
private:
  TSocket* socket;
  TSocket* signalSocket;
public:  
      ClassDef(NGMPacketSocketOutput,1)
};


#endif //__NGMPACKETBUFFERIO_H__
