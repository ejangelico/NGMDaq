#ifndef _SIS3302_CLASS_
#define _SIS3302_CLASS_

#include "sis3302.h"
#include "TObject.h"

#define SIS3302_CHANNELS_PER_CARD 8
#define SIS3302_ADCGROUP_PER_CARD 4
#define SIS3302_CHANNELS_PER_ADCGROUP 2
#define SIS3302_QDC_PER_CHANNEL 8

class vme_interface_class; //Forward Declaration

class sis3302card : public TObject
{
public:
    
	vme_interface_class *vmei; //!
	unsigned int  baseaddress;
    
private:

   public:
    
    unsigned int modid;
    unsigned int modfirmware;
    unsigned int adcheaderid[SIS3302_ADCGROUP_PER_CARD];
    unsigned int clock_source_choice; // 0 : 250MHz, 1 : 125MHz, 2 : 62.5MHz
    unsigned int broadcastaddr;
    
    
    //Analog
    unsigned short dacoffset[SIS3302_CHANNELS_PER_CARD];
    
    //FIR Single
    unsigned short firenable[SIS3302_CHANNELS_PER_CARD];
    unsigned short firlength[SIS3302_CHANNELS_PER_CARD];
    unsigned short fircfd[SIS3302_CHANNELS_PER_CARD];
    unsigned int firthresh[SIS3302_CHANNELS_PER_CARD];
    unsigned short risetime[SIS3302_CHANNELS_PER_CARD];
    unsigned short gaptime[SIS3302_CHANNELS_PER_CARD];

    //Event
    unsigned short trigconf[SIS3302_CHANNELS_PER_CARD]; //Bit0:Invert, Bit1:InternalBlockSum, Bit2:Internal, Bit3:External 
    
    //Event Block
    void setQDC(int block, int gate,int start, int length);
    unsigned short qdcstart[SIS3302_ADCGROUP_PER_CARD][SIS3302_QDC_PER_CHANNEL];
    unsigned short qdclength[SIS3302_ADCGROUP_PER_CARD][SIS3302_QDC_PER_CHANNEL];

    unsigned short gate_window_length_block[SIS3302_ADCGROUP_PER_CARD];
    unsigned short pretriggerdelay_block[SIS3302_ADCGROUP_PER_CARD];
    unsigned short sample_length_block[SIS3302_ADCGROUP_PER_CARD];
    unsigned short sample_start_block[SIS3302_ADCGROUP_PER_CARD];
    unsigned int dataformat_block[SIS3302_ADCGROUP_PER_CARD]; //Bit0:Save6QDC,Bit1:SaveQDC78,Bit2:SaveCFD //Typical 0x5
    unsigned int addressthreshold[SIS3302_ADCGROUP_PER_CARD];
    unsigned int databuffersize[SIS3302_CHANNELS_PER_CARD];
    
    unsigned int databufferread[SIS3302_CHANNELS_PER_CARD];
    //ROOT should not save the following raw data buffer
    unsigned int* databuffer[SIS3302_CHANNELS_PER_CARD]; //!
    unsigned int actualsamplevalues[SIS3302_CHANNELS_PER_CARD]; //!
    static int prevRunningBank; //!

	sis3302card(vme_interface_class *crate, unsigned int baseaddress);
	sis3302card();
    void initcard();
    void initcommon();
    virtual ~sis3302card();
    void SetClockChoice(int clck_choice, int sharing); //Sharing(0:NoSharing, 1:SharingSlave 2:SharingMaster
    void SetBroadcastAddress(unsigned int newbroadcastaddr, bool enable=true, bool master = false);
    void ConfigureEventRegisters();
    void ConfigureAnalogRegisters();
    void ConfigureFIR();
    int ReadActualSampleValues();
    int AllocateBuffers(unsigned int buffersize = 0x200000);
    int AllocateDatabuffer(int ichan, unsigned int buffersize = 0x200000 /*8MBytes*/);
    int FetchDataForChannel(int ichan);
    int FetchScalars();
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
    bool IsBlockReadout(int iblock) const;
    float ReadTemp();
    void ReadCurrentADCValues(unsigned short* adcvals);
    size_t WriteSpillToFile(FILE* fileraw);
    size_t WriteChannelToFile(int ichan, FILE* fileraw);
    size_t ReadSpillFromFile(FILE* fileraw);
    static size_t readFileWaiting(void * ptr, size_t size, size_t count, FILE * stream);

    ClassDef(sis3302card,3)
};




#endif
