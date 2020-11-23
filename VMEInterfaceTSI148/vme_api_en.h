/******************************************************************************
*
* Filename: 	vme_api_en.h
* 
* Description:	Header file for VME enhanced application programmers interface
*
* $Revision: 1.7 $
*
* $Date: 2012/07/26 15:52:53 $
*
* $Source: /home/cvs/cvsroot/Linuxvme4/linuxvmeen/vme_api_en.h,v $
*
* Copyright 2000-2005 Concurrent Technologies.
*
******************************************************************************/

#ifndef __INCvme_api_en
#define __INCvme_api_en

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*#ifndef __KERNEL__*/

/* Universe II Register Offsets */

#define PCI_ID		0x0000
#define PCI_CSR		0x0004
#define PCI_CLASS	0x0008
#define PCI_MISC0	0x000C
#define PCI_BS0		0x0010
#define PCI_BS1		0x0014
#define PCI_MISC1	0x003C

#define LSI0_CTL	0x0100
#define LSI0_BS		0x0104
#define LSI0_BD		0x0108
#define LSI0_TO		0x010C

#define LSI1_CTL	0x0114
#define LSI1_BS		0x0118
#define LSI1_BD		0x011C
#define LSI1_TO		0x0120

#define LSI2_CTL	0x0128
#define LSI2_BS		0x012C
#define LSI2_BD		0x0130
#define LSI2_TO		0x0134

#define LSI3_CTL	0x013C
#define LSI3_BS		0x0140
#define LSI3_BD		0x0144
#define LSI3_TO		0x0148

#define SCYC_CTL	0x0170
#define SCYC_ADDR	0x0174
#define SCYC_EN		0x0178
#define SCYC_CMP	0x017C
#define SCYC_SWP	0x0180
#define LMISC		0x0184
#define SLSI		0x0188
#define L_CMDERR	0x018C
#define LAERR		0x0190

#define LSI4_CTL	0x01A0
#define LSI4_BS		0x01A4
#define LSI4_BD		0x01A8
#define LSI4_TO		0x01AC

#define LSI5_CTL	0x01B4
#define LSI5_BS		0x01B8
#define LSI5_BD		0x01BC
#define LSI5_TO		0x01C0

#define LSI6_CTL	0x01C8
#define LSI6_BS		0x01CC
#define LSI6_BD		0x01D0
#define LSI6_TO		0x01D4

#define LSI7_CTL	0x01DC
#define LSI7_BS		0x01E0
#define LSI7_BD		0x01E4
#define LSI7_TO		0x01E8

#define DCTL		0x0200
#define DTBC		0x0204
#define DLA			0x0208
#define DVA			0x0210
#define DCPP		0x0218
#define DGCS		0x0220
#define D_LLUE		0x0224

#define LINT_EN		0x0300
#define LINT_STAT	0x0304
#define LINT_MAP0	0x0308
#define LINT_MAP1	0x030C
#define VINT_EN		0x0310
#define VINT_STAT	0x0314
#define VINT_MAP0	0x0318
#define VINT_MAP1	0x031C
#define STATID		0x0320
#define V1_STATID	0x0324
#define V2_STATID	0x0328
#define V3_STATID	0x032C
#define V4_STATID	0x0330
#define V5_STATID	0x0334
#define V6_STATID	0x0338
#define V7_STATID	0x033C

#define LINT_MAP2	0x340
#define VINT_MAP2	0x344
#define MBOX0		0x348
#define MBOX1		0x34C
#define MBOX2		0x350
#define MBOX3		0x354
#define SEMA0		0x358
#define SEMA1		0x35C

#define MAST_CTL	0x0400
#define MISC_CTL	0x0404
#define MISC_STAT	0x0408
#define USER_AM		0x040C

#define VSI0_CTL	0x0F00
#define VSI0_BS		0x0F04
#define VSI0_BD		0x0F08
#define VSI0_TO		0x0F0C

#define VSI1_CTL	0x0F14
#define VSI1_BS		0x0F18
#define VSI1_BD		0x0F1C
#define VSI1_TO		0x0F20

#define VSI2_CTL	0x0F28
#define VSI2_BS		0x0F2C
#define VSI2_BD		0x0F30
#define VSI2_TO		0x0F34

#define VSI3_CTL	0x0F3C
#define VSI3_BS		0x0F40
#define VSI3_BD		0x0F44
#define VSI3_TO		0x0F48

#define LM_CTL		0xF64
#define LM_BS		0xF68

#define VRAI_CTL	0x0F70
#define VRAI_BS		0x0F74
#define VCSR_CTL	0x0F80
#define VCSR_TO		0x0F84
#define V_AMERR		0x0F88
#define VAERR		0x0F8C

#define VSI4_CTL	0x0F90
#define VSI4_BS		0x0F94
#define VSI4_BD		0x0F98
#define VSI4_TO		0x0F9C

#define VSI5_CTL	0x0FA4
#define VSI5_BS		0x0FA8
#define VSI5_BD		0x0FAC
#define VSI5_TO		0x0FB0

