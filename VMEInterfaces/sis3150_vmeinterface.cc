#include <iostream>
#include <cstdio>

#include "sis3150usb.h"
#include "sis3150usb_vme.h"
#include "sis3150_vmeinterface.h"

#define MAX_SIS3150USB_DEVICES		4 

using namespace std;

ClassImp(sis3150)

sis3150::sis3150()
{
    _sis3150_device = 0;
    _used_device_no = 1;
}

sis3150::sis3150 ( unsigned int use_device_no )
{
    _sis3150_device = 0;
    _used_device_no = use_device_no;
}


int sis3150::vmeopen ( void )
{
    // -- USB VME Interface Setup
    
    /************************************
     *        init sis3150USB
     ************************************/
	int error=0;
	int return_code ;
  unsigned int reg;

    int i ;
    //UNUSED: char messages_buffer[80] ;
    char    gl_cDName[128] ;    //DriverNames
    struct SIS3150USB_Device_Struct device_info[MAX_SIS3150USB_DEVICES];

	FindAll_SIS3150USB_Devices(device_info, &_nof_devices_found, MAX_SIS3150USB_DEVICES)  ;
    
	if (_nof_devices_found == 0) {
        printf ("No SIS3150USB Device found\n");
		//exit(-1) ;
		_used_device_no = 0 ;
        _sis3150_device = NULL ;
	}
	else {
        printf ("number of SIS3150USB devices = %d \n\n",_nof_devices_found);
        
        
		
        printf ("Device    Vendor   Product   Ser.No. \n");
		for (i=0;i<_nof_devices_found;i++) {
            printf ("%s   0x%04x   0x%04x    0x%04x ",
                     (UCHAR*)  &device_info[i].cDName,
                     (USHORT)  device_info[i].idVendor,
                     (USHORT)  device_info[i].idProduct,
                     (USHORT)  device_info[i].idSerNo    );
		}
		        
	 	//sprintf (gl_cDName, "%s", (UCHAR*)&gl_sis3150_device_information_struct[gl_usb_index-1].cDName);
	 	sprintf (gl_cDName, "%s", reinterpret_cast<char*>(&device_info[_used_device_no-1].cDName));
 		printf ("\n\nuse USB Devices %s\n", gl_cDName);
		return_code = Sis3150usb_OpenDriver_And_Download_FX2_Setup (gl_cDName, &device_info[_used_device_no-1] );
    _sis3150_device = device_info[_used_device_no-1].hDev;
    
 		if(return_code != 0) {
            _sis3150_device = NULL ;
            printf ("%s\n", "ERROR Loading BIX File");
 		}
        
    if ((error = sis3150Usb_Register_Single_Read(_sis3150_device, 0x1, &reg )) != 0) {
      printf("ERROR First read sis3150Usb_Register_Single_Read");
      return error;
    }else{
      printf("Firmware: 0x%x\n", reg);
    }
		if ((error = sis3150Usb_Register_Single_Write(_sis3150_device, SIS3150USB_VME_MASTER_CONTROL_STATUS, 0xe000 )) != 0) {
			printf("ERROR sis3150Usb_Register_Single_Write");
			return error;
		}
        
        
	}

	sis3150Usb_Register_Single_Write ( _sis3150_device, 0x0, /* USB Control/Status Register */  0x9); // set Usr Led
	sis3150Usb_Register_Single_Write ( _sis3150_device, 0x10, /* USB VME Master Control/Status Register */  0xC000) ; // set VME System Controller Berr to 100us
    uint32_t data = 0;
 	sis3150Usb_Register_Single_Read ( _sis3150_device, 0x1,   &data) ;  
	_idFirmwareVersion = (USHORT) (data & 0xffff) ;

	_idVendor  = device_info[_used_device_no].idVendor ;
	_idProduct = device_info[_used_device_no].idProduct ;
	_idSerNo   = device_info[_used_device_no].idSerNo ;
	_idDriverVersion = 0 ; // reserved

	strcpy (_char_messages ,  "sis3150usb device open OK");
	return 0;
}

sis3150::~sis3150(){
    vmeclose();
}

int sis3150::vmeclose( void ){
	Sis3150usb_CloseDriver (_sis3150_device);
	return 0;
}

/**************************************************************************************/

int sis3150::get_vmeopen_messages( CHAR* messages, uint32_t* nof_found_devices ){
	strcpy (reinterpret_cast<char*>(messages),  _char_messages);
	*nof_found_devices = _nof_devices_found ;
	return 0;
}
/**************************************************************************************/

int sis3150::get_device_informations( USHORT* idVendor, USHORT* idProduct, USHORT* idSerNo, USHORT* idFirmwareVersion,  USHORT* idDriverVersion ){
	*idVendor          = _idVendor ;
	*idProduct         = _idProduct ;
	*idSerNo           = _idSerNo  ;
	*idFirmwareVersion = _idFirmwareVersion ;
	*idDriverVersion   = _idDriverVersion ;
	return 0;
}




/**************************************************************************************/
int sis3150::sis3150Usb_Register_Read (uint32_t addr, uint32_t* data)
{
int return_code ;
	return_code = sis3150Usb_Register_Single_Read(_sis3150_device, addr,  data);
	return return_code;
}

/**************************************************************************************/
int sis3150::sis3150Usb_Register_Write(uint32_t addr, uint32_t data)
{
    int return_code ;
	return_code = sis3150Usb_Register_Single_Write(_sis3150_device, addr,  data);
	return return_code;
}

