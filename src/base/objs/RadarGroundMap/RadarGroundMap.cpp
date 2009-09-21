/**
 * @file
 * $Revision: 1.1 $
 * $Date: 2009/07/09 16:39:33 $
 *
 *   Unless noted otherwise, the portions of Isis written by the USGS are
 *   public domain. See individual third-party library and package descriptions
 *   for intellectual property information, user agreements, and related
 *   information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or
 *   implied, is made by the USGS as to the accuracy and functioning of such
 *   software and related material nor shall the fact of distribution
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */
#include "RadarGroundMap.h"

namespace Isis {
  RadarGroundMap::RadarGroundMap(Camera *parent, Radar::LookDirection ldir,
    double waveLength) :
    CameraGroundMap(parent) {
    p_camera = parent;
    p_lookDirection = ldir;
    p_waveLength = waveLength;

    // Angluar tolerance based on radii and
    // slant range (Focal length)
    double radii[3];
    p_camera->Radii(radii);
//    p_tolerance = p_camera->FocalLength() / radii[2];
//    p_tolerance *= 180.0 / Isis::PI;
    p_tolerance = .00000001;

    // Compute a default time tolerance to a 1/20 of a pixel
    double et1 = p_camera->Spice::CacheStartTime();
    double et2 = p_camera->Spice::CacheEndTime();
    p_timeTolerance = (et2 - et1) / p_camera->Lines() / 20.0;
  }

  /** Compute ground position from slant range
   *
   * @param ux Slant range distance
   * @param uy Doppler shift (always 0.0)
   * @param uz Not used
   *
   * @return conversion was successful
   */
  bool RadarGroundMap::SetFocalPlane(const double ux, const double uy,
                                     double uz) {

    SpiceRotation *bodyFrame = p_camera->BodyRotation();
    SpicePosition *spaceCraft = p_camera->InstrumentPosition();

    // Get body fixed spacecraft velocity and position
    std::vector<double> Vsc = bodyFrame->ReferenceVector(spaceCraft->Velocity());
    std::vector<double> Xsc = bodyFrame->ReferenceVector(spaceCraft->Coordinate());

    // Compute intrack, crosstrack, and radial coordinate
    SpiceDouble i[3];
    vhat_c (&Vsc[0],i);

    SpiceDouble c[3];
    SpiceDouble dp;
    dp = vdot_c(&Xsc[0],i);
    SpiceDouble p[3],q[3];
    vscl_c(dp,i,p);
    vsub_c(&Xsc[0],p,q);
    vhat_c(q,c);

    SpiceDouble r[3];
    vcrss_c(i,c,r);

    // What is the initial guess for R
    double radii[3];
    p_camera->Radii(radii);
    SpiceDouble R = radii[0];
    SpiceDouble lastR = DBL_MAX;
    SpiceDouble rlat;
    SpiceDouble rlon;

    SpiceDouble lat = DBL_MAX;
    SpiceDouble lon = DBL_MAX;

    double slantRangeSqr = (ux * p_rangeSigma) / 1000.;
    slantRangeSqr = slantRangeSqr*slantRangeSqr;
    SpiceDouble X[3];

    do {
      double normXsc = vnorm_c(&Xsc[0]);
      double alpha = (R*R - slantRangeSqr - normXsc*normXsc) /
                     (2.0 * vdot_c(&Xsc[0],c));

      double arg = slantRangeSqr - alpha*alpha;
      if (arg < 0.0) return false;

      double beta = sqrt(arg);
      if (p_lookDirection == Radar::Left) beta *= -1.0;

      SpiceDouble alphac[3],betar[3];
      vscl_c(alpha,c,alphac);
      vscl_c(beta,r,betar);

      vadd_c(alphac,betar,alphac);
      vadd_c(&Xsc[0],alphac,X);

      // Convert X to lat,lon
      lastR = R;
      reclat_c(X,&R,&lon,&lat);

      rlat = lat*180.0/Isis::PI;
      rlon = lon*180.0/Isis::PI;
      R = GetRadius(rlat,rlon);
    }
    while (fabs(R-lastR) > p_tolerance);

    lat = lat*180.0/Isis::PI;
    lon = lon*180.0/Isis::PI;
    while (lon < 0.0) lon += 360.0;

    // Compute body fixed look direction
    std::vector<double> lookB;
    lookB.resize(3);
    lookB[0] = X[0] - Xsc[0];
    lookB[1] = X[1] - Xsc[1];
    lookB[2] = X[2] - Xsc[2];

    std::vector<double> lookJ = bodyFrame->J2000Vector(lookB);
    SpiceRotation *cameraFrame = p_camera->InstrumentRotation();
    std::vector<double> lookC = cameraFrame->ReferenceVector(lookJ);

    SpiceDouble unitLookC[3];
    vhat_c(&lookC[0],unitLookC);
    p_camera->SetLookDirection(unitLookC);

    return p_camera->Sensor::SetUniversalGround(lat,lon);
  }

  /** Compute undistorted focal plane coordinate from ground position
   *
   * @param lat planetocentric latitude in degrees
   * @param lon planetocentric longitude in degrees
   *
   * @return conversion was successful
   */
  bool RadarGroundMap::SetGround(const double lat, const double lon) {
    return SetGround(lat,lon,GetRadius(lat,lon));
  }

