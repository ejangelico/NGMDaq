import sys
import ROOT
from easygui import msgbox

ROOT.gSystem.Load("libNGMDaqModule.so")
ROOT.gApplication.InitializeGraphics()
LUTList = []
c1=ROOT.TCanvas("LUT","LUT")
  
def extractLUT(fname):
  fin=ROOT.TFile.Open(fname)
  fout = ROOT.TFile("lut_08Aug2011.root","recreate")
  top=fin.Get("MultiFormatReaderFolder")
  rdr=top.FindObjectAny("MultiFormatReader")
  conf=rdr.GetConfiguration()
  floodFolder=top.FindObjectAny("FloodFolder")
  det = conf.GetDetectorParameters()
  sys = conf.GetSystemParameters()
  sys.PrintRow(0)
  nrows = sys.GetParValueI("BLOCKNROWS",0)
  ncols = sys.GetParValueI("BLOCKNCOLS",0)
  c1.Divide(ncols,nrows)
#Loop over entries in 
  for idet in range(det.GetEntries()):
    irow = det.GetParValueI("BLOCK_ROW",idet)
    icol = det.GetParValueI("BLOCK_COL",idet)
    c1.cd(icol+(nrows-irow-1)*ncols+1)
    detname = det.GetParValueS("DetectorName",idet)
    if not "LSBLOCKA" in detname:
      continue
    histname="Flood_Flood_"+detname
    flood=floodFolder.FindObjectAny(histname)
    if flood==None:
      continue
    print("Found " + histname)
    flood.SetDirectory(0)
    mLUT=ROOT.LUTmaker(flood,100)
    mLUT.PixelSearch()
    msgbox("Press Ok when pixels ok")
    mLUT.GatherSeedValues()
    ROOT.gSystem.ProcessEvents()
    mLUT.FitPixels()
    ROOT.gSystem.ProcessEvents()
    mLUT.CreateLUT()
    ROOT.gSystem.ProcessEvents()
    mLUT.CreateHitPattern()
    ROOT.gSystem.ProcessEvents()
    mLUT.DrawModifiedFlood(True,"col")
    fout.cd()
    mLUT.GetLUTHistogram().Write("LUT_SN{0}".format(detname[len(detname)-2:len(detname)]))
    mLUT.GetHitPattern().Write("hitPattern_SN{0}".format(detname[len(detname)-2:len(detname)]))
    global LUTList
    LUTList.append(mLUT)
  
  fout.cd()
  c1.Write("LUT_canvas");

filename = str(sys.argv[1])
extractLUT(filename)
