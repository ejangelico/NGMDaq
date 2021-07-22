//
//  NGMBlockPicoDST.h
//  Project
//
//  Created by Newby, Robert Jason on 6/19/13.
//
//

#ifndef __NGMBlockPicoDST__
#define __NGMBlockPicoDST__

#include "TObjString.h"
#include "NGMHitIO.h"
#include <fstream>
#include "NGMHit.h"

class NGMBlockPicoDST : public NGMHitIO
{
public:
    NGMBlockPicoDST();
    NGMBlockPicoDST(const char* name, const char* title);
    virtual ~NGMBlockPicoDST();
    virtual int openOutputFile(const char* outputfilepath); // *MENU*
    virtual int closeOutputFile(); // *MENU*
    virtual bool process(const TObject &);
    virtual void setBasePath(const char* basepath);
    virtual void setBasePathVariable(const char* basepathvar);
    virtual void setTof1(int gate, double scale);
    virtual void setTof2(int gate, double scale);
    virtual void doRaw(bool doRaw = true) {_doRaw = doRaw;}
private:
    void writeRaw(const NGMHit* hit);
    
    TFile* _outputFile;  //! outputFile object
    TTree* _outputBuffer; //!inputStream within inputFile object
    TString _basepath;
    TString _basepathvar;
    int _tof1Gate;
    int _tof2Gate;
    double _tof1ScaleFactor;
    double _tof2ScaleFactor;
    bool _doRaw;
    NGMTimeStamp _runBegin;
    std::ofstream _binaryout;

public:
    ClassDef(NGMBlockPicoDST,3)
};



#endif /* defined(__NGMBlockPicoDST__) */
