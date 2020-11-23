#include "NGMBufferedPacket.h"
#include "NGMLogger.h"
#include "TROOT.h"
#include "TClass.h"
//#include <iostream>

ClassImp(NGMBufferedPacket)

void NGMBufferedPacket::CopyPacket(const NGMBufferedPacket* buffer){
  Clear();
  setSlotId(buffer->getSlotId());
  setChannelId(buffer->getChannelId());
  modTimeStamp() = buffer->getTimeStamp();
  for(int ihit = 0; ihit < buffer->getPulseCount();ihit++)
    addHit(buffer->getHit(ihit));
}
#ifdef NGMBUFFEREDPACKETV1
ClassImp(NGMBufferedPacketv1)

NGMBufferedPacketv1::NGMBufferedPacketv1()
{
  _slotid = -1;
  _pulsecount = 0;
  // Do not allocate memory in default constructor ... rootism
  _buffer = 0;
  _hitVersion = 2;
}

NGMBufferedPacketv1::NGMBufferedPacketv1(int slotid, int pulsecount, TTimeStamp time, int hitVersion)
: _buffertime(time)
{
  _slotid = slotid;
  _buffer = 0;
  _hitVersion = hitVersion;
  initializeBuffer(pulsecount);
  _buffertime = time;
  _pulsecount = 0;
}

NGMBufferedPacketv1::~NGMBufferedPacketv1()
{
  if(_buffer) delete _buffer;
  _buffer = 0;
}

void NGMBufferedPacketv1::setTimeStamp(TTimeStamp newVal)
{
  _buffertime = newVal;
}

const TTimeStamp NGMBufferedPacketv1::getTimeStamp() const
{
  return _buffertime;
}

TTimeStamp& NGMBufferedPacketv1::modTimeStamp()
{
  return _buffertime;
}

const NGMHit* NGMBufferedPacketv1::getHit( int index) const
{
  if(!_buffer) return 0;
  if(index < 0 || index >= _pulsecount) return 0;
  
  return dynamic_cast<NGMHit*>(_buffer->UncheckedAt(index));
}

NGMHit* NGMBufferedPacketv1::modHit( int index)
{
  if(!_buffer) return 0;
  if(index < 0 || index >= _pulsecount) return 0;
  
  return dynamic_cast<NGMHit*>(_buffer->UncheckedAt(index));
}

NGMHit* NGMBufferedPacketv1::addHit(const NGMHit* hit, int ngates)
{

  static TClass* tNGMHitv2Type = gROOT->GetClass("NGMHitv2"); 
  static TClass* tNGMHitv3Type = gROOT->GetClass("NGMHitv3"); 

  // If buffer not initialized yet do so now with defaults
  if(!_buffer)
    initializeBuffer(1000);
  NGMHit* tHit = 0;
  if(hit != 0){
	  if(hit->IsA() == tNGMHitv2Type && _hitVersion == 2){
		  tHit = new ((*_buffer)[_pulsecount++]) NGMHitv2(*((const NGMHitv2*)hit));
	  }else if(hit->IsA() == tNGMHitv3Type && _hitVersion == 3){
		// Copy calibrated
		  tHit = new ((*_buffer)[_pulsecount++]) NGMHitv3(*((const NGMHitv3*)hit));
    }else if(_hitVersion == 2){
      tHit = new ((*_buffer)[_pulsecount++]) NGMHitv2;
		  tHit->CopyHit(hit);
		} else if(_hitVersion == 3){
      tHit = new ((*_buffer)[_pulsecount++]) NGMHitv3;
		  tHit->CopyHit(hit);
		}
  }else{
    if(_hitVersion == 2)
	    tHit = new ((*_buffer)[_pulsecount++]) NGMHitv2();
    else
      tHit = new ((*_buffer)[_pulsecount++]) NGMHitv3();
  }

  return tHit;
}

