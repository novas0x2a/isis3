#include <cmath>

#include "Isis.h"
#include "FileList.h"
#include "Cube.h"
#include "Process.h"
#include "Camera.h"
#include "Pvl.h"
#include "Statistics.h"

using namespace std; 
using namespace Isis;

 template <typename T> inline T MIN(const T &A, const T &B) {
   if ( A < B ) { return (A); }
   else         { return (B); }
 }

 template <typename T> inline T MAX(const T &A, const T &B) {
   if ( A > B ) { return (A); }
   else         { return (B); }
 }

inline double SetFloor(double value, const int precision) {
    double scale = pow(10.0, precision);
    value = floor(value * scale) / scale;
    return (value);
}

inline double SetRound(double value, const int precision) {
    double scale = pow(10.0, precision);
    value = rint(value * scale) / scale;
    return (value);
}

inline double SetCeil(double value, const int precision) {
    double scale = pow(10.0, precision);
    value = ceil(value * scale) / scale;
    return (value);
}

void IsisMain() {
  Process p;

  // Get the list of names of input CCD cubes to stitch together
  FileList flist;
  UserInterface &ui = Application::GetUserInterface();
  flist.Read(ui.GetFilename("FROMLIST"));
  if (flist.size() < 1) {
    string msg = "The list file[" + ui.GetFilename("FROMLIST") +
    " does not contain any filenames";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  string projection("Equirectangular");
  if(ui.WasEntered("MAP")) {
      Pvl mapfile(ui.GetFilename("MAP"));
      projection = (string) mapfile.FindGroup("Mapping")["ProjectionName"];
  }

  if(ui.WasEntered("PROJECTION")) {
      projection = ui.GetString("PROJECTION");
  }

  int digits = ui.GetInteger("PRECISION");

  Progress prog;
  prog.SetMaximumSteps(flist.size());
  prog.CheckStatus();

  Statistics scaleStat;
  Statistics longitudeStat;
  Statistics latitudeStat;
  PvlObject fileset("FileSet");

  for (unsigned int i = 0 ; i < flist.size() ; i++) {
    // Set the input image, get the camera model, and a basic mapping
    // group
    Cube cube;
    cube.Open(flist[i]);

    int lines = cube.Lines();
    int samples = cube.Samples();


    PvlObject fmap("File");
    fmap += PvlKeyword("Name",flist[i]);
    fmap += PvlKeyword("Lines", lines);
    fmap += PvlKeyword("Samples", samples);

    Camera *cam = cube.Camera();
    Pvl mapping;
    cam->BasicMapping(mapping);
    PvlGroup &mapgrp = mapping.FindGroup("Mapping");
    mapgrp.AddKeyword(PvlKeyword("ProjectionName",projection),Pvl::Replace);    

    // Get the radii
    double radii[3];
    cam->Radii(radii);
    PvlGroup target("Target");
    target += PvlKeyword("TargetName",cam->Target());
    target += PvlKeyword("RadiusA",radii[0]*1000.0,"meters");
    target += PvlKeyword("RadiusB",radii[1]*1000.0,"meters");
    target += PvlKeyword("RadiusC",radii[2]*1000.0,"meters");
 //   mapgrp.AddGroup(target);

    // Get resolution
    PvlGroup res("PixelResolution");
    double lowres = cam->LowestImageResolution();
    double hires = cam->HighestImageResolution();
    scaleStat.AddData(&lowres, 1);
    scaleStat.AddData(&hires, 1);

    mapgrp.AddKeyword(PvlKeyword("PixelResolution",(lowres+hires)/2.0),
                       Pvl::Replace);
    mapgrp += PvlKeyword("MinPixelResolution",lowres,"meters");
    mapgrp += PvlKeyword("MaxPixelResolution",hires,"meters");

    // Get the universal ground range
    double minlat,maxlat,minlon,maxlon;
    cam->GroundRange(minlat,maxlat,minlon,maxlon,mapping);
    mapgrp.AddKeyword(PvlKeyword("MinimumLatitude",minlat),Pvl::Replace);
    mapgrp.AddKeyword(PvlKeyword("MaximumLatitude",maxlat),Pvl::Replace);
    mapgrp.AddKeyword(PvlKeyword("MinimumLongitude",minlon),Pvl::Replace);
    mapgrp.AddKeyword(PvlKeyword("MaximumLongitude",maxlon),Pvl::Replace);

    fmap.AddGroup(res);
    fmap.AddGroup(mapgrp);
    fileset.AddObject(fmap);

    longitudeStat.AddData(&minlon, 1);
    longitudeStat.AddData(&maxlon, 1);
    latitudeStat.AddData(&minlat, 1);
    latitudeStat.AddData(&maxlat, 1);

    p.ClearInputCubes();
    prog.CheckStatus();
  }

//  Construct the output mapping group with statistics
  PvlGroup mapping("Mapping");
  double avgScale((scaleStat.Minimum()+scaleStat.Maximum())/2.0);
  double avgLat((latitudeStat.Minimum()+latitudeStat.Maximum())/2.0);
  double avgLon((longitudeStat.Minimum()+longitudeStat.Maximum())/2.0);

  mapping += PvlKeyword("ProjectionName",projection);
  mapping += PvlKeyword("PixelResolution", SetRound(avgScale, digits), "meters");
  mapping += PvlKeyword("MinPixelResolution",scaleStat.Minimum(),"meters");
  mapping += PvlKeyword("MaxPixelResolution",scaleStat.Maximum(),"meters");
  mapping += PvlKeyword("CenterLongitude", SetRound(avgLon,digits));
  mapping += PvlKeyword("CenterLatitude",  SetRound(avgLat,digits));
  mapping += PvlKeyword("MinimumLatitude", MAX(SetFloor(latitudeStat.Minimum(),digits), -90.0));
  mapping += PvlKeyword("MaximumLatitude", MIN(SetCeil(latitudeStat.Maximum(),digits), 90.0));
  mapping += PvlKeyword("MinimumLongitude",MAX(SetFloor(longitudeStat.Minimum(),digits), -180.0));
  mapping += PvlKeyword("MaximumLongitude",MIN(SetCeil(longitudeStat.Maximum(),digits), 360.0));

  PvlKeyword clat("PreciseCenterLongitude", avgLon);
  clat.AddComment("Actual Parameters without precision applied");
  mapping += clat;
  mapping += PvlKeyword("PreciseCenterLatitude",  avgLat);
  mapping += PvlKeyword("PreciseMinimumLatitude", latitudeStat.Minimum());
  mapping += PvlKeyword("PreciseMaximumLatitude", latitudeStat.Maximum());
  mapping += PvlKeyword("PreciseMinimumLongitude",longitudeStat.Minimum());
  mapping += PvlKeyword("PreciseMaximumLongitude",longitudeStat.Maximum());

  
  Application::GuiLog(mapping);

  // Write the output file if requested
  if (ui.WasEntered("TO")) {
    Pvl temp;
    temp.AddGroup(mapping);
    temp.Write(ui.GetFilename("TO","map"));
  }

  if (ui.WasEntered("LOG")) {
    Pvl temp;
    temp.AddObject(fileset);
    temp.Write(ui.GetFilename("LOG","log"));
  }

  p.EndProcess();
}
