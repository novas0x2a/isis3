#ifndef ThemisIrCamera_h
#define ThemisIrCamera_h

#include "Camera.h"

namespace Isis {
  namespace Odyssey {
    class ThemisIrDistortionMap;

    /**
     * @brief THEMIS IR Camera
     *
     * This is the camera class for the THEMIS IR camera
     *
     * @ingroup MarsOdyssey 
     *
     * @author  2005-01-01 Jeff Anderson
     *
     * @internal
     *   @history 2007-07-13 Jeff Anderson Added support for spatial summing
     *   @history 2008-08-08 Steven Lambright Now using the new LoadCache(...)
     *            method instead of CreateCache(...).
     */
    class ThemisIrCamera : public Isis::Camera {
      public:
        // constructors
        ThemisIrCamera (Isis::Pvl &lab);
    
        // destructor
        ~ThemisIrCamera () {};
        
        // Band dependent 
        void SetBand (const int band);
        bool IsBandIndependent () { return false; };
        
      private:
        double p_etStart;
        double p_lineRate;
        double p_bandTimeOffset;
        std::string p_tdiMode;
        std::vector<int> p_originalBand;
    };
  };
};
#endif
