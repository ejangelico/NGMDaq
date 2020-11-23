import ROOT
import sys

ROOT.gSystem.Load("libNGMDaqModule.so")

mfr = ROOT.NGMMultiFormatReader()
cal = ROOT.NGMBlockDetectorCalibrator("mCal","mCal")
mfr.Add(cal)
mSort = ROOT.NGMPacketMergeSort("mSort","mSort")
cal.Add(mSort)
mSort.setMergeMode(2)
mFilt = ROOT.NGMHitFilter("hitFilt","hitFilt")
#mSort.Add(mFilt)
#mSort.setVerbosity(11);
hitOut = ROOT.NGMHitOutputFile("HitOut","HitOut")
mSort.Add(hitOut)

mFilt.SetAccept(0,1)
mFilt._baselinelow[0]=0.0
mFilt._baselinehigh[0]=1e9
mFilt._energylow[0] = 0
mFilt._energyhigh[0] = 4000
mFilt._neutroncutlow[0] = -1e9
mFilt._neutroncuthigh[0] = 1e9

filename = sys.argv[1]

mfr.initModules()
mfr.OpenInputFile(filename)
mfr.StartAcquisition()

