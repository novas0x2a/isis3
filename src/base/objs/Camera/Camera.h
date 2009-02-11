#ifndef Camera_h
#define Camera_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.14 $                                                             
 * $Date: 2009/01/05 22:42:07 $                                                                 
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

#include "Sensor.h"
#include "AlphaCube.h"

namespace Isis { 
  class Projection;
  class CameraDetectorMap;
  class CameraFocalPlaneMap;
  class CameraDistortionMap;
  class CameraGroundMap;
  class CameraSkyMap;
/**                                                                       
 * @author ??? Jeff Anderson
 * 
 * @internal                                                              
 *   @todo Finish documentation. 
 * 
 *   @history 2005-11-09 Tracie Sucharski - Added HasProjection method.
 *   @history 2006-04-11 Tracie Sucharski - Added IgnoreProjection method and
 *                                          p_ignoreProjection so that the 
 *                                          Camera is used rather than Projection.
 *   @history 2006-04-19 Elizabeth Miller - Added SpacecraftRoll method
 *   @history 2006-06-08 Elizabeth Miller - Added static Distance method that
 *                                          calculates the distance between 2
 *                                          lat/lon pts (given the radius)
 *   @history 2006-07-25 Elizabeth Miller - Fixed bug in Distance method
 *   @history 2006-07-31 Elizabeth Miller - Added OffNadirAngle method and
 *                                          removed SpacecraftRoll method
 *   @history 2007-06-11 Debbie A. Cook - Added overloaded method SetUniversalGround
 *                                        that includes a radius argument and method
 *                                        RawFocalPlanetoImage() to handle the common
 *                                        functionality between the SetUniversalGround
 *                                        methods.
 *   @history 2008-01-28 Christopher Austin - Added error throw
 *            when minlon range isn't set beyond initialization.
 *  
 *   @history 2008-02-15 Stacy Alley - In the
 *            GroundRangeResolution () method we had to subtract
 *            0.5 when looking at the far left of pixels and
 *            add 0.5 to ensure we are seeing the far right of
 *            pixels.
 *   @history 2008-05-21 Steven Lambright - Fixed boundary condition in the
 *            GroundRangeResolution () method.
 *   @history 2008-06-18 Christopher Austin - Fixed documentation errors
 *   @history 2008-07-15 Steven Lambright - Added NaifStatus calls
 *   @history 2008-07-24 Steven Lambright - Fixed memory leaks: the alpha cube,
 *            distortion map, focal plane map, sky map, detector map, and ground
 *            map were not being deleted.
 *   @history 2008-08-08 Steven Lambright - Added the LoadCache() method which
 *            tries to find the correct time range and calls
 *            Spice::CreateCache
 *   @history 2008-09-10 Steven Lambright - Added the geometric tiling methods
 *            in order to optimize push frame cameras and prevent corruption of
 *            data when running cam2map with push frame cameras
 *   @history 2009-01-05 Steven Lambright - Added InCube method
 */

  class Camera : public Isis::Sensor {
    public:
      // constructors
      Camera (Isis::Pvl &lab);
  
      // destructor
      virtual ~Camera ();
  
      // Methods
      bool SetImage (const double sample, const double line);
      bool SetUniversalGround (const double latitude, const double longitude);
      bool SetUniversalGround (const double latitude, const double longitude, 
                               const double radius);
      bool SetRightAscensionDeclination(const double ra, const double dec);

     /**
      * Checks to see if the camera object has a projection
      * 
      * @return bool Returns true if it has a projection and false if it does 
      *              not
      */
      bool HasProjection () { return p_projection != 0; };

     /**
      * Virtual method that checks if the band is independent
      * 
      * @return bool Returns true if the band is independent, and false if it is
      *              not
      */
      virtual bool IsBandIndependent () { return true; };

     /**
      * Returns the reference band
      * 
      * @return int Reference Band
      */
      int ReferenceBand() const { return p_referenceBand; };

