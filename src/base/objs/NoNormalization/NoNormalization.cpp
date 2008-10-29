#include <cmath>
#include "NoNormalization.h"
#include "SpecialPixel.h"
#include "iException.h"

namespace Isis {
  NoNormalization::NoNormalization (Pvl &pvl, PhotoModel &pmodel) :
      NormModel(pvl,pmodel) {
  }

  void NoNormalization::NormModelAlgorithm (double phase, double incidence, 
      double emission, double dn, double &albedo, double &mult,
      double &base)
  {
    double a;

    a = GetPhotoModel()->CalcSurfAlbedo(phase,incidence,emission);

    if (a != 0.0) {
      albedo = a * dn + phase;
    } else {
      albedo = NULL8;
      return;
    }
  }
}

extern "C" Isis::NormModel *NoNormalizationPlugin (Isis::Pvl &pvl, Isis::PhotoModel &pmodel) {
  return new Isis::NoNormalization(pvl,pmodel);
}
