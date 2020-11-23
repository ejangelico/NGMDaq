"""
Draw waveforms & print info from Jason N.'s HitTree. 
Use ROOT 6. 
13 July 2016 AGS
"""


import os
import sys
import ROOT
ROOT.gROOT.SetBatch(True)

if len(sys.argv) < 2: 
    print "argument [root file Hit*.root]"
    sys.exit()

print "processing", sys.argv[1]  

tfile = ROOT.TFile(sys.argv[1])
tree = tfile.Get("HitTree")
print "entries", tree.GetEntries()

canvas = ROOT.TCanvas("canvas","")
canvas.SetGrid(1,1)

prev_time = 0
event_time = None
n_channels_in_event = 0
n_events = 0
n_bad_events = 0

for i_entry in xrange(tree.GetEntries()):
    #if i_entry >= 32*2: break # debugging

    tree.GetEntry(i_entry)
    hit = tree.HitTree
    tsl = hit.GetRawClock() - prev_time
    if tsl < 0:
        print "WARNING! TSL < 0, TSL=", tsl
    #print "tsl",tsl
    #print "event_time", event_time

    #if True:
    #if tsl > 0 and tsl <= 3:
    if False:
        print "--> event %i | entry %i | slot %i | ch %i | raw clock %i | tsl %i" % (
            n_events,
            i_entry, 
            hit.GetSlot(),
            hit.GetChannel(),
            hit.GetRawClock(),
            hit.GetRawClock() - prev_time,
        )

    prev_time = hit.GetRawClock()
    if event_time is None: # this is the first event 
        event_time = hit.GetRawClock()
    elif tsl > 3: # this is a new event!
        if n_channels_in_event != 32:
            n_bad_events += 1
            print "--> event %i | entry %i | event time %i | n_channels_in_event %i" % (
                n_events,
                i_entry,
                event_time,
                n_channels_in_event,
            )
        event_time = hit.GetRawClock()
        n_channels_in_event = 0
        n_events += 1
    n_channels_in_event += 1
        
    #print "entry", i_entry
    #print "\t slot:", hit.GetSlot()
    #print "\t channel:", hit.GetChannel()
    #print "\t n samples:",hit.GetNSamples()
    #print "\t time stamp:",hit.GetTimeStamp()
    #print "\t raw clock:",hit.GetRawClock()
    #print "\t graph points:", hit.GetGraph().GetN()

    # seg violation?
    #print "rawclock:", tree._rawclock
    #print "slot:", tree._slot

    if hit.GetSlot() is not 0 or hit.GetChannel() is not 0: continue
    if not ROOT.gROOT.IsBatch():
        #hit.DisplayWaveform(2,1)
        hit.GetGraph().Draw("alp")
        canvas.Update()
        canvas.Print("entry%i_ch%i_slot%i_clock%i.pdf" % (
            i_entry, 
            hit.GetSlot(), 
            hit.GetChannel(),
            hit.GetRawClock(),
        ))
        # pause:
        val = raw_input("\t ---> enter to continue, q to quit ")
        if val == 'q':
            sys.exit()
    # end of loop over tree entries

n_events += 1
run_duration_seconds = hit.GetRawClock()/(25e6)

print "%i bad events" % n_bad_events

print "%i events in %.2f s (%.1f min) (%.2f Hz = %.2f ms period), %.2f ch per event" % (
    n_events,
    run_duration_seconds,
    run_duration_seconds/60.0,
    n_events/run_duration_seconds,
    run_duration_seconds/n_events*1e3,
    tree.GetEntries()*1.0/n_events,
)


