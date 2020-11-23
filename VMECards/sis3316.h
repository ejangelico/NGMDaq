/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316.h                                                    */
/*                                                                         */
/*  Funktion: headerfile for SIS3316		                               */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 03.07.2012                                       */
/*  last modification:    25.10.2012                                       */
/*                                                                         */
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
/*  � 2012                                                                 */
/*                                                                         */
/***************************************************************************/

/*         addresses   */

#define SIS3316_CONTROL_STATUS                       					0x0			/* read/write; D32 */
#define SIS3316_MODID                                					0x4			/* read only; D32 */
#define SIS3316_IRQ_CONFIG                           					0x8			/* read/write; D32 */
#define SIS3316_IRQ_CONTROL                          					0xC			/* read/write; D32 */

#define SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL   					0x10		/* read/write; D32 */
#define SIS3316_CBLT_BROADCAST            								0x14		/* read/write; D32 */


#define SIS3316_INTERNAL_TEMPERATURE_REG		      	    			0x20      /* read/write; D32 */
#define SIS3316_ONE_WIRE_CONTROL_REG       		   		    			0x24      /* read/write; D32 */
#define SIS3316_SERIAL_NUMBER_REG       		   		    			0x28      /* read only; D32 */

#define SIS3316_ADC_CLK_OSC_I2C_REG				      	    			0x40      /* read/write; D32 */
#define SIS3316_MGT1_OSC_I2C_REG				      	    			0x44      /* read/write; D32 */


#define SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL      	    			0x50      /* read/write; D32 */
#define SIS3316_NIM_CLK_MULTIPLIER_SPI_REG      	    				0x54      /* read/write; D32 */
#define SIS3316_FP_LVDS_BUS_CONTROL             		    			0x58      /* read/write; D32 */
#define SIS3316_NIM_INPUT_CONTROL_REG  									0x5C      /* read/write; D32 */

#define SIS3316_ACQUISITION_CONTROL_STATUS         		    			0x60      /* read/write; D32 */
#define SIS3316_TRIGGER_COINCIDENCE_LOOKUP_TABLE_CONTROL_REG                                 0x64 /* read/write; D32 */
#define SIS3316_TRIGGER_COINCIDENCE_LOOKUP_TABLE_ADDRESS_REG                                 0x68 /* read/write; D32 */
#define SIS3316_TRIGGER_COINCIDENCE_LOOKUP_TABLE_DATA_REG                                    0x6C /* read/write; D32 */

#define SIS3316_LEMO_OUT_CO_SELECT_REG  								0x70      /* read/write; D32 */
#define SIS3316_LEMO_OUT_TO_SELECT_REG  								0x74      /* read/write; D32 */
#define SIS3316_LEMO_OUT_UO_SELECT_REG  								0x78      /* read/write; D32 */


#define SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG							0x80  /* r/w; D32 */
#define SIS3316_DATA_TRANSFER_CH5_8_CTRL_REG  							0x84  /* r/w; D32 */
#define SIS3316_DATA_TRANSFER_CH9_12_CTRL_REG 							0x88  /* r/w; D32 */
#define SIS3316_DATA_TRANSFER_CH12_16_CTRL_REG  						0x8C  /* r/w; D32 */

#define SIS3316_DATA_TRANSFER_ADC1_4_STATUS_REG							0x90  /* read; D32 */
#define SIS3316_DATA_TRANSFER_ADC5_8_STATUS_REG  						0x94  /* read; D32 */
#define SIS3316_DATA_TRANSFER_ADC9_12_STATUS_REG 						0x98  /* read; D32 */
#define SIS3316_DATA_TRANSFER_ADC12_16_STATUS_REG  						0x9C  /* read; D32 */

#define SIS3316_VME_FPGA_LINK_ADC_PROT_STATUS      		    			0xA0      /* read/write; D32 */





#define SIS3316_KEY_RESET                           					0x400	  /* write only; D32 */


