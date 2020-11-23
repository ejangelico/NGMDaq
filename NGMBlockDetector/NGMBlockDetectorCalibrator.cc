/*
 *  NGMBlockDetectorCalibrator.cpp
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/28/06.
 *  Copyright 2006 LLNL. All rights reserved.
 *
 */

#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "NGMLogger.h"
#include "NGMBufferedPacket.h"
#include "NGMSystemConfiguration.h"
#include "NGMBlockDetectorCalibrator.h"
#include "NGMConfigurationTable.h"
#include "TSpectrum.h"
#include "TROOT.h"
#include "TFile.h"
#include "TFolder.h"
#include "TCanvas.h"
#include "TCutG.h"
#include "TProfile.h"
#include "TSystem.h"
#include "TRandom.h"
#include <string>
#include <fstream>
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "TSQLServer.h"
#include "TMath.h"
#include "NGMSimpleParticleIdent.h"
#include "NGMParticleIdent.h"
#include "NGMSystem.h"
#include <cmath>
#include "sis3316card.h"

ClassImp(NGMBlockDetectorCalibrator)

NGMBlockDetectorCalibrator::NGMBlockDetectorCalibrator()
{
    InitCommon();
}

NGMBlockDetectorCalibrator::NGMBlockDetectorCalibrator(const char* name, const char* title)
: NGMModule(name,title)
{
    InitCommon();
    partID = new NGMSimpleParticleIdent;
    for(int ichan = 0; ichan < maxChannels; ichan++){
        _detNames[ichan] = (TString("")+=ichan);
    }
    _calFileName = "";
    _roiFileName = "ROIs.root";
}

NGMBlockDetectorCalibrator::~NGMBlockDetectorCalibrator(){
  // Definitely need to clean up allocated memory here
  delete partID;
}
void NGMBlockDetectorCalibrator::InitCommon(){
    
    nChannels = 0;
	numberOfHWChannels = 13*8;
	numberOfSlots = 13;
    partID = 0;
    blocknrows = 1;
    blockncols = 1;
    _constantfraction = 0.1;
    _psdScheme = 0;// Default scheme assumes xia gates 70,10,4,8,1,8,1,1
    
    for(int ichan = 0; ichan < maxChannels; ichan++){
        _detType[ichan] = 0;
        _nanosecondsPerSample[ichan] = 5.0;
        _cfdscale[ichan]=-1.0;
        _mainGateLength[ichan] = 1000;
        _mainGatePreTrigger[ichan] = 200;
        _rawsampleStartIndex[ichan] = 0;
        _keVperADC[ichan] = -1.0;
        _timingCal[ichan]= 0.0; //ns
        _neutronGate[ichan] = 0;
        _nMean[ichan] = 0;
        _nSigma[ichan] = 0;
        _gMean[ichan] = 0;
        _gSigma[ichan] = 0;
        _channelenabled[ichan] = 1; // Assume all are enabled
        for(int igate = 0; igate < ngates; igate++)
        {
            _gatewidth[ichan][igate] = 0;
            _gateoffset[ichan][igate] = 0;
        }
        
        
    }
    for(int iblock = 0; iblock < maxChannels/4; iblock++)
    {
        _pixelLUTMinX[iblock] = 0.0;
        _pixelLUTMaxX[iblock] = 1.0;
        _pixelLUTMinY[iblock] = 0.0;
        _pixelLUTMaxY[iblock] = 1.0;
    }
}

bool NGMBlockDetectorCalibrator::init(){

  return true;
}
bool  NGMBlockDetectorCalibrator::SetCalFileName(const char* filename)
{
   _calFileName = filename;
   return true;
}

