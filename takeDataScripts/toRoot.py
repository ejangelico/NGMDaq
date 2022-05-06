# From Jason Newby, July 2016

"""
created from toROOT.py
  notes about inheritance and file paths:

  TTask
    NGMModuleBase/NGMModule
      NGMModuleBase/NGMSystem
        NGMModuleCore/NGMMultiFormatReader 
        NGMModuleCore/NGMMultiFormatReader _reader = NGMSIS3316RawReader (public NGMReaderBase)
  
  NGMModuleCore/NGMReaderBase
    NGMModuleCore/NGMSIS3316RawReader


  NGMModuleBase/NGMModule
    NGMModuleCore/NGMHitIO
      NGMModuleCore/NGMHitOutputFile (in NGMHitIO.{h, cc})

NGMModuleCore/NGMSIS3316RawReader.cc fills out the NGMHit object

"""

import os
import sys
import commands
import ROOT

def toRoot(fname = None):

    #If input filename is null assume we want to examine the most recent file
    if(fname == None):
        # example name: SIS3316Raw_20160712204526_1.bin
        output  = commands.getstatusoutput("ls -rt SIS3316Raw*_1.bin | tail -n1")
        print "using most recent file, ", output[1]
        fname = output[1]

    print "--> processing", fname

    basename = os.path.basename(fname)
    basename =  os.path.splitext(basename)[0]
    fin = ROOT.NGMMultiFormatReader()
    fin.SetPassName("LIVE")
    mHitOut = ROOT.NGMHitOutputFile("tier1_"+basename,"HitOut")
    mHitOut.setBasePath("./")
    mHitOut.setBasePathVariable("")
    fin.Add(mHitOut)
    fin.initModules()

    fin.OpenInputFile(fname)
    ts = ROOT.TStopwatch()
    ts.Start()
    fin.StartAcquisition(); 
    # calls NGMSIS3316RawReader::ReadAll(), where GetParent()->push() calls NGMNodule::process()
    ts.Print();

if __name__ == "__main__":
    
    # if no argument is provided, toRoot will find the most recent file.
    # otherwise, loop over arguments
    if len(sys.argv) < 2:
        toRoot()
    else:
        for filename in sys.argv[1:]:
            ext = os.path.splitext(filename)[1]
            print ext
            if ext == ".bin":
                toRoot(filename)
            else:
                print "--> skipping %s, please provide *.bin file(s) \n" % filename



