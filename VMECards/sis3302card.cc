#include<stdlib.h>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include "sis3302card.h"
#include "vme_interface_class.h"

//using namespace std;

/**************************************************************************************/

ClassImp(sis3302card)

int sis3302card::prevRunningBank = 0;

sis3302card::sis3302card()
{
    initcommon();
}

sis3302card::sis3302card(vme_interface_class *crate, unsigned int baseaddress)
{
    initcommon();
	if(crate){
		this->vmei = crate;
	}
	this->baseaddress = baseaddress;

}

void sis3302card::setQDC(int block, int gate,int start, int length)
{
    if(gate<0
	||gate>=SIS3302_QDC_PER_CHANNEL
   	||block<0
	||block>=SIS3302_ADCGROUP_PER_CARD) return;
    qdcstart[block][gate]=start;
    qdclength[block][gate]=length;
}

/****************************************************************************************************/

void sis3302card::initcard()
{
    int return_code = 0;
    return_code = vmei->vme_A32D32_write ( baseaddress + SIS3302_KEY_RESET, 0);
    usleep(10);
    return_code = vmei->vme_A32D32_read ( baseaddress + SIS3302_MODID, &modid);
	printf("vme_A32D32_read: data = 0x%08x     return_code = 0x%08x\n", modid, return_code);
}

/****************************************************************************************************/

sis3302card::~sis3302card()
{
}

int sis3302card::ReadActualSampleValues()
{
  unsigned int data = 0;
  unsigned int addr = 0;
  int return_code;
  for(int igrp = 0; igrp < SIS3302_ADCGROUP_PER_CARD; igrp++)
  {
    addr =  baseaddress + SIS3302_ACTUAL_SAMPLE_VALUE_ADC12 + (igrp)*SIS3302_NEXT_ADC_OFFSET;
    return_code = vmei->vme_A32D32_read ( addr, &data);
    actualsamplevalues[igrp*SIS3302_CHANNELS_PER_ADCGROUP] = data & 0xFFFF;
    actualsamplevalues[igrp*SIS3302_CHANNELS_PER_ADCGROUP+1] = (data>>16) & 0xFFFF;
  }

  printf("ADCs Addr:0x%08x  ", baseaddress);
  for(int ichan =0; ichan < SIS3302_CHANNELS_PER_CARD; ichan++)
  {
    printf(" 0x%04x",actualsamplevalues[ichan]);
  }
  printf("\n");
  return return_code;
}


/****************************************************************************************************/
void sis3302card::SetClockChoice(int clck_choice, int sharing)
{
    unsigned int data = SIS3302_ACQ_SET_CLOCK_TO_100MHZ;
    
    int return_code;
    clock_source_choice = clck_choice;
    // set clock , wait 20ms in sis3302_adc1->set_frequency
	// reset DCM in sis3302_adc1->set_frequency
	switch (clock_source_choice) {
        case  0: // intern 100 MHz
			data = SIS3302_ACQ_SET_CLOCK_TO_200MHZ ;	   //
			break;
        case  1: // intern 50 MHz
			data = SIS3302_ACQ_SET_CLOCK_TO_100MHZ ;	   //
			break;
        case  2: // intern 25 MHz
			data = SIS3302_ACQ_SET_CLOCK_TO_50MHZ ;	   //
 			break;
        case  3: // intern 62.25 MHz
			data = SIS3302_ACQ_SET_CLOCK_TO_LEMO_X5_CLOCK_IN;	   //
 			break;
        case  4: // extern LEMO
			data = SIS3302_ACQ_SET_CLOCK_TO_LEMO_DOUBLE_CLOCK_IN;	   //
 			break;
        case  5: // extern P2
			data = SIS3302_ACQ_SET_CLOCK_TO_P2_CLOCK_IN;	   //
 			break;
        case  6: // 2 x extern P2
			data = SIS3302_ACQ_SET_CLOCK_TO_LEMO_CLOCK_IN;	   //
 			break;
        case  7: // Dived by 2 and Multi 2 extern P2
			data = SIS3302_ACQ_SET_CLOCK_TO_P2_CLOCK_IN;	   //
 			break;
	}
    
    data = data | SIS3302_ACQ_ENABLE_LEMO_TRIGGER ;	   //
    
    data = data | SIS3302_ACQ_ENABLE_LEMO_TIMESTAMPCLR ;	   //
    //
    //data = data | SIS3302_ACQ_ENABLE_INTERNAL_TRIGGER ; // Enable Synced Trigger
    
    return_code = vmei->vme_A32D32_write ( baseaddress + SIS3302_ACQUISITION_CONTROL, data);

}
/****************************************************************************************************/
void sis3302card::SetBroadcastAddress(unsigned int newbroadcastaddr, bool enable, bool master)
{
    unsigned int data = 0;
    int return_code = 0;
    
    broadcastaddr = newbroadcastaddr;
    data=broadcastaddr;
    //enable Broadcast listen
    if(enable) data+=0x10;
	//Enabe Broadcast master
	if(master) data+=0x20;
    
	return_code = vmei->vme_A32D32_write ( baseaddress + SIS3302_CBLT_BROADCAST_SETUP, data);
	printf(" Setting Broadcast register for card: 0x%08x 0x%08x\n",baseaddress,data);

}

