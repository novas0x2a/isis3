/**
 * @file
 * $Revision: 1.6 $
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
#include "MiniRF.h"
#include "iString.h"
#include "iException.h"
#include "RadarPulseMap.h"
#include "RadarGroundRangeMap.h"
#include "RadarSlantRangeMap.h"
#include "RadarGroundMap.h"
#include "RadarSkyMap.h"

using namespace std;

namespace Isis {
  namespace Lro {
    /**
     * @brief Initialize the Mini-RF SAR radar model for LRO and Chandrayaan 1
     *
     * This constructor reads the image labels of a Mini-RF SAR file to acquire
     * its default parameters.
     *
     * @param lab The label provided to initialize the radar model.
     * @internal 
     * @history 2009-07-31 Jeannie Walldren - Added tolerance
     *            parameter value of -1 to call to
     *            Spice::CreateCache() method.
     * @history 2009-07-31 Debbie Cook - Calculated actual tolerance 
     *          value to pass into Spice::CreateCache() method.
     */
    MiniRF::MiniRF (Isis::Pvl &lab) : Isis::RadarCamera(lab) {

      // Get the ground range resolution (ScaledPixelHeight and ScaledPixelWidth
      // are expected to be equal - mrf2isis checks for this)
      Isis::PvlGroup &inst = lab.FindGroup ("Instrument",Isis::Pvl::Traverse);
      double groundRangeResolution = inst["ScaledPixelHeight"];

      // Synthesize the pixel pitch to the ground range resolution
      SetPixelPitch(groundRangeResolution);

      // Focal length should always be slant range to the current ground
      // point. This will be set each time the slant range is calculated.
      // For now, set the focal length to 1.0. 
      SetFocalLength(1.0);

      // Get the start time from labels (the SpacecraftClockStartCount is set to
      // is set to UNK in the PDS labels, so StartTime is used instead)
      string stime = inst["StartTime"];
      SpiceDouble etStart;
      str2et_c(stime.c_str(),&etStart);

      // The line rate is in units of seconds in the PDS label. The exposure
      // is the sum of the burst and the delay for the return. The movement of the
      // spacecraft is negligible compared to the speed of light, so we are assuming
      // the spacecraft hasn't moved between the burst and the return.
      double lineRate = (double) inst["LineExposureDuration"];

      // Get the incidence angle at the center of the image
      double incidenceAngle = (double) inst["IncidenceAngle"];
      incidenceAngle = incidenceAngle * Isis::PI / 180.0;

      // Get the azimuth resolution at the center of the image
      double azimuthResolution = (double) inst["AzimuthResolution"];
      azimuthResolution = azimuthResolution / 1000.0;

      // Get the range resolution at the center of the image
      double rangeResolution = (double) inst["RangeResolution"];
      rangeResolution = rangeResolution / 1000.0;

      // Get the wavelength or frequency of the instrument. This does not
      // exist in the PDS labels, so it will need to be hardcoded until the
      // PDS labels are updated. Right now, the mrf2isis program is putting
      // a frequency value in the labels based on the instrument mode id.
      double frequency = (double) inst["Frequency"];
      double waveLength = clight_c() / frequency;

      // Setup map from image(sample,line) to radar(sample,time)
      new RadarPulseMap(this,etStart,lineRate);

      // Setup map from radar(sample,time) to radar(groundrange,time)
      Radar::LookDirection ldir = Radar::Right;
      if ((std::string)inst["LookDirection"] == "LEFT") {
        ldir = Radar::Left;
      }
      RadarGroundRangeMap::setTransform(NaifIkCode(),groundRangeResolution,
                                        this->Samples(),ldir);
      new RadarGroundRangeMap(this,NaifIkCode());

      // Calculate weighting for focal plane coordinates. This is done
      // because the focal plane coordinates (slant range and Doppler
      // shift) do not have the same units of measurement and cannot
      // be used by jigsaw as is. The weighting factors convert the
      // focal plane coordinates into comparitive values. The weighting
      // factor for the Doppler shift requires spacecraft pointing and
      // velocity at the center of the image, so it is calculated  
      // after Spice gets loaded.
      double range_sigma = rangeResolution * sin(incidenceAngle) * 100;
      double etMid = etStart + 0.5 * (this->ParentLines() + 1) * lineRate;
 
      // Setup the map from Radar(groundRange,t) to Radar(slantRange,t)
      RadarSlantRangeMap *slantRangeMap = new RadarSlantRangeMap(this,
                 groundRangeResolution);
      slantRangeMap->SetCoefficients(inst["RangeCoefficientSet"]);

      // Setup the ground and sky map
      RadarGroundMap *groundMap = new RadarGroundMap(this,ldir,waveLength);
      new RadarSkyMap(this);

      // Set the time range to cover the cube
      // Must be done last as the naif kernels will be unloaded
      double etEnd = etStart + this->ParentLines() * lineRate + lineRate;
      etStart = etStart - lineRate;
      double tol=PixelResolution()/100.;

      if (tol < 0.) {
        // Alternative calculation of .01*ground resolution of a pixel
        SetEphemerisTime(etMid);
        std::vector<double> s = InstrumentPosition()->Coordinate();
        tol = PixelPitch()*SpacecraftAltitude()/FocalLength()/1000./100.;
      }
      Spice::CreateCache(etStart,etEnd,this->ParentLines()+1, tol);
      SetEphemerisTime(etMid);
      SpiceRotation *bodyFrame = this->BodyRotation();
      SpicePosition *spaceCraft = this->InstrumentPosition();
      std::vector<double> Vsc = bodyFrame->ReferenceVector(spaceCraft->Velocity());
      std::vector<double> Xsc = bodyFrame->ReferenceVector(spaceCraft->Coordinate());
      double radii[3];
      this->Radii(radii);
      double R = radii[0];
      double height = sqrt(Xsc[0]*Xsc[0] + Xsc[1]*Xsc[1] + Xsc[2]*Xsc[2]) - R;
      double speed = vnorm_c(&Vsc[0]);
      double doppler_sigma = 2.0 * speed * azimuthResolution / (waveLength *
                 height / cos(incidenceAngle)) * 100;
      slantRangeMap->SetWeightFactors(range_sigma,doppler_sigma);
      groundMap->SetWeightFactors(range_sigma,doppler_sigma);
    }
  }
}

extern "C" Isis::Camera *MiniRFPlugin (Isis::Pvl &lab) {
  return new Isis::Lro::MiniRF(lab);
}
