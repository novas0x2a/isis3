#ifndef MocNarrowAngleCamera_h
#define MocNarrowAngleCamera_h

#include "Camera.h"

namespace Isis {
  namespace Mgs {
    /**
     * @internal
     *   @history 2008-08-08 Steven Lambright Now using the new LoadCache(...)
     *            method instead of CreateCache(...).
     */
    class MocNarrowAngleCamera : public Camera {
      public:
        MocNarrowAngleCamera (Isis::Pvl &lab);

        ~MocNarrowAngleCamera () {};
    };
  };
};

#endif
