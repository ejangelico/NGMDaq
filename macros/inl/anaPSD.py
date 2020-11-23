import ROOT
import sys

ROOT.gSystem.Load("libNGMDaqModule.so")

mfr = ROOT.NGMMultiFormatReader()
cal = ROOT.NGMBlockDetectorCalibrator("mCal","mCal")
mfr.Add(cal)
mSort = ROOT.NGMPacketMergeSort("mSort","mSort")
cal.Add(mSort)
mPSD = ROOT.NGMBlockPSDMaker("PSDMaker","PSDMaker");
mSort.Add(mPSD)
mPSD.SetPSDLimits(0.85,1.15)
hitOut = ROOT.NGMHitOutputFile("HitOut","HitOut")
mSort.Add(hitOut)

filename = sys.argv[1]

mfr.initModules()
mfr.OpenInputFile(filename)
mfr.StartAcquisition()