      /**
       * Checks to see if the Camera object has a reference band
       * 
       * @return bool Returns true if it has a reference band, and false if it
       *              does not
       */
      bool HasReferenceBand() const { return p_referenceBand != 0; };

     /**
      * Virtual method that sets the band number
      * 
      * @param band Band Number
      */
      virtual void SetBand (const int band) {};

     /**
      * Returns the current sample number
      * 
      * @return double Sample Number
      */
      inline double Sample() { return p_childSample; };

     /**
      * Returns the current line number
      * 
      * @return double Line Number
      */
      inline double Line() { return p_childLine; };
  
      bool GroundRange (double &minlat, double &maxlat, 
                        double &minlon, double &maxlon, Isis::Pvl &pvl);
      bool IntersectsLongitudeDomain (Isis::Pvl &pvl);
  
      double PixelResolution ();
      double LineResolution ();
      double SampleResolution ();
      double DetectorResolution ();
  
      double LowestImageResolution ();
      double HighestImageResolution ();
  
      void BasicMapping (Isis::Pvl &map);

     /**
      * Returns the focal length
      * 
      * @return double Focal Length
      */
      inline double FocalLength () const { return p_focalLength; };

     /**
      * Returns the pixel pitch
      * 
      * @return double Pixel Pitch
      */
      inline double PixelPitch () const { return p_pixelPitch; };

     /**
      * Returns the number of samples in the image
      * 
      * @return int Number of Samples
      */
      inline int Samples () const { return p_samples; };

     /**
      * Returns the number of lines in the image
      * 
      * @return int Number of Lines
      */
      inline int Lines () const { return p_lines; };

     /**
      * Returns the number of bands in the image
      * 
      * @return int Number of Bands
      */
      inline int Bands () const { return p_bands; };

     /** 
      * Returns the number of lines in the parent alphacube
      * 
      * @return int Number of Lines in parent alphacube
      */
      inline int ParentLines () const { return p_alphaCube->AlphaLines(); };

     /**
      * Returns the number of samples in the parent alphacube
      * 
      * @return int Number of Samples in the parent alphacube
      */
      inline int ParentSamples () const { return p_alphaCube->AlphaSamples(); };
  
      bool RaDecRange (double &minra, double &maxra, 
                       double &mindec, double &maxdec);
      double RaDecResolution ();

     /**
      * Returns a pointer to the CameraDistortionMap object
      * 
      * @return CameraDistortionMap*
      */
      CameraDistortionMap *DistortionMap() { return p_distortionMap; };

     /**
      * Returns a pointer to the CameraFocalPlaneMap object
      * 
      * @return CameraFocalPlaneMap*
      */
      CameraFocalPlaneMap *FocalPlaneMap() { return p_focalPlaneMap; };

     /**
      * Returns a pointer to the CameraDetectorMap object
      * 
      * @return CameraDetectorMap*
      */
      CameraDetectorMap *DetectorMap() { return p_detectorMap; };

     /**
      * Returns a pointer to the CameraGroundMap object
      * 
      * @return CameraCGroundMap*
      */
      CameraGroundMap *GroundMap() { return p_groundMap; };

     /**
      * Returns a pointer to the CameraSkyMap object
      * 
      * @return CameraSkyMap*
      */
      CameraSkyMap *SkyMap() { return p_skyMap; };

     /**
      * Sets the Distortion Map
      * 
      * @param *map Pointer to a CameraDistortionMap object
      */
      void SetDistortionMap (CameraDistortionMap *map);

     /**
      * Sets the Focal Plane Map
      * 
      * @param *map Pointer to a CameraFocalPlaneMap object
      */
      void SetFocalPlaneMap (CameraFocalPlaneMap *map);

     /**
      * Sets the Detector Map
      * 
      * @param *map Pointer to a CameraDetectorMap object
      */
      void SetDetectorMap (CameraDetectorMap *map);

     /**
      * Sets the Ground Map
      * 
      * @param *map Pointer to a CameraGroundMap object
      */
      void SetGroundMap (CameraGroundMap *map);

