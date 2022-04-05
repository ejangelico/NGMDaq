

import os
import sys

import ROOT
import glob


import numpy as np
import matplotlib.pyplot as plt 
plt.style.use('~/evanstyle.mplstyle')

sampling_rate = 25 #MHz 
    

def process_file(fn, nevents, card, chs):


    print("processing file: " + fn)

    basename = os.path.basename(fn)
    basename = os.path.splitext(basename)[0]

    # open the root file and grab the tree
    root_file = ROOT.TFile(fn)
    tree = root_file.Get("HitTree")
    n_entries = tree.GetEntries()
    #n_entries = 100
    print("{} entries".format( n_entries))
    if(n_entries == 0):
        print("No events in file")
        sys.exit()

    evtcount = 0
    entrycount = 0
    plotted_chs = []
    while True:
        if(evtcount >= nevents):
            break
        fig, ax = plt.subplots(figsize=(10, 8))
        for en in range(entrycount, n_entries):
            tree.GetEntry(en)
              
            channel =  tree.HitTree.GetChannel()
            card_idx = tree.HitTree.GetSlot()

            if(card != card_idx):
                continue
            if(channel not in chs):
                continue

            print("entry {}, channel {}, card {}".format(en, channel,card_idx))

            wfm_length = tree.HitTree.GetNSamples()
            graph = tree.HitTree.GetGraph()
            channel_wfm = np.array([graph.GetY()[isamp] for isamp in range(graph.GetN())])
            times = np.arange(0, graph.GetN()/float(sampling_rate), 1.0/(sampling_rate))

            
            ax.plot(times, channel_wfm, label=str(channel))
            plotted_chs.append(channel)
            if(list(set(plotted_chs)) == chs):
                break

        ax.set_title("Event " + str(en) + " slot " + str(card_idx))
        ax.set_xlabel("time (us) for " + str(sampling_rate) + "MHz sampling")
        ax.set_ylabel("ADC counts")
        ax.legend()
        plt.show()
        
        evtcount += 1
        entrycount = en
        plotted_chs = []
 
def plot_all_channels(fn, nevents):
    print("processing file: " + fn)

    basename = os.path.basename(fn)
    basename = os.path.splitext(basename)[0]

    # open the root file and grab the tree
    root_file = ROOT.TFile(fn)
    tree = root_file.Get("HitTree")
    n_entries = tree.GetEntries()
    #n_entries = 100
    print("{} entries".format( n_entries))
    if(n_entries == 0):
        print("No events in file")
        sys.exit()

    evtcount = 0
    entrycount = 0
    chs = {0:range(16), 1:range(16), 2:range(16)}
    cards = [0,1,2]

    plotted_chs = {}
    for c in cards:
        plotted_chs[c] = []

    while True:
        if(evtcount >= nevents):
            break
        fig, axs = plt.subplots(figsize=(18, 6), ncols=3)


        print("\nstarting event {}".format(evtcount))

        for en in range(entrycount, n_entries):
            tree.GetEntry(en)
              
            channel =  tree.HitTree.GetChannel()
            card_idx = tree.HitTree.GetSlot()

            print("entry {}, channel {}, card {}".format(en, channel,card_idx))

            wfm_length = tree.HitTree.GetNSamples()
            graph = tree.HitTree.GetGraph()
            channel_wfm = np.array([graph.GetY()[isamp] for isamp in range(graph.GetN())])
            times = np.arange(0, graph.GetN()/float(sampling_rate), 1.0/(sampling_rate))

            if(channel in plotted_chs[card_idx]):
                break
            else:
                axs[card_idx].plot(times, channel_wfm, label=str(channel))
                plotted_chs[card_idx].append(channel)

            plotted_full_event = True

            for c in cards:
                if(list(set(plotted_chs[c])) != list(chs[c])):
                    plotted_full_event = False

            if(plotted_full_event):
                break

        for c in cards:
            axs[c].set_title(" slot " + str(c))
            axs[c].set_xlabel("time (us) for " + str(sampling_rate) + "MHz sampling")
            axs[c].set_ylabel("ADC counts")
            axs[c].legend()

        fig.suptitle('Event {}'.format(evtcount))
        plt.show()
        
        evtcount += 1
        entrycount = en
        plotted_chs = {}
        for c in cards:
            plotted_chs[c] = []



if __name__ == "__main__":

    root_files = glob.glob("tier1*.root")
    root_files = sorted(root_files)
    infile = root_files[-1] #most recent. 

    nevents = 100
    plot_all_channels(infile, nevents)

    



