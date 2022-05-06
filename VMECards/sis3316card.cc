#include<stdlib.h>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include "sis3316card.h"
#include "vme_interface_class.h"

using namespace std;

/**************************************************************************************/

ClassImp(sis3316card)

#define I2C_ACK             8
#define I2C_START			9
#define I2C_REP_START		10
#define I2C_STOP			11
#define I2C_WRITE			12
#define I2C_READ			13
#define I2C_BUSY			31
#define __IS_16BIT_MODEL__  1  // Added to switch ADC output modes correctly. (Brian L.);
#define OSC_ADR	0x55

int sis3316card::prevRunningBank = 1;

sis3316card::sis3316card()
{
    initcommon();
}

sis3316card::sis3316card(vme_interface_class *crate, unsigned int baseaddress)
{
    initcommon();
	if(crate){
		this->vmei = crate;
	}
	this->baseaddress = baseaddress;

}

void sis3316card::setQDC(int block, int gate,int start, int length)
{
    if(gate<0
	||gate>=SIS3316_QDC_PER_CHANNEL
   	||block<0
	||block>=SIS3316_ADCGROUP_PER_CARD) return;
    qdcstart[block][gate]=start;
    qdclength[block][gate]=length;
}



/****************************************************************************************************/

int sis3316card::update_firmware(char *path, int offset, void (*cb)(int percentage)){
	int rc;
	FILE *fp;
	char *buf;
	int fileSize;
	int percent, percent_old;

	if(path == NULL){
		return -100;
	}

	fp = fopen( path, "rb");
	if(fp == NULL){
		return -101;
	}

	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	rewind(fp);

	buf = (char *)malloc(fileSize);
	if(buf == NULL){
		return -102;
	}

	rc = fread(buf, 1, fileSize, fp);
	if(rc != fileSize){
		return -103;
	}

	fclose(fp);

	percent = percent_old = 0;
	if(cb){
		(cb)(percent);
	}

	int written = 0;
	int pageProgramSize;

	this->FlashEnableProg();

	while(written < fileSize){
		// erase
		if((written & (SIS3316_FLASH_ERASE_BLOCKSIZE - 1)) == 0){
			rc = this->FlashEraseBlock((offset + written) & 0xFFFF0000);
		}

		if(fileSize >= (written + SIS3316_FLASH_PROGRAM_PAGESIZE)){
			pageProgramSize = SIS3316_FLASH_PROGRAM_PAGESIZE;
		}else{
			pageProgramSize = fileSize - written;
		}

		rc = this->FlashProgramPage(offset + written, buf + written, pageProgramSize);

		written += pageProgramSize;

		if(cb){
			percent = written * 100 / fileSize;
			if(percent != percent_old){
				(cb)(percent);
				percent_old = percent;
			}
		}
	}
	
	this->FlashDisableProg();

	free(buf);

	return 0;
}

int sis3316card::FlashEnableProg(){
	uint32_t tmp;
	int rc;

	rc = vmei->vme_A32D32_read(this->baseaddress + SIS3316_SPI_FLASH_CSR, &tmp);
	if(rc){
		return rc;
	}

	tmp |= (1<<ENABLE_SPI_PROG);

	rc = vmei->vme_A32D32_write(this->baseaddress + SIS3316_SPI_FLASH_CSR, tmp);
	if(rc){
		return rc;
	}

	return 0;
}

int sis3316card::FlashDisableProg(){
	uint32_t tmp;
	int rc;

	rc = vmei->vme_A32D32_read(this->baseaddress + SIS3316_SPI_FLASH_CSR, &tmp);
	if(rc){
		return rc;
	}

	tmp &= ~(1<<ENABLE_SPI_PROG);

	rc = vmei->vme_A32D32_write(this->baseaddress + SIS3316_SPI_FLASH_CSR, tmp);
	if(rc){
		return rc;
	}

	return 0;
}

int sis3316card::FlashEnableCS(int chip){
	uint32_t tmp;
	int rc;

	rc = vmei->vme_A32D32_read(this->baseaddress + SIS3316_SPI_FLASH_CSR, &tmp);
	if(rc){
		return rc;
	}

	switch(chip){
	case 0:
		tmp |= (1<<CHIPSELECT_1);
		break;
	case 1:
		tmp |= (1<<CHIPSELECT_2);
		break;
	default:
		return -100;
	}

	rc = vmei->vme_A32D32_write(this->baseaddress + SIS3316_SPI_FLASH_CSR, tmp);
	if(rc){
		return rc;
	}

	return 0;
}

int sis3316card::FlashDisableCS(int chip){
	uint32_t tmp;
	int rc;

	rc = vmei->vme_A32D32_read(this->baseaddress + SIS3316_SPI_FLASH_CSR, &tmp);
	if(rc){
		return rc;
	}

	switch(chip){
	case 0:
		tmp &= ~(1<<CHIPSELECT_1);
		break;
	case 1:
		tmp &= ~(1<<CHIPSELECT_2);
		break;
	default:
		return -100;
	}

	rc = vmei->vme_A32D32_write(this->baseaddress + SIS3316_SPI_FLASH_CSR, tmp);
	if(rc){
		return rc;
	}

	return 0;
}

int sis3316card::FlashWriteEnable(){
	int rc;
	
	rc = this->FlashEnableCS(0);
	if(rc){
		return rc;
	}
	
	rc = this->FlashXfer(0x06, NULL); // Write Enable command
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}

	rc = this->FlashDisableCS(0);
	if(rc){
		return rc;
	}

	return 0;
}

int sis3316card::FlashProgramPage(int address, char *data, int len){
	int rc;
	char tmp;
	uint32_t utmp;
	uint32_t dmabuf[SIS3316_FLASH_PROGRAM_PAGESIZE];
	uint32_t putWords;

	if(data == NULL){
		return -100;
	}
	
	rc = this->FlashWriteEnable();
	if(rc){
		return rc;
	}

	// program command
	rc = this->FlashEnableCS(0);
	if(rc){
		return rc;
	}
	
	rc = this->FlashXfer(0x02, NULL); // Page program command
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}
	rc = this->FlashXfer((char)(address >> 16), NULL);
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}
	rc = this->FlashXfer((char)(address >> 8), NULL);
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}
	rc = this->FlashXfer((char)address, NULL);
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}

	// dma d32
	for(int k = 0;k < SIS3316_FLASH_PROGRAM_PAGESIZE;k++){
		dmabuf[k] = (uint32_t)*(data + k);
	}
	rc = vmei->vme_A32DMA_D32FIFO_write(this->baseaddress + SIS3316_SPI_FLASH_DATA, dmabuf, SIS3316_FLASH_PROGRAM_PAGESIZE, &putWords);
	if(rc || putWords != SIS3316_FLASH_PROGRAM_PAGESIZE){
		return -101;
	}

	// busy polling
	do{
		rc = vmei->vme_A32D32_read(this->baseaddress + SIS3316_SPI_FLASH_CSR, &utmp);
		if(rc){
			return rc;
		}

		utmp &= (1<<FLASH_LOGIC_BUSY^1<<FIFO_NOT_EMPTY);

		usleep(1); // testing

	}while(utmp != 0);

	// single cycles
#if 0
	for(int i = 0;i < len && i < SIS3316_FLASH_PROGRAM_PAGESIZE;i++){
		rc = this->FlashXfer(*(data + i), NULL);
		if(rc){
			this->FlashDisableCS(0);
			return rc;
		}
	}
#endif

	rc = this->FlashDisableCS(0);
	if(rc){
		return rc;
	}

	usleep(1); // testing
	// busy polling
	do{
		rc = this->FlashEnableCS(0);
		if(rc){
			return rc;
		}
	
		rc = this->FlashXfer(0x05, NULL); // read status register 1 command
		if(rc){
			this->FlashDisableCS(0);
			return rc;
		}
		rc = this->FlashXfer(0, &tmp);
		if(rc){
			this->FlashDisableCS(0);
			return rc;
		}

		tmp &= 1;

		rc = this->FlashDisableCS(0);
		if(rc){
			return rc;
		}
		usleep(1); // testing
	}while(tmp);

	return 0;
}

