#include <iostream>
#include <cstdlib>
#include "InterestOperator.h"
#include "InterestOperatorFactory.h"
#include "ForstnerOperator.h"
#include "Chip.h"
#include "Cube.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "Preference.h"

using namespace Isis;

int main () {
  try {
    Isis::Preference::Preferences(true);
  PvlGroup op("Operator");
  op += PvlKeyword("Name","Forstner");
  op += PvlKeyword("DeltaLine", 100);
  op += PvlKeyword("DeltaSamp", 100);
  op += PvlKeyword("Samples", 15);
  op += PvlKeyword("Lines", 15);
  op += PvlKeyword("MinimumInterest", 0.0);

  PvlObject o("InterestOperator");
  o.AddGroup(op);

  Pvl pvl;
  pvl.AddObject(o);
  std::cout << pvl << std::endl;

  InterestOperator *iop = InterestOperatorFactory::Create(pvl);

  Cube c;
  c.Open("$mgs/testData/ab102401.cub");

  iop->Operate(c, 100, 350);

  std::cout << "Sample: " << iop->CubeSample() << std::endl
            << "Line : " << iop->CubeLine() << std::endl
            << "Interest: " << iop->InterestAmount() << std::endl;
  }
  catch (iException &e) {
    e.Report();
  }

  return 0;
}
