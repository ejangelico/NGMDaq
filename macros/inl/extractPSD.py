import ROOT
import sys
ROOT.gSystem.Load("libNGMDaqModule.so")

def extractPSD(fname,detectorName):
  global infile
  global psd
  infile = ROOT.TFile.Open(fname)
  mf = infile.FindObjectAny("MultiFormatReaderFolder")  
  psd = mf.FindObjectAny("PSDMaker")
  #  psd.fitNeutronGammaBand("LSBLOCKA_01")
  psd.fitNeutronGammaBand(detectorName)
  psdcalc = psd.GetParentFolder().FindObjectAny(detectorName+"_PSD")
  mfr = mf.FindObjectAny("MultiFormatReader")
  fname = "PSD_%lu.root" % (mfr.GetConfiguration().getRunNumber());
  ofile = ROOT.TFile(fname,"UPDATE")
  ofile.WriteTObject(psdcalc,psdcalc.GetName())
  ofile.Close()
  

if(len(sys.argv)==3):
  extractPSD(sys.argv[1],sys.argv[2])