int sis3316card::FlashEraseBlock(int address){
	int rc;
	char tmp;
	
	rc = this->FlashWriteEnable();
	if(rc){
		return rc;
	}

	// erase command
	rc = this->FlashEnableCS(0);
	if(rc){
		return rc;
	}
	
	rc = this->FlashXfer((char)0xD8, NULL); // 64kB Block erase command
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}
	rc = this->FlashXfer((char)(address >> 16), NULL);
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}
	rc = this->FlashXfer((char)(address >> 8), NULL);
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}
	rc = this->FlashXfer((char)address, NULL);
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}

	rc = this->FlashDisableCS(0);
	if(rc){
		return rc;
	}
	usleep(1); // testing
	// busy polling
	do{
		rc = this->FlashEnableCS(0);
		if(rc){
			this->FlashDisableCS(0);
			return rc;
		}
	
		rc = this->FlashXfer(0x05, NULL); // read status register 1 command
		if(rc){
			this->FlashDisableCS(0);
			return rc;
		}
		rc = this->FlashXfer(0, &tmp);
		if(rc){
			this->FlashDisableCS(0);
			return rc;
		}

		tmp &= 1;

		rc = this->FlashDisableCS(0);
		if(rc){
			return rc;
		}
		usleep(1); // testing
	}while(tmp);

	return 0;
}

int sis3316card::FlashXfer(char in, char *out){
	uint32_t tmp;
	char ctmp;
	int rc;

	tmp = in;
	rc = vmei->vme_A32D32_write(this->baseaddress + SIS3316_SPI_FLASH_DATA, tmp);
	if(rc){
		return rc;
	}

	do{
		rc = vmei->vme_A32D32_read(this->baseaddress + SIS3316_SPI_FLASH_CSR, &tmp);
		if(rc){
			return rc;
		}

		tmp &= (1<<FLASH_LOGIC_BUSY^1<<FIFO_NOT_EMPTY);

		usleep(1); // testing

	}while(tmp != 0);

	rc = vmei->vme_A32D32_read(this->baseaddress + SIS3316_SPI_FLASH_DATA, &tmp);
	if(rc){
		return rc;
	}

	ctmp = tmp & 0xFF;
	if(out){
		*out = ctmp;
	}

	return 0;
}

int sis3316card::FlashGetId(char *id){
	int rc;

	if(id == NULL){
		return -100;
	}

	rc = this->FlashEnableCS(0);
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}
	
	rc = this->FlashXfer((char)0x9F, NULL); // JEDEC ID command
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}
	rc = this->FlashXfer(0, id);
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}
	rc = this->FlashXfer(0, id+1);
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}
	rc = this->FlashXfer(0, id+2);
	if(rc){
		this->FlashDisableCS(0);
		return rc;
	}

	rc = this->FlashDisableCS(0);
	if(rc){
		return rc;
	}

	return 0;
}


int sis3316card::I2cStart(int osc){
	int rc;
	int i;
	uint32_t tmp;

	if(osc > 3){
		return -101;
	}
	// start
	rc = this->vmei->vme_A32D32_write(this->baseaddress + SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), 1<<I2C_START);
	if(rc){
        printf("I2cStart1 ERROR(%d)\n",rc);
		return rc;
	}

	i = 0;
	do{
		// poll i2c fsm busy
		rc = this->vmei->vme_A32D32_read(this->baseaddress + SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), &tmp);
		if(rc){
            printf("I2cStart2 try(%d) ERROR(%d)\n",i,rc);
			return rc;
		}
		i++;
	}while((rc && tmp & (1<<I2C_BUSY)) && (i < 1000));

	// register access problem
	if(i == 1000){
        printf("I2cStart3 too many tries \n");
		return -100;
	}

	return 0;
}

int sis3316card::I2cStop(int osc){
	int rc;
	int i;
	uint32_t tmp;

	if(osc > 3){
		return -101;
	}

	// stop
    usleep(20000);
	rc = this->vmei->vme_A32D32_write(this->baseaddress + SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), 1<<I2C_STOP);
	if(rc){
        printf("I2cStop1 ERROR(%d)\n",rc);
		return rc;
	}
	
	i = 0;
	do{
		// poll i2c fsm busy
        usleep(20000);
		rc = this->vmei->vme_A32D32_read(this->baseaddress + SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), &tmp);
		if(rc){
            printf("I2cStop2 ERROR(%d)\n",rc);
			return rc;
		}
		i++;
	}while((tmp & (1<<I2C_BUSY)) && (i < 1000));

	// register access problem
	if(i == 1000){
		return -100;
	}

	return 0;
}

int sis3316card::I2cWriteByte(int osc, unsigned char data, char *ack){
	int rc;
	int i;
	uint32_t tmp;
	
	if(osc > 3){
		return -101;
	}

	// write byte, receive ack
	rc = this->vmei->vme_A32D32_write(this->baseaddress + SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), 1<<I2C_WRITE ^ data);
	if(rc){
        printf("I2cWriteByte1 ERROR(%d)\n",rc);
		return rc;
	}
	
	i = 0;
	do{
		// poll i2c fsm busy
		rc = this->vmei->vme_A32D32_read(this->baseaddress + SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), &tmp);
		if(rc){
            printf("I2cWriteByte2 ERROR(%d)\n",rc);
			return rc;
		}
		i++;
	}while((tmp & (1<<I2C_BUSY)) && (i < 1000));

	// register access problem
	if(i == 1000){
		return -100;
	}

	// return ack value?
	if(ack){
		// yup
		*ack = tmp & 1<<I2C_ACK ? 1 : 0;
	}

	return 0;
}

int sis3316card::I2cReadByte(int osc, unsigned char *data, char ack){
	int rc;
	int i;
	uint32_t tmp;
	
	if(osc > 3){
		return -101;
	}

	// read byte, put ack
	tmp = 1<<I2C_READ;
	tmp |= ack ? 1<<I2C_ACK : 0;
    usleep(20000);
	rc = this->vmei->vme_A32D32_write(this->baseaddress + SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), tmp);
	if(rc){
        printf("I2cReadByte1 ERROR(%d)\n",rc);
		return rc;
	}
	
	i = 0;
	do{
		// poll i2c fsm busy
        usleep(20000);
		rc = this->vmei->vme_A32D32_read(this->baseaddress + SIS3316_ADC_CLK_OSC_I2C_REG + (4 * osc), &tmp);
		if(rc){
            printf("I2cReadByte2 ERROR(%d)\n",rc);
			return rc;
		}
		i++;
	}while((tmp & (1<<I2C_BUSY)) && (i < 1000));

	// register access problem
	if(i == 1000){
		return -100;
	}

	return 0;
}