#define VSI6_CTL	0x0FB8
#define VSI6_BS		0x0FBC
#define VSI6_BD		0x0FC0
#define VSI6_TO		0x0FC4

#define VSI7_CTL	0x0FCC
#define VSI7_BS		0x0FD0
#define VSI7_BD		0x0FD4
#define VSI7_TO		0x0FD8

#define VCSR_CLR	0x0FF4
#define VCSR_SET	0x0FF8
#define VCSR_BS		0x0FFC


/* TSI148 Register Offset */

#define OTAT0		0x011C
#define OTAT1		0x013C
#define OTAT2		0x015C
#define OTAT3		0x017C
#define OTAT4		0x019C
#define OTAT5		0x01BC
#define OTAT6		0x01DC
#define OTAT7		0x01FC

#define VIACK1		0x0204
#define VIACK2		0x0208
#define VIACK3		0x020C
#define VIACK4		0x0210
#define VIACK5		0x0214
#define VIACK6		0x0218
#define VIACK7		0x021C

#define RMWAU		0x0220
#define RMWAL		0x0224
#define RMWEN		0x0228
#define RMWC		0x022C
#define RMWS		0x0230

#define VMCTRL		0x0234
#define VCTRL		0x0238
#define VSTAT		0x023C
#define VEAU		0x0260
#define VEAL		0x0264
#define VEAT		0x0268

#define ITAT0		0x0318
#define ITAT1		0x0338
#define ITAT2		0x0358
#define ITAT3		0x0378
#define ITAT4		0x0398
#define ITAT5		0x03B8
#define ITAT6		0x03D8
#define ITAT7		0x03F8

#define CBAU		0x040C
#define CBAL		0x0410
#define CRGAT		0x0414

#define CROU		0x0418
#define CROL		0x041C
#define CRAT		0x0420

#define LMBAU		0x0424
#define LMBAL		0x0428
#define LMAT		0x042C

#define VICR		0x0440
#define INTEN		0x0448
#define INTEO		0x044C
#define INTS		0x0450
#define INTC		0x0454
#define INTM1		0x0458
#define INTM2		0x045C

#define DCTL0		0x0500
#define DCTL1		0x0580
#define DCSTA0		0x0504
#define DCSTA1		0x0584
#define DSAU0		0x0520
#define DSAU1		0x05A0
#define DSAL0		0x0524
#define DSAL1		0x05A4
#define DDAU0		0x0528
#define DDAU1		0x05A8
#define DDAL0		0x052C
#define DDAL1		0x05AC
#define DSAT0		0x0530
#define DSAT1		0x05B0
#define DDAT0		0x0534
#define DDAT1		0x05B4
#define DNLAU0		0x0538
#define DNLAU1		0x05B8
#define DNLAL0		0x053C
#define DNLAL1		0x05BC
#define DCNT0 		0x0540
#define DCNT1		0x05C0
#define DDBS0		0x0544
#define DDBS1		0x05C4
#define SEMAR0		0x060C
#define SEMAR1		0x0608
#define TSIMBOX0	0x0610
#define TSIMBOX1	0x0614
#define TSIMBOX2	0x0618
#define TSIMBOX3	0x061C

#define CSRBCR		0x0FF4

/* TSI148 Bit settings */
#define TSI148_VEAT_VESCL (0x1 << 29)
#define TSI148_VSTAT_BDFAIL ( 0x1 << 14 )

/*#endif*/ /* __KERNEL__ */

/* EN DMA Constants */
#define EN_DMA_READ    0
#define EN_DMA_WRITE   1

/* VME bus constants */
#define EN_VME_D8 		0x0
#define EN_VME_D16 		0x1
#define EN_VME_D32 		0x2
#define EN_VME_D64 		0x3
#define EN_VME_A16		0x0
#define EN_VME_A24 		0x1
#define EN_VME_A32 		0x2
#define EN_VME_A64		0x4
#define EN_VME_CRCSR	        0x5
#define EN_VME_USER1_AM_UNV     0x6 	/* User 1 AM for Universe */
#define EN_VME_USER2_AM_UNV     0x7	/* User 2 AM for Universe */
#define EN_VME_USER1_AM_TSI 	0x8	/* User 1 AM for TSI148   */
#define EN_VME_USER2_AM_TSI 	0x9	/* User 2 AM for TSI148   */
#define EN_VME_USER3_AM_TSI 	0xa	/* User 3 AM for TSI148   */
#define EN_VME_USER4_AM_TSI 	0xb	/* User 4 AM for TSI148   */

#define EN_LSI_DATA		0
#define EN_LSI_PGM		1
#define EN_LSI_USER		0
#define EN_LSI_SUPER	        1

#define EN_VSI_DATA		1
#define EN_VSI_PGM		2
#define EN_VSI_USER		1
#define EN_VSI_SUPER	        2
#define EN_VSI_BOTH		3


