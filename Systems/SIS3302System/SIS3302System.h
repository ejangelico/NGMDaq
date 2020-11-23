#include "NGMSystem.h"
#include "TTimeStamp.h"

class vme_interface_class; //Forward Declaration
class sis3302card; //Forward Declaration
class sis3820card; //Forward Declaration
class NGMBufferedPacket; //Forward Declaration

class SIS3302System: public NGMSystem
{
    
public:
    
    ///Constructor
    SIS3302System();
    
    ///Destructor
    virtual ~SIS3302System();
    
    /// \brief Generate a default configuration for this hardware
    int CreateDefaultConfig(const char* configname); // *MENU*
    
    int InitializeSystem(); // *MENU*
    int ConfigureSystem(); // *MENU*
    int StartAcquisition(); // *MENU*
    int StartListModeGeneral(); // *MENU*
    int StopAcquisition(); // *MENU*
    int RequestAcquisitionStop(); // *MENU*
    void PlotAdcDistribution(int slot = 0, int channel = 0); // *MENU*
    void Default3302Card(sis3302card* vcard);

    int writePacket(int packetlength, unsigned int *data);
    int WriteGlobalToTable();
    int WriteTableToGlobal();
    double GetLiveTime() const {return _livetime; }
    double GetRunDuration() const { return _runduration; }
    int GetTotalEventsThisRun() const { return _totalEventsThisRun; }
    virtual int GetRunStatus() const { return _isRunning; }
    virtual void GetStatus(TString& status);
    void SetDataFormat(int newVal){_dataFormat = newVal;} // *MENU*
    
    void SetMaxRawBytes(Long64_t newVal){ _maxbytestoraw = newVal; }// *MENU*
    
    void SetMaxSpillTime(double maxspilltime){_maxspilltime = maxspilltime;} // *MENU*
    int FetchData();
    int ParseSingleChannelData(sis3302card* vcard, int ichan);
    int WriteSpillToFile();
    int ReadActualADCValues();

    void SetNumberOfSlots(int nslots)
    {
        _numberOfSlots = nslots;
    }
    
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
    sis3820card* _c3820; //!
    int _numberOfSlots;
    
public:
    ClassDef(SIS3302System, 0)
};