int sis3316card::Si570FreezeDCO(int osc){
	int rc;
	char ack;

	// start
	rc = this->I2cStart(osc);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	// address
	rc = this->I2cWriteByte(osc, OSC_ADR<<1, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// register offset
	rc = this->I2cWriteByte(osc, 0x89, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// write data
	rc = this->I2cWriteByte(osc, 0x10, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// stop
	rc = this->I2cStop(osc);
	if(rc){
		return rc;
	}

	return 0;
}

int sis3316card::Si570Divider(int osc, unsigned char *data){
	int rc;
	char ack;
	int i;

	// start
	rc = this->I2cStart(osc);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	// address
	rc = this->I2cWriteByte(osc, OSC_ADR<<1, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// register offset
	rc = this->I2cWriteByte(osc, 0x0D, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// write data
	for(i = 0;i < 2;i++){
		rc = this->I2cWriteByte(osc, data[i], &ack);
		if(rc){
			this->I2cStop(osc);
			return rc;
		}

		if(!ack){
			this->I2cStop(osc);
			return -101;
		}
	}

	// stop
	rc = this->I2cStop(osc);
	if(rc){
		return rc;
	}

	return 0;
}

int sis3316card::Si570UnfreezeDCO(int osc){
	int rc;
	char ack;

	// start
	rc = this->I2cStart(osc);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	// address
	rc = this->I2cWriteByte(osc, OSC_ADR<<1, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// register offset
	rc = this->I2cWriteByte(osc, 0x89, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// write data
	rc = this->I2cWriteByte(osc, 0x00, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// stop
	rc = this->I2cStop(osc);
	if(rc){
		return rc;
	}

	return 0;
}

int sis3316card::Si570NewFreq(int osc){
	int rc;
	char ack;

	// start
	rc = this->I2cStart(osc);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	// address
	rc = this->I2cWriteByte(osc, OSC_ADR<<1, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// register offset
	rc = this->I2cWriteByte(osc, 0x87, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// write data
	rc = this->I2cWriteByte(osc, 0x40, &ack);
	if(rc){
		this->I2cStop(osc);
		return rc;
	}

	if(!ack){
		this->I2cStop(osc);
		return -101;
	}

	// stop
	rc = this->I2cStop(osc);
	if(rc){
		return rc;
	}

	return 0;
}



/****************************************************************************************************/

int sis3316card::set_frequency(int osc, unsigned char *values){
	int rc;

	if(values == NULL){
		return -100;
	}
	if(osc > 3 || osc < 0){
		return -100;
	}

	rc = this->Si570FreezeDCO(osc);
	if(rc){
        printf("Si570FreezeDCO Error(%d)\n",rc);
		return rc;
	}

	rc = this->Si570Divider(osc, values);
	if(rc){
        printf("Si570Divider Error(%d)\n",rc);
		return rc;
	}

	rc = this->Si570UnfreezeDCO(osc);
	if(rc){
        printf("Si570UnfreezeDCO Error(%d)\n",rc);
		return rc;
	}

	rc = this->Si570NewFreq(osc);
	if(rc){
        printf("Si570NewFreq Error(%d)\n",rc);
		return rc;
	}

// min. 10ms wait
	usleep(20000);

	// DCM Reset
	rc = this->vmei->vme_A32D32_write(this->baseaddress + 0x438 , 0);
	if(rc){
		return rc;
	}


	return 0;
}




/****************************************************************************************************/

#ifdef raus
int sis3316card::set_external_clock_multiplier(void){
	int rc;

	if(values == NULL){
		return -100;
	}
	if(osc > 3 || osc < 0){
		return -100;
	}

	rc = this->Si570FreezeDCO(osc);
	if(rc){
		return rc;
	}

	rc = this->Si570Divider(osc, values);
	if(rc){
		return rc;
	}

	rc = this->Si570UnfreezeDCO(osc);
	if(rc){
		return rc;
	}

	rc = this->Si570NewFreq(osc);
	if(rc){
		return rc;
	}

// min. 10ms wait
	usleep(20);

	// DCM Reset
	rc = this->vmei->vme_A32D32_write(this->baseaddress + 0x438 , 0);
	if(rc){
		return rc;
	}


	return 0;
}
#endif

/****************************************************************************************************/

void sis3316card::initcard()
{
    unsigned int data = 0;
    int return_code = 0;
    

    return_code = vmei->vme_A32D32_write(baseaddress + SIS3316_KEY_ADC_FPGA_RESET, 0x0 );
    usleep(20000);
    return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_KEY_RESET, 0x0);
    usleep(1000);
    return_code = vmei->vme_A32D32_read ( baseaddress + SIS3316_MODID, &modid);
	printf("vme_A32D32_read: data = 0x%08x     return_code = 0x%08x\n", modid, return_code);
    if(return_code == 0 ){
        unsigned int serial; // AGS -- output serial number
        return_code = vmei->vme_A32D32_read ( baseaddress + SIS3316_SERIAL_NUMBER_REG, &serial);
        printf("Serial Number = %d \n", serial & 0xffff);
        vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
        vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
        vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
        vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits

        vmei->vme_A32D32_read ( baseaddress + SIS3316_ADC_CH1_4_FIRMWARE_REG, &(adcfirmware[0]));
        printf("SIS3316_ADC_CH1_4_FIRMWARE_REG   = 0x%08x \n", adcfirmware[0]);
        vmei->vme_A32D32_read ( baseaddress + SIS3316_ADC_CH5_8_FIRMWARE_REG, &(adcfirmware[1]));
        printf("SIS3316_ADC_CH5_8_FIRMWARE_REG   = 0x%08x \n", adcfirmware[1]);
        vmei->vme_A32D32_read ( baseaddress + SIS3316_ADC_CH9_12_FIRMWARE_REG, &(adcfirmware[2]));
        printf("SIS3316_ADC_CH9_12_FIRMWARE_REG  = 0x%08x \n", adcfirmware[2]);
        vmei->vme_A32D32_read ( baseaddress + SIS3316_ADC_CH13_16_FIRMWARE_REG, &(adcfirmware[3]));
        printf("SIS3316_ADC_CH13_16_FIRMWARE_REG = 0x%08x \n\n", adcfirmware[3]);
        
        vmei->vme_A32D32_read ( baseaddress + SIS3316_ADC_CH1_4_STATUS_REG, &data);
        printf("SIS3316_ADC_CH1_4_STATUS_REG     = 0x%08x \n", data);
        vmei->vme_A32D32_read ( baseaddress + SIS3316_ADC_CH5_8_STATUS_REG, &data);
        printf("SIS3316_ADC_CH5_8_STATUS_REG     = 0x%08x \n", data);
        vmei->vme_A32D32_read ( baseaddress + SIS3316_ADC_CH9_12_STATUS_REG, &data);
        printf("SIS3316_ADC_CH9_12_STATUS_REG    = 0x%08x \n", data);
        vmei->vme_A32D32_read ( baseaddress + SIS3316_ADC_CH13_16_STATUS_REG, &data);
        printf("SIS3316_ADC_CH13_16_STATUS_REG   = 0x%08x \n\n", data);

        //clear data buffer? shouldnt need to
        resetAllFifos();
    }else{
        printf("SIS3316_MODID                  = 0x%08x     return_code = 0x%08x\n", data, return_code);
    }
}

/****************************************************************************************************/

sis3316card::~sis3316card()
{
}

/****************************************************************************************************/
void sis3316card::SetClockChoice(int clck_choice, int sharing)
{
    unsigned int iob_delay_value ;
    int return_code;
    clock_source_choice = clck_choice;
    sharingmode = sharing;
    // set clock , wait 20ms in sis3316_adc1->set_frequency
	// reset DCM in sis3316_adc1->set_frequency
	switch (clock_source_choice) {
	    case 0:
			return_code = set_frequency(0, freqPreset250MHz);
			if((adcfirmware[0]>>16)&0xFFFF==0x0250){
                            iob_delay_value = 0x48 ;
                        }else{
                            iob_delay_value = 0x00 ;
                        }
			break;
	    case 1:
			return_code = set_frequency(0, freqPreset125MHz);
			//iob_delay_value = 0x48 ;
			iob_delay_value   = 0x10 ;
                        break;
	    case 2:
			return_code = set_frequency(0, freqPreset62_5MHz);
			iob_delay_value = 0x10 ;
			break;
	    case 3:
			return_code = set_frequency(0, freqPreset25MHz);
			//iob_delay_value = 0x20 ; // copied from struck root gui
                        std::cout << "FIRMWARE HERE "<< adcfirmware << " firm[0] " << adcfirmware[0] << std::endl;
                        if(((adcfirmware[0]>>16)&0xFFFF)==0x0250){
                            iob_delay_value = 0x20 ;
                        }else{
                            iob_delay_value = 0x30 ;
                        }
                        break;
	    case 4:
			return_code = set_frequency(0, freqPreset125MHz);
			iob_delay_value = 0x48 ;
			break;
	    case 5:
			return_code = set_frequency(0, freqPreset125MHz);
			iob_delay_value = 0x48 ;
			break;
	}
    printf("clock set to %i \n",clock_source_choice); // AGS
    printf("iob_delay_value: 0x%x \n", iob_delay_value); // AGS
    if(return_code){
        printf("Error setting clock ERROR(%d)\n",return_code);
    }
	usleep(10000) ;
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG, 0xf00 ); // Calibrate IOB _delay Logic
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG, 0xf00 ); // Calibrate IOB _delay Logic
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG, 0xf00 ); // Calibrate IOB _delay Logic
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0xf00 ); // Calibrate IOB _delay Logic
	usleep(10000) ;
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG, 0x300 + iob_delay_value ); // set IOB _delay Logic
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG, 0x300 + iob_delay_value ); // set IOB _delay Logic
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG, 0x300 + iob_delay_value ); // set IOB _delay Logic
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0x300 + iob_delay_value ); // set IOB _delay Logic
	usleep(10000) ;
    
	// Setup clock sharing
    //TODO: move this section above the IOB Calibrate?
    // For multiple cards
	// Write to the first card:
	// Data = 0x10 to addr = baseaddress + 0x58 SIS3316_FP_LVDS_BUS_CONTROL
	if(sharing == 2) //this is the master
        	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_FP_LVDS_BUS_CONTROL, 0x13);
   	 else if( sharing == 1 )
		return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_FP_LVDS_BUS_CONTROL, 0x2);

	// Write to all cards:
	// Data = 0x02 to addr = baseaddress + 0x50 SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL
    if( sharing > 0 )
    {
        return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL, 0x02);
        // The following lines extracted from sis3316_adc::set_frequency method
        usleep(20);
        return_code = vmei->vme_A32D32_write ( baseaddress + 0x438, 0x0);
    }
    //The following line selects the CO on the NIM CO output
    return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_LEMO_OUT_CO_SELECT_REG, 0x1);

}
/****************************************************************************************************/
void sis3316card::SetBroadcastAddress(unsigned int newbroadcastaddr, bool enable, bool master)
{
    unsigned int data = 0;
    int return_code = 0;
    
    broadcastaddr = newbroadcastaddr;
    data=broadcastaddr;
    //enable Broadcast listen
    if(enable) data+=0x10;
	//Enabe Broadcast master
	if(master) data+=0x20;
    
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_CBLT_BROADCAST, data);
	printf(" Setting Broadcast register for card: 0x%08x 0x%08x\n",baseaddress,data);

}