/* Byte Swap constants */
#define EN_SWAP_MASTER	0x08
#define EN_SWAP_SLAVE	0x10
#define EN_SWAP_FAST	0x20
#define EN_SWAP_MASK	(EN_SWAP_MASTER | EN_SWAP_SLAVE | EN_SWAP_FAST)
#define EN_SWAP_PORT	0x210

/* Interrupt Modes */
#define EN_INT_MODE_ROAK 0
#define EN_INT_MODE_RORA 1

/* Board Capabality flags */
#define EN_VME_BYTE_SWAP	0x00000008
#define EN_VME_SCT		0x00000010
#define EN_VME_BLT		0x00000020	
#define EN_VME_MBLT		0x00000040
#define EN_VME_2eVME		0x00000080
#define EN_VME_2eSST160		0x00000100
#define EN_VME_2eSST267		0x00000200
#define EN_VME_2eSST320		0x00000400
#define EN_VME_2eSSTB		0x00000800
#define EN_VME_UNIVERSE		( EN_VME_SCT | EN_VME_BLT | EN_VME_MBLT )
#define EN_VME_ALL_PROTOCOL	(EN_VME_SCT | EN_VME_BLT | EN_VME_MBLT |  \
				 EN_VME_2eVME | EN_VME_2eSST160 | EN_VME_2eSST267 | EN_VME_2eSST320 | \
				 EN_VME_2eSSTB)

/* Ioctl Functions */
enum ioctl_en_nums
{
	IOCTL_EN_GET_POS = 32,
	IOCTL_EN_ENABLE_INT,
	IOCTL_EN_DISABLE_INT,
	IOCTL_EN_ENABLE_PCI_IMAGE,
	IOCTL_EN_DISABLE_PCI_IMAGE,
	IOCTL_EN_ENABLE_VME_IMAGE,
	IOCTL_EN_DISABLE_VME_IMAGE,
	IOCTL_EN_SET_STATUSID,
	IOCTL_EN_GET_VME_INT_INFO,
	IOCTL_EN_ENABLE_REG_IMAGE,
	IOCTL_EN_DISABLE_REG_IMAGE,
	IOCTL_EN_ENABLE_CSR_IMAGE,
	IOCTL_EN_DISABLE_CSR_IMAGE,
	IOCTL_EN_ENABLE_LM_IMAGE,
	IOCTL_EN_DISABLE_LM_IMAGE,
	IOCTL_EN_ALLOC_DMA_BUFFER,
	IOCTL_EN_FREE_DMA_BUFFER,
	IOCTL_EN_DMA_DIRECT_TXFER,
	IOCTL_EN_DMA_ADD_CMD_PKT,
	IOCTL_EN_DMA_CLEAR_CMD_PKTS,
	IOCTL_EN_DMA_LIST_TXFER,
	IOCTL_EN_WAIT_LINT,
        IOCTL_EN_SET_USER_AM_CODES,
	IOCTL_EN_SET_BYTE_SWAP,
	IOCTL_EN_CLEAR_STATS,
#ifdef  LINUXOS
	IOCTL_EN_RESERVE_MEMORY,
#endif
	IOCTL_EN_GET_VERR_INFO,
	IOCTL_EN_SET_INT_MODE,
	IOCTL_EN_GET_VME_EXTINT_INFO,
	IOCTL_EN_GENERATE_INT,
	IOCTL_EN_REGISTER_INT,
	IOCTL_EN_REMOVE_INT,
	IOCTL_EN_SEM_GET,
	IOCTL_EN_SEM_REL,
	IOCTL_EN_GET_STATS,
	IOCTL_EN_GET_CAPABALITY,
	IOCTL_EN_GET_DMA_BUFFER_ADDR,
	IOCTL_EN_VME_IMAGE_ADDR,
	IOCTL_EN_PCI_IMAGE_ADDR,
	IOCTL_GET_INST_COUNT,
	IOCTL_GET_GEO_ADDR,
};

/* Interrupt Numbers */
enum en_int_nums
{
  EN_RESERVED1_VOWN =0,  /* RESERVED1(Tsi148) or VOWN(UniverseII) */
  EN_IRQ1,               
  EN_IRQ2,
  EN_IRQ3,
  EN_IRQ4,
  EN_IRQ5,
  EN_IRQ6,
  EN_IRQ7,
  EN_ACFAIL,
  EN_SYSFAIL,
  EN_IACK,
  EN_VIE_SWINT,         /* VIE(Tsi148) or SWINT(UniverseII) */ 
  EN_VERR,
  EN_PERR,
  EN_RESERVED2,
  EN_RESERVED3,
  EN_MBOX0,
  EN_MBOX1,
  EN_MBOX2,
  EN_MBOX3,
  EN_LM0,
  EN_LM1,
  EN_LM2,
  EN_LM3,
  EN_DMA0,
  EN_DMA1,             /* Supported only on Tsi148 */ 
};

