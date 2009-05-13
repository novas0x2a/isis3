#ifndef LrocNarrowAngleCamera_h
#define LrocNarrowAngleCamera_h

#include "Camera.h"

namespace Isis {
  namespace Lro {
    /** 
     * @author 2009-02-20 Jacob Danton
     *  
     * @internal
     *
     *   @history 2009-02-20  Jacob Danton, Original Object
     */
    class LroNarrowAngleCamera : public Camera {
      public:
    	  LroNarrowAngleCamera (Isis::Pvl &lab);

        ~LroNarrowAngleCamera () {};
    };
  };
};

#endif
