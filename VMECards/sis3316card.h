#ifndef _SIS3316_CLASS_
#define _SIS3316_CLASS_

#include "sis3316.h"
#include "TObject.h"
#include <fstream>
#include <vector>

#define SIS3316_CHANNELS_PER_CARD 16
#define SIS3316_ADCGROUP_PER_CARD 4
#define SIS3316_TRIGGER_STATS_PER_CHANNEL   6
#define SIS3316_CHANNELS_PER_ADCGROUP 4
#define SIS3316_QDC_PER_CHANNEL 8
#define SIS3316_ADC_FPGA_BOOT_CSR		0x30
#define SIS3316_SPI_FLASH_CSR			0x34
#define ENABLE_SPI_PROG	0
#define CHIPSELECT_1		1
#define CHIPSELECT_2		2
#define FIFO_NOT_EMPTY		14
#define FLASH_LOGIC_BUSY	31

#define SIS3316_SPI_FLASH_DATA			0x38
#define SIS3316_FLASH_PROGRAM_PAGESIZE	256
#define SIS3316_FLASH_ERASE_BLOCKSIZE	65536

#define SIS3316_ADC_CLK_OSC_I2C_REG		0x40

class vme_interface_class; //Forward Declaration

class sis3316card : public TObject
{
public:
    
	vme_interface_class *vmei; //!
	unsigned int  baseaddress;
    
private:
	//SISInterface *i;
	// flash stuff
	int FlashEnableProg(void);
	int FlashDisableProg(void);
	int FlashEnableCS(int chip);
	int FlashDisableCS(int chip);
	int FlashWriteEnable(void);
	int FlashProgramPage(int address, char *data, int len);
	int FlashEraseBlock(int address);
	int FlashGetId(char *id);
	int FlashXfer(char in, char *out);
	// i2c stuff
	int I2cStart(int osc);
	int I2cStop(int osc);
	int I2cWriteByte(int osc, unsigned char data, char *ack);
	int I2cReadByte(int osc, unsigned char *data, char ack);
	int Si570FreezeDCO(int osc);
	int Si570Divider(int osc, unsigned char *data);
	int Si570UnfreezeDCO(int osc);
	int Si570NewFreq(int osc);
    public:
    
    unsigned int modid;
    unsigned int adcfirmware[SIS3316_ADCGROUP_PER_CARD];
    unsigned int adcheaderid[SIS3316_ADCGROUP_PER_CARD];
    unsigned int clock_source_choice; // 0 : 250MHz, 1 : 125MHz, 2 : 62.5MHz, 3: 25MHz
    unsigned int broadcastaddr;
    unsigned int sharingmode; // 0 single card, 1 shared slave, 2 shared master
    //Board Settings
    unsigned int nimtriginput; //Bit0 Enable : Bit1 Invert , For typical use 0x3
    unsigned int nimtrigoutput_to; //TO nim output, page 120
    unsigned int nimtrigoutput_uo; //UO nim output, page 121
    
    //coincidence settings
    bool coincidenceEnable;
    int minimumCoincidentChannels; 
    unsigned int coincMask; 
    unsigned int coincWindow;


    
    //Analog
    unsigned short termination[SIS3316_CHANNELS_PER_CARD];
    unsigned short gain[SIS3316_CHANNELS_PER_CARD];
    unsigned short dacoffset[SIS3316_ADCGROUP_PER_CARD];
    
    //FIR Single
    unsigned short firenable[SIS3316_CHANNELS_PER_CARD];
    unsigned short firlength[SIS3316_CHANNELS_PER_CARD];
    unsigned short fircfd[SIS3316_CHANNELS_PER_CARD];
    unsigned int firthresh[SIS3316_CHANNELS_PER_CARD];
    unsigned short risetime[SIS3316_CHANNELS_PER_CARD];
    unsigned short gaptime[SIS3316_CHANNELS_PER_CARD];
    unsigned short highenergysuppress[SIS3316_CHANNELS_PER_CARD];
    unsigned int highenergythresh[SIS3316_CHANNELS_PER_CARD];

    //FIR Block
    unsigned short firenable_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned short firlength_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned short fircfd_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned int firthresh_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned short risetime_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned short gaptime_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned short highenergysuppress_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned int highenergythresh_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned int triggerstatmode_block[SIS3316_ADCGROUP_PER_CARD];