/* Status information codes */
enum vme_stats
{
	VME_STATUS_CTRL=0,
        VME_STATUS_LSI0,
        VME_STATUS_LSI1,
        VME_STATUS_LSI2,
        VME_STATUS_LSI3,
        VME_STATUS_LSI4,
        VME_STATUS_LSI5,
        VME_STATUS_LSI6,
        VME_STATUS_LSI7,
        VME_STATUS_VSI0,
        VME_STATUS_VSI1,
        VME_STATUS_VSI2,
        VME_STATUS_VSI3,
        VME_STATUS_VSI4,
        VME_STATUS_VSI5,
        VME_STATUS_VSI6,
        VME_STATUS_VSI7,
        VME_STATUS_DMA0,
        VME_STATUS_DMA1,
        VME_STATUS_INTS,
};


/* TSI148 Specific enumerators */
enum tsi148sstModes {
	TSI148_SST160=0,
	TSI148_SST267,
	TSI148_SST320
};

enum tsi148VmeCycles {
	TSI148_SCT=0,
	TSI148_BLT,
	TSI148_MBLT,
	TSI148_2eVME,
	TSI148_2eSST,
	TSI148_2eSSTB
};	

enum tsi148DmaBlkSize {
	TSI148_32=0,
	TSI148_64,
	TSI148_128,
	TSI148_256,
	TSI148_512,
	TSI148_1024,
	TSI148_2048,
	TSI148_4096
};

enum tsi148DmaBackOff {
	TSI148_1US=1,
	TSI148_2US,
	TSI148_4US,
	TSI148_8US,
	TSI148_16US,
	TSI148_32US,
	TSI148_64US,
};


/* Type Definitions */
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;
typedef unsigned long   ULONG;
typedef char    INT8;
typedef short   INT16;
typedef int             INT32;

/*Image numbers for the kernel APIs*/
#ifdef __KERNEL__
#define VME_CONTROL       0       
#define VME_LSI0          1
#define VME_LSI1          2
#define VME_LSI2          3
#define VME_LSI3          4
#define VME_LSI4          5
#define VME_LSI5          6
#define VME_LSI6          7
#define VME_LSI7          8
#define VME_VSI0          9
#define VME_VSI1          10
#define VME_VSI2          11
#define VME_VSI3          12
#define VME_VSI4          13
#define VME_VSI5          14
#define VME_VSI6          15
#define VME_VSI7          16
#define VME_DMA0          17
#define VME_DMA1          18
#define VME_MAXS	  19
#endif

/*********************************Enhanced Control Structures****************/

typedef struct
{
        UINT8 intNum;           /* VME interrupt number 1 - 7 */
        UINT32 numOfInts;       /* Number of VME interrupts since last call */
        UINT8 vecCount;         /* number of vectors stored in vectors array */
        UINT8 vectors[32];      /* array to hold the STATUSID vectors */

} EN_VME_INT_INFO;

typedef struct
{
        UINT8 intNum;           /* VME interrupt number 1 - 7 */
        UINT32 numOfInts;       /* Number of VME interrupts since last call */
        UINT8 vecCount;         /* number of vectors stored in vectors array */
        UINT8 vectors[128]; /* array to hold the STATUSID vectors */

} EN_VME_EXTINT_INFO;


typedef struct
{
        UINT32 intNum;          /* interrupt number selection, bit 31 is used for validation */
                                                /* bit 0 = Reserved bit 1 = VIRQ1 bit 2 = VIRQ2 etc. */
        UINT32 timeout;         /* wait timeout in jiffies */

} EN_VME_WAIT_LINT;

typedef struct
{
        UINT32 vmeAddress;              /* VME address, lowest address in range to monitor */
	UINT32 vmeAddressUpper;		
        UINT32 type;                    /* type 1=data, 2=program 3=both */
        UINT32 mode;                    /* mode 1=non-privileged, 2=supervisor 3=both */
        UINT32 addrSpace;               /* address space 0=A16, 1=A24, 2=A32 3=Reserved
					   4=A64 */

} EN_VME_IMAGE_ACCESS;

typedef struct
{
        UINT32 vmeAddress;              /* Lower 32bits of VME Address */
        UINT32 vmeAddressUpper;         /* Upper 32bits of VME Address */
        UINT32 size;                    /* Lower 32bits of Image size */
        UINT32 sizeUpper;               /* Upper 32bits of Image size*/
        UINT32 threshold;               /* Threshold for prefetch
					   0=prefetch on FIFO full empty
					   1=prefetch on FIFO half empty 
					   (Applicable only to TSI148)	*/
        UINT32 virtualFifoSize;         /* FIFO Size 0=64, 1=128, 2=256, 3=512 (Applicable only to TSI148)*/
	UINT32 postedWrites;            /* Posted writes 0=disable, 1=enable (Applicable only to Universe II)*/
        UINT32 prefetchRead;            /* Prefetch read 0=disable, 1=enable (Applicable only to Universe II)*/
        UINT32 vmeCycle;                /* 0=SCT , 1=BLT ,  2=MBLT , 3=2eVME , 4 = 2eSST 5 = 2eSSTB
					   (Universe support only SCT and BLTs )*/
	UINT32 sstMode;                 /* 2eSST mode 0=SST160 ,1=SST267 ,2=SST320 (Applicable only to TSI148)*/
        UINT32 type;                    /* type 1=data, 2=program 3=both */
        UINT32 mode;                    /* mode 1=non-privileged, 2=supervisor 3=both */
        UINT32 addrSpace;               /* address space 0=A16, 1=A24,  2=A32, 4=A64, 5=CRSCR ,6=User1, 7=User2                                                         (A64 is supported only in case of TSI148 and User1/2 are applicable 
					   only for Universe II */
	UINT32 pciBusSpace;             /* Pci bus space 0=pci_mem, 1=pci_IO (Applicable only to Universe II)*/
        UINT32 pciBusLock;              /* Pci bus lock on VME read modify write 0=disable, 1=enable
                                           (Applicable only to Universe II)*/
        UINT32 ioremap;                 /* ioremap  0=no, 1=yes */

} EN_VME_IMAGE_DATA;

