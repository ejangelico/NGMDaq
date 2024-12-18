from PyQt4 import QtGui, QtCore, uic
#import PyQt4.Qwt5 as Qwt
from ROOT import TCanvas, gSystem
gSystem.Load("libNGMDaq.so")
from ROOT import NGMSystemConfiguration, NGMSystem

#load the interface created
(form, formbase) = uic.loadUiType("ngmGUI.ui")

class DAQGuiWidget(QtGui.QDialog, form) : 
    def __init__(self) :
        QtGui.QDialog.__init__(self)
        #finish loading from ui file
        self.setupUi(self)
        #connect the GUI elements
        self.connect(self.startButton,QtCore.SIGNAL("clicked()"),self.doStart)
        self.connect(self.endButton,QtCore.SIGNAL("clicked()"),self.doStop)
        self.connect(self.refreshButton,QtCore.SIGNAL("clicked()"),self.refreshGui)
        self.connect(self.configButton,QtCore.SIGNAL("clicked()"),self.doConfigure)
        self.connect(self.updateCellsButton,QtCore.SIGNAL("clicked()"),self.updateCells)
        self.connect(self.hvoffButton,QtCore.SIGNAL("clicked()"),self.doHVOFF)
        self.connect(self.glHVButton,QtCore.SIGNAL("clicked()"),self.doGLHV)
        self.connect(self.gsHVButton,QtCore.SIGNAL("clicked()"),self.doGSHV)
        self.connect(self.muHVButton,QtCore.SIGNAL("clicked()"),self.doMUHV)
        self.connect(self.heHVButton,QtCore.SIGNAL("clicked()"),self.doHEHV)
        self.connect(self.lsHVButton,QtCore.SIGNAL("clicked()"),self.doLSHV)
        self.connect(self.systemTableWidget,QtCore.SIGNAL("cellChanged(int,int)"),self.syncCell)
        self.connect(self.slotTableWidget,QtCore.SIGNAL("cellChanged(int,int)"),self.syncCell)
        self.connect(self.channelTableWidget,QtCore.SIGNAL("cellChanged(int,int)"),self.syncCell)
        self.connect(self.hvTableWidget,QtCore.SIGNAL("cellChanged(int,int)"),self.syncCell)
        self.connect(self.detectorTableWidget,QtCore.SIGNAL("cellChanged(int,int)"),self.syncCell)
        self.connect(self.goURLButton,QtCore.SIGNAL("clicked()"),self.doURLChange)
        self.connect(self.le_HostName,QtCore.SIGNAL("editingFinished()"),self.setHostName)
        self.connect(self.te_maxseconds,QtCore.SIGNAL("timeChanged( const QTime)"),self.setMaxSeconds)
#        self.connect(self.pt_Comment,QtCore.SIGNAL("textChanged()"),self.setComment)
        self.connect(self.le_configuration,QtCore.SIGNAL("editingFinished()"),self.setConfiguration)
        self.connect(self.le_experimentName,QtCore.SIGNAL("editingFinished()"),self.setExperimentName)
        self.connect(self.le_facility,QtCore.SIGNAL("editingFinished()"),self.setFacility)
        self.connect(self.le_daqOperator,QtCore.SIGNAL("editingFinished()"),self.setDaqOperator)
        self.connect(self.te_maxseconds,QtCore.SIGNAL("timeChanged(const QTime &time )"),self.setMaxSeconds)
        self.connect(self.launchRawPathDialog,QtCore.SIGNAL("clicked()"),self.getNewRawPath)
        self.connect(self.saveConfigurationButton,QtCore.SIGNAL("clicked()"),self.saveConfigFile)
        self.connect(self.loadConfigurationButton,QtCore.SIGNAL("clicked()"),self.loadConfigFile)
        self.connect(self.updateDataBase,QtCore.SIGNAL("stateChanged(int)"),self.setUpdateDB)
        self.connect(self.sl_bufferFraction,QtCore.SIGNAL("valueChanged(int)"),self.doBufferFraction)
        self.connect(self.drp_Buffering,QtCore.SIGNAL("currentIndexChanged(const QString)"),self.doBufferingChange)
        self.connect(self.drp_RecordRawWaveforms,QtCore.SIGNAL("currentIndexChanged(const QString)"),self.doWaveformWriteChange)
        self.connect(QtGui.qApp,QtCore.SIGNAL("focusChanged( QWidget*, QWidget*)"),self.focusChanged)

