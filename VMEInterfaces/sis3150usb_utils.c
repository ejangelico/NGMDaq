/***************************************************************************/
/*                                                                         */
/*  Filename: sis3150usb_utils.c                                           */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 15.12.2004                                       */
/*  last modification:    01.03.2005 (DMA Write for MEMORY Clear)          */
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


/*===========================================================================*/
/* Headers								     */
/*===========================================================================*/
#include <stdio.h>

#include "sis3150usb_vme.h"
#include "sis3150usb_vme_calls.h" 

#define MAX_LOADER_FILE_LENGTH        0x0100000 

#define KEY_RESET_ALL 			0x0100 
#define KEY_CMC1_20ms_RESET 	0x0101 
#define KEY_CMC1_RESET 			0x0102 
#define KEY_TS_RESET 			0x0103 




//#define TSX_BROADCAST_BASE     	0x00000000 
#define TS1_BASE          		0x02000000 
#define TS2_BASE        		0x02400000 
#define TS_MEMORY       		0x0 







#define TS_REG_IMASKL   		0x018034C 
#define TS_REG_IMASKH   		0x018034D 
#define TS_REG_PMASKL   		0x018034E 
#define TS_REG_PMASKH   		0x018034F 

#define TS_REG_ILATCLL   		0x018035E 
#define TS_REG_ILATCLH   		0x018035F   /* !! */
//#define TS_REG_ILATCLH   		0x018034F   /* !! */


#define TS_REG_SQCTL	   		0x0180358   /* !! */
#define TS_REG_SQCTLST   		0x0180359   /* !! */
#define TS_REG_SQCTLCL   		0x018035A   /* !! */


#define TS_REG_SYSCON   		0x0180480 
#define TS_REG_SDRCON   		0x0180484 
#define TS_REG_VIRP     		0x0180730 

#define TS_REG_DMA_DC8     		0x0180440    
#define TS_REG_DMA_DC9	   		0x0180444    
#define TS_REG_DMA_DC10	   		0x0180448    
#define TS_REG_DMA_DC11	   		0x018044C    

#define TS_REG_LCTL0     					0x01804E0    
#define TS_REG_LCTL1     					0x01804E4    
#define TS_REG_LCTL2     					0x01804E8    
#define TS_REG_LCTL3     					0x01804EC     
  
/*===========================================================================*/
/* Prototypes					  			     */
/*===========================================================================*/




/*===========================================================================*/
/* Load DSP     					  		     */
/*===========================================================================*/

	