void NGMBufferedPacketv1::Clear(Option_t * option)
{
  if(_buffer){
	//for(int ihit =0; ihit <_buffer->GetEntries(); ihit++){
    //  NGMHit* tHit = (NGMHit*) (*_buffer)[ihit];
    //  if(tHit) tHit->SetNSamples(0);
    //}
    _buffer->Delete();
    //_buffer->Clear();
  }
  _pulsecount = 0;
}

void NGMBufferedPacketv1::initializeBuffer(int size)
{
  if(_buffer) return;
  if(_hitVersion == 3)
    _buffer = new TClonesArray("NGMHitv3",size);
  else
    _buffer = new TClonesArray("NGMHitv2",size);
   // This is required to be able to stream the object over a TSocket
  _buffer->BypassStreamer(kFALSE);
}

void NGMBufferedPacketv1::sortHits(){
  if(!_buffer) return;
  // Let's first test the sort
  _buffer->Sort(); return;
  bool listissorted = true;
  for(int id = 1; id < _pulsecount; id++){
    if(_buffer->At(id)->Compare(_buffer->At(id-1)) < 0){
      listissorted = false;
      LOG<<"Packet not sorted "<<ENDM_INFO;
      LOG<<"Id "<<id -1<<" " << ((NGMHit*)(_buffer->At(id-1)))->GetTimeStamp().GetSec()
        <<" "<<((NGMHit*)(_buffer->At(id-1)))->GetTimeStamp().GetNanoSec()<<ENDM_INFO;
      LOG<<"Id "<<id <<" "<< ((NGMHit*)(_buffer->At(id)))->GetTimeStamp().GetSec()
        <<" "<<((NGMHit*)(_buffer->At(id)))->GetTimeStamp().GetNanoSec()<<ENDM_INFO;
      //break;
    }
  }
  // Really should implement our own bubble sort 
  // since our data are almost always already sorted rather
  // than using default quick sort algorithm.
  if(!listissorted) _buffer->Sort();
}

void NGMBufferedPacketv1::Print(Option_t* option) const
{
  LOG<<"NGMBufferedPacket: Slot("<<getSlotId()<<") ";
  LOG<<"\tTimeStamp ("<<getTimeStamp().GetSec()<<" , "<<getTimeStamp().GetNanoSec()<<") ";
  LOG<<"\tHitCount "<<getPulseCount()<<ENDM_INFO;
  _buffer->Print();
}

NGMBufferedPacket* NGMBufferedPacketv1::DuplicatePacket(int newVersion) const
{
  // Check if we have request the version 5Hits from version1 packets
  // if so lets return a v2 packet
  NGMBufferedPacket* tPacket = 0;
  if(newVersion == 5)
  {
    tPacket = new NGMBufferedPacketv2(_slotid,_pulsecount,_buffertime, 5);
    int pulseCount = getPulseCount();
    tPacket->initializeBuffer(pulseCount);
    tPacket->Copy(this);
  }else{

    // if new version is 0: just make a copy of existing version
    // otherwise convert to a the requested version
    int versionToConvert = newVersion ? newVersion : _hitVersion;
    tPacket = new NGMBufferedPacketv1(_slotid,_pulsecount,_buffertime, versionToConvert);
    int pulseCount = getPulseCount();
    tPacket->initializeBuffer(pulseCount);
    tPacket->Copy(this);
   }
  return tPacket;
}
#endif

// Implementation for NGMBufferedPacketv2
ClassImp(NGMBufferedPacketv2)

NGMBufferedPacketv2::NGMBufferedPacketv2()
{
  _slotid = -1;
  _channelid = -1;
  _pulsecount = 0;
  // Do not allocate memory in default constructor ... rootism
  _buffer = 0;
  _hitVersion = 6;
}

NGMBufferedPacketv2::NGMBufferedPacketv2(int slotid, int pulsecount, TTimeStamp time, int hitVersion)
: _buffertime(time)
{
  _slotid = slotid;
  _channelid = -1;
  _buffer = 0;
  _hitVersion = hitVersion;
  initializeBuffer(pulsecount);
  _buffertime = time;
  _pulsecount = 0;
}