//#define SIS3316_KEY_ARM                         						0x410	  /* write only; D32 */
#define SIS3316_KEY_DISARM                         						0x414	  /* write only; D32 */
#define SIS3316_KEY_TRIGGER                         					0x418	  /* write only; D32 */
#define SIS3316_KEY_TIMESTAMP_CLEAR                 					0x41C	  /* write only; D32 */
#define SIS3316_KEY_DISARM_AND_ARM_BANK1           						0x420	  /* write only; D32 */
#define SIS3316_KEY_DISARM_AND_ARM_BANK2           						0x424	  /* write only; D32 */

#define SIS3316_KEY_ADC_FPGA_RESET           							0x434	  /* write only; D32 */
#define SIS3316_KEY_ADC_CLOCK_DCM_RESET           						0x438	  /* write only; D32 */




/* ***************************************************************************** */

/* ADC FPGA Read/Write registers                                                 */

#define SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG     						0x1000	  
#define SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG    						0x2000	  
#define SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG   						0x3000	  
#define SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG  						0x4000	  

#define SIS3316_ADC_CH1_4_ANALOG_CTRL_REG     							0x1004	  
#define SIS3316_ADC_CH5_8_ANALOG_CTRL_REG     							0x2004	  
#define SIS3316_ADC_CH9_12_ANALOG_CTRL_REG     							0x3004	  
#define SIS3316_ADC_CH13_16_ANALOG_CTRL_REG    							0x4004	  

#define SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG 							0x1008	  
#define SIS3316_ADC_CH5_8_DAC_OFFSET_CTRL_REG  							0x2008	  
#define SIS3316_ADC_CH9_12_DAC_OFFSET_CTRL_REG 							0x3008	  
#define SIS3316_ADC_CH13_16_DAC_OFFSET_CTRL_REG							0x4008	  

#define SIS3316_ADC_CH1_4_SPI_CTRL_REG 									0x100C	  
#define SIS3316_ADC_CH5_8_SPI_CTRL_REG  								0x200C	  
#define SIS3316_ADC_CH9_12_SPI_CTRL_REG 								0x300C	  
#define SIS3316_ADC_CH13_16_SPI_CTRL_REG								0x400C	  


#define SIS3316_ADC_CH1_4_EVENT_CONFIG_REG 								0x1010	  
#define SIS3316_ADC_CH5_8_EVENT_CONFIG_REG  							0x2010	  
#define SIS3316_ADC_CH9_12_EVENT_CONFIG_REG 							0x3010	  
#define SIS3316_ADC_CH13_16_EVENT_CONFIG_REG							0x4010	  


#define SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG 							0x1014	  
#define SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG  							0x2014	  
#define SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG 							0x3014	  
#define SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG							0x4014	  


#define SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG 						0x1018	  
#define SIS3316_ADC_CH5_8_ADDRESS_THRESHOLD_REG  						0x2018	  
#define SIS3316_ADC_CH9_12_ADDRESS_THRESHOLD_REG 						0x3018	  
#define SIS3316_ADC_CH13_16_ADDRESS_THRESHOLD_REG						0x4018	  

#define SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG 				0x101C	  
#define SIS3316_ADC_CH5_8_TRIGGER_GATE_WINDOW_LENGTH_REG  				0x201C	  
#define SIS3316_ADC_CH9_12_TRIGGER_GATE_WINDOW_LENGTH_REG 				0x301C	  
#define SIS3316_ADC_CH13_16_TRIGGER_GATE_WINDOW_LENGTH_REG				0x401C	  

//#define SIS3316_ADC_CH1_4_RAW_DATA_SAMPLE_LENGTH_REG 					0x1020	  
//#define SIS3316_ADC_CH5_8_RAW_DATA_SAMPLE_LENGTH_REG  					0x2020	  
//#define SIS3316_ADC_CH9_12_RAW_DATA_SAMPLE_LENGTH_REG 					0x3020	  
//#define SIS3316_ADC_CH13_16_RAW_DATA_SAMPLE_LENGTH_REG					0x4020	  

#define SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG 					0x1020	  
#define SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG  					0x2020	  
#define SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG 					0x3020	  
#define SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG					0x4020	  

#define SIS3316_ADC_CH1_4_PILEUP_CONFIG_REG 							0x1024	  
#define SIS3316_ADC_CH5_8_PILEUP_CONFIG_REG  							0x2024	  
#define SIS3316_ADC_CH9_12_PILEUP_CONFIG_REG 							0x3024	  
#define SIS3316_ADC_CH13_16_PILEUP_CONFIG_REG							0x4024	  



