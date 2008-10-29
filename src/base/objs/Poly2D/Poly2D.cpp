/**                                                                       
 * @file                                                                  
 * $Revision: 1.1.1.1 $                                                             
 * $Date: 2006/10/31 23:18:08 $                                                                 
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

#include <iostream>
#include "Poly2D.h"

namespace Isis {
 /** 
  * Create a Poly2D object    
  * 
  * @param degree The order/degree of the polynomial
  * 
  */
  Poly2D::Poly2D(int degree) : 
  Isis::BasisFunction("Poly2D",2,((degree+1)*(degree+2))/2) {
    p_degree = degree;
  }

 /** 
  * This is the the overriding virtual function that provides the expansion of 
  * the two input variables into the polynomial equation. 
  * See BasisFunction for more information.
  *
  * @param vars A vector of double values to use for the expansion.
  */
  void Poly2D::Expand(const std::vector<double> &vars) {
    std::vector<std::vector<double> > terms;
    terms.resize(p_degree+1);
    terms[0].push_back(1.0);
    for (int i=1; i<=p_degree; i++) {
      for (int t=0; t<(int)terms[i-1].size(); t++) {
        terms[i].push_back(terms[i-1][t] * vars[0]);
        if (t == ((int)terms[i-1].size() - 1)) {
          terms[i].push_back(terms[i-1][t] * vars[1]);
        }
      }
    }
  
    p_terms.clear();
    for (int i=0; i<=p_degree; i++) {
      for (int t=0; t<(int)terms[i].size(); t++) {
        p_terms.push_back(terms[i][t]);
      }
    }
  }
} // end namespace isis

