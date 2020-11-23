#ifndef __NGMBLOCKMAPPING_H__
#define __NGMBLOCKMAPPING_H__
//
//  NGMBlockMapping.h
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

class NGMSystemConfiguration; // forward declaration
class NGMHit; //forward declaration

class NGMBlockMapping : public TObject
{
  
public:
  NGMBlockMapping();
  virtual ~NGMBlockMapping(){}
  bool Init(const NGMSystemConfiguration* conf);
  bool IsBlock(int chanindex);

  int GetNRows(){return _blocknrows;}
  int GetNColumns(){return _blockncols;}
  
  int GetBlockSequence(int chanindex);
  int GetBlockRow(int iblock);
  int GetBlockColumn(int iblock);
  std::string GetDetectorName(int iblock);
  int GetNBlocks() { return _chanNum.size();}
  int GetBlockSequence(int row, int col);
  int GlobalPixelToLocalPixel(int gPixel, int& detSequence, int& localRow, int &localCol);
  double GetDistanceInPixels(int gPixel1, int gPixel2);
  int GetPixelGainGroup(int gPixel);
  int GetNPixOnASide(int detSeq = 0);
  bool IsNeighboringBlock(int gPixel1, int gPixel2 );
  
private:
  int _blocknrows;
  int _blockncols;
  TMatrixD _blockRowCol;
  // Lookup block seq from hardware channel
  std::map<int,int> _blockseq;
  // Detector number per hardware channel block
  std::vector<int> _detNum;
  std::vector<int> _chanNum;
  std::vector<std::string> _detName;
  std::vector<int> _pixelsInRow;
  
  ClassDef(NGMBlockMapping,2)

};
#endif //__NGMBLOCKMAPPING_H__
