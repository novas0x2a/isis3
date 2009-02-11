#include "Isis.h"
#include "Progress.h"
#include "iException.h"
#include "SerialNumber.h"
#include "ImagePolygon.h"
#include "Process.h"

using namespace std; 
using namespace Isis;

void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();
  Cube cube;
  cube.Open(ui.GetFilename ("FROM"), "rw");

  // Make sure cube has been run through spiceinit
  try {
    cube.Camera();
  } catch ( iException &e ) {
    string msg = "Spiceinit must be run before initializing the polygon";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  int pixInc = ui.GetInteger("PIXINCREMENT");

  Progress prog;
  prog.SetMaximumSteps(1);
  prog.CheckStatus ();

  std::string sn = SerialNumber::Compose(cube);
  cube.BlobDelete("Polygon",sn);

  ImagePolygon poly;
  poly.Create(cube,pixInc);
  cube.Write(poly);

  Process p;
  p.SetInputCube("FROM");
  p.WriteHistory(cube);

  cube.Close();
  prog.CheckStatus ();
}