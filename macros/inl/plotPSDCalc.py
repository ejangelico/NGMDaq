import ROOT
import sys
ROOT.gSystem.Load("libNGMDaqModule.so")


def plotPSDCalc(fname):
  global infile
  global klist
  global tc
  klist = []
  infile = ROOT.TFile.Open(fname)
  rklist = infile.GetListOfKeys()
  for icut in range(0,rklist.GetSize()):
    klist.append(infile.Get(rklist.At(icut).GetName()))
  tc = ROOT.TCanvas("cPSD","cPSD")
  tc.Divide(2,2)
  tc.cd(1)
  hAxis = ROOT.TH2F("axis","axis",10,0,4000,10,0.8,1.2)
  hAxis.DrawCopy("AXIS")
  for cut in klist:
    cut._nMean.Draw("LP SAME")
  tc.cd(2)
  hAxis.DrawCopy("AXIS")
  for cut in klist:
    cut._gMean.Draw("LP SAME")
  
  hAxis.SetBins(10,0,4000,10,0.0,0.2)
  tc.cd(3)
  hAxis.DrawCopy("AXIS")
  for cut in klist:
    cut._nSig.Draw("LP SAME")
  tc.cd(4)
  hAxis.DrawCopy("AXIS")
  for cut in klist:
    cut._gSig.Draw("LP SAME")
  

  

if(len(sys.argv)==2):
  plotPSDCalc(sys.argv[1])