NGMBufferedPacketv2::~NGMBufferedPacketv2()
{
  if(_buffer)
  {
    Clear();
    delete _buffer; 
  }
  _buffer = 0;
}

void NGMBufferedPacketv2::setTimeStamp(TTimeStamp newVal)
{
  _buffertime = newVal;
}

const TTimeStamp NGMBufferedPacketv2::getTimeStamp() const
{
  return _buffertime;
}

TTimeStamp& NGMBufferedPacketv2::modTimeStamp()
{
  return _buffertime;
}

const NGMHit* NGMBufferedPacketv2::getHit( int index) const
{
  if(!_buffer) return 0;
  if(index < 0 || index >= _pulsecount) return 0;
  
  return dynamic_cast<NGMHit*>(_buffer->UncheckedAt(index));
}

NGMHit* NGMBufferedPacketv2::modHit( int index)
{
  if(!_buffer) return 0;
  if(index < 0 || index >= _pulsecount) return 0;
  
  return dynamic_cast<NGMHit*>(_buffer->UncheckedAt(index));
}

NGMHit* NGMBufferedPacketv2::addHit(const NGMHit* hit, int ngates)
{

  static TClass* tNGMHitv4Type = TClass::GetClass("NGMHitv4"); 
  static TClass* tNGMHitv5Type = TClass::GetClass("NGMHitv5"); 
  static TClass* tNGMHitv6Type = TClass::GetClass("NGMHitv6"); 
  static TClass* tNGMHitv8Type = TClass::GetClass("NGMHitv8"); 

  // If buffer not initialized yet do so now with defaults
  if(!_buffer)
    initializeBuffer(1000);
  NGMHit* tHit = 0;
  if(hit != 0){
	  if(hit->IsA() == tNGMHitv4Type && _hitVersion == 4){
		  tHit = new ((*_buffer)[_pulsecount++]) NGMHitv4(*((const NGMHitv4*)hit));
	  }else if(hit->IsA() == tNGMHitv5Type && _hitVersion == 5){
		// Copy calibrated
		  tHit = new ((*_buffer)[_pulsecount++]) NGMHitv5(*((const NGMHitv5*)hit));
    }else if(hit->IsA() == tNGMHitv6Type && _hitVersion == 6){
      // Copy calibrated
		  tHit = new ((*_buffer)[_pulsecount++]) NGMHitv6(*((const NGMHitv6*)hit));
    }else if(hit->IsA() == tNGMHitv8Type && _hitVersion == 8){
      // Copy calibrated
		  tHit = new ((*_buffer)[_pulsecount++]) NGMHitv8(*((const NGMHitv8*)hit));
    }else if(_hitVersion == 4){
      tHit = new ((*_buffer)[_pulsecount++]) NGMHitv4;
		  tHit->CopyHit(hit);
		} else if(_hitVersion == 5){
      tHit = new ((*_buffer)[_pulsecount++]) NGMHitv5;
		  tHit->CopyHit(hit);
		} else if(_hitVersion == 6){
      tHit = new ((*_buffer)[_pulsecount++]) NGMHitv6;
		} else if(_hitVersion == 8){
      tHit = new ((*_buffer)[_pulsecount++]) NGMHitv8;
    }
  }else{
    if(_hitVersion == 4)
	    tHit = new ((*_buffer)[_pulsecount++]) NGMHitv4();
    else if(_hitVersion == 5)
	    tHit = new ((*_buffer)[_pulsecount++]) NGMHitv5();
    else if (ngates>0)
      tHit = new ((*_buffer)[_pulsecount++]) NGMHitv8(ngates);
    else
      tHit = new ((*_buffer)[_pulsecount++]) NGMHitv8();
  }

  return tHit;
}

void NGMBufferedPacketv2::Clear(Option_t* option)
{
  if(_buffer){
    _buffer->Delete();
  }
  _pulsecount = 0;
}

