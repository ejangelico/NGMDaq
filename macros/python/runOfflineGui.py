#!/usr/bin/python

import sys
from PyQt4 import QtCore, QtGui
from DAQGuiWidget import DAQGuiWidget
from ROOT import gSystem
gSystem.Load("libNGMDaq.so")
#from ROOT import NGMSystem
gSystem.Load("libSISDaq.so")
from ROOT import SISDaqSystem

#ngmsys = NGMSystem()
ngmsys = SISDaqSystem()
ngmsys.initModules()
ngmsys.readConfigFile("NGMConfig.root")
ngmsys.InitializeSystem()
ngmsys.configureSystem()

#options include "windows", "motif", "cde", "plastique" and "cleanlooks"
#QtGui.QApplication.setStyle(QtGui.QStyleFactory.create('plastique'))
#QtGui.QApplication.setPalette(QtGui.QApplication.style().standardPalette())

app = QtGui.QApplication(sys.argv)
window = QtGui.QMainWindow()
gui = DAQGuiWidget()

#could use gui directly, but we may loose some window buttons 
window.setCentralWidget(gui)
window.show()



if __name__ == '__main__':
    print("To keep the python prompt, use import to load file.")
    exit(app.exec_())
else:
    print("You can treat this file as an executable.")
