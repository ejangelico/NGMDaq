/***************************************************************************/
/*                                                                         */
/*  Filename: sis3302_NeutronGamma.h                                       */
/*                                                                         */
/*  Funktion: headerfile for SIS3302                 	                   */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 10.11.2006                                       */
/*  last modification:    07.02.2008                                       */
/*                                                                         */
/*  Modified by Jason Newby for 3302                                       */
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
/*  � 2008                                                                 */
/*                                                                         */
/***************************************************************************/

#define SIS3302_CONTROL_STATUS                     		0x0	       /* read/write; D32 */
#define SIS3302_MODID                              		0x4	       /* read only;  D32 */
#define SIS3302_IRQ_CONFIG                         		0x8        /* read/write; D32 */
#define SIS3302_IRQ_CONTROL                        		0xC        /* read/write; D32 */
#define SIS3302_ACQUISITION_CONTROL                 		0x10       /* read/write; D32 */


#define SIS3302_CBLT_BROADCAST_SETUP               		0x30       /* read/write; D32 */
#define SIS3302_ADC_MEMORY_PAGE_REGISTER           		0x34       /* read/write; D32 */

#define SIS3302_DAC_CONTROL_STATUS                 		0x50       /* read/write; D32 */
#define SIS3302_DAC_DATA                           		0x54       /* read/write; D32 */
#define SIS3302_ADC_GAIN_CONTROL                   		0x58       /* read/write; D32 ; only SIS3302 */

#define SIS3302_KEY_RESET                          	 	0x400	   /* write only; D32 */
#define SIS3302_KEY_DISARM                         		0x414	   /* write only; D32 */

// Neutron/Gamma special
#define SIS3302_KEY_TRIGGER                        		0x418	   /* write only; D32 */
#define SIS3302_KEY_TIMESTAMP_CLEAR                 	0x41C	   /* write only; D32 */
#define SIS3302_KEY_DISARM_AND_ARM_BANK1           		0x420	   /* write only; D32 */
#define SIS3302_KEY_DISARM_AND_ARM_BANK2           		0x424	   /* write only; D32 */




#define SIS3302_EVENT_CONFIG_ALL_ADC                	0x01000000 /* write only; D32 */

// Neutron/Gamma special
#define SIS3302_END_ADDRESS_THRESHOLD_ALL_ADC       	0x01000004 /* write only; D32 */
#define SIS3302_PRETRIGGER_ACTIVEGATE_WINDOW_ALL_ADC  	0x01000008 /* write only; D32 */
#define SIS3302_RAW_DATA_BUFFER_CONFIG_ALL_ADC         	0x0100000C /* write only; D32 */

#define SIS3302_ACCUMULATOR_GATE1_CONFIG_ALL_ADC        0x01000040 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE2_CONFIG_ALL_ADC        0x01000044 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE3_CONFIG_ALL_ADC        0x01000048 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE4_CONFIG_ALL_ADC        0x0100004C /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE5_CONFIG_ALL_ADC        0x01000050 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE6_CONFIG_ALL_ADC        0x01000054 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE7_CONFIG_ALL_ADC        0x01000058 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE8_CONFIG_ALL_ADC        0x0100005C /* write only; D32 */




// ADC1 and ADC2
#define SIS3302_EVENT_CONFIG_ADC12                		0x02000000 /* read/write; D32 */
#define SIS3302_END_ADDRESS_THRESHOLD_ADC12       		0x02000004 /* read/write; D32 */
#define SIS3302_PRETRIGGER_ACTIVEGATE_WINDOW_ADC12  	0x02000008 /* read/write; D32 */
#define SIS3302_RAW_DATA_BUFFER_CONFIG_ADC12         	0x0200000C /* read/write; D32 */

#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC1          	0x02000010 /* read only; D32 */
#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC2          	0x02000014 /* read only; D32 */
#define SIS3302_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC1   	0x02000018 /* read only; D32 */
#define SIS3302_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC2   	0x0200001C /* read only; D32 */

#define SIS3302_ACTUAL_SAMPLE_VALUE_ADC12           	0x02000020 /* read only; D32 */


