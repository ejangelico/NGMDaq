void protoGeom(){
  NGMDisplay* disp = NGMDisplay::Instance();
  
  disp->AddProtoLSWall(198.74, 0, 0, 0, 0, 0, kRed, 1, 17, 22, 1);
  disp->AddProtoLSWall(-198.74, 0, 0, 0, 0, 0, kRed, 1, 17, 22, 1200);
  disp->AddProtoLSWall(0, 0, 249.24, 0, 0, 0, kRed, 17, 17, 1, 1900);
  disp->AddProtoLSWall(0, 0, -249.24, 0, 0, 0, kRed, 17, 17, 1, 2501);

}
