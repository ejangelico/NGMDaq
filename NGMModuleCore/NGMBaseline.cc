#include "NGMBaseline.h"
#include "NGMLogger.h"
#include <cmath>

ClassImp(NGMBaseline)

NGMBaseline::NGMBaseline()
:_gatelength(1),_minentries(1),
_totalEntries(0),_baseSum(0),_bcut(5.0)
{
    _bgate = 0;
}

NGMBaseline::NGMBaseline(double gatelength, int minentries, double sigthresh)
:_gatelength(gatelength),_minentries(minentries),
_totalEntries(0),_baseSum(0),_bcut(sigthresh),_consecutiveRejects(0),_maxConsecutiveRejects(5)
{
    _bgate = new ULong64_t[minentries];
}

NGMBaseline::~NGMBaseline(){
    delete[]_bgate;
}
bool NGMBaseline::MinHere() const
{
    return _totalEntries>=_minentries;
}

void NGMBaseline::Reset()
{
    _totalEntries=0;
    _baseSum=0;
    _consecutiveRejects=0;
    _totalRejects = 0;
}

void NGMBaseline::Print(Option_t* option) const
{
    LOG<<"Total Entries("<<(Long64_t)_totalEntries
    <<") MinEntries("<<(Long64_t)_minentries
    <<") Rejected("<<(Long64_t)_totalRejects
    <<")"<<ENDM_INFO;
    
    for(unsigned int i=0;i<_minentries;i++)
    {
        LOG<<i<<" "<<(Long64_t)_bgate[i]<<ENDM_INFO;
    }
    if(MinHere()){
        LOG<<"Avg("<<Avg()/(double)_gatelength
        <<") Std("<<RMS()<<")"<<ENDM_INFO;
    }
}

bool NGMBaseline::push(int nextVal)
{
    if(MinHere())
        _baseSum-=_bgate[(_totalEntries)%_minentries];
    _bgate[_totalEntries%_minentries]=nextVal;
    _baseSum+=nextVal;
    _totalEntries++;
    return true;
}

bool NGMBaseline::AnaBaseline( int bgate )
{
    bool pileup = false;
    
    if( MinHere() && fabs((bgate - Avg())/(double)_gatelength )>_bcut){
        pileup = true;
        _totalRejects++;
        _consecutiveRejects++;
    }else{
        _consecutiveRejects=0;
        push(bgate);
        if(MinHere()&&RMS()>_bcut)
        {
            //LOG<<"NGMBaseline contaminated ... resetting"<<ENDM_WARN;
            Reset();
        }
    }
    if(_consecutiveRejects>_maxConsecutiveRejects)
    {
        LOG<<"NGMBaseline max rejects reached ... resetting"<<ENDM_WARN;
        Reset();
    }
    return pileup;
}

double NGMBaseline::Avg() const
{
    //    //Sanity check on _baseSum Algorithm
    //    double sum = 0.0;
    //    for(int i =0; i<_minentries;i++)
    //    {
    //        sum+=_bgate[i];
    //    }
    //    if(sum!=_baseSum)
    //        std::cout<<"Sum not calculated correctly: Algorithmic error"<<std::endl;
    //    return sum/(double(_minentries));
    return _baseSum/(double(_minentries));
}

double NGMBaseline::RMS() const
{
    double avg=Avg();
    double sumsqr = 0.0;
    for(unsigned int i =0; i<_minentries;i++)
    {
        sumsqr+=pow(_bgate[i]-avg ,2.0);
    }
    return sqrt(sumsqr/_minentries)/_gatelength;
}

