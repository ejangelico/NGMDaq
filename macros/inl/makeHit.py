import ROOT
import sys

ROOT.gSystem.Load("libNGMDaqModule.so")

mfr = ROOT.NGMMultiFormatReader()
cal = ROOT.NGMBlockDetectorCalibrator("mCal","mCal")
mfr.Add(cal)
mSort = ROOT.NGMPacketMergeSort("mSort","mSort")
cal.Add(mSort)
mSort.setMergeMode(1)
mSort.SetMaxLiveTime(10);
mSort.setVerbosity(11);
#mSort.Add(mFilt)
mFilt = ROOT.NGMHitFilter("hitFilt","hitFilt")
#hitOut = ROOT.NGMHitOutputFile("LS01Out","LS01Out")
#mFilt.Add(hitOut)

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

