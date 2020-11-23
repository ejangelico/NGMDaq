
TH2D* itod(TH2* hIn)
{
  TString hName = hIn->GetName();
  hName+="_D";
  TH2D* hOut = new TH2D(hName.Data(),hName.Data(),
                        hIn->GetNbinsX(),hIn->GetXaxis()->GetXmin(),hIn->GetXaxis()->GetXmax(),
                        hIn->GetNbinsY(),hIn->GetYaxis()->GetXmin(),hIn->GetYaxis()->GetXmax());
  for(int ybin = 1; ybin<=hIn->GetNbinsY(); ybin++)
      for(int xbin = 1; xbin<=hIn->GetNbinsX(); xbin++)
        hOut->SetBinContent(xbin,ybin,hIn->GetBinContent(xbin,ybin));
  return hOut;
}


void plotMaskRatio()
{
  TH2* BlockArray_Cf252Mask = (TH2*)(gDirectory->FindObjectAny("BlockArray_Cf252Mask"));
  BlockArray_Cf252NoMask->SetName("BlockArray_Cf252Mask");
  TH2* BlockArray_Cf252NoMask = (TH2*)(gDirectory->FindObjectAny("BlockArray_Cf252NoMask"));
  BlockArray_Cf252NoMask->SetName("BlockArray_Cf252NoMask");
  TH2D* BlockArray_Cf252Mask_D = itod(BlockArray_Cf252Mask);
  TH2D* BlockArray_Cf252NoMask_D = itod(BlockArray_Cf252NoMask);
  BlockArray_Cf252Mask_D->Divide(BlockArray_Cf252NoMask_D);
  BlockArray_Cf252Mask_D->SetName("MaskRatio");
  BlockArray_Cf252Mask_D->SetTitle("MaskRatio");
  BlockArray_Cf252Mask_D->Draw("colz");
}