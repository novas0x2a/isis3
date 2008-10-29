#include "Isis.h"
#include "ProcessMosaic.h"
#include "ProcessByLine.h"
#include "SpecialPixel.h"

using namespace std; 
using namespace Isis;

void CreateMosaic (Buffer &buf);

void IsisMain() {
  // See if we need to create the output file
  ProcessByLine bl;
  UserInterface &ui = Application::GetUserInterface();
  if (ui.GetString("CREATE") == "YES") {
    int ns = ui.GetInteger("NSAMPLES");
    int nl = ui.GetInteger("NLINES");
    int nb = ui.GetInteger("NBANDS");
    bl.Progress()->SetText("Initializing base mosaic");
    bl.SetInputCube("FROM");
    bl.SetOutputCube("MOSAIC",ns,nl,nb);
    bl.ClearInputCubes();
    bl.StartProcess(CreateMosaic);
    bl.EndProcess();
  }

  // Set the input cube for the process object
  ProcessMosaic p;
  p.SetBandBinMatch(ui.GetBoolean("MATCHBANDBIN"));
  p.Progress()->SetText("Mosaicking");
  p.SetInputCube ("FROM");

  // Set up the mosaic priority, either the input cube will be
  // placed ontop of the mosaic or beneath it 
  ui = Application::GetUserInterface();
  MosaicPriority priority;
  if (ui.GetString("INPUT") == "BENEATH") {
    priority = mosaic;
  }
  else {
    priority = input;
  }

  // Get the starting line/sample/band to place the input cube
  int outSample = ui.GetInteger ("OUTSAMPLE") - ui.GetInteger ("INSAMPLE") + 1;
  int outLine   = ui.GetInteger ("OUTLINE") - ui.GetInteger ("INLINE") + 1;
  int outBand   = ui.GetInteger ("OUTBAND") - ui.GetInteger ("INBAND") + 1;

  // Set the output mosaic
  Cube* to = p.SetOutputCube ("MOSAIC");
  p.WriteHistory(*to);

  // Place the input in the mosaic
  p.StartProcess(outSample, outLine, outBand, priority);
  p.EndProcess();
}

void CreateMosaic (Buffer &buf) {
  for (int i=0; i<buf.size(); i++) {
    buf[i] = NULL8;
  }
}
