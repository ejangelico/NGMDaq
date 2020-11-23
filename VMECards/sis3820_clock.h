/***************************************************************************/
/*                                                                         */
/*  Filename: sis3820_clock.h                                              */
/*                                                                         */
/*  Funktion: headerfile for SIS3820 Clock (Gamma)                         */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 04.04.2005                                       */
/*  last modification:    26.05.2005                                       */
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


/* addresses  */ 

#define SIS3820CLOCK_CONTROL_STATUS				0x0			/* read/write; D32 */
#define SIS3820CLOCK_MODID						0x4			/* read only; D32 */

#define SIS3820CLOCK_CLOCK_SOURCE				0x10		/* read/write; D32 */
#define SIS3820CLOCK_TRIGGERMASK				0x14		/* read/write  D32 */

#define SIS3820_LNE_PRESCALE					0x18		/* read/write; D32 */





#define SIS3820CLOCK_KEY_RESET					0x60
#define SIS3820CLOCK_KEY_TRIGGER				0x64
#define SIS3820CLOCK_KEY_CLR_TIMESTAMP			0x68
#define SIS3820CLOCK_DLL_KEY_RESET				0x6C





/* bit definitions  */
#define SIS3820CLOCK_GENERAL_DISABLE			0x80000000
#define SIS3820CLOCK_GENERAL_ENABLE				0x8000

#define SIS3820CLOCK_CLR_TRIGGER_VETO			0x40000000
#define SIS3820CLOCK_SET_TRIGGER_VETO			0x4000


#define SIS3820CLOCK_EXT_CLR_TIMESTAMP_DISABLE	0x10000000
#define SIS3820CLOCK_EXT_CLR_TIMESTAMP_ENABLE	0x1000

#define SIS3820CLOCK_EXT_TRIGGER_IN_DISABLE		0x8000000
#define SIS3820CLOCK_EXT_TRIGGER_IN_ENABLE		0x800

#define SIS3820CLOCK_EXT_VETO_IN_DISABLE		0x4000000
#define SIS3820CLOCK_EXT_VETO_IN_ENABLE			0x400

#define SIS3820CLOCK_FP_CLOCK_OUT_DISABLE		0x800000
#define SIS3820CLOCK_FP_CLOCK_OUT_ENABLE		0x80

#define SIS3820CLOCK_P2_OUT_DISABLE				0x400000
#define SIS3820CLOCK_P2_OUT_ENABLE				0x40


#define SIS3820CLOCK_CLOCK_SOURCE_100MHZ		0x0
#define SIS3820CLOCK_CLOCK_SOURCE_80MHZ			0x1
#define SIS3820CLOCK_CLOCK_SOURCE_EXT_CTRL		0x2
		
#define SIS3820CLOCK_CLOCK_DIVIDE_1				0x0
#define SIS3820CLOCK_CLOCK_DIVIDE_2				0x10
#define SIS3820CLOCK_CLOCK_DIVIDE_4				0x20

		
		
		
		
		

#define SIS3820_IRQ_SOURCE0_ENABLE				0x1
#define SIS3820_IRQ_SOURCE1_ENABLE				0x2
#define SIS3820_IRQ_SOURCE2_ENABLE				0x4
#define SIS3820_IRQ_SOURCE3_ENABLE				0x8
#define SIS3820_IRQ_SOURCE4_ENABLE				0x10
#define SIS3820_IRQ_SOURCE5_ENABLE				0x20
#define SIS3820_IRQ_SOURCE6_ENABLE				0x40
#define SIS3820_IRQ_SOURCE7_ENABLE				0x80

#define SIS3820_IRQ_SOURCE0_DISABLE				0x100
#define SIS3820_IRQ_SOURCE1_DISABLE				0x200
#define SIS3820_IRQ_SOURCE2_DISABLE				0x400
#define SIS3820_IRQ_SOURCE3_DISABLE				0x800
#define SIS3820_IRQ_SOURCE4_DISABLE				0x1000
#define SIS3820_IRQ_SOURCE5_DISABLE				0x2000
#define SIS3820_IRQ_SOURCE6_DISABLE				0x4000
#define SIS3820_IRQ_SOURCE7_DISABLE				0x8000

