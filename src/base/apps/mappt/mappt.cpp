#include "Isis.h"

#include <string>
#include <cmath>

#include "Process.h"
#include "iException.h"
#include "Projection.h"
#include "SpecialPixel.h"
#include "Brick.h"

using namespace std; 
using namespace Isis;

void IsisMain() {
  // Use a regular Process
  Process p;

  // Open the input cube and initialize the projection
  Cube *icube = p.SetInputCube("FROM");
  Projection *proj = icube->Projection();

  // Get the coordinate
  UserInterface &ui = Application::GetUserInterface();

  // Get the sample/line position if we have an image point
  if (ui.GetString("TYPE") == "IMAGE") {
    double samp = ui.GetDouble("SAMPLE");
    double line = ui.GetDouble("LINE");
    proj->SetWorld(samp,line);
  }

  // Get the lat/lon position if we have a ground point
  else if (ui.GetString("TYPE") == "GROUND") {
    double lat = ui.GetDouble("LATITUDE");
    double lon = ui.GetDouble("LONGITUDE");

    // Make sure we have a valid latitude value
    if (fabs(lat) > 90.0) {
      string msg = "Invalid value for [LATITUDE] outside range of ";
      msg += "[-90,90]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    proj->SetGround(lat,lon);
  }

  // Get the x/y position if we have a projection point
  else {
    double x = ui.GetDouble("X");
    double y = ui.GetDouble("Y");
    proj->SetCoordinate(x,y);
  }

  // Create Brick on samp, line to get the dn value of the pixel
  Brick b(1,1,1,icube->PixelType());
  int intSamp = (int)(proj->WorldX()+0.5);
  int intLine = (int)(proj->WorldY()+0.5);
  b.SetBasePosition(intSamp,intLine,1);
  icube->Read(b);

  // Log the position
  if (proj->IsGood()) {
    PvlGroup results("Results");
    results += PvlKeyword("Filename",Filename(ui.GetFilename("FROM")).Expanded());
    results += PvlKeyword("Sample",proj->WorldX());
    results += PvlKeyword("Line",proj->WorldY());
    results += PvlKeyword("PixelValue",PixelToString(b[0]));
    results += PvlKeyword("X",proj->XCoord());
    results += PvlKeyword("Y",proj->YCoord());
    results += PvlKeyword("Longitude",proj->Longitude());
    results += PvlKeyword("Latitude",proj->Latitude());
    results += PvlKeyword("LatitudeType",proj->LatitudeTypeString());
    results += PvlKeyword("LongitudeDirection",proj->LongitudeDirectionString());
    Application::Log(results);

    // Write an output label file if necessary
    if (ui.WasEntered("TO")) {
      // Get user params from ui
      string outFile = Filename(ui.GetFilename("TO")).Expanded();
      bool exists = Filename(outFile).Exists();
      bool append = ui.GetBoolean("APPEND");

      // Write the pvl group out to the file
      if (ui.GetString("FORMAT") == "PVL") {
        Pvl temp;
        temp.AddGroup(results);
        if (append) {
          temp.Append(outFile);
        }
        else {
          temp.Write(outFile);
        }
      }

      // Create a flatfile of the same data
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
          for(int i = 0; i < results.Keywords(); i++) {
            os << results[i].Name();

            if(i < results.Keywords()-1) {
              os << ",";
            }
          }
          os << endl;
        }

        for(int i = 0; i < results.Keywords(); i++) {
          os << (string)results[i];

          if(i < results.Keywords()-1) {
            os << ",";
          }
        }
        os << endl;
      }
    }
    else if(ui.GetString("FORMAT") == "FLAT") {
      string msg = "Flat file must have a name.";
      throw Isis::iException::Message(Isis::iException::User, msg, _FILEINFO_);
    }
  }
  else {
    string msg = "Could not project requested position";
    throw iException::Message(iException::Projection,msg,_FILEINFO_);
  }
}
