void drawLSHexHenge()
{
  double rcenter = 22.0 + 3.75; //cm
  
  for(int ils = 0; ils < 6; ils++)
  {
    double pangle = TMath::Pi()/3.0*ils;
    double xcenter =rcenter*cos(pangle);
    double ycenter =rcenter*sin(pangle);
    NGMDisplay::Instance()->AddLSAssembly(xcenter,ycenter,0.0,0,0,pangle*180.0/TMath::Pi()-90.0,(int)kRed);
  }
  
}
