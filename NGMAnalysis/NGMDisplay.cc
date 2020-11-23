#include "NGMDisplay.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TROOT.h"
#include "TColor.h"
#include "TView.h"
#include "NGMGeomVals.h"
#include "TSQLServer.h"
#include "TSQLResult.h"
#include "TSQLRow.h"
#include "NGMLogger.h"
#include "NGMModule.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <string.h>
#include <cstdlib>
#include "TGeoNode.h"
#include "TObjArray.h"
#include "TGeoShape.h"
#include "TGeoTube.h"
#include "TGeoBBox.h"
#include "TClass.h"
#include "TH1.h"
#include "TStyle.h"

using namespace std;
using namespace NGMGeomVals;

NGMDisplay* NGMDisplay::_display = 0;

ClassImp(NGMDisplay)

TGeoNode* NGMDisplay::FindNode(TObjArray* NodeBranch, const char* NodeName)
{
	TString NodeToFind(NodeName);
	if(! NodeBranch) return 0;
	if(TString(NodeName)=="") return 0;
	TGeoNode* currNode = (TGeoNode*)(NodeBranch->Last());
	if(!currNode) return 0;
  
	if(NodeToFind==currNode->GetName()) return currNode;
	
	for(int inode = 0; inode < currNode->GetVolume()->GetNdaughters(); inode++)
	{
		NodeBranch->AddLast( currNode->GetVolume()->GetNode(inode));
		TGeoNode* result = FindNode(NodeBranch,NodeName);
		if(result) return result;
	}
	//Since neither this node nor any progeny was NodeName remove this from the branch
	NodeBranch->RemoveLast();
	
	return 0;
}

const char* NGMDisplay::GetPathToNode(const char* NodeName)
{
	
	TObjArray* nlist = new TObjArray;
	nlist->AddLast(gGeoManager->GetTopNode());
	
	TGeoNode* result = FindNode(nlist,NodeName);
	if(!result) return "";
	
	TString path;
	for(int inode = 0; inode < nlist->GetEntries(); inode++)
	{
		path+="/";
		path+= ((TGeoNode*)(nlist->At(inode)))->GetName();
	}
	return path.Data();
}

int NGMDisplay::GetGlobalPosition(const char* DetectorName, TVector3& position)
{
	// Cant do anything without a geometry object
	if(!gGeoManager) return 1;
	
	// Each node should have an _0 appended to the detectorname
	TString pathToNode(GetPathToNode(TString(DetectorName)+="_0"));
	if(pathToNode == "") return 1;
	// Cd to the node associated with the detector
	if(!gGeoManager->cd(pathToNode)) return 2;
	
	Double_t localposition[3] = {0.0,0.0,0.0};
	Double_t globalposition[3] = {0.0,0.0,0.0};
	gGeoManager->LocalToMaster(localposition, globalposition);
	position.SetXYZ(globalposition[0], globalposition[1], globalposition[2]);
	
	return 0;
}



//ALL OF THE HARDCODED DIMENSIONS OF THE DETECTORS NEED TO BE MOVED TO A HEADER OR SIMILAR.
void NGMDisplay::AddLSAssembly(const double &delx, const double &dely, const double &delz, 
                               const double &alpha, const double &beta, const double &gamma, const int &color){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	sprintf(detName, "LS_Assembly_%d", lsNum);
	TGeoVolumeAssembly* lsVol = new TGeoVolumeAssembly(detName);
	TObjArray* vollist = lsVol->GetNodes();
	TGeoVolume* LSgeo;
	for(int j=0; j<8; j++){
		for(int i=0; i<2; i++){
			sprintf(detName, "LS%02d", 16-(i+j*2)+16*lsNum);
      TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
			LSgeo = gGeoManager->MakeTube(detName, medium, 0, 5.08, 3.81);
			LSgeo->SetLineColor(color);
			lsVol->AddNode(LSgeo,0,new TGeoCombiTrans(i*12.7-12.7/2.0, 0,j*11.43+15, new TGeoRotation("r",0,90,0)));
			if(!vollist) vollist = lsVol->GetNodes();
			_detNodeList->AddLast(vollist->Last());
		}
	}
	gGeoManager->GetTopVolume()->AddNode(lsVol,0,new TGeoCombiTrans(delx, dely, delz, new TGeoRotation("r",alpha,beta,gamma)));
	lsNum++;
}

void NGMDisplay::AddHeAssembly(const double &delx, const double &dely, const double &delz, 
                               const double &alpha, const double &beta, const double &gamma, const int &color){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	sprintf(detName, "HETTL%02d", 1+heNum);
	TGeoVolumeAssembly* heVol = new TGeoVolumeAssembly(detName);
	TObjArray* vollist = heVol->GetNodes();
	TGeoVolume* HEgeo;
	for(int i =0;i<10;i++){
		sprintf(detName,"HETube%02d",i+10*heNum);
    TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
		HEgeo = gGeoManager->MakeTube(detName, medium, 0, 2.54, 50.8);
		HEgeo->SetLineColor(color);
		heVol->AddNode(HEgeo,0,new TGeoTranslation(0, i*2*2.6416-9.0*2.6416, 50.8 + 4.6));
		if(!vollist) vollist = heVol->GetNodes();
		_detNodeList->AddLast(vollist->Last());
	}
	gGeoManager->GetTopVolume()->AddNode(heVol,0,new TGeoCombiTrans(delx, dely, delz, new TGeoRotation("r",alpha,beta,gamma)));
	heNum++;
}

