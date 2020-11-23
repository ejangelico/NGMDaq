#include "NGMSpy.h"

ClassImp(NGMSpy)


NGMSpy::NGMSpy(){fSock=0;}

Bool_t NGMSpy::Connect(const char* hostname, int port)
{

  Disconnect();
    
  // Connect to SpyServ
  fSock = new TSocket(hostname, port);
    
    if(fSock) return true;
    else return false;
}
Bool_t NGMSpy::Disconnect()
{
    delete fSock;
    fSock = 0;
    return true;
}

NGMSpy::~NGMSpy()
{
  // Clean up
  
  delete fSock;
}

TObject* NGMSpy::cmd(const char* cmdName)
{
    if(!fSock) return 0;
    
    if(!fSock->IsValid())
    {
        return 0;
    }
    
    TString ts(cmdName);
    fSock->Send(ts.Data());
    
    TMessage *mess;
    if (fSock->Recv(mess) <= 0) {
        return 0;
    }
    TObject* fobj = mess->ReadObject(mess->GetClass());
    
    delete mess;
    
    return fobj;
}


TObject* NGMSpy::RequestObject(const char* objName)
{
    
  if(!fSock) return 0;

  if(!fSock->IsValid())
  {
    return 0;
  }
  
  TString ts("get ");
  ts+=objName;
  fSock->Send(ts.Data());
  
  TMessage *mess;
  if (fSock->Recv(mess) <= 0) {
    return 0;
  }
  TObject* fobj = mess->ReadObject(mess->GetClass());
  
  delete mess;
  
  return fobj;
}

TObject* NGMSpy::RequestAll(const char* objName)
{
    
    if(!fSock) return 0;
    
    if(!fSock->IsValid())
    {
        return 0;
    }
    
    TString ts("getAll ");
    //ts+=objName;
    fSock->Send(ts.Data());
    
    TMessage *mess;
    if (fSock->Recv(mess) <= 0) {
        return 0;
    }
    TObject* fobj = mess->ReadObject(mess->GetClass());
    
    delete mess;
    
    return fobj;
}