/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2008/06/19 15:13:18 $                                                                 
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

#include "MinimumDifference.h"
#include "Chip.h"

namespace Isis {

/**
 * @brief Minimum difference match algorithm
 * 
 * This virtual function overrides the pure virtual method in the AutoReg
 * class.  In this case, we sum the absolute value of the differences between
 * pixels in the pattern and subsearch chips and divide by the valid pixel 
 * count. We ignore special pixels 
 *  
 * @param pattern [in] A Chip object usually containing an nxm area of a cube. 
 *                     Must be the same diminsions as \b subsearch. 
 * @param subsearch [in] A Chip object usually containing an nxm area of a cube. 
 *                  Must be the same diminsions as \b pattern. This is normally
 *                  a subarea of a larger portion of the image.
 *  
 * @return The sum of the absolute value of the DN differences divided by the 
 *         valid pixel count OR Isis::NULL if the valid pixel percent is not
 *         met.
 */
  double MinimumDifference::MatchAlgorithm (Chip &pattern, Chip &subsearch) {
    // calculate the sampling information
    double percent = sqrt(p_patternSamplingPercent/100.0);
    double numLines = (int)(percent*pattern.Lines());
    if (numLines < 1.0) numLines = 1.0;
    double linc = pattern.Lines()/numLines;
    double numSamples = (int)(percent*pattern.Samples());
    if (numSamples < 1.0) numSamples = 1.0;
    double sinc = pattern.Samples()/numSamples;
    if (linc < 1.0) linc = 1.0;
    if (sinc < 1.0) sinc = 1.0;

    double diff = 0.0;
    double count = 0;
    for (double l=1.0; l<=pattern.Lines(); l+=linc) {
      for (double s=1.0; s<=pattern.Samples(); s+=sinc) {
        int line = (int)l;
        int samp = (int)s;

        double pdn = pattern(samp,line);
        double sdn = subsearch(samp,line);
        if (IsSpecial(pdn)) continue;
        if (IsSpecial(sdn)) continue;
        diff += fabs(pdn - sdn);
        count++;
      }
    }

    double percentValid = (double) count / (numLines * numSamples);
    if (percentValid * 100.0 < this->PatternValidPercent()) return Isis::Null;
    return diff / count;
  }

  /**
   * This virtual method must return if the 1st fit is equal to or better 
   * than the second fit.
   * 
   * @param fit1  1st goodness of fit 
   * @param fit2  2nd goodness of fit
   */
  bool MinimumDifference::CompareFits (double fit1, double fit2) {
    return (fit1 <= fit2);
  }
}

extern "C" Isis::AutoReg *MinimumDifferencePlugin (Isis::Pvl &pvl) {
  return new Isis::MinimumDifference(pvl);
}

