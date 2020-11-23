//
//  NGMBlockPicoDST.cpp
//  Project
//
//  Created by Newby, Robert Jason on 6/19/13.
//
//

#include "NGMBlockPicoDST.h"
#include "NGMSimpleParticleIdent.h"
#include "TFile.h"
#include "TTree.h"
#include "NGMLogger.h"
#include "NGMHit.h"
#include "TThread.h"
#include "TClass.h"
#include "NGMSystemConfiguration.h"

////////////////////////////////////////////////////////////
//////////// NGMBlockPicoDST Implementation ///////////////
////////////////////////////////////////////////////////////

ClassImp(NGMBlockPicoDST)

NGMBlockPicoDST::NGMBlockPicoDST(){
    _outputFile = 0;
    _outputBuffer = 0;
    _basepath = "";
    _basepathvar = "";
    _tof1Gate = -1;
    _tof1ScaleFactor = 1.0;
    _tof2Gate = -1;
    _tof2ScaleFactor = 1.0;
    _doRaw = false;
}

NGMBlockPicoDST::NGMBlockPicoDST(const char* name, const char* title)
: NGMHitIO(name, title)
{
    _outputFile = 0;
    _outputBuffer = 0;
    _basepath = "";
    _basepathvar = "";
    partID = new NGMSimpleParticleIdent;
    _tof1Gate = -1;
    _tof1ScaleFactor = 1.0;
    _tof2Gate = -1;
    _tof2ScaleFactor = 1.0;
    _doRaw = false;
}

NGMBlockPicoDST::~NGMBlockPicoDST(){
    //When closing the output file
    // the associated output buffer is automatically deleted
    if(_outputFile){
        _outputFile->Close();
        delete _outputFile;
        _outputFile = 0;
    }
    _outputBuffer = 0;
    _binaryout.close();
}

void NGMBlockPicoDST::setTof1(int gate, double scale)
{
    _tof1Gate = gate;
    _tof1ScaleFactor = scale;
}

void NGMBlockPicoDST::setTof2(int gate, double scale)
{
    _tof2Gate = gate;
    _tof2ScaleFactor = scale;
}

void NGMBlockPicoDST::setBasePath(const char* basepath)
{
    _basepath = basepath;
}

void NGMBlockPicoDST::setBasePathVariable(const char* basepathvar)
{
    _basepathvar = basepathvar;
    
}


int NGMBlockPicoDST::openOutputFile(const char* outputfilepath){
    if(_outputFile){
        LOG<<"Closing previous file "<<_outputFile->GetName()<<" and opening new file "<<outputfilepath<<ENDM_INFO;
        closeOutputFile();
    }
    
    //TODO: Add lots of error checking here
    TThread::Lock();
    _outputFile = TFile::Open(outputfilepath,"RECREATE");
    if(!_localBuffer){
        // Would like to generate this by inspecting the data type
        // of the output buffer...
        _localBuffer = new NGMHitv7;
    }
    if(!_outputBuffer){
        _outputBuffer = new TTree("HitTree","HitTree");
        _outputBuffer->SetMaxTreeSize(40000000000LL);
    }
    _outputBuffer->Branch("HitTree","NGMHitv7",&_localBuffer,3200000,99);
    TThread::UnLock();
    return 0;
    
}

int NGMBlockPicoDST::closeOutputFile(){
    TThread::Lock();
    if(_outputBuffer)
    {
        _outputFile = _outputBuffer->GetCurrentFile();
        if(_outputFile)
        {
            LOG<<"Closing output file "<<_outputFile->GetName()<<ENDM_INFO;
            _outputFile->Write();
            _outputFile->Close();
            _outputFile = 0;
            _outputBuffer = 0;
        }
    }
    TThread::UnLock();
    // Do not delete or zero out _localBuffer
    return 0;
}

