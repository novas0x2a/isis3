/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2008/06/18 17:02:19 $                                                                 
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

#ifndef PushFrameCameraGroundMap_h
#define PushFrameCameraGroundMap_h

#include "CameraGroundMap.h"

namespace Isis {    
  /** Convert between undistorted focal plane and ground coordinates
   * 
   * This class is used to convert between undistorted focal plane
   * coordinates (x/y) in millimeters and ground coordinates lat/lon
   * for line scan cameras.
   * 
   * @ingroup Camera
   * 
   * @see Camera
   *  
   * @author 2007-10-17 Steven Lambright and Jeff Anderson
   *  
   * @internal
   *   @author 2008-06-18 Steven Lambright Fixed documentation
   */
  class PushFrameCameraGroundMap : public CameraGroundMap {
    public:
      /**
       * This is the constructor for the push frame ground map
       * 
       * @param cam Pointer to the camera
       * @param etStart Starting time of the image (ET)
       * @param etEnd Ending time of the image (ET)
       * @param evenFramelets True if the image contains even framelets, false for odd
       */
      PushFrameCameraGroundMap(Camera *cam, double etStart, double etEnd, bool evenFramelets) : CameraGroundMap(cam) {
        p_etStart = etStart;
        p_etEnd = etEnd;
        p_evenFramelets = evenFramelets;
      }
  
      //! Destructor
      virtual ~PushFrameCameraGroundMap() {};

      virtual bool SetGround(const double lat, const double lon);

    private:
      double FindDistance(int framelet, const double lat, const double lon);
      double FindDistance(double framelet, const double lat, const double lon);

      double p_etStart; //!<Starting ET of the image
      double p_etEnd; //!<Ending ET of the image
      bool   p_evenFramelets; //!<True if the file contains even framelets
  };
};
#endif
