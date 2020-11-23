void protoGeomHE(){
  NGMDisplay* disp = NGMDisplay::Instance();
  
  disp->AddProtoLSWall(198.74, 0, 0, 0, 0, 0, kRed, 1, 17, 22, 1);
  disp->AddProtoLSWall(-198.74, 0, 0, 0, 0, 0, kRed, 1, 17, 22, 1200);
  disp->AddProtoLSWall(0, 0, 249.24, 0, 0, 0, kRed, 17, 17, 1, 1900);
  disp->AddProtoLSWall(0, 0, -249.24, 0, 0, 0, kRed, 17, 17, 1, 2501);
	
  Color_t tubeColor = TColor::GetColor(127,127,127);
  char detName[128];
  TGeoVolume* HEgeo;
  for(int j=0; j<5;j++){
    for(int i=0; i<68;i++){
      sprintf(detName, "HE%04d", j*68 + i + 401);
      HEgeo = gGeoManager->MakeTube(detName, 0, 0, 2.54, 45);
      HEgeo->SetLineColor(tubeColor);
      gGeoManager->GetTopVolume()->AddNode(HEgeo, 0, new TGeoCombiTrans(183.04, -176.88 + i*5.28, 91.66*(j + 0.5) - 228.334, new TGeoRotation("r",90,0,0)));
    }
  }
  for(int j=0; j<5;j++){
    for(int i=0; i<68;i++){
      sprintf(detName, "HE%04d", j*68 + i + 801);
      HEgeo = gGeoManager->MakeTube(detName, 0, 0, 2.54, 45);
      HEgeo->SetLineColor(tubeColor);
      gGeoManager->GetTopVolume()->AddNode(HEgeo, 0, new TGeoCombiTrans(-183.04, -176.88 + i*5.28, 91.66*(j + 0.5) - 228.334, new TGeoRotation("r",90,0,0)));
    }
  }
  for(int j=0; j<4;j++){
    for(int i=0; i<68;i++){
      sprintf(detName, "HE%04d", j*68 + i + 1601);
      HEgeo = gGeoManager->MakeTube(detName, 0, 0, 2.54, 44.5);
      HEgeo->SetLineColor(tubeColor);
      gGeoManager->GetTopVolume()->AddNode(HEgeo, 0, new TGeoCombiTrans(-179.5 + 90.0*(j + 0.5), -176.88 + i*5.28, 233.54, new TGeoRotation("r",90,90,0)));
    }
  }
  for(int j=0; j<4;j++){
    for(int i=0; i<68;i++){
      sprintf(detName, "HE%04d", j*68 + i + 2201);
      HEgeo = gGeoManager->MakeTube(detName, 0, 0, 2.54, 44.5);
      HEgeo->SetLineColor(tubeColor);
      gGeoManager->GetTopVolume()->AddNode(HEgeo, 0, new TGeoCombiTrans(-179.5 + 90.0*(j + 0.5), -176.88 + i*5.28, -233.54, new TGeoRotation("r",90,90,0)));
    }
  }
  
}
