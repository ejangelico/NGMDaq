/*
 *  NGMSISCalibrator.cpp
 *  NGMDaq
 *
 *  Created by Jason Newby on 11/28/06.
 *  Copyright 2012 ORNL. All rights reserved.
 *
 */

#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "NGMLogger.h"
#include "NGMBufferedPacket.h"
#include "NGMSystemConfiguration.h"
#include "NGMSISCalibrator.h"
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

ClassImp(NGMSISCalibrator)

void NGMSISCalibrator::InitCommon()
{
  partID = 0;
  nChannels = 0;
	numberOfHWChannels = 13*8;
	numberOfSlots = 13;
  blocknrows = 1;
  blockncols = 1;
  _psdScheme = 0;// Default scheme assumes xia gates 70,10,4,8,1,8,1,1
  
  for(int ichan = 0; ichan < maxChannels; ichan++){
    _detType[ichan] = 0;
    _nanosecondsPerSample[ichan] = 5.0;
    _mainGateLength[ichan] = 1000;
    _mainGatePreTrigger[ichan] = 200;
    _rawsampleStartIndex[ichan] = 0;
    _keVperADC[ichan] = 0.0;
	  _timingCal[ichan]= 0.0; //ns
    _channelenabled[ichan] = 1; // Assume all are enabled
    for(int igate = 0; igate < ngates; igate++)
    {
      _gatewidth[ichan][igate] = 0;
      _gateoffset[ichan][igate] = 0;
    }
    
  }
  

  
}
NGMSISCalibrator::NGMSISCalibrator()
{
  InitCommon();
}

NGMSISCalibrator::NGMSISCalibrator(const char* name, const char* title)
: NGMModule(name,title)
{
  char cbuf[1024];
  
  InitCommon();
  for(int ichan = 0; ichan < maxChannels; ichan++){
    sprintf(cbuf,"NA%02d",ichan);
    _detNames[ichan] = cbuf;
  }
  
  nChannels = 0;
	numberOfHWChannels = 13*8;
	numberOfSlots = 13;
  partID = new NGMSimpleParticleIdent;
  blocknrows = 1;
  blockncols = 1;
  _psdScheme = 0;// Default scheme assumes xia gates 70,10,4,8,1,8,1,1

  for(int ichan = 0; ichan < maxChannels; ichan++){
    _detNames[ichan] = (TString("")+=ichan);
    _detType[ichan] = 0;
    _nanosecondsPerSample[ichan] = 5.0;
    _mainGateLength[ichan] = 1000;
    _mainGatePreTrigger[ichan] = 200;
    _rawsampleStartIndex[ichan] = 0;
    _keVperADC[ichan] = -1.0;
  	_timingCal[ichan]= 0.0; //ns

    _channelenabled[ichan] = 1; // Assume all are enabled
    for(int igate = 0; igate < ngates; igate++)
    {
      _gatewidth[ichan][igate] = 0;
      _gateoffset[ichan][igate] = 0;
    }


  }

}

NGMSISCalibrator::~NGMSISCalibrator(){
  // Definitely need to clean up allocated memory here
  delete partID;
}

bool NGMSISCalibrator::init(){

  return true;
}

bool NGMSISCalibrator::finish(){
  
  return true;
}

bool  NGMSISCalibrator::processMessage(const TObjString* mess)
{
  push(*((const TObject *)mess));
  return true;
}
bool NGMSISCalibrator::processHit(const NGMHit* hit)
{
    push(*((const TObject *)hit));
  return true;
}

