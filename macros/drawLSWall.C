void drawLSWall()
{
  double separation = 30.0; //inches
  for(int ils = 0; ils < 6; ils++)
  {
    double xcenter =separation*(ils-3.5);
    double ycenter  = 0;
    NGMDisplay::Instance()->AddLSAssembly(xcenter,ycenter,0.0,0,0,180.0,(int)kRed);
  }
  
}