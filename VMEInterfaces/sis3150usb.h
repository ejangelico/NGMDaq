/***************************************************************************/
/*                                                                         */
/*  Filename: sis3150usb.h                                                 */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 10.10.2004                                       */
/*  last modification:    16.02.2005                                       */
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
/*  © 2005                                                                 */
/*                                                                         */
/***************************************************************************/


#define SIS3150USB_CMC1_BASE      		0x08000000 
#define SIS3150USB_SDRAM_BASE     		0x04000000 


#define SIS3150USB_TS1_BASE  			0x02000000 
#define SIS3150USB_TS2_BASE  			0x02400000 

/* DSP TigerSharc internal Memory Spaces */
#define SIS3150USB_TS_MEMORY_BLOCK0   	0x0 
#define SIS3150USB_TS_MEMORY_BLOCK1   	0x80000 
#define SIS3150USB_TS_MEMORY_BLOCK2   	0x100000  

  
//#define TS2_BASE        		0x03000000 
//#define TS_MEMORY       		0x0 

#define SIS3150USB_CONTROL_STATUS				0x0	 
#define SIS3150USB_MODID_VERSION				0x1	 
#define SIS3150USB_TS_LINK_CONNECT_REG			0x2	 
#define SIS3150USB_LEMO_OUT_SELECT_REG			0x3	 


#define SIS3150USB_VME_MASTER_CONTROL_STATUS	0x10	 
#define SIS3150USB_VME_MASTER_CYCLE_STATUS		0x11	 
#define SIS3150USB_VME_INTERRUPT_STATUS			0x12


#define SIS3150USB_JTAG_TEST		0x20	 
#define SIS3150USB_JTAG_DATA_IN		0x20	 
#define SIS3150USB_JTAG_CONTROL		0x21	 


#define SIS3150USB_KEY_RESET_ALL 				0x0100 
#define SIS3150USB_KEY_CMC1_20ms_RESET 			0x0101 
#define SIS3150USB_KEY_CMC1_RESET 				0x0102 
#define SIS3150USB_KEY_TS_RESET 				0x0103 


#define KEY_RESET_ALL 			0x0100 
#define KEY_CMC1_20ms_RESET 	0x0101 
#define KEY_CMC1_RESET 			0x0102 
#define KEY_TS_RESET 			0x0103 