#define SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG 						0x1028	  
#define SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG  						0x2028	  
#define SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG 						0x3028	  
#define SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG						0x4028	  


#define SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG 						0x1030	  
#define SIS3316_ADC_CH5_8_DATAFORMAT_CONFIG_REG  						0x2030	  
#define SIS3316_ADC_CH9_12_DATAFORMAT_CONFIG_REG 						0x3030	  
#define SIS3316_ADC_CH13_16_DATAFORMAT_CONFIG_REG						0x4030	  

#define SIS3316_ADC_CH1_4_MAW_TEST_BUFFER_CONFIG_REG 					0x1034	  
#define SIS3316_ADC_CH5_8_MAW_TEST_BUFFER_CONFIG_REG  					0x2034	  
#define SIS3316_ADC_CH9_12_MAW_TEST_BUFFER_CONFIG_REG 					0x3034	  
#define SIS3316_ADC_CH13_16_MAW_TEST_BUFFER_CONFIG_REG					0x4034	  


#define SIS3316_ADC_CH1_4_INTERNAL_TRIGGER_DELAY_CONFIG_REG 			0x1038	  
#define SIS3316_ADC_CH5_8_INTERNAL_TRIGGER_DELAY_CONFIG_REG				0x2038	  
#define SIS3316_ADC_CH9_12_INTERNAL_TRIGGER_DELAY_CONFIG_REG			0x3038	  
#define SIS3316_ADC_CH13_16_INTERNAL_TRIGGER_DELAY_CONFIG_REG			0x4038	  


#define SIS3316_ADC_CH1_4_INTERNAL_GATE_DELAY_CONFIG_REG 				0x103C	  
#define SIS3316_ADC_CH5_8_INTERNAL_GATE_DELAY_CONFIG_REG				0x203C	  
#define SIS3316_ADC_CH9_12_INTERNAL_GATE_DELAY_CONFIG_REG				0x303C	  
#define SIS3316_ADC_CH13_16_INTERNAL_GATE_DELAY_CONFIG_REG				0x403C	  



#define SIS3316_ADC_CH1_FIR_TRIGGER_SETUP_REG							0x1040 
#define SIS3316_ADC_CH2_FIR_TRIGGER_SETUP_REG							0x1050 
#define SIS3316_ADC_CH3_FIR_TRIGGER_SETUP_REG							0x1060 
#define SIS3316_ADC_CH4_FIR_TRIGGER_SETUP_REG							0x1070 

#define SIS3316_ADC_CH5_FIR_TRIGGER_SETUP_REG							0x2040 
#define SIS3316_ADC_CH6_FIR_TRIGGER_SETUP_REG							0x2050 
#define SIS3316_ADC_CH7_FIR_TRIGGER_SETUP_REG							0x2060 
#define SIS3316_ADC_CH8_FIR_TRIGGER_SETUP_REG							0x2070 

#define SIS3316_ADC_CH9_FIR_TRIGGER_SETUP_REG							0x3040 
#define SIS3316_ADC_CH10_FIR_TRIGGER_SETUP_REG							0x3050 
#define SIS3316_ADC_CH11_FIR_TRIGGER_SETUP_REG							0x3060 
#define SIS3316_ADC_CH12_FIR_TRIGGER_SETUP_REG							0x3070 

#define SIS3316_ADC_CH13_FIR_TRIGGER_SETUP_REG							0x4040 
#define SIS3316_ADC_CH14_FIR_TRIGGER_SETUP_REG							0x4050 
#define SIS3316_ADC_CH15_FIR_TRIGGER_SETUP_REG							0x4060 
#define SIS3316_ADC_CH16_FIR_TRIGGER_SETUP_REG							0x4070 

#define SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_SETUP_REG						0x1080 
#define SIS3316_ADC_CH5_8_SUM_FIR_TRIGGER_SETUP_REG						0x2080 
#define SIS3316_ADC_CH9_12_SUM_FIR_TRIGGER_SETUP_REG					0x3080 
#define SIS3316_ADC_CH13_16_SUM_FIR_TRIGGER_SETUP_REG					0x4080 




