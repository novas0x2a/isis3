#ifndef SsiCamera_h
#define SsiCamera_h

#include "Camera.h"

namespace Galileo { 
  /**
   * @ingroup SpiceInstrumentsAndCameras
   * 
   * @brief Camera class
   *
   * This is the camera class for the IssNACamera
   *
   * @ingroup Galileo
   *
   * @author  Jeff Anderson
   *
   * @internal
   *   @history 2007-10-25 Steven Koechle - Fixed so that it works
   *            in Isis3
   *   @history 2008-08-08 Steven Lambright Now using the new LoadCache(...)
   *            method instead of CreateCache(...).
   */
  class SsiCamera : public Isis::Camera {
    public:
      SsiCamera (Isis::Pvl &lab);
      ~SsiCamera () {};
  };
};
#endif
