using namespace std;

#include "ThemisIrDistortionMap.h"

namespace Isis {
  namespace Odyssey {
    ThemisIrDistortionMap::ThemisIrDistortionMap (Camera *parent) :
      CameraDistortionMap (parent,1.0) {
      p_cx1 = 1.0;
      p_cy = 0.0;
    }

    void ThemisIrDistortionMap::SetDistortion (const double cx1,
                                               const double cy) {
      p_cx1 = cx1;
      p_cy = cy * p_camera->PixelPitch();
    }


    bool ThemisIrDistortionMap::SetFocalPlane(const double dx, 
                                              const double dy) {
      p_focalPlaneX = dx;
      p_focalPlaneY = dy;

      p_undistortedFocalPlaneX = p_focalPlaneX / p_cx1;
      p_undistortedFocalPlaneY = p_focalPlaneY - p_cy;

      return true;
    }
    
    bool ThemisIrDistortionMap::SetUndistortedFocalPlane(const double ux, 
                                                         const double uy) {
      p_undistortedFocalPlaneX = ux;
      p_undistortedFocalPlaneY = uy;

      p_focalPlaneX = p_undistortedFocalPlaneX * p_cx1;
      p_focalPlaneY = p_undistortedFocalPlaneY + p_cy;
      return true;
    }
  }
}
