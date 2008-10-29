#ifndef PhotoModel_h
#define PhotoModel_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.6 $                                                             
 * $Date: 2008/07/09 19:50:46 $                                                                 
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

#include <string>
#include <vector>
#include "Pvl.h"

namespace Isis {

/**
 * @brief
 *
 * @author 1998-12-21 Randy Kirk
 *
 * @internal
 *  @history 2007-07-31 Steven Lambright - 
 *  @history 2008-03-07 Janet Barrett - Moved variables and related
 *                      methods that pertain to Hapke specific parameters
 *                      to this class from the HapkeHen class. Also added
 *                      the code to set standard conditions.
 *  @history 2008-06-18 Steven Koechle - Fixed Documentation Errors
 *  @history 2008-07-09 Steven Lambright - Fixed unit test 
 */
  class PhotoModel {
    public:
      PhotoModel (Pvl &pvl);
      virtual ~PhotoModel() {};

      //! Calculate the surface brightness
      double CalcSurfAlbedo(double pha, double inc, double ema);

      /**
       * Reset variables that affect surface brightness for initialization. For
       * most photometric functions, this does nothing. However, for
       * Hapke this turns off the opposition effect.
       */
      virtual void SetStandardConditions(bool standard);

      //! Return algorithm name
      inline std::string AlgorithmName () const { return p_photoAlgorithmName; };

      void SetPhotoWh(const double wh);
      //! Return photometric Wh value
      inline double PhotoWh () const { return p_photoWh; };

      void SetPhotoHg1(const double hg1);
      //! Return photometric Hg1 value
      inline double PhotoHg1 () const { return p_photoHg1; };

      void SetPhotoHg2(const double hg2);
      //! Return photometric Hg2 value
      inline double PhotoHg2 () const { return p_photoHg2; };

      void SetPhotoHh(const double hh);
      //! Return photometric Hh value
      inline double PhotoHh () const { return p_photoHh; };

      void SetPhotoB0(const double b0);
      //! Return photometric B0 value
      inline double PhotoB0 () const { return p_photoB0; };

      void SetPhotoTheta(const double theta);
      //! Return photometric Theta value
      inline double PhotoTheta () const { return p_photoTheta; };

      void SetOldTheta(double theta) { p_photoThetaold = theta; }

      void SetPhotoBh(const double bh);
      //! Return photometric Bh value
      inline double PhotoBh () const { return p_photoBh; };

      void SetPhotoCh(const double ch);
      //! Return photometric Ch value
      inline double PhotoCh () const { return p_photoCh; };

      //! Hapke's approximation to Chandra's H function
      inline double Hfunc(double u, double gamma) {
        return (1.0 + 2.0 * u)/(1.0 + 2.0 * u * gamma);
      }

      //! Obtain topographic derivative of an arbitrary photometric
      //! function
      double PhtTopder(double phase, double incidence, double emission);

    protected:
      virtual double PhotoModelAlgorithm (double phase, 
            double incidence, double emission) = 0;

      bool StandardConditions() const { return p_standardConditions; }

      // Unique name of the photometric model
      std::string p_photoAlgorithmName; 

      double p_photoWh;
      double p_photoHg1;
      double p_photoHg2;
      double p_photoHh;
      double p_photoB0;
      double p_photoB0save;
      double p_photoTheta;
      double p_photoThetaold;
      double p_photoBh;
      double p_photoCh;
      double p_photoCott;
      double p_photoCot2t;
      double p_photoTant;
      double p_photoSr;
      double p_photoOsr;
      double p_photoR30;
      double p_photoM1;
      double p_photoM2;
      double p_photoM3;

    private:
      bool p_standardConditions;

  };
};

#endif
