#include "NGMSpyServ.h"
#include "NGMLogger.h"
#include "TDirectory.h"
#include "TSystem.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TTimer.h"
#include "NGMLogger.h"
#include "NGMSystem.h"
#include "TFolder.h"

// Create the server process to fills a number of histograms.
// A spy process can connect to it and ask for the histograms.
// There is no apriory limit for the number of concurrent spy processes.

ClassImp(NGMSpyServ)

NGMSpyServ::NGMSpyServ(int port)
{
  // Open a server socket looking for connections on a named service or
  // on a specified port
  //TServerSocket *ss = new TServerSocket("spyserv", kTRUE);
  fServ = new TServerSocket(port, kTRUE);
  if (!fServ->IsValid())
  gSystem->Exit(1);

  // Add server socket to monitor so we are notified when a client needs to be
  // accepted
  fMon  = new TMonitor;
  fMon->Add(fServ);

  // Create a list to contain all client connections
  fSockets = new TList;
  fCheckingForRequests = false;
  fRequestDaqStart = false;
  fRequestDaqStop = false;
  fDaqRunTimer = new TTimer();
  fDaqRunTimer->Connect("Timeout()","NGMSpyServ",this,"CheckDaqStartRequests()");
  fDaqRunTimer->Start(100,false);
  fCheckForRequestsTimer = new TTimer();
  fCheckForRequestsTimer->SetObject(this);
  fCheckForRequestsTimer->Start(100,true);
}

NGMSpyServ::~NGMSpyServ()
{
  // Clean up
  
  fSockets->Delete();
  delete fSockets;
  delete fServ;
  delete fDaqRunTimer;
    delete fCheckForRequestsTimer;
}

Bool_t NGMSpyServ::HandleTimer(TTimer* timer)
{
  //LOG<<"NGMSpyServ::HandleTimer"<<ENDM_INFO;
  //std::cout<<"Begin NGMSpyServ::HandleTimer"<<std::endl;
  CheckForRequests();
  //std::cout<<"End NGMSpyServ::HandleTimer"<<std::endl;
//  timer->TurnOn();
//  timer->Start(1000,true);
  return true;
}

void NGMSpyServ::CheckForRequests()
{
  //protect against re-entrance
  if(fCheckingForRequests)
  {
      std::cout<<" Re-entrance prevented in NGMSpyServ::CheckForRequests()"<<std::endl;
      return;
  }
  fCheckingForRequests = true;
  // Check if there is a message waiting on one of the sockets.
  // Wait not longer than 20ms (returns -1 in case of time-out).
  TSocket *s;
  // Process at maximum 100 requests
  int requestCount = 0;
  while(requestCount < 10)
  {
      //std::cout<<"Begin Select"<<std::endl;
      if ((s = fMon->Select(10)) != (TSocket*)-1){
        //std::cout<<"End Select"<<std::endl;
          HandleSocket(s);
      }else{
          //std::cout<<"End Select"<<std::endl;
          break;
      }
    requestCount++;
  }
  fCheckingForRequests = false;
  fCheckForRequestsTimer->Start(100,true);
}

void NGMSpyServ::CheckDaqStartRequests()
{
    if(fRequestDaqStart)
    {
        fRequestDaqStart=false;
        if(NGMSystem::getSystem())
        {
            NGMSystem::getSystem()->StartAcquisition();
        }else{
            LOG<<"Dummy Start Begin"<<ENDM_INFO;
            DummyStartAcquisition();
            LOG<<"Dummy Start Finish"<<ENDM_INFO;
        }
    }
}

void NGMSpyServ::DummyStartAcquisition()
{
    TTimeStamp begin;
    TTimeStamp tnow;
    while((dummyruntime=(tnow.AsDouble()-begin.AsDouble()))<60.0)
    {
        gSystem->Sleep(100);
        gSystem->ProcessEvents();
        if(fRequestDaqStop)
        {
            fRequestDaqStop = false;
            std::cout<<"Exiting Dummy Acquisition"<<std::endl;
            break;
        }
        tnow.Set();
    }
}