/****************************************************************************************************/
void sis3302card::initcommon()
{   
    

    
    vmei = 0;
    modid = 0;
    clock_source_choice = 0;

    
    for(int iadc =0; iadc<SIS3302_ADCGROUP_PER_CARD; iadc++)
    {
        adcheaderid[iadc] = 0;
        gate_window_length_block[iadc] = 1000;
        pretriggerdelay_block[iadc] = 300;
        sample_length_block[iadc] = gate_window_length_block[iadc];
        sample_start_block[iadc] = 0;
        addressthreshold[iadc] = 0x10000;
        dataformat_block[iadc] = 0x0505;
        
        for(int iqdc = 0; iqdc<SIS3302_QDC_PER_CHANNEL; iqdc++)
        {
            qdcstart[iadc][iqdc]=100+iqdc*100;
            qdclength[iadc][iqdc]=100;
        }
    }
    for(int ichan =0; ichan<SIS3302_CHANNELS_PER_CARD; ichan++)
    {
        
        databuffer[ichan] = 0;
        databuffersize[ichan] = 0;
        databufferread[ichan]=0;
        
        dacoffset[ichan] = 0x8000; //2V Range: -1 to 1V 0x8000, -2V to 0V 13000
        
        firenable[ichan] = 1;
        firlength[ichan] = 100;
        risetime[ichan] = 4;
        gaptime[ichan] = 4;
        fircfd[ichan] = 0x3; //CFD at Peak%
        firthresh[ichan] = 100;


    }
    broadcastaddr=0x30000000;
    

}

/****************************************************************************************************/

void sis3302card::ConfigureEventRegisters()
{
    unsigned int data = 0;
    unsigned int addr;
    int return_code = 0;
    
    for(int iadc = 0; iadc<SIS3302_ADCGROUP_PER_CARD; iadc++)
    {
        
        // Trigger Delay and Gate Window Length
        addr = baseaddress + (iadc*SIS3302_NEXT_ADC_OFFSET)  + SIS3302_PRETRIGGER_ACTIVEGATE_WINDOW_ADC12;
        data = ((pretriggerdelay_block[iadc] << 16) & 0x03ff0000) +  (gate_window_length_block[iadc] & 0x3ff);
        return_code =  vmei->vme_A32D32_write ( addr , data );
	printf("TriggerDelayAndWindow: 0x%08x 0x%08x\n",addr,data);
        usleep(1);
        
        // Raw Data Buffer Length
        addr = baseaddress + (iadc*SIS3302_NEXT_ADC_OFFSET)  + SIS3302_RAW_DATA_BUFFER_CONFIG_ADC12;
        data = ((sample_length_block[iadc] & 0x03ff) << 16) + (sample_start_block[iadc] & 0x03ff);
        return_code =  vmei->vme_A32D32_write ( addr , data );
	printf("RawBufferLength: 0x%08x 0x%08x\n",addr,data);
        usleep(1);
        
        //Header ID and Data Format Selections        
        addr = baseaddress + (iadc*SIS3302_NEXT_ADC_OFFSET) + SIS3302_EVENT_CONFIG_ADC12;
        data =  (adcheaderid[iadc]<<19) | (dataformat_block[iadc]&0xFFFF);
	printf("HeaderIdAndFormat: 0x%08x 0x%08x\n",addr,data);
        return_code =  vmei->vme_A32D32_write ( addr , data );
        usleep(1);

        // QDC Gates
        for(int igate = 0; igate<8; igate++)
		{
            addr = baseaddress + (iadc*SIS3302_NEXT_ADC_OFFSET)
                + SIS3302_ACCUMULATOR_GATE1_CONFIG_ADC12+0x4*igate;
            
            data= (qdcstart[iadc][igate]&0x3FF) | (((qdclength[iadc][igate]-1)&0x1FF)<<16) ;
            return_code =  vmei->vme_A32D32_write ( addr , data );
	    printf("GateSetup: 0x%08x 0x%08x\n",addr,data);
            usleep(1);
        }
        
        // Address Threshold Registers
        addr = baseaddress + (iadc*SIS3302_NEXT_ADC_OFFSET)
            + SIS3302_END_ADDRESS_THRESHOLD_ADC12;
        data=addressthreshold[iadc];
        return_code = vmei->vme_A32D32_write ( addr, data  );
	printf("AddressThreshold: 0x%08x 0x%08x\n",addr,data);
        usleep(1);
    }
    
    // Configure Acquisition Control Register
    // return_code = vmei->vme_A32D32_write ( baseaddress + SIS3320_ACQUISTION_CONTROL, BIT(10) | BIT(5) );
    // This write is performed in the SetClockChoice method

}

