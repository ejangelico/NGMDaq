# %%

import numpy as np
from matplotlib import pyplot as plt
import pandas as pd
import argparse

plt.rcParams['figure.figsize'] = [10, 10]
plt.rcParams.update({'font.size': 18})
# plt.rcParams["font.weight"] = 'light'
# plt.rcParams["axes.labelweight"] = 'light'
# plt.rcParams["font.family"] = ['Times New Roman']
from scipy.ndimage import gaussian_filter

import cycler

#### import histlite as hl

import sys

sys.path.append('/g/g20/lenardo1/software/')
#### from TMSAnalysis.StruckAnalysisConfiguration import StruckAnalysisConfiguration

import os
from datetime import datetime
import time

color_cycle = plt.rcParams['axes.prop_cycle'].by_key()['color']

# %%

# path_to_binary = '/p/lustre1/lenardo1/stanford_teststand/RAW_DATA_TEST/'

# input_file = 'SIS3316Raw_20200916051219_SiPMs_longTPC_sbias32_scope_trig13_36mV_cath_6kV_1.bin'

# %%

if __name__ == '__main__':

    parser = argparse.ArgumentParser('Parse NGM Struck File')
    parser.add_argument('file', help='NGM Struck file')
    _args = parser.parse_args()

with open(_args.file, mode='rb') as infile:  # b is important -> binary


    # with open(path_to_binary + input_file, 'rb') as infile:
    header = []
    for i in range(200):
        header.append(infile.read(4))
        header.append(infile.read(4))

# %%

print(header[0].hex())

# %%

import struct

i = 0
for word in header:
    print('{}\t{}'.format(i, hex(struct.unpack("<I", word)[0])))
    i += 1

# %%

word = struct.unpack("<I", header[120])[0]  # int(hex(struct.unpack("<I",header[120])[0]),16)
print(word)
print(bin((word << 4) & 0xf0))
i = 0
for bit in bin(word):
    print('{}\t{}'.format(i, bit))
    i += 1


# print(len(word))

# %%


# %%


# %%


# %%

def read_file(filename, max_num_of_spills=1):
    start_time = time.time()

    total_words_read = 0
    spills_list = []
    run_header = []
    spill_counter = 0

    with open(filename, 'rb') as infile:

        # Read in the runhdr (one per file)
        for i in range(100):
            run_header.append(hex(struct.unpack("<I", infile.read(4))[0]))

        first_word_of_spillhdr = hex(struct.unpack("<I", infile.read(4))[0])

        while True:
            # print('\n')
            spill_time = (time.time() - start_time)  # / 60.
            print('Reading spill {} at {:4.4} sec'.format(spill_counter, spill_time))
            # print('First word of spillhdr: {}'.format(first_word_of_spillhdr))

            # this allows us to check if we're actually on a new spill or not
            if first_word_of_spillhdr == '0xabbaabba':
                spill_dict, words_read, last_word_read = \
                    read_spill(infile, first_word_of_spillhdr)
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


def read_spill(infile, first_entry_of_spillhdr):
    debug = False

    spill_dict = {}
    spill_words_read = 0

    spill_header = []
    spill_header.append(first_entry_of_spillhdr)
    for i in range(9):
        spill_header.append(hex(struct.unpack("<I", infile.read(4))[0]))
    spill_dict['spillhdr'] = spill_header

    if spill_header[0] == '0xe0f0e0f':
        return spill_dict, 0

    data_list = []
    previous_card_id = 9999999

    while True:
        data_dict = {}
        hdrid = 0

        # Grab the first word, which should be either a hdrid or a phdrid
        hdrid_temp = struct.unpack("<I", infile.read(4))[0]

        # Break the loop if we've reached the next spill
        if hex(hdrid_temp) == '0xabbaabba' or \
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
            phdrid = []
            phdrid.append(hdrid_temp)
            phdrid.append(struct.unpack("<I", infile.read(4))[0])
            data_dict['phdrid'] = phdrid
            if debug:
                print('phdrid:')
                for val in phdrid:
                    print('\t{}'.format(hex(val)))

            hdrid = struct.unpack("<I", infile.read(4))[0]
            previous_card_id = this_card_id
        else:
            # if not, then the hdrid_temp read in above must be the hdrid for
            # the individual channel
            hdrid = hdrid_temp

        data_dict['hdrid'] = hex(hdrid)
        if debug: print('hdrid: {}'.format(data_dict['hdrid']))

        channel_id = ((0xc00000 & hdrid) >> 22) * 4 + ((0x300000 & hdrid) >> 20)
        if debug: print('channelid: {}'.format(channel_id))

        data_dict['chan'] = channel_id

        channel_dict, words_read = read_channel(infile)
        data_dict['data'] = channel_dict

        spill_words_read += words_read
        data_list.append(data_dict)

    spill_dict['spill'] = data_list

    return spill_dict, spill_words_read, last_word_read


def read_channel(infile):
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

    # Data_buffer_size stores the number of words needed to read all the
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
        words_read, event = read_event(infile)
        total_words_read += words_read
        events.append(event)
        num_loops += 1

    channel_dict['events'] = events

    return channel_dict, total_words_read


def read_event(infile):
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

    word = struct.unpack("<I", infile.read(4))[0]

    bytes_read += 4
    event['num_raw_samples'] = 0x03ffffff & word
    event['maw_test_flag'] = 0x08000000 & word
    event['status_flag'] = 0x04000000 & word

    event['samples'] = []
    for i in range(event['num_raw_samples']):
        word = struct.unpack("<I", infile.read(4))[0]
        bytes_read += 4
        event['samples'].append(word & 0x0000ffff)
        event['samples'].append((word >> 16) & 0x0000ffff)

    if event['maw_test_flag'] == 1:

        for i in range(event['num_raw_samples']):
            word = struct.unpack("<I", infile.read(4))[0]
            bytes_read += 4
            event['maw_test_data'].append(word & 0x0000ffff)
            event['maw_test_data'].append((word >> 16) & 0x0000ffff)

    words_read = bytes_read / 4
    return words_read, event


# %%

# run_header, spills_list, words_read, spill_counter = read_file(path_to_binary + input_file, 3)
run_header, spills_list, words_read, spill_counter = read_file(_args.file, 100)       # Was 3

# %%

print(run_header)

# %%

first_spill = spills_list[0]
print(first_spill['spillhdr'])

channel_data = first_spill['spill'][20]
print('Card: {}'.format(channel_data['card']))
print('Channel: {}'.format(channel_data['chan']))
print('Channel: {}'.format(channel_data['data']['trigger_stat_spill']))

# %%

print(channel_data['data']['events'][0])

# %%

evt_idx = 3

print(channel_data['data']['events'][evt_idx]['timestamp_full'])
plt.plot(channel_data['data']['events'][evt_idx]['samples'])

# %%


