/**                                                                       
 * @file                                                                  
 * $Revision: 1.13 $                                                             
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

#include <cmath>

#include "ProcessByLine.h"
#include "Calculator.h"
#include "InfixToPostfix.h"
#include "iException.h"
#include "SpecialPixel.h"

using namespace std;

namespace Isis {
  /** 
   * The code that performs math operations is designed
   *   to call a function and use the result. These helper
   *   methods convert standard operators into functions which
   *   perform the desired operations. See the implementation of
   *   Calculator::Negative for an example.
   */

  /**
   * Returns the nagative of the input parameter.
   * 
   * @param a Input double
   * 
   * @return double negative of a
   */
  double NegateOperator(double a) { return -1*a; }


  /**
   * Returns the result of a multiplied by b.
   * 
   * @param a Input double
   * @param b Input double
   * 
   * @return double result of a*b
   */
  double MultiplyOperator(double a, double b) { return a*b; }


  /**
   * Returns the result of dividing a by b
   * 
   * @param a Input double
   * @param b Intput double
   * 
   * @return double result of a/b
   */
  double DivideOperator(double a, double b) { return a/b; }


  /**
   * Returns the result of additing a with b.
   * 
   * @param a Input double
   * @param b Input double
   * 
   * @return double result of a+b
   */
  double AddOperator(double a, double b) { return a+b; }


  /**
   * Returns the result of subtracting b from a.
   * 
   * @param a Input subtractee 
   * @param b Input subtractor
   * 
   * @return double result of a-b
   */
  double SubtractOperator(double a, double b) { return a-b; }


  /**
   * Returns 1.0 if a is greater than b.  Otherwise 0.0 is returned.
   * 
   * @param a Input double
   * @param b Input double
   * 
   * @return 1.0 if a>b
   */
  double GreaterThanOperator(double a, double b) { return a > b ? 1.0 : 0.0; }


  /**
   * Returns 1.0 if a is less than b.  Otherwise 0.0 is returned.
   * 
   * @param a Input double
   * @param b Input double
   * 
   * @return 1.0 if a<b
   */
  double LessThanOperator(double a, double b) { return a < b ? 1.0 : 0.0; }


  /**
   * Returns 1.0 if a is equal ot b.
   * 
   * @param a Input double
   * @param b Input double
   * 
   * @return 1.0 if a==b
   */
  double EqualOperator(double a, double b) { return a == b ? 1.0 : 0.0; }


  /**
   * Returns 1.0 if a is greater than or equal to b.  Otherwise 0.0 is returned.
   * 
   * @param a Input double
   * @param b Input double
   * 
   * @return 1.0 if a>=b
   */
  double GreaterThanOrEqualOperator(double a, double b) { return a >= b ? 1.0 : 0.0; }


  /**
   * Returns 1.0 if a is less than or eqaul to b.  Otherwise 0.0 is returned.
   * 
   * @param a Input double
   * @param b Input double
   * 
   * @return 1.0 if a<=b
   */
  double LessThanOrEqualOperator(double a, double b) { return a <= b ? 1.0 : 0.0; }


  /**
   * Returns 1.0 is a is not equal to b.  Otherwise 0.0 is returned.
   * 
   * @param a Input double
   * @param b Input double
   * 
   * @return 1.0 if a!=b
   */
  double NotEqualOperator(double a, double b) { return a != b ? 1.0 : 0.0; }


  /**
   * Returns the cosecant of the input a
   * 
   * @param a Input double
   * 
   * @return the double result of the cosecant of a
   */
  double CosecantOperator(double a) { return 1.0 / sin(a); }


  /**
   * Returns the secant of the input a
   * 
   * @param a Input double
   * 
   * @return the double result of the secant of a
   */
  double SecantOperator(double a) { return 1.0 / cos(a); }

  /**
   * Returns the cotangent of the input a
   * 
   * @param a Input double
   * 
   * @return the double result of the cotangent of a
   */
  double CotangentOperator(double a) { return 1.0 / tan(a); }


  /**
   * Returns the result of rounding the input a to the closest integer.
   * 
   * @param a Inut double
   * 
   * @return the int result of rounding a to the closest whole number
   */
  int Round(double a) { return (a>0)? (int)(a+0.5) : (int)(a-0.5); }


  /**
   * Returns the result of a bitwise AND accross a and b 
   * 
   * @param a Input double
   * @param b Input double
   * 
   * @return the double result of a bitwise AND operation
   */
  double BitwiseAndOperator(double a, double b) { return (double)(Round(a)&Round(b)); }


  /**
   * Returns the result of a bitwise OR across a and b
   * 
   * @param a Input double
   * @param b INput double
   * 
   * @return the double result of a bitwise OR operation
   */
  double BitwiseOrOperator(double a, double b) { return (double)(Round(a)|Round(b)); }


  /**
   * Returns the modulus of a by b
   * 
   * @param a Input modulee
   * @param b Input modulator
   * 
   * @return double result of a%b
   */
  double ModulusOperator(double a, double b) { return (double)(Round(a)%Round(b)); }


  //! Constructor
  Calculator::Calculator() {}


  /**
   * Pops an element, negates it, then pushes the result
   */
  void Calculator::Negative() {
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), NegateOperator);
    Push(result);
  }


  /**
   * Pops two elements, multiplies them, then pushes the product on the stack
   */
  void Calculator::Multiply() {
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), MultiplyOperator);
    Push(result); 
  }


  /**
   * Pops two elements, adds them, then pushes the sum on the stack
   */
  void Calculator::Add() { 
    std::vector<double> y = Pop(); 
    std::vector<double> x = Pop();
    std::vector<double> result;
    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), AddOperator);
    Push(result);
  }


  /**
   * Pops two elements, subtracts them, then pushes the difference on the stack
   */
  void Calculator::Subtract() { 
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;
    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), SubtractOperator);
    Push(result);
  }


  /**
   * Pops two, divides them, then pushes the quotient on the stack
   */
  void Calculator::Divide() { 
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), DivideOperator);
    Push(result);
  }

  /**
   * Pops two elements, mods them, then pushes the result on the stack
   */
  void Calculator::Modulus() { 
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), ModulusOperator);
    Push(result);
  }


  /**
   * Pops two elements, computes the power then pushes the result on the stack
   * The exponent has to be a scalar.
   * 
   * @throws Isis::iException::Math
   */
  void Calculator::Exponent() {
    std::vector<double> exponent = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), exponent.begin(), exponent.end(), pow);
    Push(result);
  }


  /**
   * Pop an element, compute its square root, then push the root on the stack
   * 
   * @throws Isis::iException::Math
   */
  void Calculator::SquareRoot() {
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), sqrt);
    Push(result);
  }


  /**
   * Pop an element, compute its absolute value, then push the result on the stack
   */
  void Calculator::AbsoluteValue() {
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), fabs);
    Push(result);
  }


  /**
   * Pop an element, compute its log, then push the result on the stack
   * 
   * @throws Isis::iException::Math
   */
  void Calculator::Log() {
     std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), log);
    Push(result);
  }


  /**
   * Pop an element, compute its base 10 log, then push the result on the stack
   */
  void Calculator::Log10() {
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), log10);
    Push(result);
  }


  /**
   * Pop the top element, then perform a left shift with zero fill
   * 
   * @throws Isis::iException::Math
   */
  void Calculator::LeftShift() {
    std::vector<double> y = Pop();
    if(y.size() != 1) {
      std::string msg = "Must use scalars for shifting.";
      throw Isis::iException::Message(Isis::iException::Math, msg, _FILEINFO_);
    }
    else { 
      std::vector<double> x = Pop();
  
      if((int)y[0] > (int)x.size()) { 
        std::string msg = "Shift value must be <= to number of samples.";
        throw Isis::iException::Message(Isis::iException::Math, msg, _FILEINFO_);
      }
      else {
        std::vector<double> result;
        int shift = (int)y[0];
        result.resize(x.size());

        for(unsigned int i = 0; i < result.size(); i++) {
          if(i+shift < x.size() && i+shift >= 0)
            result[i] = x[i+shift];
          else
            result[i] = sqrt(-1.0); // create a NaN
        }

        Push(result);
      }
    }
  }


  /**
   * Pop the top element, then perform a right shift with zero fill
   * 
   * @throws Isis::iException::Math
   */
  void Calculator::RightShift() {
    std::vector<double> y = Pop();
    if(y.size() != 1) {
      std::string msg = "Must use scalars for shifting.";
      throw Isis::iException::Message(Isis::iException::Math, msg, _FILEINFO_);
    }
    else { 
      std::vector<double> x = Pop();
  
      if((int)y[0] > (int)x.size()) { 
        std::string msg = "Shift value must be <= to number of samples.";
        throw Isis::iException::Message(Isis::iException::Math, msg, _FILEINFO_);
      }
      else {
        std::vector<double> result;
        int shift = (int)y[0];
        result.resize(x.size());

        for(int i = 0; i < (int)result.size(); i++) {
          if(i-shift < (int)x.size() && i-shift >= 0) {
            result[i] = x[i-shift];
          }
          else {
            result[i] = sqrt(-1.0); // create a NaN
          }
        }

        Push(result);
      }
    }
  }


  /**
   * Pop one element, then push the minimum on the stack
   */
  void Calculator::Minimum() {
    std::vector<double> result = Pop();

    int size = result.size();
    double minVal = result[0];
    for(unsigned int i = 0; i < result.size(); i++) {
      if(!IsSpecial(result[i])) {
        minVal = min(minVal, result[i]);
      }
    }

    result.clear();
    result.resize(size, minVal);
    Push(result);
  }


  /**
   * Pop one element, then push the maximum on the stack
   */
  void Calculator::Maximum() {
    std::vector<double> result = Pop();

    int size = result.size();
    double maxVal = result[0];
    for(unsigned int i = 0; i < result.size(); i++) {
      if(!IsSpecial(result[i])) {
        maxVal = max(maxVal, result[i]);
      }
    }

    result.clear();
    result.resize(size, maxVal);
    Push(result);
  }


  /**
   * Pop two elements, then push the minimum on the stack
   */
  void Calculator::Minimum2() {
    std::vector<double> x = Pop();
    std::vector<double> y = Pop();
    std::vector<double> result;

    double minVal = x[0];
    for(unsigned int i = 0; i < x.size(); i++) {
      if(!IsSpecial(x[i])) {
        minVal = min(minVal, x[i]);
      }
    }

    for(unsigned int i = 0; i < y.size(); i++) {
      if(!IsSpecial(y[i])) {
        minVal = min(minVal, y[i]);
      }
    }

    
    result.clear();
    result.push_back(minVal);
    Push(result);
  }


  /**
   * Pop two elements, then push the maximum on the stack
   */
  void Calculator::Maximum2() {
    std::vector<double> x = Pop();
    std::vector<double> y = Pop();
    std::vector<double> result;

    double maxVal = x[0];
    for(unsigned int i = 0; i < x.size(); i++) {
      if(!IsSpecial(x[i])) {
        maxVal = max(maxVal, x[i]);
      }
    }

    for(unsigned int i = 0; i < y.size(); i++) {
      if(!IsSpecial(y[i])) {
        maxVal = max(maxVal, y[i]);
      }
    }

    result.clear();
    result.push_back(maxVal);
    Push(result);
  }


  /**
   * Pop two elements off the stack and compare them to see where one is greater
   * than the other, then push the results on the stack.
   */
  void Calculator::GreaterThan() {
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), GreaterThanOperator);
    Push(result);
  }


  /**
   * Pop two elements off the stack and compare them to see where one is less
   * than the other, then push the results on the stack.
   */
  void Calculator::LessThan() {
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), LessThanOperator);
    Push(result);
  }


  /**
   * Pop two elements off the stack and compare them to see where one is equal
   * to the other, then push the results on the stack.
   */
  void Calculator::Equal() {
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), EqualOperator);
    Push(result);
  }


  /**
   * Pop two elements off the stack and compare them to see where one is greater
   * than or equal to the other, then push the results on the stack.
   */
  void Calculator::GreaterThanOrEqual() {
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), GreaterThanOrEqualOperator);
    Push(result);
  }


  /**
   * Pop two elements off the stack and compare them to see where one is less
   * than or equal to the other, then push the results on the stack.
   */
  void Calculator::LessThanOrEqual() {
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), LessThanOrEqualOperator);
    Push(result);
  }


  /**
   * Pop two elements off the stack and compare them to see where one is not
   * equal to the other, then push the results on the stack.
   */
  void Calculator::NotEqual() {
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), NotEqualOperator);
    Push(result);
  }


  // Commented out because bitwise ops only work with integers instead of doubles

  /**
   * Pop two elements, AND them, then push the result on the stack
   */
  void Calculator::And() {
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), BitwiseAndOperator);
    Push(result);
  }


  /** 
   * Pop two elements, OR them, then push the result on the stack
   */
  void Calculator::Or() {
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), BitwiseOrOperator);
    Push(result);
  }


  /**
   * Pops one element  and push the sine
   */
  void Calculator::Sine() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), sin);
    Push(result);
  }


  /**
   * Pops one element  and push the cosine
   */
  void Calculator::Cosine() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), cos);
    Push(result);
  }


  /**
   * Pops one element  and push the tangent
   */
  void Calculator::Tangent() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), tan);
    Push(result);
  }


  /**
   * Pops one element  and push the cosecant
   */
  void Calculator::Cosecant() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), CosecantOperator);
    Push(result);
  }


  /**
   * Pops one element  and push the secant
   */
  void Calculator::Secant() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), SecantOperator);
    Push(result);
  }


  /**
   * Pops one element  and push the cotangent
   */
  void Calculator::Cotangent() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), CotangentOperator);
    Push(result);
  }


  /**
   * Pops one element  and push the arcsine
   */
  void Calculator::Arcsine() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), asin);
    Push(result);
  }


  /**
   * Pops one element  and push the arccosine
   */
  void Calculator::Arccosine() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), acos);
    Push(result);
  }


  /**
   * Pops one element  and push the arctangent
   */
  void Calculator::Arctangent() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), atan);
    Push(result);
  }


  /**
   * Pops one element and push the inverse hyperbolic sine
   */
  void Calculator::ArcsineH() {
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), asinh);
    Push(result);
  }


  /**
   * Pops one element and push the inverse hyperbolic cosine
   */
  void Calculator::ArccosineH() {
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), acosh);
    Push(result);
  }


  /**
   * Pops one element and push the inverse hyperbolic tangent
   */
  void Calculator::ArctangentH() {
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), atanh);
    Push(result);
  }


  /**
   * Pops two elements  and push the arctangent
   */
  void Calculator::Arctangent2() {
    std::vector<double> y = Pop();
    std::vector<double> x = Pop();
    std::vector<double> result;

    PerformOperation(result, x.begin(), x.end(), y.begin(), y.end(), atan2);
    p_valStack.push(result);
  }


  /**
   * Pops one element  and push the hyperbolic sine
   */
  void Calculator::SineH() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), sinh);
    Push(result);
  }


  /**
   * Pops one element  and push the hyperbolic cosine
   */
  void Calculator::CosineH() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), cosh);
    Push(result);
  }


  /**
   * Pops one element  and push the hyperbolic tangent
   */
  void Calculator::TangentH() { 
    std::vector<double> result = Pop();
    PerformOperation(result, result.begin(), result.end(), tanh);
    Push(result);
  }


  // Stack methods

  /**
   * Push a vector onto the stack
   * 
   * @param vect The vector that will be pushed on the stack
   */
  void Calculator::Push(std::vector<double> &vect) {
    p_valStack.push(vect);
  }


  /**
   * Push a scalar onto the stack
   * 
   * @param scalar The scalar that will be pushed on the stack
   */
  void Calculator::Push(double scalar) {
    std::vector<double> s;
    s.push_back(scalar);
    Push(s);
  }


  /**
   * Push a buffer onto the stack
   * 
   * @param buff The buffer that will be pushed on the stack
   */
  void Calculator::Push(Buffer &buff) {
    std::vector<double> b(buff.size());
  
    for(int i = 0; i < buff.size(); i++) {
      // Test for special pixels and map them to valid values
      if(IsSpecial(buff[i])) { 
        if(Isis::IsNullPixel(buff[i])) {
          //b[i] = NAN;
          b[i] = sqrt(-1.0);
        }
        else if(Isis::IsHrsPixel(buff[i])) {
          //b[i] = INFINITY;
          b[i] = DBL_MAX * 2;
        }
        else if(Isis::IsHisPixel(buff[i])) {
          //b[i] = INFINITY;
          b[i] = DBL_MAX * 2;
        }
        else if(Isis::IsLrsPixel(buff[i])) {
          //b[i] = -INFINITY;
          b[i] = -DBL_MAX * 2;
        }
        else if(Isis::IsLisPixel(buff[i])) {
          //b[i] = -INFINITY;
          b[i] = -DBL_MAX * 2;
        }
      }
      else    
        b[i] = buff[i];
    } 
    p_valStack.push(b);
  }


  /**
   * Pop an element off the stack
   * 
   * @param keepSpecials If true, special pixels will be
   *                     preserved; otherwise, they will be mapped
   *                     to double values
   * 
   * @return The top of the stack, which gets popped
   */
  std::vector<double> Calculator::Pop(bool keepSpecials) {
    std::vector<double> top;

    if(p_valStack.empty()) {
      std::string msg = "Stack is empty, cannot perform any more operations.";
      throw Isis::iException::Message(Isis::iException::Math, msg, _FILEINFO_);
    }

    top = p_valStack.top();

    if(keepSpecials) {
      for(int i = 0; i < (int)top.size(); i++) {
        if(isnan(top[i])) {
          top[i] = Isis::Null;
        }
        // Test for +INFINITY
        else if(top[i] > DBL_MAX) {
          top[i] = Isis::Hrs;
        }
        // Test for -INFINITY)
        else if(top[i] < -DBL_MAX) {
          top[i] = Isis::Lrs;
        }
        else { 
          // Do nothing 
        }
      }
    }
    
    p_valStack.pop();


    return top;
  }


  /**
   * Print the vector at the top of the stack
   */
  void Calculator::PrintTop() {
    if(p_valStack.empty()) return;
    
    std::cout << "[ ";
    std::vector<double> top = p_valStack.top();
    for(int i = 0; i < (int)top.size(); i++) {
      std::cout << top[i] << " ";
    }
    std::cout << "]" << std::endl;
  }


  /**
   * Check if the stack is empty
   * 
   * @return bool True if the stack is empty
   */
  bool Calculator::Empty() {
    return p_valStack.empty();
  }


  /**
   * Clear out the stack
   */
  void Calculator::Clear() {
    while(!p_valStack.empty()) {
      p_valStack.pop();
    }
  }


  /**
   * Performs the mathematical operations on each argument.
   * 
   * @param results [out] The results of the performed operation
   * @param arg1Start The first argument to have the operation done on
   * @param arg1End One argument beyond the final argument to have the operation 
   *                done upon
   * @param operation The operation to be done on all arguments 
   */
  void Calculator::PerformOperation(std::vector<double> &results,
                                    std::vector<double>::iterator arg1Start, 
                                    std::vector<double>::iterator arg1End,
                                    double operation(double))
  {
    results.resize(arg1End-arg1Start);

    for(unsigned int pos = 0; pos < results.size(); pos++) {
      results[pos] = operation(*arg1Start);
      
      arg1Start++;
    }
  }


  /**
   * Performs the mathematical operation on each pair of arguments, or a set of 
   * agruments against a single argument.
   * 
   * @param results [out] The results of the performed operation
   * @param arg1Start The first of the primary argument to have the operation done 
   *                  on
   * @param arg1End One arguement beyond the final primary argument to have the 
   *                operation done upon
   * @param arg2Start The first of the secondaty argument to have the operation 
   *                  done on
   * @param arg2End One arguement beyond the final secondary argument to have the 
   *                operation done upon
   * @param operation The operation to be done on all pairs of arguments  
   */
  void Calculator::PerformOperation(std::vector<double> &results,
                                    std::vector<double>::iterator arg1Start, 
                                    std::vector<double>::iterator arg1End,
                                    std::vector<double>::iterator arg2Start,
                                    std::vector<double>::iterator arg2End,
                                    double operation(double, double)) {
    if(arg1End-arg1Start != 1 && arg2End-arg2Start != 1 && 
       arg1End-arg1Start != arg2End-arg2Start) {
      std::string msg = "Cannot operate on vectors of differing sizes.";
      throw Isis::iException::Message(Isis::iException::Math, msg, _FILEINFO_);
    }

    int iSize = max(arg1End-arg1Start, arg2End-arg2Start);
    results.resize(iSize);

    for(unsigned int pos = 0; pos < results.size(); pos++) {
      results[pos] = operation(*arg1Start, *arg2Start);

      if(arg1Start+1 != arg1End) arg1Start++;
      if(arg2Start+1 != arg2End) arg2Start++;
    }
  }
} // End of namespace Isis
