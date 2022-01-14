"""

This script is for taking data with the 32-channel (2 x 16) DT Unit + 14-bit VME unit from ORNL
digitizers, with the 2008 version of the firmware.

For the DT unit with 2008 version of firmware, use the script takeData_16ch_DT.py. 

"""

# modified from Jason Newby, email 16 June 2016, subject:SIS3316


# from test.C
import time
import ROOT
ROOT.gROOT.SetBatch(True)

def takeData( doLoop=False, n_hours=10.0):

  # ---------------------------------------------------------------------------
  # options
  # ---------------------------------------------------------------------------
  n_cards = 1 #how many struck modules are being used

  runDuration = 50 # seconds
  #A 60s run is 720 MB with 4ms veto

  # settings
  threshold = 10 #1200
  spe_threshold = 1.
  gain = 1 # default = 1 1 = 2V; 0 = 5V, use 1 for LXe runs, 0 for testing warm
  termination = 1 # 1 = 50 ohm?
  nimtriginput = 0x10 # Bit0 Enable : Bit1 Invert , we use 0x10 (from struck root gui)
  trigconf = 0x8 # default = 0x5, we use 0x8 Bit0:Invert, Bit1:InternalBlockSum, Bit2:Internal, Bit3:External                       
  gaptime = 4 # delay
  risetime = 10 # peaking time
  firenable = 0
  min_coincident_channels = 3 #minimum number of coincident channels for an event to trigger "TO"
  coinc_enable = 0 #enable coincidence setting.


  #the channels in SiPM card that are
  #considered in the coincidence and trigger logic
  #(even if not coincidence, considered in the all-way OR)
  trig_chan_list = [3,4,5,6,8,9]
  spe_to_adc_values = [200,150,150,150,150,150]
  #unsigned int version of this list
  uint_trig_chan_list = 0x0
  for i in range(16):
    if(i in trig_chan_list):
      uint_trig_chan_list += (1 << i)

  file_suffix = "evantest"


  dacoffset = 40000 # default = 32768 

  # could have a few other clock freqs if we define them, look to struck root gui for info
  # clock_source_choice = [1,1] # 0: 250MHz, 1: 125MHz, 2=62.5MHz 3: 25 MHz (we use 3) 
  clock_source_choice = 2 # 0: 250MHz, 1: 125MHz, 2=62.5MHz 3: 25 MHz (we use 3) 
  gate_window_length = 1050 #800 (normal)
  pretriggerdelay = 275
  if clock_source_choice == 0: # preserve length of wfm in microseconds
      gate_window_length = 8000
      pretriggerdelay = 2000
  elif clock_source_choice == 1:
      #At 125MHz we have 5 times more samples in the same time window 
      #so extend the number of struck samples saved.
      gate_window_length = 13000
      pretriggerdelay = 1000
      #pretrigger=2000
      #gate_window_length = 20000
  elif clock_source_choice == 2:
      gate_window_length = 10000
      pretriggerdelay = 2000

  #gate_window_length = 300
  #pretriggerdelay    = 150

  # ---------------------------------------------------------------------------

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

  sis = ROOT.SIS3316SystemMT()
  #sis.setDebug() # NGMModuleBase/NGMModule::setDebug()
  sis.initModules() # NGMModuleBase/NGMModule::initModules()
  sis.SetNumberOfSlots(n_cards) # SIS3316SystemMT::SetNumberOfSlots()
  sis.CreateDefaultConfig("SIS3316") # SIS3316SystemMT _config = new NGMSystemConfigurationv1
  
  #sis.SetInterfaceType("sis3316_eth")
  sis.SetInterfaceType("sis3316_ethb") # SIS3316SystemMT testing this one since it has VME_FPGA_VERSION_IS_0008_OR_HIGHER

  # NGMSystem::GetConfiguration returns NGMSystemConfiguration, in NGMData/
  # NGMSystemConfiguration::GetSystemParameters() returns
  # NGMConfigurationTable, in NGMData/
  sis.GetConfiguration().GetSystemParameters().SetParameterD("MaxDuration",0,runDuration) #seconds
  sis.GetConfiguration().GetSystemParameters().SetParameterS("OutputFileSuffix",0,file_suffix) 
  sis.GetConfiguration().GetSlotParameters().AddParameterS("IPaddr")
  
  #The VME unit is the master so must go first
  sis.GetConfiguration().GetSlotParameters().SetParameterS("IPaddr",0,"192.168.4.40")
  if n_cards > 1:
    #Slave is currently the DT so goes second
    sis.GetConfiguration().GetSlotParameters().SetParameterS("IPaddr",1,"192.168.1.100")
  if n_cards > 2:
    sis.GetConfiguration().GetSlotParameters().SetParameterS("IPaddr",2,"192.168.1.102")

  print("\n----> calling InitializeSystem()")
  sis.InitializeSystem() # this also calls ConfigureSystem()
  print("----> done InitializeSystem()\n")

  # Adjust trigger thresholds etc. See sis3316card.{h,cc}
  for icard in xrange(n_cards): # loop over cards:
    sis0 = sis.GetConfiguration().GetSlotParameters().GetParValueO("card",icard)



    sis0.coincidenceEnable = coinc_enable
    sis0.nimtriginput = nimtriginput
    #Output settings that are common:
    #0x37D: or of [0,2,3,4,5,6,8,9]
    #0x1000000: stretched pulse from coincidence table 1 settings.
    if(coinc_enable):
    	sis0.nimtrigoutput = 0x1000000 
    	sis0.coincMask = uint_trig_chan_list
    	sis0.minimumCoincidentChannels = min_coincident_channels
    else:
    	#base it off of the trig_chan_list
    	sis0.nimtrigoutput = uint_trig_chan_list

    # need to use this method to set clock freq. We don't want to change the
    # master/slave sharingmode:
    sis0.SetClockChoice(clock_source_choice,sis0.sharingmode)

    print("\nSIS", icard, \
      "| nimtriginput:", sis0.nimtriginput, \
      "| clock_source_choice:", sis0.clock_source_choice, \
      "| sharingmode:", sis0.sharingmode, \
      "| nimtrigoutput:", sis0.nimtrigoutput)

    for j in xrange(4): # loop over adc groups

        sis0.gate_window_length_block[j] = gate_window_length
        sis0.sample_length_block[j] = gate_window_length # is this right?!
        sis0.pretriggerdelay_block[j] = pretriggerdelay
        sis0.dacoffset[j] = dacoffset

        print("\t ADC group",  j, \
            "| dacoffset:", sis0.dacoffset[j], \
            "| gate_window_length:", sis0.gate_window_length_block[j], \
            "| sample_length:", sis0.sample_length_block[j], \
            "| pretriggerdelay:", sis0.pretriggerdelay_block[j])

    # end loop over adc groups

    for i in xrange(16):  # loop over each channel
      #Set the gains for each channel.
      if True:
          sis0.gain[i] = gain
      else:
          sis0.gain[i] = gain

      sis0.termination[i] = termination # set termination
      sis0.trigconf[i] = trigconf # set trigger conf
      sis0.firenable[i] = firenable

      

      if icard == 0 and (i in trig_chan_list):
        trig_chan_idx = trig_chan_list.index(i)
        sis0.firthresh[i] = spe_threshold*spe_to_adc_values[trig_chan_idx] # set threshold
        sis0.firenable[i] = 1

      if icard == 0 and (i in [1,7,11,12,13,14,15]):
        sis0.trigconf[i] = 0x0

      if icard == 1 and i ==13:
        sis0.trigconf[i] = 0x0

      if icard == 2 and i == 15:
        sis0.trigconf[i] = 0x0


      print("\t SIS", icard, "ch%i" % i, \
          "| gain:", sis0.gain[i], \
          "| termination:", sis0.termination[i], \
          "| firthresh:", sis0.firthresh[i], \
          "| firenable:", sis0.firenable[i], \
          "| trigconf:", sis0.trigconf[i])

     # end loop over channels

  # end loop over cards
  print("\n-----> configure system")
  sis.ConfigureSystem()
  print("\n-----> start acquisition")
  if doLoop:
      print("===> starting %.1f-hour loop of %.1f-second runs.." % (n_hours, runDuration))
      n_loops = 0
      n_errors = 0
      start_time = time.time()
      last_time = start_time # last_time is time since start of last loop
      hours_elapsed = 0.0
      while hours_elapsed < n_hours:
          try:
              sis.StartAcquisition() 
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
      print "\n===> starting single run, %.1f seconds" % runDuration
      sis.StartAcquisition() 


if __name__ == "__main__":
   takeData()
