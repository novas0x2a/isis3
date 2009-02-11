/**
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

#ifndef LoHighCamera_h
#define LoHighCamera_h

#include "Camera.h"

using namespace std;

namespace Isis {
  namespace Lo { 
    /**
     * @brief Defines the Lunar Orbiter High Resolution camera class
     *
     * The LoHighCamera class defines the High Resolution camera for the last three
     * Lunar Orbiter missions (3, 4, and 5).
     *
     * @ingroup SpiceInstrumentsAndCameras
     * @ingroup LunarOrbiter
     *
     * @author 2007-07-17 Debbie A. Cook
     *
     * @internal
     *   @history 2007-07-17 Debbie A. Cook - Original Version
     *   @history 2008-08-08 Steven Lambright Made the unit test work with a Sensor
     *            change. Also, now using the new LoadCache(...) method instead of
     *            CreateCache(...).
     */
    class LoHighCamera : public Isis::Camera {
      public:
        LoHighCamera (Isis::Pvl &lab);
        ~LoHighCamera () {}; 
    }; 
  };
};
#endif