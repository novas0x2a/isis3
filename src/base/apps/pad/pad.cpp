#include "Isis.h"
#include "ProcessMosaic.h"
#include "ProcessByLine.h"
#include "AlphaCube.h"
#include "SpecialPixel.h"

using namespace std; 
using namespace Isis;

void CreateBase (Buffer &buf);

void IsisMain() {
  // We will be use a mosaic technique so get the size of the input file
  ProcessMosaic p;
  Cube *icube = p.SetInputCube ("FROM");
  int ins = icube->Samples();
  int inl = icube->Lines();
  int inb = icube->Bands();

  // Retrieve the padding parameters
  UserInterface &ui = Application::GetUserInterface();
  int leftPad = ui.GetInteger("LEFT");
  int rightPad = ui.GetInteger("RIGHT");
  int topPad = ui.GetInteger("TOP");
  int bottomPad = ui.GetInteger("BOTTOM");

  // Compute the output size
  int ns = ins + leftPad + rightPad;
  int nl = inl + topPad + bottomPad;
  int nb = inb;

  // We need to create the output file
  ProcessByLine bl;
  bl.SetInputCube("FROM");  // Do this to match pixelType
  bl.SetOutputCube("TO",ns,nl,nb);
  bl.ClearInputCubes();     // Now get rid of it
  bl.Progress()->SetText("Creating pad");
  bl.StartProcess(CreateBase);
  bl.EndProcess();  

  // Place the input in the file we just created
  Cube *ocube = p.SetOutputCube ("TO");
  p.Progress()->SetText("Inserting cube");
  p.StartProcess(leftPad+1, topPad+1, 1, input);

  // Add the AlphaCube group
  AlphaCube acube(ins,inl,ns,nl,
                      0.5-leftPad, 0.5-topPad, 
                      ins+rightPad+0.5, inl+bottomPad+0.5);
  acube.UpdateGroup(*ocube->Label());
  p.EndProcess();
}

void CreateBase (Buffer &buf) {
  for (int i=0; i<buf.size(); i++) {
    buf[i] = NULL8;
  }
}
