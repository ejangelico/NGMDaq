#ifndef _CAENHV6533_CLASS_
#define _CAENHV6533_CLASS_

#include "TObject.h"

class vme_interface_class; //Forward Declaration

class caenHV6533card
{
	vme_interface_class *vmei;//!
	unsigned int  baseaddress;
private:
    public:
	caenHV6533card();
	caenHV6533card(vme_interface_class *crate, unsigned int baseaddress);
    void initcard();
    void initcommon();
    virtual ~caenHV6533card(){}
    void SetVoltage(int channel, int Voltage);
    void EnableChannel(int channel,bool enable=true);

ClassDef(caenHV6533card,1)
};

#endif