/****************************************************************************************************/

void sis3302card::ConfigureAnalogRegisters()
{
    unsigned int addr = 0;
    unsigned int data = 0;
    unsigned int rdata = 0;
    int return_code = 0;
	unsigned int max_timeout, timeout_cnt;
	unsigned int error;

    
    //  set ADC offsets (DAC)
	for (int ichan=0;ichan<SIS3302_CHANNELS_PER_CARD;ichan++) { // over all 8 Channels
        data=dacoffset[ichan];
        addr = baseaddress + SIS3302_DAC_DATA ;
        return_code = vmei->vme_A32D32_write (addr,data);
		usleep(1); //unsigned int uint_usec
        
        data =  1 + (ichan << 4); // write to DAC Register
		addr = baseaddress + SIS3302_DAC_CONTROL_STATUS  ;  //roe 14.12.09
        return_code = vmei->vme_A32D32_write (addr,data);
        usleep(1);
        
        max_timeout = 5000 ;
		timeout_cnt = 0 ;
		addr = baseaddress + SIS3302_DAC_CONTROL_STATUS  ;  //roe 14.12.09
		do {
			if ((error = vmei->vme_A32D32_read(addr,&rdata )) != 0) {
                std::cerr<<" Error setting DACs 1"<<std::endl;
				return;
			}
            usleep(1);
			timeout_cnt++;
		} while ( ((rdata & 0x8000) == 0x8000) && (timeout_cnt <  max_timeout) )    ;
        
		if (timeout_cnt >=  max_timeout) {
            std::cerr<<" Error Setting DACs 2"<<std::endl;
			return;
		}

        data =  2 + (ichan << 4); // Load DACs
		addr = baseaddress + SIS3302_DAC_CONTROL_STATUS  ;   //roe 14.12.09
        vmei->vme_A32D32_write (addr,data);
        usleep(1); //unsigned int uint_usec

        timeout_cnt = 0 ;
		addr = baseaddress + SIS3302_DAC_CONTROL_STATUS  ;		//roe 14.12.09
		do {
			if ((error = vmei->vme_A32D32_read(addr,&rdata )) != 0) {
                std::cerr<<" Error setting DACs 3"<<std::endl;
				return;
			}
            usleep(1);
			timeout_cnt++;
		} while ( ((rdata & 0x8000) == 0x8000) && (timeout_cnt <  max_timeout) )    ;

 		if (timeout_cnt >=  max_timeout) {
            std::cerr<<" Error Setting DACs 4"<<std::endl;
			return;
		}
       
	}	

}

/****************************************************************************************************/