bool NGMBlockPicoDST::process(const TObject & tData)
{
    
    static TClass* tNGMHitType = TClass::GetClass("NGMHit");
    static TClass* tNGMSystemConfigurationType = TClass::GetClass("NGMSystemConfiguration");
    static TClass* tObjStringType = TClass::GetClass("TObjString");
    
    //Check data type
    if(tData.InheritsFrom(tNGMHitType)){
        
        const NGMHit* hitBuffer = (const NGMHit*)(&tData);
        if(partID->IsSelected(hitBuffer))
        {
            // Copy to our local buffer to be written to the output tree
            if(_localBuffer){
                _localBuffer->CopyHit(hitBuffer);
                if(_tof1Gate>-1)
		  _localBuffer->SetTOF1(hitBuffer->GetGate(hitBuffer->GetGateCount()-_tof1Gate)*_tof1ScaleFactor);
                if(_tof2Gate>-1)
		  _localBuffer->SetTOF2(hitBuffer->GetGate(hitBuffer->GetGateCount()-_tof2Gate)*_tof2ScaleFactor);
		if(_localBuffer->GetPixel()<0) _localBuffer->SetPixel(-partID->getPlotIndex(hitBuffer));
            }
            //Now wrtie to the outputfile
            if(_outputBuffer){
                _outputBuffer->Fill();
            }
            if(_doRaw){
                writeRaw(hitBuffer);
            }
        }
    }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
        const NGMSystemConfiguration* confBuffer = (const NGMSystemConfiguration*)(&tData);
        // Initialize particle identifier
        partID->Init(confBuffer);
        _runBegin = confBuffer->GetTimeStamp();
        TString outputPath = _basepath;
        if(_basepathvar!="")
            if(confBuffer->GetSystemParameters()->GetParIndex(_basepathvar)>=0)
                outputPath = confBuffer->GetSystemParameters()->GetParValueS(_basepathvar,0);
        TString filename = outputPath;
        filename+=GetName();
        filename+= confBuffer->getRunNumber();
        
        if(_doRaw){
            _binaryout.close();
            TString binaryName =filename;
            binaryName += "-He3-ngm.dat";
            _binaryout.open(binaryName.Data(),std::ofstream::out | std::ofstream::binary);
        }
        filename+= "-ngm.root";
        openOutputFile(filename);
        if(_outputFile){
            _outputFile->WriteTObject(confBuffer,"NGMSystemConfiguration");
        }
    }else if(tData.IsA() == tObjStringType){
        const TObjString* controlMessage = (const TObjString*)(&tData);
        
        if(getVerbosity()>10) LOG<<"Sending Message "<<controlMessage->GetString().Data()<<ENDM_INFO;
        
        if(controlMessage->GetString() == "EndRunSave")
        {
            closeOutputFile();
            _binaryout.close();

        }
    }
    
    
    push(tData);
    
    return true;
}

void NGMBlockPicoDST::writeRaw(const NGMHit* hit){
    double timeInRun = hit->TimeDiffNanoSec(_runBegin);
    double timeSincetof1 = hit->GetGate(hit->GetGateCount()-_tof1Gate)*_tof1ScaleFactor;
    int hwchan = partID->getPlotIndex(hit);
    int imagerpixel = hit->GetPixel();
    float energy = hit->GetEnergy();
    float psd = hit->GetPSD();
    float neutronid = hit->GetNeutronId();
    float gammaid = hit->GetGammaId();
    if(hwchan<76) return;
    _binaryout.write(reinterpret_cast<char*>(&timeInRun),sizeof(timeInRun));
    _binaryout.write(reinterpret_cast<char*>(&timeSincetof1),sizeof(timeSincetof1));
    _binaryout.write(reinterpret_cast<char*>(&hwchan),sizeof(hwchan));
    _binaryout.write(reinterpret_cast<char*>(&imagerpixel),sizeof(imagerpixel));
    _binaryout.write(reinterpret_cast<char*>(&energy),sizeof(energy));
    _binaryout.write(reinterpret_cast<char*>(&psd),sizeof(psd));
    _binaryout.write(reinterpret_cast<char*>(&neutronid),sizeof(neutronid));
    _binaryout.write(reinterpret_cast<char*>(&gammaid),sizeof(gammaid));
}