typedef struct
{
        UINT32 pciAddress;              /* lower 32-bits of PCI address */
        UINT32 pciAddressUpper;  	/* upper 32-bits of PCI address */
        UINT32 vmeAddress;      	/* lower 32-bits of vme address */
        UINT32 vmeAddressUpper;  	/* upper 32-bits of vme address */
        UINT32 size;                    /* lower 32-bits of the size*/
        UINT32 sizeUpper;               /* upper 32-bits of the size*/
        UINT32 readPrefetch;    	/* prefetched reads 0=disable, 1=enable(Applicable only to TSI148)*/
        UINT32 prefetchSize;    	/* cache lines 0= 2 ,1= 4 , 2= 8, 3= 16(Applicable only to TSI148)*/
	UINT32 postedWrites;            /* Posted writes 0=disable, 1=enable (Applicable only to Universe II)*/
        UINT32 dataWidth;               /* data width 0=8 bits, 1=16 bits, 2=32 bits, 3=64 bits 
					   (8 and 64 bit supported on UniverseII only) */
        UINT32 addrSpace;               /* address space 0=A16, 1=A24, 2=A32, 4=A64(tsi148), 5=CR/CSR(tsi148), 
					   6=User1(unv), 7=User2(unv), 8=User1(tsi148), 9=User2(tsi148), 
					  10=User3(tsi148), 11=User4(tsi148) */
        UINT32 type;                    /* type 0=data, 1=program */
        UINT32 mode;            	/* mode 0=non-privileged, 1=supervisor */
        UINT32 vmeCycle;                /* VME bus cycle type 0=SCT , 1=BLT ,  2=MBLT ,                                                                                 3=2eVME , 4 = 2eSST 5=2eSSTB (Universe II supports 
					   only 0=SCT and 1=BLT) */
	UINT32 sstMode;                 /* 2eSST modes SST160, 1=SST267 , 2=SST320(Applicable only to TSI148)*/
        UINT32 vton;			/* Unused */
	UINT32 vtoff;			/* Unused */
	UINT32 sstbSel;         	/* 2eSST Broadcast select ,Bit pattern 0x1FFFFF(Applicable only to TSI148)*/
	UINT32 pciBusSpace;             /* Pci bus space (Universe II only) 0=pci_mem, 1=pci_IO */
        UINT32 ioremap;         	/* ioremap  0=no, 1=yes */

} EN_PCI_IMAGE_DATA;

typedef struct
{
	UINT32 dmaController;		/* DMA Controller 0/1 .Universe II support only DMA0*/
	ULONG size;			/* Size of the DMA Buffer */
} EN_DMA_ALLOC;

typedef struct 
{
	UINT32 vmeBlkSize;		/* VME Bus block size in bytes 0=32, 1=64, 2=128 , 3=256,
						 4=512 , 5=1024 , 6=2048, 7=4096(Applicable only to TSI148)*/
	UINT32 vmeBackOffTimer;		/* VMEBus Back off timer in uS 0=0, 1=1 , 2=2, 3=4 , 4=8 ,
						 5=16 , 6=32 , 7=64(Applicable only to TSI148)*/
	UINT32 pciBlkSize;		/* PCI Bus block size in bytes 0=32, 1=64, 2=128 , 3=256,
						 4=512 , 5=1024 , 6=2048, 7=4096(Applicable only to TSI148)*/	
	UINT32 pciBackOffTimer;		/* PCI Bus Back off timer in uS 0=0, 1=1 , 2=2, 3=4 , 4=8 ,
						 5=16 , 6=32 , 7=64(Applicable only to TSI148)*/
	UINT32 vton;                    /* Unused */
        UINT32 vtoff;                   /* Unused */
	UINT32 timeout;			/* transfer timeout in jiffies */
        UINT32 ownership;               /* VME bus On/Off counters (Universe II only) */
} EN_VME_TXFER_PARAMS;


typedef struct
{
	UINT32 user1;			/* User1 address modifier code, values range from 16 - 31 */
	UINT32 user2;			/* User2 address modifier code, values range from 16 - 31 */

} EN_VME_USER_AM;			/* Applicable only to Universe II */


