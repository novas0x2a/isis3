#include "Isis.h"
#include "ProcessRubberSheet.h"
#include "enlarge.h"
#include "iException.h"
#include "AlphaCube.h"

using namespace std; 
using namespace Isis;

Cube cube;

void IsisMain() {
  ProcessRubberSheet p;

  // Open the input cube
  Cube *icube = p.SetInputCube ("FROM");

  // Set up the transform object
  UserInterface &ui = Application::GetUserInterface();
  double lmag = ui.GetDouble("LMAG");
  double smag = ui.GetDouble("SMAG");
  Transform *transform = new Enlarge(icube->Samples(), icube->Lines(), smag, lmag); 

  string from = ui.GetFilename("FROM");
  cube.Open(from);

  // Determine the output size
  int samples = transform->OutputSamples();
  int lines = transform->OutputLines();

  // Allocate the output file
  Cube *ocube = p.SetOutputCube ("TO", samples, lines, icube->Bands());

  // Set up the interpolator
  Interpolator *interp;
  if (ui.GetString("INTERP") == "NEARESTNEIGHBOR") {
    interp = new Interpolator(Interpolator::NearestNeighborType);
  }
  else if (ui.GetString("INTERP") == "BILINEAR") {
    interp = new Interpolator(Interpolator::BiLinearType);
  }
  else if (ui.GetString("INTERP") == "CUBICCONVOLUTION") {
    interp = new Interpolator(Interpolator::CubicConvolutionType);
  }
  else {
    string msg = "Unknown value for INTERP [" +
                 ui.GetString("INTERP") + "]";
    throw iException::Message(iException::Programmer,msg,_FILEINFO_);
  }
  p.StartProcess(*transform, *interp);

  try {
    PvlGroup &mapgroup = cube.Label()->FindGroup("Mapping", Pvl::Traverse);

    if(smag != lmag) {
      ocube->DeleteGroup("Mapping");
    }
    else {
      // Update pixel resolution
      double pixres = mapgroup["PixelResolution"];
      mapgroup["PixelResolution"] = pixres / smag;
      ocube->PutGroup(mapgroup);
    }
  }
  catch(iException &e) {
    e.Clear();
    // Update alphacube group
    AlphaCube alpha(cube.Samples(), cube.Lines(),
                    ocube->Samples(), ocube->Lines(),
                    0.5, 0.5, cube.Samples()+0.5, cube.Lines()+0.5);
    alpha.UpdateGroup(*ocube->Label());
  }
  
  p.EndProcess();

  delete transform;
  delete interp;
}

