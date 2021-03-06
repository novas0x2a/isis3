#define GUIHELPERS

#include "Isis.h"

#include "Camera.h"
#include "ProjectionFactory.h"
#include "ProcessRubberSheet.h"

#include "iException.h"

using namespace std; 
using namespace Isis;

void PrintMap ();
void LoadMapRes ();
void LoadCameraRes ();
void LoadMapRange ();
void LoadCameraRange ();

map <string,void*> GuiHelpers(){
  map <string,void*> helper;
  helper ["PrintMap"] = (void*) PrintMap;
  helper ["LoadMapRes"] = (void*) LoadMapRes;
  helper ["LoadCameraRes"] = (void*) LoadCameraRes;
  helper ["LoadMapRange"] = (void*) LoadMapRange;
  helper ["LoadCameraRange"] = (void*) LoadCameraRange;
  return helper;
}

class sky2map : public Transform {
  private:
    Camera *p_incam;
    Projection *p_outmap;
    int p_inputSamples;
    int p_inputLines;
    bool p_trim;
    int p_outputSamples;
    int p_outputLines;

  public:
    // constructor
    sky2map (const int inputSamples, const int inputLines, Camera *incam, 
             const int outputSamples, const int outputLines, Projection *outmap,
             bool trim);
    
    // destructor
    ~sky2map () {};

    // Implementations for parent's pure virtual members
    bool Xform (double &inSample, double &inLine,
                    const double outSample, const double outLine);
    int OutputSamples () const;
    int OutputLines () const;
};

void BandChange (const int band);
Cube *icube;
Camera *incam;