#define SIS3302_TRIGGER_SETUP_ADC1                  	0x02000030 /* read/write; D32 */
#define SIS3302_TRIGGER_THRESHOLD_ADC1              	0x02000034 /* read/write; D32 */
#define SIS3302_TRIGGER_SETUP_ADC2                  	0x02000038 /* read/write; D32 */
#define SIS3302_TRIGGER_THRESHOLD_ADC2              	0x0200003C /* read/write; D32 */


#define SIS3302_ACCUMULATOR_GATE1_CONFIG_ADC12  		0x02000040 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE2_CONFIG_ADC12  		0x02000044 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE3_CONFIG_ADC12  		0x02000048 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE4_CONFIG_ADC12  		0x0200004C /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE5_CONFIG_ADC12  		0x02000050 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE6_CONFIG_ADC12  		0x02000054 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE7_CONFIG_ADC12  		0x02000058 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE8_CONFIG_ADC12  		0x0200005C /* write only; D32 */



// ADC3 and ADC4
#define SIS3302_EVENT_CONFIG_ADC34                		0x02800000 /* read/write; D32 */
#define SIS3302_END_ADDRESS_THRESHOLD_ADC34       		0x02800004 /* read/write; D32 */
#define SIS3302_PRETRIGGER_ACTIVEGATE_WINDOW_ADC34  	0x02800008 /* read/write; D32 */
#define SIS3302_RAW_DATA_BUFFER_CONFIG_ADC34         	0x0280000C /* read/write; D32 */

#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC3          	0x02800010 /* read only; D32 */
#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC4          	0x02800014 /* read only; D32 */
#define SIS3302_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC3   	0x02800018 /* read only; D32 */
#define SIS3302_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC4   	0x0280001C /* read only; D32 */

#define SIS3302_ACTUAL_SAMPLE_VALUE_ADC34           	0x02800020 /* read only; D32 */


#define SIS3302_TRIGGER_SETUP_ADC3                  	0x02800030 /* read/write; D32 */
#define SIS3302_TRIGGER_THRESHOLD_ADC3              	0x02800034 /* read/write; D32 */
#define SIS3302_TRIGGER_SETUP_ADC4                  	0x02800038 /* read/write; D32 */
#define SIS3302_TRIGGER_THRESHOLD_ADC4              	0x0280003C /* read/write; D32 */


#define SIS3302_ACCUMULATOR_GATE1_CONFIG_ADC34  		0x02800040 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE2_CONFIG_ADC34  		0x02800044 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE3_CONFIG_ADC34  		0x02800048 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE4_CONFIG_ADC34  		0x0280004C /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE5_CONFIG_ADC34  		0x02800050 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE6_CONFIG_ADC34  		0x02800054 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE7_CONFIG_ADC34  		0x02800058 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE8_CONFIG_ADC34  		0x0280005C /* write only; D32 */





// ADC5 and ADC6
#define SIS3302_EVENT_CONFIG_ADC56                		0x03000000 /* read/write; D32 */
#define SIS3302_END_ADDRESS_THRESHOLD_ADC56       		0x03000004 /* read/write; D32 */
#define SIS3302_PRETRIGGER_ACTIVEGATE_WINDOW_ADC56  	0x03000008 /* read/write; D32 */
#define SIS3302_RAW_DATA_BUFFER_CONFIG_ADC56         	0x0300000C /* read/write; D32 */

#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC5          	0x03000010 /* read only; D32 */
#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC6          	0x03000014 /* read only; D32 */
#define SIS3302_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC5   	0x03000018 /* read only; D32 */
#define SIS3302_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC6   	0x0300001C /* read only; D32 */

#define SIS3302_ACTUAL_SAMPLE_VALUE_ADC56           	0x03000020 /* read only; D32 */


#define SIS3302_TRIGGER_SETUP_ADC5                  	0x03000030 /* read/write; D32 */
#define SIS3302_TRIGGER_THRESHOLD_ADC5              	0x03000034 /* read/write; D32 */
#define SIS3302_TRIGGER_SETUP_ADC6                  	0x03000038 /* read/write; D32 */
#define SIS3302_TRIGGER_THRESHOLD_ADC6              	0x0300003C /* read/write; D32 */


#define SIS3302_ACCUMULATOR_GATE1_CONFIG_ADC56  		0x03000040 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE2_CONFIG_ADC56  		0x03000044 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE3_CONFIG_ADC56  		0x03000048 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE4_CONFIG_ADC56  		0x0300004C /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE5_CONFIG_ADC56  		0x03000050 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE6_CONFIG_ADC56  		0x03000054 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE7_CONFIG_ADC56  		0x03000058 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE8_CONFIG_ADC56  		0x0300005C /* write only; D32 */