void NGMSpyServ::GetStatus(TString &status)
{
    if(NGMSystem::getSystem())
    {
        NGMSystem::getSystem()->GetStatus(status);
    }else{
        status.Form("Dummy Status RunTime:%.1f\n",dummyruntime);
    }
}

void NGMSpyServ::HandleSocket(TSocket *s)
{
    std::cout<<"Begin HandleSocket"<<std::endl;
  char cbuf[1024];
  if (s->IsA() == TServerSocket::Class()) {
    // accept new connection from spy
    TSocket *sock = ((TServerSocket*)s)->Accept();
    fMon->Add(sock);
    fSockets->Add(sock);
    sprintf(cbuf,"accepted connection from %s\n", sock->GetInetAddress().GetHostName());
    std::cout<<cbuf<<std::endl;
  } else {
    // we only get string based requests from the spy
    char request[1024];
    if (s->Recv(request, sizeof(request)) <= 0) {
      fMon->Remove(s);
      fSockets->Remove(s);
      sprintf(cbuf,"closed connection from %s\n", s->GetInetAddress().GetHostName());
      std::cout<<cbuf<<std::endl;
      delete s;
      return;
    }
    
    // send requested object back
    TMessage answer(kMESS_OBJECT);
    TString sRequest(request);
    //std::cout<<"Received message: "<<request<<std::endl;
    TObjArray* tmp = sRequest.Tokenize(" ");
    int itok = 1;
    if(tmp->GetLast()!=-1
       && ((TObjString*)(tmp->At(0)))->String() == "get")
      while(itok<=tmp->GetLast())
      {
          TObject* tmpObj = NGMSystem::getSystem()->GetParentFolder()
            ->FindObjectAny(((TObjString*)(tmp->At(itok)))->String().Data());
        answer.Reset();
        if(tmpObj)
        {
          answer.WriteObject(tmpObj);
        }else{
          std::cout<<"Object "<<((TObjString*)(tmp->At(itok)))->String().Data()<<" not found."<<std::endl;
        }
        s->Send(answer);
        itok++;
      }
    else if(tmp->GetLast()!=-1
            && ((TObjString*)(tmp->At(0)))->String() == "getAll")
    {
        TObject* tmpObj = NGMSystem::getSystem()->GetParentFolder();
        answer.Reset();
        if(tmpObj)
        {
            answer.WriteObject(tmpObj);
            s->Send(answer);
        }
    }else if(tmp->GetLast()!=-1
             && ((TObjString*)(tmp->At(0)))->String() == "daqStop")
    {
        
        if(NGMSystem::getSystem())
        {
            NGMSystem::getSystem()->RequestAcquisitionStop();
        }else{
            LOG<<"Requesting stop dummy daq"<<ENDM_INFO;
            fRequestDaqStop = true;
        }

        answer.Reset();
        TObjString replyOK("OK");
        answer.WriteObject(&replyOK);
        s->Send(answer);
    }else if(tmp->GetLast()!=-1
             && ((TObjString*)(tmp->At(0)))->String() == "daqStart")
    {
        fRequestDaqStart = true;
        
        answer.Reset();
        TObjString replyOK("OK");
        answer.WriteObject(&replyOK);
        s->Send(answer);
    }else if(tmp->GetLast()!=-1
             && ((TObjString*)(tmp->At(0)))->String() == "daqStatus")
    {
        
        answer.Reset();
        TObjString reply;
        GetStatus(reply.String());
        answer.WriteObject(&reply);
        s->Send(answer);
    }else
    {
      Error("SpyServ::HandleSocket", "unexpected message");
      s->Send(answer);
    }
  }
  std::cout<<"End HandleSocket"<<std::endl;

}

