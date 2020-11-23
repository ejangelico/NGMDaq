#include <iostream>
#include <cstdio>
#include "vme_api_en.h"
#include <tsi148_vmeinterface.h>
using namespace std;

ClassImp(tsi148_vmeinterface)

tsi148_vmeinterface::tsi148_vmeinterface()
{
    _devHandle = 0;
    _ctlHandle = 0;
}

int tsi148_vmeinterface::vmeopen ( void )
{
    // -- TSI148 VME Interface Setup
    
    /************************************
     *        init tsi148_vmeinterface
     ************************************/
	int error=0;
	int return_code ;

    _devHandle = vme_openDevice( "dma0" );
    _ctlHandle = vme_openDevice( "ctl" );
    vme_setInterruptMode(_ctlHandle,EN_INT_MODE_RORA);

    // //Enable byteSwapping from BigEndian VME to Little Endian x86
    // //Apparently not supported
    // return_code = vme_setByteSwap(_devHandle, 0x38);
    // if(return_code < 0){
    //     SysError("tsi148_vmeinterface::vmeopen","Error - failed to enable hardware byte swapping \n");
    // }


    uint32_t size = 0x800000;
    return_code =  vme_allocDmaBuffer( _devHandle, &size );
    if ( return_code < 0 )
    {
        SysError("tsi148_vmeinterface::vmeopen","Error - failed to allocate DMA buffer \n");
    }
    
	return 0;
}



int tsi148_vmeinterface::vmeclose( void ){
    //DMA buffer should be freed in closeDevice
    //vme_freeDmaBuffer(_devHandle);
    vme_closeDevice(_devHandle);
    vme_closeDevice(_ctlHandle);
    return 0;
}

/**************************************************************************************/

int tsi148_vmeinterface::get_vmeopen_messages( CHAR* messages, uint32_t* nof_found_devices ){
	return 0;
}
/**************************************************************************************/

int tsi148_vmeinterface::get_device_informations( USHORT* idVendor, USHORT* idProduct, USHORT* idSerNo, USHORT* idFirmwareVersion,  USHORT* idDriverVersion ){
	return 0;
}
/**************************************************************************************/

