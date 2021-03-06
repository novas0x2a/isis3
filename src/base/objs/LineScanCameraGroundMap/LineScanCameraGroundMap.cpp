/**
 * @file
 * $Revision: 1.6 $
 * $Date: 2009/03/02 15:52:09 $
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
#include "Statistics.h"

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
    FindFocalPlaneStatus status = FindFocalPlane(-1, lat, lon, radius );

    if(status == Success) return true;
    if(status == Failure) return false;

    if(status == BoundingProblem) {
      // Get ending bounding framelets and distances for iterative loop to minimize the spacecraft distance
      int startLine = 1;
      double startDist = FindSpacecraftDistance(startLine, lat, lon);
  
      int endLine = p_camera->Lines();
      double endDist = FindSpacecraftDistance(endLine, lat, lon);
  
      int deltaX = abs(startLine - endLine) / 2;
  
      // start + deltaX = middle framelet.
      //  We're able to optimize this modified binary search
      //  because the 'V' shape -- it's mostly parallel. Meaning,
      //  if the left side is higher than the right, then the 
      //  solution is closer to the right. The bias factor will
      //  determine how much closer.
      double biasFactor = startDist / endDist;
  
      if(biasFactor < 1.0) {
        biasFactor = -1.0 / biasFactor;
        biasFactor = -(biasFactor + 1) / biasFactor;
      }
      else {
        biasFactor = (biasFactor - 1) / biasFactor;
      }
  
      int middleLine = startLine + (int)(deltaX + biasFactor*deltaX);
  
      if(FindFocalPlane(middleLine, lat, lon, radius) == Success) {
        return true;
      }
      else {
        return false;
      }
    }

    return false;
  }

  double LineScanCameraGroundMap::FindSpacecraftDistance(int line, const double lat, const double lon) {
    CameraDetectorMap *detectorMap = p_camera->DetectorMap();
    detectorMap->SetParent(p_camera->ParentSamples() / 2, line);
    if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return DBL_MAX;

    return p_camera->SlantDistance();
  }

  LineScanCameraGroundMap::FindFocalPlaneStatus 
    LineScanCameraGroundMap::FindFocalPlane(const int &approxLine, const double &lat, 
                                            const double &lon, const double &radius) {
    // find middle time; cache time starting points vary
    double approxTime = 0.0;
    double lineRate = 0.0;

    CameraDistortionMap *distortionMap = p_camera->DistortionMap();
    CameraFocalPlaneMap *focalMap = p_camera->FocalPlaneMap();

    double startOffset = 0.0, endOffset = 0.0;
    double startTime = 0.0, endTime = 0.0;
    double lookC[3] = {0.0, 0.0, 0.0};
    double ux = 0.0, uy = 0.0;
    double dx = 0.0, dy = 0.0;
    const double cacheStart = p_camera->Spice::CacheStartTime();
    const double cacheEnd = p_camera->Spice::CacheEndTime();
    unsigned long lineTolerance = 100;
    bool startTimeMaxed = false, endTimeMaxed = false;

    if(approxLine >= 0) {
      p_camera->DetectorMap()->SetParent(p_camera->ParentSamples() / 2, approxLine);
      approxTime = p_camera->EphemerisTime();
      lineRate = ((LineScanCameraDetectorMap*)p_camera->DetectorMap())->LineRate();
    }

    bool bounded = false;
    while(!bounded) {
      if(approxTime != 0.0) {
        startTime = approxTime - lineTolerance * lineRate;
        endTime   = approxTime + lineTolerance * lineRate;
      }
      else {
        startTime = cacheStart;
        endTime = cacheEnd;
      }

      if(startTime <= cacheStart) {
        startTimeMaxed = true;
        startTime = cacheStart;
      }

      if(endTime >= cacheEnd) {
        endTimeMaxed = true;
        endTime = cacheEnd;
      }

      bounded = true;

      // Get beginning bounding time and offset for iterative loop
      p_camera->Sensor::SetEphemerisTime(startTime);
      if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return Failure;

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
      if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return Failure;
      double dx = distortionMap->FocalPlaneX();
      double dy = distortionMap->FocalPlaneY();
  
      CameraFocalPlaneMap *focalMap = p_camera->FocalPlaneMap();
      if (!focalMap->SetFocalPlane(dx,dy)) return Failure;
      startOffset = focalMap->DetectorLineOffset() -
                           focalMap->DetectorLine();
  
      // Get ending bounding time and offset for iterative loop
      p_camera->Sensor::SetEphemerisTime(endTime);
      if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return Failure;
  
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
  
      if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return Failure;
      dx = distortionMap->FocalPlaneX();
      dy = distortionMap->FocalPlaneY();
  
      if (!focalMap->SetFocalPlane(dx,dy)) return Failure;
      endOffset = focalMap->DetectorLineOffset() -
                         focalMap->DetectorLine();
  
      // Deal with images that have two roots
      if (backCheck == 1) {
        if (distStart < distEnd) {
          endTime = startTime + (endTime - startTime) / 2.0;
          p_camera->Sensor::SetEphemerisTime(endTime);
          if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return Failure;
  
          p_camera->Sensor::LookDirection(lookC);
          ux = p_camera->FocalLength() * lookC[0] / lookC[2];
          uy = p_camera->FocalLength() * lookC[1] / lookC[2];
  
          if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return Failure;
          dx = distortionMap->FocalPlaneX();
          dy = distortionMap->FocalPlaneY();
  
          if (!focalMap->SetFocalPlane(dx,dy)) return Failure;
          endOffset = focalMap->DetectorLineOffset() -
                      focalMap->DetectorLine();
  
        }
        else {
          startTime = endTime - (endTime - startTime) / 2.0;
          p_camera->Sensor::SetEphemerisTime(startTime);
          if (!p_camera->Sensor::SetUniversalGround (lat,lon,false)) return Failure;

          p_camera->Sensor::LookDirection(lookC);
          ux = p_camera->FocalLength() * lookC[0] / lookC[2];
          uy = p_camera->FocalLength() * lookC[1] / lookC[2];
  
          if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) return Failure;
          dx = distortionMap->FocalPlaneX();
          dy = distortionMap->FocalPlaneY();
    
          if (!focalMap->SetFocalPlane(dx,dy)) return Failure;
          startOffset = focalMap->DetectorLineOffset() -
                        focalMap->DetectorLine();
        }
      }

      // Make sure we are in the image
      if ((startOffset < 0.0) && (endOffset < 0.0)) bounded = false;
      if ((startOffset > 0.0) && (endOffset > 0.0)) bounded = false;

      if(!bounded) {
        if(startTimeMaxed || endTimeMaxed) {
          return BoundingProblem;
        }

        lineTolerance *= 2;
        
        if(lineTolerance == 0) return Failure;
      }
    }

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
      if (!p_camera->Sensor::SetUniversalGround (lat,lon,checkHidden)) return Failure;
      p_camera->Sensor::LookDirection(lookC);
      ux = p_camera->FocalLength() * lookC[0] / lookC[2];
      uy = p_camera->FocalLength() * lookC[1] / lookC[2];

      if (!distortionMap->SetUndistortedFocalPlane(ux,uy)) {
        return Failure;
      }
      dx = distortionMap->FocalPlaneX();
      dy = distortionMap->FocalPlaneY();

      if (!focalMap->SetFocalPlane(dx,dy)) {
        return Failure;
      }
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
          return Success;
        }
        else {
          checkHidden = true;
        }
      }
    }

    return Failure;
  }
}
