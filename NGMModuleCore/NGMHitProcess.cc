#include "NGMHitProcess.h"
#include "NGMSystemConfiguration.h"
#include "NGMConfigurationTable.h"
#include "NGMHit.h"
//#include "NGMBlockPSDMaker.h"
#include "NGMLogger.h"
#include "sis3316card.h"
#include <cmath>
#include <iostream>
#include "NGMHitPSD.h"

NGMHitProcess::NGMHitProcess()
{
    _hitpsd = 0;
    _doPileupCorrection = 0;
    _doRecalcFromWaveforms = false;
    _psdScheme = 0;

}

void NGMHitProcess::Print(Option_t *option) const
{
    std::cout<<"Print PSD("<<_psd<<")"<<std::endl;
}

void NGMHitProcess::SetBaseline( double nomBaseline, double nomBaselineRMS, double rctau)
{
    _nominalBaseline = nomBaseline;
    _nominalBaselineRMS=nomBaselineRMS;
    _rctau=rctau;
}

void NGMHitProcess::SetPileupCorrection(int doPileup)
{
    _doPileupCorrection=doPileup;
}

bool NGMHitProcess::Init(const NGMSystemConfiguration* conf, int chanseq)
{
    _chanSeq = chanseq;
   _nsPerClock=0.0;
    const NGMConfigurationParameter* nspar = conf->GetSlotParameters()->GetColumn("NanoSecondsPerSample");
    if(conf->GetChannelParameters()->GetColumn("QDCLen0"))
    {
        _gatewidth.resize(8);
        _gatestart.resize(8);
        if (nspar){
            int slot = chanseq/16;
            if(nspar->GetValue(slot)>0.0)
                _nsPerClock = nspar->GetValue(slot);
            }
            // For XIA the sampling may be half the clock value
            double nsPerSample =_nsPerClock;
            _gatewidth[0] = conf->GetChannelParameters()->GetParValueD("QDCLen0",_chanSeq)*1000.0/nsPerSample;
            _gatewidth[1] = conf->GetChannelParameters()->GetParValueD("QDCLen1",_chanSeq)*1000.0/nsPerSample;
            _gatewidth[2] = conf->GetChannelParameters()->GetParValueD("QDCLen2",_chanSeq)*1000.0/nsPerSample;
            _gatewidth[3] = conf->GetChannelParameters()->GetParValueD("QDCLen3",_chanSeq)*1000.0/nsPerSample;
            _gatewidth[4] = conf->GetChannelParameters()->GetParValueD("QDCLen4",_chanSeq)*1000.0/nsPerSample;
            _gatewidth[5] = conf->GetChannelParameters()->GetParValueD("QDCLen5",_chanSeq)*1000.0/nsPerSample;
            _gatewidth[6] = conf->GetChannelParameters()->GetParValueD("QDCLen6",_chanSeq)*1000.0/nsPerSample;
            _gatewidth[7] = conf->GetChannelParameters()->GetParValueD("QDCLen7",_chanSeq)*1000.0/nsPerSample;
        _gatestart[0]=0;
        for(int iqdc = 1; iqdc<_gatewidth.size(); iqdc++)
        {
            _gatestart[iqdc]=_gatewidth[iqdc-1]+_gatestart[iqdc-1];
        }
    }else if(conf->GetSlotParameters()->GetColumn("card")){
        std::cout<<" Card Type:"<<conf->GetSlotParameters()->GetParValueS("cardtype",0)<<std::endl;
        if(conf->GetSlotParameters()->GetParValueS("cardtype",0)==TString("sis3316card"))
        {
            int islot =  _chanSeq/SIS3316_CHANNELS_PER_CARD;
            if(nspar){
                _nsPerClock = nspar->GetValue(islot);
            }
            _gatewidth.resize(8);
            _gatestart.resize(8);
            const sis3316card* card = dynamic_cast<const sis3316card*>(conf->GetSlotParameters()->GetParValueO("card",islot));
            int iblock = _chanSeq%SIS3316_CHANNELS_PER_CARD/SIS3316_CHANNELS_PER_ADCGROUP;
            _waveformlength = card->sample_length_block[iblock];
            _waveformstart = card->sample_start_block[iblock];
            for(int iqdc =0; iqdc<SIS3316_QDC_PER_CHANNEL; iqdc++)
            {
                _gatewidth[iqdc] = card->qdclength[_chanSeq%SIS3316_CHANNELS_PER_CARD/SIS3316_CHANNELS_PER_ADCGROUP][iqdc];
                _gatestart[iqdc] = card->qdcstart[_chanSeq%SIS3316_CHANNELS_PER_CARD/SIS3316_CHANNELS_PER_ADCGROUP][iqdc];
            }
        }else{
            std::cerr<<"Matching card not found "<<std::endl;
        }
    }
    TString detName(conf->GetChannelParameters()->GetParValueS("DetectorName",_chanSeq));
    LOG<<"Using Widths "
    <<_gatewidth[0]<<" "
    <<_gatewidth[1]<<" "
    <<_gatewidth[2]<<" "
    <<_gatewidth[3]<<" "
    <<_gatewidth[4]<<" "
    <<_gatewidth[5]<<" "
    <<_gatewidth[6]<<" "
    <<_gatewidth[7]
    <<" for detector "<<detName.Data()
    <<ENDM_INFO;
    
    if(_psdScheme==0)
    {
        
        if(conf->GetChannelParameters()->GetParIndex("PSD_SCHEME")>=0)
        {
            _psdScheme = conf->GetChannelParameters()->GetParValueI("PSD_SCHEME",chanseq);
        }else if(conf->GetSystemParameters()->GetParIndex("XIAPSD_SCHEME")>=0)
        {
            _psdScheme = conf->GetSystemParameters()->GetParValueI("XIAPSD_SCHEME",0);
        }else if(conf->GetSystemParameters()->GetParIndex("PSD_SCHEME")>=0)
        {
            _psdScheme = conf->GetSystemParameters()->GetParValueI("PSD_SCHEME",0);
        }
        LOG<<" Using PSDScheme "<<_psdScheme << " ";
        if(_psdScheme==1 || _psdScheme ==2)
            LOG<<" ** Tail Pulse with Interpolation ** ";
        else if(_psdScheme)
            LOG<<" ** Anode Pulse with Prompt in qdc1 and total in qdc2 ** ";
        
        LOG<<ENDM_INFO;
    }
    if(detName!="")
    {
        int detRow = conf->GetDetectorParameters()->FindFirstRowMatching("DetectorName",detName);
        const NGMHitPSD* hitPSD = 0;
        if(detRow>=0)
        {
            _pixelkeVADC =conf->GetDetectorParameters()->GetParValueD("keVperADC",detRow);
            LOG<<"Found energy calibration "<<_pixelkeVADC<<" keVPerADC"<<ENDM_INFO;
            hitPSD = dynamic_cast<const NGMHitPSD*>(conf->GetDetectorParameters()->GetParValueO("HIT_PSD",detRow));
            if(hitPSD)
            {
                LOG<<"Found "<<hitPSD->GetName()<<" for "<<detName.Data()<<ENDM_INFO;
                _hitpsd=hitPSD;
                LOG<<" Found HitPSD "<< hitPSD->GetName()<<" for detector "<<detName.Data()
                << ENDM_INFO;
            }
            // Look for an energy calibration parameter
            
        } // if(detRow>=0)
    } // if detName!=""
    return true;
}

