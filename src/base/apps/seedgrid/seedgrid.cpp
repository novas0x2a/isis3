#define GUIHELPERS

#include "Isis.h"
#include "ControlNet.h"
#include "ID.h"

using namespace std;
using namespace Isis;

void PrintMap ();

map <string,void*> GuiHelpers(){
  map <string,void*> helper;
  helper ["PrintMap"] = (void*) PrintMap;
  return helper;
}

void IsisMain() {

  // Get the map projection file provided by the user
  UserInterface &ui = Application::GetUserInterface();

  ControlNet cnet;

  double equatorialRadius = 0.0;

  string spacing = ui.GetString("SPACING");
  if (spacing == "METER") {
    Pvl userMap;                               
    userMap.Read(ui.GetFilename("PROJECTION"));
    PvlGroup &mapGroup = userMap.FindGroup("Mapping", Pvl::Traverse);
  
    // Construct a Projection for converting between Lon/Lat and X/Y
    // Note: Should this be an option to include this in the program?
    string target;
    if (ui.WasEntered("TARGET")) {
      target = ui.GetString("TARGET");
    }
    else if (mapGroup.HasKeyword("TargetName")) {
       target = mapGroup.FindKeyword("TargetName")[0];
       ui.PutAsString("TARGET", target);
    }
    else {
      string msg = "A target must be specified either by the [TARGET] parameter ";
      msg += "or included as a value for keyword [TargetName] in the projection file.";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }

    PvlGroup radii = Projection::TargetRadii(target);
  
    mapGroup.AddKeyword(PvlKeyword("TargetName", target), Pvl::Replace);
    mapGroup.AddKeyword(PvlKeyword("EquatorialRadius", (string) radii["EquatorialRadius"]));
    mapGroup.AddKeyword(PvlKeyword("PolarRadius", (string) radii["PolarRadius"]));

    if (!ui.WasEntered("PROJECTION")) {
      mapGroup.AddKeyword(PvlKeyword("LatitudeType","Planetocentric"));
      mapGroup.AddKeyword(PvlKeyword("LongitudeDirection","PositiveEast"));
      mapGroup.AddKeyword(PvlKeyword("LongitudeDomain",360));         
      mapGroup.AddKeyword(PvlKeyword("CenterLatitude",0));            
      mapGroup.AddKeyword(PvlKeyword("CenterLongitude",0));           
    }
  
    mapGroup.AddKeyword(PvlKeyword("MinimumLatitude", ui.GetDouble("MINLAT")),Pvl::Replace);
    mapGroup.AddKeyword(PvlKeyword("MaximumLatitude", ui.GetDouble("MAXLAT")),Pvl::Replace);
    mapGroup.AddKeyword(PvlKeyword("MinimumLongitude", ui.GetDouble("MINLON")),Pvl::Replace);
    mapGroup.AddKeyword(PvlKeyword("MaximumLongitude", ui.GetDouble("MAXLON")),Pvl::Replace);
  
    Projection *proj = Isis::ProjectionFactory::Create(userMap);
                         
    // Convert the Lat/Lon range to an X/Y range.
    double minX;
    double minY;
    double maxX;
    double maxY;
    bool foundRange = proj->XYRange(minX, maxX, minY, maxY);
    if (!foundRange) {
      string msg = "Cannot convert Lat/Long range to an X/Y range.";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }

    // Create the control net to store the points in.
    cnet.SetType (ControlNet::ImageToGround);
    cnet.SetTarget (target);
    string networkId = ui.GetString("NETWORKID");
    cnet.SetNetworkId(networkId);
    cnet.SetUserName(Isis::Application::UserName());
    cnet.SetDescription(ui.GetString("DESCRIPTION"));

    // Set up an automatic id generator for the point ids
    ID pointId = ID(ui.GetString("POINTID"));

    double xStepSize = ui.GetDouble("XSTEP");
    double yStepSize = ui.GetDouble("YSTEP");

    equatorialRadius = radii["EquatorialRadius"];

    double x = minX;
    double y = minY;
    while(x <= maxX) {
      while(y <= maxY) {
        proj->SetCoordinate(x,y);
        if (!proj->IsSky() && proj->Latitude() < ui.GetDouble("MAXLAT") && 
            proj->Longitude() < ui.GetDouble("MAXLON") &&
            proj->Latitude() > ui.GetDouble("MINLAT") &&
            proj->Longitude() > ui.GetDouble("MINLON")) {
          ControlPoint control;
          control.SetId (pointId.Next());
          control.SetIgnore (true);
          control.SetUniversalGround(proj->Latitude(), proj->Longitude(), equatorialRadius);
          cnet.Add(control);
        }
        y += yStepSize;
      }
      x += xStepSize;
      y = minY;
    }
  }
  else {
  
    if (!ui.WasEntered("TARGET")) {
      string msg = "A target must be specified by the [TARGET] parameter ";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }

    string target = ui.GetString("TARGET");
    PvlGroup radii = Projection::TargetRadii(target);
    equatorialRadius = radii["EquatorialRadius"];

    // Create the control net to store the points in.
    cnet.SetType (ControlNet::ImageToGround);
    cnet.SetTarget (target);
    string networkId = ui.GetString("NETWORKID");
    cnet.SetNetworkId(networkId);
    cnet.SetUserName(Isis::Application::UserName());
    cnet.SetDescription(ui.GetString("DESCRIPTION"));

    // Set up an automatic id generator for the point ids
    ID pointId = ID(ui.GetString("POINTID"));

    double lat = ui.GetDouble("MINLAT");
    double maxLat = ui.GetDouble("MAXLAT");
    double latStep = ui.GetDouble("LATSTEP");
  
    double lon = ui.GetDouble("MINLON");
    double maxLon = ui.GetDouble("MAXLON");
    double lonStep = ui.GetDouble("LONSTEP");
  
    while (lon < maxLon) {
      double minLat = lat;
      while (lat < maxLat) {
        ControlPoint control;
        control.SetId (pointId.Next());
        control.SetIgnore (true);
        control.SetUniversalGround(lat, lon, equatorialRadius);
        cnet.Add(control);
  
        lat += latStep;
      }
      lon += lonStep;
      lat = minLat;
    }
  }

  PvlGroup results ("Results");
  results += PvlKeyword("EquatorialRadius", equatorialRadius);
  results += PvlKeyword("NumberControlPoints", cnet.Size());
  Application::Log(results);

  cnet.Write(ui.GetFilename("TO"));
}

// Helper function to print out mapfile to session log
void PrintMap() {
  UserInterface &ui = Application::GetUserInterface();

  // Get mapping group from map file
  Pvl userMap;
  userMap.Read(ui.GetFilename("PROJECTION"));
  PvlGroup &userGrp = userMap.FindGroup("Mapping",Pvl::Traverse);

  //Write map file out to the log
  Isis::Application::GuiLog(userGrp);
}