// ADC7 and ADC8
#define SIS3302_EVENT_CONFIG_ADC78                		0x03800000 /* read/write; D32 */
#define SIS3302_END_ADDRESS_THRESHOLD_ADC78       		0x03800004 /* read/write; D32 */
#define SIS3302_PRETRIGGER_ACTIVEGATE_WINDOW_ADC78  	0x03800008 /* read/write; D32 */
#define SIS3302_RAW_DATA_BUFFER_CONFIG_ADC78         	0x0380000C /* read/write; D32 */

#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC7          	0x03800010 /* read only; D32 */
#define SIS3302_ACTUAL_SAMPLE_ADDRESS_ADC8          	0x03800014 /* read only; D32 */
#define SIS3302_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC7   	0x03800018 /* read only; D32 */
#define SIS3302_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC8   	0x0380001C /* read only; D32 */

#define SIS3302_ACTUAL_SAMPLE_VALUE_ADC78           	0x03800020 /* read only; D32 */


#define SIS3302_TRIGGER_SETUP_ADC7                  	0x03800030 /* read/write; D32 */
#define SIS3302_TRIGGER_THRESHOLD_ADC7              	0x03800034 /* read/write; D32 */
#define SIS3302_TRIGGER_SETUP_ADC8                  	0x03800038 /* read/write; D32 */
#define SIS3302_TRIGGER_THRESHOLD_ADC8              	0x0380003C /* read/write; D32 */


#define SIS3302_ACCUMULATOR_GATE1_CONFIG_ADC78  		0x03800040 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE2_CONFIG_ADC78  		0x03800044 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE3_CONFIG_ADC78  		0x03800048 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE4_CONFIG_ADC78  		0x0380004C /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE5_CONFIG_ADC78  		0x03800050 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE6_CONFIG_ADC78  		0x03800054 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE7_CONFIG_ADC78  		0x03800058 /* write only; D32 */
#define SIS3302_ACCUMULATOR_GATE8_CONFIG_ADC78  		0x0380005C /* write only; D32 */

/*******************************************************************************/
/*                                                                             */
/*  SIS3302-250                                                                */
/*                                                                             */
/*******************************************************************************/
//#define SIS3302_ADC_SPI_ADC12              0x02000040
//#define SIS3302_ADC_SPI_ADC34              0x02800040
//#define SIS3302_ADC_SPI_ADC56              0x03000040
//#define SIS3302_ADC_SPI_ADC78              0x03800040

#define SIS3302_NG_ADC_SPI_ADC12              			0x02000060
#define SIS3302_NG_ADC_SPI_ADC34              			0x02800060
#define SIS3302_NG_ADC_SPI_ADC56              			0x03000060
#define SIS3302_NG_ADC_SPI_ADC78              			0x03800060



/*******************************************************************************/

#define SIS3302_ADC1_OFFSET                         	0x04000000
#define SIS3302_ADC2_OFFSET                         	0x04800000
#define SIS3302_ADC3_OFFSET                         	0x05000000
#define SIS3302_ADC4_OFFSET                         	0x05800000
#define SIS3302_ADC5_OFFSET                         	0x06000000
#define SIS3302_ADC6_OFFSET                         	0x06800000
#define SIS3302_ADC7_OFFSET                         	0x07000000
#define SIS3302_ADC8_OFFSET                        	 	0x07800000

#define SIS3302_NEXT_ADC_OFFSET                     	0x00800000


/* define sample clock */
#define SIS3302_ACQ_SET_CLOCK_TO_200MHZ                 0x70000000  /* default after Reset  */
#define SIS3302_ACQ_SET_CLOCK_TO_100MHZ                 0x60001000
#define SIS3302_ACQ_SET_CLOCK_TO_50MHZ                  0x50002000
#define SIS3302_ACQ_SET_CLOCK_TO_LEMO_X5_CLOCK_IN   	0x40003000

#define SIS3302_ACQ_SET_CLOCK_TO_LEMO_DOUBLE_CLOCK_IN   0x30004000
#define SIS3302_ACQ_SET_CLOCK_TO_LEMO_CLOCK_IN          0x10006000
#define SIS3302_ACQ_SET_CLOCK_TO_P2_CLOCK_IN            0x00007000

