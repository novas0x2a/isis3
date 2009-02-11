#include <iostream>
#include <cstdlib>
#include "PhotoModel.h"
#include "PhotoModelFactory.h"
#include "AtmosModel.h"
#include "AtmosModelFactory.h"
#include "TopoAtm.h"
#include "NormModelFactory.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "iException.h"
#include "Preference.h"

using namespace Isis;

int main () {
  Isis::Preference::Preferences(true);

  double result;
  double mult;
  double base;

  std::cout << "UNIT TEST for TopoAtm normalization function" << std::endl << std::endl;

  PvlGroup algp("Algorithm");
  algp += PvlKeyword("Name", "Lambert");

  PvlObject op("PhotometricModel");
  op.AddGroup(algp);

  PvlGroup alga("Algorithm");
  alga += PvlKeyword("Name", "Anisotropic1");
  alga += PvlKeyword("Bha", 0.85);

  PvlObject oa("AtmosphericModel");
  oa.AddGroup(alga);

  PvlGroup algn("Algorithm");
  algn += PvlKeyword("Name", "TopoAtm");
  algn += PvlKeyword("Albedo", 0.0690507);
  algn += PvlKeyword("Incref", 30.0);

  PvlObject on("NormalizationModel");
  on.AddGroup(algn);

  Pvl pvl;
  pvl.AddObject(op);
  pvl.AddObject(oa);
  pvl.AddObject(on);

  std::cout << pvl << std::endl << std::endl;

  try {
    PhotoModel *pm = PhotoModelFactory::Create(pvl);
    AtmosModel *am = AtmosModelFactory::Create(pvl,*pm);
    TopoAtm *nm = (TopoAtm*)NormModelFactory::Create(pvl,*pm,*am);

    nm->CalcNrmAlbedo(86.7226722,51.7002388,38.9414439,51.7910076,39.0176048,.0800618902,result,mult,base);

    std::cout << "Test phase=86.7226722, incidence=51.7002388, emission=38.9414439, " <<
        "demincidence=51.7910076, dememission=39.0176048, dn=.0800618902 ..." << std::endl;
    std::cout << "Normalization value = " << result << std::endl << std::endl;

    nm->CalcNrmAlbedo(86.7207248,51.7031305,38.9372914,51.8776595,38.9719125,.0797334611,result,mult,base);

    std::cout << "Test phase=86.7207248, incidence=51.7031305, emission=38.9372914, " <<
        "demincidence=51.8776595, dememission=38.9719125, dn=.0797334611 ..." << std::endl;
    std::cout << "Normalization value = " << result << std::endl << std::endl;

    nm->CalcNrmAlbedo(86.7187773,51.7060221,38.9331391,51.9643503,38.9264284,.0794225037,result,mult,base);

    std::cout << "Test phase=86.7187773, incidence=51.7060221, emission=38.9331391, " <<
        "demincidence=51.9643503, dememission=38.9264284, dn=.0794225037 ..." << std::endl;
    std::cout << "Normalization value = " << result << std::endl << std::endl;
  }
  catch (iException &e) {
    e.Report();
  }

  return 0;
}