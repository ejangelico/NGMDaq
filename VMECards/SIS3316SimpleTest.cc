#include "TApplication.h"
#include "TObject.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TTimeStamp.h"
#include "TGraph.h"
#include "TArrayF.h"
#include "TSystem.h"
#include "TLatex.h"
#include "TGNumberEntry.h"
#include "TRootEmbeddedCanvas.h"
#include "TFile.h"
#include "TTree.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>
#include <csignal>

using namespace std;
#include "vme_interface_class.h"
#include "sis3316card.h"
#include "caenHV6533card.h"

#define MAX_NUMBER_LWORDS_64MBYTE			0x1000000       /* 64MByte */
unsigned int gl_wblt_data[MAX_NUMBER_LWORDS_64MBYTE] ;
unsigned int gl_rblt_data[MAX_NUMBER_LWORDS_64MBYTE] ;
#define MAX_NOF_SIS3316_ADCS			1
#define BROADCAST_BASE_ADDR				0x30000000
#define FIRST_MODULE_BASE_ADDR			0x31000000
#define MODULE_BASE_OFFSET				0x01000000
#define DEFAULT_SAMPLE_LENGTH 1000
#define MAX_NOF_SIS3316_QDCS 8
bool gl_stopReq = false;

void program_stop_and_wait(void);
void CtrlHandler( int ctrlType );

unsigned int computesum (unsigned int gateStart,unsigned int gateLength, unsigned short* ushort_adc_buffer_ptr )
{
	unsigned int sum = 0;
	for(int idx = gateStart; idx<gateStart+gateLength; idx++) sum+=ushort_adc_buffer_ptr[idx];
	return sum;
}
TFile* fout = 0;
TTree* treeOut = 0;
unsigned int mawmax;
unsigned int maw1;
unsigned int maw2;

unsigned short wf[4][200];
TH1* hEnergy = 0;
TH1* hPSDEnergy = 0;
TGraph* gr = 0;
int grx[1000];
int gry[1000];
void analyzeBlock(int iblock, sis3316card* vslot, TH2* flood)
{
    int dataformat = vslot->dataformat_block[iblock];
    int nomEventSize = 3;
    if(dataformat&0x1) nomEventSize+=7;
    if(dataformat&0x2) nomEventSize+=2;
    if(dataformat&0x4) nomEventSize+=3;
    int nevents = 0;
    unsigned int numberOfSamples = 0;
    unsigned int indexOfNextEvent = 0;
    while(indexOfNextEvent<vslot->databufferread[iblock*4])
    {
        numberOfSamples=vslot->databuffer[iblock*4][indexOfNextEvent+nomEventSize-1]&0x03FFFFFF;
        if(indexOfNextEvent==0)
        {
            printf("Header\n");
            for(int ihdr=0; ihdr<nomEventSize;ihdr++)
                printf("Hdr[%d] 0x%08x\n",ihdr,vslot->databuffer[iblock*4][indexOfNextEvent+ihdr]);
        }
        if(dataformat&0x1)
        {
            double pmt[4];
            double sum = 0.0;
            double psdsum = 0.0;
            int igate = 1;
            int psdgate = 2;
            if(dataformat&0x4)
            {
                int mawoffset=9;
                if(dataformat&0x2)
                    mawoffset+=2;
                mawmax = 0;
                maw1 =  0;
                maw2 = 0;
                for(int ipmt=0; ipmt<4; ipmt++)
                {
                    mawmax += vslot->databuffer[iblock*4+ipmt][indexOfNextEvent+mawoffset];
                    maw1 +=  vslot->databuffer[iblock*4+ipmt][indexOfNextEvent+mawoffset+1];
                    maw2 += vslot->databuffer[iblock*4+ipmt][indexOfNextEvent+mawoffset+2];
                }
            }

            for(int ipmt=0; ipmt<4; ipmt++)
            {
                double baseline = (vslot->databuffer[iblock*4+ipmt][indexOfNextEvent+3] & 0xFFFFFF)/(double)(vslot->qdclength[iblock][0]);
                pmt[ipmt] = (vslot->databuffer[iblock*4+ipmt][indexOfNextEvent+igate+3]& 0xFFFFFFF) - baseline*(vslot->qdclength[iblock][igate]);
                sum+=pmt[ipmt];
                psdsum+= (vslot->databuffer[iblock*4+ipmt][indexOfNextEvent+psdgate+3]& 0xFFFFFFF) - baseline*(vslot->qdclength[iblock][psdgate]);
            }
            hEnergy->Fill(sum);
            hPSDEnergy->Fill(sum,psdsum/sum);
            double x = (pmt[2]+pmt[3])/sum;
            double y = (pmt[0]+pmt[2])/sum;
            flood->Fill(x,y);
            
        }
        for(int ipmt=0; ipmt<4; ipmt++)
        {
            unsigned short* addressOfWaveform = (unsigned short*)(&(vslot->databuffer[iblock*4+ipmt][indexOfNextEvent+nomEventSize]));
            memcpy(wf[ipmt],addressOfWaveform,200*sizeof(unsigned short));
        }
        treeOut->Fill();
        
        if(nevents==0&&numberOfSamples>0)
        {
            TString gOpt("L");
            for(int ipmt=0; ipmt<4; ipmt++)
            {
                unsigned short* addressOfWaveform = (unsigned short*)(&(vslot->databuffer[iblock*4+ipmt][indexOfNextEvent+nomEventSize]));
                for(int isample=0; isample< numberOfSamples*2; isample++)
                {
                    gry[isample]=addressOfWaveform[isample];
                }
                //printf("Plot %d %d\n",iblock*4+ipmt,gry[0]);
                gr->SetLineColor(kBlack+ipmt);
                gr->DrawGraph(numberOfSamples*2,grx,gry,gOpt.Data());
                gOpt = "LSAME";
            }
        }
        
        indexOfNextEvent+=nomEventSize+numberOfSamples;
        nevents++;
    }
}