typedef struct 
{
	UINT32 dataWidth;	/* data width 0=8 bits, 1=16 bits, 2=32 bits, 3=64 bits (8 and 64 bit 
					on UniverseII only) */
	UINT32 addrSpace;	/* address space 0=A16, 1=A24, 2=A32, 4=A64(tsi148), 5=CR/CSR, 
				6=User1(unv), 7=User2(unv), 8=User1(tsi148), 9=User2(tsi148), 10=User3(tsi148), 
				11=User4(tsi148) */
	UINT32 type;			/* type 0=data, 1=program */
	UINT32 mode;		/* mode 0=non-privileged, 1=supervisor */
	UINT32 vmeCycle;	/* VME bus cycle type 0=SCT , 1=BLT ,  2=MBLT , 3=2eVME 
				   , 4 = 2eSST 5=2eSSTB.( Universe II supports only SCT and BLT)*/
	UINT32 sstMode;         /* 2eSST Mode 0=SST160, 1=SST267 , 2=SST320(Applicable only to TSI148)*/
	UINT32 sstbSel;         /* 2eSST Broadcast select ,Bit patter 0 -0x1FFFFF(Applicable only to TSI148)*/
} EN_VME_ACCESS_PARAMS;


typedef struct 
{
	UINT32 direction;		/* 0=read (VME to PCI bus), 1=write (PCI to VME bus) */
	UINT32 vmeAddress;			/* lower 32-bit of destination address*/
	UINT32 vmeAddressUpper;		/* upper 32-bit of destination address*/
	ULONG offset;			/* offset from start of DMA buffer */
	ULONG size;				/* size in bytes to transfer */
	EN_VME_TXFER_PARAMS txfer;	/* transfer parameters */
	EN_VME_ACCESS_PARAMS access; /*destination attributes*/

} EN_VME_DIRECT_TXFER;

typedef struct
{
        UINT32 direction;                       /* 0=read (VME to PCI bus), 1=write (PCI to VME bus) */
        UINT32 vmeAddress;                      /* VME address */
	UINT32 vmeAddressUpper;			/* VME address Upper */
        ULONG offset;                          /* offset from start of DMA buffer */
        ULONG size;                            /* size in bytes to transfer */
        EN_VME_ACCESS_PARAMS access;       /* access parameters */

} EN_VME_CMD_DATA;

#ifdef OS64BIT
typedef struct
{
        ULONG physAddress;                     /* physical start address */
        ULONG size;                            /* size in bytes */
} EN_VME_MEM_ALLOC;
#else
typedef struct
{
        UINT32 physAddress;                     /* physical start address */
        UINT32 size;                            /* size in bytes */
} EN_VME_MEM_ALLOC;
#endif

typedef struct
{
	UINT8 semaphore;			/* semaphore number 0 -7 */
	UINT8 tag;				/* semaphore tag */
} EN_SEM_INFO;

typedef struct
{
	UINT32 devId;				/*VME Bridge device ID,Universe II 
						  or TSI148*/
        UINT32 intCounter[26];			/*Interrupt counters*/
        UINT32 totalIntCount;			/*VME bridge interrupt count*/
        UINT32 otherIntCount;			/*Other shared interrupt count*/
        UINT8 mode;				/*Interrupt mode*/
} EN_INT_STATUS_DATA;

typedef struct
{
        char version[30];			/*Driver version information*/
        char brdName[30];			/*Board name*/
        UINT32 devId;				/*VME Bridge device ID,Universe II
                                                  or TSI148*/
	UINT32 regBase;				/*Memory base address of VME bridge 
						 registers*/
} EN_CTL_STATUS_DATA;

typedef struct
{
        UINT32 devId;				/*VME Bridge device ID,Universe II
                                                  or TSI148*/
	UINT32 readCount;			/*vme_read count*/
	UINT32 writeCount;			/*vme_write count*/
	UINT32 errorCount;			/*vme_read/write error count*/
	UINT32 devReg1;				/*Refer the driver manual for register definitions*/	
	UINT32 devReg2;				   	
	UINT32 devReg3;
	UINT32 devReg4;
	UINT32 devReg5;
	UINT32 devReg6;
	UINT32 devReg7;
	UINT32 devReg8;
	UINT32 spare1;
	UINT32 spare2;
	UINT32 spare3;
	UINT32 spare4;
} EN_IMAGE_STATUS_DATA;

typedef struct
{
	UINT32 devId;				/*VME Bridge device ID,Universe II
                                                  or TSI148*/
	UINT32 readCount;			/*vme_read count*/
	UINT32 writeCount;			/*vme_write count*/
	UINT32 errorCount;			/*vme_read/write error count*/
	UINT32 txferCount;			/*DMA transfers count*/
	UINT32 txferErrors;			/*Number of DMA transfer errors*/
	UINT32 timeoutCount;			/*Number of DMA timeouts*/
	UINT32 cmdPktCount;			/*Command packet count*/
	UINT32 cmdPktBytes;			/*Number of bytes to transfer in linked list*/
	UINT32 devReg1;				/*Refer the driver manual for register definitions*/
	UINT32 devReg2;
	UINT32 devReg3;
	UINT32 devReg4;
	UINT32 devReg5;
	UINT32 devReg6;
	UINT32 devReg7;
	UINT32 devReg8;
	UINT32 devReg9;
	UINT32 devReg10;
	UINT32 devReg11;
	UINT32 spare1;
	UINT32 spare2;
	UINT32 spare3;
	UINT32 spare4;
} EN_DMA_STATUS_DATA;

