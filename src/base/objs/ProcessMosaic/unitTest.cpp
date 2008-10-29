#include "Isis.h"

#include <string>

#include "ProcessMosaic.h"
#include "Portal.h"
#include "Application.h"

using namespace std;

void IsisMain() {

  Isis::Preference::Preferences(true);

  cout << "Testing Isis::ProcessMosaic Class ... " << endl;

  // Create the temp parent cube
  Isis::Process p;
  p.SetOutputCube("TO", 53, 22, 3);
  p.EndProcess();

  // Drop a small area into the middle of the output
  Isis::ProcessMosaic m1;
  m1.SetOutputCube ("TO");
  m1.SetInputCube ("FROM", 1, 1, 1, 10, 5, 2);
  m1.StartProcess (25, 11, 1, input);
  m1.EndProcess ();

  // Drop the first 3x3x2 into the upper left corner
  Isis::ProcessMosaic m2;
  m2.SetInputCube ("FROM", 1, 1, 1, 3, 3, 2);
  m2.SetOutputCube ("TO");
  m2.StartProcess (1, 1, 1, input);
  m2.EndProcess ();

  // Drop 2,2,1 into the lower right corner of band 2
  Isis::ProcessMosaic m3;
  m3.SetInputCube ("FROM", 2, 2, 1);
  m3.SetOutputCube ("TO");
  m3.StartProcess (53, 22, 2, input);
  m3.EndProcess ();

  // Drop 3,3,1 into the upper right corner of band 3
  Isis::ProcessMosaic m4;
  m4.SetInputCube ("FROM", 3, 3, 1, 100, 1, 1);
  m4.SetOutputCube ("TO");
  m4.StartProcess (53, 1, 3, input);
  m4.EndProcess ();


  // Test errors

  // Try to open two input cubes
  cout << "Test multipal input error" << endl;
  try {
    Isis::ProcessMosaic m;
    m.SetInputCube ("FROM");
    m.SetInputCube ("TO");
    m.StartProcess (1, 1, 1, input);
    m.EndProcess ();
  }
  catch (Isis::iException &e) {
    e.Report(false);
    p.EndProcess();
    cout << endl;
  }

  // Try to open two output cubes
  cout << "Test multipal output error" << endl;
  try {
    Isis::ProcessMosaic m;
    m.SetOutputCube ("TO");
    m.SetOutputCube ("TO");
    m.StartProcess (1, 1, 1, input);
    m.EndProcess ();
  }
  catch (Isis::iException &e) {
    e.Report(false);
    p.EndProcess();
    cout << endl;
  }

  // Drop the input completly outside the output
  cout << "Test input does not overlap mosaic" << endl;
  try {
    Isis::ProcessMosaic m;
    m.SetInputCube ("FROM");
    m.SetOutputCube ("TO");
    m.StartProcess (-200, 0, 1, input);
    m.EndProcess ();
  }
  catch (Isis::iException &e) {
    e.Report(false);
    p.EndProcess();
    cout << endl;
  }

  // Drop the input completly outside the output
  cout << "Test input does not overlap mosaic" << endl;
  try {
    Isis::ProcessMosaic m;
    m.SetInputCube ("FROM");
    m.SetOutputCube ("TO");
    m.StartProcess (54, 23, 1, input);
    m.EndProcess ();
  }
  catch (Isis::iException &e) {
    e.Report(false);
    p.EndProcess();
    cout << endl;
  }

  // Don't open an input cube
  cout << "Test no input cube" << endl;
  try {
    Isis::ProcessMosaic m;
    m.SetOutputCube ("TO");
    m.StartProcess (1, 1, 1, input);
    m.EndProcess ();
  }
  catch (Isis::iException &e) {
    e.Report(false);
    p.EndProcess();
    cout << endl;
  }

  // Don't open an output cube
  cout << "Test no output cube" << endl;
  try {
    Isis::ProcessMosaic m;
    m.SetInputCube ("FROM");
    m.StartProcess (1, 1, 1, input);
    m.EndProcess ();
  }
  catch (Isis::iException &e) {
    e.Report(false);
    p.EndProcess();
    cout << endl;
  }

  // Read some key positions and output them
  cout << "Test key positions within the output cube" << endl;
  Isis::Cube cube;
  Isis::UserInterface &ui = Isis::Application::GetUserInterface();
  string to = ui.GetFilename ("TO");
  cube.Open(to);
  Isis::Portal portal (1, 1, cube.PixelType());

  portal.SetPosition (1, 1, 1);
  cube.Read(portal);
  cout << "Mosaic position (1,1,1-3) {" << portal[0] << ", ";
  portal.SetPosition (1, 1, 2);
  cube.Read(portal);
  cout << portal[0] << ", ";
  portal.SetPosition (1, 1, 3);
  cube.Read(portal);
  cout << portal[0] << "}" << endl;

  portal.SetPosition (53, 1, 1);
  cube.Read(portal);
  cout << "Mosaic position (53,1,1-3) {" << portal[0] << ", ";
  portal.SetPosition (53, 1, 2);
  cube.Read(portal);
  cout << portal[0] << ", ";
  portal.SetPosition (53, 1, 3);
  cube.Read(portal);
  cout << portal[0] << "}" << endl;

  portal.SetPosition (1, 22, 1);
  cube.Read(portal);
  cout << "Mosaic position (1,22,1-3) {" << portal[0] << ", ";
  portal.SetPosition (1, 22, 2);
  cube.Read(portal);
  cout << portal[0] << ", ";
  portal.SetPosition (1, 22, 3);
  cube.Read(portal);
  cout << portal[0] << "}" << endl;

  portal.SetPosition (53, 22, 1);
  cube.Read(portal);
  cout << "Mosaic position (53,22,1-3) {" << portal[0] << ", ";
  portal.SetPosition (53, 22, 2);
  cube.Read(portal);
  cout << portal[0] << ", ";
  portal.SetPosition (53, 22, 3);
  cube.Read(portal);
  cout << portal[0] << "}" << endl;

  portal.SetPosition (27, 12, 1);
  cube.Read(portal);
  cout << "Mosaic position (27,12,1-3) {" << portal[0] << ", ";
  portal.SetPosition (27, 12, 2);
  cube.Read(portal);
  cout << portal[0] << ", ";
  portal.SetPosition (27, 12, 3);
  cube.Read(portal);
  cout << portal[0] << "}" << endl;

  cube.Close(true);
}