int tsi148_vmeinterface::vme_IRQ_Status_read ( uint32_t* data)
{
    int return_code=0 ;
    uint32_t read_val=0;
    EN_VME_INT_INFO info;
    int i;
    *data=0;
    for(info.intNum=1; info.intNum<=7; info.intNum++){
      //info.intNum = 1;
      return_code = vme_readInterruptInfo(_ctlHandle,&info);
      if ( return_code < 0 ){
	SysError("vme_IRQ_Status_read","Error - failed to read interrupt info)\n");
      }else{
	if(info.vecCount>0){
	  *data = (*data) | (1<<(info.intNum));
	  printf("Number of IRQ(%d) interrupts: %u\n",info.intNum, info.numOfInts);
	  for ( i = 0; i < info.vecCount; i++ ){
	    printf("%02d     0x%02X\n", i, info.vectors[i]);
	  }
	}
      }
    }

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
int tsi148_vmeinterface::vme_A32D32_read (uint32_t addr, uint32_t* data)
{
    int return_code=0 ;


    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_READ;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = 4096;  /* read 4KB */
    tdata.size = 4;  /* read 2 Words */
    tdata.vmeAddress = addr;
    
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 0;
    tdata.access.vmeCycle = 0;
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D32;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
    
    if ( return_code < 0 ){
        printf("Error - DMA transfer failed \n"
	       );
    }else{
	
	/* read and display the first 8 bytes of the buffer */
	return_code = vme_read( _devHandle, 0, (UINT8*) data, tdata.size );
	btol(data,tdata.size>>2);
    }
    if(return_code>0) return_code = 0;
    return return_code;
}
/**************************************************************************************/
int tsi148_vmeinterface::vme_A32D16_read (uint32_t addr, uint32_t* data)
{
    int return_code=0 ;

    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_READ;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = 2;  /* read 2 Words */
    tdata.vmeAddress = addr;
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 0;
    tdata.access.vmeCycle = 0;
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D16;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
    
    if ( return_code < 0 ){
        printf("Error - DMA transfer failed \n"
	       );
    }else{
	
	/* read and display the first 8 bytes of the buffer */
	return_code = vme_read( _devHandle, 0, (UINT8*) data, tdata.size );
	btol((unsigned short*)data,tdata.size);
    }

    return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32DMA_D32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Read(_tsi148_vmeinterface_device, addr,0x9, 4, 0, data, request_nof_words, got_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32BLT32_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
    int return_code=0 ;

    *got_nof_words=0;
    
    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_READ;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = request_nof_words*4; //In Bytes
    tdata.vmeAddress = addr;
    
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 0;
    tdata.access.vmeCycle = 1;
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D32;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
    
    if ( return_code < 0 ){
      SysError("vme_A32BLT32_read",
	       "ERROR(%d) tsi148_vmeinterface::vme_A32BLT32_read: Addr(0x%08x) Request(%d)\n",
	       return_code,addr,request_nof_words);
    }else{
      /* read and display the first 8 bytes of the buffer */
      return_code = vme_read( _devHandle, 0, (UINT8*) (data), tdata.size );
    }
	
    *got_nof_words = tdata.size/4;      

    
    btol(data,*got_nof_words);
   
    return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32MBLT64_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
    int return_code=0 ;

    *got_nof_words=0;
    
    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_READ;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = request_nof_words*4; //In Bytes
    tdata.vmeAddress = addr;
    
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 0;
    tdata.access.vmeCycle = 1;
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D32;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    printf("Attempting DMA add(0x%08x) PCI(%d) Size(%d) Requested(%d) Got(%d)\n",
	   tdata.vmeAddress, tdata.offset, tdata.size , request_nof_words, *got_nof_words);
    return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
    
    if ( return_code < 0 ){
      SysError("vme_A32MBLT64_read",
	       "ERROR(%d) tsi148_vmeinterface::vme_A32MBLT64_read: Addr(0x%08x) Request(%d)\n",
	       return_code,addr,request_nof_words);
    }else{
      
      /* read and display the first 8 bytes of the buffer */
      return_code = vme_read( _devHandle, 0, (UINT8*) (data), tdata.size );
    }
	
    *got_nof_words = tdata.size/4;      

    
    btol(data,*got_nof_words);
   
    return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32_2EVME_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Read(_tsi148_vmeinterface_device, addr,  0x8, 8, 0, data, request_nof_words, got_nof_words)  ;// MBLT64 !!!
	return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32_2ESST160_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Read(_tsi148_vmeinterface_device, addr,  0x8, 8, 0, data, request_nof_words, got_nof_words)  ;// MBLT64 !!!
	return return_code;
}


/**************************************************************************************/
int tsi148_vmeinterface::vme_A32_2ESST267_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Read(_tsi148_vmeinterface_device, addr,  0x8, 8, 0, data, request_nof_words, got_nof_words)  ;// MBLT64 !!!
	return return_code;
}


/**************************************************************************************/
int tsi148_vmeinterface::vme_A32_2ESST320_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Read(_tsi148_vmeinterface_device, addr,  0x8, 8, 0, data, request_nof_words, got_nof_words)  ; // MBLT64 !!!
	return return_code;
}