void NGMDisplay::AddGL(const double &delx, const double &dely, const double &delz, 
                       const double &alpha, const double &beta, const double &gamma, const int &color){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	sprintf(detName,"GL%02d",++glNum);
  TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
	TGeoVolume* GLgeo = gGeoManager->MakeBox(detName, medium, GLActiveWidth/2.0, GLActiveLength/2.0, GLActiveHeight/2.0);
	GLgeo->SetLineColor(color);
	gGeoManager->GetTopVolume()->AddNode(GLgeo,0,new TGeoCombiTrans(delx, dely, delz+GLHeight/2.0, new TGeoRotation("r",alpha,beta,gamma)));
	_detNodeList->AddLast(gGeoManager->GetTopVolume()->GetNodes()->Last());
}

void NGMDisplay::AddGS(const double &delx, const double &dely, const double &delz, 
                       const double &alpha, const double &beta, const double &gamma, const int &color){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	sprintf(detName,"GS%02d",++gsNum);
  TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
	TGeoVolume* GSgeo = gGeoManager->MakeBox(detName, medium, GSActiveWidth/2.0, GSActiveLength/2.0, GSActiveHeight/2.0);
	GSgeo->SetLineColor(color);
	gGeoManager->GetTopVolume()->AddNode(GSgeo,0,new TGeoCombiTrans(delx, dely, delz+GSHeight/2.0, new TGeoRotation("r",alpha,beta,gamma)));
	_detNodeList->AddLast(gGeoManager->GetTopVolume()->GetNodes()->Last());
}

void NGMDisplay::AddGLAssembly(const double &delx, const double &dely, const double &delz, 
                               const double &alpha, const double &beta, const double &gamma, const int &color, const int* order){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	TGeoVolumeAssembly* GLVol = new TGeoVolumeAssembly("GLAssembly");
	TObjArray* vollist = GLVol->GetNodes();
	TGeoVolume* GLgeo;
	for(int i =0;i<2;i++){
		for(int j =0;j<6;j++){
			if(order) {
				if (order[i*6+j]>0) glNum = order[i*6+j];
				else continue;
			} else {
				glNum++;	
			}
			sprintf(detName,"GL%02d",glNum);
      TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
			GLgeo = gGeoManager->MakeBox(detName, medium, GLActiveHeight/2.0, GLActiveLength/2.0, GLActiveWidth/2.0);
			GLgeo->SetLineColor(color);
			GLVol->AddNode(GLgeo,0, new TGeoTranslation(i*GLHeight, 0, j*GLWidth + GLWidth/2.0 + GLCartFloor));
			if(!vollist) vollist = GLVol->GetNodes();
			_detNodeList->AddLast(vollist->Last());
		}
	}
	gGeoManager->GetTopVolume()->AddNode(GLVol,0,new TGeoCombiTrans(delx, dely, delz, new TGeoRotation("r",alpha,beta,gamma)));	
}

void NGMDisplay::AddGLAssemblyB(const double &delx, const double &dely, const double &delz, 
                                const double &alpha, const double &beta, const double &gamma, const int &color, const int* order){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	TGeoVolumeAssembly* GLVol = new TGeoVolumeAssembly("GLAssembly");
	TObjArray* vollist = GLVol->GetNodes();
	TGeoVolume* GLgeo;
	for(int i =0;i<2;i++){
		for(int j =0;j<5;j++){
			if(order) {
				if (order[i*5+j]>0) glNum = order[i*5+j];
				else continue;
			} else {
				glNum++;	
			}
			sprintf(detName,"GL%02d",glNum);
      TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
			GLgeo = gGeoManager->MakeBox(detName, medium, GLActiveHeight/2.0, GLActiveLength/2.0, GLActiveWidth/2.0);
			GLgeo->SetLineColor(color);
			GLVol->AddNode(GLgeo,0, new TGeoTranslation(i*GLHeight, 0, j*GLWidth + GLWidth/2.0 + GLCartFloor));
			if(!vollist) vollist = GLVol->GetNodes();
			_detNodeList->AddLast(vollist->Last());
		}
	}
	gGeoManager->GetTopVolume()->AddNode(GLVol,0,new TGeoCombiTrans(delx, dely, delz, new TGeoRotation("r",alpha,beta,gamma)));
}

