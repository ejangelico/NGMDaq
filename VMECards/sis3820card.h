#ifndef _SIS3820_CLASS_
#define _SIS3820_CLASS_

#include "TObject.h"

class vme_interface_class; //Forward Declaration

class sis3820card : public TObject
{
public:
    
	vme_interface_class *vmei; //!
	unsigned int  baseaddress;
    
private:

   public:
    
    unsigned int modid;
    unsigned int modfirmware;
    unsigned int clock_source_choice; // 0 : 100MHz, 1 : 80MHz, 2 : 50.0MHz, 3 : 40MHz, 4 : 25MHz
    unsigned int broadcastaddr;
    
    

	sis3820card(vme_interface_class *crate, unsigned int baseaddress);
	sis3820card();
    void initcard();
    void initcommon();
    virtual ~sis3820card();
    void SetClockChoice(int clck_choice); //Sharing(0:NoSharing, 1:SharingSlave 2:SharingMaster
    int ClearTimeStamp();
    float ReadTemp();

    ClassDef(sis3820card,1)
};




#endif
