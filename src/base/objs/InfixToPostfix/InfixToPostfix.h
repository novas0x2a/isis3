/**
 * @file
 * $Revision: 1.1 $
 * $Date: 2008/04/16 23:57:20 $
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

#ifndef INFIXTOPOSTFIX_H_
#define INFIXTOPOSTFIX_H_

#include "iString.h"
#include <stack>
#include <iostream>

namespace Isis {
  class InfixOperator;
  class InfixFunction;

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
 *   @history 2007-08-22 Steven Lambright - Changed p_operators from
 *                   vector to QList (static std::vector is unsafe)
 *   @history 2008-01-23 Steven Lambright - Added the operators (constants)
 *                   PI and e
 */
  class InfixToPostfix {
    public:
      InfixToPostfix();
      virtual ~InfixToPostfix();

      iString Convert(const iString &infix); 
      iString TokenizeEquation(const iString &equation);

    protected:

      virtual bool IsKnownSymbol(iString representation);
      virtual InfixOperator *FindOperator(iString representation);

      QList<InfixOperator*> p_operators;
    private:
      void Initialize();
      void Uninitialize();

      iString FormatFunctionCalls(iString equation);
      iString CleanSpaces(iString equation);

      void CloseParenthesis(iString &postfix, std::stack<InfixOperator> &theStack);
      void AddOperator(iString &postfix, const InfixOperator &op, std::stack<InfixOperator> &theStack);
      bool IsFunction(iString representation);
      void CheckArgument(iString funcName, int argNum, iString argument);
  };

  /**
   * InfixOperator and InfixFunction are helper classes for InfixToPostfix
   */
  class InfixOperator {
    public:
      InfixOperator(int prec, iString rep, bool isFunc = false) { 
        precedence = prec;
        representation = rep; 
        isFunction = isFunc;
      }
  
      int precedence;
      iString representation;
      bool isFunction;
  };

  class InfixFunction : public InfixOperator {
    public:
      InfixFunction(iString rep, int argCount) :
        InfixOperator(-1,rep,true) {
        numArguments = argCount;
      }
  
      int numArguments;
  };
};

#endif