typedef union
{
        EN_CTL_STATUS_DATA ctlStats;
        EN_IMAGE_STATUS_DATA imageStats;
        EN_DMA_STATUS_DATA dmaStats;
        EN_INT_STATUS_DATA intStats;
}EN_VME_DRIVER_STAT;

typedef struct
{
                UINT32 type;
                EN_VME_DRIVER_STAT driverStat;
}EN_VME_STATS;

typedef struct
{
	UINT32 intNum;   /* IRQ number returned to user handler*/
	UINT32 intVec;    /* Vector data to user*/
	void* usrPtr; 	 /* User passed data */
}EN_VME_INT_USR_DATA;


typedef struct
{
	UINT32 intNum;
	void (*userInt)(void*);
	EN_VME_INT_USR_DATA usrData;
}EN_VME_INT_DATA;


/* Function Prototypes */
#ifndef __KERNEL__

int vme_getApiVersion( char *buffer );
int vme_openDevice( INT8 *deviceName );
int vme_closeDevice( INT32 deviceHandle );
int vme_readRegister( INT32 deviceHandle, UINT16 offset, UINT32 *reg );
int vme_writeRegister( INT32 deviceHandle, UINT16 offset, UINT32 reg );
int vme_setInterruptMode( INT32 deviceHandle, UINT8 mode );
int vme_enableInterrupt( INT32 deviceHandle, UINT8 intNumber );
int vme_disableInterrupt( INT32 deviceHandle, UINT8 intNumber );
int vme_generateInterrupt( INT32 deviceHandle, UINT8 intNumber );
int vme_readInterruptInfo( INT32 deviceHandle, EN_VME_INT_INFO *iPtr );
int vme_readExtInterruptInfo( INT32 deviceHandle, EN_VME_EXTINT_INFO *iPtr );
int vme_setStatusId( INT32 deviceHandle, UINT8 statusId );
int vme_waitInterrupt( INT32 deviceHandle, UINT32 selectedInts, UINT32 timeout,UINT32 *intNum );
int vme_setByteSwap( INT32 deviceHandle, UINT8 enable );
int vme_setUserAmCodes( INT32 deviceHandle, EN_VME_USER_AM *amPtr );
int vme_enableRegAccessImage( INT32 deviceHandle, EN_VME_IMAGE_ACCESS *iPtr );
int vme_disableRegAccessImage( INT32 deviceHandle );
int vme_enableCsrImage( INT32 deviceHandle, UINT8 imageNumber );
int vme_disableCsrImage( INT32 deviceHandle, UINT8 imageNumber );
int vme_enableLocationMon( INT32 deviceHandle, EN_VME_IMAGE_ACCESS *iPtr );
int vme_disableLocationMon( INT32 deviceHandle );
int vme_enablePciImage( INT32 deviceHandle, EN_PCI_IMAGE_DATA *iPtr );
int vme_disablePciImage( INT32 deviceHandle );
int vme_enableVmeImage( INT32 deviceHandle, EN_VME_IMAGE_DATA *iPtr );
int vme_disableVmeImage( INT32 deviceHandle );
int vme_freeDmaBuffer( INT32 deviceHandle );
int vme_dmaDirectTransfer( INT32 deviceHandle, EN_VME_DIRECT_TXFER *dPtr );
int vme_addDmaCmdPkt( INT32 deviceHandle, EN_VME_CMD_DATA *cmdPtr );
int vme_clearDmaCmdPkts( INT32 deviceHandle );
int vme_dmaListTransfer( INT32 deviceHandle, EN_VME_TXFER_PARAMS *tPtr );
int vme_clearStats( INT32 deviceHandle );
int vme_getStats( INT32 deviceHandle, UINT32 type, void *iPtr );
int vme_getBoardCap( INT32 deviceHandle, UINT32 *boardFlags );
int vme_getInstanceCount( INT32 deviceHandle, UINT32 *pCount );
int vme_getGeographicAddr( INT32 deviceHandle, UINT8 *slotNumber );

