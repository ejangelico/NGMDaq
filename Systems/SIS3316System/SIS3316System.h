#include "NGMSystem.h"
#include "TTimeStamp.h"

class vme_interface_class; //Forward Declaration
class sis3316card; //Forward Declaration
class NGMBufferedPacket; //Forward Declaration

class SIS3316System: public NGMSystem
{
    
public:
    
    ///Constructor
    SIS3316System();
    
    ///Destructor
    virtual ~SIS3316System();
    
    /// \brief Generate a default configuration for this hardware
    int CreateDefaultConfig(const char* configname); // *MENU*
    
    int InitializeSystem(); // *MENU*
    int ConfigureSystem(); // *MENU*
    int StartAcquisition(); // *MENU*
    int StartListModeGeneral(); // *MENU*
    int StopAcquisition(); // *MENU*
    int RequestAcquisitionStop(); // *MENU*
    int readConfigFile(const char* configfile); // *MENU*
    void PlotAdcDistribution(int slot = 0, int channel = 0); // *MENU*
    void Default3316Card(sis3316card* vcard);

    int writePacket(int packetlength, unsigned int *data);
    int WriteGlobalToTable();
    int WriteTableToGlobal();
    double GetLiveTime() const {return _livetime; }
    double GetRunDuration() const { return _runduration; }
    int GetTotalEventsThisRun() const { return _totalEventsThisRun; }
    virtual int GetRunStatus() const { return _isRunning; }
    void SetDataFormat(int newVal){_dataFormat = newVal;} // *MENU*
    
    void SetMaxRawBytes(Long64_t newVal){ _maxbytestoraw = newVal; }// *MENU*
    
    void SetMaxSpillTime(double maxspilltime){_maxspilltime = maxspilltime;} // *MENU*
    int FetchData();
    int ParseADCBlockData(sis3316card* vcard, int iadc);
    int ParseSingleChannelData(sis3316card* vcard, int ichan);
    int WriteSpillToFile();
    
    void SetNumberOfSlots(int nslots)
    {
        _numberOfSlots = nslots;
    }
    void SetInterfaceType(const char* vmeinterfacestr);
    
    /// \brief Get Bytes Written this run
    Long64_t GetBytesWritten(){ return _totalBytesWritten; } //*MENU*
    
    /// \brief Request Update of all plots
    TArrayI GetBufferLevelArray(){ return _bufferlevels; } //*MENU*
    
    vme_interface_class* GetVMEInterface(){return vmei;}
    
    NGMBufferedPacket* _outBuffer;
private:
    void plotBufferLevels(); // *MENU*
    void updateBufferLevels();
    void closeRawOutputFile();
    void openRawOutputFile();
    
    vme_interface_class* vmei; //!
    UInt_t _broadcastbase;
    TTimeStamp* _runBegin;
    bool _requestStop;
    double _livetime; // seconds
    double _runduration; //seconds
    int _totalEventsThisRun;
    TTimeStamp _firstTimeOfRun;
    TTimeStamp _earliestTimeInSpill;
    TTimeStamp _latestTimeInSpill;
    int _lastPlotIndex;
    int _dataFormat; // 0 for root, 1 for SIS RAW BINARY
    FILE* _rawFilePointer; //!
    int _rawFileCount; //!
    Long64_t _totalBytesWritten; //!
    
    Long64_t _maxbytestoraw; //!
    double   _maxspilltime; //!
    TArrayI  _bufferlevels;
    TArrayI  _bufferlevelsids;
    bool _isRunning;
    int _numberOfSlots;
    std::string _vmeinterfacetype; //SISUSB, TSI148, etc

public:
    ClassDef(SIS3316System, 0)
};