void sis3302card::ConfigureFIR()
{
    unsigned int addr;
    unsigned int data;
    int return_code;
    // set FIR Trigger Setup
	for (int ichan=0;ichan<SIS3302_CHANNELS_PER_CARD;ichan++) {
        // FIR Conf
        addr = baseaddress + SIS3302_TRIGGER_SETUP_ADC1
                + (ichan/SIS3302_CHANNELS_PER_ADCGROUP)*SIS3302_NEXT_ADC_OFFSET
                + 0x8*(ichan%SIS3302_CHANNELS_PER_ADCGROUP);
        data = (risetime[ichan]&0x7F) | ((gaptime[ichan]&0x7F) << 8) | (firlength[ichan]&0x3F)<<16;
		return_code = vmei->vme_A32D32_write ( addr, 0) ;
		printf("TriggerSetup: 0x%08x 0x%08x\n",addr,data);
		return_code = vmei->vme_A32D32_write ( addr, data) ;
		usleep(1);
        //FIR Thresh
        addr = baseaddress + SIS3302_TRIGGER_THRESHOLD_ADC1
            + (ichan/SIS3302_CHANNELS_PER_ADCGROUP)*SIS3302_NEXT_ADC_OFFSET
            + 0x8*(ichan%SIS3302_CHANNELS_PER_ADCGROUP);
        data= ( (0x1 & (firenable[ichan]+1)) << 26) //This bit disables so add 1 before anding with 0x1
              | ( (0x3 & fircfd[ichan]) << 24 )
              | 0x10000 + firthresh[ichan] ;
		printf("TriggerThreshold: 0x%08x 0x%08x\n",addr,data);
		return_code = vmei->vme_A32D32_write ( addr, data) ;
		usleep(1);
	}
    
}
/****************************************************************************************************/

int sis3302card::AllocateDatabuffer(int ichan, unsigned int buffersize)
{
    delete [] databuffer[ichan];
    databuffer[ichan] = new unsigned int[buffersize];
    databuffersize[ichan] = buffersize;
    databufferread[ichan]=0;
    return buffersize;
    
};
/****************************************************************************************************/
int sis3302card::AllocateBuffers(unsigned int buffersize)
{
    if(buffersize==0)
    {
        for (int ichan=0;ichan<SIS3302_CHANNELS_PER_CARD;ichan++) {
            AllocateDatabuffer(ichan,databuffersize[ichan]);
        }
    }else{
        for (int ichan=0;ichan<SIS3302_CHANNELS_PER_CARD;ichan++) {
            AllocateDatabuffer(ichan,buffersize);
        }        
    }
    return 0;
}
/****************************************************************************************************/
int sis3302card::ResetRunScalars()
{

    return 0;
}

