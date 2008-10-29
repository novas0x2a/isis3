#ifndef MocWideAngleCamera_h
#define MocWideAngleCamera_h

#include "Camera.h"

namespace Isis {
  namespace Mgs {
    /**                                                                       
     * @brief Moc Wide Angle Camera Class
     *
     * @internal
     *   @history 2008-08-08 Steven Lambright Now using the new LoadCache(...)
     *            method instead of CreateCache(...).
     */ 
    class MocWideAngleCamera : public Isis::Camera {
      public:
        // constructors
        MocWideAngleCamera (Isis::Pvl &lab);
    
        // destructor
        ~MocWideAngleCamera () {};      
    };
  };
};

#endif
