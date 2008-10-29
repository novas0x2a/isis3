#include "Isis.h"
#include "ProcessBySpectra.h"
#include "UserInterface.h"
#include "QuickFilter.h"

#include <climits>

using namespace std; 
using namespace Isis;

// Which pixel types to filter
bool filterNull;
bool filterLis;
bool filterLrs;
bool filterHis;
bool filterHrs;
bool propagate;
int bands;
double low ;
double high;

// Prototype
void Filter (Buffer &in, Buffer &out);

void IsisMain() {
  //Set up ProcessBySpectra
  ProcessBySpectra p;

  // Obtain input and output cubes
  p.SetInputCube("FROM");
  p.SetOutputCube("TO");

  // Find out which special pixels are to be filtered
  UserInterface &ui = Application::GetUserInterface();
 
  //Set the Boxcar Parameters
  low = -DBL_MAX;
  high = DBL_MAX;

  if (ui.WasEntered("LOW")) {
    low = ui.GetDouble("LOW");
  }
  if (ui.WasEntered("HIGH")) {
    high = ui.GetDouble("HIGH");
  }
 
  bands = ui.GetInteger("BANDS");
  //Start the filter method
  p.StartProcess(Filter);
  p.EndProcess();
  
}

//Function to loop through the bands, and determine the
//average value of the pixels around each valid pixel, writing that
//average to the output at the pixel index
void Filter (Buffer &in, Buffer &out) {
  Isis::QuickFilter *filter = new Isis::QuickFilter(in.size(), bands, 1);
  filter->SetMinMax(low, high);
  filter->AddLine(in.DoubleBuffer());

 for (int i=0; i < in.size(); i++) {
   out[i] = filter->Average(i);
 }
 delete filter;
 
}

