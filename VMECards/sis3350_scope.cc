/***************************************************************************/
/*                                                                         */
/*  Filename: sis3350_scope.c                                       */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                MKI                                              */
/*  date:                 30.08.2008                                       */
/*  last modification:                                                     */
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
/*  � 2008                                                                 */
/*                                                                         */
/***************************************************************************/


/*===========================================================================*/
/* Headers								     */
/*===========================================================================*/
#include <stdio.h>
#include <errno.h>
#include <string.h>
//#include <sys/types.h>
//#include <fcntl.h>

#include <stdlib.h>
#include <time.h>
#include <iostream>

#include <signal.h>

typedef unsigned char           u_int8_t;
typedef unsigned short          u_int16_t;

typedef unsigned long           U32;

#include <unistd.h>
#include "vme_interface_class.h"
#include "sis3350.h"

#define MAX_USB_DEV_NUMBER 4

#define VME_WRITE_MODE_D32			0x0
#define VME_WRITE_MODE_DMA_D32		0x1
#define VME_WRITE_MODE_BLT32		0x2
#define VME_WRITE_MODE_MBLT64		0x3

#define VME_READ_MODE_D32			0x0
#define VME_READ_MODE_DMA_D32		0x1
#define VME_READ_MODE_BLT32			0x2
#define VME_READ_MODE_MBLT64		0x3

#include "NGMHit.h"
#include "TFile.h"
#include "TTree.h"
#include "TRint.h"
#include "TApplication.h"
#include "TSystem.h"
#include "TString.h"
#include "TCanvas.h"
#include "TH2.h"
#include "TGraph.h"
#include "TStyle.h"

/*===========================================================================*/
/* Globals					  			     */
/*===========================================================================*/

#define MAX_NUMBER_LWORDS 0x1000000       /* 64MByte */
u_int32_t wblt_data[MAX_NUMBER_LWORDS] ;
u_int32_t rblt_data1[MAX_NUMBER_LWORDS] ;
u_int32_t rblt_data2[MAX_NUMBER_LWORDS] ;
u_int32_t rblt_data3[MAX_NUMBER_LWORDS] ;

bool gl_stopReq = false;

void program_stop_and_wait(void);
void CtrlHandler( int ctrlType );

/* --------------------------------------------------------------------------
 SIS3350 DAC Offsets
 module_addr			    vme module base address
 offset Value_array		DAC offset value (16 bit)
 -------------------------------------------------------------------------- */
int sis3350_write_dac_offset(vme_interface_class *vme_crate, unsigned int module_dac_control_status_addr, unsigned int dac_select_no, unsigned int dac_value )
{
	unsigned int i, error, temp;
	unsigned int return_code;
	unsigned int data, addr, base_addr;
	unsigned int max_timeout, timeout_cnt;
    
	
    
    data =  dac_value ;
    addr = module_dac_control_status_addr + 4 ; // DAC_DATA
    if ((error = vme_crate->vme_A32D32_write(addr,data )) != 0) {
        return -1;
    }
    
    data =  1 + (dac_select_no << 4); // write to DAC Register
    addr = module_dac_control_status_addr ;
    if ((error = vme_crate->vme_A32D32_write(addr,data )) != 0) {
        return -1;
    }
    
    max_timeout = 5000 ;
    timeout_cnt = 0 ;
    addr = module_dac_control_status_addr  ;
    do {
        if ((error = vme_crate->vme_A32D32_read(addr,&data )) != 0) {
            return -1;
        }
        timeout_cnt++;
    } while ( ((data & 0x8000) == 0x8000) && (timeout_cnt <  max_timeout) );
    
    if (timeout_cnt >=  max_timeout) {
        return -2 ;
    }
    
    
    
    data =  2 + (dac_select_no << 4); // Load DACs
    addr = module_dac_control_status_addr  ;
    if ((error = vme_crate->vme_A32D32_write(addr,data )) != 0) {
        return -1;
    }
    timeout_cnt = 0 ;
    addr = module_dac_control_status_addr  ;
    do {
        if ((error = vme_crate->vme_A32D32_read(addr,&data )) != 0) {
            return -1;
        }
        timeout_cnt++;
    } while ( ((data & 0x8000) == 0x8000) && (timeout_cnt <  max_timeout) )    ;
    
    if (timeout_cnt >=  max_timeout) {
        return -3 ;
    }
    
    
    
	return 0x0 ;
    
} // end function  ()


