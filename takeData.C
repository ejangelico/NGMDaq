// modified from Jason Newby, email 16 June 2016, subject:SIS3316


// BEGIN takeData.C


void takeData(){

  // ---------------------------------------------------------------------------
  // options
  // ---------------------------------------------------------------------------

  int threshold = 0;
  unsigned short gain = 1; // default = 1; 1 = 2V; 0 = 5V, use 1 for LXe runs, 0 for testing warm
  //unsigned short termination = 1; // 1 = 50 ohm?
  unsigned int nimtriginput = 0x10; // Bit0 Enable : Bit1 Invert , we use 0x10 (from struck root gui)
  unsigned short trigconf = 0x8; // default = 0x5, we use 0x8; Bit0:Invert, Bit1:InternalBlockSum, Bit2:Internal, Bit3:External                       
  unsigned int dacoffset = 32768; // default = 32768 
  //double runDuration = 1*60; // seconds
  double runDuration = 10; // seconds

  // could have a few other clock freqs if we define them, look to struck root gui for info
  unsigned int clock_source_choice = 3; // 0: 250MHz, 1: 125MHz, 2=62.5MHz 3: 25 MHz 
  unsigned int gate_window_length = 800;
  unsigned int pretriggerdelay = 200; 

  // ---------------------------------------------------------------------------

  /* 
  NGM inheritance info (mashed up with file path info!):

  ROOT::TTask
    NGMModuleBase/NGMModule
      NGMModuleBase/NGMSystem
        Systems/SIS3316System/SIS3316SystemMT

  ROOT::TNamed
    NGMData/NGMSystemConfiguration
      NGMData/NGMSystemConfigurationv1

  */

  SIS3316SystemMT* sis = new SIS3316SystemMT();
  sis->setDebug(); // NGMModuleBase/NGMModule::setDebug()
  sis->initModules(); // NGMModuleBase/NGMModule::initModules()
  sis->SetNumberOfSlots(2); // SIS3316SystemMT::SetNumberOfSlots()
  sis->CreateDefaultConfig("SIS3316"); // SIS3316SystemMT; _config = new NGMSystemConfigurationv1

  sis->SetInterfaceType("sis3316_eth");
  //sis->SetInterfaceType("sis3316_ethb"); // SIS3316SystemMT; testing this one since it has VME_FPGA_VERSION_IS_0008_OR_HIGHER

  // NGMSystem::GetConfiguration returns NGMSystemConfiguration, in NGMData/
  // NGMSystemConfiguration::GetSystemParameters() returns
  // NGMConfigurationTable, in NGMData/
  sis->GetConfiguration()->GetSystemParameters()->SetParameterD("MaxDuration",0,runDuration); //seconds?
  sis->GetConfiguration()->GetSlotParameters()->AddParameterS("IPaddr");
  sis->GetConfiguration()->GetSlotParameters()->SetParameterS("IPaddr",0,"192.168.1.100");
  sis->GetConfiguration()->GetSlotParameters()->SetParameterS("IPaddr",1,"192.168.2.100");

  cout << "calling InitializeSystem()" << endl;
  sis->InitializeSystem(); // this also calls ConfigureSystem()
  cout << "done InitializeSystem()" << endl;

  // Adjust trigger thresholds etc. See sis3316card.{h,cc}
  for (size_t icard = 0; icard < 2; icard++) { // loop over cards:
    sis3316card* sis0 = (sis3316card*) sis->GetConfiguration()->GetSlotParameters()->GetParValueO("card",icard);

    sis0->nimtriginput = nimtriginput; 

    // need to use this method to set clock freq. We don't want to change the
    // master/slave sharingmode:
    sis0->SetClockChoice(clock_source_choice,sis0->sharingmode);

    cout << "SIS " << icard
      << " | nimtriginput: " << sis0->nimtriginput
      << " | clock_source_choice: " << sis0->clock_source_choice
      << " | sharingmode: " << sis0->sharingmode // // 0 single card, 1 shared slave, 2 shared master
      << " | nimtrigoutput: " << sis0->nimtrigoutput
      << endl;

    for (size_t j = 0; j < 4; j++ ) { // loop over adc groups

        sis0->gate_window_length_block[j] = gate_window_length;
        sis0->sample_length_block[j] = gate_window_length; // is this right?!
        sis0->pretriggerdelay_block[j] = pretriggerdelay;
        sis0->dacoffset[j] = dacoffset;

        cout << "\t ADC group " << j 
          << " | dacoffset: " << sis0->dacoffset[j]
          << " | gate_window_length: " << sis0->gate_window_length_block[j]
          << " | sample_length: " << sis0->sample_length_block[j]
          << " | pretriggerdelay: " << sis0->pretriggerdelay_block[j]
          << endl; 
    } // end loop over adc groups

    for (size_t i = 0; i< 16; i++) { // loop over each channel

      //sis0->firthresh[i] = threshold; // set threshold
      sis0->gain[i] = gain; // set gain
      //sis0->termination[i] = termination; // set termination
      sis0->trigconf[i] = trigconf; // set trigger conf

      cout << "\t SIS " << icard << " ch" << i 
        << " | gain: " << sis0->gain[i]
        << " | termination: " << sis0->termination[i]
        << " | firthresh: " << sis0->firthresh[i]
        << " | trigconf: " << sis0->trigconf[i]
        << endl;
    } // end loop over channels

  } // end loop over cards

  cout << "-----> configure system" << endl;
  sis->ConfigureSystem();
  cout << "-----> start acquisition" << endl;
  sis->StartAcquisition(); 
}
// END test.C
