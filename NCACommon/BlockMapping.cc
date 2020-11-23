//
//  BlockMapping.cpp
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//

#include "BlockMapping.h"
#include <cmath>

ClassImp(nca::BlockMapping)

namespace nca {

BlockMapping::BlockMapping(){}

int BlockMapping::GetNPixOnASide(int detSeq){
    if(_pixelsInRow.size()<=detSeq) return 0;
    return _pixelsInRow[detSeq];
}

bool BlockMapping::IsBlock(int chanindex) { return (bool)(_blockseq.count(chanindex)); }

int BlockMapping::GetBlockSequence(int chanindex){
  if(IsBlock(chanindex))
    return _blockseq[chanindex];
  else
    return -1;
}

int BlockMapping::GetBlockRow(int blockseq){return _blockRowCol[blockseq][0];}

int BlockMapping::GetBlockColumn(int blockseq){return _blockRowCol[blockseq][1];}

std::string BlockMapping::GetDetectorName(int blockseq){return _detName[blockseq];}

int BlockMapping::GetBlockSequence(int row, int col)
{
    return _blockSeq2D(row,col);
}

int BlockMapping::GlobalPixelToLocalPixel(int gPixel, int& detSequence, int& localRow, int &localCol)
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

int BlockMapping::GetGlobalPixelFromGlobal2D(int gPixX, int gPixY)
{
    int pIR = _pixelsInRow[0];
    return _blockncols*pIR*gPixY+gPixX;
}

bool BlockMapping::GetGlobal2DFromLocal(int detSequence, int lPix, int &gPixX, int &gPixY)
{
    int pIR = _pixelsInRow[0];
    
    gPixY = _blockRowCol(detSequence,0)*pIR + lPix/pIR;
    gPixX = _blockRowCol(detSequence,1)*pIR + lPix%pIR;
    return true;
}
    
int BlockMapping::GetGlobalFromLocal(int detSequence, int lPix)
{
    int pIR = _pixelsInRow[0];
    int gPixY = _blockRowCol(detSequence,0)*pIR + lPix/pIR;
    int gPixX = _blockRowCol(detSequence,1)*pIR + lPix%pIR;
    return pIR * _blockncols * gPixY + gPixX;
}
    
bool BlockMapping::GetLocal2DFromGlobal2D(int gPixX, int gPixY, int& detSequence, int &lPixX, int &lPixY)
{
    int pIR = _pixelsInRow[0];
    int detCol = gPixX/pIR;
    int detRow = gPixY/pIR;
    
    detSequence = GetBlockSequence(detRow,detCol);
    lPixY = gPixY%pIR;
    lPixX = gPixX%pIR;
    return true;
}

    
bool BlockMapping::IsNeighboringBlock(int gPixel1, int gPixel2 )
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



double BlockMapping::GetDistanceInPixels(int gPixel1, int gPixel2){
  int pIR = _pixelsInRow[0];
  int gCol1 = gPixel1%(_blockncols*pIR);
  int gRow1 = gPixel1/(_blockncols*pIR);
  int gCol2 = gPixel2%(_blockncols*pIR);
  int gRow2 = gPixel2/(_blockncols*pIR);
  
  return sqrt( (gCol2-gCol1)*(gCol2-gCol1) + (gRow2-gRow1)*(gRow2-gRow1));
}

int BlockMapping::GetPixelGainGroup(int gPixel)
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

bool BlockMapping::Init(std::vector<int>& pixelsInDetRow, int ncols, int nrows, TMatrixD blockRowCol, std::vector<std::string>& detName, std::map<int,int> blockseq)
{
    _blockncols = ncols;
    _blocknrows = nrows;
    _blockSeq2D.ResizeTo(nrows, ncols);
    _blockRowCol.ResizeTo(nrows*ncols,2);
    _pixelsInRow = pixelsInDetRow;
    _blockRowCol = blockRowCol;
    _detName = detName;
    _blockseq = blockseq;
    for(int irow = 0; irow<_blockRowCol.GetNrows(); irow++)
        _blockSeq2D(_blockRowCol(irow,0),_blockRowCol(irow,1))=irow;

    return true;
}
    
}// namespace nca