void NGMDisplay::AddGLAssemblyC(const double &delx, const double &dely, const double &delz, 
                                const double &alpha, const double &beta, const double &gamma, const int &color, const int* order){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	TGeoVolumeAssembly* GLVol = new TGeoVolumeAssembly("GLAssembly");
	TObjArray* vollist = GLVol->GetNodes();
	TGeoVolume* GLgeo;
	for(int i =0;i<11;i++){
		if(order) {
			if (order[i]>0) glNum = order[i];
			else continue;
		}else {
			glNum++;	
		}
		sprintf(detName,"GL%02d",glNum);
    TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
    GLgeo = gGeoManager->MakeBox(detName, medium, GLActiveWidth/2.0, GLActiveLength/2.0, GLActiveHeight/2.0);
		GLgeo->SetLineColor(color);
		GLVol->AddNode(GLgeo,0, new TGeoTranslation(0, 0, i*GLHeight + GLHeight/2.0 + GLCartFloor));
		if(!vollist) vollist = GLVol->GetNodes();
		_detNodeList->AddLast(vollist->Last());
	}
	gGeoManager->GetTopVolume()->AddNode(GLVol,0,new TGeoCombiTrans(delx, dely, delz, new TGeoRotation("r",alpha,beta,gamma)));
}

void NGMDisplay::AddGSAssembly(const double &delx, const double &dely, const double &delz, 
                               const double &alpha, const double &beta, const double &gamma, const int &color, const int* order){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	TGeoVolumeAssembly* GSVol = new TGeoVolumeAssembly("GSAssembly");
	TObjArray* vollist = GSVol->GetNodes();
	TGeoVolume* GSgeo;
	float _delz = 0;
	for(int i =0;i<12;i++){
		if(order) {
			if (order[i]>0) gsNum = order[i];
			else continue;
		} else {
			gsNum++;	
		}
		sprintf(detName,"GS%02d",gsNum);
    TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
		GSgeo = gGeoManager->MakeBox(detName, medium, GSActiveWidth/2.0, GSActiveHeight/2.0, GSActiveLength/2.0);
		GSgeo->SetLineColor(color);
		_delz = 0.0;
		if(i==0 || i==11) _delz = 15.0;
		GSVol->AddNode(GSgeo,0, new TGeoTranslation(0, i*GSHeight - GSHeight*5.5, GSLength/2.0 + GSCartFloor + _delz));
		if(!vollist) vollist = GSVol->GetNodes();
		_detNodeList->AddLast(vollist->Last());
	}
	gGeoManager->GetTopVolume()->AddNode(GSVol,0,new TGeoCombiTrans(delx, dely, delz, new TGeoRotation("r",alpha,beta,gamma)));
}

void NGMDisplay::AddGSAssemblyB(const double &delx, const double &dely, const double &delz, 
                                const double &alpha, const double &beta, const double &gamma, const int &color, const int* order){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	TGeoVolumeAssembly* GSVol = new TGeoVolumeAssembly("GSAssembly");
	TObjArray* vollist = GSVol->GetNodes();
	TGeoVolume* GSgeo;
	for(int i =0;i<10;i++){
		if(order) {
			if (order[i]>0) gsNum = order[i];
			else continue;
		} else {
			gsNum++;	
		}
		sprintf(detName,"GS%02d",gsNum);
    TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
		GSgeo = gGeoManager->MakeBox(detName, medium, GSActiveWidth/2.0, GSActiveHeight/2.0, GSActiveLength/2.0);
		GSgeo->SetLineColor(color);
		GSVol->AddNode(GSgeo,0, new TGeoTranslation(0, i*GSHeight - GSHeight*4.5, GSLength/2.0 + GSCartFloor));
		if(!vollist) vollist = GSVol->GetNodes();
		_detNodeList->AddLast(vollist->Last());
	}
	gGeoManager->GetTopVolume()->AddNode(GSVol,0,new TGeoCombiTrans(delx, dely, delz, new TGeoRotation("r",alpha,beta,gamma)));
}

void NGMDisplay::AddProtoLSWall(const double &delx, const double &dely, const double &delz, 
                                const double &alpha, const double &beta, const double &gamma, const int &color, 
                                const int &Nx,const int &Ny,const int &Nz,const int &offset){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	if(offset >= 0) lsNum = offset-1;
	char detName[128];
	sprintf(detName, "LSProto_Assembly_%d", lsNum);
	if(gGeoManager->GetVolume(detName)){	
		cout<<"Cowardly refusing replace exhisting volume "<<detName<<"."<<endl;
		return;
	}
	TGeoVolumeAssembly* LSVol = new TGeoVolumeAssembly(detName);
	TGeoVolume* LSgeo;
	TObjArray* vollist = LSVol->GetNodes();
	
	for(int ix=0;ix<Nx;ix++){
		for(int iz=0;iz<Nz;iz++){
			for(int iy=0;iy<Ny;iy++){
				sprintf(detName,"LS%04d",++lsNum);
        TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
				LSgeo = gGeoManager->MakeBox(detName, medium, protoScinActiveWidth/2.0, protoScinActiveHeight/2.0, protoScinActiveLength/2.0);
				LSgeo->SetLineColor(color);
				LSVol->AddNode(LSgeo,0, new TGeoTranslation(protoScinWidth * ix, protoScinLength *iy, protoScinHeight * iz));
				if(!vollist) vollist = LSVol->GetNodes();
				_detNodeList->AddLast(vollist->Last());
			}
		}
	}
	double Delx = delx - (Nx-1.0)*protoScinWidth/2.0;
	double Dely = dely - (Ny-1.0)*protoScinLength/2.0;
	double Delz = delz - (Nz-1.0)*protoScinHeight/2.0;
	
	gGeoManager->GetTopVolume()->AddNode(LSVol,0,new TGeoCombiTrans(Delx, Dely, Delz, new TGeoRotation("r",alpha,beta,gamma)));
}