/****************************************************************************************************/
void sis3316card::initcommon()
{   
    

    // frequency presets setup
	freqPreset62_5MHz[0] = 0x23;
	freqPreset62_5MHz[1] = 0xC2;
	freqPreset62_5MHz[2] = 0xBC;
	freqPreset62_5MHz[3] = 0x33;
	freqPreset62_5MHz[4] = 0xE4;
	freqPreset62_5MHz[5] = 0xF2;
    
	freqPreset125MHz[0] = 0x21;
	freqPreset125MHz[1] = 0xC2;
	freqPreset125MHz[2] = 0xBC;
	freqPreset125MHz[3] = 0x33;
	freqPreset125MHz[4] = 0xE4;
	freqPreset125MHz[5] = 0xF2;
    
	freqPreset250MHz[0] = 0x20;
	freqPreset250MHz[1] = 0xC2;
	freqPreset250MHz[2] = 0xBC;
	freqPreset250MHz[3] = 0x33;
	freqPreset250MHz[4] = 0xE4;
	freqPreset250MHz[5] = 0xF2;
 
        // AGS -- added vals from struckRootGUI/rootGUI/src/sis3316_class.cpp 
	freqPreset25MHz[0] = 0x29;
	freqPreset25MHz[1] = 0xC2;
	freqPreset25MHz[2] = 0xBC;
	freqPreset25MHz[3] = 0x89;
	freqPreset25MHz[4] = 0x4F;
	freqPreset25MHz[5] = 0xF9;

    
    vmei = 0;
    modid = 0;
    clock_source_choice = 0;
    nimtriginput = 0;
    nimtrigoutput_to = 0;
    nimtrigoutput_uo = 0;

    coincidenceEnable = 0;
    minimumCoincidentChannels = 0;
    coincMask = 0xFFFF;
    
    for(int iadc =0; iadc<SIS3316_ADCGROUP_PER_CARD; iadc++)
    {
        adcfirmware[iadc] = 0;
        adcheaderid[iadc] = (((unsigned int)(iadc))&0x3)<<22;
        gate_window_length_block[iadc] = 1000;
        pretriggerdelay_block[iadc] = 300;
        pretriggerdelaypg_block[iadc] = 1;
        sample_length_block[iadc] = gate_window_length_block[iadc];
        sample_start_block[iadc] = 0;
        addressthreshold[iadc] = 0x10000;
        dataformat_block[iadc] = 0x05050505; //Bit0:Save6QDC,Bit1:SaveQDC78,Bit2:SaveCFD //Typical 0x05050505
        dacoffset[iadc] = 0x8000; //2V Range: -1 to 1V 0x8000, -2V to 0V 13000
        firenable_block[iadc]=1;
        firlength_block[iadc] = 4;
        risetime_block[iadc] = 4;
        gaptime_block[iadc] = 4;
        fircfd_block[iadc] = 0x3; //CFD at 50%
        firthresh_block[iadc] = 1000.0;
        highenergysuppress_block[iadc] = 0;
        highenergythresh_block[iadc] = 0;
        triggerstatmode_block[iadc]=1;
        
        for(int iqdc = 0; iqdc<SIS3316_QDC_PER_CHANNEL; iqdc++)
        {
            qdcstart[iadc][iqdc]=100+iqdc*100;
            qdclength[iadc][iqdc]=100;
        }
    }
    for(int ichan =0; ichan<SIS3316_CHANNELS_PER_CARD; ichan++)
    {
        
        databuffer[ichan] = 0;
        databuffersize[ichan] = 0;
        databufferread[ichan]=0;
        
        trigconf[ichan] = 0x5;

        highenergythresh[ichan] = 0;
        termination[ichan] = 1;
        gain[ichan] = 1;
        
        firenable[ichan] = 1;
        firlength[ichan] = 4;
        risetime[ichan] = 4;
        gaptime[ichan] = 4;
        fircfd[ichan] = 0x3; //CFD at 50%
        firthresh[ichan] = 1000.0;
        highenergysuppress[ichan] = 0;
        highenergythresh[ichan] = 0;


    }
    broadcastaddr=0x30000000;
    

}

/****************************************************************************************************/

void sis3316card::ConfigureEventRegisters()
{
    unsigned int data = 0;
    unsigned int addr;
    int return_code = 0;
    
    for(int iadc = 0; iadc<SIS3316_ADCGROUP_PER_CARD; iadc++)
    {
        // Gate Window Length
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG;
        data = (gate_window_length_block[iadc] & 0xffff) ;
        return_code =  vmei->vme_A32D32_write ( addr , data );
        
        // Trigger Delay
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG;
        data = ( pretriggerdelay_block[iadc] & 0x7ff )
                | ( (0x1 & pretriggerdelaypg_block[iadc]) << 15);
        return_code =  vmei->vme_A32D32_write ( addr , data );
        
        
        // Raw Data Buffer Length
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG;
        data = ((sample_length_block[iadc] & 0xffff) << 16) + (sample_start_block[iadc] & 0xffff);
        return_code =  vmei->vme_A32D32_write ( addr , data );
        
        //Data Format Selections
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG;
        data = dataformat_block[iadc];
        return_code =  vmei->vme_A32D32_write ( addr , data );

        //Trigger Selections
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_EVENT_CONFIG_REG;
        data = 0;
        for(int ib = 0; ib < SIS3316_CHANNELS_PER_ADCGROUP; ib++)
        {
            data = data |  ( ( trigconf[iadc * SIS3316_CHANNELS_PER_ADCGROUP + ib] & 0xFF ) << (8*ib));
        }
        return_code =  vmei->vme_A32D32_write ( addr , data );
        
        // QDC Gates
        for(int igate = 0; igate<8; igate++)
		{
            addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)
                + SIS3316_ADC_CH1_4_ACCUMULATOR_GATE1_CONFIG_REG+0x4*igate;
            
            data= (qdcstart[iadc][igate]&0xFFFF) | (((qdclength[iadc][igate]-1)&0x1FF)<<16) ;
            return_code =  vmei->vme_A32D32_write ( addr , data );
        }
        
        // Address Threshold Registers
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)
            + SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG;
        data=addressthreshold[iadc];
        return_code = vmei->vme_A32D32_write ( addr, data  );

    }

  // // Clear LEDs
  return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_CONTROL_STATUS, 0x70000);
  // Enable LEDs for Sample and Bank Writing
  return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_CONTROL_STATUS, 0x70);
  
    // Configure NIM Trigger Input
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_NIM_INPUT_CONTROL_REG, ((0x1FFF & nimtriginput)));
    
    // Configure NIM Trigger Output, TO, UO, CO
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_LEMO_OUT_TO_SELECT_REG, (0xFFFFFFFF & nimtrigoutput_to));
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_LEMO_OUT_UO_SELECT_REG, (0xFFFFFFFF & nimtrigoutput_uo));

    // Configure Acquisition Control Register
    // BIT(5)  : FP-BUS Control as Veto Enable
    // BIT(6)  : FP-BUS Control2 as Timestamp Clear
    // BIT(8)  : External Trigger as Trigger Enable which should include both NIM Input and vme broadcasts to SIS3316_KEY_TRIGGER )
    // BIT(10) : External Timestamp Clear Function Enabled
    if(sharingmode>0){
         //Change from JASON
        //return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ACQUISITION_CONTROL_STATUS, BIT(10) | BIT(5) | BIT(6) | BIT(7) | BIT(8) );
        return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ACQUISITION_CONTROL_STATUS, BIT(10) | BIT(4) | BIT(6) | BIT(7) | BIT(8) );
    }else{
        return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_ACQUISITION_CONTROL_STATUS, BIT(10) | BIT(5) | BIT(8) );
    }
    

}

/****************************************************************************************************/

