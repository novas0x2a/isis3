#include "Isis.h"
#include <string>
#include "Camera.h"
#include "ProcessByLine.h"
#include "SpecialPixel.h"
#include "Photometry.h"
#include "Pvl.h"
#include "Cube.h"

#include "PvlGroup.h"
#include "iException.h"

#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))

using namespace std; 
using namespace Isis;

// Global variables
Camera *cam;
Cube *icube;
Photometry *pho;

void photomet (Buffer &in, Buffer &out);

void IsisMain() {
  // We will be processing by line 
  ProcessByLine p;

  // Set up the input cube and get camera information
  icube = p.SetInputCube("FROM");
  cam = icube->Camera();

  // Create the output cube
  p.SetOutputCube("TO");

  // Set up the user interface
  UserInterface &ui = Application::GetUserInterface();
  // Get the name of the parameter file
  Pvl par(ui.GetFilename("PHOPAR"));

  // Get the BandBin Center from the image
  PvlGroup pvlg = icube->GetGroup("BandBin");
  double wl;
  if (pvlg.HasKeyword("Center")) {
    PvlKeyword &wavelength = pvlg.FindKeyword("Center");
    wl = wavelength[0];
  } else {
    wl = 1.0;
  }

  // Create the photometry object and set the wavelength
  PvlGroup &algo = par.FindObject("NormalizationModel").FindGroup("Algorithm",Pvl::Traverse);
  if (!algo.HasKeyword("Wl")) {
    algo.AddKeyword(Isis::PvlKeyword("Wl",wl));
  }
  pho = new Photometry(par);
  pho->SetPhotomWl(wl);

  // Start the processing
  p.StartProcess(photomet);
  p.EndProcess();
}

void photomet (Buffer &in, Buffer &out) {

  double pha,inc,ema,mult,base;
  for (int i=0; i<in.size(); i++) {
    if (cam->SetImage(in.Sample(i),in.Line(i)) &&
        IsValidPixel(in[i])) {
      pha = cam->PhaseAngle();
      pha = MIN(180.0,pha);
      pha = MAX(0.0,pha);
      inc = cam->IncidenceAngle();
      inc = MIN(90.0,inc);
      inc = MAX(0.0,inc);
      ema = cam->EmissionAngle();
      ema = MIN(90.0,ema);
      ema = MAX(0.0,ema);
      pho->Compute(pha,inc,ema,in[i],out[i],mult,base);
    } else {
      out[i] = in[i];
    }
  }
}
