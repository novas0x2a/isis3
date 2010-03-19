#include "Isis.h"

#include <cmath>

#include "ProcessByLine.h"
#include "Projection.h"
#include "ProjectionFactory.h"
#include "Camera.h"
#include "CameraFactory.h"
#include "GroundGrid.h"
#include "UniversalGroundMap.h"

using namespace std; 
using namespace Isis;

void imageGrid(Buffer &in, Buffer &out);
void groundGrid(Buffer &in, Buffer &out);

void createGroundImage(Camera *cam, Projection *proj);

bool outline;
int baseLine, baseSample, lineInc, sampleInc;  
double baseLat, baseLon, latInc, lonInc;

int inputSamples, inputLines;
GroundGrid *latLonGrid;

void IsisMain() {
  latLonGrid = NULL;

  // We will be processing by line
  ProcessByLine p;
  Cube *icube = p.SetInputCube("FROM");

  UserInterface &ui = Application::GetUserInterface();
  string mode = ui.GetString("MODE");

  outline = ui.GetBoolean("OUTLINE");

  inputSamples = icube->Samples();
  inputLines   = icube->Lines();

  // Line & sample based grid
  if(mode == "IMAGE") { 
    p.SetOutputCube ("TO");
    baseLine = ui.GetInteger("BASELINE"); 
    baseSample = ui.GetInteger("BASESAMPLE"); 
    lineInc = ui.GetInteger("LINC"); 
    sampleInc = ui.GetInteger("SINC"); 
    p.StartProcess(imageGrid);
    p.EndProcess();
  }
  // Lat/Lon based grid
  else {
    CubeAttributeOutput oatt("+32bit");
    p.SetOutputCube (ui.GetFilename("TO"), oatt, icube->Samples(), 
                     icube->Lines(), icube->Bands());
  
    baseLat = ui.GetDouble("BASELAT");
    baseLon = ui.GetDouble("BASELON");
    latInc = ui.GetDouble("LATINC");
    lonInc = ui.GetDouble("LONINC");

    UniversalGroundMap *gmap = new UniversalGroundMap(*icube);
    latLonGrid = new GroundGrid(gmap, icube->Samples(), icube->Lines());

    Progress progress;
    progress.SetText("Calculating Grid");

    latLonGrid->CreateGrid(baseLat, baseLon, latInc, lonInc, &progress);

    p.StartProcess(groundGrid);
    p.EndProcess();

    delete latLonGrid;
    latLonGrid = NULL;

    delete gmap;
    gmap = NULL;
  }
}

// Line processing routine
void imageGrid(Buffer &in, Buffer &out) {
  // Add gridlines to input image
  // Check if current line is affected by grid
  if(in.Line() % lineInc == baseLine % lineInc) {
    for(int i = 0; i < in.size(); i++) {
      out[i] = Isis::Hrs;
    }
  }
  // Draw vertical lines here 
  else {
    for(int i = 0; i < in.size(); i++) {
      if((i+1) % sampleInc == baseSample % sampleInc) {
        out[i] = Isis::Hrs; 
      }
      else {
        out[i] = in[i];
      }
    }
  }

  // draw outline
  if(outline) {
    if(in.Line() == 1 || in.Line() == inputLines) {
      for(int i = 0; i < in.size(); i++) {
        out[i] = Isis::Hrs; 
      }
    }
    else {
      out[0] = Isis::Hrs;
      out[out.size()-1] = Isis::Hrs; 
    }
  }
}

// Line processing routine
void groundGrid(Buffer &in, Buffer &out) {
  for(int samp = 1; samp <= in.SampleDimension(); samp++) {
    if(latLonGrid->PixelOnGrid(samp - 1, in.Line() - 1)) {
      out[samp-1] = Isis::Hrs;
    }
    else {
      out[samp-1] = in[samp-1];
    }
  }
}
