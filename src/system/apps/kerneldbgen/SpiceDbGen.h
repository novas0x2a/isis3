#include <iostream>
#include "Filename.h"
#include "Pvl.h"
#include "iString.h"
#include "naif/SpiceUsr.h"
#include "qstringlist.h"
#include "KernelDb.h"
#include "iException.h"

#ifndef SpiceDbGen_h
#define SpiceDbGen_h

using namespace std;
using namespace Isis;

class SpiceDbGen{
public:
  SpiceDbGen(iString type);
  PvlObject Direct(iString quality, iString location, iString filter);
  void FurnishDependencies(string sclk, string fk);
private:
  QStringList GetFiles(Filename location, iString filter);
  PvlGroup AddSelection(Filename fileIn);
  PvlGroup FormatIntervals(SpiceCell &coverage, string type);
  PvlGroup GetIntervals(SpiceCell &cover);
  //private instance variables
  iString p_type;
  char* calForm;
};
#endif
