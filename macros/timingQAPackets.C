int plotIndex(const NGMHit* tHit)
{
  if(!tHit) return -1;
  return tHit->GetSlot()*8+ tHit->GetChannel();
}

void timingQAPackets(const char* fname)
{
  const int timingMaxChannels = 120;
  NGMBufferedPacket* packet[] = (NGMBufferedPacket**)( new size_t[timingMaxChannels] );

  for(int ipacket = 0; ipacket < timingMaxChannels; ipacket++)
  {
    packet[ipacket] = 0;
  }

  TFile* inputFile = TFile::Open(fname);
  if(!inputFile) return;
  
  TTree* pulse = (TTree*)inputFile->FindObjectAny("pulse");
  if(!pulse) return;

  NGMBufferedPacket* tPacket = new NGMBufferedPacketv1;
  pulse->SetBranchAddress("pulsebuffer",&tPacket);

  int prevChannel = 0;
  int chanIndex = -1;
  // Loop over all packets and test data integrity
  for(int ipacket = 0; ipacket < 1E5 /*pulse->GetEntries()*/; ipacket++)
  {
    pulse->GetEntry(ipacket);
    if(!tPacket) {std::cout << "Null packet returned\n"; return; }
    if( tPacket->getPulseCount() <1 ) continue;
    int pIndex = plotIndex(tPacket->getHit(0));

    chanIndex++;
    //std::cout<<ipacket<<" : "<<chanIndex<<" : "<<pIndex<<std::endl;


    if( ipacket == (pulse->GetEntries() -1) || prevChannel > pIndex )
    {
      // perform Checks on all packets in packet arrays
      bool chanCountBad = false;
      int firstPulseCount = packet[0]->getPulseCount();
      for(int ichan = 0; ichan < chanIndex; ichan++)
      {
        if(ichan > 0){
          if(firstPulseCount != packet[ichan]->getPulseCount())
          {
            std::cout<<"Count mismatch Spill("<<ipacket/chanIndex<<") First("<<firstPulseCount
              <<") This("<<packet[ichan]->getPulseCount()
              <<") Channel ("<<plotIndex(packet[ichan]->getHit(0))
              <<")\n";
            chanCountBad = true;
          }
        }else{
          //std::cout<<"First Channel Index "<<plotIndex(packet[ichan]->getHit(0))<<std::endl;
        }
      }
      // Loop over each hit index and compare timestamps from each channel
      if(!chanCountBad)
      for(int ihit = 0; ihit < packet[0]->getPulseCount(); ihit++)
      {
        NGMTimeStamp firstChanTime;
        firstChanTime = packet[0]->getHit(ihit)->GetNGMTime();
        for(int ichan = 1; ichan < chanIndex; ichan++)
        {
          //firstChanTime.Print();
          //std::cout<<firstChanTime.GetPicoSecs()<<std::endl;
          //packet[ichan]->getHit(ihit)->GetNGMTime().Print();
          //std::cout<<packet[ichan]->getHit(ihit)->GetNGMTime().GetPicoSecs()<<std::endl;
          if(fabs(packet[ichan]->getHit(ihit)->TimeDiffNanoSec(firstChanTime))>25.0 )
          {
            std::cout<<"Timing mismatch Spill("<<ipacket/chanIndex
              <<") Chan("<< plotIndex(packet[ichan]->getHit(ihit))
              <<") TimeDiff("<<packet[ichan]->getHit(ihit)->TimeDiffNanoSec(firstChanTime)
              <<")"<<std::endl;
          }
        }
      }

      // Delete all packet copies...
      for(int ichan = 0; ichan < chanIndex; ichan++)
      {
        delete packet[ichan];
        packet[ichan] = 0;
      }
      chanIndex = 0;
    }
    
    packet[chanIndex] = tPacket->DuplicatePacket();
    prevChannel = pIndex;
  }

}