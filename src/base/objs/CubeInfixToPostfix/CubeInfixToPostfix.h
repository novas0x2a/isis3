/**
 * @file
 * $Revision: 1.1 $
 * $Date: 2008/04/16 23:58:08 $
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

#ifndef CUBEINFIXTOPOSTFIX_H_
#define CUBEINFIXTOPOSTFIX_H_

#include "iString.h"
#include "InfixToPostfix.h"
#include <stack>
#include <iostream>

namespace Isis {

/**
 * @brief Converter for math equations
 *
 * This class converts infix equations to postfix
 *
 * @ingroup Math
 *
 * @author 2007-08-21 Steven Lambright
 * 
 * @internal
 */
  class CubeInfixToPostfix : public InfixToPostfix {
    public:
      CubeInfixToPostfix() {} ;
      ~CubeInfixToPostfix() {};

    protected:
      bool IsKnownSymbol(iString representation);
      InfixOperator *FindOperator(iString representation);
  };
};

#endif