#ifdef OS64BIT
int vme_read( INT32 deviceHandle, ULONG offset, UINT8 *buffer, ULONG length );
int vme_write( INT32 deviceHandle, ULONG offset, UINT8 *buffer, ULONG length );
int vme_mmap( INT32 deviceHandle, ULONG offset, ULONG length, ULONG *userAddress );
int vme_unmap( INT32 deviceHandle, ULONG userAddress, ULONG length );
int vme_allocDmaBuffer( INT32 deviceHandle, ULONG *size );
int vme_readVerrInfo( INT32 deviceHandle, ULONG *Address,UINT8 *Direction, UINT8 *AmCode);
#ifdef LINUXOS
int vme_reserveMemory( INT32 deviceHandle, ULONG physicalAddress, ULONG size );
#endif
#else
int vme_read( INT32 deviceHandle, UINT32 offset, UINT8 *buffer, UINT32 length );
int vme_write( INT32 deviceHandle, UINT32 offset, UINT8 *buffer, UINT32 length );
int vme_mmap( INT32 deviceHandle, UINT32 offset, UINT32 length, UINT32 *userAddress );
int vme_unmap( INT32 deviceHandle, UINT32 userAddress, UINT32 length );
int vme_allocDmaBuffer( INT32 deviceHandle, UINT32 *size );
int vme_readVerrInfo( INT32 deviceHandle, UINT32 *Address,UINT8 *Direction, UINT8 *AmCode);
#ifdef LINUXOS
int vme_reserveMemory( INT32 deviceHandle, UINT32 physicalAddress, UINT32 size );
#endif
#endif

#else
int vmekrn_acquireDevice( UINT32 imageNumber );
void vmekrn_releaseDevice( UINT32 imageNumber );
int vmekrn_setInterruptMode( UINT8 mode );
int vmekrn_enableInterrupt( UINT8 intNumber );
int vmekrn_disableInterrupt( UINT8 intNumber );
int vmekrn_generateInterrupt( UINT8 intNumber );
int vmekrn_readInterruptInfo( EN_VME_INT_INFO *iPtr );
int vmekrn_readExtInterruptInfo(  EN_VME_EXTINT_INFO *iPtr );
int vmekrn_setStatusId(  UINT8 statusId );
int vmekrn_waitInterrupt( UINT32 selectedInts, UINT32 timeout, UINT32 *intNum );
int vmekrn_setByteSwap(  UINT8 enable );
int vmekrn_setUserAmCodes( EN_VME_USER_AM *amPtr );
int vmekrn_readVerrInfo(  ULONG *Address,UINT8 *Direction, UINT8 *AmCode);
int vmekrn_enableLocationMon( EN_VME_IMAGE_ACCESS *iPtr );
int vmekrn_disableLocationMon( void );
int vmekrn_enableRegAccessImage( EN_VME_IMAGE_ACCESS *iPtr );
int vmekrn_disableRegAccessImage( void );
int vmekrn_enableCsrImage( UINT8 imageNumber );
int vmekrn_disableCsrImage( UINT8 imageNumber );
int vmekrn_readRegister( UINT32 offset, UINT32 *reg );
int vmekrn_writeRegister( UINT32 offset, UINT32 reg );
int vmekrn_readImage( UINT32 imageNumber,ULONG offset, UINT8 *buf, ULONG count );
int vmekrn_writeImage( UINT32 imageNumber,ULONG offset, const UINT8 *buf, ULONG count );
int vmekrn_readDma( UINT32 imageNumber,ULONG offset, UINT8 *buf, ULONG count );
int vmekrn_writeDma( UINT32 imageNumber,ULONG offset, const UINT8 *buf, ULONG count );
int vmekrn_enablePciImage( UINT32 imageNumber, EN_PCI_IMAGE_DATA *iPtr );
int vmekrn_disablePciImage( UINT32 imageNumber );
int vmekrn_enableVmeImage( UINT32 imageNumber, EN_VME_IMAGE_DATA *iPtr );
int vmekrn_disableVmeImage( UINT32 imageNumber );
int vmekrn_allocDmaBuffer( UINT32 imageNumber, ULONG *size );
int vmekrn_freeDmaBuffer( UINT32 imageNumber );
int vmekrn_getDmaBufferAddr( UINT32 imageNumber, ULONG *bufferAddress );
int vmekrn_dmaDirectTransfer( UINT32 imageNumber, EN_VME_DIRECT_TXFER *dPtr );
int vmekrn_addDmaCmdPkt( UINT32 imageNumber, EN_VME_CMD_DATA *cmdPtr );
int vmekrn_clearDmaCmdPkts( UINT32 imageNumber );
int vmekrn_dmaListTransfer( UINT32 imageNumber, EN_VME_TXFER_PARAMS *tPtr );
int vmekrn_getStats( UINT32 type, void *iPtr );
int vmekrn_clearStats( void );
int vmekrn_getBoardCap( UINT32 *boardFlags );
int vmekrn_PciImageAddr( UINT32 imageNumber, ULONG* iPtr );
int vmekrn_VmeImageAddr( UINT32 imageNumber, ULONG* iPtr );
int vmekrn_registerInterrupt( EN_VME_INT_DATA *iPtr );
int vmekrn_removeInterrupt( EN_VME_INT_DATA *iPtr );
int vmekrn_getInstanceCount( INT32 minorNum, UINT32 *pCount );
int vmekrn_getGeographicAddr( INT32 deviceHandle, UINT8 *slotNumber );
#endif /* __KERNEL__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* __INCvme_api_en */
