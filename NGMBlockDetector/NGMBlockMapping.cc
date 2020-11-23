//
//  NGMBlockMapping.cpp
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "NGMBlockMapping.h"
#include "NGMSystemConfiguration.h"
#include "NGMLogger.h"
#include <cmath>

ClassImp(NGMBlockMapping)

NGMBlockMapping::NGMBlockMapping(){}

bool NGMBlockMapping::Init(const NGMSystemConfiguration* conf)
{ 
  
  _blocknrows = conf->GetSystemParameters()->GetParValueI("BLOCKNROWS",0);
  _blockncols = conf->GetSystemParameters()->GetParValueI("BLOCKNCOLS",0);

  _blockseq.clear();
  _detNum.clear();
  _chanNum.clear();
  _detName.clear();
  
  _blockRowCol.ResizeTo(_blocknrows*_blockncols,2);

  int iblockseq  = 0;
  for(int ichan = 0; ichan < conf->GetChannelParameters()->GetEntries(); ichan+=4) {
    
    std::string detName(conf->GetChannelParameters()->GetParValueS("DetectorName",ichan));
    
    // Let's see if this is one of our block detectors
    if(detName.find("LSBLOCKA")!=std::string::npos
       || detName.find("PSDBlock")!=std::string::npos
       || detName.find("PSDPlBlock")!=std::string::npos) {
      _detName.push_back(detName);
      int detRow = conf->GetDetectorParameters()->FindFirstRowMatching("DetectorName",detName.c_str());
      _chanNum.push_back(ichan);
      _blockseq[ichan]=iblockseq;
      if(detName.find("LSBLOCKA")!=std::string::npos) _pixelsInRow.push_back(10);
      if(detName.find("PSDBlockA")!=std::string::npos) _pixelsInRow.push_back(8);
      if(detName.find("PSDPlBlock8")!=std::string::npos) _pixelsInRow.push_back(8);
      if(detName.find("PSDPlBlock10")!=std::string::npos) _pixelsInRow.push_back(10);
        
      if(detRow>=0) {
        _detNum.push_back(detRow);
        _blockRowCol(detRow,0) = conf->GetDetectorParameters()->GetParValueI("BLOCK_ROW",detRow);
        _blockRowCol(detRow,1) = conf->GetDetectorParameters()->GetParValueI("BLOCK_COL",detRow);
        
      }else{
        _detNum.push_back(-1);
      }
      LOG<<"Adding Block "<<_detName.back().c_str()<<" Seq:"<<iblockseq<<" Det:"<<_detNum.back()<<" Chan:"<<_chanNum.back()<<"PixelsOnASide: "<<_pixelsInRow.back()<<ENDM_INFO;
      iblockseq++;
    }
  }
  
  return true;
}

int NGMBlockMapping::GetNPixOnASide(int detSeq){
    if(_pixelsInRow.size()<=detSeq) return 0;
    return _pixelsInRow[detSeq];
}

bool NGMBlockMapping::IsBlock(int chanindex) { return (bool)(_blockseq.count(chanindex)); }

int NGMBlockMapping::GetBlockSequence(int chanindex){
  if(IsBlock(chanindex))
    return _blockseq[chanindex];
  else
    return -1;
}

int NGMBlockMapping::GetBlockRow(int blockseq){return _blockRowCol[blockseq][0];}

int NGMBlockMapping::GetBlockColumn(int blockseq){return _blockRowCol[blockseq][1];}

std::string NGMBlockMapping::GetDetectorName(int blockseq){return _detName[blockseq];}

int NGMBlockMapping::GetBlockSequence(int row, int col)
{
    for(int irow = 0; irow<_blockRowCol.GetNrows(); irow++)
        if(_blockRowCol(irow,0)==row&&_blockRowCol(irow,1)==col) return irow;
    return -1;
}

int NGMBlockMapping::GlobalPixelToLocalPixel(int gPixel, int& detSequence, int& localRow, int &localCol)
{
    int pIR = _pixelsInRow[0];
    int gCol = gPixel%(_blockncols*pIR);
    int gRow = gPixel/(_blockncols*pIR);
    int detCol = gCol/pIR;
    int detRow = gRow/pIR;

    detSequence = GetBlockSequence(detRow,detCol);
    localRow = gRow%pIR;
    localCol = gCol%pIR;
    return 0;
}

bool NGMBlockMapping::IsNeighboringBlock(int gPixel1, int gPixel2 )
{
  int pIR = _pixelsInRow[0];
  int gCol1 = gPixel1%(_blockncols*pIR);
  int gRow1 = gPixel1/(_blockncols*pIR);
  int detCol1 = gCol1/pIR;
  int detRow1 = gRow1/pIR;

  int gCol2 = gPixel2%(_blockncols*pIR);
  int gRow2 = gPixel2/(_blockncols*pIR);
  int detCol2 = gCol2/pIR;
  int detRow2 = gRow2/pIR;

  if(abs(detCol1-detCol2)<=1 && abs(detRow1-detRow2)<=1) return true;
  return false;
}



double NGMBlockMapping::GetDistanceInPixels(int gPixel1, int gPixel2){
  int pIR = _pixelsInRow[0];
  int gCol1 = gPixel1%(_blockncols*pIR);
  int gRow1 = gPixel1/(_blockncols*pIR);
  int gCol2 = gPixel2%(_blockncols*pIR);
  int gRow2 = gPixel2/(_blockncols*pIR);
  
  return sqrt( (gCol2-gCol1)*(gCol2-gCol1) + (gRow2-gRow1)*(gRow2-gRow1));
}

int NGMBlockMapping::GetPixelGainGroup(int gPixel)
{
    int pIR = _pixelsInRow[0];
    int detSeq, localRow, localCol;
    GlobalPixelToLocalPixel(gPixel,detSeq,localRow,localCol);
    //Center
    if(1<localRow&&localRow<pIR-2&&1<localCol&&localCol<pIR-2) return 0;
    // Inner ring
    if(((localCol==1||localCol==pIR-2)&&(1<localRow&&localRow<pIR-2))
       ||((localRow==1||localRow==pIR-2)&&(1<localCol&&localCol<pIR-2)))
        return 1;
    //Outer ring
    if(((localCol==0||localCol==pIR-1)&&(1<localRow&&localRow<pIR-1))
        ||((localRow==0||localRow==pIR-1)&&(1<localCol&&localCol<pIR-1)))
          return 2;
    //Corner
    if((localCol==0||localCol==pIR-1)&&(localRow==0||localRow==pIR-1))
       return 4;
    //Next to Corner
       return 3;
}