void NGMDisplay::AddProtoLS(const double &delx, const double &dely, const double &delz, 
                            const double &alpha, const double &beta, const double &gamma, const int &color, const int& number){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	if(number >= 0) lsNum = number-1;
	sprintf(detName,"LS%04d",++lsNum);
	if(gGeoManager->GetVolume(detName)){	
		cout<<"Cowardly refusing replace exhisting volume "<<detName<<"."<<endl;
		return;
	}
  TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
	TGeoVolume* LSgeo = gGeoManager->MakeBox(detName, medium, protoScinActiveWidth/2.0, protoScinActiveHeight/2.0, protoScinActiveLength/2.0);
	LSgeo->SetLineColor(color);
	gGeoManager->GetTopVolume()->AddNode(LSgeo,0,new TGeoCombiTrans(delx, dely, delz, new TGeoRotation("r",alpha,beta,gamma)));
	_detNodeList->AddLast(gGeoManager->GetTopVolume()->GetNodes()->Last());
}

void NGMDisplay::AddMuPanel(const double &delx, const double &dely, const double &delz, 
                            const double &alpha, const double &beta, const double &gamma, const int &color, const int& number){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	if(!_detNodeList) _detNodeList = new TObjArray;
	char detName[128];
	sprintf(detName,"MU%02d", number);
	if(number>0){
		sprintf(detName,"MU%02d", number);
		if(gGeoManager->GetVolume(detName)){	
			cout<<"Cowardly refusing replace exhisting volume "<<detName<<"."<<endl;
			return;
		}
		muNum = number;
	} else {
		sprintf(detName,"MU%02d", ++muNum);
		while(gGeoManager->GetVolume(detName)) sprintf(detName,"MU%02d", ++muNum);
	}
  TGeoMedium* medium = new TGeoMedium(detName, medNum++, new TGeoMaterial("Polystyrene",13.01f,7,1.032f,42.4f,81.9f));
	TGeoVolume* MBgeo = gGeoManager->MakeBox(detName, medium, MUActiveWidth/2.0, MUActiveLength/2.0, MUActiveHeight/2.0);
	MBgeo->SetLineColor(color);
	gGeoManager->GetTopVolume()->AddNode(MBgeo, 0, new TGeoCombiTrans(delx, dely, MUHeight/2.0 + delz, new TGeoRotation("r",alpha,beta,gamma)));
	_detNodeList->AddLast(gGeoManager->GetTopVolume()->GetNodes()->Last());
}

void NGMDisplay::AddFloor(){
	if (!gGeoManager) return;
	if(gGeoManager->IsClosed()) return;
	float tileSize = 20;
	int Ntiles = 20;
	int tileCount = 0;
	//  TGeoVolume* floorVol = gGeoManager->MakeBox("floor",0,Ntiles*tileSize/2, Ntiles*tileSize/2, 1);
	TGeoVolumeAssembly* floorVol = new TGeoVolumeAssembly("floor");
	TGeoVolume* tileVol = 0;
	for(int i=0; i<Ntiles; i++){
		for(int j=0; j<Ntiles; j++){
			tileCount++;
			tileVol = gGeoManager->MakeBox("floorTile",0,tileSize/2.0, tileSize/2.0, 1);
			if((j+i)%2 == 0) tileVol->SetLineColor(kWhite);
			else tileVol->SetLineColor(kBlack);//TColor::GetColor(10,10,10));
			floorVol->AddNode(tileVol,tileCount,new TGeoTranslation(tileSize*(0.5+j)-Ntiles*tileSize/2,tileSize*(0.5+i)-Ntiles*tileSize/2,-1));
		}
	}
	//gGeoManager->GetTopVolume()->AddNode(floorVol,0,new TGeoCombiTrans(Ntiles*tileSize/2,Ntiles*tileSize/2,-20,new TGeoRotation("r",45,0,0)));
	gGeoManager->GetTopVolume()->AddNode(floorVol,0,new TGeoTranslation(Ntiles*tileSize/2,Ntiles*tileSize/2,0));
}

void NGMDisplay::AddHit(const char* detName, Color_t color, int trans){
	//  if(i>0 && gGeoManager->GetListOfUVolumes()->At(i-1)) ((TGeoVolume*) gGeoManager->GetListOfUVolumes()->At(i-1))->SetLineColor(kBlack);
	if (!gGeoManager) return;
	TGeoVolume* vol = gGeoManager->FindVolumeFast(detName);
	if(!vol){
		cout<<"Volume "<<detName<<" not found."<<endl;
	} else {
		if(strstr(detName,"HETTL")){
			for(int i=0;i<vol->GetNdaughters();i++){
				vol->GetNode(i)->GetVolume()->SetLineColor(color);
        vol->GetNode(i)->GetVolume()->SetTransparency(trans);
			} 
		}else {
			vol->SetLineColor(color);
      vol->SetTransparency(trans);
		}
	}
}