void IsisMain() {
  // Get the camera model established from the input file.  We want to have
  // TargetName = Sky in the labels so make it happpen
  ProcessRubberSheet p;

  UserInterface &ui = Application::GetUserInterface();
  Pvl userMap;
  userMap.Read(ui.GetFilename("MAP"));
  PvlGroup &userGrp = userMap.FindGroup("Mapping",Pvl::Traverse);

  // Open the input cube, get the camera object, and the cam map projection
  // Note: The default target info is positive west, planetocentric, 360
  icube = p.SetInputCube ("FROM");
  incam = icube->Camera();

 // Pvl lab(ui.GetFilename("FROM"));
 // PvlGroup &inst = lab.FindGroup("Instrument",Pvl::Traverse);
  //inst["TargetName"] = "Sky";
  
  // Add the default mapping info to the user entered mapping group 
  userGrp.AddKeyword(PvlKeyword("TargetName","Sky"),Pvl::Replace);
  userGrp.AddKeyword(PvlKeyword("EquatorialRadius",1.0),Pvl::Replace);
  userGrp.AddKeyword(PvlKeyword("PolarRadius",1.0),Pvl::Replace);
  userGrp.AddKeyword(PvlKeyword("LatitudeType","Planetocentric"),Pvl::Replace);
  userGrp.AddKeyword(PvlKeyword("LongitudeDirection","PositiveWest"),Pvl::Replace);
  userGrp.AddKeyword(PvlKeyword("LongitudeDomain","360"),Pvl::Replace);
  if (userGrp.HasKeyword("PixelResolution")) {
    userGrp.DeleteKeyword("PixelResolution");
  }

  if (ui.GetString("DEFAULTRANGE") == "CAMERA") { 
    // Get the default ra/dec range
    double minRa,maxRa,minDec,maxDec;
    incam->RaDecRange(minRa,maxRa,minDec,maxDec);
    userGrp.AddKeyword(PvlKeyword("MinimumLongitude",minRa),Pvl::Replace);
    userGrp.AddKeyword(PvlKeyword("MaximumLongitude",maxRa),Pvl::Replace);
    userGrp.AddKeyword(PvlKeyword("MinimumLatitude",minDec),Pvl::Replace);
    userGrp.AddKeyword(PvlKeyword("MaximumLatitude",maxDec),Pvl::Replace);
  }
  if (ui.GetString("DEFAULTSCALE") == "CAMERA") {
    double res = incam->RaDecResolution();
    userGrp.AddKeyword(PvlKeyword("Scale",1.0/res),Pvl::Replace);
  }

  // Override computed range with the users request
  if (ui.WasEntered("SRA")) {
    userGrp.AddKeyword(PvlKeyword("MinimumLongitude",ui.GetDouble("SRA")),Pvl::Replace);  
  }
  if (ui.WasEntered("ERA")) { 
    userGrp.AddKeyword(PvlKeyword("MaximumLongitude",ui.GetDouble("ERA")),Pvl::Replace);  
  }
  if (ui.WasEntered("SDEC")){ 
    userGrp.AddKeyword(PvlKeyword("MinimumLatitude",ui.GetDouble("SDEC")),Pvl::Replace);
  }
  if (ui.WasEntered("EDEC")){ 
    userGrp.AddKeyword(PvlKeyword("MaximumLatitude",ui.GetDouble("EDEC")),Pvl::Replace);
  }

  // Get the resolution from the user if they decided to enter it
  if (ui.GetString("DEFAULTSCALE") == "USER") {
    userGrp.AddKeyword(PvlKeyword("Scale",ui.GetDouble("SCALE")),Pvl::Replace);  
  }
               
  // Create the projection
  int samples,lines;
  Projection *proj = ProjectionFactory::CreateForCube(userMap,samples,lines);

  // Output the mapping group used to the gui session log
  Application::GuiLog(userGrp);

  // Create the transform object which maps 
  //   output line/samp -> output lat/lon (dec/ra) -> input line/samp
  Transform *xform = new sky2map (icube->Samples(),icube->Lines(),incam,
                                  samples,lines,proj,
                                  ui.GetBoolean("Trim"));

  // Create the output cube and add the projection group
  Cube *ocube = p.SetOutputCube("TO",xform->OutputSamples(),
                                         xform->OutputLines(),
                                         icube->Bands());
  ocube->PutGroup(userGrp);

  // Set up the interpolator
  Interpolator *interp = NULL;
  if (ui.GetString("INTERP") == "NEARESTNEIGHBOR") {
    interp = new Interpolator(Interpolator::NearestNeighborType);
  }
  else if (ui.GetString("INTERP") == "BILINEAR") {
    interp = new Interpolator(Interpolator::BiLinearType);
  }
  else if (ui.GetString("INTERP") == "CUBICCONVOLUTION") {
    interp = new Interpolator(Interpolator::CubicConvolutionType);
  }

  // If we need to deal with band dependent camera models, register
  // our function to handle that
  if (!incam->IsBandIndependent()) {
    p.BandChange(BandChange);
  }

  //  See if center of input image projects.  If it does, force tile
  //  containing this center to be processed in ProcessRubberSheet.

  double centerSamp = icube->Samples () / 2.;
  double centerLine = icube->Lines () / 2.;
  incam->SetImage(centerSamp,centerLine);
  if (proj->SetGround(incam->Declination(),
                      incam->RightAscension())) {
    p.ForceTile (proj->WorldX(),proj->WorldY ());
  }

  // Create an alpha cube group for the output cube
  if (!ocube->HasGroup("AlphaCube")) {
    PvlGroup alpha("AlphaCube");
    alpha += PvlKeyword("AlphaSamples",icube->Samples());
    alpha += PvlKeyword("AlphaLines",icube->Lines());
    alpha += PvlKeyword("AlphaStartingSample",0.5);
    alpha += PvlKeyword("AlphaStartingLine",0.5);
    alpha += PvlKeyword("AlphaEndingSample",icube->Samples()+0.5);
    alpha += PvlKeyword("AlphaEndingLine",icube->Lines()+0.5);
    alpha += PvlKeyword("BetaSamples",icube->Samples());
    alpha += PvlKeyword("BetaLines",icube->Lines());
    ocube->PutGroup(alpha);
  }

  // Warp the cube
  p.StartProcess(*xform, *interp);
  p.EndProcess();

  // add mapping to print.prt
  PvlGroup mapping = proj->Mapping(); 
  Application::Log(mapping); 

  // Cleanup
  delete xform;
  delete interp;
}

void BandChange (const int band) {
  incam->SetBand(band);
}

sky2map::sky2map (const int inputSamples, const int inputLines,
                  Camera *incam, const int outputSamples,
                  const int outputLines, Projection *outmap,
                  bool trim) {
  p_inputSamples = inputSamples;
  p_inputLines = inputLines;
  p_incam = incam;

  p_outputSamples = outputSamples;
  p_outputLines = outputLines;
  p_outmap = outmap;

  p_trim = trim;
}

// Transform method mapping output line/samps to lat/lons to input line/samps
bool sky2map::Xform (double &inSample, double &inLine,
                         const double outSample, const double outLine) {
  // See if the output image coordinate converts to lat/lon
  if (!p_outmap->SetWorld(outSample,outLine)) return false;

  // See if we should trim
  if ((p_trim) && (p_outmap->HasGroundRange())) {
    if (p_outmap->Latitude() < p_outmap->MinimumLatitude()) return false;
    if (p_outmap->Latitude() > p_outmap->MaximumLatitude()) return false;
    if (p_outmap->Longitude() < p_outmap->MinimumLongitude()) return false;
    if (p_outmap->Longitude() > p_outmap->MaximumLongitude()) return false;
  }

  // Get the lat/lon and see if it can be converted to input line/samp
  double lat = p_outmap->Latitude();
  double lon = p_outmap->Longitude();

  if (!p_incam->SetRightAscensionDeclination(lon,lat)) return false;

  // Make sure the point is inside the input image
  if (p_incam->Sample() < 0.5) return false;
  if (p_incam->Line() < 0.5) return false;
  if (p_incam->Sample() > p_inputSamples + 0.5) return false;
  if (p_incam->Line() > p_inputLines + 0.5) return false;

  // Everything is good
  inSample = p_incam->Sample();
  inLine = p_incam->Line();
  return true;
}

