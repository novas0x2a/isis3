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

#ifndef RadarGroundMap_h
#define RadarGroundMap_h

#include "Camera.h"
#include "CameraGroundMap.h"

namespace Isis {

#ifndef RADAR_LOOK_DIR
namespace Radar {
  enum LookDirection { Left, Right };
}
#define RADAR_LOOK_DIR
#endif

  /** Convert between undistorted focal plane coordinate (slant range) 
   *  and ground coordinates
   *
   * This class is used to convert between undistorted focal plane
   * coordinate (the slant range) and ground coordinates lat/lon. This
   * class handles the case of Radar instruments.
   *
   * @ingroup Camera
   *
   * @see Camera
   *
   * @internal
   *
   * @history 2008-06-16 Jeff Anderson
   * Original version
   *
   *  @history 2009-07-01 Janet Barrett - Fixed intrack, crosstrack, and radial
   *                      coordinate calculations; changed boundary check to use
   *                      radius instead of lat,lon; updated calculation of Doppler
   *                      shift
   *
   */
  class RadarGroundMap : public CameraGroundMap {
    public:
      RadarGroundMap(Camera *parent, Radar::LookDirection ldir, double waveLength);

      //! Destructor
      virtual ~RadarGroundMap() {};

      virtual bool SetFocalPlane(const double ux, const double uy,
                                 const double uz);

      virtual bool SetGround(const double lat, const double lon);
      virtual bool SetGround(const double lat, const double lon, const double radius);

      void SetWeightFactors(double range_sigma, double doppler_sigma);

    private:
      double ComputeXv(SpiceDouble X[3]);
      double GetRadius(double lat, double lon);

      Radar::LookDirection p_lookDirection;
      double p_tolerance;
      double p_slantRange;
      double p_timeTolerance;

      double p_rangeSigma;
      double p_dopplerSigma;
      double p_waveLength;
      Camera *p_camera;
  };
};
#endif
