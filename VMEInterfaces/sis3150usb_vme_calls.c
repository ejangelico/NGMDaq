/***************************************************************************/
/*                                                                         */
/*  Filename: sis3150usb_vme_calls.c                                       */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 15.12.2004                                       */
/*  modification:         08.03.2005                                       */
/*                        add vme_IACK_D8_read(...)                        */
/*                                                                         */
/*  last modification:    20.07.2010                                       */
/*                        add vme_A32DMA_D16_read                          */
/*                        add vme_A32BLT16_read                            */
/*                        add vme_A32DMA_D16_write                         */
/*                        add vme_A32BLT16_write                           */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  � 2010                                                                 */
/*                                                                         */
/***************************************************************************/



/*---------------------------------------------------------------------------*/
/* Include files                                                             */
/*---------------------------------------------------------------------------*/

#include <stdio.h>

    
#ifndef unix			/* only needed for windows. */
typedef unsigned char           u_int8_t;
typedef unsigned short          u_int16_t;
typedef unsigned long           u_int32_t;

#endif

typedef unsigned long           U32;

#include "sis3150usb_vme.h"
#include "sis3150usb_vme_calls.h"  



#ifdef DEBUG_TRACE
#define RETURN(code) if(code) {fprintf(stderr, "FAIL %s returning %x\n", __PRETTY_FUNCTION__, code); } \
                     return (code);
#else
#define RETURN(code) return (code)
#endif



/**********************/
/*                    */
/*    VME SYSReset    */
/*                    */
/**********************/

int vmesysreset(HANDLE  hXDev)
{
	sis3150Usb_VmeSysreset(hXDev)  ;
  RETURN(0) ;
}




/***************************/
/*                         */
/*    D08 IRQ Ackn. Cycle  */
/*                         */
/***************************/

/* VME Interrupt Acknowledge D08 Read Cycles */

int vme_IACK_D8_read(HANDLE  hXDev, u_int32_t vme_irq_level, u_int8_t* vme_data )
{
  uint32_t readdata ;
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Read(hXDev, ((vme_irq_level << 1)+1), 0x4000 /*AM register*/ ,1,&readdata)  ;
  if (return_code < 0)  {
    RETURN(return_code) ;
  }
  *vme_data = (u_int8_t) readdata; 
  RETURN(return_code) ;
}


/*****************/
/*               */
/*    VME A16    */
/*               */
/*****************/

/* VME A16  Read Cycles */

int vme_A16D8_read(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t* vme_data )
{
  uint32_t readdata ;
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Read(hXDev, vme_adr, 0x29,1, &readdata)  ;
  if (return_code < 0)  {
    RETURN(return_code) ;
  }
  *vme_data = (u_int8_t) readdata; 
  RETURN(return_code) ;
}


int vme_A16D16_read(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t* vme_data )
{
  uint32_t readdata ;
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Read(hXDev, vme_adr, 0x29,2, &readdata)  ;
  if (return_code < 0)  {
    RETURN(return_code) ;
  }
  *vme_data = (u_int16_t) readdata; 
  RETURN(return_code) ;
}


int vme_A16D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data )
{
  uint32_t readdata ;
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Read(hXDev, vme_adr, 0x29,4,&readdata)  ;
  if (return_code < 0)  {
    RETURN(return_code) ;
  }
  *vme_data = readdata; 
  RETURN(return_code) ;
}









/* VME A16  Write Cycles */
int vme_A16D8_write(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t vme_data )
{
  u_int32_t data_32 ;
  int return_code ;
  data_32 =  (u_int32_t) vme_data ;
  return_code = sis3150Usb_Vme_Single_Write(hXDev, vme_adr, 0x29, 1,  data_32) ;
  RETURN(return_code) ;
}

int vme_A16D16_write(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t vme_data )
{
  u_int32_t data_32 ;
  int return_code ;
  data_32 =  (u_int32_t) vme_data ;
  return_code = sis3150Usb_Vme_Single_Write(hXDev, vme_adr, 0x29, 2,  data_32) ;
  RETURN(return_code) ;
}


int vme_A16D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t vme_data )
{
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Write(hXDev, vme_adr, 0x29, 4,  vme_data) ;
  RETURN(return_code) ;
}













/*****************/
/*               */
/*    VME A24    */
/*               */
/*****************/

/* VME A24  Read Cycles */

int vme_A24D8_read(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t* vme_data )
{
  uint32_t readdata ;
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Read(hXDev, vme_adr, 0x39,1,&readdata)  ;
  if (return_code < 0)  {
    RETURN(return_code) ;
  }
  *vme_data = (u_int8_t) readdata; 
  RETURN(return_code) ;
}