/**************************************************************************************/

int sis3150::vme_IRQ_Status_read ( uint32_t* data)
{
int return_code=0 ;
uint32_t read_val;
	return_code = sis3150Usb_Register_Single_Read(_sis3150_device, SIS3150USB_VME_INTERRUPT_STATUS,  &read_val);
	*data = (read_val ) & 0xff ;
	return return_code;
}

/*****************/
/*               */
/*    VME A32    */
/*               */
/*****************/

// non priv
//   vme_am_mode = 0x8 ;  // MBLT64 
//   vme_am_mode = 0xB ;  // BLT32
//   vme_am_mode = 0x9 ;  // D32

// supervisor
//   vme_am_mode = 0xC ;  // MBLT64 
//   vme_am_mode = 0xF ;  // BLT32
//   vme_am_mode = 0xD ;  // D32

/**************************************************************************************/
int sis3150::vme_A32D32_read (uint32_t addr, uint32_t* data)
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Single_Read(_sis3150_device, addr, 0x9,4, data)  ;
    if(return_code)
    {
        printf("ERROR sis3150::vme_A32D32_read: Addr(0x%08x) Data(0x%08x)\n",addr,data[0]);
    }
	return return_code;
}
/**************************************************************************************/
int sis3150::vme_A32D16_read (uint32_t addr, uint32_t* data)
{
    uint32_t readdata;
    int return_code=0 ;
    return_code = sis3150Usb_Vme_Single_Read(_sis3150_device, addr, 0x9,2, &readdata)  ;
    *data = (uint16_t)readdata;
    printf("vme_A32D16_read 0x%08x %x %x\n",addr,readdata,*data);
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32DMA_D32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,0x9, 4, 0, data, request_nof_words, got_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32BLT32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,0xb, 4, 0, data, request_nof_words, got_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32MBLT64_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr, 0x8, 8, 0, data, request_nof_words, got_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32_2EVME_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
  int return_code=0 ;
  return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,  0x20, 8, 0, data, request_nof_words & 0xfffffffe, got_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32_2ESST160_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,  0x60, 8, 0, data, request_nof_words, got_nof_words)  ;// MBLT64 !!!
	return return_code;
}


/**************************************************************************************/
int sis3150::vme_A32_2ESST267_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,  0x8, 8, 0, data, request_nof_words, got_nof_words)  ;// MBLT64 !!!
	return return_code;
}


/**************************************************************************************/
int sis3150::vme_A32_2ESST320_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
  /* new SIS3153 */

  int return_code=0 ;
  return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,  0x260, 8, 0, data, request_nof_words & 0xfffffffe, got_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32DMA_D32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,0x9, 4, 1, data, request_nof_words, got_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32BLT32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,0xb, 4, 1, data, request_nof_words, got_nof_words)  ;
    if(return_code)
    {
        printf("ERROR(%d) sis3150::vme_A32BLT32FIFO_read: Addr(0x%08x) Request(%d)\n",return_code,addr,request_nof_words);
    }
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32MBLT64FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr, 0x8, 8, 1, data, request_nof_words, got_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32_FastestFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
  //vme_A32MBLT64FIFO_read
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr, 0x8, 8, 1, data, request_nof_words, got_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32_2EVMEFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,  0x8, 8, 1, data, request_nof_words, got_nof_words)  ;// MBLT64 !!!
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32_2ESST160FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,  0x8, 8, 1, data, request_nof_words, got_nof_words)  ;// MBLT64 !!!
	return return_code;
}


/**************************************************************************************/
int sis3150::vme_A32_2ESST267FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,  0x8, 8, 1, data, request_nof_words, got_nof_words)  ;// MBLT64 !!!
	return return_code;
}


/**************************************************************************************/
int sis3150::vme_A32_2ESST320FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Read(_sis3150_device, addr,  0x260, 8, 1, data, request_nof_words, got_nof_words)  ;
	return return_code;
}

/***********************************************************************************************************/
int sis3150::vme_A32D32_write (uint32_t addr, uint32_t data)
{
int return_code ;
	return_code = sis3150Usb_Vme_Single_Write(_sis3150_device, addr, 0x9,4, data)  ;
    if(return_code)
    {
        printf("ERROR sis3150::vme_A32D32_write: Addr(0x%08x) Data(0x%08x)\n",addr,data);
    }

	return return_code;
}

int sis3150::vme_A32D16_write (uint32_t addr, uint16_t data)
{
    uint32_t data_32;
    data_32 = (uint32_t) data ;
    int return_code ;
	return_code = sis3150Usb_Vme_Single_Write(_sis3150_device, addr, 0x9,2, data_32)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32DMA_D32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Write( _sis3150_device, addr,0x9, 4, 0, data, request_nof_words, written_nof_words)  ;
	return return_code;
}
/**************************************************************************************/
int sis3150::vme_A32BLT32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Write( _sis3150_device, addr,0xb, 4, 0, data, request_nof_words, written_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32MBLT64_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Write( _sis3150_device, addr,0x8, 8, 0, data, request_nof_words, written_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32DMA_D32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Write( _sis3150_device, addr,0x9, 4, 1, data, request_nof_words, written_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32BLT32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Write( _sis3150_device, addr,0xb, 4, 1, data, request_nof_words, written_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int sis3150::vme_A32MBLT64FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 return_code = sis3150Usb_Vme_Dma_Write( _sis3150_device, addr,0x8, 8, 1, data, request_nof_words, written_nof_words)  ;
	return return_code;
}