#The new syntax requires PyQ>t4.5 I think
#        self.startButton.clicked.connect(self.doStart)


        self.endButton.setEnabled(False)
        self.tabWidget.setCurrentIndex(0)

        #Not required, but I think it makes things neater
        self.canvas = None
        self.system = NGMSystem.getSystem()
#        self.system = NGMSystem() #just a test
        if self.system == None : 
            print("WARNING: There is no NGMSystem!!!!!")
            self.configuration = None
        else : 
            self.configuration = NGMSystem.getSystem().GetConfiguration()

        self.systemTableWidget.ngmTable = None
        self.slotTableWidget.ngmTable = None
        self.channelTableWidget.ngmTable = None
        self.hvTableWidget.ngmTable = None
        self.detectorTableWidget.ngmTable = None
        
        self.timer = QtCore.QTimer()
        self.timer.setInterval(1000)
        self.connect(self.timer,QtCore.SIGNAL("timeout()"),self.doTimer)
        self.timer.start()

        
        ###Make example graphs
#        self.curve = Qwt.QwtPlotCurve()
#        self.curve.setBrush(QtGui.QBrush(QtGui.QColor(255,0,0),QtCore.Qt.Dense4Pattern))
#        self.curve.setStyle(Qwt.QwtPlotCurve.Steps)
#        self.curve.setData((0,1,2,3,4),(0,2,4,6,8))
#        self.curve.attach(self.Plot1)
#        self.Plot1.setAxisTitle(0,'Y')
#        self.Plot1.setAxisTitle(2,'X')
#        self.Plot1.setTitle('Title')
#        self.Plot1.setMinimumSize(10,10) # needed to fix zoomer
#        self.zoomer = Qwt.QwtPlotZoomer(self.Plot1.canvas())

#        self.curve2 = Qwt.QwtPlotCurve()
#        self.curve2.setBrush(QtGui.QBrush(QtGui.QColor(255,0,0),QtCore.Qt.Dense4Pattern))
#        self.curve2.setStyle(Qwt.QwtPlotCurve.Lines)
#        self.curve2.setData((0,1,2,3,4),(0,2,4,6,8))
#        self.curve2.attach(self.Plot2)
#        self.Plot2.setAxisTitle(0,'Y')
#        self.Plot2.setAxisTitle(2,'X')
#        self.Plot2.setTitle('Title')
#        self.Plot2.setMinimumSize(10,10)
#        self.zoomer2 = Qwt.QwtPlotZoomer(self.Plot2.canvas())

        self.refreshGui()


#######################  
    def doURLChange(self):
        self.webView.setUrl(QtCore.QUrl(str(self.urlEdit.text())))

#######################  
    def focusChanged(self, oldWidget, newWidget):
        if oldWidget != self.pt_Comment : return
        if self.pt_Comment.document().isModified() == False: return
        self.setComment()
        self.pt_Comment.document().setModified(False)
        

#######################  
    def doBufferingChange(self,newVal):
        if NGMSystem.getSystem() == None : return
        if newVal == "Single":
           NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterI("RunReadoutMode",0,1)
        if newVal == "Double":
           NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterI("RunReadoutMode",0,2)

#######################  
    def doWaveformWriteChange(self,newVal):
        if NGMSystem.getSystem() == None : return
        if newVal == "Never":
           NGMSystem.getSystem().GetConfiguration().GetSlotParameters().SetParameterAsStringThatBeginsWith("RawDataSampleMode","0","RawDataSampleMode","")
        if newVal == "Always":
           NGMSystem.getSystem().GetConfiguration().GetSlotParameters().SetParameterAsStringThatBeginsWith("RawDataSampleMode","1","RawDataSampleMode","")
        if newVal == "First":
           NGMSystem.getSystem().GetConfiguration().GetSlotParameters().SetParameterAsStringThatBeginsWith("RawDataSampleMode","2","RawDataSampleMode","")
        if newVal == "On Pileup":
           NGMSystem.getSystem().GetConfiguration().GetSlotParameters().SetParameterAsStringThatBeginsWith("RawDataSampleMode","3","RawDataSampleMode","")
        if newVal == "First or On Pileup":
           NGMSystem.getSystem().GetConfiguration().GetSlotParameters().SetParameterAsStringThatBeginsWith("RawDataSampleMode","4","RawDataSampleMode","")
        
