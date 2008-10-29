#include "Isis.h"
#include "ProcessByLine.h"
#include "Projection.h"
#include "ProjectionFactory.h"
#include "Camera.h"
#include "CameraFactory.h"
#include <cmath>

using namespace std; 
using namespace Isis;

void imageGrid(Buffer &in, Buffer &out);
void groundGrid(Buffer &in, Buffer &out);
void computeRemainders(vector<double> &buff, int line);

bool outline;
int numLines;
int baseLine, baseSample, lineInc, sampleInc;  
double baseLat, baseLon, latInc, lonInc;
Projection *proj; 
Camera *cam;

vector<double> topLine; 
vector<double> bottomLine;

void IsisMain() {
  // We will be processing by line
  ProcessByLine p;
  Cube *icube = p.SetInputCube("FROM");
  
  // Avoid segfault
  proj = NULL;
  cam = NULL;

  UserInterface &ui = Application::GetUserInterface();
  string mode = ui.GetString("MODE");

  outline = ui.GetBoolean("OUTLINE");
  numLines = icube->Lines();

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
  // Lat & lon based grid
  else {
    CubeAttributeOutput oatt("+32bit");
    p.SetOutputCube (ui.GetFilename("TO"), oatt, icube->Samples(), 
                     icube->Lines(), icube->Bands());
    try {
      proj = icube->Projection();
    }
    catch(iException &e) {
      e.Clear();
      try {
        cam = icube->Camera();
      }
      catch(iException &e) {
        string msg = "Input file does not have a projection or camera";
        throw iException::Message(iException::User, msg, _FILEINFO_);
      }
    }
    
    baseLat = ui.GetDouble("BASELAT");
    baseLon = ui.GetDouble("BASELON");
    latInc = ui.GetDouble("LATINC");
    lonInc = ui.GetDouble("LONINC");
    
    topLine.resize(icube->Samples()+1); 
    bottomLine.resize(icube->Samples()+1); 
    p.StartProcess(groundGrid);
    p.EndProcess();
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
    if(in.Line() == 1 || in.Line() == numLines) {
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
  // Get remainders for gradient computation
  if(in.Line() == 1) {
    computeRemainders(topLine, 1);
  }
  else {
    topLine = bottomLine;
  }

  computeRemainders(bottomLine, in.Line() + 1);

  // Loop and compute gradient on remainders 
  for(int i = 0; i < in.size(); i++) {
    double gradient;
    if(topLine[i] == Isis::Null || topLine[i+1] == Isis::Null ||
       bottomLine[i] == Isis::Null || bottomLine[i+1] == Isis::Null) {
      gradient = 0.0;
    }
    else {
      gradient = abs(topLine[i] - bottomLine[i+1]) + abs(bottomLine[i] - topLine[i+1]);
    }

    double tolerance = (lonInc < latInc) ? lonInc*0.75 : latInc*0.75;
    if(gradient < tolerance) {
      out[i] = in[i];
    }
    else {
      out[i] = Isis::Hrs;
    }
  }

  // draw outline
  if(outline) {
    if(in.Line() == 1 || in.Line() == numLines) {
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

// Takes the lat/lon coordinates at center of pixel and get the remainder
// after dividing by latInc and lonInc.  These results will allow a gradient
// algorithm to highlight exactly where a gridline should be.
void computeRemainders(vector<double> &buff, int line) {

  for(int i = 0; i < (int)buff.size(); i++) {
    buff[i] = Isis::Null;
    
    double lat, lon;
    if(proj != NULL) {
      bool hasCoords = proj->SetWorld(i+1, line);
      if(!hasCoords) continue;

      lat = proj->Latitude();
      lon = proj->Longitude();

      // This excludes areas outside of the projection
      if(proj->Has360Domain() && (lon < 0 || lon > 360)) continue;
      if(proj->Has180Domain() && (lon < -180 || lon > 180)) continue;
    }
    // Use camera to get coordinates instead
    else {
      bool hasCoords = cam->SetImage(i+1, line);
      if(!hasCoords) continue;
      lat = cam->UniversalLatitude();
      lon = cam->UniversalLongitude();
    }

    double latRemainder = fmod(lat - baseLat, latInc);
    double lonRemainder = fmod(lon - baseLon, lonInc); 
    buff[i] = latRemainder + lonRemainder;

    // This fixes problems when crossing the 0 lat or 0 lon boundary
    if(lat < baseLat) buff[i] += latInc;
    if(lon < baseLon) buff[i] += lonInc;
  }
}
