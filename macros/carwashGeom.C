using namespace NGMGeomVals;

void carwashGeom(){
	const double lanewidth = 381;//cm
	
	NGMDisplay* disp = NGMDisplay::Instance();
	std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl
		 <<"This geometry is meant to describe background test "<<endl
		 <<"using volunteer automobiles in Nov 17-20, 2008"<<endl
		 <<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
	
	
	int orderGL[11] = {2,8,7,5,3,4,6,1,11,9,-12};
	disp->AddGLAssemblyC(-lanewidth/2.0 - GLWidth/2.0 - HEAssemHeight ,0,0,0,0,0,kRed,orderGL);
	int orderGS[10] = {11,5,10,6,2,1,7,9,8,12};
	disp->AddGSAssemblyB(lanewidth/2.0 + GSWidth/2.0 + HEAssemHeight,0,0,0,0,0,kBlue,orderGS);
	
	disp->AddHeAssembly(-lanewidth/2.0 - HEAssemHeight/2.0,-HEAssemWidth/2.0,GLCartFloor,0,0,0,kGreen);
	disp->AddHeAssembly(lanewidth/2.0 + HEAssemHeight/2.0,-HEAssemWidth/2.0,GSCartFloor,0,0,0,kGreen);
	disp->AddHeAssembly(lanewidth/2.0 + HEAssemHeight/2.0,HEAssemWidth/2.0,GSCartFloor,0,0,0,kGreen);
	disp->AddHeAssembly(-lanewidth/2.0 - HEAssemHeight/2.0,HEAssemWidth/2.0,GLCartFloor,0,0,0,kGreen);
	
	double LScartY = - GSHeight*5.0 - LSAssemWidth/2.0 - 44.45;
	disp->AddLSAssembly(lanewidth/2.0 + LSAssemWidth/2.0 - HEAssemHeight, LScartY-2.0*LSAssemHeight,GSCartFloor,270,0,0,kYellow);
	disp->AddLSAssembly(lanewidth/2.0 + LSAssemWidth/2.0 - HEAssemHeight, LScartY-LSAssemHeight,GSCartFloor,270,0,0,kYellow);
	disp->AddLSAssembly(lanewidth/2.0 + LSAssemWidth/2.0 - HEAssemHeight, LScartY,GSCartFloor,270,0,0,kYellow);

	LScartY = - GLLength/2.0 - LSAssemWidth/2.0 - 38.1;
	disp->AddLSAssembly(-lanewidth/2.0 - LSAssemWidth/2.0,LScartY-LSAssemHeight,GLCartFloor,90,0,0,kYellow);
	disp->AddLSAssembly(-lanewidth/2.0 - LSAssemWidth/2.0,LScartY-2.0*LSAssemHeight,GLCartFloor,90,0,0,kYellow);
        disp->AddLSAssembly(-lanewidth/2.0 - LSAssemWidth/2.0,LScartY,GLCartFloor,90,0,0,kYellow);
	
	const double MUoffset = (GSHeight*5.0 - MULength/2.0) + 62.23;
	const double MUbreak = 27.94;
	disp->AddMuPanel(MUWidth/2.0,MUoffset,0,0,0,0,kMagenta,13);
	disp->AddMuPanel(MUWidth/2.0,MUoffset-MULength,0,0,0,0,kMagenta,16);
	disp->AddMuPanel(MUWidth/2.0,MUoffset-2.0*MULength,0,0,0,0,kMagenta,2);
	disp->AddMuPanel(MUWidth/2.0,MUoffset-3.0*MULength-MUbreak,0,0,0,0,kMagenta,3);
	disp->AddMuPanel(-MUWidth/2.0,MUoffset,0,0,0,0,kMagenta,8);
	disp->AddMuPanel(-MUWidth/2.0,MUoffset-MULength,0,0,0,0,kMagenta,1);
	disp->AddMuPanel(-MUWidth/2.0,MUoffset-2.0*MULength,0,0,0,0,kMagenta,7);
	disp->AddMuPanel(-MUWidth/2.0,MUoffset-3.0*MULength-MUbreak,0,0,0,0,kMagenta,9);


}
