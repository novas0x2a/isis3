#include <cmath>
#include "Pvl.h"
#include "PhotoModel.h"
#include "NumericalMethods.h"
#include "Plugin.h"
#include "iException.h"
#include "Filename.h"

using namespace std;

namespace Isis {
  /**
   * Create a PhotoModel object.  Because this is a pure virtual
   * class you can not create a PhotoModel class directly.  Instead,
   * see the PhotoModelFactory class.
   *
   * @param pvl  A pvl object containing a valid PhotoModel specification
   *
   * @see photometricModels.doc
   */
  PhotoModel::PhotoModel (Pvl &pvl) {
    p_photoWh = 0.5;
    p_photoHg1 = 0.0;
    p_photoHg2 = 0.0;
    p_photoHh = 0.0;
    p_photoB0 = 0.0;
    p_photoTheta = 0.0;
    p_photoThetaold = -999.0;
    p_photoBh = 0.0;
    p_photoCh = 0.0;
    p_standardConditions = false;

    PvlGroup &algorithm = pvl.FindObject("PhotometricModel").FindGroup("Algorithm",Pvl::Traverse);

    if (algorithm.HasKeyword("Name")) {
      p_photoAlgorithmName = std::string(algorithm["Name"]);
    }

    if (algorithm.HasKeyword("Wh")) {
      SetPhotoWh(algorithm["Wh"]);
    }

    if (algorithm.HasKeyword("Hg1")) {
      SetPhotoHg1(algorithm["Hg1"]);
    }

    if (algorithm.HasKeyword("Hg2")) {
      SetPhotoHg2(algorithm["Hg2"]);
    }

    if (algorithm.HasKeyword("Hh")) {
      SetPhotoHh(algorithm["Hh"]);
    }

    if (algorithm.HasKeyword("B0")) {
      SetPhotoB0(algorithm["B0"]);
    }
    p_photoB0save = p_photoB0;

    if (algorithm.HasKeyword("Theta")) {
      SetPhotoTheta(algorithm["Theta"]);
    }

    if (algorithm.HasKeyword("Bh")) {
      SetPhotoBh(algorithm["Bh"]);
    }

    if (algorithm.HasKeyword("Ch")) {
      SetPhotoCh(algorithm["Ch"]);
    }
  }

  /**
    * Set the Hapke single scattering albedo component.
    * This parameter is limited to values that are >0 and
    * <=1.
    *
    * @param wh  Hapke single scattering albedo component, default is 0.5
    */
   void PhotoModel::SetPhotoWh (const double wh) {
     if (wh <= 0.0 || wh > 1.0) {
       std::string msg = "Invalid value of Hapke wh [" +
           iString(wh) + "]";
       throw iException::Message(iException::User,msg,_FILEINFO_);
     }
     p_photoWh = wh;
   }