void sis3316card::ConfigureAnalogRegisters()
{
    unsigned int adata = 0;
    int return_code = 0;

    for(int iadc = 0; iadc<SIS3316_ADCGROUP_PER_CARD; iadc++)
    {
        adata = 0;
        for(int ic = 0; ic<SIS3316_CHANNELS_PER_ADCGROUP; ic++)
        {
            unsigned int tdata = 0x3 & gain[iadc*SIS3316_CHANNELS_PER_ADCGROUP+ic];
            if(termination[iadc*SIS3316_CHANNELS_PER_ADCGROUP+ic] == 0)
                tdata = tdata | 0x4;
            adata = adata | (tdata<<(ic*8));
        }
        printf("Gain Register 0x%08d 0x%08d\n",baseaddress + SIS3316_FPGA_ADC_REG_OFFSET*iadc + SIS3316_ADC_CH1_4_ANALOG_CTRL_REG,adata);
        return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_FPGA_ADC_REG_OFFSET*iadc + SIS3316_ADC_CH1_4_ANALOG_CTRL_REG, adata);
    }
    
    // set ADC chips via SPI
	for (int iadc=0;iadc<SIS3316_ADCGROUP_PER_CARD;iadc++) {
	      
              uint32_t write_code;
              if( __IS_16BIT_MODEL__ == 1 )
                write_code = 0x81001440; // Make sure the AD9268 output is LVDS, not CMOS
              else
                write_code = 0x81001400; // The AD9463 only has LVDS; bit 6 set to 0.
        
        cout << "*16bit reg write: " << write_code << endl;
	return_code = vmei->vme_A32D32_write ( baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SPI_CTRL_REG, write_code );
        usleep(100); //unsigned int uint_usec
        return_code = vmei->vme_A32D32_write ( baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SPI_CTRL_REG, write_code + 0x00400000 );
        //return_code = vmei->vme_A32D32_write ( baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SPI_CTRL_REG, write_code );
        usleep(100); //unsigned int uint_usec
		return_code = vmei->vme_A32D32_write ( baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SPI_CTRL_REG, 0x8100ff01 ); // SPI (OE)  update
		usleep(100); //unsigned int uint_usec
		return_code = vmei->vme_A32D32_write ( baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SPI_CTRL_REG, 0x8140ff01 ); // SPI (OE)  update
		usleep(100); //unsigned int uint_usec
	}
    
    //  set ADC offsets (DAC)
	for (int iadc=0;iadc<SIS3316_ADCGROUP_PER_CARD;iadc++) { // over all 4 ADC-FPGAs
		return_code = vmei->vme_A32D32_write ( baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG, 0x80000000 + 0x8000000 +  0xf00000 + 0x1);  // set internal Reference
		usleep(100); //this command needs 23us time to execute on board
		return_code = vmei->vme_A32D32_write ( baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG, 0x80000000 + 0x2000000 +  0xf00000 + ((dacoffset[iadc] & 0xffff) << 4) );  // clear error Latch bits
		usleep(100); 
		return_code = vmei->vme_A32D32_write ( baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG, 0xC0000000 );  // clear error Latch bits
		usleep(100); 
	}	

}

/****************************************************************************************************/

void sis3316card::ConfigureFIR()
{
    unsigned int addr;
    unsigned int data;
    int return_code;
    // set FIR Trigger Setup
	for (int ichan=0;ichan<SIS3316_CHANNELS_PER_CARD;ichan++) {
        // FIR Conf
        addr = baseaddress
                + ( (ichan/SIS3316_CHANNELS_PER_ADCGROUP) + 1)*SIS3316_FPGA_ADC_REG_OFFSET
                + 0x10*(ichan%SIS3316_CHANNELS_PER_ADCGROUP)
                + 0x40;
		return_code = vmei->vme_A32D32_write ( addr, 0) ;
		return_code = vmei->vme_A32D32_write ( addr, (risetime[ichan]&0xFFF) | ((gaptime[ichan]&0xFFF) << 12) | (0xF << 24)) ;
        
        //FIR Thresh
        addr = baseaddress
                + ( (ichan/SIS3316_CHANNELS_PER_ADCGROUP) + 1)*SIS3316_FPGA_ADC_REG_OFFSET
                + 0x10*(ichan%SIS3316_CHANNELS_PER_ADCGROUP)
                + 0x44;
        data= ( (0x1 & firenable[ichan]) << 31)
              | ( (0x1 & highenergysuppress[ichan]) << 30)
              | ( (0x3 & fircfd[ichan]) << 28 ) // different
              | (0x08000000 + (risetime[ichan] * firthresh[ichan]) );
		return_code = vmei->vme_A32D32_write ( addr, data) ;
		//printf("%08x written to FIR_THRESH channel %d \n", data, ichan);
        
        //High Energy Threshold
        addr = baseaddress
                 + ( (ichan/SIS3316_CHANNELS_PER_ADCGROUP) + 1)*SIS3316_FPGA_ADC_REG_OFFSET
                 + 0x10*(ichan%SIS3316_CHANNELS_PER_ADCGROUP)
                 + 0x48;
        data= 0xFFF & highenergythresh[ichan];
		return_code = vmei->vme_A32D32_write ( addr, data) ;

        // Internal Gate Delay configuration
        addr = baseaddress
                + ( (ichan/SIS3316_CHANNELS_PER_ADCGROUP) + 1)*SIS3316_FPGA_ADC_REG_OFFSET
                + 0x3C;
        data = 0x000F0A0A; // Enables gate 1 for all four channels, and sets lengths to 10 * (2*clock)
        return_code = vmei->vme_A32D32_write( addr, data );


        }
    
    // set FIR Block Trigger Setup, this determines what "SUM" trigger from FIR is. 
	for (int iadc=0;iadc<SIS3316_ADCGROUP_PER_CARD;iadc++) {
        // FIR Conf
        addr = baseaddress
            + (iadc + 1)*SIS3316_FPGA_ADC_REG_OFFSET
            + 0x80;
		return_code = vmei->vme_A32D32_write ( addr, 0) ;
		return_code = vmei->vme_A32D32_write ( addr, (risetime_block[iadc]&0xFFF) | ((gaptime_block[iadc]&0xFFF) << 12)) ;
        
        //FIR Thresh
        addr = baseaddress
            + (iadc + 1)*SIS3316_FPGA_ADC_REG_OFFSET
            + 0x84;
        data= ( (0x1 & firenable_block[iadc]) << 31)
        | ( (0x1 & highenergysuppress_block[iadc]) << 30)
        | ( (0x3 & fircfd_block[iadc]) << 28 )
        | (0x08000000 + (risetime_block[iadc] * firthresh_block[iadc]) );
		return_code = vmei->vme_A32D32_write ( addr, data) ;
        
        //High Energy Threshold
        addr = baseaddress
            + (iadc + 1)*SIS3316_FPGA_ADC_REG_OFFSET
            + 0x88;
        data= 0xFFF & highenergythresh_block[iadc];
		return_code = vmei->vme_A32D32_write ( addr, data) ;
        
        addr = baseaddress
                + SIS3316_FPGA_ADC_REG_OFFSET*iadc
                + SIS3316_ADC_CH1_4_TRIGGER_STATISTIC_COUNTER_MODE_REG;
        data=triggerstatmode_block[iadc];
        return_code = vmei->vme_A32D32_write ( addr, data) ;
	}
    

}

/****************************************************************************************************/
void sis3316card::ConfigureCoincidenceTable()
{

	std::cout << "CONFIGURING COINCIDENCE TABLE" << std::endl;
	unsigned int data_reg = baseaddress + SIS3316_TRIGGER_COINCIDENCE_LOOKUP_TABLE_DATA_REG;
	unsigned int ctrl_reg = baseaddress + SIS3316_TRIGGER_COINCIDENCE_LOOKUP_TABLE_CONTROL_REG;
	unsigned int adr_reg = baseaddress + SIS3316_TRIGGER_COINCIDENCE_LOOKUP_TABLE_ADDRESS_REG;
	unsigned int data;
	int return_code;



	// Clear the register and set the pulse length
	data = 0x80000000; // clear
	data += 0xf; // pulse length 16 sample clocks
	return_code = vmei->vme_A32D32_write( ctrl_reg, data ); 


	//loop over all possible channel combinations, count the 
	//number of '1' bits, if it passes minimum number of coincident
	//channels, set its validation register to 1.
	unsigned int bitcount = 0;
	unsigned int temp;
	for(unsigned int chComb = 0x0; chComb <= 0xFFFF; chComb++)
	{
		//count number of '1' bits in the word. 
		//using the subtraction by 1 method, see 
		//https://www.geeksforgeeks.org/count-set-bits-in-an-integer/
		bitcount = 0;
		temp = chComb;
		while(temp)
		{
			temp &= (temp - 1);
			bitcount++;
		}
		//set the validation based on whether it passes min number of channels
		if(bitcount >= minimumCoincidentChannels)
		{
			//select this combination of channels
			return_code = vmei->vme_A32D32_write(adr_reg, chComb);
			//allow it to trigger.
			return_code = vmei->vme_A32D32_write(data_reg, 1);
		}
		//otherwise set it to 0. 
		else
		{
			//select this combination of channels
			return_code = vmei->vme_A32D32_write(adr_reg, chComb);
			//allow it to trigger.
			return_code = vmei->vme_A32D32_write(data_reg, 0);
		}
	}


	//set the channel trigger mask in one swoop. 
	return_code = vmei->vme_A32D32_write(adr_reg, (coincMask << 16));
}