int NGMBlockDetectorCalibrator::getPlotIndex(const NGMHit* tHit) const
{
  // Now this is handled via the particle ident class
    return partID->getPlotIndex(tHit);
}
bool  NGMBlockDetectorCalibrator::processHit(const NGMHit* tHit)
{
    static TClass* tNGMHitv5Type = gROOT->GetClass("NGMHitv5");
    static TClass* tNGMHitv6Type = gROOT->GetClass("NGMHitv6");
    {
        int hwseq = getPlotIndex(tHit);
        if(!_channelenabled[getPlotIndex(tHit)]) return 0;
        
        NGMHit* calHit = new NGMHitv6;
        calHit->CopyHit(tHit);
        if(_bmap.IsBlock(hwseq)){
            analyzeHitBlock(calHit);
        }else{
            analyzeHitSingle(calHit);
        }
        push(*((const TObject *)calHit));
        delete calHit;
    }
    
    return true;
}
bool NGMBlockDetectorCalibrator::processPacket(const NGMBufferedPacket* packet)
{
    
    if(packet->getSlotId()<0)
    {
        //Assume this a sorted packet of hits and pass it through
        push(*((const TObject*)packet));
        return true;
    }
    const NGMHit* tHit = packet->getHit(0);
    if(!tHit) return 0;
    
    if(!_channelenabled[partID->getPlotIndex(tHit)]) return 0;
    
    //LOG<<"Calibrating Old Packet Slot("<<tHit->GetSlot()
    // <<") Channel("<<tHit->GetChannel()
    // <<ENDM_INFO;
    NGMBufferedPacket* newPacket = new NGMBufferedPacketv2(packet->getSlotId(), packet->getPulseCount(),
                                                           TTimeStamp(packet->getTimeStamp()),6);
    newPacket->CopyPacket(packet);
    analyzePacket(newPacket);
    push(*((const TObject*)newPacket));
    delete newPacket;
    
    return true;
}
bool  NGMBlockDetectorCalibrator::processConfiguration(const NGMSystemConfiguration* confBuffer)
{
    partID->Init(confBuffer);
    _bmap.Init(confBuffer);
    _blockPSD.clear();
    _singleDetectorPSD.clear();
    
    for(int hwchan =0; hwchan<confBuffer->GetChannelParameters()->GetEntries();hwchan++){
        if(_bmap.IsBlock(hwchan)){hwchan+=3; continue;}
        _singleDetectorPSD[hwchan]=NGMHitProcess();
        _singleDetectorPSD[hwchan].Init(confBuffer,hwchan);
    }
    int nchan =confBuffer->GetChannelParameters()->GetEntries();
    _nombaselines.resize(nchan);
    for(int ichan = 0; ichan<nchan; ichan++){
        _nombaselines[ichan] = 0.0;
    }
    if(confBuffer->GetChannelParameters()->GetColumn("NomBaseline")){
        for(int ichan = 0; ichan<nchan; ichan++){
            _nombaselines[ichan] = confBuffer->GetChannelParameters()->GetParValueD("NomBaseline", ichan);
            LOG<<"NomBaseline "<<_nombaselines[ichan]<<ENDM_INFO;
        }
    }
    
    int pIR = _bmap.GetNPixOnASide(0);
    if(confBuffer->GetChannelParameters()->GetColumn("QDCLen0"))
    {
        const NGMConfigurationParameter* nspar = confBuffer->GetSlotParameters()->GetColumn("NanoSecondsPerSample");
        for(int ichan = 0; ichan < confBuffer->GetChannelParameters()->GetEntries(); ichan++)
        {
            double nsPerSample = 4.0; // Should retreive this from a slot parameter
            if(nspar && confBuffer->GetSlotParameters()->GetParValueD("NanoSecondsPerSample",0)==0.0)
                nsPerSample=10.0;
            if (nspar)
            {
                int slot = ichan/16;
                if(nspar->GetValue(slot)>0.0)
                    nsPerSample = nspar->GetValue(slot);
                
            }
            _gatewidth[ichan][0] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen0",ichan)*1000.0/nsPerSample;
            _gatewidth[ichan][1] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen1",ichan)*1000.0/nsPerSample;
            _gatewidth[ichan][2] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen2",ichan)*1000.0/nsPerSample;
            _gatewidth[ichan][3] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen3",ichan)*1000.0/nsPerSample;
            _gatewidth[ichan][4] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen4",ichan)*1000.0/nsPerSample;
            _gatewidth[ichan][5] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen5",ichan)*1000.0/nsPerSample;
            _gatewidth[ichan][6] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen6",ichan)*1000.0/nsPerSample;
            _gatewidth[ichan][7] = confBuffer->GetChannelParameters()->GetParValueD("QDCLen7",ichan)*1000.0/nsPerSample;
            
        }
    }else if(confBuffer->GetSlotParameters()->GetColumn("card")){
        
        for(int islot = 0; islot < confBuffer->GetSlotParameters()->GetEntries(); islot++)
        {
            if(! confBuffer->GetSlotParameters()->GetParValueI("ModEnable",islot)) continue;
            const sis3316card* card = dynamic_cast<const sis3316card*>(confBuffer->GetSlotParameters()->GetParValueO("card",islot));
            for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
            {
                for(int iqdc =0; iqdc<SIS3316_QDC_PER_CHANNEL; iqdc++)
                {
                    _gatewidth[SIS3316_CHANNELS_PER_CARD*islot+ichan][iqdc] = card->qdclength[ichan/SIS3316_CHANNELS_PER_ADCGROUP][iqdc];
                }
            }
        }
        
    }
    
    LOG<<"Using Widths "
    <<_gatewidth[0][0]<<" "
    <<_gatewidth[0][1]<<" "
    <<_gatewidth[0][2]<<" "
    <<_gatewidth[0][3]<<" "
    <<_gatewidth[0][4]<<" "
    <<_gatewidth[0][5]<<" "
    <<_gatewidth[0][6]<<" "
    <<_gatewidth[0][7]<<ENDM_INFO;
    
    // Test if we have timing calibrations availible
    if(confBuffer->GetDetectorParameters()->GetParIndex("CalTimingOffset")<0)
    {
        for(int ichan = 0; ichan < numberOfHWChannels; ichan++)
        {
            _timingCal[ichan]= 0.0; //ns
        }
    }else{
        for(int ichan = 0; ichan < numberOfHWChannels; ichan++)
        {
            TString detName = confBuffer->GetChannelParameters()->GetParValueS("DetectorName",ichan);
            if(detName!="")
            {
                TString sCalTimingOffset = confBuffer->GetDetectorParameters()->LookupParValueAsString("CalTimingOffset",
                                                                                                       "DetectorName",detName.Data());
                _timingCal[ichan]= sCalTimingOffset.Atof(); //ns
                
                if(getDebug())LOG<<" Timing Offset for detector "<<detName.Data()<<" is "
                    <<confBuffer->GetDetectorParameters()->LookupParValueAsString("CalTimingOffset","DetectorName",detName.Data())
                    <<ENDM_INFO;
            }else{
                _timingCal[ichan] = 0.0;
            }
        }
    }
    
    
    //Lets loop through each of the channels groups and determine the Block row, column, and pixelLUT
    blocknrows = confBuffer->GetSystemParameters()->GetParValueI("BLOCKNROWS",0);
    blockncols = confBuffer->GetSystemParameters()->GetParValueI("BLOCKNCOLS",0);
    _blockRowCol.ResizeTo(confBuffer->GetChannelParameters()->GetEntries()/4,2);
    _pixelkeVADC.ResizeTo(blocknrows*pIR,blockncols*pIR);
    _pixelSigmaScaling.ResizeTo(blocknrows*pIR,blockncols*pIR);
    _psdScheme = 0;
    if(confBuffer->GetSystemParameters()->GetParIndex("XIAPSD_SCHEME")>=0)
    {
        _psdScheme = confBuffer->GetSystemParameters()->GetParValueI("XIAPSD_SCHEME",0);;
    }
    if(confBuffer->GetSystemParameters()->GetParIndex("PSD_SCHEME")>=0)
    {
        _psdScheme = confBuffer->GetSystemParameters()->GetParValueI("PSD_SCHEME",0);;
    }
    LOG<<" Using PSDScheme "<<_psdScheme<<ENDM_INFO;
    LOG<<" Using block detector array of "<<blocknrows<<" rows by "<<blockncols<<" columns." << ENDM_INFO;
    for(int ichan = 0; ichan < confBuffer->GetChannelParameters()->GetEntries(); ichan+=4)
    {
        int iblock = ichan/4;
        TString detName(confBuffer->GetChannelParameters()->GetParValueS("DetectorName",ichan));
        if(detName!="")
        {
            int detRow = confBuffer->GetDetectorParameters()->FindFirstRowMatching("DetectorName",detName);
            const TH2* hFlood = 0;
            const TH2* hkeVADC = 0;
            const NGMBlockPSD* blockPSD = 0;
            if(detRow>=0)
            {
                _blockRowCol(iblock,0) = confBuffer->GetDetectorParameters()->GetParValueI("BLOCK_ROW",detRow);
                _blockRowCol(iblock,1) = confBuffer->GetDetectorParameters()->GetParValueI("BLOCK_COL",detRow);
                LOG<<"Assigning detector "<<detName.Data()
                <<" a.k.a. iblock "<<iblock
                <<" to position ("<<_blockRowCol(iblock,0)
                <<","<<_blockRowCol(iblock,1)<<")"<<ENDM_INFO;
                hFlood = dynamic_cast<const TH2*>(confBuffer->GetDetectorParameters()->FindFirstParValueAsObject("FLOOD_LUT","DetectorName",detName.Data()));
                hkeVADC = dynamic_cast<const TH2*>(confBuffer->GetDetectorParameters()->FindFirstParValueAsObject("BlockEnergyCal","DetectorName",detName.Data()));
                blockPSD = dynamic_cast<const NGMBlockPSD*>(confBuffer->GetDetectorParameters()->FindFirstParValueAsObject("BLOCK_PSD","DetectorName",detName.Data()));
            }
            if(hFlood)
            {
                LOG<<"Found "<<hFlood->GetName()<<" for "<<detName.Data()<<ENDM_INFO;
                _pixelLUTMinX[iblock] = hFlood->GetXaxis()->GetXmin();
                _pixelLUTMaxX[iblock] = hFlood->GetXaxis()->GetXmax();
                _pixelLUTMinY[iblock] = hFlood->GetYaxis()->GetXmin();
                _pixelLUTMaxY[iblock] = hFlood->GetYaxis()->GetXmax();
                _pixelLUT[iblock].ResizeTo(hFlood->GetNbinsX(),hFlood->GetNbinsY());
                for(int xbin = 1; xbin <=hFlood->GetNbinsX();xbin++) {
                    for(int ybin = 1; ybin <=hFlood->GetNbinsY();ybin++) {
                        _pixelLUT[iblock](ybin-1,xbin-1) = hFlood->GetBinContent(xbin,ybin);
                    }
                }
                LOG<<" Found flood mapping "<< hFlood->GetName()<<" for detector "<<detName.Data()
                <<" with (xmin,xmax,ymin,ymax) = (" << _pixelLUTMinX[iblock] << ","
                << _pixelLUTMaxX[iblock] << "," << _pixelLUTMinY[iblock] << ","
                << _pixelLUTMaxY[iblock] << ")" << ENDM_INFO;
            }
            if(blockPSD)
            {
                LOG<<"Found "<<blockPSD->GetName()<<" for "<<detName.Data()<<ENDM_INFO;
                _blockPSD[_bmap.GetBlockSequence(ichan)]=*blockPSD;
                LOG<<" Found BlockPSD "<< blockPSD->GetName()<<" for detector "<<detName.Data()
                <<" block seq "<<_bmap.GetBlockSequence(ichan)<< ENDM_INFO;
            }
            if(hkeVADC)
            {
                LOG<<"Found "<<hkeVADC->GetName()<<" for "<<detName.Data()<<ENDM_INFO;
                // NGMBlockMap routines should replace the code below JN
                int blockRow = _blockRowCol(iblock,0);
                int blockCol = _blockRowCol(iblock,1);
                double centerPixelAverage = 0.0;
                double centerPixelAverageCnt=0.0;
                for(int blockPix = 0; blockPix<pIR*pIR; blockPix++)
                {
                    int lPixelCol = blockPix%pIR;
                    int lPixelRow = blockPix/pIR;
                    if(1<lPixelCol&&lPixelCol<pIR-2&&1<lPixelRow&&lPixelRow<pIR-2)
                    {
                        centerPixelAverage+=hkeVADC->GetBinContent(blockPix%pIR+1,blockPix/pIR+1);
                        centerPixelAverageCnt+=1.0;
                    }
                }
                centerPixelAverage = centerPixelAverage/centerPixelAverageCnt;
                
                for(int blockPix = 0; blockPix<pIR*pIR; blockPix++)
                {
                    int globalPixCol = blockCol*pIR+blockPix%pIR;
                    int globalPixRow = blockRow*pIR+blockPix/pIR;
                    _pixelkeVADC(globalPixRow,globalPixCol) = hkeVADC->GetBinContent(blockPix%pIR+1,blockPix/pIR+1);
                    //Scaling will be used to scale PSD widths from block average to larger-width, lower gain pixels.
                    _pixelSigmaScaling(globalPixRow,globalPixCol) = sqrt(_pixelkeVADC(globalPixRow,globalPixCol)/centerPixelAverage);
                }
            }
        }
    }

    
    if(numberOfHWChannels%numberOfSlots!=0){
        LOG<<"Number of Slots = "<<numberOfSlots<<" Number of Channels = "<<numberOfHWChannels<<". Not even multiple."<<ENDM_FATAL;
        return false;
    }
    
    push(*((const TObject*)confBuffer));
    return true;
}