int sky2map::OutputSamples () const {
  return p_outputSamples;
}

int sky2map::OutputLines () const {
  return p_outputLines;
}

// Helper function to print out mapfile to session log
void PrintMap() {
  UserInterface &ui = Application::GetUserInterface();

  // Get mapping group from map file
  Pvl userMap;
  userMap.Read(ui.GetFilename("MAP"));
  PvlGroup &userGrp = userMap.FindGroup("Mapping",Pvl::Traverse);

  //Write map file out to the log
  Isis::Application::GuiLog(userGrp);
}

// Helper function to get mapping resolution.
void LoadMapRes () {
  UserInterface &ui = Application::GetUserInterface();

  // Get mapping group from map file
  Pvl userMap;
  userMap.Read(ui.GetFilename("MAP"));
  PvlGroup &userGrp = userMap.FindGroup("Mapping",Pvl::Traverse);

  // Set resolution
  if (userGrp.HasKeyword("Scale")) {
    ui.Clear("SCALE");
    ui.PutDouble("SCALE",userGrp["Scale"]);
  }
  else {
    string msg = "Mapfile [" + ui.GetFilename("MAP") + 
                  "] does not have [SCALE] keyword to load";
    throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
  }
}

//Helper function to get camera resolution.
void LoadCameraRes () {
  UserInterface &ui = Application::GetUserInterface();

  // Open the input cube, get the camera object, and the cam map projection
  Cube c;
  c.Open(ui.GetFilename("FROM"));
  Camera *cam = c.Camera();
  double res = cam->RaDecResolution();

  ui.Clear("SCALE");
  ui.PutDouble("SCALE",1.0 / res);
}

//Helper function to get ground range from map file.
void LoadMapRange() {
  UserInterface &ui = Application::GetUserInterface();

  ui.Clear("SRA");
  ui.Clear("ERA");
  ui.Clear("SDEC");
  ui.Clear("EDEC");

  // Get map file
  Pvl userMap;
  userMap.Read(ui.GetFilename("MAP"));
  PvlGroup &userGrp = userMap.FindGroup("Mapping",Pvl::Traverse);

  // Set ground range keywords that are found in mapfile
  int count = 0;
  if (userGrp.HasKeyword("MinimumLongitude")) {
    ui.PutDouble("SRA",userGrp["MinimumLongitude"]);
    count++;
  }
  if (userGrp.HasKeyword("MaximumLongitude")) {
    ui.PutDouble("ERA",userGrp["MaximumLongitude"]);
    count++;
  }
  if (userGrp.HasKeyword("MinimumLatitude")) {
    ui.PutDouble("SDEC",userGrp["MinimumLatitude"]);
    count++;
  }
  if (userGrp.HasKeyword("MaximumLatitude")) {
    ui.PutDouble("EDEC",userGrp["MaximumLatitude"]);
    count++;
  }

  // Set default ground range param to map
  ui.Clear("DEFAULTRANGE");
  ui.PutAsString("DEFAULTRANGE","MAP");

  if (count < 4) {
    string msg = "One or more of the values for the sky range was not found";
    msg += " in [" + ui.GetFilename("MAP") + "]";
    throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
  }
}

//Helper function to load camera range.
void LoadCameraRange () {
  UserInterface &ui = Application::GetUserInterface();

  // Open the input cube, get the camera object, and the cam map projection
  Cube c;
  c.Open(ui.GetFilename("FROM"));
  Camera *cam = c.Camera();

  // Make the target info match the user mapfile
  double minra,maxra,mindec,maxdec;
  cam->RaDecRange(minra,maxra,mindec,maxdec);

  // Set ground range parameters in UI
  ui.Clear("SRA");
  ui.PutDouble("SRA",minra);
  ui.Clear("ERA");
  ui.PutDouble("ERA",maxra);
  ui.Clear("SDEC");
  ui.PutDouble("SDEC",mindec);
  ui.Clear("EDEC");
  ui.PutDouble("EDEC",maxdec);

  // Set default ground range param to camera
  ui.Clear("DEFAULTRANGE");
  ui.PutAsString("DEFAULTRANGE","CAMERA");
}