/**************************************************************************************/
int tsi148_vmeinterface::vme_A32DMA_D32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Read(_tsi148_vmeinterface_device, addr,0x9, 4, 1, data, request_nof_words, got_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32BLT32FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
    int return_code=0 ;
    const int blksize = (0x1FFFF0-0x100000);// In Bytes for SIS3316

    *got_nof_words=0;
    
    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_READ;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = blksize; //In Bytes
    tdata.vmeAddress = addr;
    
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 0;
    tdata.access.vmeCycle = 1;
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D32;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    while( (request_nof_words - *got_nof_words) > 0 && return_code >=0 ){

      if( (request_nof_words - *got_nof_words)*4 < blksize ){
	  tdata.size = 4*(request_nof_words - *got_nof_words);
      }

      return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
      
      if ( return_code < 0 ){
        SysError("vme_A32BLT32FIFO_read",
		 "ERROR(%d) tsi148_vmeinterface::vme_A32BLT32FIFO_read: Addr(0x%08x) Request(%d)\n",
		 return_code,addr,request_nof_words);
      }else{
	return_code = vme_read( _devHandle, 0, (UINT8*) (&(data[*got_nof_words])), tdata.size );
      }
	
      *got_nof_words += tdata.size/4;      
    }
    
    btol(data,*got_nof_words);
   
    return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32MBLT64FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
    int return_code=0 ;
    const int blksize = (0x1FFFF0-0x100000);// In Bytes for SIS3316

    *got_nof_words=0;
    
    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_READ;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = blksize; //In Bytes
    tdata.vmeAddress = addr;
    
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 0;
    tdata.access.vmeCycle = 2; //MBLT
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D32;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    while( (request_nof_words - *got_nof_words) > 0 && return_code >=0 ){

      if( (request_nof_words - *got_nof_words)*4 < blksize ){
	  tdata.size = 4*(request_nof_words - *got_nof_words);
	  //printf("Setting size to %d(%d)\n",tdata.size,4*(request_nof_words - *got_nof_words));
      }

      //printf("Attempting DMA add(0x%08x) PCI(%d) Size(%d) Requested(%d) Got(%d)\n",
      //	     tdata.vmeAddress, tdata.offset, tdata.size , request_nof_words, *got_nof_words);
      return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
      
      if ( return_code < 0 ){
        SysError("vme_A32MBLT64FIFO_read",
		 "ERROR(%d) tsi148_vmeinterface::vme_A32MBLT64FIFO_read: Addr(0x%08x) Request(%d)\n",
		 return_code,addr,request_nof_words);
      }else{
      
	/* read and display the first 8 bytes of the buffer */
	return_code = vme_read( _devHandle, 0, (UINT8*) (&(data[*got_nof_words])), tdata.size );
      }
	
      *got_nof_words += tdata.size/4;      
    }
    
    btol(data,*got_nof_words);
   
    return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32_2EVMEFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Read(_tsi148_vmeinterface_device, addr,  0x8, 8, 1, data, request_nof_words, got_nof_words)  ;// MBLT64 !!!
	return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32_2ESST160FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
    int return_code=0 ;
    const int blksize = (0x1FFFF0-0x100000);// In Bytes for SIS3316

    *got_nof_words=0;
    
    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_READ;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = blksize;
    tdata.vmeAddress = addr;
    
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 0;
    tdata.access.vmeCycle = 4; //2eSST
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D32;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    while( (request_nof_words - *got_nof_words) > 0 && return_code >=0 ){

      if( (request_nof_words - *got_nof_words)*4 < blksize ){
	  tdata.size = 4*(request_nof_words - *got_nof_words);
	  //printf("Setting size to %d(%d)\n",tdata.size,4*(request_nof_words - *got_nof_words));
      }

      //printf("Attempting DMA add(0x%08x) PCI(%d) Size(%d) Requested(%d) Got(%d)\n",
      //	     tdata.vmeAddress, tdata.offset, tdata.size , request_nof_words, *got_nof_words);
      return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
      
      if ( return_code < 0 ){
        SysError("vme_A32FIFO_read",
		 "ERROR(%d) tsi148_vmeinterface::vme_A32BLT64FIFO_read: Addr(0x%08x) Request(%d)\n",
		 return_code,addr,request_nof_words);
      }else{
      
	/* read and display the first 8 bytes of the buffer */
	return_code = vme_read( _devHandle, 0, (UINT8*) (&(data[*got_nof_words])), tdata.size );
      }
	
      *got_nof_words += tdata.size/4;      
    }
    
    btol(data,*got_nof_words);
   
    return return_code;
}


