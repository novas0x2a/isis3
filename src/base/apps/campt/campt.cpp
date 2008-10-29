#include "Isis.h"

#include <string>
#include <iomanip> 

#include "Camera.h"
#include "Process.h"
#include "iException.h"
#include "SpecialPixel.h"
#include "Brick.h"
#include "iTime.h"

using namespace std; 
using namespace Isis;

void IsisMain() {
  // Use a regular Process
  Process p;

  // Open the input cube and initialize the camera
  Cube *icube = p.SetInputCube("FROM");
  Camera *cam = icube->Camera();

  // See if the user entered a line/sample
  UserInterface &ui = Application::GetUserInterface();
  if (ui.GetString("TYPE") == "IMAGE") {
    double sample = 0.0;

    if(ui.WasEntered("SAMPLE")) {
      sample = ui.GetDouble("SAMPLE");
    }
    else {
      sample = icube->Samples()/2.0;
    }

    double line = 0.0;

    if(ui.WasEntered("LINE")) {
      line = ui.GetDouble("LINE");
    }
    else {
      line = icube->Lines()/2.0;
    }

    cam->SetImage(sample,line);
  }
  else {
    double lat = ui.GetDouble("LATITUDE");
    double lon = ui.GetDouble("LONGITUDE");
    cam->SetUniversalGround(lat,lon);
  }

  // See if we can proceed 
  if (!cam->HasSurfaceIntersection()) {
    string msg = "Requested position does not project in camera model";
    throw iException::Message(iException::Camera,msg,_FILEINFO_);
  }

  // Create Brick on samp, line to get the dn value of the pixel
  Brick b(3,3,1,icube->PixelType());
  int intSamp = (int)(cam->Sample()+0.5);
  int intLine = (int)(cam->Line()+0.5);
  b.SetBasePosition(intSamp,intLine,1);
  icube->Read(b);

  // Declare arrays of body coords for later use
  double pB[3], spB[3], sB[3];
  string utc;
  double ssplat,ssplon,sslat,sslon, pwlon, oglat;

  // Create group with ground position
  PvlGroup gp("GroundPoint"); 
  {
    gp += PvlKeyword("Filename",Filename(ui.GetFilename("FROM")).Expanded());
    gp += PvlKeyword("Sample",cam->Sample());
    gp += PvlKeyword("Line",cam->Line());
    gp += PvlKeyword("PixelValue",PixelToString(b[0]));
    gp += PvlKeyword("RightAscension",cam->RightAscension());
    gp += PvlKeyword("Declination",cam->Declination());
    gp += PvlKeyword("PlanetocentricLatitude",cam->UniversalLatitude());

    // Convert lat to planetographic
    double radii[3];
    cam->Radii(radii);
    oglat = Isis::Projection::ToPlanetographic(cam->UniversalLatitude(),
                                               radii[0],radii[2]);
    gp += PvlKeyword("PlanetographicLatitude",oglat);
    gp += PvlKeyword("PositiveEastLongitude",cam->UniversalLongitude());

    //Convert lon to positive west
    pwlon = -cam->UniversalLongitude();
    while (pwlon < 0.0) pwlon += 360.0;
    gp += PvlKeyword("PositiveWestLongitude",pwlon);

    cam->Coordinate(pB);
    PvlKeyword coord("BodyFixedCoordinate");
    coord.AddValue(pB[0],"km");
    coord.AddValue(pB[1],"km");
    coord.AddValue(pB[2],"km");
    gp += coord;

    gp += PvlKeyword("LocalRadius",cam->LocalRadius(),"m");
    gp += PvlKeyword("SampleResolution",cam->SampleResolution(),"m");
    gp += PvlKeyword("LineResolution",cam->LineResolution(),"m");

    cam->InstrumentPosition(spB);
    PvlKeyword spcoord("SpacecraftPosition");
    spcoord.AddValue(spB[0],"km");
    spcoord.AddValue(spB[1],"km");
    spcoord.AddValue(spB[2],"km");
    spcoord.AddComment("Spacecraft Information");
    gp += spcoord;

    gp += PvlKeyword("SpacecraftAzimuth",cam->SpacecraftAzimuth());
    gp += PvlKeyword("SlantDistance",cam->SlantDistance(),"km");
    gp += PvlKeyword("TargetCenterDistance",cam->TargetCenterDistance(),"km");
    cam->SubSpacecraftPoint(ssplat,ssplon);
    gp += PvlKeyword("SubSpacecraftLatitude",ssplat);
    gp += PvlKeyword("SubSpacecraftLongitude",ssplon);
    gp += PvlKeyword("SpacecraftAltitude",cam->SpacecraftAltitude(),"km");
    gp += PvlKeyword("OffNadirAngle",cam->OffNadirAngle());

    cam->SunPosition(sB);
    PvlKeyword scoord("SunPosition");
    scoord.AddValue(sB[0],"km");
    scoord.AddValue(sB[1],"km");
    scoord.AddValue(sB[2],"km");
    scoord.AddComment("Sun Information");
    gp += scoord;

    gp += PvlKeyword("SubSolarAzimuth",cam->SunAzimuth());
    gp += PvlKeyword("SolarDistance",cam->SolarDistance(),"AU");
    cam->SubSolarPoint(sslat,sslon);
    gp += PvlKeyword("SubSolarLatitude",sslat);
    gp += PvlKeyword("SubSolarLongitude",sslon);

    PvlKeyword phase("Phase",cam->PhaseAngle());
    phase.AddComment("Illumination and Other");
    gp += phase;
    gp += PvlKeyword("Incidence",cam->IncidenceAngle());
    gp += PvlKeyword("Emission",cam->EmissionAngle());
    gp += PvlKeyword("NorthAzimuth",cam->NorthAzimuth());

    PvlKeyword et("EphemerisTime",cam->EphemerisTime(),"seconds");
    et.AddComment("Time");
    gp += et;    
    iTime t(cam->EphemerisTime());
    utc = t.UTC();
    gp += PvlKeyword("UTC",utc);
    gp += PvlKeyword("LocalSolarTime",cam->LocalSolarTime(),"hour");
    gp += PvlKeyword("SolarLongitude",cam->SolarLongitude());
  }
  Application::Log(gp);

  if (ui.WasEntered("TO")) {
    // Get user params from ui
    string outFile = Filename(ui.GetFilename("TO")).Expanded();
    bool exists = Filename(outFile).Exists();
    bool append = ui.GetBoolean("APPEND");

    // Write the pvl group out to the file
    if (ui.GetString("FORMAT") == "PVL") {
      Pvl temp;
      temp.SetTerminator("");
      temp.AddGroup(gp);
      if (append) {
        temp.Append(outFile);
      }
      else {
        temp.Write(outFile);
      }
    } 
  
    // Create a flatfile from PVL data
    // The flatfile is comma delimited and can be imported into Excel
    else {
      ofstream os;
      bool writeHeader = false;
      if (append) {
        os.open(outFile.c_str(),ios::app);
        if (!exists) {
          writeHeader = true;
        }
      }
      else {
        os.open(outFile.c_str(),ios::out);
        writeHeader = true;
      } 

      if(writeHeader) {
        for(int i = 0; i < gp.Keywords(); i++) {
          if(gp[i].Size() == 3) {
            os << gp[i].Name() << "X," 
               << gp[i].Name() << "Y," 
               << gp[i].Name() << "Z";
          }
          else {
            os << gp[i].Name();
          }

          if(i < gp.Keywords()-1) {
            os << ",";
          }
        }
        os << endl;
      }
      
      for(int i = 0; i < gp.Keywords(); i++) {
        if(gp[i].Size() == 3) {
          os << (string)gp[i][0] << "," 
             << (string)gp[i][1] << "," 
             << (string)gp[i][2];
        }
        else {
          os << (string)gp[i];
        }
        
        if(i < gp.Keywords()-1) {
          os << ",";
        }
      }
      os << endl;
    }
  }
  else {
    if (ui.GetString("FORMAT") == "FLAT") {
      string msg = "Flat file must have a name.";
      throw Isis::iException::Message(Isis::iException::User, msg, _FILEINFO_ );
    }
  }
}
