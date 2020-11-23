#ifndef __NGMBUFFEREDPACKET_H__
#define __NGMBUFFEREDPACKET_H__

#include "TObject.h"
#include "TTimeStamp.h"
#include "TClonesArray.h"
#include "NGMHit.h"

/// \brief NGMBufferedPacket provides the buffered output object and container
/// for the NGMHit objects.

class NGMBufferedPacket : public TObject
{
public:
  ///Default Constructor
  NGMBufferedPacket(){};
  NGMBufferedPacket(int slotid, int pulsecount, TTimeStamp time){}
  virtual ~NGMBufferedPacket(){}
  virtual void setSlotId(int slotid) {}
  virtual int getSlotId() const {return -1;}
  virtual void setChannelId(int channelid) {}
  virtual int getChannelId() const {return -1;}
  virtual int getPulseCount() const {return -1;}
  virtual const TTimeStamp getTimeStamp() const = 0;
  virtual TTimeStamp& modTimeStamp() = 0;
  virtual void setTimeStamp(TTimeStamp newVal) = 0;
  virtual NGMHit* modHit(int index) {return 0;}
  virtual const NGMHit* getHit(int index) const {return 0;}
  virtual NGMHit* addHit(const NGMHit* hitToCopy = 0, int ngates = 0) {return 0;}
  virtual void Clear(Option_t * /*option*/ =""){}
  virtual void initializeBuffer(int size = 10){}
  virtual void sortHits(){}
  virtual void CopyPacket(const NGMBufferedPacket*);
  virtual int getHitVersion() const { return 0; }
  /// Create a duplicate of the current packet
  /// If newVersion is 0 make a copy of the current version
  /// otherwise convert to the specified version
  virtual NGMBufferedPacket* DuplicatePacket(int newVersion = 0) const = 0;
  // OverloadedMethods
  virtual void Print(Option_t* option = "") const {}
  
  
  ClassDef(NGMBufferedPacket,1)
};
#ifdef NGMBUFFEREDPACKETV1
class NGMBufferedPacketv1 : public NGMBufferedPacket
{
public:
  NGMBufferedPacketv1();
  NGMBufferedPacketv1(int slotid, int pulsecount, TTimeStamp time, int hitVersion = 2);
  ~NGMBufferedPacketv1();
  virtual void setSlotId(int slotid) {_slotid = slotid;}
  virtual int getSlotId() const {return _slotid;}
  virtual int getPulseCount() const {return _pulsecount;}
  virtual const TTimeStamp getTimeStamp() const;
  virtual TTimeStamp& modTimeStamp();
  virtual void setTimeStamp(TTimeStamp newVal);
  virtual const NGMHit* getHit(int index) const;
  virtual NGMHit* modHit(int index);
  virtual NGMHit* addHit(const NGMHit* hitToCopy = 0, int ngates = 0);
  virtual void Clear(Option_t * /*option*/ ="");
  virtual void initializeBuffer(int size = 10);
  virtual void sortHits();
  virtual void Print(Option_t* option = "") const;
  virtual NGMBufferedPacket* DuplicatePacket(int newVersion = 0) const;
  virtual int getHitVersion() const { return _hitVersion; }
  
private:
  Int_t _slotid;
  TTimeStamp _buffertime;
  Int_t _pulsecount;
  TClonesArray* _buffer;
  int _hitVersion;
  
public:
    ClassDef(NGMBufferedPacketv1,2)
};
#endif


class NGMBufferedPacketv2 : public NGMBufferedPacket
{
public:
  NGMBufferedPacketv2();
  NGMBufferedPacketv2(int slotid, int pulsecount, TTimeStamp time, int hitVersion = 6);
  ~NGMBufferedPacketv2();
  virtual void setSlotId(int slotid) {_slotid = slotid;}
  virtual int getSlotId() const {return _slotid;}
  virtual void setChannelId(int channelid) {_channelid = channelid;}
  virtual int getChannelId() const {return _channelid;}
  virtual int getPulseCount() const {return _pulsecount;}
  virtual const TTimeStamp getTimeStamp() const;
  virtual TTimeStamp& modTimeStamp();
  virtual void setTimeStamp(TTimeStamp newVal);
  virtual const NGMHit* getHit(int index) const;
  virtual NGMHit* modHit(int index);
  virtual NGMHit* addHit(const NGMHit* hitToCopy = 0, int ngates = 0);
  virtual void Clear(Option_t * /*option*/ ="");
  virtual void initializeBuffer(int size = 10);
  virtual void sortHits();
  virtual void Print(Option_t* option = "") const;
  virtual NGMBufferedPacket* DuplicatePacket(int newVersion = 0) const;
  virtual int getHitVersion() const { return _hitVersion; }

private:
  Int_t _slotid;
  Int_t _channelid;
  TTimeStamp _buffertime;
  Int_t _pulsecount;
  TClonesArray* _buffer;
  int _hitVersion;
  
public:
    ClassDef(NGMBufferedPacketv2,1)
};

class NGMBufferedPacketv5 : public NGMBufferedPacket
{
public:
  NGMBufferedPacketv5();
  NGMBufferedPacketv5(int slotid, int pulsecount, TTimeStamp time, int hitVersion = 5);
  ~NGMBufferedPacketv5();
  virtual void setSlotId(int slotid) {_slotid = slotid;}
  virtual int getSlotId() const {return _slotid;}
  virtual void setChannelId(int channelid) {_channelid = channelid;}
  virtual int getChannelId() const {return _channelid;}
  virtual int getPulseCount() const {return _pulsecount;}
  virtual const TTimeStamp getTimeStamp() const;
  virtual TTimeStamp& modTimeStamp();
  virtual void setTimeStamp(TTimeStamp newVal);
  virtual const NGMHit* getHit(int index) const;
  virtual NGMHit* modHit(int index);
  virtual NGMHit* addHit(const NGMHit* hitToCopy = 0,  int ngates = 0);
  virtual void Clear(Option_t * /*option*/ ="");
  virtual void initializeBuffer(int size = 10);
  virtual void sortHits();
  virtual void Print(Option_t* option = "") const;
  virtual NGMBufferedPacket* DuplicatePacket(int newVersion = 0) const;
  virtual int getHitVersion() const { return 5; }

private:
  Int_t _slotid;
  Int_t _channelid;
  TTimeStamp _buffertime;
  Int_t _pulsecount;
  TClonesArray* _buffer;

  
public:
    ClassDef(NGMBufferedPacketv5,1)
};
#endif // __NGMBUFFEREDPACKET_H__
