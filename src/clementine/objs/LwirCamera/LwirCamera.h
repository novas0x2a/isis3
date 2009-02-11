#ifndef LwirCamera_h
#define LwirCamera_h

#include "Camera.h"

namespace Clementine { 
  /**                                                                       
   * @brief LWIR Camera class
   *
   * This is the camera class for Clementine's long wavelength 
   * infared camera. 
   *
   * @ingroup Clementine
   *
   * @author  2009-01-16 Jeannie Walldren
   *
   * @internal
   *   @history 2009-01-16 Jeannie Walldren - Original Version
   */
  class LwirCamera : public Isis::Camera {
    public:
      LwirCamera (Isis::Pvl &lab);
      ~LwirCamera () {};      
  };
};
#endif