/****************************************************************************************************/

int sis3316card::AllocateDatabuffer(int ichan, unsigned int buffersize)
{
    delete [] databuffer[ichan];
    databuffer[ichan] = new unsigned int[buffersize];
    databuffersize[ichan] = buffersize;
    databufferread[ichan]=0;
    return buffersize;
    
};
/****************************************************************************************************/
int sis3316card::AllocateBuffers(unsigned int buffersize)
{
    if(buffersize==0)
    {
        for (int ichan=0;ichan<SIS3316_CHANNELS_PER_CARD;ichan++) {
            AllocateDatabuffer(ichan,databuffersize[ichan]);
        }
    }else{
        for (int ichan=0;ichan<SIS3316_CHANNELS_PER_CARD;ichan++) {
            AllocateDatabuffer(ichan,buffersize);
        }        
    }
    return 0;
}
/****************************************************************************************************/
int sis3316card::ResetRunScalars()
{
    for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
        for(int iscalar=0; iscalar<SIS3316_TRIGGER_STATS_PER_CHANNEL; iscalar++)
        {
            triggerstatrun[ichan][iscalar]=0;
            triggerstatspill[ichan][iscalar]=0;;
        }
    return 0;
}

/****************************************************************************************************/
int sis3316card::FetchScalars()
{
    //Copy previous values to runstats
    for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
        for(int iscalar=0; iscalar<SIS3316_TRIGGER_STATS_PER_CHANNEL; iscalar++)
        {
            triggerstatrun[ichan][iscalar]=triggerstatspill[ichan][iscalar];
        }

    for (int iadc=0;iadc<SIS3316_ADCGROUP_PER_CARD;iadc++) {
        vmei->vme_A32D32_write ( baseaddress +
                                SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (iadc*4), 0x80000000 + 0x30000000 ); // Space = Statistic counter
        // read from FIFO
        unsigned int got_nof_32bit_words = 0;
        unsigned int req_nof_32bit_words = 24 ;
        vmei->vme_A32BLT32FIFO_read ( baseaddress
                                           + SIS3316_FPGA_ADC1_MEM_BASE
                                           + (iadc*SIS3316_FPGA_ADC_MEM_OFFSET), triggerstatspill[iadc*4],
                                    req_nof_32bit_words,
                                     &got_nof_32bit_words);
    }
    return 0;
}

/****************************************************************************************************/
void sis3316card::LogScalars(std::ofstream &out){
    char buff[4096];

    for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
    {
        sprintf(buff,"%02d",ichan);
	out<<buff;
        for(int iscalar=0; iscalar<SIS3316_TRIGGER_STATS_PER_CHANNEL; iscalar++)
        {
	  sprintf(buff," %08d",triggerstatspill[ichan][iscalar]);
	  out<<buff;
        }
        sprintf(buff," (%f) ",databufferread[ichan]/double(0xfffffE)*100.0);
        out<<buff;
        out<<std::endl;
    }
}
/****************************************************************************************************/
int sis3316card::SetCardHeader(unsigned int cardhdr)
{
    unsigned int addr = 0;
    int return_code;
    for (int iadc=0;iadc<SIS3316_ADCGROUP_PER_CARD;iadc++) {
        adcheaderid[iadc] = (cardhdr<<24)|(((unsigned int)(iadc))&0x3)<<22;
        addr = baseaddress + iadc*SIS3316_FPGA_ADC_REG_OFFSET + SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG;
        if(vmei)
            return_code = vmei->vme_A32D32_write ( addr, adcheaderid[iadc]) ;
    }
    
    return 0;
}

/****************************************************************************************************/
int sis3316card::FetchDataForChannel(int ichan)
{
    FetchDataSizeForChannel(ichan);
    return FetchDataOnlyForChannel(ichan);
}


/****************************************************************************************************/
double sis3316card::FetchDataSizeForChannel(int ichan)
{
    int return_code = 0;
    int i_adc = ichan/4;
    unsigned int addr = 0;
    unsigned int max_poll_counter ;
    unsigned int expectedNumberOfWords = 0;
    unsigned int got_nof_32bit_words = 0;
    unsigned int prevBankReadBeginAddress = 0;
    unsigned int prevBankEndingRegister = baseaddress
    + SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG
    + i_adc*SIS3316_FPGA_ADC_REG_OFFSET
    + (ichan%4)*0x4;
    // Verify that the previous bank address is valid
    max_poll_counter = 10000; //in good cases, it always only takes 1 try! verified. 
    do {
        return_code = vmei->vme_A32D32_read (prevBankEndingRegister, &previousBankEndingAddress[ichan]); //
        if(return_code < 0) {
            printf("%d Address: %08x %d\n",max_poll_counter,previousBankEndingAddress[ichan],prevRunningBank-1);
            return return_code;
        }
        max_poll_counter--;
        if (max_poll_counter == 0) {
            std::cout<<"Max tries exceeded for channel: "<<ichan
          <<" card "<< ((baseaddress >> 20) & 0x3) <<", Previous Bank: "<< (prevRunningBank - 1) << " but read " << ((previousBankEndingAddress[ichan] & 0x1000000) >> 24 ) << std::endl;
            return 0x900;
        }
    } while (((previousBankEndingAddress[ichan] & 0x1000000) >> 24 )  != (prevRunningBank-1)) ; // previous Bank sample address is valid if bit 24 is equal bank2_read_flag
    
    prevBankReadBeginAddress = (previousBankEndingAddress[ichan] & 0x03000000) + 0x10000000*((ichan/2)%2);
    expectedNumberOfWords = previousBankEndingAddress[ichan] & 0x00FFFFFF;
    
    databufferread[ichan] = 0;
    //Fraction of buffer
    double fractionOfBuffer = expectedNumberOfWords/double(0xfffffE);
    std::cout << "Fraction of buffer for chan " << ichan << ": " << fractionOfBuffer  << std::endl;
    return fractionOfBuffer;
}

/****************************************************************************************************/
int sis3316card::FetchDataOnlyForChannel(int ichan)
{
    int return_code = 0;
    int i_adc = ichan/4;
    unsigned int addr = 0;
    unsigned int expectedNumberOfWords = 0;
    unsigned int got_nof_32bit_words = 0;
    unsigned int prevBankReadBeginAddress = 0;
    
    prevBankReadBeginAddress = (previousBankEndingAddress[ichan] & 0x03000000) + 0x10000000*((ichan/2)%2);
    expectedNumberOfWords = previousBankEndingAddress[ichan] & 0x00FFFFFF;

    
    databufferread[ichan] = 0;
    
    // Start FPGA Transfer Logic
    addr = baseaddress
           + SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG
           + i_adc*0x4;
    return_code = vmei->vme_A32D32_write ( addr , 0x80000000 + prevBankReadBeginAddress);
    if(return_code < 0) {
        printf("vme_A32D32_write: %d Address: %08x %08x %d\n",ichan, addr, 0x80000000 + prevBankReadBeginAddress, prevRunningBank-1);
        return return_code;
    }
    
    // Start the DMA Fifo Transfer
    addr = baseaddress
            + SIS3316_FPGA_ADC1_MEM_BASE
            +i_adc*SIS3316_FPGA_ADC_MEM_OFFSET;
    return_code = vmei->vme_A32_FastestFIFO_read( addr , databuffer[ichan], ((expectedNumberOfWords + 1) & 0xfffffE), &got_nof_32bit_words);

    //printf("Chan %d Received %d words\n",ichan,got_nof_32bit_words);
    if(return_code < 0) {
        printf("vme_A32MBLT64FIFO_read: %d Address: 0x%08x %d %d\treturn code from read %i\n",ichan, addr, expectedNumberOfWords, prevRunningBank-1, return_code);
        return return_code;
    }
    
    if((got_nof_32bit_words)!=((expectedNumberOfWords + 1) & 0xfffffE) | return_code == 0x121)
    {
        databufferread[ichan] = 0;
        std::cerr<<" Channel " <<(adcheaderid[0]>>24)<<":"<< ichan << " did not receive the expected number of words "
        <<got_nof_32bit_words<<"("<<((expectedNumberOfWords + 1) & 0xfffffE)<< ")" << std::endl;
        return 1;
    }else{
        databufferread[ichan] = expectedNumberOfWords;
    }
    
    //Struck code has a "reset FSM" at this stage. 
    addr = baseaddress + SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG
    + i_adc*0x4; 
    return_code = vmei->vme_A32D32_write(addr, 0x00000000);

    return 0;
}


