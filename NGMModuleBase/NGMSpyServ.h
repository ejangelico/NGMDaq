#include "TSocket.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TRandom.h"
#include "TList.h"
#include "TString.h"

//Forward Declaration
class TTimer;

class NGMSpyServ : public TObject {
private:
  TServerSocket *fServ;      // server socket
  TMonitor      *fMon;       // socket monitor
  TList         *fSockets;   // list of open spy sockets
  TTimer* fDaqRunTimer;
  TTimer* fCheckForRequestsTimer;
  bool fCheckingForRequests;
  bool fRequestDaqStart;
  bool fRequestDaqStop;
  void DummyStartAcquisition();
  void GetStatus(TString &status);
  double dummyruntime;
public:
  NGMSpyServ(int port = 9090);
  ~NGMSpyServ();
  void CheckForRequests();
  void CheckDaqStartRequests();
  void HandleSocket(TSocket *s);
  Bool_t HandleTimer(TTimer* timer);
  ClassDef(NGMSpyServ,0)
};