#define SIS3316_ADC_CH1_FIR_TRIGGER_THRESHOLD_REG						0x1044 
#define SIS3316_ADC_CH2_FIR_TRIGGER_THRESHOLD_REG						0x1054 
#define SIS3316_ADC_CH3_FIR_TRIGGER_THRESHOLD_REG						0x1064 
#define SIS3316_ADC_CH4_FIR_TRIGGER_THRESHOLD_REG						0x1074 

#define SIS3316_ADC_CH5_FIR_TRIGGER_THRESHOLD_REG						0x2044 
#define SIS3316_ADC_CH6_FIR_TRIGGER_THRESHOLD_REG						0x2054 
#define SIS3316_ADC_CH7_FIR_TRIGGER_THRESHOLD_REG						0x2064 
#define SIS3316_ADC_CH8_FIR_TRIGGER_THRESHOLD_REG						0x2074 

#define SIS3316_ADC_CH9_FIR_TRIGGER_THRESHOLD_REG						0x3044 
#define SIS3316_ADC_CH10_FIR_TRIGGER_THRESHOLD_REG						0x3054 
#define SIS3316_ADC_CH11_FIR_TRIGGER_THRESHOLD_REG						0x3064 
#define SIS3316_ADC_CH12_FIR_TRIGGER_THRESHOLD_REG						0x3074 

#define SIS3316_ADC_CH13_FIR_TRIGGER_THRESHOLD_REG						0x4044 
#define SIS3316_ADC_CH14_FIR_TRIGGER_THRESHOLD_REG						0x4054 
#define SIS3316_ADC_CH15_FIR_TRIGGER_THRESHOLD_REG						0x4064 
#define SIS3316_ADC_CH16_FIR_TRIGGER_THRESHOLD_REG						0x4074 

#define SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_THRESHOLD_REG					0x1084 
#define SIS3316_ADC_CH5_8_SUM_FIR_TRIGGER_THRESHOLD_REG					0x2084 
#define SIS3316_ADC_CH9_12_SUM_FIR_TRIGGER_THRESHOLD_REG				0x3084 
#define SIS3316_ADC_CH13_16_SUM_FIR_TRIGGER_THRESHOLD_REG				0x4084 


#define SIS3316_ADC_CH1_FIR_HIGH_ENERGY_THRESHOLD_REG					0x1048 
#define SIS3316_ADC_CH2_FIR_HIGH_ENERGY_THRESHOLD_REG					0x1058 
#define SIS3316_ADC_CH3_FIR_HIGH_ENERGY_THRESHOLD_REG					0x1068 
#define SIS3316_ADC_CH4_FIR_HIGH_ENERGY_THRESHOLD_REG					0x1078 

#define SIS3316_ADC_CH5_FIR_HIGH_ENERGY_THRESHOLD_REG					0x2048 
#define SIS3316_ADC_CH6_FIR_HIGH_ENERGY_THRESHOLD_REG					0x2058 
#define SIS3316_ADC_CH7_FIR_HIGH_ENERGY_THRESHOLD_REG					0x2068 
#define SIS3316_ADC_CH8_FIR_HIGH_ENERGY_THRESHOLD_REG					0x2078 

#define SIS3316_ADC_CH9_FIR_HIGH_ENERGY_THRESHOLD_REG					0x3048 
#define SIS3316_ADC_CH10_FIR_HIGH_ENERGY_THRESHOLD_REG					0x3058 
#define SIS3316_ADC_CH11_FIR_HIGH_ENERGY_THRESHOLD_REG					0x3068 
#define SIS3316_ADC_CH12_FIR_HIGH_ENERGY_THRESHOLD_REG					0x3078 

#define SIS3316_ADC_CH13_FIR_HIGH_ENERGY_THRESHOLD_REG					0x4048 
#define SIS3316_ADC_CH14_FIR_HIGH_ENERGY_THRESHOLD_REG					0x4058 
#define SIS3316_ADC_CH15_FIR_HIGH_ENERGY_THRESHOLD_REG					0x4068 
#define SIS3316_ADC_CH16_FIR_HIGH_ENERGY_THRESHOLD_REG					0x4078 