bool  NGMBlockDetectorCalibrator::processMessage(const TObjString* mess)
{
    push(*((const TObject*)mess));
    return true;
}


bool NGMBlockDetectorCalibrator::process(const TObject &tData){
  
  static TClass* tNGMBufferedPacketType = gROOT->GetClass("NGMBufferedPacket");
  static TClass* tNGMSystemConfigurationType = gROOT->GetClass("NGMSystemConfiguration");
  static TClass* tNGMHitType = gROOT->GetClass("NGMHit");
  static TClass* tObjStringType = gROOT->GetClass("TObjString");
  //Check data type
  if(tData.InheritsFrom(tNGMHitType)){
      return processHit((const NGMHit*)(&tData));
  }else if(tData.InheritsFrom(tNGMBufferedPacketType)){
      const NGMBufferedPacket* packetBuffer = (const NGMBufferedPacket*)(&tData);
      return processPacket(packetBuffer);
  }else if(tData.InheritsFrom(tNGMSystemConfigurationType)){
    const NGMSystemConfiguration* confBuffer = dynamic_cast<const NGMSystemConfiguration*>(&tData);
    return processConfiguration(confBuffer);
  }if(tData.InheritsFrom(tObjStringType)){
      return processMessage((const TObjString*)(&tData));
  }else{// Test if NGMConfiguration
    push(tData);
  }
   
  return true;  
}