#define SIS3302_ACQ_DISABLE_LEMO_TRIGGER        	 	0x01000000	   /* LLNL */
#define SIS3302_ACQ_ENABLE_LEMO_TRIGGER      	   		0x00000100	   /* LLNL */
#define SIS3302_ACQ_DISABLE_LEMO_TIMESTAMPCLR        	0x02000000	   /* LLNL */
#define SIS3302_ACQ_ENABLE_LEMO_TIMESTAMPCLR         	0x00000200	   /* LLNL */


#define SIS3302_ACQ_DISABLE_LEMO_START_STOP         0x01000000
#define SIS3302_ACQ_ENABLE_LEMO_START_STOP          0x00000100

#define SIS3302_ACQ_DISABLE_INTERNAL_TRIGGER        0x00400000
#define SIS3302_ACQ_ENABLE_INTERNAL_TRIGGER         0x00000040

#define SIS3302_ACQ_DISABLE_MULTIEVENT              0x00200000
#define SIS3302_ACQ_ENABLE_MULTIEVENT               0x00000020

#define SIS3302_ACQ_DISABLE_AUTOSTART               0x00100000
#define SIS3302_ACQ_ENABLE_AUTOSTART                0x00000010


#define ADC1_GAIN_HALFSCALE_BIT               		 0x01
#define ADC2_GAIN_HALFSCALE_BIT               		 0x02
#define ADC3_GAIN_HALFSCALE_BIT               		 0x04
#define ADC4_GAIN_HALFSCALE_BIT               		 0x08
#define ADC5_GAIN_HALFSCALE_BIT               		 0x10
#define ADC6_GAIN_HALFSCALE_BIT               		 0x20
#define ADC7_GAIN_HALFSCALE_BIT               		 0x40
#define ADC8_GAIN_HALFSCALE_BIT               		 0x80


#define SIS3302_BROADCAST_MASTER_ENABLE        		0x20
#define SIS3302_BROADCAST_ENABLE               		0x10






#define EVENT_CONF_ADC2_SAVE_RAW_DATA_FIRST_EVENT_ENABLE_BIT	0x8000	   /* LLNL */
#define EVENT_CONF_ADC2_TEST_SAVE_FIR_DATA_ENABLE_BIT			0x4000	   /* LLNL */
#define EVENT_CONF_ADC2_SAVE_RAW_DATA_IF_PILEUP_ENABLE_BIT		0x2000	   /* LLNL */
#define EVENT_CONF_ADC2_SAVE_RAW_DATA_ALWAYS_ENABLE_BIT			0x1000	   /* LLNL */

#define EVENT_CONF_ADC2_EXTERN_TRIGGER_ENABLE_BIT				0x800	   /* LLNL */
#define EVENT_CONF_ADC2_INTERN_TRIGGER_ENABLE_BIT				0x400	   /* LLNL */
#define EVENT_CONF_ADC2_ERROR_CORRECTION_ENABLE_BIT				0x200	   /* LLNL */
#define EVENT_CONF_ADC2_INPUT_INVERT_BIT						0x100	   /* LLNL */


#define EVENT_CONF_ADC1_SAVE_RAW_DATA_FIRST_EVENT_ENABLE_BIT	0x80	   /* LLNL */
#define EVENT_CONF_ADC1_TEST_SAVE_FIR_DATA_ENABLE_BIT			0x40	   /* LLNL */
#define EVENT_CONF_ADC1_SAVE_RAW_DATA_IF_PILEUP_ENABLE_BIT		0x20	   /* LLNL */
#define EVENT_CONF_ADC1_SAVE_RAW_DATA_ALWAYS_ENABLE_BIT			0x10	   /* LLNL */

#define EVENT_CONF_ADC1_EXTERN_TRIGGER_ENABLE_BIT				0x8	   /* LLNL */
#define EVENT_CONF_ADC1_INTERN_TRIGGER_ENABLE_BIT				0x4	   /* LLNL */
#define EVENT_CONF_ADC1_ERROR_CORRECTION_ENABLE_BIT				0x2	   /* LLNL */
#define EVENT_CONF_ADC1_INPUT_INVERT_BIT						0x1	   /* LLNL */




