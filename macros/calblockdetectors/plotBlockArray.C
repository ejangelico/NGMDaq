void plotBlockArray()
{
  gStyle->SetPalette(1);
  NGMBlockArrayMonitorDisplay* fd = new NGMBlockArrayMonitorDisplay("BlockArray");
  TTimer* fdUpdate = new TTimer(fd,1000,true);
  fdUpdate->TurnOn();
  fdUpdate->Start(1000,true);
}