/****************************************************************************************************/
int sis3302card::FetchScalars()
{

    return 0;
}
/****************************************************************************************************/
int sis3302card::SetCardHeader(unsigned int cardhdr)
{
    for (int iadc=0;iadc<SIS3302_ADCGROUP_PER_CARD;iadc++) {
        adcheaderid[iadc] = cardhdr;
        //Will be uploaded with ConfigureEventRegisters
    }
    
    return 0;
}
/****************************************************************************************************/
int sis3302card::FetchDataForChannel(int ichan)
{
    int return_code = 0;
    int i_adc = ichan/SIS3302_CHANNELS_PER_ADCGROUP;
    unsigned int addr = 0;
    unsigned int data = 0;
    unsigned int max_poll_counter ;
    unsigned int expectedNumberOfWords = 0;
    unsigned int got_nof_32bit_words = 0;
    unsigned int prevBankReadBeginAddress = 0;
    unsigned int prevBankEndingAddress = 0;    
    unsigned int prevBankEndingRegister = baseaddress
                                         + SIS3302_PREVIOUS_BANK_SAMPLE_ADDRESS_ADC1
                                         + i_adc*SIS3302_NEXT_ADC_OFFSET
                                         + (ichan%SIS3302_CHANNELS_PER_ADCGROUP)*0x4;

    if(prevRunningBank==2)
        data = 0x4;
    else
        data = 0x0;
    addr = baseaddress + SIS3302_ADC_MEMORY_PAGE_REGISTER;
    return_code = vmei->vme_A32D32_write ( addr, data);
    usleep(1);
    // Read Ending Address
    addr = prevBankEndingRegister;
    return_code = vmei->vme_A32D32_read ( addr, &prevBankEndingAddress);
    printf("Previous Bank:%d Base:0x%08x Chan:%d Addr:0x%08x\n",data,baseaddress,ichan,prevBankEndingAddress);
    // check Ending address
    prevBankEndingAddress = prevBankEndingAddress & 0xffffff ; // mask bank2 address bit (bit 24)
    if (prevBankEndingAddress > 0x3fffff) {   // more than 1 page memory buffer is used
        std::cout<<" Data spans more than one page" <<std::endl;
    }
    
    expectedNumberOfWords = (prevBankEndingAddress & 0x3ffffc)>>1;
    databufferread[ichan] = 0;
    
    // Start the DMA Fifo Transfer
    addr = baseaddress
            + SIS3302_ADC1_OFFSET
            +ichan*SIS3302_NEXT_ADC_OFFSET;
    //return_code = vmei->vme_A32MBLT64FIFO_read ( addr , databuffer[ichan], ((expectedNumberOfWords + 1) & 0xfffffE), &got_nof_32bit_words);
    return_code = vmei->vme_A32MBLT64_read ( addr , databuffer[ichan], expectedNumberOfWords, &got_nof_32bit_words);
    if(return_code != 0) {
        printf("vme_A32MBLT64FIFO_read: %d Address: %08x %d %d\n",ichan, addr, expectedNumberOfWords, prevRunningBank-1);
        return return_code;
    }
    
    if((got_nof_32bit_words)!=expectedNumberOfWords)//((expectedNumberOfWords + 1) & 0xfffffE))
    {
        databufferread[ichan] = 0;
        std::cerr<<" Channel " <<(adcheaderid[0])<<":"<< ichan << " did not receive the expected number of words "
        <<got_nof_32bit_words<<"("<<((expectedNumberOfWords + 1) & 0xfffffE)<<std::endl;
        return 1;
    }else{
        databufferread[ichan] = expectedNumberOfWords;
    }
    
    if(got_nof_32bit_words!=expectedNumberOfWords) return 1;
    
    return 0;
}
/****************************************************************************************************/
int sis3302card::FetchDataForBlock(int iblock)
{
    for(int ic = 0; ic<4; ic++)
    {
        FetchDataForChannel(iblock*SIS3302_CHANNELS_PER_ADCGROUP + ic);
    }
    return 0;
}
/****************************************************************************************************/
int sis3302card::FetchAllData()
{
    for(int ichan = 0; ichan<SIS3302_CHANNELS_PER_CARD; ichan++)
    {
        FetchDataForChannel(ichan);
    }
    
    for(int ichan = 0; ichan<SIS3302_CHANNELS_PER_CARD; ichan++)
    {
        printf("DataRead[%02d] %08d (%02.1f%%)\n",ichan,databufferread[ichan],(databufferread[ichan])/(double)0x1000000*100.0);
    }

    return 0;
}
/****************************************************************************************************/
int sis3302card::EnableThresholdInterrupt()
{
    unsigned int data = 0;
    unsigned int addr;
    int return_code = 0;
    
    // VME IRQ generation
    data = 0x800 + 0x600 ;		// IRQ 6 enable
    addr = baseaddress + SIS3302_IRQ_CONFIG ;
    return_code = vmei->vme_A32D32_write(addr,data );

    data = 0x2 ;		// Enable IRQ Address Threshold Flag Level sensitive
    addr = baseaddress + SIS3302_IRQ_CONTROL ;
    return_code = vmei->vme_A32D32_write(addr,data );
    
    return 0;
}
/****************************************************************************************************/
int sis3302card::ClearTimeStamp(){
    int return_code;
    return_code = vmei->vme_A32D32_write ( broadcastaddr + SIS3302_KEY_TIMESTAMP_CLEAR , 0);  //
    return 0;
}
/****************************************************************************************************/
int sis3302card::ArmBank()
{
    prevRunningBank=1;
    return DisarmAndArmBank();
}
/****************************************************************************************************/
int sis3302card::Disarm()
{
    int return_code;
    return_code = vmei->vme_A32D32_write ( broadcastaddr + SIS3302_KEY_DISARM , 0);
    return return_code;
}

/****************************************************************************************************/
int sis3302card::DisarmAndArmBank()
{
    int return_code;
    if(prevRunningBank==2){
        return_code = vmei->vme_A32D32_write ( broadcastaddr + SIS3302_KEY_DISARM_AND_ARM_BANK2 , 0);
        prevRunningBank=1;
    }else{
        return_code = vmei->vme_A32D32_write ( broadcastaddr + SIS3302_KEY_DISARM_AND_ARM_BANK1 , 0);
        prevRunningBank=2;
    }
    return 0;
}
/****************************************************************************************************/
int sis3302card::PrintRegisters()
{
    unsigned int dataShould;
    unsigned int data = 0;
    unsigned int addr;
    int return_code = 0;
    return 0;
}