/*===========================================================================*/
/* Main      					  	                     */
/*===========================================================================*/
int main(int argc, char* argv[]) {
    
    CHAR char_messages[128];
    uint32_t nof_found_devices ;
    
    unsigned int i;
    
    unsigned int base_addr, addr;
    unsigned int no_of_lwords ;
    unsigned int get_lwords ;
    unsigned int no_of_lwords2 ;
    unsigned int get_lwords2 ;
    unsigned int no_of_lwords3 ;
    unsigned int get_lwords3 ;
    unsigned int readdatum ;
    unsigned int busy ;
    int arrayelement;
    int line,column;
    int maxspills=800;
    int maxeventsperspill=100;
    
    
    //SNS liquid scin settings
    /*
     int dSampleLength= 16376/8;
     int dPreSampleLength = 200;
     float dBaselineLength = 100;
     */
    int dSampleLength=16376;
    int dPreSampleLength=200;
    float dBaselineLength = 100;
    
    float prompt=0;
    float delayed=0;
    float baseline=0;
    if(argc>1){
        maxspills=TString(argv[1]).Atoi();
        printf("Using MaxSpills %d\n",maxspills);
    }
    if(argc>2){
        maxeventsperspill=TString(argv[2]).Atoi();
        printf("Using MaxEventsPerSpill %d\n",maxeventsperspill);
    }
    
    signal(SIGINT, CtrlHandler);
    
    int return_code ;
    int argcroot = 1;
    char** argvroot = 0;
    argvroot = (char**)(new size_t[argcroot]);
    for(int i = 0; i < 1; i++)
    {
        argvroot[i] = new char[TString(argv[i]).Length()+1];
        sprintf(argvroot[i],"%s",argv[i]);
    }
    
    TRint *theApp = new TRint("NGMBatch", &argcroot, argvroot);
    
    // see http://root.cern.ch/phpBB2/viewtopic.php?p=24177
    // for discussion on eliminating "Note: File <foo> already loaded." warnings
    TApplication::NeedGraphicsLibs();
    gApplication->InitializeGraphics();
    gStyle->SetOptStat(0);
    //gStyle->SetOptTitle(0);
    TCanvas* c1 = new TCanvas("c1","c1",1200,1000);
    c1->Divide(2,3);
    //TCanvas* c2 = new TCanvas("c2","c2 - peak");
    //TCanvas* c3 = new TCanvas("c3","c3 - baseline");
    //TCanvas* c4 = new TCanvas("c4","c4 - baseline RMS");
    //TCanvas* c5 = new TCanvas("c5","c5 - baseline RMS ABS");
    
    TFile* tout = TFile::Open("Test.root","RECREATE");
    TH2* hAxis = new TH2F("hAxis","sampled pulse",1000,0,dSampleLength,100,0,2400);  //4096
    TGraph* cGr = new TGraph(1000);
    //TGraph* cGrCFD = new TGraph(1000);
    TH1* hPeak = new TH1F("hPeak","hPeak",4096/8,0,4096);
    //TH1* hBaseline = new TH1F("hBaseline","hBaseline",4096,0,4096);
    TH1* hBaseline = new TH1F("hBaseline","hBaseline",2700,0,2700);
    TH1* hBaselineRMS = new TH1F("hBaselineRMS","hBaselineRMS",2000,-200.0,200.0);
    TH1* hBaselineRMSAbs = new TH1F("hBaselineRMSAbs","hBaselineRMSAbs",4096,0,4096);
    TH2* hPSD = new TH2F("hPSD","hPSD",1e3,0,2e4,1e2,0,2);
    
    cGr->SetLineColor(kBlue);
    int xVals[20000];
    for(int ival = 0; ival < 20000; ival++) xVals[ival] = ival;
    int yVals1[20000];
    int yVals2[20000];
    int yVals3[20000];
    bool dummyVMETest=false;
    vme_interface_class *vme_crate = 0;
    if(dummyVMETest)
        vme_crate = vme_interface_class::Factory("DUMMY");
    else
        vme_crate = vme_interface_class::Factory("SISUSB");
    if(!vme_crate)
    {
        printf("No VME Interface found\n");
        return 0;
    }
    
    return_code = vme_crate->vmeopen ();  // open Vme interface
    vme_crate->get_vmeopen_messages (char_messages, &nof_found_devices);  // open Vme interface
    printf("\n%s    (found %d vme interface device[s])\n\n",char_messages, nof_found_devices);
    
    if(return_code != 0x0) {
        //printf("ERROR: vme_crate->vmeopen: return_code = 0x%08x\n\n", return_code);
        program_stop_and_wait();
        return -1 ;
    }
	
	
	// set base address to factory dfault
    base_addr = 0x88000000 ;
    
    // key reset
	addr = base_addr + SIS3350_KEY_RESET;
    return_code = vme_crate->vme_A32D32_write( addr, 1) ;
    
	// set mode of operation bit 0 (ring buffer sync mode)
	// set internal channel trigger of operation bit 5
	// set multi-event operation bit 6
	addr = base_addr + SIS3350_ACQUISITION_CONTROL;
    return_code = vme_crate->vme_A32D32_write( addr, 0x50061) ;
    
	addr = base_addr + SIS3350_MULTIEVENT_MAX_NOF_EVENTS;
    return_code = vme_crate->vme_A32D32_write( addr, maxeventsperspill) ;  //events per spill
    
    
    // ring buffer length
	// set mode of operation bit 0 (ring buffer sync mode)
	addr = base_addr + SIS3350_RINGBUFFER_SAMPLE_LENGTH_ALL_ADC;
    //return_code = vme_crate->vme_A32D32_write( addr, 16376) ;
    return_code = vme_crate->vme_A32D32_write( addr, dSampleLength) ; //donny - shorten it up for Xylene
    //return_code = vme_crate->vme_A32D32_write( addr, 0x3ff8) ;
    //return_code = vme_crate->vme_A32D32_write( addr, 0x3ff8) ;
	
    // special header test
	// set T1/T2
	addr = base_addr + SIS3350_EVENT_CONFIG_ALL_ADC;
    //return_code = vme_crate->vme_A32D32_write( addr, 0x1) ;
    return_code = vme_crate->vme_A32D32_write( addr, 0x0) ;
    
    
    addr = base_addr + SIS3350_RINGBUFFER_PRE_DELAY_ALL_ADC;
    //return_code = vme_crate->vme_A32D32_write( addr, 1000) ;
    return_code = vme_crate->vme_A32D32_write( addr, dPreSampleLength) ;  //donny - shorten it up for Xylene
    
    addr = base_addr + SIS3350_ADC_VGA_ADC1;
    
    //return_code = vme_crate->vme_A32D32_write( addr, 10) ; //7.992V
    //return_code = vme_crate->vme_A32D32_write( addr, 47) ; //1.94V
    //return_code = vme_crate->vme_A32D32_write( addr, 22) ; //4.0V
    return_code = vme_crate->vme_A32D32_write( addr, 95) ; //0.950V
    //return_code = vme_crate->vme_A32D32_write( addr, 127) ; //0.720V
    
    addr = base_addr + SIS3350_ADC_VGA_ADC2;
    return_code = vme_crate->vme_A32D32_write( addr, 10) ; //4.0V
    addr = base_addr + SIS3350_ADC_VGA_ADC3;
    return_code = vme_crate->vme_A32D32_write( addr, 10) ; //4.0V
    addr = base_addr + SIS3350_ADC_VGA_ADC4;
    return_code = vme_crate->vme_A32D32_write( addr, 10) ; //4.0V
    
    
    //// At VGA 47
    //32719 - 1800
    //35000 - 1500
    //37000 - 1200
    //39000 -  860
    //41000 -  540
    //42000 -  385
    
    //// At VGA 10
    //42000 - 2400
    //44000 - 1615
    //47000 - 1510
    //50000 - 1410
    //65535
    
    sis3350_write_dac_offset(vme_crate,base_addr+ SIS3350_ADC12_DAC_CONTROL_STATUS, 0, 65535/2);
    usleep(10000);
    sis3350_write_dac_offset(vme_crate,base_addr+ SIS3350_ADC12_DAC_CONTROL_STATUS, 1, 65535/2);
    usleep(10000);
    sis3350_write_dac_offset(vme_crate,base_addr+ SIS3350_ADC34_DAC_CONTROL_STATUS, 0, 65535/2);
    usleep(10000);
    sis3350_write_dac_offset(vme_crate,base_addr+ SIS3350_ADC34_DAC_CONTROL_STATUS, 1, 65535/2);
    usleep(10000);
    
    //return_code = vme_crate->vme_A32D32_write( addr, 22) ; //4.0V
    
    
    //sis3350_write_dac_offset(base_addr+ SIS3350_ADC12_DAC_CONTROL_STATUS, 0, 35000);//1500
    
    usleep(10000);
    
    addr = base_addr + SIS3350_ADC_VGA_ADC1;
    return_code = vme_crate->vme_A32D32_read( addr, &readdatum) ;
    printf("VGA Setting %d\n",readdatum);
    
    addr = base_addr + SIS3350_ADC12_DAC_DATA;
    return_code = vme_crate->vme_A32D32_read( addr, &readdatum) ;
    if(return_code != 0) {
        printf("ERROR Reading DAC Value \n\n") ;
    }
    printf("DAC12 Setting 0x%x\n",readdatum);
    
    addr = base_addr + SIS3350_TRIGGER_SETUP_ADC1;
    // 8 Sample wide Trigger
    //return_code = vme_crate->vme_A32D32_write( addr, 0x5080808) ;
    // 16 Sample wide Trigger
    
    //The following is for negative signals
    return_code = vme_crate->vme_A32D32_write( addr, 0x5081010) ;
    //30 wide
    //return_code = vme_crate->vme_A32D32_write( addr, 0x5081E1E) ;
    
    addr = base_addr + SIS3350_TRIGGER_SETUP_ADC2;
    return_code = vme_crate->vme_A32D32_write( addr, 0x2081010) ;
    addr = base_addr + SIS3350_TRIGGER_SETUP_ADC3;
    return_code = vme_crate->vme_A32D32_write( addr, 0x2081010) ;
    addr = base_addr + SIS3350_TRIGGER_SETUP_ADC3;
    return_code = vme_crate->vme_A32D32_write( addr, 0x2081010) ;
    
    addr = base_addr + SIS3350_TRIGGER_THRESHOLD_ADC1;
    return_code = vme_crate->vme_A32D32_write( addr, 400) ;
    addr = base_addr + SIS3350_TRIGGER_THRESHOLD_ADC2;
    return_code = vme_crate->vme_A32D32_write( addr, 400) ;
    addr = base_addr + SIS3350_TRIGGER_THRESHOLD_ADC3;
    return_code = vme_crate->vme_A32D32_write( addr, 1000) ;
    addr = base_addr + SIS3350_TRIGGER_THRESHOLD_ADC4;
    return_code = vme_crate->vme_A32D32_write( addr, 1000) ;
    
    // special header test
	// enable special header
	addr = base_addr + 0x02000070;
    return_code = vme_crate->vme_A32D32_write( addr, 0x2000100) ;
	addr = base_addr + 0x02000074;
    return_code = vme_crate->vme_A32D32_write( addr, 0xa200210) ;
    
    const int headerWords = 4;
    NGMHit* hit1 = new NGMHitv6;
    NGMHit* hit2 = new NGMHitv6;
    NGMHit* hit3 = new NGMHitv6;
    
    //NGMHit* hit = hit1;
    NGMHit* cfd = new NGMHitv6;
    
    TTree* HitTree = new TTree("HitTree","HitTree");
    HitTree->Branch("h1",&hit1);
    HitTree->Branch("h2",&hit2);
    HitTree->Branch("h3",&hit3);
    // endless readout loop
	for (int i = 0; i<maxspills;i++) {
        // set start address
	    addr = base_addr + SIS3350_SAMPLE_START_ADDRESS_ALL_ADC;
        return_code = vme_crate->vme_A32D32_write( addr, 0x0) ;
        
		// arm digitizer
	    addr = base_addr + SIS3350_KEY_ARM  ;
        return_code = vme_crate->vme_A32D32_write( addr, 0) ;
        //		// issue VME key trigger
        //	    addr = base_addr + SIS3350_KEY_TRIGGER  ;
        //        return_code = vme_crate->vme_A32D32_write( addr, 0) ;
        
		// digitizer sampling -> wait for ADC sampling busy to clear
        addr = base_addr + SIS3350_ACQUISITION_CONTROL  ;
        int armed = 0;
		do {
            usleep(10000);
            //            // issue VME key trigger
            //            addr = base_addr + SIS3350_KEY_TRIGGER  ;
            //            return_code = vme_crate->vme_A32D32_write( addr, 0) ;
            //
            //            addr = base_addr + SIS3350_ACQUISITION_CONTROL  ;
            return_code = vme_crate->vme_A32D32_read( addr, &readdatum) ;
			busy = readdatum & SIS3350_ACQ_STATUS_BUSY_FLAG;
            armed = readdatum & SIS3350_ACQ_STATUS_ARMED_FLAG;
			//printf("waiting %x\n",readdatum);
		} while (busy==0&&armed&&!gl_stopReq);
        
        addr = base_addr + SIS3350_KEY_DISARM  ;
        return_code = vme_crate->vme_A32D32_write( addr, 0) ;
        
        return_code = vme_crate->vme_A32D32_read( base_addr+SIS3350_MULTIEVENT_EVENT_COUNTER, &readdatum) ;
        int eventsThisSpill = readdatum;
        //printf("Event Counter %d\n",eventsThisSpill);
        
        
        // if nofevent times event length greater than 16 MBytes , data must be retrieved in
        // multiple 4MB pages
        addr = base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC2  ;
        return_code = vme_crate->vme_A32D32_read( addr, &readdatum) ;
        int adc2datalength = readdatum;
        
        addr = base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC3  ;
        return_code = vme_crate->vme_A32D32_read( addr, &readdatum) ;
        int adc3datalength = readdatum;
        
        addr = base_addr + SIS3350_ACTUAL_SAMPLE_ADDRESS_ADC1  ;
        return_code = vme_crate->vme_A32D32_read( addr, &readdatum) ;
        int adc1datalength = readdatum;
        
        if(adc1datalength>=0x00800000)
        {
            printf("Data spans multiple pages. Condition not currently handled!");
            break;
        }
        
		// set number of long words
		no_of_lwords=adc1datalength;
        
        printf("Spill(%d) Event Counter %d 0x%x 0x%x 0x%x\n",i,eventsThisSpill,adc1datalength,adc2datalength,adc3datalength);
        
        // set memory page to 0 (needed once only in priciple)
	    addr = base_addr + SIS3350_ADC_MEMORY_PAGE_REGISTER ;
        return_code = vme_crate->vme_A32D32_write( addr, 0) ;
        usleep(10000);
        
        if(no_of_lwords==0){
            //printf("No data in channel 1\n");
            i--;
            continue;
        }
		// address of ADC1
		addr = base_addr + SIS3350_ADC1_OFFSET;
	    // read in BLT32 (MBLT64, 2eVMV and SST are possible here also)
        return_code =   vme_crate->vme_A32BLT32_read( addr, rblt_data1, adc1datalength, &get_lwords) ;
		// display number of read words
        addr = base_addr + SIS3350_ADC2_OFFSET;
        // Now readout the second ADC
        // read in BLT32 (MBLT64, 2eVMV and SST are possible here also)
        return_code =   vme_crate->vme_A32BLT32_read( addr, rblt_data2, adc2datalength, &get_lwords2) ;
        
        addr = base_addr + SIS3350_ADC3_OFFSET;
        // Now readout the second ADC
        // read in BLT32 (MBLT64, 2eVMV and SST are possible here also)
        return_code =   vme_crate->vme_A32BLT32_read( addr, rblt_data3, adc3datalength, &get_lwords3) ;

        // A few sanity checks
        if(no_of_lwords!=get_lwords)
        {
            printf("Incorrect number of words returned in block transfer %d out of %d\n",get_lwords,no_of_lwords);
            i--;
            break;
        }
        unsigned long long nsamples = (rblt_data1[3]&0xFFF) + ((rblt_data1[3]&0xFFF0000)>>4);

        
        if(i%1==0){
            printf(" %d got 0x%x long words out of 0x%x ADC1\n",i,get_lwords,no_of_lwords);
            printf(" %d got 0x%x long words out of 0x%x ADC2\n",i,get_lwords2,no_of_lwords2);
            // display header information
            printf("time stamp word 1: 0x%8.8x)\n",rblt_data1[0]);
            printf("time stamp word 2: 0x%8.8x)\n",rblt_data1[1]);
            printf("event info 1:      0x%8.8x)\n",rblt_data1[2]);
            printf("event info 2:      0x%8.8x)\n",rblt_data1[3]);
            printf("nsamples:      %d\n",(int)nsamples);
        }
        
        if(get_lwords!=get_lwords2)
        {
            printf("Packet sizes for this spill do not match.");
        }
        
        unsigned int thisEventBase = 0;
        unsigned int eventno = 0;
        NGMHit* goodHit = 0;
        while(thisEventBase<get_lwords/2)
        {
            // display header information
            nsamples = (rblt_data1[thisEventBase+3]&0xFFF) + ((rblt_data1[thisEventBase+3]&0xFFF0000)>>4);
            if(0){
                printf("Event Number %d\n",eventno);
                printf("time stamp word 1: 0x%8.8x)\n",thisEventBase+rblt_data1[0]);
                printf("time stamp word 2: 0x%8.8x)\n",thisEventBase+rblt_data1[1]);
                printf("event info 1:      0x%8.8x)\n",thisEventBase+rblt_data1[2]);
                printf("event info 2:      0x%8.8x)\n",thisEventBase+rblt_data1[3]);
                printf("nsamples:      %d\n",(int)nsamples);
            }
            if(nsamples>0x3ff8||nsamples<=0){
                printf("Bailing on this packet %d(%d) (%d,%d)\n",eventno,eventsThisSpill,thisEventBase,get_lwords);
                break;
            }
            hit1->SetNSamples(nsamples);
            hit2->SetNSamples(nsamples);
            hit3->SetNSamples(nsamples);
            //cfd=hit->FastFilter(20);
            if(!goodHit) goodHit=hit1->DuplicateHit();
            for(int isample = 0; isample<hit1->GetNSamples(); isample++)
            {
                if(isample%2==0){
                    hit1->SetSample(isample,0xFFF-rblt_data1[thisEventBase+headerWords+isample/2]&0xFFF);
                    hit2->SetSample(isample,0xFFF-rblt_data2[thisEventBase+headerWords+isample/2]&0xFFF);
                    hit3->SetSample(isample,0xFFF-rblt_data3[thisEventBase+headerWords+isample/2]&0xFFF);
                }else{
                    hit1->SetSample(isample,0xFFF-((rblt_data1[thisEventBase+headerWords+isample/2])>>16)&0xFFF);
                    hit2->SetSample(isample,0xFFF-((rblt_data2[thisEventBase+headerWords+isample/2])>>16)&0xFFF);
                    hit3->SetSample(isample,0xFFF-((rblt_data3[thisEventBase+headerWords+isample/2])>>16)&0xFFF);
                }
                yVals1[isample] = hit1->GetSample(isample);
                yVals2[isample] = hit2->GetSample(isample);
                yVals3[isample] = hit3->GetSample(isample);

            }
            if(0){
                double baseline=hit2->ComputeSum(0,int(dBaselineLength))/dBaselineLength;
                for(int isample=0;isample<100;isample++){
                    hBaselineRMS->Fill(hit1->GetSample(isample)-baseline);
                    hBaselineRMSAbs->Fill(hit1->GetSample(isample));
                }
                hBaseline->Fill(baseline);
                int maxvalue = hit2->getMaxValue();
                hPeak->Fill(maxvalue-baseline);
                //if(maxvalue<4094)
                
                baseline = hit1->ComputeSum(0,100);
                prompt = hit1->ComputeSum(140,25)-baseline/100.*25;
                delayed = hit1->ComputeSum(140,400)-baseline/100.*400;
                
                //std::cout << delayed << "/t" << prompt/delayed << std::endl;
                hPSD->Fill(delayed,prompt/delayed);
            }
            HitTree->Fill();
            thisEventBase+=headerWords+nsamples/2;
            
            //		arrayelement=0;
            //		for (line=0;line<0x110/4/2+1;line++) {
            //			for (column=0;column<4;column++) {
            //			   printf("0x%8.8x  ",rblt_data[arrayelement++]);
            //			}
            //			printf("\n");
            
            eventno++;
        }
        HitTree->AutoSave("selfsave");
        //		}
        if(i%1==0){
            c1->cd(1);
            hAxis->Draw("AXIS");
            if(goodHit)
            {
                cGr->SetLineColor(kBlack);
                cGr->DrawGraph(goodHit->GetNSamples(),xVals,yVals1,"L");
                cGr->SetLineColor(kRed);
                cGr->DrawGraph(goodHit->GetNSamples(),xVals,yVals2,"LSAME");
                cGr->SetLineColor(kBlue);
                cGr->DrawGraph(goodHit->GetNSamples(),xVals,yVals3,"LSAME");
            }
            delete goodHit;
            goodHit=0;
            c1->Modified();
            c1->Update();
            
            c1->cd(2);
            hPeak->Draw();
            c1->Modified();
            c1->Update();
            
            c1->cd(3);
            hBaseline->Draw();
            
            c1->cd(4);
            hBaselineRMS->Draw();
            
            c1->cd(5);
            hBaselineRMSAbs->Draw();
            
            c1->cd(6);
            hPSD->Draw("colz");
            c1->Update();
            
            gSystem->ProcessEvents();
        }
		//usleep(10000);
        if(gl_stopReq) break;
	}
    
    tout->Write();
    //tout->Close();
    theApp->Run();
    
    return 0;
}

void program_stop_and_wait(void)
{
	gl_stopReq = false;
	printf( "\n\nProgram stopped");
	printf( "\n\nEnter ctrl C");
	do {
		usleep(10000) ;
        gSystem->ProcessEvents();
	} while (gl_stopReq == false) ;
	
	//		result = scanf( "%s", line_in );
}



void CtrlHandler( int ctrlType ){
	switch( ctrlType ){
        case SIGINT:
            printf( "\n\nCTRL-C pressed. finishing current task.\n\n");
            gl_stopReq = true;
            //return( true );
            return;
            break;
        default:
            printf( "\n\ndefault pressed. \n\n");
            //return( false );
            return;
            break;
	}
}


