/***************************************************************************/
/*                                                                         */
/*  Filename: sis3150vme_calls.h                                           */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 15.12.2004                                       */
/*  modification:    08.03.2005                                            */
/*                        add vme_IACK_D8_read(...)                        */
/*                                                                         */
/*  last modification:    20.07.2010                                       */
/*                        add vme_A32DMA_D16_read                          */
/*                        add vme_A32BLT16_read                            */
/*                        add vme_A32DMA_D16_write                         */
/*                        add vme_A32BLT16_write                           */
/*                                                                         */
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
/*  � 2005                                                                 */
/*                                                                         */
/***************************************************************************/

#ifndef __SIS3150USBCALLS_H
#define __SIS3150USBCALLS_H

#include "sis3150usb_vme.h"


#ifdef __cplusplus
extern "C" {
#endif


/**********************/
/*                    */
/*    VME SYSReset    */
/*                    */
/**********************/

 
int vmesysreset(HANDLE  hXDev) ;


/***************************/
/*                         */
/*    D08 IRQ Ackn. Cycle  */
/*                         */
/***************************/

int vme_IACK_D8_read(HANDLE  hXDev, u_int32_t vme_irq_level, u_int8_t* vme_data ) ;


/*****************/
/*               */
/*    VME A16    */
/*               */
/*****************/

int vme_A16D8_read(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t* vme_data ) ;

int vme_A16D16_read(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t* vme_data ) ;
int vme_A16D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data ) ;

int vme_A16D8_write(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t vme_data ) ;
int vme_A16D16_write(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t vme_data ) ;
int vme_A16D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t vme_data ) ;





/*****************/
/*               */
/*    VME A24    */
/*               */
/*****************/


int vme_A24D8_read(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t* vme_data ) ;
int vme_A24D16_read(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t* vme_data ) ;
int vme_A24D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data ) ;


int vme_A24DMA_D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_no_of_lwords) ;

int vme_A24BLT32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_no_of_lwords) ;

int vme_A24MBLT64_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_no_of_lwords) ;



int vme_A24DMA_D32FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_no_of_lwords) ;

int vme_A24BLT32FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                          u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords) ;

int vme_A24MBLT64FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                          u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords) ;




int vme_A24D8_write(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t vme_data ) ;
int vme_A24D16_write(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t vme_data ) ;
int vme_A24D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t vme_data ) ;


int vme_A24DMA_D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;

int vme_A24BLT32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;


int vme_A24MBLT64_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;



int vme_A24DMA_D32FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;


int vme_A24BLT32FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;


int vme_A24MBLT64FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;








/*****************/
/*               */
/*    VME A32    */
/*               */
/*****************/


int vme_A32D8_read(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t* vme_data ) ;
int vme_A32D16_read(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t* vme_data ) ;
int vme_A32D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data ) ;



int vme_A32DMA_D32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_no_of_lwords) ;

int vme_A32BLT32_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_no_of_lwords) ;



int vme_A32MBLT64_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_no_of_lwords) ;





int vme_A32DMA_D32FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* got_no_of_lwords) ;

int vme_A32BLT32FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                          u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords) ;

int vme_A32MBLT64FIFO_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                          u_int32_t req_num_of_lwords, u_int32_t* got_num_of_lwords) ;


// begin new 2010-07-20
int vme_A32DMA_D16_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_words, u_int32_t* got_num_of_words);

int vme_A32BLT16_read(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_words, u_int32_t* got_num_of_words);


// end new 2010-07-20

int vme_A32D8_write(HANDLE  hXDev, u_int32_t vme_adr, u_int8_t vme_data ) ;
int vme_A32D16_write(HANDLE  hXDev, u_int32_t vme_adr, u_int16_t vme_data ) ;

int vme_A32D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t vme_data )		;



int vme_A32DMA_D32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;

int vme_A32BLT32_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;


int vme_A32MBLT64_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;




int vme_A32DMA_D32FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;


int vme_A32BLT32FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;


int vme_A32MBLT64FIFO_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_lwords, u_int32_t* put_num_of_lwords) ;


// begin new 2010-07-20
int vme_A32DMA_D16_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_words, u_int32_t* put_num_of_words);

int vme_A32BLT16_write(HANDLE  hXDev, u_int32_t vme_adr, u_int32_t* vme_data, 
                      u_int32_t req_num_of_words, u_int32_t* put_num_of_words);

// end new 2010-07-20

#ifdef __cplusplus
}
#endif
#endif