int vme_A24D16_read(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t* vme_data )
{
  uint32_t readdata ;
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Read(hXDev, vme_adr, 0x39,2,&readdata)  ;
  if (return_code < 0)  {
    RETURN(return_code) ;
  }
  *vme_data = (u_int16_t) readdata; 
  RETURN(return_code) ;
}


int vme_A24D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data )
{
  uint32_t readdata ;
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Read(hXDev, vme_adr, 0x39,4,&readdata)  ;
  if (return_code < 0) {
    RETURN(return_code) ;
  }
  *vme_data = readdata; 
  RETURN(return_code) ;
}







int vme_A24DMA_D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x39, 4, 0,
					      (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)got_num_of_lwords);
   RETURN(return_code) ;
}


int vme_A24BLT32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x3b, 4, 0, 
					      (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)got_num_of_lwords);
   RETURN(return_code) ;
}


int vme_A24MBLT64_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x38, 8, 0, 
					      (uint32_t*)vme_data, req_num_of_lwords & 0xfffffffe, (uint32_t*)got_num_of_lwords);
   RETURN(return_code) ;
}





int vme_A24DMA_D32FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x39, 4, 1,
					      (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)got_num_of_lwords);
   RETURN( return_code) ;
}


int vme_A24BLT32FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x3b, 4, 1,
					      (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)got_num_of_lwords);
   RETURN(return_code) ;
}


int vme_A24MBLT64FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x38, 8, 1,
					      (uint32_t*)vme_data, req_num_of_lwords & 0xfffffffe, (uint32_t*)got_num_of_lwords);
   RETURN(return_code) ;
}











 

/* VME A24  Write Cycles */


int vme_A24D8_write(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t vme_data )
{
  u_int32_t data_32 ;
  int return_code ;
  data_32 =  (u_int32_t) vme_data ;
  return_code = sis3150Usb_Vme_Single_Write(hXDev, vme_adr, 0x39, 1,  data_32) ;
  RETURN(return_code) ;
}

int vme_A24D16_write(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t vme_data )
{
  u_int32_t data_32 ;
  int return_code ;
  data_32 =  (u_int32_t) vme_data ;
  return_code = sis3150Usb_Vme_Single_Write(hXDev, vme_adr, 0x39, 2,  data_32) ;
  RETURN(return_code) ;
}


int vme_A24D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t vme_data )
{
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Write(hXDev, vme_adr, 0x39, 4,  vme_data) ;
  RETURN(return_code) ;
}








int vme_A24DMA_D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x39/*am*/, 4 /*size*/, 0,
					       (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}



int vme_A24BLT32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x3b/*am*/, 4 /*size*/, 0,
					       (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}


int vme_A24MBLT64_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x38/*am*/, 8 /*size*/, 0,
					       (uint32_t*)vme_data, req_num_of_lwords & 0xfffffffe, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}




int vme_A24DMA_D32FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x39/*am*/, 4 /*size*/, 1,
					       (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}


int vme_A24BLT32FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x3b/*am*/, 4 /*size*/, 1,
					       (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}


int vme_A24MBLT64FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x38/*am*/, 8 /*size*/, 1,
					       (uint32_t*)vme_data, req_num_of_lwords & 0xfffffffe, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}




















/*****************/
/*               */
/*    VME A32    */
/*               */
/*****************/


/* VME A32  Read Cycles */


int vme_A32D8_read(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t* vme_data )
{
  uint32_t readdata ;
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Read(hXDev, vme_adr, 0x9,1,&readdata)  ;
  if (return_code < 0)  {
    RETURN(return_code) ;
  }
  *vme_data = (u_int8_t) readdata; 
  RETURN(return_code) ;
}


int vme_A32D16_read(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t* vme_data )
{
  uint32_t readdata ;
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Read(hXDev, vme_adr, 0x9,2,&readdata)  ;

  if (return_code < 0)  {
    RETURN(return_code) ;
  }
  *vme_data = (u_int16_t) readdata; 
  RETURN(return_code) ;
}


int vme_A32D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data )
{
  uint32_t readdata ;
  int return_code ;
  
  return_code = sis3150Usb_Vme_Single_Read(hXDev, vme_adr, 0x9,4,&readdata)  ;

  if (return_code < 0)  {
    RETURN(return_code) ;
  }
  *vme_data = readdata; 
  RETURN(return_code) ;
}



    








int vme_A32DMA_D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x9, 4, 0,
					      (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)got_num_of_lwords);
   RETURN(return_code );
}



int vme_A32BLT32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0xb, 4, 0,
					      (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)got_num_of_lwords);
   RETURN(return_code) ;
}


int vme_A32MBLT64_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x8, 8, 0,
					      (uint32_t*)vme_data, req_num_of_lwords & 0xfffffffe, (uint32_t*)got_num_of_lwords);
   RETURN(return_code );
}





