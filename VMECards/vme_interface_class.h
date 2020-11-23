#ifndef _VME_INTERFACE_CLASS_
#define _VME_INTERFACE_CLASS_


#define DEF_VME_READ_MODE_D32				0  
#define DEF_VME_READ_MODE_D32_DMA			1  
#define DEF_VME_READ_MODE_BLT32				2  
#define DEF_VME_READ_MODE_MBLT64			3  
#define DEF_VME_READ_MODE_2EVME				4  
#define DEF_VME_READ_MODE_2ESST160			5  
#define DEF_VME_READ_MODE_2ESST267			6  
#define DEF_VME_READ_MODE_2ESST320			7  

#include "TObject.h"

//#include <stdint.h>

typedef	signed char		int8_t;
typedef	unsigned char		uint8_t;
typedef	unsigned short uint16_t;
typedef	signed short int16_t;
typedef	unsigned int uint32_t;
typedef	signed int int32_t;
typedef int8_t CHAR;
typedef uint8_t  UCHAR;
typedef uint16_t USHORT;

#pragma once
class vme_interface_class : public TObject {
public:
    static vme_interface_class* Factory(const char* InterfaceType = "SISUSB");
    vme_interface_class(){};
    virtual ~vme_interface_class(){};

	virtual int vmeopen( void ) = 0;
	virtual int vmeclose( void ) = 0;

	virtual int get_vmeopen_messages( CHAR* messages, uint32_t* nof_found_devices ) = 0;

	virtual int vme_A32D32_read( uint32_t addr, uint32_t* data ) = 0;
	virtual int vme_A32D16_read( uint32_t addr, uint32_t* data ) = 0;

	virtual int vme_A32DMA_D32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32BLT32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32MBLT64_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32_2EVME_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32_2ESST160_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32_2ESST267_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32_2ESST320_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;

	virtual int vme_A32DMA_D32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32BLT32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32MBLT64FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32_2EVMEFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32_2ESST160FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32_2ESST267FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32_2ESST320FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;
	virtual int vme_A32_FastestFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ) = 0;


	virtual int vme_A32D32_write( uint32_t addr, uint32_t data ) = 0;
	virtual int vme_A32D16_write( uint32_t addr, uint16_t data ) = 0;

	virtual int vme_A32DMA_D32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ) = 0;
	virtual int vme_A32BLT32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ) = 0;
	virtual int vme_A32MBLT64_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ) = 0;

	virtual int vme_A32DMA_D32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ) = 0;
	virtual int vme_A32BLT32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ) = 0;
	virtual int vme_A32MBLT64FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ) = 0;

	virtual int vme_IRQ_Status_read( uint32_t* data ) = 0;

	virtual void setDigitizerIPaddress( const char* digiIP ) {};
  virtual void setPCIPaddress( const char* digiIP ) {};
  
	ClassDef(vme_interface_class,0)
};

class vme_dummy : public vme_interface_class
{
private:
    
    
public:
    vme_dummy(){}
	vme_dummy ( unsigned int  use_device_no ){}
    virtual ~vme_dummy(){}
    
	int vmeopen ( void ){return 0;}
	int vmeclose( void ){return 0;}
    
	int get_vmeopen_messages( CHAR* messages, uint32_t* nof_found_devices ){return 0;}
	int get_device_informations( USHORT* idVendor, USHORT* idProduct, USHORT* idSerNo, USHORT* idFirmwareVersion,  USHORT* idDriverVersion ){return 0;}
    
    
	int vme_A32D32_read (uint32_t addr, uint32_t* data){return 0;}
	int vme_A32D16_read (uint32_t addr, uint32_t* data){return 0;}
    
	int vme_A32DMA_D32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32BLT32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32MBLT64_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32_2EVME_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32_2ESST160_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32_2ESST267_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32_2ESST320_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
    
	int vme_A32DMA_D32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32BLT32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32MBLT64FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32_2EVMEFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32_2ESST160FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32_2ESST267FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32_2ESST320FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
	int vme_A32_FastestFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words ){return 0;}
    
    
	int vme_A32D32_write (uint32_t addr, uint32_t data);
    int vme_A32D16_write ( uint32_t addr, uint16_t data );
    
	int vme_A32DMA_D32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ){return 0;}
	int vme_A32BLT32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ){return 0;}
	int vme_A32MBLT64_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ){return 0;}
    
	int vme_A32DMA_D32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ){return 0;}
	int vme_A32BLT32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ){return 0;}
	int vme_A32MBLT64FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words ){return 0;}
    
	int vme_IRQ_Status_read( uint32_t* data ){return 0;}
    
	int sis3150Usb_Register_Read ( uint32_t addr, uint32_t* data) {return 0;}
    
    ClassDef(vme_dummy,0);
    
};

#endif