void NGMDisplay::DrawHistogram(TH1* h1, const char* detType, bool doReset){
  if(!h1) return;
  if (!gGeoManager) return;
  if (doReset) Reset();
  int ncolors = gStyle->GetNumberOfColors();
  int ndivz   = TMath::Abs(h1->GetContour());
  if(ndivz > ncolors || ndivz == 0) ndivz = ncolors;
  double zMax = h1->GetMaximum();
  double zMin = h1->GetMinimum();
  TAxis axis(ndivz, zMin, zMax);
  TAxis coloraxis(ndivz, 0, ncolors);
  int nbins = h1->GetNbinsX()*h1->GetNbinsY()*h1->GetNbinsZ();
  char detName [512];
  for(int idet = 1; idet <=  nbins; idet++){
    int theColor = -1;
    sprintf(detName, detType, idet);
    if(h1->GetBinContent(idet) == 0.0 && zMin >= 0.0) continue;
    int theBin;
    if(h1->GetBinContent(idet) >= zMax)
      theBin = ndivz;
    else if(h1->GetBinContent(idet) < zMin)
      theBin = 1;
    else
      theBin = axis.FindBin(h1->GetBinContent(idet));
    theColor = (int)(0.999*coloraxis.GetBinUpEdge(theBin));

    AddHit(detName, gStyle->GetColorPalette(theColor));
  }
  Update();
}

bool NGMDisplay::GetDetectorOrigin(const char* detName, double& x, double& y, double& z){
	if (!gGeoManager) return false;
	char currPath[512];
	char newPath[512];
	sprintf(currPath, "%s", gGeoManager->GetPath());
	sprintf(newPath, "/TOP_1/%s_0", detName);
	if(!gGeoManager->CheckPath(newPath)){
		//I only search 1 level deep
		TObjArray* myNodes = gGeoManager->GetTopVolume()->GetNodes();
		for(int i=0; i<myNodes->GetEntries(); i++){
			TGeoNode* mynode = (TGeoNode*)gGeoManager->GetTopVolume()->GetNodes()->At(i);
			if(mynode->GetNdaughters() > 0){
				sprintf(newPath, "/TOP_1/%s/%s_0", mynode->GetName(), detName);
				if(gGeoManager->CheckPath(newPath)) break;
			}
			// sprintf(newPath, "");
		}
	}
	//cout<<"SEARCHING PATH "<<newPath<<endl;
	double local[] = {0,0,0};
	double master[] = {-999,-999,-999};
	if (gGeoManager->CheckPath(newPath)){
		gGeoManager->cd(newPath);
		gGeoManager->LocalToMaster(local,master);
		gGeoManager->cd(currPath);
	} else {
		cout<<"DETECTOR "<<detName<<" NOT FOUND!"<<endl;
		return false;
	}
	x = master[0];
	y = master[1];
	z = master[2];
	return true;
}

double NGMDisplay::GetDetectorSeparation(const char* detName1, const char* detName2){
	double x1=0;
	double x2=0;
	double y1=0;
	double y2=0;
	double z1=0;
	double z2=0;
	if(GetDetectorOrigin(detName1, x1, y1, z1) &&  GetDetectorOrigin(detName2, x2, y2, z2)) return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2));
	return -999;
}

void NGMDisplay::Reset(){
	if (!gGeoManager) return;
	for(int i=0;i<gGeoManager->GetListOfUVolumes()->GetLast()+1;i++){
		if(gGeoManager->GetListOfUVolumes()->At(i)) ((TGeoVolume*) gGeoManager->GetListOfUVolumes()->At(i))->SetLineColor(TColor::GetColor(127,127,127));
	}
	Update();	
}

void NGMDisplay::SetTransparency(int trans){
	if (!gGeoManager) return;
	for(int i=0;i<gGeoManager->GetListOfUVolumes()->GetLast()+1;i++){
		if(gGeoManager->GetListOfUVolumes()->At(i)) ((TGeoVolume*) gGeoManager->GetListOfUVolumes()->At(i))->SetTransparency(trans);
	}
}

void NGMDisplay::Update(){
	if (!gGeoManager) return;
	_c1 = GetCanvas();
	if(!_c1) return;
	_c1->cd();
	//	if(gGeoManager->GetTopVolume()->IsRaytracing()) gGeoManager->GetTopVolume()->Raytrace();
	if(!gGeoManager->GetTopVolume()->IsRaytracing()) gGeoManager->GetTopVolume()->Draw();
	_c1->Modified();
	_c1->Update();
	//	_c1->Draw();
}

void NGMDisplay::IsRaytracing(bool isRaytracing){
	if (!gGeoManager) return;
	if(!GetCanvas()) return;
	GetCanvas()->cd();
	gGeoManager->GetTopVolume()->Draw();
	gGeoManager->GetTopVolume()->Raytrace(isRaytracing);
}

