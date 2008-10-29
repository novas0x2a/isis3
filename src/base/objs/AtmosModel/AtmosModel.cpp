#include <cmath>
#include <string>
#include "Pvl.h"
#include "iString.h"
#include "AtmosModel.h"
#include "NumericalMethods.h"
#include "PhotoModel.h"
#include "Minnaert.h"
#include "LunarLambert.h"
#include "Plugin.h"
#include "iException.h"
#include "Filename.h"

using namespace std;

namespace Isis {
  /**
   * Create an AtmosModel object.  Because this is a pure virtual 
   * class you can not create an AtmosModel class directly.  Instead, 
   * see the AtmosModelFactory class.
   * 
   * @param pvl  A pvl object containing a valid AtmosModel specification 
   * @param pmodel A PhotoModel object containing valud AtmosModel data 
   * 
   * @see atmosphericModels.doc
   */
  AtmosModel::AtmosModel (Pvl &pvl, PhotoModel &pmodel) {
    p_atmosAlgorithmName = "Unknown";
    p_atmosPM = &pmodel;

    p_atmosAb = 0.0;
    p_atmosCosphi = 0.0;
    p_atmosAtmSwitch = 0;
    p_atmosEulgam = 0.5772156;
    p_atmosHahgsb = 0.0;
    p_atmosHga = 0.68;
    p_atmosInc = 0.0;
    p_atmosMunot = 0.0;
    p_atmosNinc = 91;
    p_atmosPhi = 0.0;
    p_atmosSini = 0.0;
    p_atmosTau = 0.28;
    p_atmosTauref = 0.0;
    p_atmosTauold = -1.0;
    p_atmosWha = 0.95;
    p_atmosWhaold = 1.0e30;
    p_atmosBha = 0.85;
    p_pstd = 0.0;
    p_sbar = 0.0;
    p_trans = 0.0;
    p_trans0 = 0.0;
    p_standardConditions = false;

    for(unsigned int i = 0; i < sizeof(p_atmosIncTable) / sizeof(double); i ++) {
      p_atmosIncTable[i] = 0.0;
    }

    for(unsigned int i = 0; i < sizeof(p_atmosAhTable) / sizeof(double); i ++) {
      p_atmosAhTable[i] = 0.0;
      p_atmosAhTable2[i] = 0.0;
    }

    for(unsigned int i = 0; i < sizeof(p_atmosHahgtTable) / sizeof(double); i ++) {
      p_atmosHahgtTable[i] = 0.0;
      p_atmosHahgtTable2[i] = 0.0;
    }

    for(unsigned int i = 0; i < sizeof(p_atmosHahgt0Table) / sizeof(double); i ++) {
      p_atmosHahgt0Table[i] = 0.0;
      p_atmosHahgt0Table2[i] = 0.0;
    }

    PvlGroup &algorithm = pvl.FindObject("AtmosphericModel").FindGroup("Algorithm",Pvl::Traverse);

    if (algorithm.HasKeyword("Nulneg")) {
      SetAtmosNulneg( ((std::string)algorithm["Nulneg"]).compare("YES") == 0 );
    }
    else {
      p_atmosNulneg = false;
    }

    if (algorithm.HasKeyword("Tau")) {
      SetAtmosTau( algorithm["Tau"] );
    }
    p_atmosTausave = p_atmosTau;

    if (algorithm.HasKeyword("Tauref")) {
      SetAtmosTauref( algorithm["Tauref"] );
    }

    if (algorithm.HasKeyword("Wha")) {
      SetAtmosWha( algorithm["Wha"] );
    }
    p_atmosWhasave = p_atmosWha;

    if (algorithm.HasKeyword("Wharef")) {
      SetAtmosWharef( algorithm["Wharef"] );
    } else {
      p_atmosWharef = p_atmosWha;
    }

    if (algorithm.HasKeyword("Hga")) {
      SetAtmosHga( algorithm["Hga"] );
    }
    p_atmosHgasave = p_atmosHga;

    if (algorithm.HasKeyword("Hgaref")) {
      SetAtmosHgaref( algorithm["Hgaref"] );
    } else {
      p_atmosHgaref = p_atmosHga;
    }

    if (algorithm.HasKeyword("Bha")) {
      SetAtmosBha( algorithm["Bha"] );
    }
    p_atmosBhasave = p_atmosBha;

    if (algorithm.HasKeyword("Bharef")) {
      SetAtmosBharef( algorithm["Bharef"] );
    } else {
      p_atmosBharef = p_atmosBha;
    }

    if (algorithm.HasKeyword("Inc")) {
      SetAtmosInc( algorithm["Inc"] );
    }

    if (algorithm.HasKeyword("Phi")) {
      SetAtmosPhi( algorithm["Phi"] );
    }
  }

