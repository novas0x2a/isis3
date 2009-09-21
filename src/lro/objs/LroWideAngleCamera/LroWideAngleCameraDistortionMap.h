/**
 * @file
 * $Revision: 1.1 $
 * $Date: 2009/07/08 16:59:49 $
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
#ifndef LroWideAngleCameraDistortionMap_h
#define LroWideAngleCameraDistortionMap_h

#include "CameraDistortionMap.h"

namespace Isis {
  namespace Lro {
  /** Distort/undistort focal plane coordinates
   *
   * Creates a map for adding/removing optical distortions
   * from the focal plane of a camera.
   *
   * @ingroup Camera
   *
   * @see Camera
   *
   * @author 2008-08-22 Steven Lambright
   *
   * @internal
   */
    class LroWideAngleCameraDistortionMap : public CameraDistortionMap {
    public:
      LroWideAngleCameraDistortionMap(Camera *parent, int naifIkCode);

      //! Destructor
      virtual ~LroWideAngleCameraDistortionMap() {};

      virtual bool SetFocalPlane(const double dx, const double dy);

      virtual bool SetUndistortedFocalPlane(const double ux, const double uy);

      void SetFilter(int filter) { p_filter = filter; }

    private:
      double GuessDx(double uX);
      int p_filter;
      double p_k1;
      double p_k2;
    };
  };
};
#endif