void NGMDisplay::Animate(){
	if (!gGeoManager) return;
	IsRaytracing(false);
	TCanvas* temp_can = GetCanvas();
	if(!temp_can) return;
	temp_can->cd();
	Update();
	temp_can->GetView()->FrontView();
	TView* view = temp_can->GetView();
	int irep=0;
	for(int i=0;i<360; i++) {
		view->SetView(view->GetLongitude(),view->GetLatitude(),view->GetPsi()+1,irep);
		temp_can->Modified();
		temp_can->Update();
	}
	for(int i=0;i<360; i++) {
		view->SetView(view->GetLongitude(),view->GetLatitude()+1,view->GetPsi(),irep);
		temp_can->Modified();
		temp_can->Update();
	}
	for(int i=0;i<360; i++) {
		view->SetView(view->GetLongitude()+1,view->GetLatitude(),view->GetPsi(),irep);
		temp_can->Modified();
		temp_can->Update();
	}
}


void NGMDisplay::MakeNTSGeomFromDB(long long runNumber, const char* passwd){
	// Connect to database
	TSQLServer* db = NGMModule::GetDBConnection();
	if(!db)
	{
		LOG<<"Unable to connect to run database"<<ENDM_WARN;
		return;
	}
	
	//First test if this runnumber is already in database
	TString sqlStatement("SELECT standoff_GL, standoff_GL FROM filelog WHERE runnumber=\"");
	sqlStatement+=runNumber;
	sqlStatement+="\"";
	std::cout<<sqlStatement.Data()<<std::endl;
	
	TSQLResult* res = db->Query(sqlStatement);
	if(res && res->GetRowCount()==1){
		TSQLRow* myrow = res->Next();
		MakeNTSGeom(atof(myrow->GetField(0)) + atof(myrow->GetField(1)));
		delete myrow;
	} else {
		LOG<<"Run database returned ambiguous result."<<ENDM_WARN;
	}
	db->Close();
	delete db;
	delete res;
}


void NGMDisplay::MakeNTSGeom(float separation){
	float centerX = 190.0;
	float centerY = 100.0;
	float centerZ = 0.0;
	
	if (gGeoManager) delete gGeoManager;
	new TGeoManager("myGeoManager", "ngmDisplay");
	_c1 = GetCanvas();
	if(_c1){
		_c1->cd();	
		_c1->Clear();
	}
	lsNum = 0;
	heNum = 0;
	muNum = 0;
	glNum = 0;
	gsNum = 0;
  medNum = 2;
	
	TGeoMaterial *mat = new TGeoMaterial("VOID");
	TGeoMedium *med = new TGeoMedium("MED",1,mat);
	TGeoVolume *top = gGeoManager->MakeBox("TOP",med,1000,1000,1000);
	gGeoManager->SetTopVolume(top);
	
	AddGSAssemblyB(centerX + separation/2.0 + HEAssemHeight + GSWidth/2.0, centerY, centerZ, 0, 0, 0, kBlue);
	AddHeAssembly(centerX + separation/2.0 + HEAssemHeight/2.0, centerY - HEAssemWidth/2.0, centerZ + GSCartFloor, 0, 0, 0, kGreen);
	AddHeAssembly(centerX + separation/2.0 + HEAssemHeight/2.0, centerY + HEAssemWidth/2.0, centerZ + GSCartFloor, 0, 0, 0, kGreen);
	AddGLAssemblyC(centerX - separation/2.0 - HEAssemHeight - GLWidth/2.0, centerY, centerZ, 180, 0, 0, kRed);
	AddHeAssembly(centerX - separation/2.0 - HEAssemHeight/2.0, centerY + HEAssemWidth/2.0, centerZ + GLCartFloor, 0, 0, 0, kGreen);
	AddHeAssembly(centerX - separation/2.0 - HEAssemHeight/2.0, centerY - HEAssemWidth/2.0, centerZ + GLCartFloor, 0, 0, 0, kGreen);
	
	AddMuPanel(centerX - MULength, centerY + MUWidth, centerZ, 90, 0, 0, kMagenta, 13);
	AddMuPanel(centerX - MULength, centerY, centerZ, 90, 0, 0, kMagenta, 12);
	AddMuPanel(centerX - MULength, centerY - MUWidth, centerZ, 90, 0, 0, kMagenta, 18);
	AddMuPanel(centerX, centerY + MUWidth/2.0, centerZ, 90, 0, 0, kMagenta, 14);
	AddMuPanel(centerX, centerY - MUWidth/2.0, centerZ, 90, 0, 0, kMagenta, 8);
	AddMuPanel(centerX + MULength, centerY + MUWidth, centerZ, 90, 0, 0, kMagenta, 3);
	AddMuPanel(centerX + MULength, centerY, centerZ, 90, 0, 0, kMagenta, 16);
	AddMuPanel(centerX + MULength, centerY - MUWidth, centerZ, 90, 0, 0, kMagenta, 7);
	gGeoManager->SetNsegments(10);
	gGeoManager->CloseGeometry();
	Update();	
}

