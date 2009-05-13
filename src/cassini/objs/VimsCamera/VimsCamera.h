#ifndef VimsCamera_h
#define VimsCamera_h

#include "Camera.h"
    
namespace Isis{
  namespace Cassini {
     /** Cassini Vims camera model
     *  
     *   This is the camera model for the Cassini Vims instrument
     *
     * @ingroup SpiceInstrumentsAndCameras 
     * @ingroup Cassini-Huygens 
     *
     * @see Camera
     *
     * @internal
     *
     * @history 2006-03-16 Tracie Sucharski Original version
     * @history 2009-04-06 Steven Lambright Fixed problem that caused double 
     *          deletion of sky map / ground map.
     *
     *
     */
    class VimsCamera : public Camera {
      public:
        // constructors
        VimsCamera (Pvl &lab);
    
        // destructor
        ~VimsCamera () {};
    
//        void SetBand (const int physicalBand);
//        bool IsBandIndependent () { return false; };
                    
    };
  };
};
#endif
