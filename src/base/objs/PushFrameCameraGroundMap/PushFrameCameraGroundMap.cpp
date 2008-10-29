/**
 * @file
 * $Revision: 1.3 $
 * $Date: 2008/06/18 17:02:18 $
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

#include <iomanip>
#include "PushFrameCameraGroundMap.h"

#include "CameraDistortionMap.h"
#include "PushFrameCameraDetectorMap.h"
#include "CameraFocalPlaneMap.h"

namespace Isis {
  /** Compute undistorted focal plane coordinate from ground position
   *
   * @param lat planetocentric latitude in degrees
   * @param lon planetocentric longitude in degrees
   *
   * @return conversion was successful 
   */
  bool PushFrameCameraGroundMap::SetGround(const double lat, const double lon) {
    PushFrameCameraDetectorMap *detectorMap = (PushFrameCameraDetectorMap *) p_camera->DetectorMap();

    // Get ending bounding framelets and distances for iterative loop
    int startFramelet = 1;
    double startDist = FindDistance(1, lat, lon);

    int endFramelet = detectorMap->TotalFramelets();
    double endDist = FindDistance(endFramelet, lat, lon);

    for (int j=0; j<30; j++) {
      int middleFramelet = (startFramelet + endFramelet) / 2;
      double middleDist = FindDistance(middleFramelet, lat, lon);

      if(startDist > endDist) {
        // This makes sure we don't get stuck halfway between framelets
        if(startFramelet == middleFramelet) middleFramelet++;
        startFramelet = middleFramelet;
        startDist = middleDist;
      }
      else {
        endFramelet = middleFramelet;
        endDist = middleDist;
      }

      // See if we converged on the point so set up the undistorted
      // focal plane values and return
      if (startFramelet == endFramelet) {
        int realFramelet = startFramelet;
        bool frameletEven = (startFramelet % 2 == 0);

        // Do we need to find a neighboring framelet? Get the closest (minimize distance)
        if(frameletEven != p_evenFramelets) {
          double beforeDist = FindDistance(startFramelet-1, lat, lon);
          double afterDist = FindDistance(startFramelet+1, lat, lon);

          if(beforeDist < afterDist) {
            realFramelet = startFramelet - 1;
          }
          else {
            realFramelet = startFramelet + 1;
          }
        }

        detectorMap->SetFramelet(realFramelet);
        if (!CameraGroundMap::SetGround(lat,lon)) return false;
        return true;
      }
    }

    return false;
  }

  /** 
   * This method finds the distance from the center of the framelet to the lat,lon.
   *   The distance is only in the Y-direction and is squared.
   * 
   * @param framelet
   * @param lat 
   * @param lon
   * 
   * @return double Y-Distance squared from center of framelet to lat,lon
   */
  double PushFrameCameraGroundMap::FindDistance(int framelet, const double lat, const double lon) {
    PushFrameCameraDetectorMap *detectorMap = (PushFrameCameraDetectorMap *) p_camera->DetectorMap();
    CameraDistortionMap *distortionMap = (CameraDistortionMap *) p_camera->DistortionMap();

    detectorMap->SetFramelet(framelet);
    if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return false;

    double lookC[3];
    p_camera->Sensor::LookDirection(lookC);
    double ux = p_camera->FocalLength() * lookC[0] / lookC[2];
    double uy = p_camera->FocalLength() * lookC[1] / lookC[2];

    if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return false;

    double dx = distortionMap->FocalPlaneX();
    double dy = distortionMap->FocalPlaneY();

    CameraFocalPlaneMap *focalMap = p_camera->FocalPlaneMap();
    if (!focalMap->SetFocalPlane(dx,dy)) return false;

    detectorMap->SetDetector(focalMap->DetectorSample(), focalMap->DetectorLine());
    double actualFrameletHeight = detectorMap->frameletHeight() / detectorMap->LineScaleFactor();
    double frameletDeltaY = detectorMap->frameletLine() - (actualFrameletHeight / 2.0);
    return frameletDeltaY*frameletDeltaY;
  }

  /** 
   * This method finds the distance from the center of the framelet to the lat,lon.
   *   The distance is only in the Y-direction and is squared.
   * 
   * @param framelet
   * @param lat 
   * @param lon
   * 
   * @return double Y-Distance squared from center of framelet to lat,lon
   */
  double PushFrameCameraGroundMap::FindDistance(double framelet, const double lat, const double lon) {
    PushFrameCameraDetectorMap *detectorMap = (PushFrameCameraDetectorMap *) p_camera->DetectorMap();
    CameraDistortionMap *distortionMap = (CameraDistortionMap *) p_camera->DistortionMap();

    detectorMap->SetFramelet((int)(framelet));
    if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return false;

    double lookC[3];
    p_camera->Sensor::LookDirection(lookC);
    double ux = p_camera->FocalLength() * lookC[0] / lookC[2];
    double uy = p_camera->FocalLength() * lookC[1] / lookC[2];

    if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return false;

    double dx = distortionMap->FocalPlaneX();
    double dy = distortionMap->FocalPlaneY();

    CameraFocalPlaneMap *focalMap = p_camera->FocalPlaneMap();
    if (!focalMap->SetFocalPlane(dx,dy)) return false;

    detectorMap->SetDetector(focalMap->DetectorSample(), focalMap->DetectorLine());
    double fractionalFramelet = framelet - (int)framelet;
    double actualFrameletHeight = detectorMap->frameletHeight() / detectorMap->LineScaleFactor();
    double frameletDeltaY = detectorMap->frameletLine() - (actualFrameletHeight * fractionalFramelet);
    return fabs(frameletDeltaY);
  }
}
