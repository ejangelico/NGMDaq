//#include "/Users/glenn22/NGM/NGMDaq_i/include/NGMGeomVals.h"
using namespace NGMGeomVals;
void pile070108Geom(){
  NGMDisplay* disp = NGMDisplay::Instance();

  int orderA[4] = {5,14,7,13};
  for (int i=0;i<4;i++){
    disp->AddMuPanel(-1.5*MUWidth+i*MUWidth,MULength+17.78,0,0,0,0,kMagenta,orderA[i]);
  }
  int orderB[4] = {18,12,4,10};
  for (int i=0;i<4;i++){
    disp->AddMuPanel(-1.5*MUWidth+i*MUWidth,-MULength-16.5,0,0,0,0,kMagenta,orderB[i]);
  }
  int orderC[3] = {15,11,2};
  for (int i=0;i<3;i++){
    disp->AddMuPanel(-33.02-2.5*MUWidth+i*MUWidth,0,0,0,0,0,kMagenta,orderC[i]);
  }
  int orderD[3] = {3,8,16};
  for (int i=0;i<3;i++){
    disp->AddMuPanel(33.02+0.5*MUWidth+i*MUWidth,0,0,0,0,0,kMagenta,orderD[i]);
  }
  disp->AddMuPanel(0,0,MUHeight,90,0,0,kMagenta,1);
  disp->AddMuPanel(0,0,35.56 + 50.8,90,0,0,kMagenta,17);
  disp->AddMuPanel(0,-MUWidth/2.0-MUHeight/2.0, 35.56 + 15.24 +MUWidth/2.0 -MUHeight/2.0, 0,90,90,kMagenta,9);
  disp->AddMuPanel(0,+MUWidth/2.0+MUHeight/2.0, 35.56 + 15.24 +MUWidth/2.0 -MUHeight/2.0, 0,90,90,kMagenta,6);

  int orderE[11] = {2,8,7,5,3,4,6,1,11,9,12};
  disp->AddGLAssemblyC(-71.12 - GLWidth/2.0 - HEAssemHeight ,0,0,0,0,0,kRed);
  int orderF[10] = {11,5,10,6,2,1,7,9,8,12};
  disp->AddGSAssemblyB(72.39 + GSWidth/2.0 + HEAssemHeight,0,0,0,0,0,kBlue);

  disp->AddHeAssembly(-71.12 - HEAssemHeight/2.0,-HEAssemWidth/2.0,GLCartFloor,0,0,0,kGreen);
  disp->AddHeAssembly(72.39 + HEAssemHeight/2.0,-HEAssemWidth/2.0,GSCartFloor,0,0,0,kGreen);
  disp->AddHeAssembly(72.39 + HEAssemHeight/2.0,HEAssemWidth/2.0,GSCartFloor,0,0,0,kGreen);
  disp->AddHeAssembly(-71.12 - HEAssemHeight/2.0,HEAssemWidth/2.0,GLCartFloor,0,0,0,kGreen);

  TGeoVolume* pile = gGeoManager->MakeBox("PbPile",0, 20.32, 20.32, 25.4);
  pile->SetLineColor(kYellow);
  gGeoManager->GetTopVolume()->AddNode(pile,0,new TGeoCombiTrans(0, 0, 25.4 + 35.56, new TGeoRotation("r",0,0,0)));
  pile = gGeoManager->MakeBox("PbPileF",0, 20.32, 5.08, 7.62);
  pile->SetLineColor(kYellow);
  gGeoManager->GetTopVolume()->AddNode(pile,0,new TGeoCombiTrans(0, -5.08/2.0-20.32, 7.62 + 35.56, new TGeoRotation("r",0,0,0)));
  pile = gGeoManager->MakeBox("PbPileB",0, 20.32, 5.08, 7.62);
  pile->SetLineColor(kYellow);
  gGeoManager->GetTopVolume()->AddNode(pile,0,new TGeoCombiTrans(0, 5.08/2.0+20.32, 7.62 + 35.56, new TGeoRotation("r",0,0,0)));


}