bool NGMBlockDetectorCalibrator::SetROIFileName(const char* filename)
{
  _roiFileName = filename;
  return true;
}

bool NGMBlockDetectorCalibrator::finish(){
  
  return true;
}

bool NGMBlockDetectorCalibrator::analyzePacket(NGMBufferedPacket* packet){
  if(!packet) return false;
  int nHits = packet->getPulseCount();
  for(int ihit = 0; ihit < nHits; ihit++){
    analyzeHitBlock(packet->modHit(ihit));
  }  
  return true;
}

bool  NGMBlockDetectorCalibrator::analyzeHitSingle(NGMHit* tHit){
    int hwchan = partID->getPlotIndex(tHit);
    _singleDetectorPSD[hwchan].ProcessHit(tHit);
    return true;
}

bool NGMBlockDetectorCalibrator::analyzeHitBlock(NGMHit* tHit){
  
  int plotIndex = getPlotIndex(tHit);
  // Is something sneaking through?
  if(plotIndex<0 || plotIndex>60) return false;

  int blockSeq = _bmap.GetBlockSequence(plotIndex);
  int pIR = _bmap.GetNPixOnASide(blockSeq);
  int iblock = plotIndex/4;
  if(plotIndex < 0 || plotIndex >= maxChannels){
    LOG<<"plotIndex out of range "<<plotIndex<<ENDM_WARN;
    return false;
  }
    
    
  double SumIntegral = 0.0;
  double SumBaseline = 0.0;
    
    // These should be read from the configuration object
    double g1w = _gatewidth[plotIndex][0];
//    double g2w = _gatewidth[plotIndex][1];
    double g3w = _gatewidth[plotIndex][2];
//    double g4w = _gatewidth[plotIndex][3];
    double g5w = _gatewidth[plotIndex][4];
//    double g6w = _gatewidth[plotIndex][5];
//    double g7w = _gatewidth[plotIndex][6];
//    double g8w = _gatewidth[plotIndex][7];
    double pmt0,pmt1,pmt2,pmt3;
    double ppmt0,ppmt1,ppmt2,ppmt3;
    double bl0,bl1,bl2,bl3;
    
    if(_psdScheme==1){
      //Time calibration has been moved to Pixie16RawReaderv4
        double cfd = tHit->GetCFD();
      //        NGMTimeStamp ts = tHit->GetRawTime();
      //        ts.IncrementNs(10.0*cfd-_timingCal[plotIndex]);
      //        tHit->SetCalibratedTime(ts);
      
        SumBaseline = tHit->GetQuadGate(0)/g1w;
        // double SumIntegral = ComputeGateSum(1,tHit) + ComputeGateSum(2,tHit) + ComputeGateSum(3,tHit) + ComputeGateSum(4,tHit)
        //                    - SumBaseline*(g2w+g3w+g4w+g5w);
        if(_psdScheme==1) SumIntegral = (tHit->GetQuadGate(6)*(1.0-cfd)+tHit->GetQuadGate(7)*cfd)  - SumBaseline;
        else SumIntegral = tHit->GetQuadGate(2) - SumBaseline*g3w;
        
        pmt0 = (tHit->GetGate(6)*(1.0-cfd)+tHit->GetGate(7)*cfd) - tHit->GetGate(0)/g1w;
        pmt1 = (tHit->GetGate(14)*(1.0-cfd)+tHit->GetGate(15)*cfd) - tHit->GetGate(8)/g1w;
        pmt2 = (tHit->GetGate(22)*(1.0-cfd)+tHit->GetGate(23)*cfd) - tHit->GetGate(16)/g1w;
        pmt3 = (tHit->GetGate(30)*(1.0-cfd)+tHit->GetGate(31)*cfd) - tHit->GetGate(24)/g1w;
        ppmt0 = (tHit->GetGate(3)*(1.0-cfd)+tHit->GetGate(4)*cfd) - tHit->GetGate(0)/g1w;
        ppmt1 = (tHit->GetGate(11)*(1.0-cfd)+tHit->GetGate(12)*cfd) - tHit->GetGate(8)/g1w;
        ppmt2 = (tHit->GetGate(19)*(1.0-cfd)+tHit->GetGate(20)*cfd) - tHit->GetGate(16)/g1w;
        ppmt3 = (tHit->GetGate(27)*(1.0-cfd)+tHit->GetGate(28)*cfd) - tHit->GetGate(24)/g1w;
        // double pmt3 = tHit->GetGate(25)+tHit->GetGate(26)+tHit->GetGate(27)+tHit->GateGate(28) - tHit->GetGate(24)/g1w*(g2w+g3w+g4w+g5w);
        double psdAll = (ppmt0+ppmt1+ppmt2+ppmt3)/(pmt0+pmt1+pmt2+pmt3);
        double psdResidual =sqrt((pow(psdAll-ppmt0/pmt0,2.0)*pmt0
                                 + pow(psdAll-ppmt1/pmt1,2.0)*pmt1
                                 + pow(psdAll-ppmt2/pmt2,2.0)*pmt2
                                 + pow(psdAll-ppmt3/pmt3,2.0)*pmt3)/(pmt0+pmt1+pmt2+pmt3));
        
	tHit->SetPSD((ppmt0+ppmt1+ppmt2+ppmt3)/(pmt0+pmt1+pmt2+pmt3));
        tHit->SetPileUpCounter(1000.0*psdResidual);
        tHit->SetPulseHeight(SumIntegral);
        tHit->SetEnergy(SumIntegral);
        
        if(SumIntegral>0) tHit->SetBlockX((pmt2+pmt3)/(SumIntegral));
        if(SumIntegral>0) tHit->SetBlockY((pmt0+pmt2)/(SumIntegral));
    }else if(_psdScheme==2)
    {
        //Time calibration should be done here
        double cfd = tHit->GetCFD();
        //NGMTimeStamp ts = tHit->GetRawTime();
        //ts.IncrementNs(4.0*cfd-_timingCal[plotIndex]);
        //tHit->SetCalibratedTime(ts);
        
        SumBaseline = tHit->GetQuadGate(0)/g1w;
        // double SumIntegral = ComputeGateSum(1,tHit) + ComputeGateSum(2,tHit) + ComputeGateSum(3,tHit) + ComputeGateSum(4,tHit)
        //                    - SumBaseline*(g2w+g3w+g4w+g5w);
        SumIntegral = (tHit->GetQuadGate(3)*(1.0-cfd)+tHit->GetQuadGate(4)*cfd)  - SumBaseline;
        
        bl0 = tHit->GetGate(0)/g1w;
        bl1 = tHit->GetGate(5)/g1w;
        bl2 = tHit->GetGate(10)/g1w;
        bl3 = tHit->GetGate(15)/g1w;
        
        pmt0 = (tHit->GetGate(3)*(1.0-cfd)+tHit->GetGate(4)*cfd) - tHit->GetGate(0)/g1w;
        pmt1 = (tHit->GetGate(3+5)*(1.0-cfd)+tHit->GetGate(4+5)*cfd) - tHit->GetGate(5)/g1w;
        pmt2 = (tHit->GetGate(3+10)*(1.0-cfd)+tHit->GetGate(4+10)*cfd) - tHit->GetGate(10)/g1w;
        pmt3 = (tHit->GetGate(3+15)*(1.0-cfd)+tHit->GetGate(4+15)*cfd) - tHit->GetGate(15)/g1w;

        ppmt0 = (tHit->GetGate(1)*(1.0-cfd)+tHit->GetGate(2)*cfd) - tHit->GetGate(0)/g1w;
        ppmt1 = (tHit->GetGate(1+5)*(1.0-cfd)+tHit->GetGate(2+5)*cfd) - tHit->GetGate(5)/g1w;
        ppmt2 = (tHit->GetGate(1+10)*(1.0-cfd)+tHit->GetGate(2+10)*cfd) - tHit->GetGate(10)/g1w;
        ppmt3 = (tHit->GetGate(1+15)*(1.0-cfd)+tHit->GetGate(2+15)*cfd) - tHit->GetGate(15)/g1w;
        
        tHit->SetPulseHeight(SumIntegral);
        tHit->SetEnergy(SumIntegral);
        
        if(SumIntegral>0)
        {
            tHit->SetBlockX((pmt2+pmt3)/(SumIntegral));
            tHit->SetBlockY((pmt0+pmt2)/(SumIntegral));
        }
    }
  int sysrow = 0;
  int syscol = 0;
  int pixel = 0;
    
  if(_pixelLUT[iblock].GetNrows()>0)
  {
    //Using xy we will see if there are lookups
    // This (x,y) to pixel conversion is coupled to the histogram min/max in BlockFlood!!!!!
    int trow = _pixelLUT[iblock].GetNrows()*
      (tHit->GetBlockY()-_pixelLUTMinY[iblock])/
      (_pixelLUTMaxY[iblock]-_pixelLUTMinY[iblock]);
    int tcol = _pixelLUT[iblock].GetNcols()*
      (tHit->GetBlockX()-_pixelLUTMinX[iblock])/
      (_pixelLUTMaxX[iblock]-_pixelLUTMinX[iblock]);
    //    LOG<< trow <<" : ";
    //    LOG<< tcol <<" : ";
    //Lets make sure everything gets mapped to a pixel
    trow = TMath::Min(_pixelLUT[iblock].GetNrows()-1,TMath::Max(trow,0));
    tcol = TMath::Min(_pixelLUT[iblock].GetNcols()-1,TMath::Max(tcol,0));
    int blockpixel = _pixelLUT[iblock](trow,tcol);
    sysrow = _blockRowCol(iblock,0)*pIR + blockpixel/pIR;
    syscol = _blockRowCol(iblock,1)*pIR + blockpixel%pIR;
    pixel = pIR * blockncols * sysrow + syscol;
    
    //    LOG<< trow <<" : ";
    //    LOG<< tcol <<" : ";
    //    LOG<< blockpixel <<" : ";
    //    LOG<< sysrow <<" : ";
    //    LOG<< syscol <<" : ";
    //    LOG<< pixel <<" : ";
    //    LOG<<ENDM_INFO;
    
    tHit->SetPixel(pixel);
    if(_psdScheme==1||_psdScheme==2)
      tHit->SetEnergy((SumIntegral)*_pixelkeVADC(sysrow,syscol));      
    else if(_psdScheme==0)
      tHit->SetEnergy((tHit->GetQuadGate(4)-SumBaseline)*_pixelkeVADC(sysrow,syscol));
  }else{
    tHit->SetPixel(-1);
  }

  if(_psdScheme==0){
    tHit->SetPSD((tHit->GetQuadGate(2) - SumBaseline*g3w + gRandom->Uniform()-0.5)/(tHit->GetQuadGate(4)-SumBaseline*g5w)/g3w);
  }else if( (_psdScheme==1 || _psdScheme==2) && tHit->GetPixel()>=0){
      tHit->SetPSD((ppmt0+ppmt1+ppmt2+ppmt3)/(pmt0+pmt1+pmt2+pmt3));
      tHit->SetEnergy(tHit->GetPulseHeight()*_pixelkeVADC(sysrow,syscol));
//      tHit->SetGateSize(8);
//      tHit->SetGate(0,ppmt0/pmt0*1000.0);
//      tHit->SetGate(1,ppmt1/pmt1*1000.0);
//      tHit->SetGate(2,ppmt2/pmt2*1000.0);
//      tHit->SetGate(3,ppmt3/pmt3*1000.0);
//      tHit->SetGate(4,(bl0 - _nombaselines[plotIndex])*1000.0 );
//      tHit->SetGate(5,(bl1 - _nombaselines[plotIndex+1])*1000.0 );
//      tHit->SetGate(6,(bl2 - _nombaselines[plotIndex+2])*1000.0 );
//      tHit->SetGate(7,(bl3 - _nombaselines[plotIndex+3])*1000.0 );
      
      if(_blockPSD.count(blockSeq))
      {
          try {
              tHit->SetNeutronId(_blockPSD[blockSeq].GetNSigma(tHit)/_pixelSigmaScaling(sysrow,syscol));
              tHit->SetGammaId(_blockPSD[blockSeq].GetGSigma(tHit)/_pixelSigmaScaling(sysrow,syscol));
          }
          catch (...){
              LOG<<"Cannot Set NeutronId's for Slot("
              <<tHit->GetSlot()<<" Channel("
              <<tHit->GetChannel()<<") Energy("
              <<tHit->GetEnergy()<<") PSD("
              <<tHit->GetPSD()<<")"<<ENDM_WARN;
              tHit->SetNeutronId(-1e9);
              tHit->SetGammaId(-1e9);

          }
          
      }

  }

    
  return true;
  
}



