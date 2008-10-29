#include "Isis.h"
#include "ProcessByLine.h"
#include "SpecialPixel.h"
#include "Stretch.h"

using namespace std; 
using namespace Isis;

void stretch (Buffer &in, Buffer &out);
Stretch str;

void IsisMain() {
  // We will be processing by line
  ProcessByLine p;

  // Setup the input and output cubes
  p.SetInputCube("FROM");
  p.SetOutputCube ("TO");
  
  // Get the stretch string and parse it
  UserInterface &ui = Application::GetUserInterface();
  if (ui.WasEntered("PAIRS")) str.Parse(ui.GetString("PAIRS"));

  // Setup new mappings for special pixels if necessary
  if (ui.WasEntered("NULL")) 
    str.SetNull(StringToPixel(ui.GetString("NULL")));
  if (ui.WasEntered("LIS"))
    str.SetLis(StringToPixel(ui.GetString("LIS")));
  if (ui.WasEntered("LRS"))
    str.SetLrs(StringToPixel(ui.GetString("LRS")));
  if (ui.WasEntered("HIS"))
     str.SetHis(StringToPixel(ui.GetString("HIS")));
  if (ui.WasEntered("HRS"))
     str.SetHrs(StringToPixel(ui.GetString("HRS")));


  // Start the processing
  p.StartProcess(stretch);
  p.EndProcess();
}

// Line processing routine
void stretch (Buffer &in, Buffer &out) {
  for (int i=0; i<in.size(); i++) {
    out[i] = str.Map(in[i]);
  }
}
