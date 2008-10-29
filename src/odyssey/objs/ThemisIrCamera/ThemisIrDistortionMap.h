#ifndef ThemisIrDistortionMap_h
#define ThemisIrDistortionMap_h

#include "CameraDistortionMap.h"

namespace Isis {    
  namespace Odyssey {
    /** Distort/undistort focal plane coordinates
     * 
     * Creates a map for adding/removing optical distortions 
     * from the focal plane of the Themis IR camera.  
     * 
     * @ingroup Camera
     * 
     * @see Camera
     * 
     * @internal
     * 
     * @history 2005-02-01 Jeff Anderson
     * Original version
     * 
     */
    class ThemisIrDistortionMap : public CameraDistortionMap {
      public:
        ThemisIrDistortionMap(Camera *parent);

        virtual bool SetFocalPlane(const double dx, const double dy);
  
        virtual bool SetUndistortedFocalPlane(const double ux, const double uy);  

        void SetDistortion (const double cx1, const double cy);

      private:
        double p_cx1;
        double p_cy;
    };
  };
};
#endif
