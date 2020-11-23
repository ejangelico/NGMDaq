#ifndef __NGMHITIO_H__
#define __NGMHITIO_H__

#include <iostream>

#include "NGMModule.h"
#include "TStopwatch.h"
#include "TArrayS.h"
#include "TMatrixD.h"

class TFile;
class TTree;
class NGMHit;
class TList;
class NGMSystemConfiguration;
class TObjArray;

/*! \class NGMHitIO NGMHitIO "NGMHitIO.h"
 *  \brief IO Interface for the sorted hit data from a 
 *
 *  This class is responsible for implementing connections between
 *  the NGMSystem and the output file.  Or reading in a file to the rest of
 *  the analysis chain.
 */

class NGMHitIO : public NGMModule
{
public:
  NGMHitIO();
  NGMHitIO(const char* name, const char* title);
  virtual ~NGMHitIO();
  virtual int pushHit(NGMHit*);
  virtual int openInputFile(const char* fileName); // *MENU*
  virtual int closeInputFile(); // *MENU*
  virtual int openInputSocket(const char*){return 0;}
  virtual int readBuffer(Long64_t eventToRead);
  virtual int readNextBuffer(); // *MENU*
  virtual int readNextSpill(); // *MENU*
  virtual int run();// *MENU*
  virtual void setVerbosity(int newVal){ _verbosity = newVal; }
  virtual int getVerbosity(){ return _verbosity; }
  virtual bool init(){return true;}
  virtual bool process(const TObject &);
  virtual bool finish(){return true;}
  
private:    
  TFile* _inputFile; //! inputFile object
  TTree* _inputBuffer;//! inputStream within inputFile object
  Long64_t _currEventNumber; //! index of current event
  Int_t _verbosity;
  
protected:
  NGMHit* _localBuffer; //! local buffer used for IO
  TStopwatch _timer;
public:  
  ClassDef(NGMHitIO,1)
};

class NGMHitOutputFile : public NGMHitIO
{
public:
  NGMHitOutputFile();
  NGMHitOutputFile(const char* name, const char* title);
  virtual ~NGMHitOutputFile();
  virtual int openOutputFile(const char* outputfilepath); // *MENU*
  virtual int closeOutputFile(); // *MENU*
  virtual bool process(const TObject &);
  virtual void setBasePath(const char* basepath);
  virtual void setBasePathVariable(const char* basepathvar);
private:
  TFile* _outputFile;  //! outputFile object
  TTree* _outputBuffer; //!inputStream within inputFile object
  TString _basepath;
  TString _basepathvar;
public:
  ClassDef(NGMHitOutputFile,1)
};

class NGMHitSerialize : public NGMModule
{
public:
  NGMHitSerialize();
  NGMHitSerialize(const char* name, const char* title);
  virtual ~NGMHitSerialize();
    virtual void setVerbosity(int newVal){ _verbosity = newVal; }
    virtual int getVerbosity(){ return _verbosity; }
    virtual bool init(){return true;}
    virtual bool process(const TObject &);
    virtual bool finish(){return true;}
    
private:    
      Int_t _verbosity;
      
protected:
      TStopwatch _timer;
public:  
        ClassDef(NGMHitSerialize,1)
};

class NGMHitRandomIO : public NGMModule
{
public:
  NGMHitRandomIO();
  NGMHitRandomIO(const char* name, const char* title);
  virtual ~NGMHitRandomIO();
  virtual int openInputFile(const char* fileName); // *MENU*
  virtual int closeInputFile(); // *MENU*
  virtual int readBuffer(Long64_t eventToRead, Long64_t eventlength);
  virtual Long64_t readBuffer(Long64_t eventToRead, double timeWindowNanoSeconds);
  virtual Long64_t getMaxEvents() const;
    virtual void setVerbosity(int newVal){ _verbosity = newVal; }
    virtual int getVerbosity(){ return _verbosity; }
    virtual void setHitWindowCounts(int newVal) { _countwidth = newVal; }
    virtual int findNextTrigger(int nevents, double timeInNanoSec);
    virtual bool init(){return true;}
    virtual bool process(const TObject &);
    virtual bool finish(){return true;}
    virtual TList* getHitList() {return _hitList;}
    
private:    
      TFile* _inputFile; //! inputFile object
    TTree* _inputBuffer;//! inputStream within inputFile object
      Long64_t _currEventNumber; //! index of current event
      Int_t _verbosity;
      Long64_t _countwidth;
      
protected:
        NGMHit* _localBuffer; //! local buffer used for IO
      TStopwatch _timer;
      TList* _hitList;
public:  
        ClassDef(NGMHitRandomIO,1)
};

/*! \class NGMHitIO NGMHitIO "NGMHitIO.h"
 *  \brief IO Interface for the sorted hit data from a 
 *
 *  This class is responsible for implementing connections between
 *  the NGMSystem and the output file.  Or reading in a file to the rest of
 *  the analysis chain.
 */

class NGMHitFilter : public NGMModule
{
public:
  NGMHitFilter();
  NGMHitFilter(const char* name, const char* title);
  virtual ~NGMHitFilter();
  virtual bool init();
  virtual bool process(const TObject &);
  virtual bool finish();
  virtual void SetAccept(int ichan, bool accept);
  virtual bool AnaHit(const NGMHit* hit);
  virtual bool ProcessConfiguration(const NGMSystemConfiguration* conf);
    // nchan specifies how many channels to print out; default is all (more than exist).
    virtual void print(int nchan = -1, std::ostream &os = std::cerr) const;
    // because python doesn't know from std::ostream
    virtual void printdef(int nchan = -1) const {print(nchan);}
  
  TArrayI _pileup;
  TArrayF _baselinelow;
  TArrayF _baselinehigh;
  TArrayF _energylow;
  TArrayF _energyhigh;
  TArrayF _neutroncutlow;
  TArrayF _neutroncuthigh;
  TArrayF _gammacutlow;
  TArrayF _gammacuthigh;
  TArrayF _padclow;
  TArrayF _padchigh;
  TArrayF _baselinecut;

private:    
  TArrayS _accept;
  TMatrixD _gatelength;
  TObjArray* _blockPMTBaselines; //TObjArray NGMBaseline Objects
    void _print_TArrayF(std::ostream &os, const TArrayF &arr,
                        int nchan, const char *name) const;

  
protected:
  
public:  
  int _psdScheme;
  ClassDef(NGMHitFilter,6)
};

#endif //__NGMHITIO_H__