/**************************************************************************************/
int tsi148_vmeinterface::vme_A32_2ESST267FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
    int return_code=0 ;
    const int blksize = (0x1FFFF0-0x100000);// In Bytes for SIS3316

    *got_nof_words=0;
    
    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_READ;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = blksize;
    tdata.vmeAddress = addr;
    
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 1;
    tdata.access.vmeCycle = 4; //2eSST
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D32;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    while( (request_nof_words - *got_nof_words) > 0 && return_code >=0 ){

      if( (request_nof_words - *got_nof_words)*4 < blksize ){
	  tdata.size = 4*(request_nof_words - *got_nof_words);
	  //printf("Setting size to %d(%d)\n",tdata.size,4*(request_nof_words - *got_nof_words));
      }

      //printf("Attempting DMA add(0x%08x) PCI(%d) Size(%d) Requested(%d) Got(%d)\n",
      //	     tdata.vmeAddress, tdata.offset, tdata.size , request_nof_words, *got_nof_words);
      return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
      
      if ( return_code < 0 ){
        SysError("vme_A32_2ESST267FIFO_read",
		 "ERROR(%d) tsi148_vmeinterface::vme_A32_2ESST267FIFO_read: Addr(0x%08x) Request(%d)\n",
		 return_code,addr,request_nof_words);
      }else{
      
	/* read and display the first 8 bytes of the buffer */
	return_code = vme_read( _devHandle, 0, (UINT8*) (&(data[*got_nof_words])), tdata.size );
      } 
	
      *got_nof_words += tdata.size/4;      
    }
    
    btol(data,*got_nof_words);
   
    return return_code;
}


/**************************************************************************************/
int tsi148_vmeinterface::vme_A32_2ESST320FIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
    int return_code=0 ;
    const int blksize = (0x1FFFF0-0x100000);// In Bytes for SIS3316

    *got_nof_words=0;
    
    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_READ;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = blksize;
    tdata.vmeAddress = addr;
    
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 2;
    tdata.access.vmeCycle = 4; //2eSST
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D32;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    while( (request_nof_words - *got_nof_words) > 0 && return_code >=0 ){

      if( (request_nof_words - *got_nof_words)*4 < blksize ){
	  tdata.size = 4*(request_nof_words - *got_nof_words);
	  //printf("Setting size to %d(%d)\n",tdata.size,4*(request_nof_words - *got_nof_words));
      }

      //printf("Attempting DMA add(0x%08x) PCI(%d) Size(%d) Requested(%d) Got(%d)\n",
      //	     tdata.vmeAddress, tdata.offset, tdata.size , request_nof_words, *got_nof_words);
      return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
      
      if ( return_code < 0 ){
        SysError("vme_A32_2ESST320FIFO_read",
		 "ERROR(%d) tsi148_vmeinterface::vme_A32_2ESST320FIFO_read: Addr(0x%08x) Request(%d)\n",
		 return_code,addr,request_nof_words);
      }else{
      
	/* read and display the first 8 bytes of the buffer */
	return_code = vme_read( _devHandle, 0, (UINT8*) (&(data[*got_nof_words])), tdata.size );
      } 
	
      *got_nof_words += tdata.size/4;      
    }
    
    btol(data,*got_nof_words);
   
    return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32_FastestFIFO_read (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* got_nof_words )
{
  return vme_A32MBLT64FIFO_read(addr,data,request_nof_words,got_nof_words);
}
/***********************************************************************************************************/
int tsi148_vmeinterface::vme_A32D32_write (uint32_t addr, uint32_t data)
{
    int return_code=0 ;

    btol(&data,1);

    return_code = vme_write( _devHandle, 0, (UINT8*) (&data), 4);
    
    if(return_code<0){
      SysError("vme_A32D32_write","vme_write error");
      return return_code;
    }

    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_WRITE;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = 4;  /* write 1 Words */
    tdata.vmeAddress = addr;
    
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 0;
    tdata.access.vmeCycle = 0;
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D32;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
    
    if ( return_code < 0 ){
        printf("Error - DMA transfer failed \n"
	       );
    }else{
        //printf("DMA transfer successful\n");
    }

    if(return_code)
    {
        printf("ERROR tsi148_vmeinterface::vme_A32D32_write: Addr(0x%08x) Data(0x%08x)\n",addr,data);
    }

    return return_code;
}

