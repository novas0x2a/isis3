#ifndef HiresCamera_h
#define HiresCamera_h

#include "Camera.h"

namespace Clementine { 
  /**                                                                       
   * @brief Camera class
   *
   * This is the camera class for the HiresCamera
   *
   * @ingroup Clementine
   *
   * @author  2009-01-16 Tracie Sucharski
   *
   * @internal
   */
  class HiresCamera : public Isis::Camera {
    public:
      HiresCamera (Isis::Pvl &lab);
      ~HiresCamera () {};      
  };
};
#endif