void NGMDisplay::InitFromMacro(const char* fileName, Long64_t runnumber){
	// Skipping for identical configuration specified by runnumber
	if(runnumber != 0 && _runnumber == runnumber)
	{
		return;
	}
	_runnumber = runnumber;
	
	if(TString(fileName)!="")
	{
		_macroName = fileName;
	}
	
	LOG<<"Initializing geometry from "<<_macroName.Data()<<ENDM_INFO;
	
	if (gGeoManager) delete gGeoManager;
	if(_detNodeList) _detNodeList->Clear();
	if(_detPositions) _detNodeList->Delete();
	new TGeoManager("myGeoManager", "ngmDisplay");
	_c1 = GetCanvas();
	if(_c1){
		_c1->cd();	
		_c1->Clear();
	}
	lsNum = 0;
	heNum = 0;
	muNum = 0;
	glNum = 0;
	gsNum = 0;
	medNum = 2;
	
	TGeoMaterial *mat = new TGeoMaterial("VOID");
	TGeoMedium *med = new TGeoMedium("MED",1,mat);
	TGeoVolume *top = gGeoManager->MakeBox("TOP",med,1000,1000,1000);
	gGeoManager->SetTopVolume(top);
	
	int errCode = 0;
	gROOT->Macro(_macroName.Data(), &errCode);
	if(errCode>0){
		LOG<<"Could not process macro "<<_macroName.Data()<<"."<<ENDM_FATAL;
		//MakeNTSGeom(120);	
		return;
	}
	gGeoManager->SetNsegments(10);
	gGeoManager->CloseGeometry();
	Update();
	
	// Loop over nodes and save global positions
	if(!_detPositions) _detPositions = new TObjArray;
	_detPositions->Delete();
	for(int idet = 0; idet < _detNodeList->GetEntries(); idet++)
	{
		TVector3* pos = new TVector3;
		TGeoNode* tdet = (TGeoNode*)(_detNodeList->At(idet));
		
		GetGlobalPosition(tdet->GetVolume()->GetName(),*pos);
		_detPositions->AddLast(pos);
	}
	
}

void NGMDisplay::InitFromTextFile(const char* fileName){
	ifstream geoFile(fileName);
	if(!geoFile.is_open()){
		cout<<"Could not open "<<fileName<<". Did not update geometry."<<endl;
		return;
	}
	
	if (gGeoManager) delete gGeoManager;
	if(_detNodeList) _detNodeList->Clear();
	if(_detPositions) _detPositions->Delete();
	new TGeoManager("myGeoManager", "ngmDisplay");
	_c1 = GetCanvas();
	if(_c1){
		_c1->cd();	
		_c1->Clear();
	}
	lsNum = 0;
	heNum = 0;
	muNum = 0;
	glNum = 0;
	gsNum = 0;
  medNum = 2;
	int detType = 0;
	float x = 0;
	float y = 0;
	float z = 0;
	float alpha = 0;
	float beta = 0;
	float gamma = 0;
	int color = kBlack;
	
	
	TGeoMaterial *mat = new TGeoMaterial("VOID");
	TGeoMedium *med = new TGeoMedium("MED",1,mat);
	TGeoVolume *top = gGeoManager->MakeBox("TOP",med,800,800,800);
	gGeoManager->SetTopVolume(top);
	
	//AddFloor();
	
	while(geoFile>>detType>>x>>y>>z>>alpha>>beta>>gamma>>color){
		//cout<<detType<<" "<<x<<" "<<y<<" "<<z<<" "<<color<<endl;
		if(detType == gammaLongA){
			AddGLAssembly(x,y,z,alpha,beta,gamma,color);
		} else if(detType == gammaLongB){
			AddGLAssemblyB(x,y,z,alpha,beta,gamma,color);
		} else if(detType == gammaLongC){
			AddGLAssemblyC(x,y,z,alpha,beta,gamma,color);
		} else if(detType == gammaShortA){
			AddGSAssembly(x,y,z,alpha,beta,gamma,color);
		} else if(detType == gammaShortB){
			AddGSAssemblyB(x,y,z,alpha,beta,gamma,color);
		} else if(detType == liquidscint){
			AddLSAssembly(x,y,z,alpha,beta,gamma,color);
		} else if(detType == hewall){
			AddHeAssembly(x,y,z,alpha,beta,gamma,color);
		} else if(detType == muonBlock){
			AddMuPanel(x,y,z,alpha,beta,gamma,color);
		} else if(detType == gammaSBlock){
			AddGS(x,y,z,alpha,beta,gamma,color);
		} else if(detType == gammaLBlock){	
			AddGL(x,y,z,alpha,beta,gamma,color);
		} else if(detType == 100){
			AddFloor();
		} else {
			cout<<"ERROR: Unknown detector type"<<endl;
		}
	}
	
	gGeoManager->SetNsegments(10);
	gGeoManager->CloseGeometry();
	//IsRaytracing(false);
	Update();
	//gPad->GetView()->FrontView();
	//IsRaytracing(true);
}

TCanvas* NGMDisplay::GetCanvas(){
	if (_c1 && _c1 == gROOT->FindObject(fName.Data())) return _c1;
	if ( _hideCanvas ) return 0;
	_c1 = new TCanvas(fName.Data(),fName.Data());
	_c1->SetFillColor(41);
	return _c1;
}

