#include <iostream>
#include <cstdlib>
#include "AutoReg.h"
#include "AutoRegFactory.h"
#include "Chip.h"
#include "Cube.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "Preference.h"

using namespace Isis;

int main () {
  Isis::Preference::Preferences(true);
  try {
  PvlGroup alg("Algorithm");
  alg += PvlKeyword("Name","MinimumDifference");
  alg += PvlKeyword("Tolerance", 5.0);
  alg += PvlKeyword("SubpixelAccuracy", "True");
  alg += PvlKeyword("MinimumPatternZScore", "1");

  PvlGroup pchip("PatternChip");
  PvlKeyword psamps("Samples");
  psamps.AddValue(25);
  psamps.AddValue(20);
  psamps.AddValue(15);
  pchip.AddKeyword(psamps);

  PvlKeyword plines("Lines");
  plines.AddValue(25);
  plines.AddValue(20);
  plines.AddValue(15);
  pchip.AddKeyword(plines);

  PvlKeyword ppers("FloatPercent");
  ppers.AddValue(50);
  ppers.AddValue(25);
  pchip.AddKeyword(ppers);

  PvlGroup schip("SearchChip");
  schip += PvlKeyword("Samples",50);
  schip += PvlKeyword("Lines",50);

  PvlGroup smodel("SurfaceModel");
  smodel += PvlKeyword("DistanceTolerance", 1.0);
  smodel += PvlKeyword("WindowSize", 3);

  PvlObject o("AutoReg");
  o.AddGroup(alg);
  o.AddGroup(pchip);
  o.AddGroup(schip);
  o.AddGroup(smodel);

  Pvl pvl;
  pvl.AddObject(o);
  std::cout << pvl << std::endl;
  AutoReg *ar = AutoRegFactory::Create(pvl);

  Cube c;
  c.Open("$mgs/testData/ab102401.cub");

  ar->SearchChip()->TackCube(125.0,50.0);
  ar->SearchChip()->Load(c);
  ar->PatternChip()->TackCube(120.0,45.0);
  ar->PatternChip()->Load(c);

  std::cout << "Register = " << ar->Register() << std::endl;
  std::cout << "Position = " << ar->CubeSample() << " " << 
                                ar->CubeLine() << std::endl;
  }
  catch (iException &e) {
    e.Report();
  }

  return 0;
}