int main(int argc, char* argv[])
{
    
    CHAR char_messages[128];
    uint32_t nof_found_devices ;

    int return_code ;
    unsigned int first_mod_base, nof_modules   ;
    unsigned int module_base_addr   ;
    unsigned int addr, data;
    unsigned int i, module_index;
    unsigned int loop_counter;
    unsigned int error_loop_counter;

    uint32_t req_nof_32bit_words, got_nof_32bit_words;
    unsigned int sample_length;
    unsigned int sample_start_index;
    unsigned int trigger_gate_window_length;
    unsigned int address_threshold;
    unsigned int nof_events;
    unsigned int pre_trigger_delay ;
    unsigned int bank1_armed_flag ;
    unsigned int poll_counter ;
    unsigned int i_adc;
    unsigned int memory_bank_offset_addr ;
    unsigned int ch_last_bank_address_array[16];
    int pheightTot;
    bool dummyVMETest = false;
    // create SIS3150USB vme interface device
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

    signal(SIGINT, CtrlHandler);
    
    for(int ix = 0; ix <1000; ix++)
    {
        grx[ix]=ix;
    }
    
    TApplication theApp("SIS3316 Application: Test", &argc, (char**)argv);
    TCanvas* cSISScope = new TCanvas("cSISScope","SIS Scope");
    TH2* hWaveformAxis = new TH2D("hWaveformAxis","hWaveformAxis",100,0,1000,100,0,(1<<14));
    hWaveformAxis->Draw("AXIS");
    TGraph* gWaveform = new TGraph(DEFAULT_SAMPLE_LENGTH);
    Double_t* wavex = new Double_t[DEFAULT_SAMPLE_LENGTH];
    Double_t* wavey = new Double_t[DEFAULT_SAMPLE_LENGTH];
    for(int i = 0; i< DEFAULT_SAMPLE_LENGTH; i++) wavex[i]=(Double_t)i;
    
    fout = TFile::Open("Out.root","RECREATE");
    treeOut = new TTree("waveform","SIS3316 Block Waveforms");
    treeOut->Branch("wf_br",wf,"wf[4][200]/s");
    treeOut->Branch("mmax_br",&mawmax,"mmax/i");
    treeOut->Branch("m1_br",&maw1,"m1/i");
    treeOut->Branch("m2_br",&maw2,"m2/i");
 	TCanvas* cEnergy = new TCanvas("cEnergy","cEnergy");
	hEnergy = new TH1F("hEnergy","hEnergy",1024,0,1000);
	hEnergy->GetXaxis()->SetCanExtend(true);
 	TCanvas* cPSDEnergy = new TCanvas("cPSDEnergy","cPSDEnergy");
	hPSDEnergy = new TH2D("hPSDEnergy","hPSDEnergy",1024,0,(double)(1<<14),1000,0.0,2.0);
    gr = new TGraph(1000);
    
 	TCanvas* cFlood = new TCanvas("cFlood","cFlood");
	TH2* hFlood = new TH2I("hFlood","hFlood",512*1.2,-0.1,1.1,512*1.2,-0.1,1.1);
    hFlood->Draw("colz");
    
    caenHV6533card* hvcard = new caenHV6533card(vme_crate,0x40000000);
    hvcard->initcard();
    
    hvcard->SetVoltage(0,0.0);
    hvcard->EnableChannel(0,false);
    
    //return 0;
    
    unsigned int cardNum [ ] = {0x31000000, 0x32000000, 0x33000000, 0x34000000, 0x35000000};
	sis3316card  *sis3316_adca[MAX_NOF_SIS3316_ADCS];
	int islot;
	unsigned int header_length = 3;
	//commenting out next line and making the moddule_base_address be an array value in a for loop
	//module_base_addr = 0x31000000 ;
	for (islot=0; islot<MAX_NOF_SIS3316_ADCS; islot++){
		sis3316_adca[islot] = new sis3316card(vme_crate,cardNum[islot]);
        sis3316_adca[islot]->initcard();
        sis3316_adca[islot]->AllocateBuffers();
	}
	TArrayD evtESums[MAX_NOF_SIS3316_ADCS][SIS3316_CHANNELS_PER_CARD];
    
	for (islot=0; islot<MAX_NOF_SIS3316_ADCS; islot++){
        sis3316card* vslot = sis3316_adca[islot];
        if(!dummyVMETest)
            vslot->SetClockChoice(0,0);
        vslot->SetBroadcastAddress(BROADCAST_BASE_ADDR,true,islot==0/*first card is broadcast master*/);
        /*************Event Configuration Registers*******************/
        for(int iadc=0; iadc < SIS3316_ADCGROUP_PER_CARD;iadc++ ){
			vslot->gate_window_length_block[iadc]=200;
            vslot->pretriggerdelay_block[iadc]=0x50;
            vslot->sample_length_block[iadc]=200;
            vslot->sample_start_block[iadc]=0;
            vslot->pretriggerdelaypg_block[iadc]=0;
            for(int igate=0;igate<MAX_NOF_SIS3316_QDCS;igate++){
                vslot->qdcstart[iadc][igate]=80;
                vslot->qdclength[iadc][igate]=10;
            }
            vslot->qdcstart[iadc][0]=0;
			vslot->qdclength[iadc][0]=60;
			vslot->qdcstart[iadc][1]=180;
			vslot->qdclength[iadc][1]=1;
			vslot->qdcstart[iadc][2]=74;
			vslot->qdclength[iadc][2]=1;
            vslot->addressthreshold[iadc]=0x100;
        }
        
        for (int iadc=0;iadc<SIS3316_ADCGROUP_PER_CARD;iadc++)
        {
            vslot->dataformat_block[iadc] = 0x07070707; //
        }
        vslot->nimtriginput=0x0; //Disable:0x0 Enable:0x1 Enable+Invert:0x3
        vslot->nimtrigoutput_to=0x0;
        vslot->nimtrigoutput_uo=0x0;
        for (int ichan=0;ichan<SIS3316_CHANNELS_PER_CARD;ichan++) {
                if (ichan<4)
                {
                    vslot->trigconf[ichan] = 0x2;
                }else{
                    vslot->trigconf[ichan] = 0x0;
                }
        }
        vslot->ConfigureEventRegisters();
        /************Configure Analog Registers*******************/
        for(int ic = 0; ic<SIS3316_CHANNELS_PER_CARD;ic++)
        {
            vslot->gain[ic]=1; //0:5VRange 1:2VRange
            vslot->termination[ic]=1;// 1:50Ohm 0:1kOhm
        }
        vslot->ConfigureAnalogRegisters();
        /*********Configure Triggering*****************************************/
        for (int ichan=0;ichan<SIS3316_CHANNELS_PER_CARD;ichan++) {
            vslot->risetime[ichan]=4;
            vslot->gaptime[ichan]=0;
            vslot->firenable[ichan]=0;
            vslot->highenergysuppress[ichan]=0;
            vslot->fircfd[ichan]=0x3;
            vslot->firthresh[ichan]=120;
            vslot->highenergythresh[ichan]=0;
        }
        for (int iadc=0;iadc<SIS3316_ADCGROUP_PER_CARD;iadc++) {
            vslot->risetime_block[iadc]=4;
            vslot->gaptime_block[iadc]=0;
            vslot->firenable_block[iadc]=1;
            vslot->highenergysuppress_block[iadc]=0;
            vslot->fircfd_block[iadc]=0x3;
            vslot->firthresh_block[iadc]=120;
            vslot->highenergythresh_block[iadc]=0;
        }
        vslot->ConfigureFIR();
        /***************************************************/
        vslot->EnableThresholdInterrupt();
        printf("Slot %d Modid %x Firmware 0x%08x\n",islot,vslot->modid,vslot->adcfirmware[0]);
        vslot->PrintRegisters();
	}

    //return 0;
    //DAQ Loop
    // Clear Timestamp  */
	return_code = sis3316_adca[0]->ClearTimeStamp();  //
    
    // Start Readout Loop  */
    return_code = sis3316_adca[0]->DisarmAndArmBank();  //

    unsigned int triggerCount;
    unsigned int data_rd = 0;
    do {
        triggerCount = 0;
		poll_counter = 0 ;
        do {
			poll_counter++;
			vme_crate->vme_IRQ_Status_read( &data_rd);
            usleep(100000);
            if(poll_counter%1==0&&false){
                triggerCount++;
                return_code = vme_crate->vme_A32D32_write ( cardNum[0] + SIS3316_KEY_TRIGGER, 0x0);
                usleep(100);
                for(int ichan = 0; ichan<0; ichan++){
                    unsigned int prevBankEndingRegister = cardNum[0] + SIS3316_ADC_CH1_ACTUAL_SAMPLE_ADDRESS_REG + 0x1000*(ichan/4)+ (ichan%4)*0x4;
                    return_code = vme_crate->vme_A32D32_read (prevBankEndingRegister,&data);
                    printf("Trigger Count %d Address 0x%08x %d\n",triggerCount,data,ichan);
                }
                if(0){
                    unsigned int actualAddress = 0;
                    unsigned int ctlstatus = 0;
                    unsigned int irqstatus = 0;
                    unsigned int irqconfig = 0;
                    return_code = vme_crate->vme_A32D32_read(cardNum[0] + SIS3316_IRQ_CONFIG,&irqconfig);
                    return_code = vme_crate->vme_A32D32_read(cardNum[0] + SIS3316_ADC_CH1_ACTUAL_SAMPLE_ADDRESS_REG,&actualAddress);
                    return_code = vme_crate->vme_A32D32_read(cardNum[0] + SIS3316_ACQUISITION_CONTROL_STATUS,&ctlstatus);
                    return_code = vme_crate->vme_A32D32_read(cardNum[0] + SIS3316_IRQ_CONTROL,&irqstatus);
                    printf("Trigger Count %d  Sample(0x%08x) Ctl(0x%08x) IRQ(0x%08x 0x%08x) USB(0x%08x)\n",triggerCount,actualAddress,ctlstatus,irqstatus,irqconfig,data_rd);
                }
            }

        } while ((data_rd & 0xFE) == 0x00000 && !gl_stopReq && poll_counter<100);
        
        sis3316_adca[0]->DisarmAndArmBank();
        usleep(100);
        
        for (islot=0; islot<MAX_NOF_SIS3316_ADCS; islot++){
            sis3316card* vslot = sis3316_adca[islot];
            vslot->FetchAllData();
            cSISScope->cd();
            hWaveformAxis->Draw("AXIS");
            analyzeBlock(0,vslot,hFlood);
            cSISScope->Update();
            cFlood->Modified();
            cFlood->Update();
            cEnergy->cd();
            hEnergy->Draw();
            cEnergy->Update();
            cPSDEnergy->cd();
            hPSDEnergy->Draw("colz");
            cPSDEnergy->Update();
            gSystem->ProcessEvents();
        }
        
        triggerCount=0;
    }while (!gl_stopReq);
    
    program_stop_and_wait();
    fout->Write();
    fout->Close();
    return 0;
}


/***************************************************/


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