#define SIS3316_ADC_CH1_4_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG				0x1088 
#define SIS3316_ADC_CH5_8_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG				0x2088 
#define SIS3316_ADC_CH9_12_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG			0x3088 
#define SIS3316_ADC_CH13_16_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG			0x4088 



#define SIS3316_ADC_CH1_4_TRIGGER_STATISTIC_COUNTER_MODE_REG            0x1090
#define SIS3316_ADC_CH5_8_TRIGGER_STATISTIC_COUNTER_MODE_REG            0x2090
#define SIS3316_ADC_CH9_12_TRIGGER_STATISTIC_COUNTER_MODE_REG           0x3090
#define SIS3316_ADC_CH13_16_TRIGGER_STATISTIC_COUNTER_MODE_REG          0x4090


#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE1_CONFIG_REG 					0x10A0	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE1_CONFIG_REG  				0x20A0	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE1_CONFIG_REG 				0x30A0	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE1_CONFIG_REG				0x40A0	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE2_CONFIG_REG 					0x10A4	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE2_CONFIG_REG  				0x20A4	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE2_CONFIG_REG 				0x30A4	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE2_CONFIG_REG				0x40A4	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE3_CONFIG_REG 					0x10A8	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE3_CONFIG_REG  				0x20A8	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE3_CONFIG_REG 				0x30A8	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE3_CONFIG_REG				0x40A8	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE4_CONFIG_REG 					0x10AC	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE4_CONFIG_REG  				0x20AC	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE4_CONFIG_REG 				0x30AC	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE4_CONFIG_REG				0x40AC	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE5_CONFIG_REG 					0x10B0	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE5_CONFIG_REG  				0x20B0	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE5_CONFIG_REG 				0x30B0	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE5_CONFIG_REG				0x40B0	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE6_CONFIG_REG 					0x10B4	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE6_CONFIG_REG  				0x2004	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE6_CONFIG_REG 				0x30B4	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE6_CONFIG_REG				0x40B4	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE7_CONFIG_REG 					0x10B8	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE7_CONFIG_REG  				0x20B8	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE7_CONFIG_REG 				0x30B8	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE7_CONFIG_REG				0x40B8	  

#define SIS3316_ADC_CH1_4_ACCUMULATOR_GATE8_CONFIG_REG 					0x10BC	  
#define SIS3316_ADC_CH5_8_ACCUMULATOR_GATE8_CONFIG_REG  				0x20BC	  
#define SIS3316_ADC_CH9_12_ACCUMULATOR_GATE8_CONFIG_REG 				0x30BC	  
#define SIS3316_ADC_CH13_16_ACCUMULATOR_GATE8_CONFIG_REG				0x40BC	  




/* ***************************************************************************** */

/* ADC FPGA Read registers                                                       */

#define SIS3316_ADC_CH1_4_FIRMWARE_REG     								0x1100 /* rd only */  
#define SIS3316_ADC_CH5_8_FIRMWARE_REG    								0x2100 /* rd only */
#define SIS3316_ADC_CH9_12_FIRMWARE_REG   								0x3100 /* rd only */
#define SIS3316_ADC_CH13_16_FIRMWARE_REG  								0x4100 /* rd only */

#define SIS3316_ADC_CH1_4_STATUS_REG     								0x1104 /* rd only */  
#define SIS3316_ADC_CH5_8_STATUS_REG    								0x2104 /* rd only */
#define SIS3316_ADC_CH9_12_STATUS_REG   								0x3104 /* rd only */
#define SIS3316_ADC_CH13_16_STATUS_REG  								0x4104 /* rd only */

#define SIS3316_ADC_CH1_4_DAC_OFFSET_READBACK_REG 						0x1108	  
#define SIS3316_ADC_CH5_8_DAC_OFFSET_READBACK_REG 						0x2108	  
#define SIS3316_ADC_CH9_12_DAC_OFFSET_READBACK_REG						0x3108	  
#define SIS3316_ADC_CH13_16_DAC_OFFSET_READBACK_REG						0x4108	  

#define SIS3316_ADC_CH1_4_SPI_READBACK_REG 								0x110C	  
#define SIS3316_ADC_CH5_8_SPI_READBACK_REG  							0x210C	  
#define SIS3316_ADC_CH9_12_SPI_READBACK_REG 							0x310C	  
#define SIS3316_ADC_CH13_16_SPI_READBACK_REG							0x410C	  




