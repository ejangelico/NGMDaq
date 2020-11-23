#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include "caenHV6533card.h"
#include "vme_interface_class.h"

//using namespace std;

/**************************************************************************************/

ClassImp(caenHV6533card)

caenHV6533card::caenHV6533card()
{
    initcommon();
}
caenHV6533card::caenHV6533card(vme_interface_class *crate, unsigned int baseaddress)
{
    initcommon();
	if(crate){
		this->vmei = crate;
	}
	this->baseaddress = baseaddress;
}

void caenHV6533card::initcommon()
{
    this->vmei = 0;
    baseaddress = 0;
}

void caenHV6533card::initcard()
{
    unsigned int CAENV6533_board_vmax = 0x50;
    unsigned int CAENV6533_board_imax = 0x54;
    unsigned int CAENV6533_channel_rampup = 0xA4;
    unsigned int CAENV6533_channel_rampdown = 0xA0;
    unsigned int CAENV6533_channel_address_inc = 0x80;
    unsigned int vmax = 0;
    unsigned int imax = 0;
    int rc;
    unsigned int addr = 0;
    rc = this->vmei->vme_A32D16_read(baseaddress+CAENV6533_board_vmax,&vmax);
    if(rc<0){
        printf("ERROR Reading vmax at address 0x%x code %d\n",baseaddress+CAENV6533_board_vmax,rc);
    }
    rc = this->vmei->vme_A32D16_read(baseaddress+CAENV6533_board_imax,&imax);
    printf("VMAX %d V\n",vmax);
    printf("IMAX %d uA\n",imax);
    for(int ichan = 0; ichan < 6; ichan++)
    {
      addr = baseaddress+CAENV6533_channel_address_inc*ichan+CAENV6533_channel_rampup;
      rc = this->vmei->vme_A32D16_write(addr,200);
      addr = baseaddress+CAENV6533_channel_address_inc*ichan+CAENV6533_channel_rampdown;
      rc = this->vmei->vme_A32D16_write(addr,200);
    }

}

void caenHV6533card::SetVoltage(int channel, int Voltage)
{
    //unsigned int CAENV6533_channel_enable = 0x90;
    unsigned int CAENV6533_channel_vset = 0x80;
    unsigned int CAENV6533_channel_imaxset = 0x84;
    unsigned int CAENV6533_channel_address_inc = 0x80;

    double CAENV6533_channel_vset_res = 0.1;

    int rc = 0;
    unsigned int addr = 0;
    unsigned short newVoltage = Voltage/CAENV6533_channel_vset_res;
    addr = baseaddress+CAENV6533_channel_address_inc*channel+CAENV6533_channel_vset;
    printf("VMON(%d) addr(0x%08x) %d\n",channel,addr,newVoltage);
    rc = this->vmei->vme_A32D16_write(addr , newVoltage);
    
    unsigned short newimax = (3000.0/0.05); //3000 uA
    addr = baseaddress+CAENV6533_channel_address_inc*channel+CAENV6533_channel_imaxset;
    rc = this->vmei->vme_A32D16_write(addr , newimax);
}

void caenHV6533card::EnableChannel(int channel,bool enable)
{
    
    unsigned int CAENV6533_channel_enable = 0x90;
    unsigned int CAENV6533_channel_vset = 0x80;
    //unsigned int CAENV6533_channel_imaxset = 0x84;
    unsigned int CAENV6533_channel_vmon = 0x88;
    unsigned int CAENV6533_channel_imon = 0x8C;
    unsigned int CAENV6533_channel_address_inc = 0x80;
    double vmon_resolution = 0.1; //Volts
    double imon_resolution = 0.05; //uAmps
    uint16_t data = 0x0;
    
    if(enable)
        data = 0x1;

    uint32_t addr = baseaddress+CAENV6533_channel_address_inc*channel+CAENV6533_channel_enable;
    int rc = this->vmei->vme_A32D16_write(addr,data);
    uint32_t vmon = 0;
    uint32_t imon = 0;
    uint32_t imax = 0;
    uint32_t vset= 0;

    rc = this->vmei->vme_A32D16_read(baseaddress+CAENV6533_channel_address_inc*channel+CAENV6533_channel_vmon,&vmon);
    rc = this->vmei->vme_A32D16_read(baseaddress+CAENV6533_channel_address_inc*channel+CAENV6533_channel_vset,&vset);

    
    for(int ic = 0; ic<30; ic++)
    {
        rc = this->vmei->vme_A32D16_read(baseaddress+CAENV6533_channel_address_inc*channel+CAENV6533_channel_vmon,&vmon);
        rc = this->vmei->vme_A32D16_read(baseaddress+CAENV6533_channel_address_inc*channel+CAENV6533_channel_imon,&imon);
        printf("CHAN(%d) VMON(%f)[%f] IMON(%f) IMAX(%f)\n",
               channel,
               vmon*vmon_resolution,
               vset*vmon_resolution,
               imon*imon_resolution,
               imax*imon_resolution);
        if(abs(int(vmon)-int(vset))<10.0/vmon_resolution )break;
        usleep(1000000);
    }

}
