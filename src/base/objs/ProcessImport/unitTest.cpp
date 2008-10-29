#include "Isis.h"
#include "ProcessImport.h"
#include "Cube.h"
#include "Application.h"
#include "Preference.h"
#include "Statistics.h"

using namespace std;

void IsisMain() {

  Isis::Preference::Preferences(true);

  cout << "Testing Isis::ProcessImport Class ... " << endl;

  Isis::Preference::Preferences(true);
  
  Isis::ProcessImport p;
  p.SetInputFile("$base/testData/isisTruth.dat");
  p.SetBase(0.0);
  p.SetMultiplier(1.0);
  p.SetDataHeaderBytes(0);
  p.SetDataPrefixBytes(0);
  p.SetDataSuffixBytes(0);
  p.SetDataTrailerBytes(0);
  p.SetDimensions(126, 126, 1);
  p.SetFileHeaderBytes(16384);
  p.SetOrganization(Isis::ProcessImport::BSQ);
  p.SetPixelType(Isis::Real);
  p.SetByteOrder(Isis::Lsb);
  p.SetOutputCube("TO");
  p.StartProcess();
  p.EndProcess();

  Isis::Process p2;
  Isis::CubeAttributeInput att;
  string file = Isis::Application::GetUserInterface().GetFilename("TO");
  Isis::Cube *icube = p2.SetInputCube(file, att);
  Isis::Statistics *stat = icube->Statistics();
  cout << stat->Average() << endl;
  cout << stat->Variance() << endl; 
  p2.EndProcess();
  remove(file.c_str());
  cout << endl;
}