void NGMBufferedPacketv2::initializeBuffer(int size)
{
  if(_buffer) return;
  if(_hitVersion == 6)
    _buffer = new TClonesArray("NGMHitv6",size);
  //_buffer = new TObjArray(size);
  else if(_hitVersion == 5)
    _buffer = new TClonesArray("NGMHitv5",size);
  else
    _buffer = new TClonesArray("NGMHitv4",size);
  // This is required to be able to stream the object over a TSocket
  _buffer->BypassStreamer(kFALSE);

}

void NGMBufferedPacketv2::sortHits(){
  if(!_buffer) return;
  // Let's first test the sort
  _buffer->Sort(); return;
  bool listissorted = true;
  for(int id = 1; id < _pulsecount; id++){
    if(_buffer->At(id)->Compare(_buffer->At(id-1)) < 0){
      listissorted = false;
      LOG<<"Packet not sorted "<<ENDM_INFO;
      LOG<<"Id "<<id -1<<" " << ((NGMHit*)(_buffer->At(id-1)))->GetNGMTime().GetSec()
        <<" "<<((NGMHit*)(_buffer->At(id-1)))->GetNGMTime().GetNanoSec()<<ENDM_INFO;
      LOG<<"Id "<<id <<" "<< ((NGMHit*)(_buffer->At(id)))->GetNGMTime().GetSec()
        <<" "<<((NGMHit*)(_buffer->At(id)))->GetNGMTime().GetNanoSec()<<ENDM_INFO;
      break;
    }
  }
  // Really should implement our own bubble sort 
  // since our data are almost always already sorted rather
  // than using default quick sort algorithm.
  if(!listissorted) _buffer->Sort();
}

void NGMBufferedPacketv2::Print(Option_t* option) const
{
  LOG<<"NGMBufferedPacket: Slot("<<getSlotId()<<") ";
  LOG<<"NGMBufferedPacket: Channel("<<getChannelId()<<") ";
  LOG<<"\tTimeStamp ("<<getTimeStamp().GetSec()<<" , "<<getTimeStamp().GetNanoSec()<<") ";
  LOG<<"\tHitCount "<<getPulseCount()<<ENDM_INFO;
  _buffer->Print();
}

NGMBufferedPacket* NGMBufferedPacketv2::DuplicatePacket(int newVersion) const
{
  // if new version is 0: just make a copy of existing version
  // otherwise convert to a the requested version
  int versionToConvert = newVersion ? newVersion : _hitVersion;
  NGMBufferedPacket* tPacket = new NGMBufferedPacketv2(_slotid,_pulsecount,_buffertime, versionToConvert);
  int pulseCount = getPulseCount();
  tPacket->initializeBuffer(pulseCount);
  tPacket->CopyPacket(this);
  return tPacket;
}

// Implementation for NGMBufferedPacketv5
ClassImp(NGMBufferedPacketv5)

NGMBufferedPacketv5::NGMBufferedPacketv5()
{
  _slotid = -1;
  _channelid = -1;
  _pulsecount = 0;
  // Do not allocate memory in default constructor ... rootism
  _buffer = 0;
}

NGMBufferedPacketv5::NGMBufferedPacketv5(int slotid, int pulsecount, TTimeStamp time, int hitVersion)
: _buffertime(time)
{
  _slotid = slotid;
  _channelid = -1;
  _buffer = 0;
  initializeBuffer(pulsecount);
  _buffertime = time;
  _pulsecount = 0;
}

NGMBufferedPacketv5::~NGMBufferedPacketv5()
{
  if(_buffer) delete _buffer;
  _buffer = 0;
}

void NGMBufferedPacketv5::setTimeStamp(TTimeStamp newVal)
{
  _buffertime = newVal;
}

const TTimeStamp NGMBufferedPacketv5::getTimeStamp() const
{
  return _buffertime;
}

TTimeStamp& NGMBufferedPacketv5::modTimeStamp()
{
  return _buffertime;
}

const NGMHit* NGMBufferedPacketv5::getHit( int index) const
{
  if(!_buffer) return 0;
  if(index < 0 || index >= _pulsecount) return 0;
  
  return dynamic_cast<NGMHit*>(_buffer->UncheckedAt(index));
}

