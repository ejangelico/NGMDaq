void goSimDaq(){

gSystem->Load("libNGMSimSystem.so");
NGMSimSystem* ss = new NGMSimSystem;
ss->CreateDefaultConfig("NGMSimDefault");
ss->OpenOutputFile("Test.root");
ss->InitializeSystem();
//ss->LaunchAcquisitionStartThread();
ss->StartAcquisition();
ss->CloseOutputFile();

}