  /** Compute undistorted focal plane coordinate from ground position that includes a local radius
   *
   * @param lat planetocentric latitude in degrees
   * @param lon planetocentric longitude in degrees
   * @param radius local radius in meters
   *
   * @return conversion was successful
   */
  bool RadarGroundMap::SetGround(const double lat, const double lon, const double radius) {
    // Get the ground point in rectangular coordinates (X)
    SpiceDouble X[3];
    SpiceDouble rlat = lat*Isis::PI/180.0;
    SpiceDouble rlon = lon*Isis::PI/180.0;
    latrec_c(radius,rlon,rlat,X);

    // Compute lower bound for Doppler shift 
    double et1 = p_camera->Spice::CacheStartTime();
    p_camera->Sensor::SetEphemerisTime(et1);
    double xv1 = ComputeXv(X);

    // Compute upper bound for Doppler shift
    double et2 = p_camera->Spice::CacheEndTime();
    p_camera->Sensor::SetEphemerisTime(et2);
    double xv2 = ComputeXv(X);

    // Make sure we bound root (xv = 0.0)
    if ((xv1 < 0.0) && (xv2 < 0.0)) return false;
    if ((xv1 > 0.0) && (xv2 > 0.0)) return false;

    // Order the bounds
    double fl,fh,xl,xh;
    if (xv1 < xv2) {
      fl = xv1;
      fh = xv2;
      xl = et1;
      xh = et2;
    }
    else {
      fl = xv2;
      fh = xv1;
      xl = et2;
      xh = et1;
    }

    // Iterate a max of 30 times
    for (int j=0; j<30; j++) {
      // Use the secant method to guess the next et
      double etGuess = xl + (xh - xl) * fl / (fl - fh);

      // Compute the guessed Doppler shift.  Hopefully
      // this guess converges to zero at some point
      p_camera->Sensor::SetEphemerisTime(etGuess);
      double fGuess = ComputeXv(X);

      // Update the bounds
      double delTime;
      if (fGuess < 0.0) {
        delTime = xl - etGuess;
        xl = etGuess;
        fl = fGuess;
      }
      else {
        delTime = xh - etGuess;
        xh = etGuess;
        fh = fGuess;
      }

      // See if we are done
      if ((fabs(delTime) <= p_timeTolerance) || (fGuess == 0.0)) {
        SpiceRotation *bodyFrame = p_camera->BodyRotation();
        SpicePosition *spaceCraft = p_camera->InstrumentPosition();

        // Get body fixed spacecraft velocity and position
        std::vector<double> Vsc = bodyFrame->ReferenceVector(spaceCraft->Velocity());
        std::vector<double> Xsc = bodyFrame->ReferenceVector(spaceCraft->Coordinate());

        // Compute body fixed look direction
        std::vector<double> lookB;
        lookB.resize(3);
        lookB[0] = X[0] - Xsc[0];
        lookB[1] = X[1] - Xsc[1];
        lookB[2] = X[2] - Xsc[2];

        std::vector<double> lookJ = bodyFrame->J2000Vector(lookB);
        SpiceRotation *cameraFrame = p_camera->InstrumentRotation();
        std::vector<double> lookC = cameraFrame->ReferenceVector(lookJ);

        SpiceDouble unitLookC[3];
        vhat_c(&lookC[0],unitLookC);
        p_camera->SetLookDirection(unitLookC);

        p_camera->SetFocalLength(p_slantRange*1000.0);
        p_focalPlaneX = p_slantRange / p_rangeSigma;
        p_focalPlaneY = 0.0;
        return true;
      }
    }

    return false;
  }


  double RadarGroundMap::ComputeXv(SpiceDouble X[3]) {
    // Get the spacecraft position (Xsc) and velocity (Vsc) in body fixed
    // coordinates
    SpiceRotation *bodyFrame = p_camera->BodyRotation();
    SpicePosition *spaceCraft = p_camera->InstrumentPosition();
    std::vector<double> Vsc = bodyFrame->ReferenceVector(spaceCraft->Velocity());
    std::vector<double> Xsc = bodyFrame->ReferenceVector(spaceCraft->Coordinate());

    // Compute the slant range
    SpiceDouble lookB[3];
    vsub_c(&Xsc[0],X,lookB);
    p_slantRange = vnorm_c(lookB);

    // Compute and return xv
    double xv = -2.0 * vdot_c(lookB,&Vsc[0]) / (vnorm_c(lookB) * p_waveLength);
    return xv;
  }


  double RadarGroundMap::GetRadius(double lat, double lon) {
    if (p_camera->HasElevationModel()) {
      return p_camera->DemRadius(lat,lon);
    }

    double radii[3];
    p_camera->Radii(radii);
    double a = radii[0];
    double b = radii[1];
    double c = radii[2];
    double xyradius = a * b / sqrt(pow(b*cos(lon),2) + pow(a*sin(lon),2) );
    return xyradius * c / sqrt(pow(c*cos(lat),2) + pow(xyradius*sin(lat),2) );
  }

  /** Set the weight factors for slant range and Doppler shift
   *
   */
  void RadarGroundMap::SetWeightFactors(double range_sigma, double doppler_sigma) {
    p_rangeSigma = range_sigma;
    p_dopplerSigma = doppler_sigma;
  }
}