int usb_load_tigersharcs(HANDLE hDevice,  char* loaderfile_path)
{
	int error=0;
    int retcode=1;
    int count=0,loadcount=0;
    //UNUSED:int offset;
    //UNUSED:int currentaddress ;
    char line_in[1024] = "\n";
    FILE *loaderfile;
    unsigned int tempword[0x10000];
    //UNUSED:unsigned int read_tempword[0x10000]; // 
    unsigned int clear_pattern[0x10000];
    uint32_t data ;
    uint32_t addr ;
    uint32_t req_nof_lwords ;
    uint32_t got_nof_lwords ;

	int i;
    unsigned int  loader_header_word;
    unsigned int  loader_format_length;
    unsigned int  loader_format_addr;
    //UNUSED:unsigned int  ts_imaskl, ts_imaskh;
    //UNUSED:unsigned int  ts_pmaskl, ts_pmaskh;
    //UNUSED:unsigned int  timer;
    //UNUSED:unsigned int  clear_virp_error;


  /* Reset ??? */

 
 	for (i=0;i<0x10000;i++) {
	  clear_pattern[i] = 0 ;
 	}

  /* Reset TigerSharcs */
    addr =  KEY_TS_RESET   ;
	data =  0x0 ;
	req_nof_lwords = 1;
    if ((error = sis3150Usb_Register_Single_Write(hDevice, addr, data)) != 0) { 
        return -1;
	}
  



// SYSCON setup
//	data = 0x279e7 ;  default after reset

    data = 0x0 ;

    data = data + 0x27         ; // Bank 0 : slow (5), pipe = 0 (4:3), 3 wait cycles (2:1) , idle (0)
    data = data + (0x27 << 6)  ; // Bank 1 : slow (5), pipe = 0 (4:3), 3 wait cycles (2:1) , idle (0)
//  data = data + (0x27 << 12) ; // HOST   : slow (5), pipe = 0 (4:3), 3 wait cycles (2:1) , idle (0)
    data = data + (0x23 << 12) ; // HOST   : slow (5), pipe = 0 (4:3), 1 wait cycles (2:1) , idle (0)
//    data = data + (0x25 << 12) ; // HOST   : slow (5), pipe = 0 (4:3), 2 wait cycles (2:1) , idle (0)

    data = data + (0x1  << 19) ; // BUS Width MEM:    64 bit
    data = data + (0x1  << 20) ; // BUS Width Multi:  64 bit
    data = data + (0x1  << 21) ; // BUS Width HOST:   64 bit erstmal

	
	addr = TS1_BASE +  TS_REG_SYSCON  ;
	req_nof_lwords = 1;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { 
 		return -1;
	}

	addr = TS2_BASE +  TS_REG_SYSCON  ;
	req_nof_lwords = 1;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { 
 		return -1;
	}



// SDRCON SDRAM configuration 
//	data = 0 ;  default after reset ; SDRAM is disabled

    data = 0x0 ;
    data = data + 0x1         ; // SDRAM Enable
    data = data + (0x1 << 1)  ; // CAS Latency = 2 Cycles 
    data = data + (0x0 << 3)  ; // no pipe
    data = data + (0x1 << 4)  ; // page boundary = 512
    data = data + (0x0 << 6)  ; // reserved

    data = data + (0x1 << 7)  ; // Refresh rate every 900 cycles; 900 x 16 ns = 14,4 us (must min 15,6)
    data = data + (0x0 << 9)  ; // Row precharge (Trp min =20ns)  2 cycles x 16 ns = 32 ns
    data = data + (0x1 << 11) ; // Row active time (Tras min = 45ns)  3 cycles x 16ns 48ns
    data = data + (0x1 << 14) ; // INIT Sequence

	
	addr = TS1_BASE +  TS_REG_SDRCON  ;
	req_nof_lwords = 1;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { 
 		return -1;
	}
	addr = TS2_BASE +  TS_REG_SDRCON  ;
	req_nof_lwords = 1;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { 
 		return -1;
	}


	/* disable all Link Input DMA channels ; from TS_REG_DMA_DC8 to TS_REG_DMA_DC11 */ 
    data = 0x0 ;
	for (i=0;i<16;i++) {
		addr = TS1_BASE +  TS_REG_DMA_DC8 + (i) ;
		if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) {  
 			return -1;
		}
	}
	for (i=0;i<16;i++) {
		addr = TS2_BASE +  TS_REG_DMA_DC8 + (i) ;
		if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) {  
 			return -1;
		}
	}

  
	/* disable all Link Receive and Transmit  */ 
    data = 0x0 ;
	for (i=0;i<16;i++) {
		addr = TS1_BASE +  TS_REG_LCTL0 + (i) ;
		if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) {  
 			return -1;
		}
	}
	for (i=0;i<16;i++) {
		addr = TS2_BASE +  TS_REG_LCTL0 + (i) ;
		if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) {  
 			return -1;
		}
	}
  
  
  

    //usw
    loaderfile=fopen(loaderfile_path,"r");
    retcode = 1 ;
    if ((int)loaderfile>0) {
      // printf("loader file %s opened\n",loaderfile_path);
       while (retcode>0) {
          tempword[count]= strtoul(line_in,NULL,32); 
	      retcode=fscanf(loaderfile,"0x%8x\n",&tempword[count]); 
          if (count<MAX_LOADER_FILE_LENGTH) {
             count++;
          }
          else {
	  //   printf("load file size too big\n");
             return -1;
	      }
       }
     // printf("load file length: %d\n",count);
     }
    else {
    //  printf("loader file %s not found\n",dsppath);
      return -1;
     }
    fclose(loaderfile);


   // printf("loading SHARC DSP\n");


	req_nof_lwords = 1;
    loadcount=0 ;
    while (loadcount<(count-1)) {  
      loader_header_word = (tempword[loadcount] & 0xc0000000) ;
      switch (loader_header_word) {

	     case 0x0: {       // final init
			if ((tempword[loadcount] & 0x38000000) == 0x0) { // 
			  addr = TS1_BASE +  TS_MEMORY   ;
			} 
			else {
			  addr = TS2_BASE +  TS_MEMORY   ;
			} 


			loader_format_length = 0x100 ; // 256 word 			 
			loadcount++;
			if ((count-loadcount) < 0x100 ) {
				return -1;
			} 

#ifdef raus
			for (i=0;i<0x100;i++) {
				data = tempword[loadcount] ;
				if ((error = sis3150Usb_TsBus_Dma_Write(hDevice, addr+i, &data, req_nof_lwords, &got_nof_lwords)) != 0) { 
 					return -1;
				}
				loadcount++;
			}
#endif
			req_nof_lwords = 0x100;
			if ((error = sis3150Usb_TsBus_Dma_Write(hDevice, addr, (uint32_t*)&tempword[loadcount], req_nof_lwords, &got_nof_lwords)) != 0) { 
				return -1;
			}
			loadcount=loadcount+0x100;
			req_nof_lwords = 1;

          
          }
         break;  // case final init  

	     case 0x40000000: {       // write data to memory
			if ((tempword[loadcount] & 0x38000000) == 0x0) { // 
			  addr = TS1_BASE +  TS_MEMORY  ;
			} 
			else {
			  addr = TS2_BASE +  TS_MEMORY  ;
			} 
			loader_format_length = (tempword[loadcount] & 0xffff) ;			 
			loadcount++;
			loader_format_addr = (tempword[loadcount] ) ;			 
			loadcount++;
	
			if ((count-loadcount) < loader_format_length ) {
				return -1;
			} 

#ifdef raus
			for (i=0;i<loader_format_length;i++) {
				data = tempword[loadcount] ;
				if ((error = sis3150Usb_TsBus_Dma_Write(hDevice, addr+loader_format_addr+i, &data, req_nof_lwords, &got_nof_lwords)) != 0) { 
 					return -1;
				}
				loadcount++;
			}
#endif
			req_nof_lwords = loader_format_length;
			if ((error = sis3150Usb_TsBus_Dma_Write(hDevice, addr+loader_format_addr, (uint32_t*)&tempword[loadcount], req_nof_lwords, &got_nof_lwords)) != 0) { 
				return -1;
			}
			loadcount=loadcount+loader_format_length;
			req_nof_lwords = 1;

		  }
          break;   // case write data to memory


	    case 0x80000000: {       // clear memory
			if ((tempword[loadcount] & 0x38000000) == 0x0) { // 
			  addr = TS1_BASE +  TS_MEMORY  ;
			} 
			else {
			  addr = TS2_BASE +  TS_MEMORY  ;
			} 
			loader_format_length = (tempword[loadcount] & 0xffff) ;			 
			loadcount++;
			loader_format_addr = (tempword[loadcount] ) 		;	 
			loadcount++;
	
#ifdef raus
			for (i=0;i<loader_format_length;i++) {
				data = 0x0 ;
				if ((error = sis3150Usb_TsBus_Dma_Write(hDevice, addr+loader_format_addr+i, &data, req_nof_lwords, &got_nof_lwords)) != 0) { 
 					return -1;
				}
			}
#endif
			
			req_nof_lwords = loader_format_length;
			if ((error = sis3150Usb_TsBus_Dma_Write(hDevice, addr+loader_format_addr, (uint32_t*)clear_pattern, req_nof_lwords, &got_nof_lwords)) != 0) { 
				return -1;
			}
			req_nof_lwords = 1;
		      
		  
		  
		  }
          break;


	     case 0xC0000000: {       // not defined
           return -1 ;
           }
		   break;
        }    // end switch(loader_header_word)
		
 	} //while








