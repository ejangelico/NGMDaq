#ifndef __NGMSPY_H__
#define __NGMSPY_H__

#include "TH2.h"
#include "TCanvas.h"
#include "TSocket.h"
#include "TMessage.h"
#include "TObject.h"

class NGMSpy {
    
private:
  TSocket             *fSock;
  
public:
  NGMSpy();
  virtual ~NGMSpy();
  
  Bool_t Connect(const char* hostname = "localhost", int port = 9090);
  Bool_t Disconnect();
  TObject* RequestObject(const char* objName);
  TObject* RequestAll(const char* objName);
  TObject* cmd(const char* cmdName);

  ClassDef(NGMSpy,0)
};

#endif //__NGMSPY_H__