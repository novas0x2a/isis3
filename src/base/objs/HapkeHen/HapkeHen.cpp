#include <cmath>
#include "HapkeHen.h"
#include "iException.h"

#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

namespace Isis {
  HapkeHen::HapkeHen (Pvl &pvl) : PhotoModel(pvl) {
  }

  double HapkeHen::PhotoModelAlgorithm (double phase, double incidence,
        double emission) {
    double pht_hapkehen;
    double pharad;
    double incrad;
    double emarad;
    double munot;
    double mu;
    double cost;
    double sint;
    double tan2t;
    double gamma;
    double hgs;
    double sing;
    double cosg;
    double tang;
    double bg;
    double pg;
    double pg1;
    double pg2;
    double sini;
    double coti;
    double cot2i;
    double ecoti;
    double ecot2i;
    double u0p0;
    double sine;
    double cote;
    double cot2e;
    double cosei;
    double sinei;
    double caz;
    double az;
    double az2;
    double faz;
    double tanaz2;
    double sin2a2;
    double api;
    double ecote;
    double ecot2e;
    double up0;
    double q;
    double ecei;
    double s2ei;
    double u0p;
    double up;
    double ecee;
    double s2ee;
    double rr1;
    double rr2;

    pharad = phase * Isis::PI / 180.0;
    incrad = incidence * Isis::PI / 180.0;
    emarad = emission * Isis::PI / 180.0;
    munot = cos(incrad);
    mu = cos(emarad);

    if (p_photoTheta != p_photoThetaold) {
      cost = cos(p_photoTheta * Isis::PI / 180.0);
      sint = sin(p_photoTheta * Isis::PI / 180.0);
      p_photoCott = cost / MAX(1.0e-10, sint);
      p_photoCot2t = p_photoCott * p_photoCott;
      p_photoTant = sint / cost;
      tan2t = p_photoTant * p_photoTant;
      p_photoSr = sqrt(1.0 + Isis::PI * tan2t);
      p_photoOsr = 1.0 / p_photoSr;
      SetOldTheta(p_photoTheta);
    }

    if (incidence >= 90.0) {
      pht_hapkehen = 0.0;
      return pht_hapkehen;
    }

    gamma = sqrt(1.0 - p_photoWh);
    hgs = p_photoHg1 * p_photoHg1;

    sing = sin(pharad);
    cosg = cos(pharad);
    tang = sing / MAX(1.0e-10, cosg);

    if (p_photoHh == 0.0) {
      bg = 0.0;
    } else {
      if (phase <= 90.0) {
        bg = p_photoB0 / MAX(-5.0, 1.0 + tang / p_photoHh);
      } else {
        bg = 0.0;
      }
    }

    pg1 = (1.0 - p_photoHg2) * (1.0 - hgs) / pow((1.0 + hgs + 2.0 * 
        p_photoHg1 * cosg), 1.5); 
    pg2 = p_photoHg2 * (1.0 - hgs) / pow((1.0 + hgs - 2.0 * 
        p_photoHg1 * cosg), 1.5);
    pg = pg1 + pg2;

    if (p_photoTheta <= 0.0) {
      pht_hapkehen = p_photoWh / 4.0 * munot / (munot + mu) * ((1.0 + bg) *
          pg - 1.0 + Hfunc(munot,gamma) * Hfunc(mu,gamma));
      return pht_hapkehen;
    }

    sini = sin(incrad);
    coti = munot / MAX(1.0e-10, sini);
    cot2i = coti * coti;
    ecoti = exp(MIN(-p_photoCot2t * cot2i / Isis::PI , 23.0));
    ecot2i = exp(MIN(-2.0 * p_photoCott * coti / Isis::PI, 23.0));
    u0p0 = p_photoOsr * (munot + sini * p_photoTant * ecoti / (2.0 - ecot2i));

    sine = sin(emarad);
    cote = mu / MAX(1.0e-10, sine);
    cot2e = cote * cote;

    cosei = mu * munot;
    sinei = sine * sini;

    if (sinei == 0.0) {
      caz = 1.0;
      az = 0.0;
    } else {
      caz = (cosg - cosei) / sinei;
      if (caz <= -1.0) {
        az = 180.0;
      } else if (caz > 1.0) {
        az = 0.0;
      } else {
        az = acos(caz) * 180.0 / Isis::PI;
      }
    }

    az2 = az / 2.0;
    if (az2 >= 90.0) {
      faz = 0.0;
    } else {
      tanaz2 = tan(az2 * Isis::PI / 180.0);
      faz = exp(MIN(-2.0 * tanaz2, 23.0));
    }

    sin2a2 = pow(sin(az2 * Isis::PI / 180.0), 2.0);
    api = az / 180.0;

    ecote = exp(MIN(-p_photoCot2t * cot2e / Isis::PI, 23.0));
    ecot2e = exp(MIN(-2.0 * p_photoCott * cote / Isis::PI, 23.0));
    up0 = p_photoOsr * (mu + sine * p_photoTant * ecote / (2.0 - ecot2e));

    if (incidence <= emission) {
      q = p_photoOsr * munot / u0p0;
    } else {
      q = p_photoOsr * mu / up0;
    }

    if (incidence <= emission) {
      ecei = (2.0 - ecot2e - api * ecot2i);
      s2ei = sin2a2 * ecoti;
      u0p = p_photoOsr * (munot + sini * p_photoTant * (caz * ecote + s2ei) / ecei);
      up = p_photoOsr * (mu + sine * p_photoTant * (ecote - s2ei) / ecei);
    } else {
      ecee = (2.0 - ecot2i - api * ecot2e);
      s2ee = sin2a2 * ecote;
      u0p = p_photoOsr * (munot + sini * p_photoTant * (ecoti - s2ee) / ecee);
      up = p_photoOsr * (mu + sine * p_photoTant * (caz * ecoti + s2ee) / ecee);
    }

    rr1 = p_photoWh / 4.0 * u0p / (u0p + up) * ((1.0 + bg) * pg -
        1.0 + Hfunc(u0p, gamma) * Hfunc(up, gamma));
    rr2 = up * munot / (up0 * u0p0 * p_photoSr * (1.0 - faz + faz * q));
    pht_hapkehen = rr1 * rr2;

    return pht_hapkehen;
  }
}

extern "C" Isis::PhotoModel *HapkeHenPlugin (Isis::Pvl &pvl) {
  return new Isis::HapkeHen(pvl);
}