#######################  
    def doBufferFraction(self):
        print("Set Buffer Fraction\n")
        if NGMSystem.getSystem() == None : return
        NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterD("BufferFractionPerSpill",0,self.sl_bufferFraction.value()/100.0)

####################### 
    def doHVOFF(self):
        NGMSystem.getSystem().GetConfiguration().GetHVParameters().SetParameterAsStringThatBeginsWith("Voltage","0.0","DetectorName","")
        self.refreshGui()

    def doGLHV(self):
        self.setHVbyName("GL")
            
    def doGSHV(self):
        self.setHVbyName("GS")
        
    def doMUHV(self):
        self.setHVbyName("MU")
        
    def doHEHV(self):
        self.setHVbyName("HETTL")
        
    def doLSHV(self):
        self.setHVbyName("LS")
        
    def setHVbyName(self, name):
        if NGMSystem.getSystem() == None : return
        if NGMSystem.getSystem().GetConfiguration() == None : return
        NGMSystem.getSystem().GetConfiguration().GetDetectorParameters().CopyParValuesMatching("TargetHV","Voltage","DetectorName",name,NGMSystem.getSystem().GetConfiguration().GetHVParameters())
        self.refreshGui()
        
    def setHostName(self):
        NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterS("DaqName",0,str(self.le_HostName.displayText()))

    def setDaqOperator(self):
        NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterS("DaqOperator",0,str(self.le_daqOperator.displayText()))
    
    def setFacility(self):
        NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterS("Facility",0,str(self.le_facility.displayText()))
    
    def setExperimentName(self):
        NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterS("ExperimentName",0,str(self.le_experimentName.displayText()))
    
    def setConfiguration(self):
        NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterS("Configuration",0,str(self.le_configuration.displayText()))
    
    def setComment(self):
        NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterS("Comment",0,str(self.pt_Comment.toPlainText()))
    
    def setMaxSeconds(self):
        maxseconds = self.te_maxseconds.time().hour()*3600+self.te_maxseconds.time().minute()*60+self.te_maxseconds.time().second()
        NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterI("RunMaxSecondsCounter",0,maxseconds)

    def setUpdateDB(self):
        NGMSystem.getSystem().SetUpdateDB(self.updateDataBase.isChecked())

#######################   
    def updateCells(self):
        cellcout = 0
        tablecount = 0
        for table in [self.systemTableWidget, self.slotTableWidget, 
                      self.channelTableWidget,  self.hvTableWidget, self.detectorTableWidget]:
            if len(table.selectedItems()) > 1:
                tablecount = tablecount + 1
                for cell in table.selectedItems():
                    cell.setText(self.cellLine.text())
                    cellcout = cellcout + 1
            table.clearSelection()
        self.refreshGui()
        print("There were " + str(cellcout) + " cells from " + str(tablecount) + " tables updated.")

####################### 
    def doTimer(self):
        if NGMSystem.getSystem() != None:
            
            self.eventsLine.setText(str(NGMSystem.getSystem().GetTotalEventsThisRun()))
            self.liveTimeLine.setText(str(NGMSystem.getSystem().GetLiveTime()))
            self.durationLine.setText(QtCore.QString("%1").arg(NGMSystem.getSystem().GetRunDuration(),0,"f",1))
            maxseconds = NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParValueI("RunMaxSecondsCounter",0)
            if maxseconds>0:
                self.fractionOfMaxDuration.setValue(NGMSystem.getSystem().GetRunDuration()/maxseconds*100)
            if NGMSystem.getSystem().GetRunDuration() > 0:
                self.liveFractionLine.setText(QtCore.QString("%1").arg(NGMSystem.getSystem().GetLiveTime()
                                                  /NGMSystem.getSystem().GetRunDuration(),0,'f',2))
            if NGMSystem.getSystem().GetConfiguration() != None:
                self.runNumLabel.setText('Run ' + str(NGMSystem.getSystem().GetConfiguration().getRunNumber()))    
                self.le_RunNumber.setText('Run ' + str(NGMSystem.getSystem().GetConfiguration().getRunNumber()))    
            
            if NGMSystem.getSystem().GetRunStatus() == 0:
                self.loadConfigurationButton.setEnabled(True)
                self.configButton.setEnabled(True)
                self.startButton.setEnabled(True)
                self.endButton.setEnabled(False)
            else:
                self.loadConfigurationButton.setEnabled(False)
                self.startButton.setEnabled(False)
                self.configButton.setEnabled(False)
                self.endButton.setEnabled(True)
                
            self.le_mbwritten.setText(str(NGMSystem.getSystem().GetBytesWritten()/1000000))
        else:
            self.startButton.setEnabled(False)
            self.endButton.setEnabled(False)
            self.configButton.setEnabled(False)

