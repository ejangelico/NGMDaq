#ifndef __NCABlockMapping_H__
#define __NCABlockMapping_H__
//
//  BlockMapping.h
//  NGMDaq
//
//  Created by Newby, Robert Jason on 8/19/11.
//  Copyright 2011 ORNL. All rights reserved.
//
#include <vector>
#include <map>
#include <string>
#include "TObject.h"
#include "TMatrixD.h"
#include "TMatrixT.h"

namespace nca {

class BlockMapping : public TObject
{
  
public:
  BlockMapping();
  virtual ~BlockMapping(){}
  bool IsBlock(int chanindex);

  int GetNRows(){return _blocknrows;}
  int GetNColumns(){return _blockncols;}
  
  int GetBlockSequence(int chanindex);
  int GetBlockRow(int iblock);
  int GetBlockColumn(int iblock);
  std::string GetDetectorName(int iblock);
  int GetNBlocks() { return _blocknrows*_blockncols;}
  int GetBlockSequence(int row, int col);
  int GlobalPixelToLocalPixel(int gPixel, int& detSequence, int& localRow, int &localCol);
  double GetDistanceInPixels(int gPixel1, int gPixel2);
  int GetPixelGainGroup(int gPixel);
  int GetNPixOnASide(int detSeq = 0);
  bool IsNeighboringBlock(int gPixel1, int gPixel2 );
    bool Init(std::vector<int>& pixelsInDetRow, int ncols, int nrows, TMatrixD blockRowCol, std::vector<std::string>& detName, std::map<int,int> blockseq);
  int GetGlobalPixelFromGlobal2D(int gPixX, int gPixY);
  bool GetLocal2DFromGlobal2D(int gPixX, int gPixY, int& detSequence, int &lPixX, int &lPixY);
  bool GetGlobal2DFromLocal(int detSequence, int lPix, int &gPixX, int &gPixY);
  int GetGlobalFromLocal(int detSequence, int lPix);

private:
  int _blocknrows;
  int _blockncols;
  TMatrixD _blockRowCol;
  TMatrixD _blockSeq2D;
  // Lookup block seq from hardware channel
  std::map<int,int> _blockseq;
  // Detector number per hardware channel block
  std::vector<int> _detNum;
  std::vector<int> _chanNum;
  std::vector<std::string> _detName;
  std::vector<int> _pixelsInRow;
  
  ClassDef(BlockMapping,2)

};

}// namespace nca

#endif //__BlockMapping_H__