bool  NGMBlockDetectorCalibrator::SaveNeutronCuts(const char* filename){
  
  TFile* cutOutput = 0;
  for(int plotIndex= 0; plotIndex < maxChannels; plotIndex++)
    {
      if(_neutronGate[plotIndex])
	{
	  if(!cutOutput)
	    {
	      cutOutput = TFile::Open(filename,"RECREATE");
	    }
	  cutOutput->WriteTObject(_neutronGate[plotIndex],_neutronGate[plotIndex]->GetName());
	}
    }
  if(cutOutput){
    cutOutput->Write();
    cutOutput->Close();
  }
  delete cutOutput;
  return true;
}

bool  NGMBlockDetectorCalibrator::ReadNeutronCuts(const char* filename, const char* prefixname)
{
	TFile* cutOutput = TFile::Open(filename);
	if(!cutOutput)
	{
		LOG<<" Neutron Cut File "<<filename<<" not found."<<ENDM_WARN;
		return false;
	}
	
	  for(int plotIndex = 0; plotIndex < maxChannels; plotIndex++)
	  {
      TString detName(_detNames[plotIndex].Data());
	    if(!detName.BeginsWith("LS")) continue;
	    char cbuf[1024];
	    sprintf(cbuf,"%s_%s_%s_NCut",prefixname,"g3pRg2pCal",detName.Data());
	    TCutG* tCut = (TCutG*)(cutOutput->Get(cbuf));
	    if(tCut)
	    {
	      LOG<<"Found Cut "<<cbuf<<ENDM_INFO;
	      if(_neutronGate[plotIndex]) delete _neutronGate[plotIndex];
	      _neutronGate[plotIndex] = tCut;
        //gROOT->Add(tCut);
	    }else{
	      LOG<<"Attempting to find "<<cbuf<<" failed"<<ENDM_INFO;
	    }

	  }  

	if(cutOutput){
		cutOutput->Close();
		delete cutOutput;
	}

	return true;
}

bool  NGMBlockDetectorCalibrator::FindCalibrationFileFromDB(const char* runnumber)
{
  // Use the NGMModule common runinfo lookup
  NGMModule::GetRunInfoFromDatabase(runnumber);
  
  if(NGMModule::_calibFileName!="")
  {
    SetCalFileName(NGMModule::_calibFileName);
  }
  
  return true;
}

ClassImp(NGMBlockBaseline)

NGMBlockBaseline::NGMBlockBaseline(double gatelength, int minentries, double tau)
: NGMBaseline(gatelength,minentries,5.0), _tau(tau),_rms(0.0),_sigthresh(5.0)
{
}

bool NGMBlockBaseline::AnaHit(NGMHit* hit)
{
  bool pileup = false;
  if(_rms==0.0 && MinHere())
  {
    _rms=RMS();
  }
  int bgate = hit->GetQuadGate(0);
  
  if( fabs(bgate/(double)_gatelength - Avg())>_sigthresh)
    pileup = true;
    
  push(bgate);
  return pileup;
}
