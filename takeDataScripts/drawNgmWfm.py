#!/usr/bin/env python

"""
This script draws a spectrum from an NGM root tree. 

arguments [NGM sis tier1 root files]
"""

import os
import sys

import ROOT
from ROOT import TF1
from ROOT import gROOT
#ROOT.gROOT.SetBatch(True)

import numpy as np
from scipy.fftpack import fft
from numpy import mean, sqrt, square, arange

     
ROOT.gROOT.SetStyle("Plain")     
ROOT.gStyle.SetOptStat(0)        
ROOT.gStyle.SetPalette(1)        
ROOT.gStyle.SetTitleStyle(0)     
ROOT.gStyle.SetTitleBorderSize(0)       
ROOT.gStyle.SetPadLeftMargin(0.13);
ROOT.gStyle.SetPadBottomMargin(0.1);
ROOT.gStyle.SetPadTopMargin(0.1);
ROOT.gStyle.SetPadRightMargin(0.13);
    

def process_file(filename):

    # options
    #channel = 0

    # maw options
    #gap_time = 30 # delay
    #peaking_time = 10 # length of moving average unit

    # gt 30, pt 10 seems ok for 250 MS/s
    # gt 30, pt 2 to 4 seems ok for 25 MS/s
    gap_time = 30 # delay
    peaking_time = 2 # length of moving average unit

    print "processing file: ", filename

    basename = os.path.basename(filename)
    basename = os.path.splitext(basename)[0]

    # open the root file and grab the tree
    root_file = ROOT.TFile(filename)
    tree = root_file.Get("HitTree")
    n_entries = tree.GetEntries()
    #n_entries = 100
    print "%i entries" % n_entries

    tree.SetLineWidth(2)
    tree.SetLineColor(ROOT.kBlue+1)
    tree.SetMarkerStyle(8)
    tree.SetMarkerSize(0.3)

    # set up a canvas
    c1 = ROOT.TCanvas("canvas1","canvas1",600,400)
    c1.SetGrid()

    c2 = ROOT.TCanvas("canvas2","canvas2",600,400)
    c2.SetGrid()
    #c2.SetLogy(1)

    c3 = ROOT.TCanvas("canvas3","canvas3",600,400)
    c3.SetGrid()

    i_entry = 0
    while i_entry < n_entries:
    #for i_entry in xrange(n_entries):

        tree.GetEntry(i_entry)

        #channel =  tree.HitTree.GetChannel()
        #gate_size = tree.HitTree.GetGateCount()

        #print "entry %i, channel %i" % (i_entry, channel)

        #legend = ROOT.TLegend(0.7, 0.92, 0.9, 0.99)
        #legend.AddEntry(tree, "channel %i" % channel, "pl")
        #selection = "Entry$==%i" % i_entry
        #n_drawn = tree.Draw("_waveform:Iteration$",selection, "lp")
        #n_drawn = tree.Draw("(_waveform-_waveform[0])*5000/pow(2,16):Iteration$",selection, "p")
        #print "waveform length", len(_waveform)
        #tree.Draw("(_waveform-_waveform[0])*5000/pow(2,16):Iteration$",selection, "pl")
        #print "waveform length", len(tree.HitTree._waveform)
        #n_drawn = tree.Draw("(_waveform-_waveform[0]):Iteration$",selection, "p")
        #n_drawn = tree.Draw("_waveform:Iteration$",selection, "p")
        
        n_channels=32+16
          
        for i_channel in xrange(n_channels):
            tree.GetEntry(i_entry)

            channel =  tree.HitTree.GetChannel()
            gate_size = tree.HitTree.GetGateCount()
            card_idx = tree.HitTree.GetSlot()

            print "entry %i, channel %i, card %i" % (i_entry, channel,card_idx)

            wfm_length = tree.HitTree.GetNSamples()
 
            print "sample length: ", wfm_length

            graph = tree.HitTree.GetGraph()

            graph.SetTitle("")
            graph.GetXaxis().SetTitle("Samples [8 ns]")
            graph.GetYaxis().SetTitle("ADC count")
            graph.GetXaxis().CenterTitle(1)
            graph.GetYaxis().CenterTitle(1)
            graph.GetYaxis().SetTitleOffset(1.4)

            legend = ROOT.TLegend(0.55, 0.85, 0.9, 0.99)
            legend.AddEntry(graph, "event %i, channel %i, card %i" % (i_entry/48+1,channel, card_idx), "pl")
            
            channel_wfm = np.array([graph.GetY()[isamp] for isamp in xrange(graph.GetN())])
 
            channel_fft = np.fft.rfft(channel_wfm)
             
            graph_fft=ROOT.TGraph() 
            #graph_fft=ROOT.TGraph(graph.GetN()/2+1,x_axis,channel_wfm)
            #print "fft length: ", len(channel_fft)
            #print channel_fft
            
            for isamp in xrange(len(channel_fft)):
                graph_fft.SetPoint(isamp,isamp,channel_fft[isamp])
          
            c1.Clear()
            c2.Clear()
            c3.Clear()

            c1.cd()
            graph.Draw()

            legend.Draw()

            c2.cd()
 
            graph_fft.GetYaxis().SetRangeUser(-2000000,3000000)
            graph_fft.Draw()

            c3.cd()

            #channel_fft[1]=0

            channel_fft[50:]=0

            channel_wfm_fft=np.fft.irfft(channel_fft)
          
            graph_fftcut=ROOT.TGraph()

            graph_fftcut.SetTitle("")
            graph_fftcut.GetXaxis().SetTitle("Samples [8 ns]")
            graph_fftcut.GetYaxis().SetTitle("ADC count")
            graph_fftcut.GetXaxis().CenterTitle(1)
            graph_fftcut.GetYaxis().CenterTitle(1)
            graph_fftcut.GetYaxis().SetTitleOffset(1.4)

            #graph_fftcut.GetYaxis().SetLimits(graph.GetHistogram().GetMinimum(),graph.GetHistogram().GetMaximum())
            #graph_fftcut.GetYaxis().SetRangeUser(graph.GetHistogram().GetMinimum(),graph.GetHistogram().GetMaximum())
 
            for isamp in xrange(graph.GetN()):
                x = graph.GetX()[isamp]
                graph_fftcut.SetPoint(isamp,x,channel_wfm_fft[isamp])

            #graph_fftcut.GetYaxis().SetLimits(graph.GetHistogram().GetMinimum(),graph.GetHistogram().GetMaximum())
            #graph_fftcut.GetYaxis().SetRangeUser(graph.GetHistogram().GetMinimum(),graph.GetHistogram().GetMaximum())

            graph_fftcut.SetTitle("")
            graph_fftcut.GetXaxis().SetTitle("Samples [8 ns]")
            graph_fftcut.GetYaxis().SetTitle("ADC count")
            graph_fftcut.GetXaxis().CenterTitle(1)
            graph_fftcut.GetYaxis().CenterTitle(1)
            graph_fftcut.GetYaxis().SetTitleOffset(1.4)

            graph_fftcut.Draw()

            rms_wfm = np.array([graph_fftcut.GetY()[isamp] for isamp in xrange(8500,9000,1)])
 
            rms_wfm=rms_wfm-np.mean(rms_wfm)
            
            rms_value=np.sqrt(rms_wfm.dot(rms_wfm)/rms_wfm.size)

            legend1=ROOT.TLegend(0.4, 0.85, 0.9, 0.99)
            legend1.SetTextSize(0.04)
            legend1.AddEntry(graph_fftcut, "RMS value (Ne-) is %f" % (rms_value*240), "pl")

            legend1.Draw()

            #graph_dummy=ROOT.TGraph()

            #for isamp in xrange(500,9500,1):
            #    graph_dummy.SetPoint(isamp,graph_fftcut.GetX()[isamp],graph_fftcut.GetY()[isamp])

            print "RMS value (Ne-) is: ", rms_value*240


            noise_wfm = np.array([graph_fftcut.GetY()[isamp] for isamp in xrange(700,800,1)])

            signal_wfm = np.array([graph_fftcut.GetY()[isamp] for isamp in xrange(1250,1350,1)])

            noise_baseline = np.mean(noise_wfm)
            noise_wfm_blsubt=noise_wfm-noise_baseline
            noise_rms=np.sqrt(noise_wfm_blsubt.dot(noise_wfm_blsubt)/noise_wfm_blsubt.size)
                   
            signal_amp = np.mean(signal_wfm)
            #signal_rms = np.rms(signal_wfm)

            print "noise baseline:%f noise rms:%f signal amplitude:%f" % (noise_baseline,noise_rms,signal_amp) 
            print "SNR: %f" % ((signal_amp-noise_baseline)/noise_rms)

            #if (signal_amp - noise_baseline)>2.5*noise_rms or (noise_baseline-signal_amp)>2*noise_rms: 
                #graph_fftcut.Fit(fitFunc,"SR")
                #fitFunc.Draw("SAME")
                 
            c1.Update()
            c2.Update()
            c3.Update()

            val = raw_input("--> entry %i | ch %i | enter to continue (q to quit, p to print, or entry number) " % (i_entry, channel))
            i_entry += 1

            if val == 'q': break
            if val == 'p':
                c1.Update()
                c1.Print("%s_entry_%i.png" % (basename, i_entry))
                c2.Update()
                c2.Print("%s_entry_%i.png" % (basename, i_entry))
            try:
                i_entry = int(val)
            except: 
                pass


        #if i_entry > 2: break # debugging

    #legend.Draw()
    #canvas.Update()
    #canvas.Print("%s_spectrum.png" % basename)


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print "arguments: [sis root files]"
        sys.exit(1)


    for filename in sys.argv[1:]:
        process_file(filename)



