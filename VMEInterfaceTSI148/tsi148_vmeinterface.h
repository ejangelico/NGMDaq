#ifndef _tsi148_vmeinterface_VME_CLASS_
#define _tsi148_vmeinterface_VME_CLASS_


#include "vme_interface_class.h"

class tsi148_vmeinterface : public vme_interface_class
{
private:

	int _devHandle;
	int _ctlHandle;

public:
    tsi148_vmeinterface();
    virtual ~tsi148_vmeinterface(){};
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

	void btol(unsigned int* data, unsigned int nwords){
	    unsigned int iword = 0;
	    for(;iword<nwords;++iword){
	        *data =   ( ( (*data) & 0xFF ) << 24)
		        | ( ( (*data) & 0xFF00 ) << 8 )
                        | ( ( (*data) & 0xFF0000 ) >> 8 )
                        | ( ( (*data) & 0xFF000000 ) >> 24 );
                data++;
            }
	}
	void btol(unsigned short* data, unsigned int nwords){
	    unsigned int iword = 0;
	    for(;iword<nwords;++iword){
	        *data =   ( ( (*data) & 0xFF ) << 8)
		        | ( ( (*data) & 0xFF00 ) >> 8 );
                data++;
            }
	}
    
    ClassDef(tsi148_vmeinterface,0);

};

#endif
