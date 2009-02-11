#ifndef CTXCamera_h
#define CTXCamera_h

#include "Camera.h"

namespace Isis {
  namespace Mro {
    /**
     * @internal
     *   @todo Allow the programmer to apply scale and shear.
     *   @todo Write multiplaction method (operator*) for Affine * Affine.
     *
     *   @history 2006-08-03  Tracie Sucharski, Added Scale method
     *   @history 2007-07-12  Debbie A. Cook, Added methods Coefficients and
     *            InverseCoefficients
     *   @history 2008-02-21 Steven Lambright Boresight, focal length, pixel pitch
     *            keywords now loaded from kernels instead of being hard-coded.
     *            The distortion map is now being used.
     *   @history 2008-08-08 Steven Lambright Made the unit test work with a Sensor
     *            change. Also, now using the new LoadCache(...) method instead of
     *            CreateCache(...).
     */
    class CTXCamera : public Camera {
      public:
        CTXCamera (Isis::Pvl &lab);

        ~CTXCamera () {};
    };
  };
};

#endif