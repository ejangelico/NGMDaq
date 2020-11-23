#ifndef _SIS3150W_VME_CLASS_
#define _SIS3150W_VME_CLASS_


#include "vme_interface_class.h"

struct usb_dev_handle; // Forward declaration from usb.h

class sis3150 : public vme_interface_class
{
private:

	usb_dev_handle* _sis3150_device;
    uint32_t  _used_device_no;
	char _char_messages[128] ;
	uint32_t _nof_devices_found ;
    
	USHORT _idVendor;
	USHORT _idProduct;
	USHORT _idSerNo;
	USHORT _idFirmwareVersion;
	USHORT _idDriverVersion;

public:
    sis3150();
	sis3150 ( unsigned int  use_device_no );
    virtual ~sis3150();
	int vmeopen ( void );
	int vmeclose( void );

	int get_vmeopen_messages( CHAR* messages, uint32_t* nof_found_devices );
	int get_device_informations( USHORT* idVendor, USHORT* idProduct, USHORT* idSerNo, USHORT* idFirmwareVersion,  USHORT* idDriverVersion );


	int vme_A32D32_read (uint32_t addr, uint32_t* data);
	int vme_A32D16_read( uint32_t addr, uint32_t* data );

	int vme_A32DMA_D32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32BLT32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32MBLT64_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32_2EVME_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32_2ESST160_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32_2ESST267_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32_2ESST320_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );

	int vme_A32DMA_D32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32BLT32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32MBLT64FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32_2EVMEFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32_2ESST160FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32_2ESST267FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32_2ESST320FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );
	int vme_A32_FastestFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words );


	int vme_A32D32_write (uint32_t addr, uint32_t data);
	int vme_A32D16_write (uint32_t addr, uint16_t data);

	int vme_A32DMA_D32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );
	int vme_A32BLT32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );
	int vme_A32MBLT64_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );

	int vme_A32DMA_D32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );
	int vme_A32BLT32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );
	int vme_A32MBLT64FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words );

	int vme_IRQ_Status_read( uint32_t* data ) ;

	int sis3150Usb_Register_Read ( uint32_t addr, uint32_t* data);
    
    int sis3150Usb_Register_Write(uint32_t addr, uint32_t data);
    
    ClassDef(sis3150,0);

};

#endif
