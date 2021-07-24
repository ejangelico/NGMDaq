import ROOT
import time

class SIS3316():
    def __init__(self):
        # ---------------------------------------------------------------------------
        # options
        # ---------------------------------------------------------------------------

        # file_suffix = "_test" # this gets appended to the file name
        self.file_suffix = "_PMT_in_dewar_30mvDT_coolingdown"  # this gets appended to the file name
        self.runDuration = 2 * 60  # seconds
        # runDuration = 10 # seconds -- debugging! FIXME
        # A 60s run is 720 MB with 4ms veto

        # settings
        threshold = 0
        self.gain = 0  # default = 1 1 = 2V; 0 = 5V, use 1 for LXe runs, 0 for testing warm
        # termination = 1 # 1 = 50 ohm?
        self.nimtriginput = 0x10  # Bit0 Enable : Bit1 Invert , we use 0x10 (from struck root gui)
        self.trigconf = 0x8  # default = 0x5, we use 0x8 Bit0:Invert, Bit1:InternalBlockSum, Bit2:Internal, Bit3:External
        self.dacoffset = 32768  # default = 32768

        # could have a few other clock freqs if we define them, look to struck root gui for info
        self.clock_source_choice = 3  # 0: 250MHz, 1: 125MHz, 2=62.5MHz 3: 25 MHz (we use 3)
        self.gate_window_length = 800
        self.pretriggerdelay = 200
        if self.clock_source_choice == 0:  # preserve length of wfm in microseconds
            self.gate_window_length = 8000
            self.pretriggerdelay = 2000

        # ---------------------------------------------------------------------------

    def configure(self):

      """
      NGM inheritance info (mashed up with file path info!):
    
      ROOT::TTask
        NGMModuleBase/NGMModule
          NGMModuleBase/NGMSystem
            Systems/SIS3316System/SIS3316SystemMT
    
      ROOT::TNamed
        NGMData/NGMSystemConfiguration
          NGMData/NGMSystemConfigurationv1
      """

      self.sis = ROOT.SIS3316SystemMT()
      self.sis.setDebug() # NGMModuleBase/NGMModule::setDebug()
      self.sis.initModules() # NGMModuleBase/NGMModule::initModules()
      self.sis.SetNumberOfSlots(2) # SIS3316SystemMT::SetNumberOfSlots()
      self.sis.CreateDefaultConfig("SIS3316") # SIS3316SystemMT _config = new NGMSystemConfigurationv1

      self.sis.SetInterfaceType("sis3316_eth")
      #sis.SetInterfaceType("sis3316_ethb") # SIS3316SystemMT testing this one since it has VME_FPGA_VERSION_IS_0008_OR_HIGHER

      # NGMSystem::GetConfiguration returns NGMSystemConfiguration, in NGMData/
      # NGMSystemConfiguration::GetSystemParameters() returns
      # NGMConfigurationTable, in NGMData/
      self.sis.GetConfiguration().GetSystemParameters().SetParameterD("MaxDuration",0,self.runDuration) #seconds
      self.sis.GetConfiguration().GetSystemParameters().SetParameterS("OutputFileSuffix",0,self.file_suffix)
      self.sis.GetConfiguration().GetSlotParameters().AddParameterS("IPaddr")
      self.sis.GetConfiguration().GetSlotParameters().SetParameterS("IPaddr",0,"192.168.1.100")
      self.sis.GetConfiguration().GetSlotParameters().SetParameterS("IPaddr",1,"192.168.2.100")

    def initialize(self):
        print("\n----> calling InitializeSystem()")
        self.sis.InitializeSystem()  # this also calls ConfigureSystem()
        print("----> done InitializeSystem()\n")

        # Adjust trigger thresholds etc. See sis3316card.{h,cc}
        for icard in range(2):  # loop over cards:
            sis0 = self.sis.GetConfiguration().GetSlotParameters().GetParValueO("card", icard)

            sis0.nimtriginput = self.nimtriginput

            # need to use this method to set clock freq. We don't want to change the
            # master/slave sharingmode:
            sis0.SetClockChoice(self.clock_source_choice, sis0.sharingmode)

            print("\nSIS", icard, \
                  "| nimtriginput:", sis0.nimtriginput, \
                  "| clock_source_choice:", sis0.clock_source_choice, \
                  "| sharingmode:", sis0.sharingmode, \
                  "| nimtrigoutput:", sis0.nimtrigoutput)

            for j in range(4):  # loop over adc groups

                sis0.gate_window_length_block[j] = self.gate_window_length
                sis0.sample_length_block[j] = self.gate_window_length  # is this right?!
                sis0.pretriggerdelay_block[j] = self.pretriggerdelay
                sis0.dacoffset[j] = self.dacoffset

                print("\t ADC group", j, \
                      "| dacoffset:", sis0.dacoffset[j], \
                      "| gate_window_length:", sis0.gate_window_length_block[j], \
                      "| sample_length:", sis0.sample_length_block[j], \
                      "| pretriggerdelay:", sis0.pretriggerdelay_block[j])

            # end loop over adc groups

            for i in range(16):  # loop over each channel

                # sis0.firthresh[i] = threshold # set threshold
                sis0.gain[i] = self.gain  # set gain
                # sis0.termination[i] = termination # set termination
                sis0.trigconf[i] = self.trigconf  # set trigger conf

                print("\t SIS", icard, "ch%i" % i, \
                      "| gain:", sis0.gain[i], \
                      "| termination:", sis0.termination[i], \
                      "| firthresh:", sis0.firthresh[i], \
                      "| trigconf:", sis0.trigconf[i])

            # end loop over channels

        # end loop over cards

        print("\n-----> configure system")
        self.sis.ConfigureSystem()

    def startAcquisition(self, doLoop=False, n_hours=10.0):
        print("\n-----> start acquisition" )
        if doLoop:
            print("===> starting %.1f-hour loop of %.1f-second runs.." % (n_hours, self.runDuration))
            n_loops = 0
            n_errors = 0
            start_time = time.time()
            last_time = start_time # last_time is time since start of last loop
            hours_elapsed = 0.0
            while hours_elapsed < n_hours:
                  try:
                      self.sis.StartAcquisition()
                  except:
                      print("error!")
                      n_errors += 1
                  n_loops += 1
                  now = time.time()
                  hours_elapsed = (now - start_time)/60.0/60.0
                  print("=====> %i loops | %i errors | %.1f seconds | %.1e seconds total | %.2f hours total \n" % (
                      n_loops,
                      n_errors,
                      now - last_time,
                      now - start_time,
                      hours_elapsed,
                  ))
                  last_time = now
        else:
          "\n===> starting single run, %.1f seconds" % self.runDuration
          self.sis.StartAcquisition()
