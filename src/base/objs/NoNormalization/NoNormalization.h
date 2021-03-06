#if !defined(NoNormalization_h)
#define NoNormalization_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2008/06/18 16:51:47 $                                                                 
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

#include "NormModel.h"

namespace Isis {
  class Pvl;

/**
 * @brief NoNormalization - perform simple correction without normalization (a*dn +b)
 *
 * @author 2008-03-17 Janet Barrett
 *
 * @internal 
 *   @history 2008-03-17 Janet Barrett - Original version
 *   @history 2008-06-18 Steven Koechle - Fixed Documentation Errors
 */
  class NoNormalization : public NormModel {
    public:
      NoNormalization (Pvl &pvl, PhotoModel &pmodel);
      virtual ~NoNormalization() {};

    protected:
      virtual void NormModelAlgorithm (double pha, double inc, double ema,
          double dn, double &albedo, double &mult, double &base);
      virtual void NormModelAlgorithm (double pha, double inc, double ema, 
          double deminc, double demema, double dn, double &albedo,
	  double &mult, double &base) {};
  };
};

#endif
