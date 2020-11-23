#!/usr/local/bin/bash

source /usr/gapps/ngm/NGMcurrent/setupyana.sh
wrkdir=/tmp/$USER/ngmquickstart
datapath="/proj/ngm/simu3/RFShells3-26withShielding/5000_lbs_lead/helium_tubes/simulation20081103234144-ngm.root"
#datapath="/proj/ngm/simu2/Aug08/RockyFlatShells/cr+spfiss/RFshells3-30/mcnpx/simulation20080915233319-ngm.root"

if [ ! -f $HOME/.ngmdbpw ] ; then
    cp /usr/gapps/ngm/.ngmdbpw $HOME
    chmod go-rwx $HOME/.ngmdbpw
fi

if [ ! -d $wrkdir ] ; then  
    mkdir -p $wrkdir
fi 

cd $wrkdir

datadir=$( dirname $datapath )
datafile=$( basename $datapath )

if [ ! -f $datafile ] ; then

if [ -f ftp.input ] ; then
    rm ftp.input
fi

echo $datadir
echo $datafile

ftp storage<<EOF


cd $datadir
get $datafile
bye
EOF

fi

cat > quickStart.C << EOF
void quickStart()
{
  gROOT->SetStyle("Plain");

  NGMMultiFormatReader* fin = new NGMMultiFormatReader;
  fin->SetPassName("$USERQuickTest");

  NGMCalibrator* mCal = new NGMCalibrator("mCal","mCal");
  mCal->SetAllowDatabaseReads();

  NGMPacketMergeSort* mMerge = new NGMPacketMergeSort("mMerge","mMerge");
  mMerge->setMergeMode(NGMPacketMergeSort::NoWaitOnPackets);
  mMerge->setVerbosity(0);
  mMerge->SetPlotFrequency(60.0);
  //mMerge->SetMaxLiveTime(600.0);

  NGMRateMonitor* rmon = 0;
  rmon = new NGMRateMonitor("RATEMON","RATEMON");
  if(rmon)
  {
    rmon->SetAllowDatabaseReads();
    //rmon->SetUpdateDB();
  }

   NGMCountDistR* countdist = 0;
   countdist = new NGMCountDistR("CDistSN","CDistSN");
   if(countdist){
    countdist->setGateInterval(10.0);
    countdist->getParticleIdent()->AddCut(NGMSimpleParticleIdent::hettlid,-1.,0.);
    countdist->SetDisplayInterval(1E9);
    countdist->setPromptStepForY2FandZ2F(false);
    countdist->setAllGraphs(false);
    countdist->useFastNeutronEquation(false);
    countdist->SetAllowDatabaseReads();
    //countdist->SetUpdateDB();
  }

  fin->Add(mCal);
  mCal->Add(mMerge);

  if (countdist) mMerge->Add(countdist);
  if (rmon) mMerge->Add(rmon);
  fin->initModules();

  gROOT->Add(fin->GetParentFolder());

  new TBrowser;

  fin->OpenInputFile("$datafile");
  fin->StartAcquisition();
  fin->SaveAnaTree();

}

EOF

#NGMBatch quickStart.C 2>&1 > quickStart.log
NGMBatch quickStart.C