 /**
   * Set the Hapke opposition surge component. This is one of 
   * two opposition surge components needed for the Hapke model. 
   * This parameter is limited to values that are >=0.
   *
   * @param hh  Hapke opposition surge component, default is 0.0
   */
  void PhotoModel::SetPhotoHh (const double hh) {
    if (hh < 0.0) {
      std::string msg = "Invalid value of Hapke hh [" +
          iString(hh) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_photoHh = hh;
  }

 /**
   * Set the Hapke opposition surge component. This is one of
   * two opposition surge components needed for the Hapke model.
   * This parameter is limited to values that are >=0.
   *
   * @param b0  Hapke opposition surge component, default is 0.0
   */
  void PhotoModel::SetPhotoB0 (const double b0) {
    if (b0 < 0.0) {
      std::string msg = "Invalid value of Hapke b0 [" +
          iString(b0) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_photoB0 = b0;
  }

 /**
   * Set the Hapke macroscopic roughness component. This parameter
   * is limited to values that are >=0 and <=90.
   *
   * @param theta  Hapke macroscopic roughness component, default is 0.0
   */
  void PhotoModel::SetPhotoTheta (const double theta) {
    if (theta < 0.0 || theta > 90.0) {
      std::string msg = "Invalid value of Hapke theta [" +
          iString(theta) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_photoTheta = theta;
  }

 /**
   * Set the Hapke Henyey Greenstein coefficient for the single
   * particle phase function. This is one of two coefficients
   * needed for the single particle phase function. This parameter
   * is limited to values that are >-1 and <1.
   *
   * @param hg1  Hapke Henyey Greenstein coefficient, default is 0.0
   */
  void PhotoModel::SetPhotoHg1 (const double hg1) {
    if (hg1 <= -1.0 || hg1 >= 1.0) {
      std::string msg = "Invalid value of Hapke Henyey Greenstein hg1 [" +
          iString(hg1) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_photoHg1 = hg1;
  }

 /**
   * Set the Hapke Henyey Greenstein coefficient for the single
   * particle phase function. This is one of two coefficients
   * needed for the single particle phase function. This parameter
   * is limited to values that are >=0 and <=1.
   *
   * @param hg2  Hapke Henyey Greenstein coefficient, default is 0.0
   */
  void PhotoModel::SetPhotoHg2 (const double hg2) {
    if (hg2 < 0.0 || hg2 > 1.0) {
      std::string msg = "Invalid value of Hapke Henyey Greenstein hg2 [" +
          iString(hg2) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_photoHg2 = hg2;
  }

 /**
   * Set the Hapke Legendre coefficient for the single
   * particle phase function. This is one of two coefficients
   * needed for the single particle phase function. This parameter
   * is limited to values that are >=-1 and <=1.
   *
   * @param bh  Hapke Legendre coefficient, default is 0.0
   */
  void PhotoModel::SetPhotoBh (const double bh) {
    if (bh < -1.0 || bh > 1.0) {
      std::string msg = "Invalid value of Hapke Legendre bh [" +
          iString(bh) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_photoBh = bh;
  }

 /**
   * Set the Hapke Legendre coefficient for the single
   * particle phase function. This is one of two coefficients
   * needed for the single particle phase function. This parameter
   * is limited to values that are >=-1 and <=1.
   *
   * @param ch  Hapke Legendre coefficient, default is 0.0
   */
  void PhotoModel::SetPhotoCh (const double ch) {
    if (ch < -1.0 || ch > 1.0) {
      std::string msg = "Invalid value of Hapke Legendre ch [" +
          iString(ch) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_photoCh = ch;
  }

  void PhotoModel::SetStandardConditions(bool standard) {
    p_standardConditions = standard;
    if (p_standardConditions) {
      p_photoB0save = p_photoB0;
      p_photoB0 = 0.0;
    } else {
      p_photoB0 = p_photoB0save;
    }
  }

  /**
   * Calculate the surface brightness using photometric angle information
   * 
   * @return  Returns the surface brightness calculated by the photometric
   *          function.
   *          
   */
  double PhotoModel::CalcSurfAlbedo(double pha, double inc, double ema) {

    // Check validity of photometric angles
    if (pha > 180.0 || inc > 90.0 || ema > 90.0 || pha < 0.0 ||
        inc < 0.0 || ema < 0.0) {
      std::string msg = "Invalid photometric angles";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    // Apply photometric function
    double albedo = PhotoModelAlgorithm(pha,inc,ema);
    return albedo;
  }

  /**
   * Obtain topographic derivative of an arbitrary photometric
   * function
   *
   * @param phase  input phase angle
   * @param incidence  input incidence angle
   * @param emission  input emission angle
   *
   */
  double PhotoModel::PhtTopder(double phase, double incidence,
      double emission)
  {
    double xi,zi;
    double cphi;
    double phi;
    double xe,ye,ze;
    double epsh;
    double xy,z;
    double cinc;
    double inc1,inc2,inc3,inc4;
    double cema;
    double ema1,ema2,ema3,ema4;
    double d1,d2;
    double result;
    double eps;

    eps = 0.04;

    // set up incidence vector
    xi = sin((Isis::PI/180.0)*incidence);
    zi = cos((Isis::PI/180.0)*incidence);

    // phi is the azimuth from xz plane to emission direction; if
    // either incidence or emission is zero, it's arbitrary and gets
    // set to zero. Thus cos(phi), cphi, is set to one.
    if ((incidence == 0.0) || (emission == 0.0)) {
      cphi = 1.0;
    }
    else {
      cphi = (cos((Isis::PI/180.0)*phase) -
              cos((Isis::PI/180.0)*incidence) *
	      cos((Isis::PI/180.0)*emission)) /
             (sin((Isis::PI/180.0)*incidence) *
	      sin((Isis::PI/180.0)*emission));
    }

    // now calculate the emission vector
    phi = NumericalMethods::PhtAcos(cphi) * (180.0/Isis::PI);
    xe = cphi * sin((Isis::PI/180.0)*emission);
    ye = sin((Isis::PI/180.0)*phi) * sin((Isis::PI/180.0)*emission);
    ze = cos((Isis::PI/180.0)*emission);

    // now evaluate two orthogonal derivatives
    epsh = eps * 0.5;
    xy = sin((Isis::PI/180.0)*epsh);
    z = cos((Isis::PI/180.0)*epsh);

    cinc = max(-1.0,min(xy*xi+z*zi,1.0));
    inc1 = NumericalMethods::PhtAcos(cinc) * (180.0/Isis::PI);
    cema = max(-1.0,min(xy*xe+z*ze,1.0));
    ema1 = NumericalMethods::PhtAcos(cema) * (180.0/Isis::PI);

    cinc = max(-1.0,min(-xy*xi+z*zi,1.0));
    inc2 = NumericalMethods::PhtAcos(cinc) * (180.0/Isis::PI);
    cema = max(-1.0,min(-xy*xe+z*ze,1.0));
    ema2 = NumericalMethods::PhtAcos(cema) * (180.0/Isis::PI);

    cinc = max(-1.0,min(z*zi,1.0));
    inc3 = NumericalMethods::PhtAcos(cinc) * (180.0/Isis::PI);
    cema = max(-1.0,min(xy*ye+z*ze,1.0));
    ema3 = NumericalMethods::PhtAcos(cema) * (180.0/Isis::PI);

    cinc = max(-1.0,min(z*zi,1.0));
    inc4 = NumericalMethods::PhtAcos(cinc) * (180.0/Isis::PI);
    cema = max(-1.0,min(-xy*ye+z*ze,1.0));
    ema4 = NumericalMethods::PhtAcos(cema) * (180.0/Isis::PI);

    d1 = (CalcSurfAlbedo(phase,inc1,ema1) - CalcSurfAlbedo(phase,inc2,ema2)) / eps;
    d2 = (CalcSurfAlbedo(phase,inc3,ema3) - CalcSurfAlbedo(phase,inc4,ema4)) / eps;

    //  Combine these two derivatives and return the gradient
    result = sqrt(max(1.0e-30,d1*d1+d2*d2));
    return result;
  }
}
