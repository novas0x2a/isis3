#include "Isis.h"
#include "ProcessMapMosaic.h"
#include "PvlGroup.h"

using namespace std; 
using namespace Isis;

void IsisMain() {
  // Get the user interface
  UserInterface &ui = Application::GetUserInterface();

  // Get the input cube to add to the mosaic and the projection information
  ProcessMapMosaic m;
  m.SetBandBinMatch(ui.GetBoolean("MATCHBANDBIN"));

  // Get the output projection set up properly
  if (ui.GetBoolean("CREATE")) {
    Cube inCube;
    inCube.Open(ui.GetFilename("FROM"));

    // Use the input projection as a starting point for the mosaic
    PvlGroup mapGroup = inCube.Label()->FindGroup("Mapping", Pvl::Traverse);
    inCube.Close();

    mapGroup.AddKeyword(PvlKeyword("MinimumLatitude",
                                       ui.GetDouble("MINLAT")),
                                       Pvl::Replace);
    mapGroup.AddKeyword(PvlKeyword("MaximumLatitude",
                                       ui.GetDouble("MAXLAT")),
                                       Pvl::Replace);
    mapGroup.AddKeyword(PvlKeyword("MinimumLongitude",
                                       ui.GetDouble("MINLON")),
                                       Pvl::Replace);
    mapGroup.AddKeyword(PvlKeyword("MaximumLongitude",
                                       ui.GetDouble("MAXLON")),
                                       Pvl::Replace);

    CubeAttributeOutput oAtt = ui.GetOutputAttribute("MOSAIC");
    m.SetOutputCube(ui.GetFilename("FROM"), mapGroup, oAtt, ui.GetFilename("MOSAIC"));
  }
  else {
    m.SetOutputCube(ui.GetFilename("MOSAIC"));
  }

  // Set up the mosaic priority, either the input cubes will be
  // placed ontop of each other in the mosaic or beneath each other
  MosaicPriority priority;
  if (ui.GetString("PRIORITY") == "BENEATH") {
    priority = mosaic;
  }
  else {
    priority = input;
  }

  PvlGroup outsiders("Outside");

  // Logs the cube if it falls outside of the given mosaic
  if(!m.StartProcess(ui.GetFilename("FROM"), priority)) {
    outsiders += PvlKeyword("File", ui.GetFilename("FROM"));
  }
  m.EndProcess();

  Application::Log(outsiders);
}
