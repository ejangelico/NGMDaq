#include<stdlib.h>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include "sis3820card.h"
#include "sis3820_clock.h"
#include "vme_interface_class.h"

//using namespace std;

/**************************************************************************************/

ClassImp(sis3820card)

sis3820card::sis3820card()
{
    initcommon();
}

sis3820card::sis3820card(vme_interface_class *crate, unsigned int baseaddress)
{
    initcommon();
	if(crate){
		this->vmei = crate;
	}
	this->baseaddress = baseaddress;

}

/****************************************************************************************************/

void sis3820card::initcard()
{
    int return_code = 0;
    return_code = vmei->vme_A32D32_write ( baseaddress + SIS3820CLOCK_KEY_RESET, 0);
    usleep(10);
}

/****************************************************************************************************/

sis3820card::~sis3820card()
{
}

/****************************************************************************************************/
void sis3820card::SetClockChoice(int clck_choice)
{
    unsigned int addr = 0;
    unsigned int data = 0;
    int return_code;
    
    
    switch (clck_choice)	{
        case  0: // 100 MHz
	    data = SIS3820CLOCK_CLOCK_SOURCE_100MHZ + SIS3820CLOCK_CLOCK_DIVIDE_1 ;	   // 
	    break;
        case  1: // 80 MHz
	    data = SIS3820CLOCK_CLOCK_SOURCE_80MHZ  + SIS3820CLOCK_CLOCK_DIVIDE_1;	   // 
	    break;
        case  2: // 50 MHz
	    data = SIS3820CLOCK_CLOCK_SOURCE_100MHZ + SIS3820CLOCK_CLOCK_DIVIDE_2;	   // 
	    break;
        case  3: // 40 MHz
	    data = SIS3820CLOCK_CLOCK_SOURCE_80MHZ + SIS3820CLOCK_CLOCK_DIVIDE_2 ;	   // 
	    break;
        case  4: // 25 MHz
	    data = SIS3820CLOCK_CLOCK_SOURCE_100MHZ + SIS3820CLOCK_CLOCK_DIVIDE_4 ;	   // 
	    break;
        case  5: // 20 MHz
	    data = SIS3820CLOCK_CLOCK_SOURCE_80MHZ + SIS3820CLOCK_CLOCK_DIVIDE_4;	   // 
	    break;
        case  6: // extern LEMO  CTRL 1
	    data = SIS3820CLOCK_CLOCK_SOURCE_EXT_CTRL;	   // 
	    break;
    } // end switch (uintClockMode)			

    data = data+0x5c0 ; 	   // Timestamp clear via Key 

    addr = baseaddress + SIS3820CLOCK_CLOCK_SOURCE   ;
    return_code = vmei->vme_A32D32_write ( addr, data);
    usleep(10);
    
    data = 127 ;
	addr = baseaddress + SIS3820CLOCK_TRIGGERMASK   ;
    return_code = vmei->vme_A32D32_write ( addr, data);
    usleep(10);

    data = SIS3820CLOCK_GENERAL_ENABLE ;
	addr = baseaddress + SIS3820CLOCK_CONTROL_STATUS   ;
    return_code = vmei->vme_A32D32_write ( addr, data);
    usleep(10);

    data = 0 ;
    data = data + SIS3820CLOCK_EXT_CLR_TIMESTAMP_ENABLE ;
    data = data + SIS3820CLOCK_EXT_VETO_IN_DISABLE ;
    data = data + SIS3820CLOCK_EXT_TRIGGER_IN_DISABLE ;
    data = data + 0x2 ; // 16 x Clock / 16 x Timestamp clear
    
	addr = baseaddress + SIS3820CLOCK_CONTROL_STATUS   ;
    return_code = vmei->vme_A32D32_write ( addr, data);
    usleep(10);
    
    
    // Enable  P2 and Frontpanel Outputs
	data = SIS3820CLOCK_FP_CLOCK_OUT_ENABLE + SIS3820CLOCK_P2_OUT_ENABLE ;
	addr = baseaddress + SIS3820CLOCK_CONTROL_STATUS   ;
    return_code = vmei->vme_A32D32_write ( addr, data);
    usleep(10);

    // Reset DLL
	data = 0 ;
	addr = baseaddress + SIS3820CLOCK_DLL_KEY_RESET   ;
    return_code = vmei->vme_A32D32_write ( addr, data);
    usleep(10);

    
}

/****************************************************************************************************/
void sis3820card::initcommon()
{   
    
    vmei = 0;
    modid = 0;
    clock_source_choice = 0;

}


/****************************************************************************************************/
int sis3820card::ClearTimeStamp(){
    
    unsigned int addr = 0;
    unsigned int data = 0;
    int return_code;
    addr = baseaddress + SIS3820CLOCK_KEY_CLR_TIMESTAMP;
    return_code = vmei->vme_A32D32_write ( addr , data);  //
    return 0;
}

float sis3820card::ReadTemp()
{
    return 0.0;
}