bool sis3302card::IsBlockReadout(int iadc) const
{
    return false;
}

size_t sis3302card::WriteChannelToFile(int ichan, FILE* fileraw)
{
    size_t written = 0;
    int errorcode = 0;
    unsigned int hdrid = ((adcheaderid[ichan/SIS3302_ADCGROUP_PER_CARD]<<3)|ichan)<<24;
    
    written+= fwrite(&hdrid,0x4,1,fileraw)*0x4;
    errorcode = ferror(fileraw);
    if(errorcode){
        std::cerr<<"sis3302card::WriteChannelToFile: Error writing channel header to binary file "<<errorcode<<std::endl;
    }
    
    written+= fwrite(&(databufferread[ichan]),0x4,1,fileraw)*0x4;
    errorcode = ferror(fileraw);
    if(errorcode){
        std::cerr<<"sis3302card::WriteChannelToFile: Error writing packet size to binary file "<<errorcode<<std::endl;
    }
    
    written+= fwrite(databuffer[ichan],0x4,databufferread[ichan],fileraw)*0x4;
    errorcode = ferror(fileraw);
    if(errorcode){
        std::cerr<<"sis3302card::WriteChannelToFile: Error writing packet to binary file "<<errorcode<<std::endl;
    }
    
    return written;
}

size_t sis3302card::WriteSpillToFile(FILE* fileraw)
{
    size_t written = 0;
    int errorcode = 0;

    unsigned int phdrid[2];
    phdrid[0] = adcheaderid[0]<<24&0xFF000000;
    phdrid[1] = 0;
    written+= fwrite(phdrid,0x4,2,fileraw)*0x4;
    errorcode = ferror(fileraw);
    if(errorcode){
        std::cerr<<"sis3302card::WriteSpillToFile: Error writing spill header to binary file "<<errorcode<<std::endl;
    }
    for(int ichan = 0; ichan < SIS3302_CHANNELS_PER_CARD; ichan++)
    {
        written+=WriteChannelToFile(ichan, fileraw);
    }
    return written;
}

size_t sis3302card::readFileWaiting(void * ptr, size_t size, size_t count, FILE * stream)
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

size_t sis3302card::ReadSpillFromFile(FILE* fileraw)
{
    size_t readbytes = 0;
    unsigned int phdrid[2];
    readbytes+= readFileWaiting(phdrid,0x4,2,fileraw)*0x4;
    //printf("Read packet header: 0x%08x\n",phdrid[0]);
    for(int ichan = 0; ichan < SIS3302_CHANNELS_PER_CARD; ichan++)
    {
        unsigned int hdrid = 0;
        unsigned int wordsToRead = 0;
        readbytes+= readFileWaiting(&hdrid,0x4,1,fileraw)*0x4;
        readbytes+= readFileWaiting(&wordsToRead,0x4,1,fileraw)*0x4;
	//printf("Read channel header: 0x%08x 0x%08x\n ",hdrid,wordsToRead);
        readbytes+= readFileWaiting(databuffer[ichan],0x4,wordsToRead,fileraw)*0x4;
        databufferread[ichan]=wordsToRead;
    }
    return readbytes;
}

void sis3302card::ReadCurrentADCValues(unsigned short* adcvals)
{
    for(int ichan = 0; ichan<8; ichan+=2)
    {
        int error = 0;
        unsigned int addr = baseaddress;
        unsigned int data = 0;
        if(ichan/2 == 0)
        {
            addr = addr | SIS3302_EVENT_CONFIG_ADC12;
        }else if(ichan/2 == 1){
            addr = addr | SIS3302_EVENT_CONFIG_ADC34;
        }else if(ichan/2 == 2){
            addr = addr | SIS3302_EVENT_CONFIG_ADC56;
        }else  {
            addr = addr | SIS3302_EVENT_CONFIG_ADC78;
        }
        addr = addr | 0x20;
        
        for(int isample = 0; isample < 10; isample++)
	    {
            if((error = vmei->vme_A32D32_read(addr,&data )))
            {
                return;
            }
            
            adcvals[ichan]=(data>>16);
            adcvals[ichan+1]=(data&0xFFFF);
	    }
        
        
    }

}

float sis3302card::ReadTemp()
{
    return 0.0;
}
