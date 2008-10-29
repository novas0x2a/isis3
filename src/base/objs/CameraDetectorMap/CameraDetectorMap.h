/**
 * @file
 * $Revision: 1.1.1.1 $
 * $Date: 2006/10/31 23:18:06 $
 *
 *   Unless noted otherwise, the portions of Isis written by the USGS are
 *   public domain. See individual third-party library and package descriptions
 *   for intellectual property information, user agreements, and related
 *   information.
 *
 *   Although Isis has been used by the USGS, no warranty, expressed or
 *   implied, is made by the USGS as to the accuracy and functioning of such
 *   software and related material nor shall the fact of distribution
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.
 *
 *   For additional information, launch
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.
 */
#ifndef CameraDetectorMap_h
#define CameraDetectorMap_h

#include "Camera.h"

namespace Isis {
  /** Convert between parent image coordinates and detector coordinates
   *
   * This base class is used to convert between parent dector coordinates
   * (sample/line) and detector coordinates for the camera.
   *
   * @ingroup Camera
   *
   * @see Camera
   *
   * @author 2005-02-03 Jeff Anderson
   *
   * @internal
   *
   */
  class CameraDetectorMap {
    public:
      CameraDetectorMap(Camera *parent=0);

      //! Destructor
      virtual ~CameraDetectorMap() {};

      virtual bool SetParent(const double sample, const double line);

      virtual bool SetDetector(const double sample, const double line);

      //! Return parent sample
      inline double ParentSample() const { return p_parentSample; };

      //! Return parent line
      inline double ParentLine() const { return p_parentLine; };

      //! Return detector sample
      inline double DetectorSample() const { return p_detectorSample; };

      //! Return detector line
      inline double DetectorLine() const { return p_detectorLine; };

      /** Set the starting detector sample
       *
       * Use this method to specify the starting detector that represents
       * the first image sample in the cube.  If not set the default is 1.
       *
       * @param sample Starting detector sample
       *
       */
      inline void SetStartingDetectorSample (const double sample) {
        p_startingDetectorSample = sample;
        Compute();
      };

      /** Set the starting detector line
       *
       * Use this method to specify the starting detector that represents
       * the first image line in the cube.  If not set the default is 1.
       *
       * @param line Starting detector line
       *
       */
      inline void SetStartingDetectorLine (const double line) {
        p_startingDetectorSample = line;
        Compute();
      };

      /** Set sample summing mode
       *
       * Use this method to specify if detector samples are summed/averaged.
       * That is, one image sample represents the average of N detectors.
       * If not set the default is 1.
       *
       * @param summing Sample summing mode
       *
       */
      inline void SetDetectorSampleSumming (const double summing) {
        p_detectorSampleSumming = summing;
        Compute();
      };

      /** Set line summing mode
       *
       * Use this method to specify if detector lines are summed/averaged.
       * That is, one image lines represents the average of N detectors.
       * If not set the default is 1.
       *
       * @param summing Line summing mode
       *
       */
      inline void SetDetectorLineSumming (const double summing) {
        p_detectorLineSumming = summing;
        Compute();
      };

      //! Return scaling factor for computing sample resolution
      virtual double SampleScaleFactor () const {
        return p_detectorSampleSumming;
      };

      //! Return scaling factor for computing line resolution
      virtual double LineScaleFactor () const {
        return p_detectorLineSumming;
      };

      //! Return the line collection rate (0 for framing cameras)
      virtual double LineRate () const { return 0.0; };

      //! Is the x axis time dependent
      bool IsXAxisTimeDependent() const { return p_xAxisTimeDependent; };

      //! Is the y axis time dependent
      bool IsYAxisTimeDependent() const { return p_yAxisTimeDependent; };

      /**
       *  Set the direction of the x axis, if not set it will be positive right
       */
      void SetXAxisDirection(bool right) {
        p_xAxisDirection = (right) ? 1.0 : -1.0;
      }

      /**
       *  Set the direction of the y axis, if not set it will be
       *  positive up
       */
      void SetYAxisDirection(bool up) {
        p_yAxisDirection = (up) ? 1.0 : -1.0;
      }

      //! Return direction of x axis (1 is positive right, -1 positive left)
      double XAxisDirection() const { return p_xAxisDirection; };

      //! Return direction of y axis (1 is positive up, -1 positive down)
      double YAxisDirection() const { return p_yAxisDirection; };

    protected:
      Camera *p_camera;

      double p_parentSample;
      double p_parentLine;
      double p_detectorLine;
      double p_detectorSample;

      double p_detectorSampleSumming;
      double p_detectorLineSumming;
      double p_startingDetectorSample;
      double p_startingDetectorLine;

      double p_ss;
      double p_sl;

      bool p_xAxisTimeDependent;
      bool p_yAxisTimeDependent;
      double p_xAxisDirection;
      double p_yAxisDirection;

    private:
      void Compute();
  };
};
#endif
