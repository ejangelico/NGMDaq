import ROOT
import sys
ROOT.gSystem.Load("libNGMDaqModule.so")

def addPSDToCal(calname,dataname):
  global infile
  global klist
  global tc
  klist = []
  infile = ROOT.TFile.Open(calname)
  rklist = infile.GetListOfKeys()
  for icut in range(0,rklist.GetSize()):
    klist.append(infile.Get(rklist.At(icut).GetName()))
  
  datain = ROOT.TFile.Open(dataname)
  conf = datain.FindObjectAny("NGMSystemConfiguration")
  detTable = conf.GetDetectorParameters()
  detTable.AddParameterO("BLOCK_PSD",0)
  for psdcalc in klist:
    detName = psdcalc.GetName().replace("_PSD","")
    irow = detTable.FindFirstRowMatching("DetectorName",detName)
    if irow > -1:
      detTable.SetParameterO("BLOCK_PSD",irow,psdcalc)
      print("Saving " + psdcalc.GetName())
  ofname = ROOT.gSystem.BaseName(dataname)
  ofname = ofname.replace(".root","-cal.root")
  ofile = ROOT.TFile.Open(ofname,"NEW")
  ofile.WriteTObject(conf,"NGMSystemConfiguration")
  ofile.Close()
  

if(len(sys.argv)==3):
  addPSDToCal(sys.argv[1],sys.argv[2])