// write VIRP Service routine to TS at address 0x10fff0
// VIRP service routine
//     0     01820088    41     xr0 = 0x00000201;;                  // set NMOD and BTB enable (1/24/00) bits in sequencer control register
//     1     321a0088    42     SQCTLST = xr0;;                     // mnemonic changed to SQCTLST (12/19/00)
//     2     400008b3    43     rds;;                               // reduce interrupt to subroutine level
//     3     000008b0    44     jump 0x0 (ABS) (NP);;               // jump to last patch

// 	data = 0x01820088 ;
 	data = 0x88008201  ;
	addr = TS1_BASE +  0x10fff0 ;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { return error;}
	addr = TS2_BASE +  0x10fff0 ;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { return error;}

// 	data = 0x321a0088 ;
 	data = 0x88001a32       ;
	addr = TS1_BASE +  0x10fff1 ;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { return error;}
	addr = TS2_BASE +  0x10fff1 ;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { return error;}

// 	data = 0x400008b3 ;
 	data = 0xb3080040     ;
	addr = TS1_BASE +  0x10fff2 ;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { return error;}
	addr = TS2_BASE +  0x10fff2 ;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { return error;}

// 	data = 0x000008b0 ;
 	data = 0xb0080000 ;
	addr = TS1_BASE +  0x10fff3 ;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { return error;}
	addr = TS2_BASE +  0x10fff3 ;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { return error;}

// start Tigerscharc
	data = 0x10fff0 ;
	addr = TS1_BASE +  TS_REG_VIRP ;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { return error;}
	addr = TS2_BASE +  TS_REG_VIRP ;
	if ((error = sis3150Usb_TsBus_Single_Write(hDevice, addr, data)) != 0) { return error;}








 return 0 ;
}