#define SIS3820_IRQ_SOURCE0_CLEAR				0x10000
#define SIS3820_IRQ_SOURCE1_CLEAR				0x20000
#define SIS3820_IRQ_SOURCE2_CLEAR				0x40000
#define SIS3820_IRQ_SOURCE3_CLEAR				0x80000
#define SIS3820_IRQ_SOURCE4_CLEAR				0x100000
#define SIS3820_IRQ_SOURCE5_CLEAR				0x200000
#define SIS3820_IRQ_SOURCE6_CLEAR				0x400000
#define SIS3820_IRQ_SOURCE7_CLEAR				0x800000

#define SIS3820_IRQ_SOURCE0_FLAG				0x1000000
#define SIS3820_IRQ_SOURCE1_FLAG				0x2000000
#define SIS3820_IRQ_SOURCE2_FLAG				0x4000000
#define SIS3820_IRQ_SOURCE3_FLAG				0x8000000
#define SIS3820_IRQ_SOURCE4_FLAG				0x10000000
#define SIS3820_IRQ_SOURCE5_FLAG				0x20000000
#define SIS3820_IRQ_SOURCE6_FLAG				0x40000000
#define SIS3820_IRQ_SOURCE7_FLAG				0x80000000


/* Control register bit defintions */

#define CTRL_USER_LED_OFF						0x10000	   /* default after Reset */
#define CTRL_USER_LED_ON						0x1


#define CTRL_COUNTER_TEST_MODE_DISABLE			0x200000
#define CTRL_COUNTER_TEST_MODE_ENABLE			0x20

#define CTRL_REFERENCE_CH1_DISABLE				0x400000
#define CTRL_REFERENCE_CH1_ENABLE				0x40


/* Status register bit defintions */

#define STAT_OPERATION_SCALER_ENABLED			0x10000
#define STAT_OPERATION_MCS_ENABLED				0x40000
#define STAT_OPERATION_VME_WRITE_ENABLED		0x800000



/* Acqusition / Mode register bit defintions */
#define SIS3820_CLEARING_MODE					0x0
#define SIS3820_NON_CLEARING_MODE				0x1

#define SIS3820_MCS_DATA_FORMAT_32BIT			0x0
#define SIS3820_MCS_DATA_FORMAT_24BIT			0x4
#define SIS3820_MCS_DATA_FORMAT_16BIT			0x8
#define SIS3820_MCS_DATA_FORMAT_8BIT			0xC

#define SIS3820_SCALER_DATA_FORMAT_32BIT		0x0
#define SIS3820_SCALER_DATA_FORMAT_24BIT		0x4

#define SIS3820_LNE_SOURCE_VME					0x0
#define SIS3820_LNE_SOURCE_CONTROL_SIGNAL		0x10
#define SIS3820_LNE_SOURCE_INTERNAL_10MHZ		0x20
#define SIS3820_LNE_SOURCE_CHANNEL_N			0x30
#define SIS3820_LNE_SOURCE_PRESET				0x40

#define SIS3820_ARM_ENABLE_CONTROL_SIGNAL		0x000
#define SIS3820_ARM_ENABLE_CHANNEL_N			0x100

#define SIS3820_FIFO_MODE						0x0000
#define SIS3820_SDRAM_MODE						0x1000

#define SIS3820_CONTROL_INPUT_MODE0				0x00000
#define SIS3820_CONTROL_INPUT_MODE1				0x10000
#define SIS3820_CONTROL_INPUT_MODE2				0x20000
#define SIS3820_CONTROL_INPUT_MODE3				0x30000
#define SIS3820_CONTROL_INPUT_MODE4				0x40000

#define SIS3820_CONTROL_INPUTS_INVERT			0x80000

#define SIS3820_CONTROL_OUTPUT_MODE0			0x000000
#define SIS3820_CONTROL_OUTPUT_MODE1			0x100000

#define SIS3820_CONTROL_OUTPUTS_INVERT			0x800000


#define SIS3820_OP_MODE_SCALER					0x00000000
#define SIS3820_OP_MODE_MULTI_CHANNEL_SCALER	0x20000000
#define SIS3820_OP_MODE_VME_FIFO_WRITE			0x70000000


/* preset enable/hit register */
#define SIS3820_PRESET_STATUS_ENABLE_GROUP1			0x1
#define SIS3820_PRESET_REACHED_GROUP1				0x2
#define SIS3820_PRESET_LNELATCHED_REACHED_GROUP1	0x4
#define SIS3820_PRESET_STATUS_ENABLE_GROUP2			0x10000
#define SIS3820_PRESET_REACHED_GROUP2				0x20000
#define SIS3820_PRESET_LNELATCHED_REACHED_GROUP2	0x40000
