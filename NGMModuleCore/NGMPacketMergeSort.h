#ifndef __NGMPACKETMERGESORT_H__
#define __NGMPACKETMERGESORT_H__

#include "NGMPacketBufferIO.h"
#include "NGMHit.h"
#include "NGMTimeGapFinder.h"

#include <map>

class TObjArray; //Forward Declaration
class TH1; //Forward Declaration

class NGMPacketMergeSort : public NGMPacketBufferIO
{
public:
  NGMPacketMergeSort();
  NGMPacketMergeSort(const char* name, const char* title);
  virtual ~NGMPacketMergeSort();
  virtual int pushPacket(NGMBufferedPacket*);
  virtual int setRequiredSlots(int); // *MENU*
  virtual void setRequiredBuffersPerSlot(int); // *MENU*
  virtual void setMergeMode(int); // *MENU*
  virtual int flush(); // *MENU*
  virtual void initialize();
  virtual int mergeHitsToLatestTime(NGMTimeStamp latestTime);
    virtual void setMaximumTimingCorrection(Double_t newVal) { _maxTimingCorrection = newVal ;}
    
    virtual int minBuffersInQueue();  
    virtual int flushBufferToList(int index);
    virtual NGMTimeStamp latestTimeInHitLists() const;
    virtual NGMTimeStamp earliestTimeInNextBuffers() const;
    virtual int pushHit(const NGMHit* tHit);
    virtual int getChannelIndex(const NGMHit* tHit) const;
    virtual bool init();
    virtual bool process(const TObject &);
    virtual bool finish();
  /// \brief SetMaxLiveTime sets the time in seconds after which the module will block further hits
  /// and issue a RequestAcquisitionStop to the top level NGMSystem
  virtual void SetMaxLiveTime(double maxtime) { _maxLiveTime = maxtime; } // *MENU*

  /// \brief SetPlotFrequency sets the time since previous plot update 
  /// and issue a ReqestPlotUpdate to daughter Modules
  virtual void SetPlotFrequency(double plotFrequency) { _plotfrequency = plotFrequency; } // *MENU*
  
  /// \brief SetSaveOnUpdate sets the option to save a copy of the analysis tree after each PlotUpdate
  virtual void SetSaveOnPlotUpdate(bool saveOnPlotUpdate = true) { _saveOnPlotUpdate = saveOnPlotUpdate; } // *MENU*

  /// \brief SetSetResetOnUpdate sets the option to save a copy of the analysis tree after each PlotUpdate
  virtual void SetResetOnPlotUpdate(bool resetOnPlotUpdate = true) { _resetOnPlotUpdate = resetOnPlotUpdate; } // *MENU*
  int getMergeSlot(const NGMBufferedPacket* packet) const;
  
	enum MergeMode
	{WaitOnPackets = 1, NoWaitOnPackets = 2};
  enum localconsts { maxchannels = 128 };
  
private:
  void InitCommon();
  
  int _requiredSlots;
  int _slotCount;
  int _bufferFlushWatermark;
  int _numpackets;
  int _numhits;
  int _maxactivechannels;
  int _channelsPerSlot;
  int _channelsPerBuffer;
  int _prevchanID;
  MergeMode _mergemode;
  double _avgcomps;
  Long64_t _ncompares;
  double _maxLiveTime; // Time in seconds
  bool _maxLiveTimeReached;
  double _plotfrequency;
  double _livetimeOfLastPlot;
  double _maxTimingCorrection;
  bool _saveOnPlotUpdate;
  bool _resetOnPlotUpdate;
  //containers and associative arrays could be stl
  std::map<int,int> _idMap; //! Map (1 per Slot)
                        // of a Linked List of NGMBufferedPackets
  TObjArray* _packetArray; //! Array (1 per Slot)
                           // of a Linked List of NGMBufferedPackets
  TObjArray* _hitArray; //! Array (1 per Slot)
                           // of a Linked List of NGMHits
  TObjArray* _hitSortedArray; //! Array (1 per Slot)
                             // of Sorted NGMHits
  TH1* _hTimeStampErrors;
  TH1* _hTotalEvents;
  NGMTimeGapFinder _gapFinder;
  
public:
    ClassDef(NGMPacketMergeSort,3)
};

#endif // __NGMPACKETMERGESORT_H__
