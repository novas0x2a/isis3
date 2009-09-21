#include <cmath>

#include "iString.h"
#include "LroNarrowAngleDistortionMap.h"

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
  LroNarrowAngleDistortionMap::LroNarrowAngleDistortionMap(Camera *parent) : CameraDistortionMap(parent, 1) {
  }

  void LroNarrowAngleDistortionMap::SetDistortion(const int naifIkCode) {
	  std::string odkkey = "INS" + Isis::iString(naifIkCode) + "_OD_K";
	  p_odk.clear();
	  p_odk.push_back(p_camera->GetDouble(odkkey, 0));
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
  bool LroNarrowAngleDistortionMap::SetFocalPlane(const double dx, const double dy) {
    p_focalPlaneX = dx;
    p_focalPlaneY = dy;

    // Get the distance from the focal plane center and if we are close
    // then skip the distortion
    double radialDist2 = (dx * dx) + (dy * dy);

    if (radialDist2 <= 1.0E-3) {
      p_undistortedFocalPlaneX = dx;
      p_undistortedFocalPlaneY = dy;
      return true;
    }

    p_undistortedFocalPlaneX = dx/(1+p_odk[0]*radialDist2);
    p_undistortedFocalPlaneY = dy/(1+p_odk[0]*radialDist2);

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
   */
  bool LroNarrowAngleDistortionMap::SetUndistortedFocalPlane(const double ux,
                                                    const double uy) {
    p_undistortedFocalPlaneX = ux;
    p_undistortedFocalPlaneY = uy;

    // Get the distance from the focal plane center and if we are close
    // then skip the distortion
    double radialDist2 = (ux * ux) + (uy *uy);

    if (radialDist2 <= 1.0E-3) {
      p_focalPlaneX = ux;
      p_focalPlaneY = uy;
      return true;
    }

    p_focalPlaneX = ux * (1+p_odk[0]*radialDist2);
    p_focalPlaneY = uy * (1+p_odk[0]*radialDist2);

    return true;
  }
}
}

