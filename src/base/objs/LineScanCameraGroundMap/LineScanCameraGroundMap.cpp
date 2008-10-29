/**
 * @file
 * $Revision: 1.2 $
 * $Date: 2007/12/21 23:31:50 $
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

#include "LineScanCameraGroundMap.h"

#include "CameraDistortionMap.h"
#include "LineScanCameraDetectorMap.h"
#include "CameraFocalPlaneMap.h"

namespace Isis {
  /** Compute undistorted focal plane coordinate from ground position
   *
   * @param lat planetocentric latitude in degrees
   * @param lon planetocentric longitude in degrees
   *
   * @return conversion was successful
   */
  bool LineScanCameraGroundMap::SetGround(const double lat, const double lon) {
    // Set a time in order to get Spice so SetUniversalGround will not fail
    // We are only interested in getting the radius for this lat/lon and
    // calling the overload SetGround to do the work.
    p_camera->Sensor::SetEphemerisTime(p_camera->Spice::CacheStartTime());
    if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return false;
    // Get radius in meters
    double radius = p_camera->Sensor::LocalRadius();
    return SetGround (lat, lon, radius);
  }

  /** Compute undistorted focal plane coordinate from ground position
   *
   * @param lat planetocentric latitude in degrees
   * @param lon planetocentric longitude in degrees
   * @param radius local radius in meters
   *
   * @return conversion was successful
   */
  bool LineScanCameraGroundMap::SetGround(const double lat, const double lon, const double radius) {
    // Get beginning bounding time and offset for iterative loop
    p_camera->Sensor::SetEphemerisTime(p_camera->Spice::CacheStartTime());
    if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return false;

    double lookC[3];
    p_camera->Sensor::LookDirection(lookC);
    double ux = p_camera->FocalLength() * lookC[0] / lookC[2];
    double uy = p_camera->FocalLength() * lookC[1] / lookC[2];

    double s[3],p[3];
    p_camera->InstrumentPosition(s);
    p_camera->Coordinate(p);
    double distStart = sqrt((s[0]-p[0])*(s[0]-p[0]) +
                            (s[1]-p[1])*(s[1]-p[1]) +
                            (s[2]-p[2])*(s[2]-p[2]));

    int backCheck = 0;
    p_camera->SetLookDirection(lookC);
    if ((fabs(p_camera->UniversalLatitude() - lat) > 45.0) ||
        (fabs(p_camera->UniversalLongitude() - lon) > 180.0)) backCheck++;

    CameraDistortionMap *distortionMap = p_camera->DistortionMap();
    if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return false;
    double dx = distortionMap->FocalPlaneX();
    double dy = distortionMap->FocalPlaneY();

    CameraFocalPlaneMap *focalMap = p_camera->FocalPlaneMap();
    if (!focalMap->SetFocalPlane(dx,dy)) return false;
    double startOffset = focalMap->DetectorLineOffset() -
                         focalMap->DetectorLine();

    // Get ending bounding time and offset for iterative loop
    p_camera->Sensor::SetEphemerisTime(p_camera->Spice::CacheEndTime());
    if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return false;

    p_camera->Sensor::LookDirection(lookC);
    ux = p_camera->FocalLength() * lookC[0] / lookC[2];
    uy = p_camera->FocalLength() * lookC[1] / lookC[2];

    p_camera->InstrumentPosition(s);
    p_camera->Coordinate(p);
    double distEnd = sqrt((s[0]-p[0])*(s[0]-p[0]) +
                          (s[1]-p[1])*(s[1]-p[1]) +
                          (s[2]-p[2])*(s[2]-p[2]));

    p_camera->SetLookDirection(lookC);
    if ((fabs(p_camera->UniversalLatitude() - lat) > 45.0) ||
        (fabs(p_camera->UniversalLongitude() - lon) > 180.0)) backCheck++;

    if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return false;
    dx = distortionMap->FocalPlaneX();
    dy = distortionMap->FocalPlaneY();

    if (!focalMap->SetFocalPlane(dx,dy)) return false;
    double endOffset = focalMap->DetectorLineOffset() -
                       focalMap->DetectorLine();

    // Deal with images that have two roots
    double startTime = p_camera->Spice::CacheStartTime();
    double endTime = p_camera->Spice::CacheEndTime();
    if (backCheck == 1) {
      if (distStart < distEnd) {
        endTime = startTime + (endTime - startTime) / 2.0;
        p_camera->Sensor::SetEphemerisTime(endTime);
        if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return false;

        p_camera->Sensor::LookDirection(lookC);
        ux = p_camera->FocalLength() * lookC[0] / lookC[2];
        uy = p_camera->FocalLength() * lookC[1] / lookC[2];

        if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return false;
        dx = distortionMap->FocalPlaneX();
        dy = distortionMap->FocalPlaneY();

        if (!focalMap->SetFocalPlane(dx,dy)) return false;
        endOffset = focalMap->DetectorLineOffset() -
                    focalMap->DetectorLine();

      }
      else {
        startTime = endTime - (endTime - startTime) / 2.0;
        p_camera->Sensor::SetEphemerisTime(startTime);
        if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return false;

        double lookC[3];
        p_camera->Sensor::LookDirection(lookC);
        double ux = p_camera->FocalLength() * lookC[0] / lookC[2];
        double uy = p_camera->FocalLength() * lookC[1] / lookC[2];

        CameraDistortionMap *distortionMap = p_camera->DistortionMap();
        if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return false;
        double dx = distortionMap->FocalPlaneX();
        double dy = distortionMap->FocalPlaneY();

        CameraFocalPlaneMap *focalMap = p_camera->FocalPlaneMap();
        if (!focalMap->SetFocalPlane(dx,dy)) return false;
        startOffset = focalMap->DetectorLineOffset() -
                      focalMap->DetectorLine();
      }
    }

    // Make sure we are in the image
    if ((startOffset < 0.0) && (endOffset < 0.0)) return false;
    if ((startOffset > 0.0) && (endOffset > 0.0)) return false;

    // Get everything ordered for iteration
    double fl,fh,xl,xh;
    if (startOffset < endOffset) {
      fl = startOffset;
      fh = endOffset;
      xl = startTime;
      xh = endTime;
    }
    else {
      fl = endOffset;
      fh = startOffset;
      xl = endTime;
      xh = startTime;
    }

    // Iterate to find the time at which the instrument imaged the ground point
    LineScanCameraDetectorMap *detectorMap =
      (LineScanCameraDetectorMap *) p_camera->DetectorMap();
    double timeTol = detectorMap->LineRate() / 10.0;
    bool checkHidden = false;
    for (int j=0; j<30; j++) {
      double etGuess = xl + (xh - xl) * fl / (fl - fh);
      p_camera->Sensor::SetEphemerisTime(etGuess);
      if (!p_camera->Sensor::SetUniversalGround (lat,lon,checkHidden)) return false;
      p_camera->Sensor::LookDirection(lookC);
      ux = p_camera->FocalLength() * lookC[0] / lookC[2];
      uy = p_camera->FocalLength() * lookC[1] / lookC[2];

      if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return false;
      dx = distortionMap->FocalPlaneX();
      dy = distortionMap->FocalPlaneY();

      if (!focalMap->SetFocalPlane(dx,dy)) return false;
      double f = focalMap->DetectorLineOffset() -
                 focalMap->DetectorLine();

      double delTime;
      if (f < 0.0) {
        delTime = xl - etGuess;
        xl = etGuess;
        fl = f;
      }
      else {
        delTime = xh - etGuess;
        xh = etGuess;
        fh = f;
      }

      // See if we converged on the point so set up the undistorted
      // focal plane values and return
      if (fabs(delTime) < timeTol || f == 0.0) {
        if (checkHidden) {
          p_focalPlaneX = ux;
          p_focalPlaneY = uy;
          return true;
        }
        else {
          checkHidden = true;
        }
      }
    }
    return false;
  }
}
