
import glob
import pandas as pd
import numpy as np
import time
import os
import sys
import struct
import matplotlib.pyplot as plt 


import pylab
cm = pylab.get_cmap('gist_rainbow')
n = 16
clist = [cm(float(i)/n) for i in range(16)]

plt.style.use('~/evanstyle.mplstyle')

sampling_rate = 125 #MHz 
baseline_subtract = False


class NGMBinaryFile:

     ####################################################################
     def __init__( self, input_filename=None):
          print('NGMBinaryFile object constructed.')

          package_directory = os.path.dirname(os.path.abspath(__file__))
          
          if input_filename is not None:
               self.LoadBinaryFile( input_filename )
          

     ####################################################################
     def LoadBinaryFile( self, filename ):
          self.filename = filename
          print('Input file: {}'.format(self.filename))


          self.run_header, self.spills_list, self.words_read, self.spill_counter = \
                                  self.ReadFile( self.filename )
          #self.run_header = run_header
          #self
          if len(self.spills_list) == 0:
               print('Spills_list is empty... no spills found in file.')     

    ####################################################################
     def ReadFile( self, filename, max_num_of_spills=-1):
         start_time = time.time()
     
         total_words_read = 0
         spills_list = []
         run_header = []
         spill_counter = 0
         READ_WHOLE_FILE = True 

         if READ_WHOLE_FILE:


             # Read entire file as a numpy array of 32-bit integers
             with open(filename, 'rb') as infile:
                  file_content_array = np.fromfile( infile, dtype=np.uint32 )

             # 'fileidx' is the index of where we are in the file. It should
             # be set to the end of the last operation.
             fileidx = 0

             run_header = file_content_array[0:100]
             fileidx += 100
             print('fileidx after run_header = {}'.format(fileidx))

             while True:
                   spill_time = (time.time() - start_time)
                   print('Reading spill {} at {:4.4} sec'.format(spill_counter, spill_time), end="\r")

                   first_word_of_spillhdr = hex(file_content_array[fileidx])
                   if first_word_of_spillhdr == '0xabbaabba':
                      spill_dict, words_read, last_word_read = \
                            self.ReadSpillFast( file_content_array, fileidx )
                      fileidx += words_read
                   elif first_word_of_spillhdr == '0xe0f0e0f':
                      break
                   else: 
                      print('\n****** ERROR *******')
                      print('Unrecognizable first word of spillhdr: {}'.format(\
                                       first_word_of_spillhdr))

                   spills_list.append( spill_dict )
                   total_words_read += words_read
                   spill_counter += 1
                   

         else:
             # The basic unit in which data are stored in the NGM binary files is the
             # "spill", which consists of a full read of a memory bank on the Struck
             # board. The spill contains some header information and then all of the data stored
             # in the memory bank, sorted sequentially by card and then by channel within the card. 
             with open(filename, 'rb') as infile:
         
                 # Read in the runhdr (one per file)
                 for i in range(100):
                     run_header.append(hex(struct.unpack("<I", infile.read(4))[0]))
         
                 first_word_of_spillhdr = hex(struct.unpack("<I", infile.read(4))[0])
         
                 while True:
                     spill_time = (time.time() - start_time)  # / 60.
                     print('Reading spill {} at {:4.4} sec'.format(spill_counter, spill_time))
         
                     # this allows us to check if we're actually on a new spill or not
                     if first_word_of_spillhdr == '0xabbaabba':
                         spill_dict, words_read, last_word_read = \
                             self.ReadSpill(infile, first_word_of_spillhdr)
                     elif first_word_of_spillhdr == '0xe0f0e0f':
                         break
                     spills_list.append(spill_dict)
                     total_words_read += words_read
                     spill_counter += 1
                     first_word_of_spillhdr = last_word_read
         
                     if (spill_counter > max_num_of_spills) and \
                             (max_num_of_spills >= 0):
                         break
         
             end_time = time.time()
             print('\nTime elapsed: {:4.4} min'.format((end_time - start_time) / 60.))
     
         return run_header, spills_list, words_read, spill_counter

     ####################################################################
     def ReadSpillFast( self, file_content_array, fileidx ):

         debug = False
 
         spill_dict = {}
         spill_words_read = 0
         initial_fileidx = fileidx

         spill_header = file_content_array[ fileidx:fileidx+10 ]
         fileidx += 10
         if debug:
            print('Spill header: {}'.format(spill_header))
            print('fileidx: {}'.format(fileidx))         


         if spill_header[0] == '0xe0f0e0f':
            return spill_dict, 0, spill_header[-1]
         
         data_list = []
         previous_card_id = 9999999

         while True:
             data_dict = {}
             hdrid = 0

             hdrid_temp = file_content_array[ fileidx ]
          
             # Break the loop if we've reached the next spill
             if hex(hdrid_temp) == '0xabbaabba' or\
                   hex(hdrid_temp) == '0xe0f0e0f':
                last_word_read = hex(hdrid_temp)
                break             

             this_card_id = (0xff000000 & hdrid_temp) >> 24
             if debug:
                 print('hdrid_temp: {}'.format(hex(hdrid_temp)))
                 print('Card ID: {}'.format(this_card_id))
             data_dict['card'] = this_card_id

             if (this_card_id != previous_card_id) and (hdrid_temp & 0xff0000 == 0):
                 # If we've switched to a new card and are on channel 0, there should
                 # be a phdrid; two 32-bit words long.
     
                 # print('\nREADING NEXT CARD')
                 data_dict['phdrid'] = file_content_array[fileidx:fileidx+2]
                 fileidx += 2
                 if debug:
                     print('phdrid:')
                     for val in data_dict['phdrid']:
                         print('\t{}'.format(hex(val)))
     
                 hdrid = file_content_array[fileidx]
                 fileidx += 1

                 previous_card_id = this_card_id
             else:
                 # if not, then the hdrid_temp must be the hdrid for the channel the individual channel
                 hdrid = hdrid_temp
                 fileidx += 1

             data_dict['hdrid'] = hex(hdrid)
             if debug: print('hdrid: {}'.format(data_dict['hdrid']))
     
             channel_id = ((0xc00000 & hdrid) >> 22) * 4 + ((0x300000 & hdrid) >> 20)
             if debug: print('channelid: {}'.format(channel_id))
     
             data_dict['chan'] = channel_id
     
             channel_dict, words_read = self.ReadChannelFast( file_content_array, fileidx )
             data_dict['data'] = channel_dict
             fileidx += words_read    
 
             #spill_words_read += words_read
             data_list.append(data_dict)

         spill_words_read = fileidx - initial_fileidx    
 
         spill_dict['spill_data'] = data_list
     
         return spill_dict, spill_words_read, last_word_read

     ####################################################################
     def ReadChannel( self, infile):
          # Assumes we've already read in the hdrid
          trigger_stat_spill = []
      
          channel_dict = {}
      
          # Trigger stat. counters are defined in Chapter 4.9 of Struck manual
          # 0 - Internal trigger counter
          # 1 - Hit trigger counter
          # 2 - Dead time trigger counter
          # 3 - Pileup trigger counter
          # 4 - Veto trigger counter
          # 5 - High-Energy trigger counter
          for i in range(6):
              trigger_stat_spill.append(hex(struct.unpack("<I", infile.read(4))[0]))
          channel_dict['trigger_stat_spill'] = trigger_stat_spill
      
          # data_buffer_size stores the number of words needed to read all the
          # events for a channel in the current spill. Its size should be an integer
          # multiple of:
          # (# of header words, defined by format bits) + (# of samples/waveform)/2.
          data_buffer_size = struct.unpack("<I", infile.read(4))[0]
          channel_dict['data_buffer_size'] = data_buffer_size
          if data_buffer_size == 0:
              i = 0 
      
          total_words_read = 0 
          num_loops = 0 
          events = []
      
          while total_words_read < data_buffer_size:
              # if num_loops%10==0: print('On loop {}'.format(num_loops))
              words_read, event = self.ReadEvent(infile)
              total_words_read += words_read
              events.append(event)
              num_loops += 1
      
          channel_dict['events'] = events
      
          return channel_dict, total_words_read

     ####################################################################
     def ReadChannelFast( self, file_content_array, fileidx ):

          initial_fileidx = fileidx

          # Assumes we've already read in the hdrid
          channel_dict = {}
      
          # Trigger stat. counters are defined in Chapter 4.9 of Struck manual
          # 0 - Internal trigger counter
          # 1 - Hit trigger counter
          # 2 - Dead time trigger counter
          # 3 - Pileup trigger counter
          # 4 - Veto trigger counter
          # 5 - High-Energy trigger counter
          channel_dict['trigger_stat_spill'] = file_content_array[fileidx:fileidx+6]
          fileidx += 6
 
          # data_buffer_size stores the number of words needed to read all the
          # events for a channel in the current spill. Its size should be an integer
          # multiple of:
          # (# of header words, defined by format bits) + (# of samples/waveform)/2.
          channel_dict['data_buffer_size'] = file_content_array[fileidx]
          fileidx += 1
 
          total_words_read = 0 
          num_loops = 0 
          events = []
      
          while total_words_read < channel_dict['data_buffer_size']:
              # if num_loops%10==0: print('On loop {}'.format(num_loops))
              words_read, event = self.ReadEventFast( file_content_array, fileidx )
              total_words_read += words_read
              fileidx += words_read
              events.append(event)
              num_loops += 1
      
          channel_dict['events'] = events
          total_words_read = fileidx - initial_fileidx      

          return channel_dict, total_words_read
       
     ####################################################################
     def ReadEvent( self, infile):
         # The "event" structure is defined in Chapter 4.6 of the Struck manual.
         # This starts with the Timestamp and ends with ADC raw data (we do not use
         # the MAW test data at this time)
     
         event = {}
         bytes_read = 0
     
         word = struct.unpack("<I", infile.read(4))[0]
         bytes_read += 4
     
         event['format_bits'] = 0xf & word
         event['channel_id'] = 0xff0 & word
         event['timestamp_47_to_32'] = 0xffff0000 & word
     
         word = struct.unpack("<I", infile.read(4))[0]
         bytes_read += 4
         event['timestamp_full'] = word | (event['timestamp_47_to_32'] << 32)
       
         # Read the various event metadata, specificed by the format bits
         if bin(event['format_bits'])[-1] == '1':
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['peakhigh_val'] = 0x0000ffff & word
             event['index_peakhigh_val'] = 0xffff0000 & word
     
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['information'] = 0xff00000 & word
             event['acc_g1'] = 0x00ffffff & word
     
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['acc_g2'] = 0x00ffffff & word
     
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['acc_g3'] = 0x00ffffff & word
     
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['acc_g4'] = 0x00ffffff & word
     
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['acc_g5'] = 0x00ffffff & word
     
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['acc_g6'] = 0x00ffffff & word
     
         if bin(event['format_bits'] >> 1)[-1] == '1':
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['acc_g7'] = 0x00ffffff & word
     
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['acc_g8'] = 0x00ffffff & word
     
         if bin(event['format_bits'] >> 2)[-1] == '1':
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['maw_max_val'] = 0x00ffffff & word
     
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['maw_val_pre_trig'] = 0x00ffffff & word
     
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['maw_val_post_trig'] = 0x00ffffff & word
     
         if bin(event['format_bits'] >> 3)[-1] == '1':
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['start_energy_val'] = 0x00ffffff & word
     
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['max_energy_val'] = 0x00ffffff & word
     
         # Read the sampling information
         word = struct.unpack("<I", infile.read(4))[0]
         bytes_read += 4
         event['num_raw_samples'] = 0x03ffffff & word
         event['maw_test_flag'] = 0x08000000 & word
         event['status_flag'] = 0x04000000 & word
     
         # Read the actual ADC samples. Note that each 32bit
         # word contains two samples, so we need to split them.
         event['samples'] = []
         for i in range(event['num_raw_samples']):
             word = struct.unpack("<I", infile.read(4))[0]
             bytes_read += 4
             event['samples'].append(word & 0x0000ffff)
             event['samples'].append((word >> 16) & 0x0000ffff)
     
         # There is an option (never used in the Gratta group to my knowledge) to have
         # the digitizers perform on-board shaping with a moving-average window (MAW)
         # and then save the resulting waveform:
         if event['maw_test_flag'] == 1:
     
             for i in range(event['num_raw_samples']):
                 word = struct.unpack("<I", infile.read(4))[0]
                 bytes_read += 4
                 event['maw_test_data'].append(word & 0x0000ffff)
                 event['maw_test_data'].append((word >> 16) & 0x0000ffff)
     
         words_read = bytes_read / 4
         return words_read, event
    
 
     ####################################################################
     def ReadEventFast( self, file_content_array, fileidx ):
         # The "event" structure is defined in Chapter 4.6 of the Struck manual.
         # This starts with the Timestamp and ends with ADC raw data (we do not use
         # the MAW test data at this time)
     
         event = {}
         initial_fileidx = fileidx    
     
         # First word contains format bits, chan ID, and beginning of timestamp
         event['format_bits'] = 0xf & file_content_array[fileidx]
         event['channel_id'] = 0xff0 & file_content_array[fileidx]
         event['timestamp_47_to_32'] = 0xffff0000 & file_content_array[fileidx]
         fileidx += 1
 
         # Next word completes the timestamp
         event['timestamp_full'] = file_content_array[fileidx] | (event['timestamp_47_to_32'] << 32)
         fileidx += 1
       
         # Read the various event metadata, specificed by the format bits
         if bin(event['format_bits'])[-1] == '1':
             event['peakhigh_val'] = 0x0000ffff & file_content_array[fileidx]
             event['index_peakhigh_val'] = 0xffff0000 & file_content_array[fileidx]
             fileidx += 1
     
             event['information'] = 0xff00000 & file_content_array[fileidx]
             event['acc_g1'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1
     
             event['acc_g2'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1
     
             event['acc_g3'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
 
             event['acc_g4'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
     
             event['acc_g5'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
     
             event['acc_g6'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
     
         if bin(event['format_bits'] >> 1)[-1] == '1':
             event['acc_g7'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
     
             event['acc_g8'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
     
         if bin(event['format_bits'] >> 2)[-1] == '1':
             event['maw_max_val'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
     
             event['maw_val_pre_trig'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
     
             event['maw_val_post_trig'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
     
         if bin(event['format_bits'] >> 3)[-1] == '1':
             event['start_energy_val'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
     
             event['max_energy_val'] = 0x00ffffff & file_content_array[fileidx]
             fileidx += 1    
     
         # Read the sampling information
         event['num_raw_samples'] = 0x03ffffff & file_content_array[fileidx]
         event['maw_test_flag'] = 0x08000000 & file_content_array[fileidx]
         event['status_flag'] = 0x04000000 & file_content_array[fileidx]
         fileidx += 1
     
         # Read the actual ADC samples. Note that each 32bit word contains
         # two samples, so we need to split them and zip them together using ravel.
         temp_samples = file_content_array[ fileidx : fileidx + event['num_raw_samples'] ]
         first_samples = temp_samples & 0x0000ffff
         second_samples = (temp_samples >> 16) & 0x0000ffff
         event['samples'] = np.ravel([first_samples,second_samples],'F')
         fileidx += event['num_raw_samples']
 
         # There is an option (never used in the Gratta group to my knowledge) to have
         # the digitizers perform on-board shaping with a moving-average window (MAW)
         # and then save the resulting waveform:
         if event['maw_test_flag'] == 1:
             temp_maw_samples = file_content_array[ fileidx : fileidx + event['num_raw_samples'] ]
             first_maw_samples = temp_maw_samples & 0x0000ffff
             second_maw_samples = (temp_maw_samples >> 16) & 0x0000ffff
             event['maw_test_data'] = np.ravel([first_maw_samples,second_maw_samples],'F')
             fileidx += event['num_raw_samples']
     
         words_read = fileidx - initial_fileidx
         return words_read, event
 
def plot_all_channels(fn, nevents):
    print("processing file: " + fn)

    binfile = NGMBinaryFile(fn) #loads the binary file
    spills = binfile.spills_list 

    evtcount = 0
    entrycount = 0
    cards = [0,1,2]

    plotted_chs = {}
    for c in cards:
        plotted_chs[c] = []

    
    for spill_dict in spills:
        spill_data = spill_dict['spill_data']
        num_channels = len(spill_data)
        evts_in_spill = 0 
        for i in range(num_channels):
            if(len(spill_data[i]['data']['events']) > evts_in_spill):
                evts_in_spill = len(spill_data[i]['data']['events'])

        for i in range(evts_in_spill):
            fig, axs = plt.subplots(figsize=(18, 6), ncols=3)
            for ch, channel_data in enumerate(spill_data):
                if(i >= len(channel_data['data']['events'])):
                    continue #this happens if that channel is not recorded in that event number
                card_idx = channel_data['card']
                channel_wfm = np.array(channel_data['data']['events'][i]['samples'])
                channel = card_idx*16 + channel_data['chan']
                channel_16 = channel_data['chan']
                times = np.arange(0, len(channel_wfm)/float(sampling_rate), 1.0/(sampling_rate))
                if(baseline_subtract):
                    axs[card_idx].plot(times, channel_wfm - np.mean(channel_wfm[:200]), color=clist[channel_16%16], label="{:d}/{:d}".format(channel_16, channel))
                else:
                    axs[card_idx].plot(times, channel_wfm, color=clist[channel_16%16], label="{:d}/{:d}".format(channel_16, channel))

        

            for c in cards:
                axs[c].set_title(" slot " + str(card_idx))
                axs[c].set_xlabel("time (us) for " + str(sampling_rate) + "MHz sampling")
                axs[c].set_ylabel("ADC counts")
                axs[c].legend()

            fig.suptitle('Event {}'.format(evtcount))
            plt.show()
        
            evtcount += 1
            if(evtcount > nevents):
                return



if __name__ == "__main__":


    bin_files = glob.glob("*.bin")
    bin_files = sorted(bin_files)
    infile = bin_files[-1] #most recent. 

    nevents = 100
    plot_all_channels(infile, nevents)

    