#define SIS3316_ADC_CH1_ACTUAL_SAMPLE_ADDRESS_REG						0x1110 /* rd only */  
#define SIS3316_ADC_CH2_ACTUAL_SAMPLE_ADDRESS_REG						0x1114 /* rd only */  
#define SIS3316_ADC_CH3_ACTUAL_SAMPLE_ADDRESS_REG						0x1118 /* rd only */  
#define SIS3316_ADC_CH4_ACTUAL_SAMPLE_ADDRESS_REG						0x111C /* rd only */  

#define SIS3316_ADC_CH5_ACTUAL_SAMPLE_ADDRESS_REG						0x2110 /* rd only */  
#define SIS3316_ADC_CH6_ACTUAL_SAMPLE_ADDRESS_REG						0x2114 /* rd only */  
#define SIS3316_ADC_CH7_ACTUAL_SAMPLE_ADDRESS_REG						0x2118 /* rd only */  
#define SIS3316_ADC_CH8_ACTUAL_SAMPLE_ADDRESS_REG						0x211C /* rd only */  

#define SIS3316_ADC_CH9_ACTUAL_SAMPLE_ADDRESS_REG						0x3110 /* rd only */  
#define SIS3316_ADC_CH10_ACTUAL_SAMPLE_ADDRESS_REG						0x3114 /* rd only */  
#define SIS3316_ADC_CH11_ACTUAL_SAMPLE_ADDRESS_REG						0x3118 /* rd only */  
#define SIS3316_ADC_CH12_ACTUAL_SAMPLE_ADDRESS_REG						0x311C /* rd only */  

#define SIS3316_ADC_CH13_ACTUAL_SAMPLE_ADDRESS_REG						0x4110 /* rd only */  
#define SIS3316_ADC_CH14_ACTUAL_SAMPLE_ADDRESS_REG						0x4114 /* rd only */  
#define SIS3316_ADC_CH15_ACTUAL_SAMPLE_ADDRESS_REG						0x4118 /* rd only */  
#define SIS3316_ADC_CH16_ACTUAL_SAMPLE_ADDRESS_REG						0x411C /* rd only */  



#define SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x1120 
#define SIS3316_ADC_CH2_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x1124    
#define SIS3316_ADC_CH3_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x1128    
#define SIS3316_ADC_CH4_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x112C    

#define SIS3316_ADC_CH5_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x2120    
#define SIS3316_ADC_CH6_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x2124    
#define SIS3316_ADC_CH7_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x2128    
#define SIS3316_ADC_CH8_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x212C    

#define SIS3316_ADC_CH9_PREVIOUS_BANK _SAMPLE_ADDRESS_REG				0x3120    
#define SIS3316_ADC_CH10_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x3124    
#define SIS3316_ADC_CH11_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x3128    
#define SIS3316_ADC_CH12_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x312C    

#define SIS3316_ADC_CH13_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x4120    
#define SIS3316_ADC_CH14_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x4124    
#define SIS3316_ADC_CH15_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x4128    
#define SIS3316_ADC_CH16_PREVIOUS_BANK_SAMPLE_ADDRESS_REG				0x412C    


/* ***************************************************************************** */

#define SIS3316_FPGA_ADC1_REG_BASE					  					0x1000	  
#define SIS3316_FPGA_ADC2_REG_BASE					  					0x2000	  
#define SIS3316_FPGA_ADC3_REG_BASE					  					0x3000	  
#define SIS3316_FPGA_ADC4_REG_BASE					  					0x4000	  

#define SIS3316_FPGA_ADC_REG_OFFSET					  					0x1000	  



#define SIS3316_FPGA_ADC1_MEM_BASE					  					0x100000	  
#define SIS3316_FPGA_ADC2_MEM_BASE					  					0x200000	  
#define SIS3316_FPGA_ADC3_MEM_BASE					  					0x300000	  
#define SIS3316_FPGA_ADC4_MEM_BASE					  					0x400000	  

#define SIS3316_FPGA_ADC_MEM_OFFSET					  					0x100000	  


