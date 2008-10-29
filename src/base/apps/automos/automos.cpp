#define GUIHELPERS

#include "Isis.h"
#include "ProcessMapMosaic.h"
#include "ProcessByLine.h"
#include "FileList.h"
#include "iException.h"
#include "SpecialPixel.h"
#include "ProjectionFactory.h"

using namespace std; 
using namespace Isis;

void calcRange(double &minLat, double &maxLat,
               double &minLon, double &maxLon);
void helperButtonCalcRange ();

map <string,void*> GuiHelpers(){
  map <string,void*> helper;
  helper ["helperButtonCalcRange"] = (void*) helperButtonCalcRange;
  return helper;
}
void IsisMain() {
  // Get the list of cubes to mosaic
  Process p;
  FileList list;
  UserInterface &ui = Application::GetUserInterface();
  list.Read(ui.GetFilename("FROMLIST"));
  if (list.size() < 1) {
    string msg = "The list file [" + ui.GetFilename("FROMLIST") +
                 "does not contain any data";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  ProcessMapMosaic m;
  CubeAttributeOutput &oAtt = ui.GetOutputAttribute("MOSAIC");
  if (ui.GetString("GRANGE") == "USER") {
    m.SetOutputCube(list, 
                    ui.GetDouble("MINLAT"), ui.GetDouble("MAXLAT"),
                    ui.GetDouble("MINLON"), ui.GetDouble("MAXLON"),
                    oAtt, ui.GetFilename("MOSAIC"));
  }
  else {
    m.SetOutputCube(list, oAtt, ui.GetFilename("MOSAIC"));
  }

  // Set up the mosaic priority, either the input cubes will be
  // placed ontop of each other in the mosaic or beneath each other
  ui = Application::GetUserInterface();
  MosaicPriority priority;
  if (ui.GetString("PRIORITY") == "BENEATH") {
    priority = mosaic;
  }
  else {
    priority = input;
  }

  // Loop for each input file and place it in the output mosaic
  PvlGroup outsiders("Outside");
  m.SetBandBinMatch(ui.GetBoolean("MATCHBANDBIN"));

  CubeAttributeInput inAtt;

  for (unsigned int i=0; i<list.size(); i++) {
    if(!m.StartProcess(list[i], priority)) {
      outsiders += PvlKeyword("File", list[i]); 
    }
  }

  m.EndProcess();
  Application::Log(outsiders); 
}

// Function to calculate the ground range from multiple inputs (list of images)
void calcRange(double &minLat, double &maxLat, 
               double &minLon, double &maxLon) {
  UserInterface &ui = Application::GetUserInterface();
  FileList list(ui.GetFilename("FROMLIST"));
  minLat = DBL_MAX;
  maxLat = -DBL_MAX;
  minLon = DBL_MAX;
  maxLon = -DBL_MAX;
  // We will loop through each input cube and do some 
  // computations needed for mosaicking
  int nbands = 0;
  Projection *firstProj = NULL;

  for (unsigned int i=0; i<list.size(); i++) {
    // Open the cube and get the maximum number of band in all cubes
    Cube cube;
    cube.Open(list[i]);
    if (cube.Bands() > nbands) nbands = cube.Bands();

    // See if the cube has a projection and make sure it matches
    // previous input cubes
    Projection *proj = Isis::ProjectionFactory::CreateFromCube(*(cube.Label()));
    if (firstProj == NULL) {
      firstProj = proj;
    }
    else if (*proj != *firstProj) {
      string msg = "Mapping groups do not match between cubes [" + 
                   list[0] + "] and [" + list[i] + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    if (proj->HasGroundRange()) {
      if (proj->MinimumLatitude() < minLat) minLat = proj->MinimumLatitude();
      if (proj->MaximumLatitude() > maxLat) maxLat = proj->MaximumLatitude();
      if (proj->MinimumLongitude() < minLon) minLon = proj->MinimumLongitude();
      if (proj->MaximumLongitude() > maxLon) maxLon = proj->MaximumLongitude();
    }

    // Cleanup
    cube.Close();
    if (proj != firstProj) delete proj;
  }
}

//Helper function to run calcRange function.
void helperButtonCalcRange () {
  UserInterface &ui = Application::GetUserInterface();
  double minLat;
  double maxLat;
  double minLon;
  double maxLon;

  // Run the function calcRange of calculate range info
  calcRange(minLat,maxLat,minLon,maxLon);

  // Write ranges to the GUI
  string use = "USER";
  ui.Clear("GRANGE");
  ui.PutAsString("GRANGE",use);
  ui.Clear("MINLAT");
  ui.PutDouble("MINLAT",minLat);
  ui.Clear("MAXLAT");
  ui.PutDouble("MAXLAT",maxLat);
  ui.Clear("MINLON");
  ui.PutDouble("MINLON",minLon);
  ui.Clear("MAXLON");
  ui.PutDouble("MAXLON",maxLon);
}