NGMHit* NGMBufferedPacketv5::modHit( int index)
{
  if(!_buffer) return 0;
  if(index < 0 || index >= _pulsecount) return 0;
  
  return dynamic_cast<NGMHit*>(_buffer->UncheckedAt(index));
}

NGMHit* NGMBufferedPacketv5::addHit(const NGMHit* hit, int ngates)
{

  static TClass* tNGMHitv5Type = gROOT->GetClass("NGMHitv5"); 

  // If buffer not initialized yet do so now with defaults
  if(!_buffer)
    initializeBuffer(1000);
  NGMHit* tHit = 0;
  if(hit != 0){
	if(hit->IsA() == tNGMHitv5Type){
		// Copy calibrated
		  tHit = new ((*_buffer)[_pulsecount++]) NGMHitv5(*((const NGMHitv5*)hit));
    }else{
      tHit = new ((*_buffer)[_pulsecount++]) NGMHitv5;
		  tHit->CopyHit(hit);
	}
  }else{
	tHit = new ((*_buffer)[_pulsecount++]) NGMHitv5();
  }

  return tHit;
}

void NGMBufferedPacketv5::Clear(Option_t* option)
{
  if(_buffer){
	//for(int ihit =0; ihit <_buffer->GetEntries(); ihit++){
    //  NGMHit* tHit = (NGMHit*) (*_buffer)[ihit];
    //  if(tHit) tHit->SetNSamples(0);
    //}
    _buffer->Delete();
    //_buffer->Clear();
  }
  _pulsecount = 0;
}

void NGMBufferedPacketv5::initializeBuffer(int size)
{
  if(_buffer) return;
  _buffer = new TClonesArray("NGMHitv5",size);

  // This is required to be able to stream the object over a TSocket
  _buffer->BypassStreamer(kFALSE);

}

void NGMBufferedPacketv5::sortHits(){
  if(!_buffer) return;
  // Let's first test the sort
  bool listissorted = true;
  for(int id = 1; id < _pulsecount; id++){
    if(_buffer->At(id)->Compare(_buffer->At(id-1)) < 0){
      listissorted = false;
      LOG<<"Packet not sorted "<<ENDM_INFO;
      LOG<<"Id "<<id -1<<" " << ((NGMHit*)(_buffer->At(id-1)))->GetNGMTime().GetSec()
        <<" "<<((NGMHit*)(_buffer->At(id-1)))->GetNGMTime().GetNanoSec()<<ENDM_INFO;
      LOG<<"Id "<<id <<" "<< ((NGMHit*)(_buffer->At(id)))->GetNGMTime().GetSec()
        <<" "<<((NGMHit*)(_buffer->At(id)))->GetNGMTime().GetNanoSec()<<ENDM_INFO;
      //break;
    }
  }
  // Really should implement our own bubble sort 
  // since our data are almost always already sorted rather
  // than using default quick sort algorithm.
  if(!listissorted) _buffer->Sort();
}

void NGMBufferedPacketv5::Print(Option_t* option) const
{
  LOG<<"NGMBufferedPacket: Slot("<<getSlotId()<<") ";
  LOG<<"NGMBufferedPacket: Channel("<<getChannelId()<<") ";
  LOG<<"\tTimeStamp ("<<getTimeStamp().GetSec()<<" , "<<getTimeStamp().GetNanoSec()<<") ";
  LOG<<"\tHitCount "<<getPulseCount()<<ENDM_INFO;
  _buffer->Print();
}

NGMBufferedPacket* NGMBufferedPacketv5::DuplicatePacket(int newVersion) const
{
  // if new version is 0: just make a copy of existing version
  // otherwise convert to a the requested version
  NGMBufferedPacket* tPacket = new NGMBufferedPacketv5(_slotid,_pulsecount,_buffertime);
  tPacket->CopyPacket(this);
  return tPacket;
}