NGMDisplay::NGMDisplay(){
	lsNum = 0;
	heNum = 0;
	muNum = 0;
	glNum = 0;
	gsNum = 0;
  medNum = 2;
	_detNodeList = 0;
	_detPositions = 0;
	_isRaytracing = false;
	_runnumber = -1;
  _hideCanvas = false;
	//TFile f("out.root","RECREATE");
	//gROOT->SetStyle("Plain");
	//gSystem->Load("libGeom");
	
	//if (gGeoManager){
	//  cout<<"gGeoManager exhists! Not continuing"<<endl;
	//  return;
	// }
	fName = "NGMDisplayCanvas";
	_c1 = 0;
	_c1 = GetCanvas();
	if(_c1)_c1->cd();
	
	if (gGeoManager) delete gGeoManager;
	
	new TGeoManager("myGeoManager", "ngmDisplay");
	TGeoMaterial *mat = new TGeoMaterial("VOID");
	TGeoMedium *med = new TGeoMedium("MED",1,mat);
	TGeoVolume *top = gGeoManager->MakeBox("TOP",med,800,800,800);
	gGeoManager->SetTopVolume(top);
	
	//AddFloor();
	
	AddGSAssembly(100,200,0,0,0,0,kBlue);
	AddHeAssembly(125,230,GSCartFloor,0,0,0,kGreen);
	AddHeAssembly(125,170,GSCartFloor,0,0,0,kGreen);
	
	AddGLAssembly(250,200,0,0,0,0,kRed);
	AddHeAssembly(230,230,GLCartFloor,0,0,0,kGreen);
	AddHeAssembly(230,170,GLCartFloor,0,0,0,kGreen);
	
	AddMuPanel(30,200,0,0,0,0,kMagenta);
	AddMuPanel(80,200,0,0,0,0,kMagenta);
	AddMuPanel(130,200,0,0,0,0,kMagenta);
	AddMuPanel(230,200,0,0,0,0,kMagenta);
	AddMuPanel(280,200,0,0,0,0,kMagenta);
	AddMuPanel(330,200,0,0,0,0,kMagenta);
	
	AddMuPanel(150,200,110,0,0,0,kMagenta);
	AddMuPanel(200,200,110,0,0,0,kMagenta);
	
	AddLSAssembly(150,150,0,0,0,0,kYellow);
	AddLSAssembly(200,150,0,0,0,0,kYellow);
	AddLSAssembly(150,250,0,0,0,0,kYellow);
	AddLSAssembly(200,250,0,0,0,0,kYellow);
	
	gGeoManager->SetNsegments(10);
  SetTransparency();
	gGeoManager->CloseGeometry();
	
	//top->FindNode("GSAssembly_0")->GetMatrix()->SetDx(50);
	//Apparently if you move things around, you must recalculate voxels.
	//top->Voxelize(0);
	
	//Apparently you can add volumes after closing. Not sure if this could cause issues?
	//AddMuPanel(30,200,150,45,0,0,kMagenta);
	//top->Voxelize(0);
	
	//top->Raytrace();
	//top->Draw();
	Update();
	
	//gPad->GetView()->FrontView();
	//c1->Update();
	
	//f.Add(gGeoManager);
	//f.WriteTObject(gGeoManager,"myGeoManager");
	/* 
	 for(int i=0;i<gGeoManager->GetListOfUVolumes()->GetLast()+1;i++){
	 if(gGeoManager->GetListOfUVolumes()->At(i)) ((TGeoVolume*) gGeoManager->GetListOfUVolumes()->At(i))->SetLineColor(kBlack);
	 }
	 top->Raytrace();
	 for(int i=0;i<gGeoManager->GetListOfUVolumes()->GetLast()+1;i++){
	 if(!gGeoManager->GetListOfUVolumes()->At(i)) continue;
	 ((TGeoVolume*) gGeoManager->GetListOfUVolumes()->At(i))->SetLineColor(kBlue);
	 if(i>0 && gGeoManager->GetListOfUVolumes()->At(i-1)) ((TGeoVolume*) gGeoManager->GetListOfUVolumes()->At(i-1))->SetLineColor(kBlack);
	 top->Raytrace();
	 c1->Update();
	 gSystem->ProcessEvents();
	 }
	 */
	
}

double NGMDisplay::getMaxRadialPathLength(TGeoShape* shape) const
{
	static TClass* geoTubeClass = gROOT->GetClass("TGeoTube");
	static TClass* geoBBoxClass = gROOT->GetClass("TGeoBBox");
	if(!shape)
	{
		LOG<<"Attempt to call with null pointer "<<ENDM_WARN;
		return 0.0;
	}
	
	if(shape->IsA() == geoTubeClass )
	{
		TGeoTube* ts = (TGeoTube*)shape;
		return sqrt(ts->GetDz()*ts->GetDz() + ts->GetRmax()*ts->GetRmax());
	}else if(shape->IsA() == geoBBoxClass){
		TGeoBBox* ts = (TGeoBBox*)shape;
		return sqrt(ts->GetDZ()*ts->GetDZ() + ts->GetDY()*ts->GetDY() + ts->GetDX()*ts->GetDX());    
	}else{
		LOG<<"Shape not implemented for "<<shape->IsA()->GetName()<<ENDM_WARN;
	}
	return 0.0;
}


NGMDisplay::~NGMDisplay(){
	delete GetCanvas(); 
	_display = 0;
	delete gGeoManager;
	gGeoManager = 0;
	delete _detNodeList;
	_detNodeList = 0;
	if(_detPositions) _detPositions->Delete();
	delete _detPositions;
}
