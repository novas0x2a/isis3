#ifndef MiniRF_h
#define MiniRF_h
/**
 * @file
 * $Revision: 1.4 $
 * $Date: 2009/08/31 15:12:33 $
 *
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for
 *   intellectual property information,user agreements, and related information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software
 *   and related material nor shall the fact of distribution constitute any such
 *   warranty, and no responsibility is assumed by the USGS in connection
 *   therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see
 *   the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */
#include "RadarCamera.h"

namespace Isis {
  namespace Lro {
    /**
     * @brief LRO Mini-RF SAR and Chandrayaan 1 Mini-RF SAR
     *
     * This is the camera model for both LRO Mini-RF SAR and
     * Chandrayaan 1 Mini-RF SAR radar systems.
     *
     * @ingroup SpiceInstrumentsAndCameras
     * @ingroup LRO
     *
     * @author 2008-07-01 Jeff Anderson
     *
     * @internal
     *   @history 2009-07-01 Janet Barrett - Updated with code to handle
     *            weighting of slant range and Doppler shift; fixed so that
     *            the frequency and wavelength of the instrument are made
     *            available to Radar classes.
     *   @history 2009-07-31 Debbie A. Cook and Jeannie Walldren - Added
     *            new tolerance argument to LoadCache call to be compatible 
     *            with update to Spice class
     *   @history 2009-08-05 Debbie A. Cook - corrected altitude in tolerance
     *            calculation
     */
    class MiniRF : public RadarCamera {
      public:
        MiniRF (Isis::Pvl &lab);

        ~MiniRF () {};
    };
  };
};

#endif