####################### 
#update the ngmtable when gui cell changes
    def syncCell(self,row,col):
        tableWidget = self.sender()
        ngmTable = tableWidget.ngmTable
        cell = tableWidget.item(row,col)
        if cell == None : return
#        print("Row "+str(row) +" col "+str(col) +" has new value " + cell.text())
        if ngmTable.IsList() or ngmTable.GetEntries() == 1 :
            ngmTable.SetParameterFromString(ngmTable.GetParName(row),0,str(cell.text()))
        else :
            ngmTable.SetParameterFromString(ngmTable.GetParName(col),row,str(cell.text()))
            
#######################
    def doStart(self):
        #Lets ask what type of run this is
        runTypeMessage = QtGui.QMessageBox(QtGui.QMessageBox.NoIcon, 'Run Type','Please select run type')
        junkButton = runTypeMessage.addButton('Junk', QtGui.QMessageBox.AcceptRole)
        calibButton = runTypeMessage.addButton('Calibration', QtGui.QMessageBox.AcceptRole)
        physButton = runTypeMessage.addButton('Physics', QtGui.QMessageBox.AcceptRole)
        runTypeMessage.setDefaultButton(physButton)
        runTypeMessage.exec_()
        if runTypeMessage.clickedButton() == physButton:
            NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterFromString("RunType",0,"PHYSICS")
        elif runTypeMessage.clickedButton() == calibButton:
            NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterFromString("RunType",0,"CALIBRATION")
        elif runTypeMessage.clickedButton() == junkButton:
            NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterFromString("RunType",0,"JUNK")
        else:
            print('Run type not selected')
            return()

#        NGMSystem.getSystem().StartAcquisition()
        self.startButton.setEnabled(False)
        self.configButton.setEnabled(False)
        self.endButton.setEnabled(True)
        self.loadConfigurationButton.setEnabled(False)
        NGMSystem.getSystem().LaunchAcquisitionStartThread()
#Is there a check I can do to see if it started

#        if self.canvas == None:
#            self.canvas = TCanvas("c1","My Canvas")
#            print ("Canvas is just an example")

####################### 
    def doStop(self):
        NGMSystem.getSystem().RequestAcquisitionStop()
        #Is there a test to see if it acually stoped
        self.startButton.setEnabled(True)
        self.configButton.setEnabled(True)
        self.endButton.setEnabled(False)
        self.loadConfigurationButton.setEnabled(True)

        self.canvas = None
        print ("Stop button clicked")

####################### 
    def doConfigure(self):
        NGMSystem.getSystem().ConfigureSystem()

####################### 
    def showTable(self, tableWidget):
        sysTable = tableWidget.ngmTable
        header = list()
        if sysTable.IsList() or sysTable.GetEntries() == 1 :
            tableWidget.setColumnCount(sysTable.GetEntries())
            tableWidget.setRowCount(sysTable.GetParCount())
            for i in range(sysTable.GetParCount()):
                header.append(sysTable.GetParName(i))
                tableWidget.setItem(
                    0,i,QtGui.QTableWidgetItem(sysTable.GetParameterAsString(header[i],0)))
                
            tableWidget.setVerticalHeaderLabels(header)
            tableWidget.setHorizontalHeaderLabels(['Value'])
        else :
            tableWidget.setColumnCount(sysTable.GetParCount())
            tableWidget.setRowCount(sysTable.GetEntries())
            for i in range(sysTable.GetParCount()):
                header.append(sysTable.GetParName(i))
                for j in range(sysTable.GetEntries()):
                    tableWidget.setItem(
                        j,i,QtGui.QTableWidgetItem(sysTable.GetParameterAsString(header[i],j)))

            tableWidget.setHorizontalHeaderLabels(header)
        
        tableWidget.resizeColumnsToContents()

####################### 
    def refreshGui(self):
        if self.configuration == None: return
        self.showConfig()
        self.showRunParameters()


