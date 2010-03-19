/**
 * @file
 * $Revision: 1.9 $
 * $Date: 2010/02/23 16:58:28 $
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

// Calculator.h
#ifndef CALCULATOR_H_
#define CALCULATOR_H_

#include "Buffer.h"
#include <vector>
#include <stack>

namespace Isis {
/**
 * @brief Calculator for arrays
 *
 * This class is a RPN calculator on arrays.  It uses classic
 * push/pop/operator methods.  That is, push array1, push
 * array2, add, pop arrayResult.
 *
 * @ingroup Math
 *
 * @author 2007-04-01 Sean Crosby
 *
 * @internal
 *  @history 2007-06-11 Jeff Anderson - Fixed bug in
 *           Push(Buffer) method.  NAN was not computed
 *           properly.
 *  @history 2007-08-21 Steven Lambright - Moved the infix to postfix
 *           conversion into its own class.
 *  @history 2008-01-28 Steven Lambright - Added more support for the
 *           power operator
 *  @history 2008-03-28 Steven Lambright - Condensed math methods to
 *           just call PerformOperation(...). Converted valarray's to vectors
 *           (in order to use iterators)
 *  @history 2008-06-18 Christopher Austin - Added as well as fixed
 *           documentation
 *  @history 2010-02-23 Steven Lambright - Added Minimum2, Maximum2 and all
 *           min/max operations now ignore special pixels.
 */
  class Calculator {
    public:
      Calculator();   // Constructor
      //! Virtual Constructor
      virtual ~Calculator() {}

      // Math methods
      void Negative();
      void Multiply();
      void Add();
      void Subtract();
      void Divide();
      void Modulus();

      void Exponent();
      void SquareRoot();
      void AbsoluteValue();
      void Log();
      void Log10();

      void LeftShift();
      void RightShift();
      void Minimum();
      void Maximum();
      void Minimum2();
      void Maximum2();
      void GreaterThan();
      void LessThan();
      void Equal();
      void LessThanOrEqual();
      void GreaterThanOrEqual();
      void NotEqual();
      void And();
      void Or();

      void Sine();
      void Cosine();
      void Tangent();
      void Secant();
      void Cosecant();
      void Cotangent();
      void Arcsine();
      void Arccosine();
      void Arctangent();
      void Arctangent2();
      void SineH();
      void CosineH();
      void TangentH();
      void ArcsineH();
      void ArccosineH();
      void ArctangentH();

      // Stack methods
      void Push(double scalar);
      void Push(Buffer &buff);
      void Push(std::vector<double> &vect);
      std::vector<double> Pop(bool keepSpecials = false);
      void PrintTop();
      bool Empty();
      virtual void Clear();

    protected:
      void PerformOperation(std::vector<double> &results,
                            std::vector<double>::iterator arg1Start, 
                            std::vector<double>::iterator arg1End,
                            double operation(double));
      void PerformOperation(std::vector<double> &results,
                            std::vector<double>::iterator arg1Start, 
                            std::vector<double>::iterator arg1End,
                            std::vector<double>::iterator arg2Start,
                            std::vector<double>::iterator arg2End,
                            double operation(double, double));

      //! Returns the current stack size
      int StackSize() { return p_valStack.size(); }

    private:
      //! The current stack of arguments
      std::stack<std::vector<double> > p_valStack;
  };
};

#endif