    //Event
    unsigned short trigconf[SIS3316_CHANNELS_PER_CARD]; //Bit0:Invert, Bit1:InternalBlockSum, Bit2:Internal, Bit3:External 
    
    //Event Block
    void setQDC(int block, int gate,int start, int length);
    unsigned short qdcstart[SIS3316_ADCGROUP_PER_CARD][SIS3316_QDC_PER_CHANNEL];
    unsigned short qdclength[SIS3316_ADCGROUP_PER_CARD][SIS3316_QDC_PER_CHANNEL];

    unsigned short gate_window_length_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned short pretriggerdelay_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned short pretriggerdelaypg_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned short sample_length_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned short sample_start_block[SIS3316_ADCGROUP_PER_CARD];
    unsigned int dataformat_block[SIS3316_ADCGROUP_PER_CARD]; //Bit0:Save6QDC,Bit1:SaveQDC78,Bit2:SaveCFD //Typical 0x5
    unsigned int addressthreshold[SIS3316_ADCGROUP_PER_CARD];
    unsigned int databuffersize[SIS3316_CHANNELS_PER_CARD];
    
    // Trigger stats read from card
    unsigned int triggerstatrun[SIS3316_CHANNELS_PER_CARD][SIS3316_TRIGGER_STATS_PER_CHANNEL];
    unsigned int triggerstatspill[SIS3316_CHANNELS_PER_CARD][SIS3316_TRIGGER_STATS_PER_CHANNEL];
    unsigned int databufferread[SIS3316_CHANNELS_PER_CARD];
    // This is the size of the data for the buffer on the card
    // not necessarily how much is read
    unsigned int previousBankEndingAddress[SIS3316_CHANNELS_PER_CARD];
    //ROOT should not save the following raw data buffer
    unsigned int* databuffer[SIS3316_CHANNELS_PER_CARD]; //!
    static int prevRunningBank; //!

	sis3316card(vme_interface_class *crate, unsigned int baseaddress);
	sis3316card();
    void initcard();
    void initcommon();
    virtual ~sis3316card();
	int update_firmware(char *path, int offset, void (*cb)(int percentage));
	unsigned char freqPreset25MHz[6];
	unsigned char freqPreset62_5MHz[6];
	unsigned char freqPreset125MHz[6];
	unsigned char freqPreset250MHz[6];
    void SetClockChoice(int clck_choice, int sharing); //Sharing(0:NoSharing, 1:SharingSlave 2:SharingMaster
    void SetBroadcastAddress(unsigned int newbroadcastaddr, bool enable=true, bool master = false);
    void ConfigureEventRegisters();
    void ConfigureAnalogRegisters();
    void ConfigureFIR();
    void ConfigureCoincidenceTable();
	int set_frequency(int osc, unsigned char *values);
    int AllocateBuffers(unsigned int buffersize = 0x1000000);
    int AllocateDatabuffer(int ichan, unsigned int buffersize = 0x1000000 /*64MBytes*/); //64MB is maximum channel size
    int resetAllFifos();
    double FetchDataSizeForChannel(int ichan);
    int FetchDataForChannel(int ichan);
    int FetchDataOnlyForChannel(int ichan);
    int FetchScalars();
    void LogScalars(std::ofstream &out);
    int ResetRunScalars();
    int SetCardHeader(unsigned int cardhdr);
    int FetchDataForBlock(int iblock);
    int FetchAllData();
    int ArmBank();
    int Disarm();
    int DisarmAndArmBank();
    int ClearTimeStamp();
    int PrintRegisters();
    int EnableThresholdInterrupt();
    unsigned int GetAcquisitionControl();
    unsigned int GetActualSampleAddress();
  
  
    bool DataThresholdReached();
    bool IsBlockReadout(int iblock) const;
    float ReadTemp();
    size_t WriteSpillToFile(FILE* fileraw);
    size_t WriteChannelToFile(int ichan, FILE* fileraw);
    size_t ReadSpillFromFile(FILE* fileraw);
    static size_t readFileWaiting(void * ptr, size_t size, size_t count, FILE * stream);
    void GetScalars(std::vector<unsigned int> & recordedThisSpill, std::vector<unsigned int> &recordedThisRun,
                    std::vector<unsigned int> & triggerredThisSpill, std::vector<unsigned int> &triggerredThisRun);
    
    ClassDef(sis3316card,8)
};




#endif