int sis3316card::resetAllFifos()
{
	cout << "Resetting all fifos" << endl;
	int retval_sum = 0;
	int return_code = 0; 
	unsigned int addr;
	for(int i_adc = 0; i_adc < 4; i_adc++)
	{
		//Struck code has a "reset FSM" at this stage. 
	    //Struck code has a "reset FSM" at this stage. 
	    addr = baseaddress + SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG
	    + i_adc*0x4; 
	    return_code = vmei->vme_A32D32_write(addr, 0x00000000);
	    retval_sum += return_code;
	}
	return retval_sum;
}


/****************************************************************************************************/
int sis3316card::FetchDataForBlock(int iblock)
{
    for(int ic = 0; ic<4; ic++)
    {
        FetchDataForChannel(iblock*SIS3316_CHANNELS_PER_ADCGROUP + ic);
    }
    return 0;
}
/****************************************************************************************************/
int sis3316card::FetchAllData()
{
    for(int ichan = 0; ichan<SIS3316_CHANNELS_PER_CARD; ichan++)
    {
        FetchDataForChannel(ichan);
    }
    
    for(int ichan = 0; ichan<SIS3316_CHANNELS_PER_CARD; ichan++)
    {
        printf("DataRead[%02d] %08d (%02.1f%%)\n",ichan,databufferread[ichan],(databufferread[ichan])/(double)0x1000000*100.0);
    }

    return 0;
}
/****************************************************************************************************/
int sis3316card::EnableThresholdInterrupt()
{
    unsigned int data = 0;
    unsigned int addr;
    int return_code = 0;
    
    // VME IRQ generation
    data = 0x800 + 0x600 ;		// IRQ 6 enable
    addr = baseaddress + SIS3316_IRQ_CONFIG ;
    return_code = vmei->vme_A32D32_write(addr,data );

    data = 0x8 ;		// Enable IRQ Address Threshold Flag Level sensitive
    addr = baseaddress + SIS3316_IRQ_CONTROL ;
    return_code = vmei->vme_A32D32_write(addr,data );
    
    return 0;
}
/****************************************************************************************************/
bool sis3316card::DataThresholdReached(){
  int return_code;
  unsigned int ctlstatus = 0;
  return_code = vmei->vme_A32D32_read(baseaddress + SIS3316_IRQ_CONTROL,&ctlstatus);
  if(return_code < 0){
    SysError("sis3316card:DataThresholdReached","Error reading status from card at address 0x%08x\n",baseaddress);
    return false;
  }
  //this is effectively an "or" between the two IRQ bits active in 0x0000c000 (14 and 15),
  //because the and operation returns any integer > 0 if either of the bits are active, and
  //the if statement returns true if any integer > 0.  (Evan)
  if(ctlstatus&0x0000c000) return true;
  return false;
}
/****************************************************************************************************/
int sis3316card::ClearTimeStamp(){
    int return_code;
    return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_KEY_TIMESTAMP_CLEAR , 0);  //
    return 0;
}
/****************************************************************************************************/
int sis3316card::ArmBank()
{
    prevRunningBank=2;
    return DisarmAndArmBank();
}
/****************************************************************************************************/
int sis3316card::Disarm()
{
    int return_code;
    return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_KEY_DISARM , 0);
    return return_code;
}

/****************************************************************************************************/
int sis3316card::DisarmAndArmBank()
{
    int return_code;
    cout << "Previous bank is " << prevRunningBank << ", about to arm that bank" << endl;
    if(prevRunningBank==2){
        return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_KEY_DISARM_AND_ARM_BANK2 , 0);
        prevRunningBank=1;
    }else{
        return_code = vmei->vme_A32D32_write ( baseaddress + SIS3316_KEY_DISARM_AND_ARM_BANK1 , 0);
        prevRunningBank=2;
    }
    return 0;
}
/****************************************************************************************************/
int sis3316card::PrintRegisters()
{
    unsigned int dataShould;
    unsigned int data = 0;
    unsigned int addr;
    int return_code = 0;
 
    unsigned int serial; // AGS -- output serial number
    return_code = vmei->vme_A32D32_read ( baseaddress + SIS3316_SERIAL_NUMBER_REG, &serial);
    printf("Serial Number = %d \n", serial & 0xffff);

    return_code = vmei->vme_A32D32_read ( baseaddress + SIS3316_CBLT_BROADCAST, &data);
    printf("SIS3316_CBLT_BROADCAST 0x%08x\n",data);
    return_code = vmei->vme_A32D32_read ( baseaddress + SIS3316_IRQ_CONFIG, &data);
    printf("SIS3316_IRQ_CONFIG 0x%08x\n",data);
    return_code = vmei->vme_A32D32_read ( baseaddress + SIS3316_FP_LVDS_BUS_CONTROL, &data);
    printf("SIS3316_FP_LVDS_BUS_CONTROL 0x%08x\n",data);
    return_code = vmei->vme_A32D32_read ( baseaddress + SIS3316_ACQUISITION_CONTROL_STATUS, &data);
    printf("SIS3316_ACQUISITION_CONTROL_STATUS 0x%08x\n",data);
    
    for(int iadc = 0; iadc<SIS3316_ADCGROUP_PER_CARD; iadc++)
    {
        // Gate Window Length
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG;
        dataShould = (gate_window_length_block[iadc] & 0xffff) ;
        return_code =  vmei->vme_A32D32_read ( addr , &data );
        printf("SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG 0x%08x 0x%08x\n",data,dataShould);
       
        // Trigger Delay
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG;
        dataShould = ( pretriggerdelay_block[iadc] & 0x3f )
        | ( (0x1 & pretriggerdelaypg_block[iadc]) << 15);
        return_code =  vmei->vme_A32D32_read ( addr , &data );
        printf("SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG 0x%08x 0x%08x\n",data,dataShould);
        
        
        // Raw Data Buffer Length
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG;
        dataShould = ((sample_length_block[iadc] & 0xffff) << 16) + (sample_start_block[iadc] & 0xffff);
        return_code =  vmei->vme_A32D32_read ( addr , &data );
        printf("SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG 0x%08x 0x%08x\n",data,dataShould);
        
        //Data Format Selections
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG;
        dataShould = dataformat_block[iadc];
        return_code =  vmei->vme_A32D32_read ( addr , &data );
        printf("SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG 0x%08x 0x%08x\n",data,dataShould);
        
        //Trigger Selections
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_EVENT_CONFIG_REG;
        data = 0;
        for(int ib = 0; ib < SIS3316_CHANNELS_PER_ADCGROUP; ib++)
        {
            data = data |  ( ( trigconf[iadc * SIS3316_CHANNELS_PER_ADCGROUP + ib] & 0xFF ) << (8*ib));
        }
        dataShould=data;
        return_code =  vmei->vme_A32D32_read ( addr , &data );
        printf("SIS3316_ADC_CH1_4_EVENT_CONFIG_REG 0x%08x 0x%08x\n",data,dataShould);
        
        // QDC Gates
        for(int igate = 0; igate<8; igate++)
		{
            addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)
            + SIS3316_ADC_CH1_4_ACCUMULATOR_GATE1_CONFIG_REG+0x4*igate;
            
            dataShould= (qdcstart[iadc][igate]&0xFFFF) | (((qdclength[iadc][igate]-1)&0x1FF)<<16) ;
            return_code =  vmei->vme_A32D32_read ( addr , &data );
            printf("SIS3316_ADC_CH1_4_ACCUMULATOR_GATE1_CONFIG_REG 0x%08x 0x%08x\n",data,dataShould);
        }
        
        // Address Threshold Registers
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)
        + SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG;
        data=addressthreshold[iadc];
        dataShould=data;
        return_code =  vmei->vme_A32D32_read ( addr , &data );
        printf("SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG 0x%08x 0x%08x\n",data,dataShould);

        //Gain and offset
        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG;
        dataShould = dataformat_block[iadc];
        return_code =  vmei->vme_A32D32_read ( addr , &data );
        printf("SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG 0x%08x 0x%08x\n",data,dataShould);

        addr = baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET)  + SIS3316_ADC_CH1_4_ANALOG_CTRL_REG;
        dataShould = dataformat_block[iadc];
        return_code =  vmei->vme_A32D32_read ( addr , &data );
        printf("SIS3316_ADC_CH1_4_ANALOG_CTRL_REG 0x%08x 0x%08x\n",data,dataShould);
       
        // SPI Control registers
        return_code = vmei->vme_A32D32_read( baseaddress + (iadc*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SPI_CTRL_REG, &data );
        printf("SIS3316_ADC_CH1_4_SPI_CTRL_REG 0x%08x\n", data);

    }
    
    // Configure NIM Trigger Input
    dataShould=((0xFFFF & nimtriginput));
	return_code = vmei->vme_A32D32_read ( baseaddress + SIS3316_NIM_INPUT_CONTROL_REG, &data);
    printf("SIS3316_NIM_INPUT_CONTROL_REG 0x%08x 0x%08x\n",data,dataShould);
    
    // Configure NIM Trigger Output
    dataShould=(0xFFFFFFFF & nimtrigoutput_to);
	return_code = vmei->vme_A32D32_read( baseaddress + SIS3316_LEMO_OUT_TO_SELECT_REG, &data );
    printf("SIS3316_LEMO_OUT_TO_SELECT_REG 0x%08x 0x%08x\n",data,dataShould);

    // Configure NIM Trigger Output
    dataShould=(0xFFFFFFFF & nimtrigoutput_uo);
	return_code = vmei->vme_A32D32_read( baseaddress + SIS3316_LEMO_OUT_UO_SELECT_REG, &data );
    printf("SIS3316_LEMO_OUT_UO_SELECT_REG 0x%08x 0x%08x\n",data,dataShould);
    
    return 0;
}