bool NGMHitProcess::ProcessHit( NGMHit* hit)
{
    ProcessHit((const NGMHit*) hit);
    //Timing Calibrations should not be done here
    //hit->SetCalibratedTime(_ts);
    hit->SetPulseHeight(_pulseHeight);
    hit->SetPixel(-1);
    hit->SetEnergy(_energy);
    hit->SetPSD(_psd);
    hit->SetNeutronId(_neutronSigma);
    hit->SetGammaId(_gammaSigma);
    
    return true;
}

bool NGMHitProcess::ProcessHit(const NGMHit* hit)
{

    // psdScheme
    // 0 is Block Detector old XIA algorithm with 8 gates
    // 1 is Block Detector XIA algorithm 8 gates
    // 2 is Block Detector Struck algorithm using 5 gates
    // 3 is Single PMT PSD Detectors using 3 gates
    // 4 is Dual PMT Liquid Cells
    
    const int npmt = 1;
    double g1w = _gatewidth[0];
    int ngates = 0;
    int nsPerClock = _nsPerClock;
    int totalPair=0;
    int promptPair=0;
    double cfd = hit->GetCFD();
    if(_psdScheme==1)
    {
        if (_nsPerClock == 0.0) nsPerClock = 10.0;
        ngates = 8;
        promptPair=3;
        totalPair=6;
    }else if(_psdScheme==2){
        if (_nsPerClock == 0.0) nsPerClock = 4.0;
        ngates = 5;
        promptPair=1;
        totalPair=3;
    }else if(_psdScheme==3){// Single Cell PSD
        if (_nsPerClock == 0.0) nsPerClock = 10.0;
        ngates = 6;
        promptPair=1;
        totalPair=3;
    }else if(_psdScheme==4){ //Dual PMT Single Cell PSD
        if (_nsPerClock == 0.0) nsPerClock = 4.0;
        ngates = 6;
        promptPair=1;
        totalPair=3;
    }else if(_psdScheme==5){// NaI
        if (_nsPerClock == 0.0) nsPerClock = 4.0;
        ngates = 6;
        promptPair=1;
        totalPair=3;
    }
    if(_psdScheme==1||_psdScheme==2){
        //Time calibration should not be done here
        //_ts = hit->GetRawTime();
        //_ts.IncrementNs(10.0*cfd);
        
        for(int ipmt = 0; ipmt<npmt;ipmt++)
        {
            _baseline=hit->GetGate(0)/g1w;
            _pulseHeight=hit->GetGate(totalPair)*(1.0-cfd) + hit->GetGate(totalPair+1)*cfd - _baseline;
            _prompt=hit->GetGate(promptPair)*(1.0-cfd) + hit->GetGate(promptPair+1)*cfd - _baseline;
        }
    }else if(_psdScheme==3)
    {
        if(!_doRecalcFromWaveforms){
            _baseline = hit->GetGate(0)/_gatewidth[0];
            _prompt = hit->GetGate(1)-_baseline*_gatewidth[1];
            _pulseHeight= hit->GetGate(2)-_baseline*_gatewidth[2];
        }else{
            _baseline = hit->ComputeSum(_gatestart[0],_gatewidth[0])/_gatewidth[0];
            _prompt = hit->ComputeSum(_gatestart[1],_gatewidth[1])-_baseline*_gatewidth[1];
            _pulseHeight= hit->ComputeSum(_gatestart[2],_gatewidth[2])-_baseline*_gatewidth[2];
        }
    }else if(_psdScheme==4)
    {
        if(!_doRecalcFromWaveforms){
            double bl1 =hit->GetGate(0)/_gatewidth[0];
            double bl2 =hit->GetGate(0+5)/_gatewidth[0];
            _baseline = bl1+bl2;
            double p1 = hit->GetGate(1)-bl1;
            double p2 = hit->GetGate(1+5)-bl1;
            double tot1 = hit->GetGate(2)-bl1;
            double tot2 = hit->GetGate(2+5)-bl2;
            _prompt = sqrt(p1*p2);
            _pulseHeight= sqrt(tot1+tot2);
        }else{
            double bl1 =hit->ComputeSum(_gatestart[0]-_waveformstart, _gatewidth[0])/_gatewidth[0];
            double bl2 =hit->ComputeSum(_gatestart[0]-_waveformstart+_waveformlength, _gatewidth[0])/_gatewidth[0];
            _baseline = bl1+bl2;
            double p1 = hit->ComputeSum(_gatestart[1]-_waveformstart, _gatewidth[1])-bl1;
            double p2 = hit->ComputeSum(_gatestart[1]-_waveformstart+_waveformlength, _gatewidth[1])-bl2;
            double tot1 = hit->ComputeSum(_gatestart[2]-_waveformstart, _gatewidth[2])-bl1;
            double tot2 = hit->ComputeSum(_gatestart[2]-_waveformstart+_waveformlength, _gatewidth[2])-bl2;
            _prompt = sqrt(p1*p2);
            _pulseHeight= sqrt(tot1+tot2);
           
            
            _baseline = hit->ComputeSum(_gatestart[0],_gatewidth[0])/_gatewidth[0]
                +hit->ComputeSum(_gatestart[0],_gatewidth[0])/_gatewidth[0];
            _prompt = hit->ComputeSum(_gatestart[1],_gatewidth[1])-_baseline*_gatewidth[1];
            _pulseHeight= hit->ComputeSum(_gatestart[2],_gatewidth[2])-_baseline*_gatewidth[2];
        }
    }else if(_psdScheme==5)//NaI
    {
        if(!_doRecalcFromWaveforms){
            _baseline = hit->GetGate(0)/_gatewidth[0];
            _prompt = hit->GetGate(1)-_baseline*_gatewidth[1];
            _pulseHeight= hit->GetGate(2)-_baseline*_gatewidth[2];
        }else{
            _baseline = hit->ComputeSum(_gatestart[0],_gatewidth[0])/_gatewidth[0];
            _prompt = hit->ComputeSum(_gatestart[1],_gatewidth[1])-_baseline*_gatewidth[1];
            _pulseHeight= hit->ComputeSum(_gatestart[2],_gatewidth[2])-_baseline*_gatewidth[2];
        }
    }

    // Reject RePileups
    if(_doPileupCorrection == 2)
    {
        
        bool reject = false;
        for(int ipmt = 0; ipmt<npmt;ipmt++)
        {
            printf("%f(%f) ",fabs(_baseline-_nominalBaseline),_nominalBaselineRMS);
            if(fabs(_baseline-_nominalBaseline)>_nominalBaselineRMS)
                reject=true;
        }
        printf("\n");
        if(reject)
            return false;
    }
    

    
    if(_doPileupCorrection == 1)
    {

            double I0 = (_baseline - _nominalBaseline)*_gatewidth[0]
                        /(_rctau*(1.0 - exp(-_gatewidth[0]*nsPerClock/_rctau)));
            double promptCorrection = I0*exp(-(_gatestart[promptPair]+cfd)*nsPerClock/_rctau);
            double totalCorrection = I0*exp(-(_gatestart[totalPair]+cfd)*nsPerClock/_rctau);
            double oldBaseline = _baseline;
            _pulseHeight-= totalCorrection;
            _prompt-= promptCorrection;
            _baseline=_nominalBaseline;
        
    }
    
    _energy = _pulseHeight*_pixelkeVADC;
    
    if(_psdScheme==1||_psdScheme==2||_psdScheme==3||_psdScheme==4)
    {
        _psd = _prompt/_pulseHeight;

        if(_hitpsd)
        {
            _neutronSigma= _hitpsd->GetNSigma(_psd,_energy);
            _gammaSigma = _hitpsd->GetGSigma(_psd,_energy);
        }
    }
    
    
    return true;
}
