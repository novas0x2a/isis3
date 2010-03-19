/**                                                                       
 * @file                                                                  
 * $Revision: 1.6 $                                                             
 * $Date: 2009/10/15 01:38:43 $                                                                 
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
#include "CameraGroundMap.h"
#include "NaifStatus.h"

namespace Isis {
  CameraGroundMap::CameraGroundMap(Camera *parent) {
    p_camera = parent;
    p_camera->SetGroundMap(this);
  }

  /** Compute ground position from focal plane coordinate
   * 
   * This method will compute the ground position given an
   * undistorted focal plane coordinate.  Note that the latitude/longitude
   * value can be obtained from the camera class passed into the constructor.
   * 
   * @param ux distorted focal plane x in millimeters
   * @param uy distorted focal plane y in millimeters
   * @param uz distorted focal plane z in millimeters
   * 
   * @return conversion was successful
   */
  bool CameraGroundMap::SetFocalPlane(const double ux, const double uy, 
                                      double uz) {
    NaifStatus::CheckErrors();

    SpiceDouble lookC[3];
    lookC[0] = ux;
    lookC[1] = uy;
    lookC[2] = uz;

    SpiceDouble unitLookC[3];
    vhat_c(lookC,unitLookC);

    NaifStatus::CheckErrors();

    return p_camera->SetLookDirection(unitLookC);
  }

  /** Compute undistorted focal plane coordinate from ground position
   * 
   * @param lat planetocentric latitude in degrees
   * @param lon planetocentric longitude in degrees
   * 
   * @return conversion was successful
   */
  bool CameraGroundMap::SetGround(const double lat, const double lon) {
    if (p_camera->Sensor::SetUniversalGround (lat,lon)) {
      LookCtoFocalPlaneXY();
      return true;
    }
    return false;
  }

  //! Compute undistorted focal plane coordinate from camera look vector
  void CameraGroundMap::LookCtoFocalPlaneXY(){
    double lookC[3];
    p_camera->Sensor::LookDirection(lookC);
    double scale = p_camera->FocalLength() / lookC[2];
    p_focalPlaneX = lookC[0] * scale;
    p_focalPlaneY = lookC[1] * scale;
  }

  /** Compute undistorted focal plane coordinate from ground position that includes a local radius
   * 
   * @param lat planetocentric latitude in degrees
   * @param lon planetocentric longitude in degrees
   * @param radius local radius in meters
   * 
   * @return conversion was successful
   */
  bool CameraGroundMap::SetGround(const double lat, const double lon, const double radius) {
    if (p_camera->Sensor::SetUniversalGround (lat,lon, radius)) {
      LookCtoFocalPlaneXY();
      return true;
    }
    return false;
  }

  /** Compute undistorted focal plane coordinate from ground position using current Spice from SetImage call
   * 
   * This method will compute the undistorted focal plane coordinate for 
   * a ground position, using the current Spice settings (time and kernels) 
   * without resetting the current point values for lat/lon/radius/x/y.
   *  
   * @param lat planetocentric latitude in degrees
   * @param lon planetocentric longitude in degrees 
   * @param radius local radius in m 
   * 
   * @return conversion was successful
   */
  bool CameraGroundMap::GetXY(const double lat, const double lon, const double radius,
                              std::vector<double> &lookJ) {

    // Check for Sky images
    if ( p_camera->IsSky() ) {
      return false;
    }

    // Should a check be added to make sure SetImage has been called???
    
    // Compute the look vector in body-fixed coordinates
    double pB[3]; // Point on surface
    latrec_c( radius/1000.0, lon*Isis::PI/180.0, lat*Isis::PI/180.0, pB);

    // Get spacecraft vector in body-fixed coordinates
    SpiceRotation *bodyRot = p_camera->BodyRotation();
    std::vector<double> sB = bodyRot->ReferenceVector(p_camera->InstrumentPosition()->Coordinate());
    std::vector<double> lookB(3);
    for (int ic=0; ic<3; ic++)   lookB[ic] = pB[ic] - sB[ic];

    // Check for point on back of planet by checking to see if surface point is viewable (test emission angle)
    // During iterations, we may not want to do the back of planet test???
    double upsB[3],upB[3],dist;
    vminus_c ( (SpiceDouble *) &lookB[0], upsB);
    unorm_c (upsB, upsB, &dist);
    unorm_c (pB, upB, &dist);
    double angle = vdot_c(upB, upsB);
    double emission;
    if (angle > 1) {
      emission = 0;
    }
    else if (angle < -1) {
      emission = 180.;
    }
    else {
      emission = acos (angle) * 180.0 / Isis::PI;
    }
    if (fabs(emission) > 90.) return false;

    // Get the look vector in the camera frame and the instrument rotation
    lookJ.resize(3);
    lookJ = p_camera->BodyRotation()->J2000Vector( lookB );
    return true;
  }
}
