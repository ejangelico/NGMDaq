#include "vme_interface_class.h"
#include "TClass.h"
#include "TString.h"
//#include <stdio.h>
#include "TSystem.h"

ClassImp(vme_interface_class)

vme_interface_class* vme_interface_class::Factory(const char* InterfaceType)
{
    if(InterfaceType==TString("SISUSB")){
        gSystem->Load("libVMEInterfaces");
        TClass* vi = TClass::GetClass("sis3150");
        if(vi){
            return (vme_interface_class*)(vi->New());
        }else{
            printf("vme_interface_class::Factory : Class: sis3150 not found\n");
            return 0;
        }
    }else if(InterfaceType==TString("TSI148")){
        gSystem->Load("libVMEInterfaces");
        TClass* vi = TClass::GetClass("tsi148_vmeinterface");
        if(vi){
            return (vme_interface_class*)(vi->New());
        }else{
            printf("vme_interface_class::Factory : Class: tsi148_vmeinterface not found\n");
            return 0;
        }
    }
    else if( InterfaceType==TString("sis3316_eth") ) {
      // Firmware version 2004
      TClass* vi = TClass::GetClass("sis3316_eth");
      if(vi) {
        return (vme_interface_class*)(vi->New());
      }
      else {
        printf("ERROR: vme_interface_class::Factory : Class \'sis3316_eth\' not found\n");
        return 0;
      }
    }
    else if( InterfaceType==TString("sis3316_ethb") ) {
        // Firmware version 2008
        TClass* vi = TClass::GetClass("sis3316_ethb");
        if(vi) {
            return (vme_interface_class*)(vi->New());
        }
        else {
            printf("ERROR: vme_interface_class::Factory : Class \'sis3316_ethb\' not found\n");
            return 0;
        }
    }
    else{
        return new vme_dummy();
    }
}


ClassImp(vme_dummy)

int vme_dummy::vme_A32D32_write (uint32_t addr, uint32_t data)
{
    printf("vme_A32D32_write: 0x%08x 0x%08x\n",addr,data);
    return 0;
}

int vme_dummy::vme_A32D16_write (uint32_t addr, uint16_t data)
{
    printf("vme_A32D16_write: 0x%08x 0x%04x\n",addr,data);
    return 0;
}
