#ifndef NumericalMethods_h
#define NumericalMethods_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.3 $                                                             
 * $Date: 2008/07/08 18:46:59 $                                                                 
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

namespace Isis {
  /**
   * @brief Needs Documentation
   *
   * Needs Documentation
   *
   * @author unknown
   *
   * @internal
   *   @history date unknown author unknown - Original version
   *   @history 2008-06-18 Steven Koechle Updated unitTest
   *  
   */
  class NumericalMethods {
    public:
      NumericalMethods();
      ~NumericalMethods();

      //! Static member functions
      static void r8polint(double *xa, double *ya, int n,
                 double x, double *y, double *dy);
      static void r8spline(double *x, double *y, int n,
                 double yp1, double ypn, double *y2);
      static void r8splint(double *xa, double *ya,
                 double *y2a, int n, double x, double *y);
      static double r8expint(int n, double x);
      static double r8ei(double x);

      //! Function that performs Chandra and Van de Hulst's
      //! series approximation for the g'11 function needed
      //! in second order scattering theory 
      static double G11Prime(double tau);

      //! Obtain arc cosine with error checking - if argument is
      //! outside of valid range (-1 to 1), then get arc cosine
      //! of closest valid value
      static double PhtAcos(double cosang);

    protected:
  };
};

#endif