#######################
    def getNewRawPath(self):

        rawDirName = QtGui.QFileDialog.getExistingDirectory(self,
                                                "Select Raw Path", "", QtGui.QFileDialog.ShowDirsOnly);
        if rawDirName != "" :
            self.le_RawOutputPath.setText(str(rawDirName))
            NGMSystem.getSystem().GetConfiguration().GetSystemParameters().SetParameterS("RawOutputPath",0,str(self.le_RawOutputPath.displayText()))

#######################
    def saveConfigFile(self):
        fileName = QtGui.QFileDialog.getSaveFileName(self,"Save Config","","*.root")
        if fileName != "":
             NGMSystem.getSystem().saveConfigFile(str(fileName))
                     
#######################
    def loadConfigFile(self):
        fileName = QtGui.QFileDialog.getOpenFileName(self,"Open Config","","*.root")
        if fileName != "":
             NGMSystem.getSystem().readConfigFile(str(fileName))
             self.setConfig(NGMSystem.getSystem().GetConfiguration())
             self.refreshGui()
                     
####################### 
    def setConfig(self, config):
        #if self.configuration != None: return
        self.configuration = config
        config.Print()
        self.refreshGui()

####################### 
    def showConfig(self):
        if self.configuration == None: return
        config = self.configuration

        sysTable = config.GetSystemParameters()
        if sysTable != None: 
            self.systemTableWidget.ngmTable = sysTable
            self.showTable(self.systemTableWidget)

        slotTable = config.GetSlotParameters()
        if slotTable != None:
            self.slotTableWidget.ngmTable = slotTable
            self.showTable(self.slotTableWidget)

        chanTable = config.GetChannelParameters()
        if chanTable != None:
            self.channelTableWidget.ngmTable = chanTable
            self.showTable(self.channelTableWidget)

        hvTable = config.GetHVParameters()
        if hvTable != None:
            self.hvTableWidget.ngmTable = hvTable
            self.showTable(self.hvTableWidget)

        detTable =  config.GetDetectorParameters()
        if detTable != None:
            self.detectorTableWidget.ngmTable = detTable
            self.showTable(self.detectorTableWidget)
            
        self.runNumLabel.setText('Run ' + str(config.getRunNumber()))

####################### 
    def showRunParameters(self):
        if NGMSystem.getSystem() != None:
            
            self.le_HostName.setText(NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParameterAsString("DaqName",0))
            self.le_daqOperator.setText(NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParameterAsString("DaqOperator",0))
            self.le_facility.setText(NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParameterAsString("Facility",0))
            self.le_experimentName.setText(NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParameterAsString("ExperimentName",0))
            self.le_HostName.setText(NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParameterAsString("DaqName",0))
            self.le_configuration.setText(NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParameterAsString("Configuration",0))
            self.pt_Comment.setPlainText(NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParameterAsString("Comment",0))
            self.liveTimeLine.setText(str(NGMSystem.getSystem().GetLiveTime()))
            self.durationLine.setText(str(NGMSystem.getSystem().GetRunDuration()))
            maxseconds = NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParValueI("RunMaxSecondsCounter",0)
            if maxseconds>0:
                self.fractionOfMaxDuration.setValue(NGMSystem.getSystem().GetRunDuration()/maxseconds*100)
            maxTime = QtCore.QTime()
            maxTime = maxTime.addSecs(maxseconds)
            self.te_maxseconds.setTime(maxTime)
            if NGMSystem.getSystem().GetRunDuration() > 0:
                self.liveFractionLine.setText(str(NGMSystem.getSystem().GetLiveTime()
                                                  /NGMSystem.getSystem().GetRunDuration()))
            self.le_RawOutputPath.setText(NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParValueS("RawOutputPath",0))
            if NGMSystem.getSystem().GetConfiguration() != None:
                self.le_RunNumber.setText('Run ' + str(NGMSystem.getSystem().GetConfiguration().getRunNumber()))    
                self.sl_bufferFraction.setValue(NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParValueD("BufferFractionPerSpill",0)*100)
                self.drp_Buffering.setCurrentIndex(NGMSystem.getSystem().GetConfiguration().GetSystemParameters().GetParValueI("RunReadoutMode",0)-1)
                self.drp_RecordRawWaveforms.setCurrentIndex(NGMSystem.getSystem().GetConfiguration().GetSlotParameters().GetParValueI("RawDataSampleMode",0))
        else:
            self.startButton.setEnabled(False)
            self.endButton.setEnabled(False)
            self.configButton.setEnabled(False)
        