int vme_A32DMA_D32FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x9, 4, 1,
					      (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)got_num_of_lwords);
   RETURN( return_code );
}


int vme_A32BLT32FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0xb, 4, 1,
					      (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)got_num_of_lwords);
   RETURN(return_code) ;
}


int vme_A32MBLT64FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords)
{
  int return_code ;
	return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x8, 8, 1,
					      (uint32_t*)vme_data, req_num_of_lwords & 0xfffffffe, (uint32_t*)got_num_of_lwords);
   RETURN(return_code) ;
}




// begin new 2010-07-20
		

int vme_A32DMA_D16_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_words, u_int32_t* got_num_of_words)
{
  int return_code ;
 //  printf(" :  sizeof(uint32_t) = 0x%08x\n", sizeof(uint32_t) );
 
  return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0x9/*am*/, 2 /*size*/, 0,
					      (unsigned int*) vme_data, req_num_of_words, (uint32_t*)got_num_of_words ) ;

					      
  return (return_code) ;
}



int vme_A32BLT16_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_words, u_int32_t* got_num_of_words)
{
  int return_code ;
  return_code = sis3150Usb_Vme_Dma_Read(hXDev, vme_adr, 0xb/*am*/, 2 /*size*/, 0,
					      (unsigned int*) vme_data, req_num_of_words, (uint32_t*)got_num_of_words ) ;     
  return (return_code) ;
}



// end new 2010-07-20
















/* VME A32  Write Cycles */


int vme_A32D8_write(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t vme_data )
{
  u_int32_t data_32 ;
  int return_code ;
  data_32 =  (u_int32_t) vme_data ;
  return_code = sis3150Usb_Vme_Single_Write(hXDev, vme_adr, 0x9, 1,  data_32) ;
  RETURN(return_code) ;
}

int vme_A32D16_write(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t vme_data )
{
  u_int32_t data_32 ;
  int return_code ;
  data_32 =  (u_int32_t) vme_data ;
  return_code = sis3150Usb_Vme_Single_Write(hXDev, vme_adr, 0x9, 2,  data_32) ;
  RETURN(return_code) ;
}


int vme_A32D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t vme_data )
{
  int return_code ;
  return_code = sis3150Usb_Vme_Single_Write(hXDev, vme_adr, 0x9, 4,  vme_data) ;
  RETURN(return_code) ;
}



 
int vme_A32DMA_D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x9/*am*/, 4 /*size*/, 0,
					       (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}



int vme_A32BLT32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0xb/*am*/, 4 /*size*/, 0,
					       (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)put_num_of_lwords ) ;
  RETURN( return_code) ;
}


int vme_A32MBLT64_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x8/*am*/, 8 /*size*/, 0,
					       (uint32_t*)vme_data, req_num_of_lwords & 0xfffffffe, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}










int vme_A32DMA_D32FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x9/*am*/, 4 /*size*/, 1,
					       (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}



int vme_A32BLT32FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0xb/*am*/, 4 /*size*/, 1,
					       (uint32_t*)vme_data, req_num_of_lwords, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}


int vme_A32MBLT64FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords)
{
  int return_code ;
  	return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x8/*am*/, 8 /*size*/, 1,
					       (uint32_t*)vme_data, req_num_of_lwords & 0xfffffffe, (uint32_t*)put_num_of_lwords ) ;
  RETURN(return_code) ;
}



// begin new 2010-07-20
int vme_A32DMA_D16_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_words, u_int32_t* put_num_of_words)
{
  int return_code ;
 //  printf(" :  sizeof(uint32_t) = 0x%08x\n", sizeof(uint32_t) );
 
  return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0x9/*am*/, 2 /*size*/, 0,
					     //  (uint32_t*)vme_data, req_num_of_words, (uint32_t*)put_num_of_words ) ;
					      (unsigned int*) vme_data, req_num_of_words, (uint32_t*)put_num_of_words ) ;


 
					      
  return (return_code) ;
}



int vme_A32BLT16_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_words, u_int32_t* put_num_of_words)
{
  int return_code ;
 //  printf(" :  sizeof(uint32_t) = 0x%08x\n", sizeof(uint32_t) );
 
  return_code = sis3150Usb_Vme_Dma_Write(hXDev, vme_adr, 0xb/*am*/, 2 /*size*/, 0,
					     //  (uint32_t*)vme_data, req_num_of_words, (uint32_t*)put_num_of_words ) ;
					      (unsigned int*) vme_data, req_num_of_words, (uint32_t*)put_num_of_words ) ;


 
					      
  return (return_code) ;
}

// end new 2010-07-20