     /**
      * Sets the Sky Map
      * 
      * @param *map Pointer to a CameraSkyMap object
      */
      void SetSkyMap (CameraSkyMap *map);

      double NorthAzimuth ();
      double SunAzimuth();
      double SpacecraftAzimuth();
      double OffNadirAngle();

      static double Distance(double lat1, double lon1, 
                             double lat2, double lon2, double radius);
      
      static double GroundAzimuth(double glat, double glon, double slat,
                                  double slon);

      /**
       * Set whether or not the camera should ignore the Projection
       * 
       * @param ignore 
       */
      void IgnoreProjection (bool ignore) { p_ignoreProjection = ignore; };

      void LoadCache(int cacheSize = 0);

      void GetGeometricTilingHint(int &startSize, int &endSize);

      bool InCube();

    protected:

     /**
      * Sets the focal length
      * 
      * @param v Focal Length
      */
      void SetFocalLength (double v) { p_focalLength = v; };

     /**
      * Sets the pixel pitch
      * 
      * @param v Pixel Pitch
      */
      void SetPixelPitch (double v) { p_pixelPitch = v; };
  
      void SetFocalLength ();
      void SetPixelPitch ();

      void SetGeometricTilingHint(int startSize = 128, int endSize = 8);

    private:
      double p_focalLength;  //!<The focal length, in units of millimeters
      double p_pixelPitch;   //!<The pixel pitch, in millimeters per pixel
  
      void GroundRangeResolution ();
      double p_minlat;                    //!<The minimum latitude
      double p_maxlat;                    //!<The maximum latitude
      double p_minlon;                    //!<The minimum longitude
      double p_maxlon;                    //!<The maximum longitude
      double p_minres;                    //!<The minimum resolution
      double p_maxres;                    //!<The maximum resolution
      double p_minlon180;                 //!<The minimum longitude in the 180 domain
      double p_maxlon180;                 //!<The maximum longitude in the 180 domain
      bool p_groundRangeComputed;         /**<Flag showing if the ground range 
                                              was computed successfully.*/
                         
      int p_samples;                      //!<The number of samples in the image
      int p_lines;                        //!<The number of lines in the image
      int p_bands;                        //!<The number of bands in the image
  
      int p_referenceBand;                //!<The reference band 

      Projection *p_projection;           //!<A pointer to the Projection
      bool p_ignoreProjection;            //!<Whether or no to ignore the Projection

      double p_mindec;                    //!<The minimum declination
      double p_maxdec;                    //!<The maximum declination
      double p_minra;                     //!<The minimum right ascension
      double p_maxra;                     //!<The maxumum right ascension
      double p_minra180;                  //!<The minimum right ascension in the 180 domain
      double p_maxra180;                  //!<The maximum right ascension in the 180 domain
      bool p_raDecRangeComputed;      /**<Flag showing if the raDec range
                                          has been computed successfully.*/

      Isis::AlphaCube *p_alphaCube;   //!<A pointer to the AlphaCube
      double p_childSample;           //!<The number of samples in the AlphaCube
      double p_childLine;             //!<The number of lines in the AlphaCube
      CameraDistortionMap *p_distortionMap;  //!<A pointer to the DistortionMap
      CameraFocalPlaneMap *p_focalPlaneMap;  //!<A pointer to the FocalPlaneMap
      CameraDetectorMap *p_detectorMap;      //!<A pointer to the DetectorMap
      CameraGroundMap *p_groundMap;          //!<A pointer to the GroundMap
      CameraSkyMap *p_skyMap;                //!<A pointer to the SkyMap

      double ComputeAzimuth(const double radius, 
                            const double lat, const double lon);

      bool RawFocalPlanetoImage();

      int p_geometricTilingStartSize; //!< The ideal geometric tile size to start with when projecting
      int p_geometricTilingEndSize; //!< The ideal geometric tile size to end with when projecting
  };
};

#endif
