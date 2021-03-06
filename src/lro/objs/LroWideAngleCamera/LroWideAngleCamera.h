#ifndef LroWideAngleCamera_h
#define LroWideAngleCamera_h

#include "PushFrameCamera.h"

namespace Isis {
  namespace Lro {
    /**
     * @brief LRO Narrow Angle Camera Model 
     *  
     * @ingroup SpiceInstrumentsAndCameras
     * @ingroup LunarReconnaissanceOrbiter
     *
     * @author 2009-07-08 Jeff Anderson
     *
     * @internal 
     *   @history 2009-07-15 Steven Lambright - Added support for COLOROFFSET 
     *   @history 2009-08-28 Steven Lambright - Changed inheritance to no longer
     *            inherit directly from Camera
     *   @history 2009-09-09 Steven Lambright - Updated wavelengths 
     */
    class LroWideAngleCamera : public Isis::PushFrameCamera {
      public:
        // constructor
        LroWideAngleCamera (Isis::Pvl &lab);

        //! Destroys the Themis Vis Camera object
        ~LroWideAngleCamera () {};

        // Sets the band to the band number given
        void SetBand (const int band);

       /**
        * The camera model is band dependent, so this method returns false
        *
        * @return bool False
        */
        bool IsBandIndependent () { return false; };

      private:
        double p_etStart;              //!<Ephemeris Start iTime
        double p_bandTimeOffset;       //!<Offset iTime for Band
        double p_exposureDur;          //!<Exposure Duration value from labels
        double p_interframeDelay;      //!<Interframe Delay value from labels
        int p_nframelets;                 //!<Number of framelets in whole image
        std::vector<int> p_detectorStartLines;
        std::vector<int> p_frameletOffsets;
    };
  };
};

#endif

