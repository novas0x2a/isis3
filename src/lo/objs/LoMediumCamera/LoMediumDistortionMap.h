#ifndef LoMediumDistortionMap_h
#define LoMediumDistortionMap_h

#include <vector>
#include "CameraDistortionMap.h"

namespace Isis {    
  namespace Lo {
    /** Distort/undistort focal plane coordinates
     * 
     * Creates a map for adding/removing optical distortions 
     * from the focal plane of the Lunar Orbiter medium resolution camera.  
     * 
     * @ingroup Camera
     * 
     * @see Camera
     * 
     * @internal
     * 
     * @history 2007-07-31 Debbie A. Cook - Original version
     * @history 2008-02-04 Jeff Anderson - Made change to support variable
     * focal length in THEMIS IR camera 
     * @history 2008-07-25 Steven Lambright - Fixed constructor; CameraDistortionMap 
     *          is responsible both for setting the p_camera protected member and
     *          calling Camera::SetDistortionMap. When the parent called
     *          Camera::SetDistortionMap the Camera took ownership of the instance
     *          of this object. By calling this twice, and with Camera only
     *          supporting having one distortion map, this object was deleted before
     *          the constructor was finished.
     * 
     */
    class LoMediumDistortionMap : public CameraDistortionMap {
      public:
        LoMediumDistortionMap(Camera *parent);
  
        void SetDistortion(const int naifIkCode);
        virtual bool SetFocalPlane(const double dx, const double dy);
  
        virtual bool SetUndistortedFocalPlane(const double ux, const double uy);  

      private:
        double p_sample0;                  /* Center of distortion on sample axis */
        double p_line0;                       /* Center of distortion on line axis */ 
        std::vector<double> p_coefs;
        std::vector<double> p_icoefs;
    };
  };
};
#endif