bool NGMSISCalibrator::processPacket(const NGMBufferedPacket* packetBuffer)
{
  if(packetBuffer->getSlotId()<0)
  {
    //Assume this a sorted packet of hits and pass it through
    push(*((const TObject *)packetBuffer));
    return true;
  }
  const NGMHit* tHit = packetBuffer->getHit(0);
  if(!tHit) return 0;
  
  if(!_channelenabled[partID->getPlotIndex(tHit)]) return 0;
  
  //LOG<<"Calibrating Old Packet Slot("<<tHit->GetSlot()
  // <<") Channel("<<tHit->GetChannel()
  // <<ENDM_INFO;
  NGMBufferedPacket* newPacket = new NGMBufferedPacketv2(packetBuffer->getSlotId(), packetBuffer->getPulseCount(), 
                                                         TTimeStamp(packetBuffer->getTimeStamp()),6);
  newPacket->CopyPacket(packetBuffer);
  analyzePacket(newPacket);
  push(*((const TObject*)newPacket));
  delete newPacket;
  return true;
}

bool NGMSISCalibrator::processConfiguration(const NGMSystemConfiguration* confBuffer)
{ 
  partID->Init(confBuffer);
  _blockPSD.clear();
  for(int ichan = 0; ichan < confBuffer->GetChannelParameters()->GetEntries(); ichan++)
  {
    _gatewidth[ichan][0] = confBuffer->GetSlotParameters()->GetParValueI("Gate1_Length",ichan/8);
    _gatewidth[ichan][1] = confBuffer->GetSlotParameters()->GetParValueI("Gate2_Length",ichan/8);
    _gatewidth[ichan][2] = confBuffer->GetSlotParameters()->GetParValueI("Gate3_Length",ichan/8);
    _gatewidth[ichan][3] = confBuffer->GetSlotParameters()->GetParValueI("Gate4_Length",ichan/8);
    _gatewidth[ichan][4] = confBuffer->GetSlotParameters()->GetParValueI("Gate5_Length",ichan/8);
    _gatewidth[ichan][5] = confBuffer->GetSlotParameters()->GetParValueI("Gate6_Length",ichan/8);
    _gatewidth[ichan][6] = confBuffer->GetSlotParameters()->GetParValueI("Gate7_Length",ichan/8);
    _gatewidth[ichan][7] = confBuffer->GetSlotParameters()->GetParValueI("Gate8_Length",ichan/8);
    _gateoffset[ichan][0] = confBuffer->GetSlotParameters()->GetParValueI("Gate1_StartIndex",ichan/8);
    _gateoffset[ichan][1] = confBuffer->GetSlotParameters()->GetParValueI("Gate2_StartIndex",ichan/8);
    _gateoffset[ichan][2] = confBuffer->GetSlotParameters()->GetParValueI("Gate3_StartIndex",ichan/8);
    _gateoffset[ichan][3] = confBuffer->GetSlotParameters()->GetParValueI("Gate4_StartIndex",ichan/8);
    _gateoffset[ichan][4] = confBuffer->GetSlotParameters()->GetParValueI("Gate5_StartIndex",ichan/8);
    _gateoffset[ichan][5] = confBuffer->GetSlotParameters()->GetParValueI("Gate6_StartIndex",ichan/8);
    _gateoffset[ichan][6] = confBuffer->GetSlotParameters()->GetParValueI("Gate7_StartIndex",ichan/8);
    _gateoffset[ichan][7] = confBuffer->GetSlotParameters()->GetParValueI("Gate8_StartIndex",ichan/8);
    _rawsampleStartIndex[ichan] = confBuffer->GetSlotParameters()->GetParValueI("RawSample_StartIndex",ichan/8);
    
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
  
  //Lets loop through each of the channels groups and determine the Block row, column, and pixelLUT
  blocknrows = confBuffer->GetSystemParameters()->GetParValueI("BLOCKNROWS",0);
  blockncols = confBuffer->GetSystemParameters()->GetParValueI("BLOCKNCOLS",0);
  _blockRowCol.ResizeTo(confBuffer->GetChannelParameters()->GetEntries(),2);
  _detkeVADC.Set(confBuffer->GetChannelParameters()->GetEntries());
  _psdScheme = 1;
  if(confBuffer->GetSystemParameters()->GetParIndex("PSD_SCHEME")>=0)
  {
    _psdScheme = confBuffer->GetSystemParameters()->GetParValueI("PSD_SCHEME",0);;
  }
  LOG<<" Using PSDScheme "<<_psdScheme<<ENDM_INFO;
  LOG<<" Using block detector array of "<<blocknrows<<" rows by "<<blockncols<<" columns." << ENDM_INFO;
  for(int ichan = 0; ichan < confBuffer->GetChannelParameters()->GetEntries(); ichan++)
  {
    int iblock = ichan;
    TString detName(confBuffer->GetChannelParameters()->GetParValueS("DetectorName",ichan));
    if(detName!="")
    {
      int detRow = confBuffer->GetDetectorParameters()->FindFirstRowMatching("DetectorName",detName);
      const NGMBlockPSD* blockPSD = 0;
      if(detRow>=0)
      {
        _blockRowCol(iblock,0) = confBuffer->GetDetectorParameters()->GetParValueI("BLOCK_ROW",detRow);
        _blockRowCol(iblock,1) = confBuffer->GetDetectorParameters()->GetParValueI("BLOCK_COL",detRow);
        LOG<<"Assigning detector "<<detName.Data()
        <<" a.k.a. iblock "<<iblock
        <<" to position ("<<_blockRowCol(iblock,0)
        <<","<<_blockRowCol(iblock,1)<<")"<<ENDM_INFO;
        blockPSD = dynamic_cast<const NGMBlockPSD*>(confBuffer->GetDetectorParameters()->GetParValueO("BLOCK_PSD",detRow));
      }
      if(blockPSD)
      {
        _blockPSD[ichan]=*blockPSD;
        LOG<<" Found BlockPSD "<< blockPSD->GetName()<<" for detector "<<detName.Data()
        <<" block seq "<<ichan<< ENDM_INFO;
      }
    }
  }
  
  if(numberOfHWChannels%numberOfSlots!=0){
    LOG<<"Number of Slots = "<<numberOfSlots<<" Number of Channels = "<<numberOfHWChannels<<". Not even multiple."<<ENDM_FATAL;
    return false;
  }
  push(*((const TObject *)confBuffer));
  return true;
}

bool NGMSISCalibrator::analyzePacket(NGMBufferedPacket* packet){
  if(!packet) return false;
  int nHits = packet->getPulseCount();
  for(int ihit = 0; ihit < nHits; ihit++){
    analyzeHit(packet->modHit(ihit));
  }  
  return true;
}

bool NGMSISCalibrator::analyzeHit(NGMHit* tHit){
  
  int plotIndex = partID->getPlotIndex(tHit);
  if(plotIndex < 0 || plotIndex >= maxChannels){
    LOG<<"plotIndex out of range "<<plotIndex<<ENDM_WARN;
    return false;
  }
  
  //Time calibration should be done here 
  tHit->SetCalibratedTime(tHit->GetRawTime());
  
  // These should be read from the configuration object
  double g1w = _gatewidth[plotIndex][0];
  double g2w = _gatewidth[plotIndex][1];
  double g3w = _gatewidth[plotIndex][2];
  double g4w = _gatewidth[plotIndex][3];
  double g5w = _gatewidth[plotIndex][4];
  double g6w = _gatewidth[plotIndex][5];
  double g7w = _gatewidth[plotIndex][6];
  double g8w = _gatewidth[plotIndex][7];
  double SumBaseline = tHit->GetGate(0)/g1w;

  double cfd = tHit->GetCFD();
  double SumIntegral = tHit->GetGate(1) - SumBaseline*g2w;

  double psd = (tHit->GetGate(2) - SumBaseline*g3w)/SumIntegral;
    
  //tHit->SetPulseHeight(SumIntegral);
  tHit->SetEnergy(SumIntegral);
  tHit->SetPSD(psd);
  
  if(_blockPSD.count(plotIndex))
  {
      tHit->SetNeutronId(_blockPSD[plotIndex].GetNSigma(tHit));
      tHit->SetGammaId(_blockPSD[plotIndex].GetGSigma(tHit));
  }

  return true;
  
}

