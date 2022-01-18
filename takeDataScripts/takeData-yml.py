#Adapted by Evan Angelico, Sept 2021, for testing Struck modules for use with
#the CRYO ASIC software. 

import time
import ROOT
import yaml
import numpy as np 
import sys
ROOT.gROOT.SetBatch(True)

def takeData(config_path):


  #we have migrated to a yaml file input for configuring the strucks. 
  with open(config_path, 'r') as stream:
    try: 
      config = yaml.safe_load(stream)
    except yaml.YAMLError as exc:
      print("Had trouble opening yaml file:")
      print(exc)
      sys.exit()

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
  sis.SetNumberOfSlots(len(config['run']['card_numbers'])) # SIS3316SystemMT::SetNumberOfSlots()
  sis.CreateDefaultConfig("SIS3316") # SIS3316SystemMT _config = new NGMSystemConfigurationv1
  
  #sis.SetInterfaceType("sis3316_eth")
  sis.SetInterfaceType("sis3316_ethb") # SIS3316SystemMT testing this one since it has VME_FPGA_VERSION_IS_0008_OR_HIGHER

  # NGMSystem::GetConfiguration returns NGMSystemConfiguration, in NGMData/
  # NGMSystemConfiguration::GetSystemParameters() returns
  # NGMConfigurationTable, in NGMData/
  sis.GetConfiguration().GetSystemParameters().SetParameterD("MaxDuration",0,config["run"]["duration_per_file"]) #seconds
  sis.GetConfiguration().GetSystemParameters().SetParameterS("OutputFileSuffix",0,config["run"]["file_suffix"]) 
  sis.GetConfiguration().GetSlotParameters().AddParameterS("IPaddr")
  
  #IP addresses of cards, that are all confirmed connected by
  #StruckRootGUI
  for i, no in enumerate(config["run"]["card_numbers"]):
    #index of card number matches index of IP number 
    sis.GetConfiguration().GetSlotParameters().SetParameterS("IPaddr",no,config["run"]["card_ips"][i])

  print("\n----> calling InitializeSystem()")
  sis.InitializeSystem() 
  print("----> done InitializeSystem()\n")


  #set each card's configurations 
  for icard in config["run"]["card_numbers"]: # loop over cards:
    sis0 = sis.GetConfiguration().GetSlotParameters().GetParValueO("card",icard)
    sis0.SetClockChoice(config['digitization']['clock'],sis0.sharingmode) #share clock amongst all cards

    #-----trigger settings-----#
    trg = config["trigger"]
    strg = trg['self_trig_settings']

    #get active channels if self-trigger mode is relevant
    #if the card is not listed in the enabled channels zone, internal trig is disabled
    if(icard not in strg["enabled_channels"]):
      active_chs = []
    else:
      active_chs = strg["enabled_channels"][icard]
    #form an unsigned int, representing the active channels
    active_chs_uint = 0x0
    for i in range(16):
      if(i in active_chs):
        active_chs_uint += (1 << i) #used late for coinc settings

        trigconf = 0x0 #to start
        trigconf = trigconf | (1 << 2) #enables internal triggering 
        if(trg['level0_input'] == 'nim'):
          trigconf = trigconf | (1 << 3) #enables external trigger
        if(strg['pulse_polarity'] == 'n'):
          trigconf = trigconf | (1 << 0)
        sis0.trigconf[i] = trigconf # set trigger conf
        sis0.firenable[i] = 1

        #set thresholds
        if(icard not in strg['pe_to_adc']):
          print("You forgot to add thresholds, pe_to_adc, for card : " + str(icard))
          print("Exiting.")
          return
        adc_conversion = strg['pe_to_adc'][icard] #thresholds for each channel
        npe = strg['pe_for_trigger'] #same for all channels
        #these are lists that index-match the active channels list.
        #first, find which index this channel corresponds to. 
        chidx = active_chs.index(i) #index of this channel in the channel list

        sis0.firthresh[i] = int(npe*adc_conversion[chidx])

      else:
        trigconf = 0x0
        if(trg['level0_input'] == 'nim'):
          trigconf = trigconf | (1 << 3) #enables external trigger
        sis0.trigconf[i] = trigconf #allows only for external trigger if provided
        sis0.firenable[i] = 0x0

    #applies coincidence conditions, regardless of the level0_input
    sis0.coincidenceEnable = strg["coinc_enable"]
    if(strg["coinc_enable"]):
      sis0.coincMask = active_chs_uint 
      sis0.minimumCoincidentChannels = strg["min_coinc_channels"]

    #trigger input settings. 
    if(trg["level0_input"] == 'nim'):
      sis0.nimtriginput = 0x10 #set "TI as Trigger Enable", see page 114
    elif(trg["level0_input"] == "self"):
      sis0.nimtriginput = 0x0 
    else:
      pass #default settings

    #trigger output settings
    to = trg['to_settings'] #list of settings to enable
    tofull = 0x0
    for setting in to:
      if(setting == 'coinc'):
        tofull = tofull | (1 << 24)
      if(setting == 'sum'):
        tofull = tofull | (active_chs_uint)
      if(setting == 'ext'):
        tofull = tofull | (1 << 25)

    uo = trg['uo_settings'] #list of settings to enable
    uofull = 0x0
    for setting in uo:
      if(setting == 'busy'):
        uofull = uofull | (1 << 9)
      if(setting == 'ready'):
        uofull = uofull | (1 << 8)
      if(setting == 'coinc'):
        uofull = uofull | (1 << 24)





    #----digitization settings------#
    dig = config['digitization']

    for j in range(4): # loop over adc groups
        sis0.gate_window_length_block[j] = dig['gate_window_length'] 
        sis0.sample_length_block[j] = dig['raw_data_sample_length']
        sis0.pretriggerdelay_block[j] = dig['pretrig_delay']

        #sample start block, which determines the sample_start_index
        #shown on page 60, will adjust how much baseline is taken before
        #the time of trigger. This is set in the config as a "baseline_samples",
        #and here we calculate what the sample index should be based
        #on the pretrig delay. The baseline can at most be equal to
        #the pretrigger delay. 
        if(dig['baseline_samples'] > dig['pretrig_delay']):
          dig['baseline_samples'] = dig['pretrig_delay']
        sis0.sample_start_block[j] = dig['pretrig_delay'] - dig['baseline_samples']

        #dac_offset: see page 113: a calc is made in the run script based on the gain setting. 
        #for example, parametrized here: 
        #0 is centered in the full ADC range, -1V to +1V
        #-1 is fully shifted negative polar, -2V to 0V
        #1 is fully shifted positive, 0V to 2V. 
        #all floats in between are allowed. default will floor or ceiling if outside of -1 to 1
        doff = dig['dac_offset']
        if(doff < -1): doff = -1
        if(doff > 1): doff = 1 
        if(dig['gain'] == 1):
          #gain is for 2V range
          doff = ((52000 - 13000)/2.0)*doff + 32768
        elif(dig['gain'] == 0):
          doff = (65535/2.0)*doff + 32768

        sis0.dacoffset[j] = int(doff)



    for i in range(16):  # loop over each channel
      if(dig['gain'] != 0 and dig['gain'] != 1):
        print("Warning! Gain setting must be 0 or 1. Setting to 1")
        dig['gain'] = 1
      sis0.gain[i] = dig['gain']
      sis0.termination[i] = dig['termination'] # set termination
      

      

     # end loop over channels

  # end loop over cards

  print("\n-----> configure system") 
  sis.ConfigureSystem()
  
  if(config['run']['do_loop']):
      n_hours = config['run']['hours_per_iteration']
      print("===> starting %.1f-hour loop of %.1f-second runs.." % (n_hours, config['run']['duration_per_file']))
      n_loops = 0
      n_errors = 0
      start_time = time.time()
      last_time = start_time # last_time is time since start of last loop
      hours_elapsed = 0.0
      while hours_elapsed < n_hours:
          try:
              print("\n-----> start acquisition") 
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
      print("\n===> starting single run, %.1f seconds" % config['run']['duration_per_file'])
      print("\n-----> start acquisition") 
      sis.StartAcquisition() 


if __name__ == "__main__":
  if(len(sys.argv) != 2):
    print("Progam usage: python takeData.py <config file path>.yml")
    sys.exit()
  takeData(sys.argv[1])