  /**
   * Set the Atmospheric function parameter. This determines
   * if negative values after removal of atmospheric effects
   * will be set to NULL. This parameter is limited to values
   * of YES or NO.
   *
   * @param nulneg  Atmospheric function parameter, default is NO
   */
  void AtmosModel::SetAtmosNulneg (const std::string nulneg) {
    Isis::iString temp(nulneg);
    temp = temp.UpCase();

    if (temp != "NO" && temp != "YES") {
      std::string msg = "Invalid value of Atmospheric nulneg [" + temp + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    SetAtmosNulneg( temp.compare("YES") == 0 );
  }

  /**
   * Calculate the atmospheric scattering effect using photometric angle 
   * information
   * 
   * @param pha  phase angle
   * @param inc  incidence angle
   * @param ema  emission angle
   *          
   */
  void AtmosModel::CalcAtmEffect(double pha, double inc, double ema, 
        double *pstd, double *trans, double *trans0, double *sbar) {

    // Check validity of photometric angles
    if (pha > 180.0 || inc > 90.0 || ema > 90.0 || pha < 0.0 ||
        inc < 0.0 || ema < 0.0) {
      std::string msg = "Invalid photometric angles";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    // Apply atmospheric function
    AtmosModelAlgorithm(pha,inc,ema);
    *pstd = p_pstd;
    *trans = p_trans;
    *trans0 = p_trans0;
    *sbar = p_sbar;
  }

  void AtmosModel::SetStandardConditions(bool standard) {
    p_standardConditions = standard;
    if (p_standardConditions) {
      p_atmosBhasave = p_atmosBha;
      p_atmosBha = p_atmosBharef;
      p_atmosHgasave = p_atmosHga;
      p_atmosHga = p_atmosHgaref;
      p_atmosTausave = p_atmosTau;
      p_atmosTau = p_atmosTauref;
      p_atmosWhasave = p_atmosWha;
      p_atmosWha = p_atmosWharef;
    } else {
      p_atmosBha = p_atmosBhasave;
      p_atmosHga = p_atmosHgasave;
      p_atmosTau = p_atmosTausave;
      p_atmosWha = p_atmosWhasave;
    }
  }

  /**
   * Obtain the hemispheric albedo by integrating the photometric
   * function times mu over mu = 0 to 1 and then over phi = 0 to
   * 2*pi to calculate the hemispheric reflectance Ah needed for
   * the photometric model with atmosphere. The Trapezoid rule is
   * applied to the table of Ah to obtain bihemispheric albedo
   * Ab.
   *
   */
  void AtmosModel::PhtGetAhTable() {
    int inccnt;
    double fun_temp;
    double factor;
    double yp1,yp2;
    int sub;

    sub = 0;

    p_atmosAb = 0.0;
    for (inccnt=0; inccnt<p_atmosNinc; inccnt++) {
      p_atmosInc = (double) inccnt;
      p_atmosIncTable[inccnt] = p_atmosInc;
      p_atmosMunot = cos((Isis::PI/180.0)*p_atmosInc);
      p_atmosSini = sin((Isis::PI/180.0)*p_atmosInc);

      Isis::iString phtName = p_atmosPM->AlgorithmName();
      phtName.UpCase();
      if (p_atmosInc == 90.0) {
        p_atmosAhTable[inccnt] = 0.0;
      }
      else if (phtName == "LAMBERT") {
        p_atmosAhTable[inccnt] = 1.0;
      }
      else if (phtName == "LOMMELSEELIGER") {
        p_atmosAhTable[inccnt] = 2.0 * log((1.0/p_atmosMunot)/p_atmosMunot);
      }
      else if (phtName == "MINNAERT") {
        p_atmosAhTable[inccnt] = (pow(p_atmosMunot,((Minnaert*)p_atmosPM)->PhotoK()))/((Minnaert*)p_atmosPM)->PhotoK();
      }
      else if (phtName == "LUNARLAMBERT") {
        p_atmosAhTable[inccnt] = 2.0 * ((LunarLambert*)p_atmosPM)->PhotoL() * 
                                 log((1.0+p_atmosMunot)/p_atmosMunot) + 1.0 - 
                                 ((LunarLambert*)p_atmosPM)->PhotoL();
      }
      else {
        p_atmosAtmSwitch = 0;
        r8qromb(sub,0.0,180.0,&fun_temp);
        p_atmosAhTable[inccnt] = fun_temp / (90.0*p_atmosMunot);
      }

      if ((inccnt == 0) || (inccnt == p_atmosNinc-1)) {
        factor = 0.5;
      }
      else {
        factor = 1.0;
      }

      p_atmosAb = p_atmosAb + p_atmosAhTable[inccnt] * p_atmosMunot * p_atmosSini * factor;
    }

    factor = 2.0 * Isis::PI/180.0;
    p_atmosAb = p_atmosAb * factor;

    yp1 = 1.0e30;
    yp2 = 1.0e30;
    NumericalMethods::r8spline(p_atmosIncTable,p_atmosAhTable,p_atmosNinc,yp1,yp2,p_atmosAhTable2);
  }

  /**
   * Integrate functions involving the single particle phase
   * function (assumed to be Hapke Henyey-Greenstein) over
   * mu = 0 to 1 and then over phi = 0 to 2*pi to calculate the
   * corrections needed for the anisotropic photometric model with
   * atmosphere. The Trapezoid rule is applied to the table of
   * Ah to obtain bihemispheric albedo Ab.
   *
   */
  void AtmosModel::GetHahgTables() {
    int inccnt;
    double fun_temp;
    double hahgsb_temp;
    double factor;
    double yp1,yp2;
    int sub;

    sub = 0;

    p_atmosHahgsb = 0.0;
    for (inccnt = 0; inccnt < p_atmosNinc; inccnt++) {
      p_atmosInc = (double) inccnt;
      p_atmosIncTable[inccnt] = p_atmosInc;
      p_atmosMunot = cos((Isis::PI/180.0)*p_atmosInc);
      p_atmosSini = sin((Isis::PI/180.0)*p_atmosInc);

      p_atmosAtmSwitch = 1;
      r8qromb(sub,0.0,180.0,&fun_temp);
      p_atmosHahgtTable[inccnt] = fun_temp * AtmosWha() / 360.0;
      p_atmosAtmSwitch = 2;
      r8qromb(sub,0.0,180.0,&fun_temp);
      hahgsb_temp = fun_temp * AtmosWha() / 360.0;

      if ((inccnt == 0) || (inccnt == p_atmosNinc-1)) {
        factor = 0.5;
      }
      else {
        factor = 1.0;
      }

      p_atmosHahgsb = p_atmosHahgsb + p_atmosSini * factor * hahgsb_temp;
      if (p_atmosInc == 0.0) {
        p_atmosHahgt0Table[inccnt] = 0.0;
      }
      else {
        p_atmosAtmSwitch = 3;
        r8qromb(sub,0.0,180.0,&fun_temp);
        p_atmosHahgt0Table[inccnt] = fun_temp * AtmosWha() * p_atmosMunot / (360.0 * p_atmosSini);
      }
    }

    factor = 2.0 * Isis::PI/180.0;
    p_atmosHahgsb = p_atmosHahgsb * factor;

    yp1 = 1.0e30;
    yp2 = 1.0e30;
    NumericalMethods::r8spline(p_atmosIncTable,p_atmosHahgtTable,p_atmosNinc,yp1,yp2,p_atmosHahgtTable2);
    yp1 = 1.0e30;
    yp2 = 1.0e30;
    NumericalMethods::r8spline(p_atmosIncTable,p_atmosHahgt0Table,p_atmosNinc,yp1,yp2,p_atmosHahgt0Table2);
  }

  /**
   * Integrate a function
   *
   * @param phi  angle at which the function will be integrated
   *
   */
  double AtmosModel::OutrFunc2Bint(double phi) {
    double result;
    int sub;

    sub = 1;

    p_atmosPhi = phi;
    p_atmosCosphi = cos((Isis::PI/180.0)*phi);
    r8qromb(sub,1.0e-6,1.0,&result);
    return result;
  }

  /**
   * Integrate a function
   *
   * @param mu  angle at which the function will be integrated
   *
   */
  double AtmosModel::InrFunc2Bint(double mu) {
    double ema;
    double sine;
    double alpha;
    double phase;
    double result;
    double phasefn;
    double xx;
    double emunot;
    double emu;
    double tfac;

    ema = acos(mu) * (180.0/Isis::PI);
    sine = sin(ema*(Isis::PI/180.0));

    if ((p_atmosAtmSwitch == 0) || (p_atmosAtmSwitch == 2)) {
      alpha = p_atmosSini * sine * p_atmosCosphi +
          p_atmosMunot * mu;
    }
    else {
      alpha = p_atmosSini * sine * p_atmosCosphi -
          p_atmosMunot * mu;
    }

    phase = acos(alpha) * (180.0/Isis::PI);

    if (p_atmosAtmSwitch == 0) {
      result = mu * p_atmosPM->CalcSurfAlbedo(phase,p_atmosInc,ema);
    }
    else {
      phasefn = (1.0 - AtmosHga() * AtmosHga()) /pow((1.0+2.0*AtmosHga()*alpha+AtmosHga()*AtmosHga()),1.5);

      xx = -AtmosTau() / max(p_atmosMunot,1.0e-30);
      if (xx < -69.0) {
        emunot = 0.0;
      }
      else if (xx > 69.0) {
        emunot = 1.0e30;
      }
      else {
        emunot = exp(xx);
      }

      xx = -AtmosTau() / max(mu,1.0e-30);

      if (xx < -69.0) {
        emu = 0.0;
      }
      else if (xx > 69.0) {
        emu = 1.0e30;
      }
      else {
        emu = exp(xx);
      }

      if (mu == p_atmosMunot) {
        tfac = AtmosTau() * emunot / (p_atmosMunot * p_atmosMunot);
      }
      else {
        tfac = (emunot - emu) / (p_atmosMunot - mu);
      }

      if (p_atmosAtmSwitch == 1) {
        result = mu * (phasefn - 1.0) * tfac;
      }
      else if (p_atmosAtmSwitch == 2) {
        result = p_atmosMunot * mu * (phasefn - 1.0) * (1.0 - emunot * emu) / (p_atmosMunot + mu);
      }
      else if (p_atmosAtmSwitch == 3) {
        result = -sine * p_atmosCosphi * (phasefn - 1.0) * tfac;
      }
      else {
        std::string msg = "Invalid value of atmospheric ";
        msg += "switch used as argument to this function";
        throw iException::Message(iException::Programmer,msg,_FILEINFO_);
      }
    }

    return result;
  }

  /**
   * Determine the integral of a function between 2 points.
   * 
   * @param a  Lower limit of integration
   * @param b  Upper limit of integration
   * @param ss  Will contain the integral of the function on return
   * 
   */
  void AtmosModel::r8qromb (int sub, double a, double b,
      double *ss)
  {
    double dss;
    double h[21];
    double s[21];
    double eps;
    double epsa;

    eps = 1.0e-4;
    epsa = 1.0e-6;

    h[0] = 1.0;
    for (int i=0; i<20; i++) {
      r8trapzd(sub,a,b,&s[i],i+1);
      if (i >= 4) {
        NumericalMethods::r8polint(&h[i-4],&s[i-4],5,0.0,ss,&dss);

        if (fabs(dss) <= eps*fabs(*ss)) return;
        if (fabs(dss) <= epsa) return;
      }

      s[i+1] = s[i];
      h[i+1] = 0.25 * h[i];
    }
  }

  /**
   * Determine the nth stage of refinement of an extended
   * trapezoidal rule.
   * 
   * @param a  Lower limit of integration
   * @param b  Upper limit of integration
   * @param s  Will contain the integral of the function on return
   * @param n  Number of partitions to use when integrating
   *
   */
  void AtmosModel::r8trapzd (int sub, double a, double b, double *s, int n)
  {
    double begin;
    double end;
    int it;
    double del,tnm,x,sum;

    if (n == 1) {
      if (sub) {
        begin = InrFunc2Bint(a);
      }
      else {
        begin = OutrFunc2Bint(a);
      }

      if (sub) {
        end = InrFunc2Bint(b);
      }
      else {
        end = OutrFunc2Bint(b);
      }

      *s = 0.5 * (b - a) * (begin + end);
    }
    else {
      it = (int)(pow(2.0,(double)(n-2)));
      tnm = it;
      del = (b - a) / tnm;
      x = a + 0.5 * del;
      sum = 0.0;
      for (int i=0; i<it; i++) {
        if (sub) {
          sum = sum + InrFunc2Bint(x);
        }
        else {
          sum = sum + OutrFunc2Bint(x);
        }

        x = x + del;
      }

      *s = 0.5 * (*s + (b - a) * sum / tnm);
    }
  }

  /**
   * Set the Atmospheric function parameter. This specifies the
   * normal optical depth of the atmosphere. This parameter is
   * limited to values that are >=0.
   *
   * @param tau  Atmospheric function parameter, default is 0.28
   */
  void AtmosModel::SetAtmosTau (const double tau) {
    if (tau < 0.0) {
      std::string msg = "Invalid value of Atmospheric tau [" + iString(tau) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosTau = tau;
  }

  /**
   * Set the Atmospheric function parameter. This specifies the
   * reference optical depth of the atmosphere to which the image
   * will be normalized. This parameter is limited to values that
   * are >=0.
   *
   * @param tauref  Atmospheric function parameter, default is 0.0
   */
  void AtmosModel::SetAtmosTauref (const double tauref) {
    if (tauref < 0.0) {
      std::string msg = "Invalid value of Atmospheric tauref [" + iString(tauref) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosTauref = tauref;
  }

  /**
   * Set the Atmospheric function parameter. This is the single-
   * scattering albedo of atmospheric particles. This parameter
   * is limited to values that are >0 and <=1.
   *
   * @param wha  Atmospheric function parameter, default is 0.95
   */
  void AtmosModel::SetAtmosWha (const double wha) {
    if (wha <= 0.0 || wha > 1.0) {
      std::string msg = "Invalid value of Atmospheric wha [" + iString(wha) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosWha = wha;
  }

  /**
   * Set the Atmospheric function parameter. This is the reference
   * single-scattering albedo of atmospheric particles to which
   * the image will be normalized. This parameter is limited to
   * values that are >0 and <=1.
   *
   * @param wharef  Atmospheric function parameter, no default 
   */
  void AtmosModel::SetAtmosWharef (const double wharef) {
    if (wharef <= 0.0 || wharef > 1.0) {
      std::string msg = "Invalid value of Atmospheric wharef [" + iString(wharef) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosWharef = wharef;
  }

  /**
   * Set the Hapke atmospheric function parameter. This is the
   * coefficient of the single particle Henyey-Greenstein phase
   * function. This parameter is limited to values that are >-1
   * and <1.
   *
   * @param hga  Hapke atmospheric function parameter,
   *             default is 0.68
   */
  void AtmosModel::SetAtmosHga (const double hga) {
    if (hga <= -1.0 || hga >= 1.0) {
      std::string msg = "Invalid value of Hapke atmospheric hga [" + iString(hga) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosHga = hga;
  }

  /**
   * Set the Atmospheric function parameter. This specifies the
   * reference value of Hga to which the image will be normalized. 
   * If no value is given, then this parameter defaults to the
   * value provided for Hga. This parameter is limited to values
   * that are >-1 and <1.
   *
   * @param hgaref  Hapke atmospheric function parameter, no default 
   */
  void AtmosModel::SetAtmosHgaref (const double hgaref) {
    if (hgaref <= -1.0 || hgaref >= 1.0) {
      std::string msg = "Invalid value of Atmospheric hgaref [" + iString(hgaref) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosHgaref = hgaref;
  }

  /**
   * Set the Anisotropic Atmospheric function parameter. This is
   * the coefficient of the single particle Legendre phase
   * function. This parameter is limited to values that are >=-1
   * and <=1.
   *
   * @param bha  Anisotropic atmospheric function parameter,
   *             default is 0.85
   */
  void AtmosModel::SetAtmosBha (const double bha) {
    if (bha < -1.0 || bha > 1.0) {
      std::string msg = "Invalid value of Anisotropic atmospheric bha [" +
                        iString(bha) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosBha = bha;
  }

  /**
   * Set the Atmospheric function parameter. This specifies the
   * reference value of Bha to which the image will be normalized. 
   * If no value is given, then this parameter defaults to the
   * value provided for Bha. This parameter is limited to values
   * that are >=-1 and <=1.
   *
   * @param bharef  Atmospheric function parameter, no default 
   */
  void AtmosModel::SetAtmosBharef (const double bharef) {
    if (bharef < -1.0 || bharef > 1.0) {
      std::string msg = "Invalid value of Atmospheric bharef [" + iString(bharef) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosBharef = bharef;
  }

  /**
   * Set the switch that controls the function that will be 
   * integrated. This method is only used for testing the 
   * methods in this class. This parameter is limited to the 
   * values 0, 1, 2, and 3.
   *
   * @param atmswitch  Internal atmospheric function parameter,
   *                   there is no default
   */
  void AtmosModel::SetAtmosAtmSwitch (const int atmswitch) {
    if (atmswitch < 0 || atmswitch > 3) {
      std::string msg = "Invalid value of atmospheric atmswitch [" + iString(atmswitch) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosAtmSwitch = atmswitch;
  }



  /**
   * Set the incidence angle. This method is only used for
   * testing the methods in this class. This parameter is
   * limited to values >=0 and <=90.
   *
   * @param inc  Internal atmospheric function parameter,
   *             there is no default
   */
  void AtmosModel::SetAtmosInc (const double inc) {
    if (inc < 0.0 || inc > 90.0) {
      std::string msg = "Invalid value of atmospheric inc [" + iString(inc) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosInc = inc;
    p_atmosMunot = cos(inc*Isis::PI/180.0);
    p_atmosSini = sin(inc*Isis::PI/180.0);
  }

  /**
   * Set the azimuth angle. This method is only used for
   * testing the methods in this class. This parameter is
   * limited to values >=0 and <=360.
   *
   * @param phi  Internal atmospheric function parameter,
   *             there is no default
   */
  void AtmosModel::SetAtmosPhi (const double phi) {
    if (phi < 0.0 || phi > 360.0) {
      std::string msg = "Invalid value of atmospheric phi [" + iString(phi) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_atmosPhi = phi;
    p_atmosCosphi = cos(phi*Isis::PI/180.0);
  }

  bool AtmosModel::TauOrWhaChanged() const {
    return (AtmosTau() != p_atmosTauold) || (AtmosWha() != p_atmosWhaold);
  }
}
