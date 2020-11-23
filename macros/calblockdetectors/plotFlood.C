

//class FloodDisplay : public TObject
//{
//public:
//  FloodDisplay()
//  {
//    cFloodDisplay = new TCanvas("cFloodDisplay","Flood");
//    spy.Connect();
//  }
//  TCanvas* cFloodDisplay;
//  NGMSpy spy;
//  
//  virtual Bool_t HandleTimer(TTimer* timer)
//  {
//    std::cout<<"Execute Handler"<<std::endl;
//    TH2* hHist = (TH2*)(spy.RequestObject("Flood_Flood_00"));
//    hHist->Draw("colz");
//    tc->Modified();
//    tc->Update();
//    return true;
//  }
//};


void plotFlood()
{
  gStyle->SetPalette(1);
  NGMBlockFloodDisplay* fd = new NGMBlockFloodDisplay();
  TTimer* fdUpdate = new TTimer(fd,1000,true);
  //TTimer* fdUpdate = new TTimer("printTest()",1000,true);
  fdUpdate->TurnOn();
  fdUpdate->Start(1000,true);
}


void printTest()
{
  std::cout<<"Test Print"<<std::endl;
}


void plotFlood2()
{
  gStyle->SetPalette(1);
  NGMSpy spy;
  spy.Connect();
  
  TCanvas* tc = new TCanvas("myFlood","myFlood");
  TTimeStamp timeOfLast(0,0);
  double updatePeriod = 1.0;
  TTimeStamp curTime;
  
  while(1)
  {
    gSystem->ProcessEvents();
    gSystem->Sleep(100);
    curTime.Set();
    if(curTime.AsDouble() - timeOfLast.AsDouble()>updatePeriod)
    {
      timeOfLast = curTime;
      TH2* hHist = (TH2*)(spy.RequestObject("Flood_Flood_00"));
      hHist->Draw("colz");
      tc->Modified();
      tc->Update();
    }
  }
}