bool sis3316card::IsBlockReadout(int iadc) const
{
    // Assume that if the first channel of a group has the trigger on sum bit set then
    // the adcgroup should be readout as a group
    if(trigconf[iadc*SIS3316_CHANNELS_PER_ADCGROUP]&0x2) return true;
    return false;
}
size_t sis3316card::WriteChannelToFile(int ichan, FILE* fileraw)
{
    size_t header_written = 0;
    size_t data_written = 0;
    int errorcode = 0;
    unsigned int hdrid = adcheaderid[ichan/SIS3316_ADCGROUP_PER_CARD]|(ichan%SIS3316_ADCGROUP_PER_CARD)<<20;
    
    header_written+= fwrite(&hdrid,0x4,1,fileraw)*0x4;
    errorcode = ferror(fileraw);
    if(errorcode){
        std::cerr<<"sis3316card::WriteChannelToFile: Error writing channel header to binary file "<<errorcode<<std::endl;
    }

    header_written+= fwrite(triggerstatspill[ichan],0x4,6,fileraw)*0x4;
    errorcode = ferror(fileraw);
    if(errorcode){
        std::cerr<<"sis3316card::WriteChannelToFile: Error writing trigger stat to binary file "<<errorcode<<std::endl;
    }
    
    header_written+= fwrite(&(databufferread[ichan]),0x4,1,fileraw)*0x4;
    errorcode = ferror(fileraw);
    if(errorcode){
        std::cerr<<"sis3316card::WriteChannelToFile: Error writing packet size to binary file "<<errorcode<<std::endl;
    }
    
    data_written+= fwrite(databuffer[ichan],0x4,databufferread[ichan],fileraw)*0x4;
    errorcode = ferror(fileraw);
    if(errorcode){
        std::cerr<<"sis3316card::WriteChannelToFile: Error writing packet to binary file "<<errorcode<<std::endl;
    }
    if(data_written > 0)
    {
    	cout << "Just wrote " << data_written/4.0 << " bytes waveform data to file in WriteChannelToFile for chan : " << ichan << endl;
    }

    size_t total_written = data_written + header_written;
    
    
    return total_written;
}

size_t sis3316card::WriteSpillToFile(FILE* fileraw)
{
    size_t written = 0;
    int errorcode = 0;

    unsigned int phdrid[2];
    phdrid[0] = adcheaderid[0]&0xFF000000;
    phdrid[1] = 0;
    written+= fwrite(phdrid,0x4,2,fileraw)*0x4;
    errorcode = ferror(fileraw);
    if(errorcode){
        std::cerr<<"sis3316card::WriteSpillToFile: Error writing spill header to binary file "<<errorcode<<std::endl;
    }
    for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
    {
        written+=WriteChannelToFile(ichan, fileraw);
    }
    return written;
}

size_t sis3316card::readFileWaiting(void * ptr, size_t size, size_t count, FILE * stream)
{
    size_t readcount = 0;
        
    int ntriestoread=0;
    fpos_t curpos;
    fgetpos(stream,&curpos);
    
    do{
        if(ntriestoread>0){
            fsetpos(stream,&curpos);
            usleep(1000000);
        }
        readcount= fread(ptr,size,count,stream);
        ntriestoread++;
    }while(readcount<count&&ntriestoread<20);
    return readcount;
}

void sis3316card::GetScalars(std::vector<unsigned int> & recordedThisSpill, std::vector<unsigned int> &recordedThisRun,
                             std::vector<unsigned int> & triggerredThisSpill, std::vector<unsigned int> &triggerredThisRun){
    if(recordedThisSpill.size()!=SIS3316_CHANNELS_PER_CARD) recordedThisSpill.resize(SIS3316_CHANNELS_PER_CARD);
    if(recordedThisRun.size()!=SIS3316_CHANNELS_PER_CARD) recordedThisRun.resize(SIS3316_CHANNELS_PER_CARD);
    if(triggerredThisSpill.size()!=SIS3316_CHANNELS_PER_CARD) triggerredThisSpill.resize(SIS3316_CHANNELS_PER_CARD);
    if(triggerredThisRun.size()!=SIS3316_CHANNELS_PER_CARD) triggerredThisRun.resize(SIS3316_CHANNELS_PER_CARD);
    
    for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
    {
        recordedThisRun[ichan] = triggerstatspill[ichan][1];
        triggerredThisRun[ichan] = triggerstatspill[ichan][0];
        recordedThisSpill[ichan] = triggerstatspill[ichan][1] - triggerstatrun[ichan][1];
        triggerredThisSpill[ichan] = triggerstatspill[ichan][0] - triggerstatrun[ichan][0];
    }
}

size_t sis3316card::ReadSpillFromFile(FILE* fileraw)
{
    size_t readbytes = 0;
    //Copy previous values to runstats
    for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
        for(int iscalar=0; iscalar<SIS3316_TRIGGER_STATS_PER_CHANNEL; iscalar++)
        {
            triggerstatrun[ichan][iscalar]=triggerstatspill[ichan][iscalar];
        }

    unsigned int phdrid[2];
    readbytes+= readFileWaiting(phdrid,0x4,2,fileraw)*0x4;
    for(int ichan = 0; ichan < SIS3316_CHANNELS_PER_CARD; ichan++)
    {
        unsigned int hdrid = 0;
        unsigned int wordsToRead = 0;
        readbytes+= readFileWaiting(&hdrid,0x4,1,fileraw)*0x4;
        readbytes+= readFileWaiting(triggerstatspill[ichan],0x4,6,fileraw)*0x4;
        readbytes+= readFileWaiting(&wordsToRead,0x4,1,fileraw)*0x4;
        readbytes+= readFileWaiting(databuffer[ichan],0x4,wordsToRead,fileraw)*0x4;
        databufferread[ichan]=wordsToRead;
    }
    return readbytes;
}

float sis3316card::ReadTemp()
{
    int return_code;
    unsigned int data;
    if(!vmei)
        return 0.0;
    return_code = vmei->vme_A32D32_read ( baseaddress + SIS3316_INTERNAL_TEMPERATURE_REG, &data);
    return ((((data&0x03FF)))/4.0);
}

unsigned int sis3316card::GetAcquisitionControl(){
  unsigned int data;
  vmei->vme_A32D32_read(baseaddress + SIS3316_ACQUISITION_CONTROL_STATUS,
                        &data);
  return data;
}

unsigned int sis3316card::GetActualSampleAddress(){
  unsigned int data;
  vmei->vme_A32D32_read(baseaddress + SIS3316_ADC_CH1_ACTUAL_SAMPLE_ADDRESS_REG,
                        &data);
  return data;
}

