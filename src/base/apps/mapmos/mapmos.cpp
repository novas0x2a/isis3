#include "Isis.h"
#include "ProcessMapMosaic.h"
#include "ProcessByLine.h"
#include "FileList.h"
#include "iException.h"

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
                                       ui.GetDouble("SLAT")),
                                       Pvl::Replace);
    mapGroup.AddKeyword(PvlKeyword("MaximumLatitude",
                                       ui.GetDouble("ELAT")),
                                       Pvl::Replace);
    mapGroup.AddKeyword(PvlKeyword("MinimumLongitude",
                                       ui.GetDouble("SLON")),
                                       Pvl::Replace);
    mapGroup.AddKeyword(PvlKeyword("MaximumLongitude",
                                       ui.GetDouble("ELON")),
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

  m.StartProcess(ui.GetFilename("FROM"), priority);
  m.EndProcess();
}
