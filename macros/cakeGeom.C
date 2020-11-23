void cakeGeom(){
	
	NGMDisplay* disp = NGMDisplay::Instance();
	
	//lengths are in cm	
	double LSrad = 5.08;
	double LShalfLen = 3.81;
	double stackWidth = 24.13;
	double xSep = 11.192;
	double zSep = 11.192;
	
	int NSTACKS = 8;
	double oRadius = stackWidth/2.0/TMath::Tan(TMath::Pi()/NSTACKS) + LShalfLen;
	
	int lsNum = 2; //The A stacks weren't used
	double delx = 0;
	double dely = 0;
	double delz = 0;
	double alpha = 0;
	double beta = 0;
	double gamma = 0;
	int color = kYellow;
	char detName[128];
	
	for(int iStack=0; iStack<NSTACKS; iStack++){
		delx = oRadius*TMath::Sin(2.0*TMath::Pi()/NSTACKS*iStack);
		dely = oRadius*TMath::Cos(2.0*TMath::Pi()/NSTACKS*iStack);
		alpha = -TMath::RadToDeg()*2.0*TMath::Pi()/NSTACKS*iStack;
		sprintf(detName, "LS_Stack_%d", iStack);
		TGeoVolumeAssembly* lsVol = new TGeoVolumeAssembly(detName);
		TObjArray* vollist = lsVol->GetNodes();
		TGeoVolume* LSgeo;
		for(int j=0; j<4; j++){
			for(int i=0; i<2; i++){
				int temp_i = i;
				if(j==3) temp_i = TMath::Abs(i-1);
				sprintf(detName, "LS%02d", 1+(temp_i+j*2)+8*lsNum);
				TGeoMedium* medium = new TGeoMedium(detName, 0, new TGeoMaterial("Fake",1,1,1,1,1));
				LSgeo = gGeoManager->MakeTube(detName, medium, 0, LSrad, LShalfLen);
				LSgeo->SetLineColor(color);
				//Two magic numbers; distance from floor to 1st center and added space from 6pack +2pack
				lsVol->AddNode(LSgeo,0,new TGeoCombiTrans(i*xSep-xSep/2.0, 0, j*zSep + 6.27 + (j==3 ? 1.826:0.0) , new TGeoRotation("r",0,90,0)));
				if(!vollist) vollist = lsVol->GetNodes();
				if(disp->GetNodes()) disp->GetNodes()->AddLast(vollist->Last());
			}
		}
		gGeoManager->GetTopVolume()->AddNode(lsVol,0,new TGeoCombiTrans(delx, dely, delz, new TGeoRotation("r",alpha,beta,gamma)));
		lsNum++;
	}
	
	
	
	alpha = 90;
	beta = 0;
	gamma = 0;
	delx = 0;
	dely = 0;
	delz = 48 + LShalfLen;// THIS IS A TOTAL GUESS
	
	lsNum = 0;
	int lsOffset = 80;
	{
		int lsMap[13] = {1,2,4,6,8,10,12,13,3,5,7,9,11};
		int cansInRing[3] = {1,6,6};
		int ringRadius[3] = {0,xSep,TMath::Sqrt(3.0)*xSep};
		double angOffset = TMath::Pi()/6.0;
		
		TGeoVolumeAssembly* lsVol = new TGeoVolumeAssembly("LS_Cake");
		TObjArray* vollist = lsVol->GetNodes();
		for(int j=0; j<3; j++){
			for(int i=0; i<cansInRing[j]; i++){
				float x = ringRadius[j]*TMath::Sin(2.0*TMath::Pi()/cansInRing[j]*i + (j==1 ? angOffset : 0 ));
				float y = ringRadius[j]*TMath::Cos(2.0*TMath::Pi()/cansInRing[j]*i + (j==1 ? angOffset : 0 ));
				
				TGeoVolume* LSgeo;
				sprintf(detName, "LS%02d", lsMap[lsNum]+lsOffset);
				TGeoMedium* medium = new TGeoMedium(detName, 0, new TGeoMaterial("Fake",1,1,1,1,1));
				LSgeo = gGeoManager->MakeTube(detName, medium, 0, LSrad, LShalfLen);
				LSgeo->SetLineColor(color);
				lsVol->AddNode(LSgeo,0,new TGeoCombiTrans(x,y,0, new TGeoRotation("r",0,0,0)));
				if(!vollist) vollist = lsVol->GetNodes();
				if(disp->GetNodes()) disp->GetNodes()->AddLast(vollist->Last());
				lsNum++;
			}
		}
		gGeoManager->GetTopVolume()->AddNode(lsVol,0,new TGeoCombiTrans(delx, dely, delz, new TGeoRotation("r",alpha,beta,gamma)));
	}
	
}