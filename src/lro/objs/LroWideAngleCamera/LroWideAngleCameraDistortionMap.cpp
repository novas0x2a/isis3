/**
 * @file
 * $Revision: 1.2 $
 * $Date: 2009/07/24 14:45:12 $
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
#include <cmath>

#include "iString.h"
#include "LroWideAngleCameraDistortionMap.h"

namespace Isis {
namespace Lro {
  /** Camera distortion map constructor
   *
   * Create a camera distortion map.  This class maps between distorted
   * and undistorted focal plane x/y's.  The default mapping is the
   * identity, that is, the focal plane x/y and undistorted focal plane
   * x/y will be identical.
   *
   * @param parent        the parent camera that will use this distortion map
   * @param zDirection    the direction of the focal plane Z-axis
   *                      (either 1 or -1)
   *
   */
  LroWideAngleCameraDistortionMap::LroWideAngleCameraDistortionMap(Camera *parent, int naifIkCode) : CameraDistortionMap(parent) {
//    std::string odkkey = "INS" + Isis::iString(naifIkCode) + "_DISTORTION_COEFFS";
//    for(int i = 0; i < 4; i++) {
//      p_odk.push_back(p_camera->GetDouble(odkkey, i));
//    }

    if(naifIkCode == -85621) {
      // vis
      //printf("VIS Distortion\n");
      p_k1 = -0.0099;
      p_k2 = -0.00050;
    }
    else {
      // uv
      //printf("UV Distortion\n");
      p_k1 = -0.024;
      p_k2 = -0.00070;
    }
    // TODO will need to change for UV
  }

  /** Compute undistorted focal plane x/y
   *
   * Compute undistorted focal plane x/y given a distorted focal plane x/y.
   *
   * @param dx distorted focal plane x in millimeters
   * @param dy distorted focal plane y in millimeters
   *
   * @return if the conversion was successful
   * @see SetDistortion
   * @todo Generalize polynomial equation
   */
  bool LroWideAngleCameraDistortionMap::SetFocalPlane(const double dx, const double dy) {
    p_focalPlaneX = dx;
    p_focalPlaneY = dy;

    // Get the distance from the focal plane center 
    // Compute R**2 and R**3 for the equation
    double radialDist = sqrt((dx * dx) + (dy * dy));
    double radialDist2 = radialDist * radialDist;
    double radialDist3 = radialDist * radialDist2;

    double fR = 1.0 + p_k1*radialDist2 + p_k2*radialDist3;
    if (fR == 0.0) return false;

    // Compute the undistorted positions
    p_undistortedFocalPlaneX = dx / fR; 
    p_undistortedFocalPlaneY = dy / fR; 

    return true;
  }

  /** Compute distorted focal plane x/y
   *
   * Compute distorted focal plane x/y given an undistorted focal plane x/y.
   *
   * @param ux undistorted focal plane x in millimeters
   * @param uy undistorted focal plane y in millimeters
   *
   * @return if the conversion was successful
   * @see SetDistortion
   * @todo Generalize polynomial equation
   * @todo Figure out a better solution for divergence condition
   */
  bool LroWideAngleCameraDistortionMap::SetUndistortedFocalPlane(const double ux,
                                                    const double uy) {
    // Get the distance from the focal plane center and if we are close
    // then skip the distortion
    p_undistortedFocalPlaneX = ux;
    p_undistortedFocalPlaneY = uy;

    double Ru = sqrt((ux * ux) + (uy *uy));
    if (Ru <= 1.0E-6) {
      p_focalPlaneX = ux;
      p_focalPlaneY = uy;
      return true;
    }

    // Estimate Rd 
    double Rd = Ru * 8.0 / 70.0;
    double delta = 1.0;
    int iter = 0;

    while(fabs(delta) > 1E-9) {
      if(fabs(delta) > 1E30 || iter > 50) {
        return false;
      }

      double Rd2 = Rd * Rd;
      double Rd3 = Rd2 * Rd;
      double temp = 1.0 + p_k1*Rd2 + p_k2*Rd3;

      double fRd = Rd / temp - Ru;

      // Use the quotient rule
      double derivefRd = (temp - Rd * (2.0*p_k1*Rd + 3.0*p_k2*Rd2)) / (temp * temp);

      delta = fRd / derivefRd;
      Rd = Rd - delta;

      iter ++;
    }

    double mult = Rd / Ru;
    p_focalPlaneX = ux * mult;
    p_focalPlaneY = uy * mult;

    return true;
  }

  // TODO:  Should we use this for LRO WAC to make the
  // convergence run faster??? or just delete?
  double LroWideAngleCameraDistortionMap::GuessDx(double uX) {
    // We're using natural log fits, but if uX < 1 the fit doesnt work
    if(fabs(uX) < 1) return uX;

    if(p_filter == 0) { // BLUE FILTER
      return (1.4101 * log(fabs(uX)));
    }

    else if(p_filter == 1) { // GREEN FILTER
      return (1.1039 * log(fabs(uX)));
    }

    else if(p_filter == 2) { // ORANGE FILTER
      return (0.8963 * log(fabs(uX)) + 2.1644);
    }

    else if(p_filter == 3) { // RED FILTER
      return (1.1039 * log(fabs(uX)));
    }

    else if(p_filter == 4) { // NIR FILTER
      return (1.4101 * log(fabs(uX)));
    }

    return uX;
  }
}}

