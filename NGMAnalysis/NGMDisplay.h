#ifndef NGMDISPLAY
#define NGMDISPLAY

#include "TCanvas.h"
#include "TString.h"
#include "TObjArray.h"
#include "TVector3.h"

class TGeoNode;
class TObjArray;
class TGeoShape;
class TGeoMaterial;
class TGeoMedium;
class TH1;

class NGMDisplay {
 public:
  virtual ~NGMDisplay();
  static NGMDisplay* Instance(){if(!_display) _display = new NGMDisplay; return _display;}
  static TGeoNode* FindNode(TObjArray* NodeBranch, const char* NodeName);
  static const char* GetPathToNode(const char* NodeName);
  static int GetGlobalPosition(const char* DetectorName, TVector3& position);
  static bool HasInstance(){return _display ? true:false;}
  void AddLSAssembly(const double &delx, const double &dely, const double &delz, 
                     const double &alpha, const double &beta, const double &gamma, const int &color);
  void AddHeAssembly(const double &delx, const double &dely, const double &delz, 
                     const double &alpha, const double &beta, const double &gamma, const int &color);
  void AddGLAssembly(const double &delx, const double &dely, const double &delz, 
                      const double &alpha, const double &beta, const double &gamma, const int &color, const int* order=0);
  void AddGLAssemblyB(const double &delx, const double &dely, const double &delz, 
                      const double &alpha, const double &beta, const double &gamma, const int &color, const int* order=0);
  void AddGLAssemblyC(const double &delx, const double &dely, const double &delz, 
                      const double &alpha, const double &beta, const double &gamma, const int &color, const int* order=0);
  void AddGS(const double &delx, const double &dely, const double &delz, 
                     const double &alpha, const double &beta, const double &gamma, const int &color);
  void AddGL(const double &delx, const double &dely, const double &delz, 
                     const double &alpha, const double &beta, const double &gamma, const int &color);
  void AddGSAssembly(const double &delx, const double &dely, const double &delz, 
                      const double &alpha, const double &beta, const double &gamma, const int &color, const int* order=0);
  void AddGSAssemblyB(const double &delx, const double &dely, const double &delz, 
                      const double &alpha, const double &beta, const double &gamma, const int &color, const int* order=0);
  void AddMuPanel(const double &delx, const double &dely, const double &delz, 
                  const double &alpha, const double &beta, const double &gamma, const int &color, const int& number = 0);
  void AddProtoLS(const double &delx, const double &dely, const double &delz, 
                  const double &alpha, const double &beta, const double &gamma, const int &color, const int& number = -1);
  void AddProtoLSWall(const double &delx, const double &dely, const double &delz, 
                      const double &alpha, const double &beta, const double &gamma, const int &color, 
                      const int &Nx = 1,const int &Ny = 17,const int &Nz = 17,const int &offset = -1);
  void DrawHistogram(TH1* h1, const char* detType = "LS%04d", bool doReset = true);
  void AddHit(const char* detName, Color_t color = kRed, int trans = 0);
  bool GetDetectorOrigin(const char* detName, double& x, double& y, double& z);
  double GetDetectorSeparation(const char* detName1, const char* detName2);
  void AddFloor();
  void Reset();
  void Update();
  void SetTransparency(int trans = 50);
  void IsRaytracing(bool isRaytracing=true);
  void InitFromTextFile(const char* fileName);
  void MakeNTSGeom(float separation = 0);
  void MakeNTSGeomFromDB(long long runNumber, const char* passwd);
  void Animate();
  void HideCanvas(bool  hideCanvas = true){_hideCanvas = hideCanvas; if (_hideCanvas) delete GetCanvas();}
  TCanvas* GetCanvas();
  
  // Initialize Geometry using description specified in provided root macro filename
  // for runnumber. If runnumber is identical, skip initialization.
  void InitFromMacro(const char* fileName, Long64_t runnumber = 0);
  void SetInitMacroPath(const char* fileName) { _macroName = fileName; }
  const char* GetInitMacroPath() const { return _macroName.Data(); }
  TObjArray* GetNodes() { return _detNodeList; }
  TObjArray* GetPositions() { return _detPositions; }
  double getMaxRadialPathLength(TGeoShape* shape) const;
  
  enum detectortype {gammaLongA = 1, gammaLongB = 2, gammaShortA = 3, gammaShortB = 4, 
	liquidscint = 5, hewall = 6, muonBlock = 7, gammaSBlock = 8, gammaLBlock = 9, gammaLongC = 10};
  
 private:
  NGMDisplay();
  TCanvas* _c1;
  int lsNum;
  int heNum;
  int muNum;
  int glNum;
  int gsNum;
  int medNum;
  bool _isRaytracing;
  TString	fName;
  TObjArray* _detNodeList;
  TObjArray* _detPositions;
  bool _hideCanvas;
  Long64_t _runnumber; // Geometry initialized for runnumber
  TString _macroName;

  static NGMDisplay* _display;

 ClassDef(NGMDisplay,0)
};

#endif
