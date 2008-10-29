#if !defined(Moonpr_h)
#define Moonpr_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.1 $                                                             
 * $Date: 2008/03/07 23:04:53 $                                                                 
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

#include "PhotoModel.h"

namespace Isis {
  class Pvl;

/**
 * @brief Moonpr photometric model
 *  Computes normalized albedo for the Moon,
 *  normalized to 0 degrees emission angle and
 *  30 degrees illumination and phase angles.
 *
 * @author 1995-11-27 Alfred McEwen
 *
 * @internal
 */
  class Moonpr : public PhotoModel {
    public:
      Moonpr (Pvl &pvl);
      virtual ~Moonpr () {};
      
    protected:
      virtual double PhotoModelAlgorithm (double phase, double incidence,
            double emission);

  };
};

#endif