int tsi148_vmeinterface::vme_A32D16_write (uint32_t addr, uint16_t data)
{

    int return_code=0 ;

    btol(&data,1);

    return_code = vme_write( _devHandle, 0, (UINT8*) (&data), 4);
    
    if(return_code<0){
      SysError("vme_A32D32_write","vme_write error");
      return return_code;
    }

    EN_VME_DIRECT_TXFER tdata;
    tdata.direction = EN_DMA_WRITE;
    tdata.offset = 0;       /* start of DMA buffer */
    tdata.size = 2;  /* write 2 Bytes */
    tdata.vmeAddress = addr;
    
    tdata.txfer.timeout = 200;  /* 2 second timeout */
    tdata.txfer.vmeBlkSize = TSI148_4096;
    tdata.txfer.vmeBackOffTimer = 0;
    tdata.txfer.pciBlkSize = TSI148_4096;
    tdata.txfer.pciBackOffTimer = 0;		
    
    tdata.access.sstMode = 0;
    tdata.access.vmeCycle = 0;
    tdata.access.sstbSel = 0;
    tdata.access.dataWidth = EN_VME_D16;
    tdata.access.addrSpace = EN_VME_A32;
    tdata.access.type = EN_LSI_DATA;   /* data AM code */
    tdata.access.mode = EN_LSI_USER;   /* non-privileged */
    
    return_code = vme_dmaDirectTransfer(_devHandle, &tdata)  ;
    
    if ( return_code < 0 ){
        printf("Error - DMA transfer failed \n"
	       );
    }else{
        //printf("DMA transfer successful\n");
    }

    if(return_code)
    {
        printf("ERROR tsi148_vmeinterface::vme_A32D32_write: Addr(0x%08x) Data(0x%08x)\n",addr,data);
    }

    return return_code;

}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32DMA_D32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Write( _tsi148_vmeinterface_device, addr,0x9, 4, 0, data, request_nof_words, written_nof_words)  ;
	return return_code;
}
/**************************************************************************************/
int tsi148_vmeinterface::vme_A32BLT32_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Write( _tsi148_vmeinterface_device, addr,0xb, 4, 0, data, request_nof_words, written_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32MBLT64_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Write( _tsi148_vmeinterface_device, addr,0x8, 8, 0, data, request_nof_words, written_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32DMA_D32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Write( _tsi148_vmeinterface_device, addr,0x9, 4, 1, data, request_nof_words, written_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32BLT32FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Write( _tsi148_vmeinterface_device, addr,0xb, 4, 1, data, request_nof_words, written_nof_words)  ;
	return return_code;
}

/**************************************************************************************/
int tsi148_vmeinterface::vme_A32MBLT64FIFO_write (uint32_t addr, uint32_t* data, uint32_t request_nof_words, uint32_t* written_nof_words )
{
int return_code=0 ;
	 //return_code = tsi148_vmeinterfaceUsb_Vme_Dma_Write( _tsi148_vmeinterface_device, addr,0x8, 8, 1, data, request_nof_words, written_nof_words)  ;
	return return_code;
}
