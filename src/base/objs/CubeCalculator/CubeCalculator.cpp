/**                                                                       
 * @file                                                                  
 * $Revision: 1.4 $                                                             
 * $Date: 2009/03/03 17:15:59 $                                                                 
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

#include "CubeCalculator.h"
#include "iString.h"

using namespace std;

namespace Isis {

  //! Constructor
  CubeCalculator::CubeCalculator() {}

  void CubeCalculator::Clear() {
    Calculator::Clear();

    p_calculations.clear();
    p_methods.clear();
    p_data.clear();
    p_dataDefinitions.clear();
  }

  /** 
   * This method will execute the calculations built up when PrepareCalculations was called.
   * 
   * @param cubeData The input cubes' data
   * @param curLine The current line in the output cube
   * @param curBand The current band in the output cube
   * 
   * @return std::vector<double> The results of the calculations (with Isis Special Pixels)
   * 
   * @throws Isis::iException::Math
   */
  std::vector<double> CubeCalculator::RunCalculations(std::vector<Buffer*> &cubeData, int curLine, int curBand) {
    // For now we'll only process a single line in this method for our results. In order
    //    to do more powerful indexing, passing a list of cubes and the output cube will
    //    be necessary.

    int methodIndex = 0;
    int dataIndex = 0;
    for(unsigned int i = 0; i < p_calculations.size(); i++) {
      if(p_calculations[i] == callNextMethod) {
        void (Calculator::*aMethod)() = p_methods[methodIndex];
        (this->*aMethod)();
        methodIndex ++;
      }
      else {
        if(p_dataDefinitions[dataIndex] == constant) {
          Push(p_data[dataIndex]);
        }
        else if(p_dataDefinitions[dataIndex] == band) {
          Push(curBand);
        }
        else if(p_dataDefinitions[dataIndex] == line) {
          Push(curLine);
        }
        else {
          int iCube = (int)(p_dataDefinitions[dataIndex]) - (int)(CubeCalculator::cubeData);
          Push(*cubeData[iCube]);
        }

        dataIndex ++;
      }
    }

    if(StackSize() != 1) {
      string msg = "Too many operands in the equation.";
      throw Isis::iException::Message(Isis::iException::Math, msg, _FILEINFO_);
    }

    return Pop(true);
  }


  /** 
   * This method builds a list of actions to perform based on the postfix expression.
   *   Error checking is done using the inCubeInfos, and the outCubeInfo is necessary
   *   to tell the dimensions of the output cube. Call this method before calling
   *   RunCalculations(). This method will also erase all calculator history before building
   *   up a new set of calculations to run.
   * 
   * @param equation The postfix equation
   * @param inCubes The input cubes
   * @param outCube The output cube
   */
  void CubeCalculator::PrepareCalculations(std::string equation, 
                                           std::vector<Cube *> &inCubes, 
                                           Cube *outCube) {
    Clear();
    iString eq = equation;
    while(eq != "") { 
      string token = eq.Token(" ");
  
      // Step through every part of the postfix equation and set up the appropriate
      // action list based on the current token. Attempting to order if-else conditions
      // in terms of what would probably be encountered more often.    
  
      // Scalars
      if(isdigit(token[0]) || token[0] == '.') {
        iString tok(token);
        AddDataPush(tok.ToDouble());
      }
      // File, e.g. F1 = first file in list. Must come after any functions starting with 'f' that
      //   is not a cube.
      else if(token[0] == 'f') {
        iString tok(token.substr(1));
        int file = tok.ToInteger() - 1;
        if(file < 0 || file >= (int)inCubes.size()) {
          std::string msg = "Invalid file number [" + tok + "]";
          throw Isis::iException::Message(Isis::iException::Math, msg, _FILEINFO_);
        }

        std::vector<double> tmp;
        p_dataDefinitions.push_back((dataValue)(cubeData+file));
        p_calculations.push_back(pushNextData);

        // This keeps the data definitions and data vectors in alignment
        p_data.push_back(tmp);
      }
      else if(token == "band") {
        AddDataPush(band);
      } 
      else if(token == "line") {
        AddDataPush(line);
      } 
      else if(token == "sample") {
        std::vector<double> sampleVector;
        sampleVector.resize(outCube->Samples());

        for(int i = 0; i < outCube->Samples(); i++) {
          sampleVector[i] = i+1;
        }

        AddDataPush(sampleVector);
      } 
      // Addition
      else if(token == "+") {
        AddMethodCall(&Isis::Calculator::Add);
      }
      
      // Subtraction
      else if(token == "-") {
        AddMethodCall(&Isis::Calculator::Subtract);
      }
      
      // Multiplication
      else if(token == "*") {
        AddMethodCall(&Isis::Calculator::Multiply);
      }
      
      // Division
      else if(token == "/") {
        AddMethodCall(&Isis::Calculator::Divide);
      }
  
      // Modulus
      else if(token == "%") {
        AddMethodCall(&Isis::Calculator::Modulus);
      }
  
      // Exponent
      else if(token == "^") {
        AddMethodCall(&Isis::Calculator::Exponent);
      }
  
      // Negative
      else if(token == "--") {
        AddMethodCall(&Isis::Calculator::Negative);
      }
  
      // Left shift
      else if(token == "<<") {
        AddMethodCall(&Isis::Calculator::LeftShift);
      }
  
      // Right shift
      else if(token == ">>") {
        AddMethodCall(&Isis::Calculator::RightShift);
      }
  
      // Minimum
      else if(token == "min") {
        AddMethodCall(&Isis::Calculator::Minimum);
      }
  
      // Maximum
      else if(token == "max") {
        AddMethodCall(&Isis::Calculator::Maximum);
      }
  
      // Absolute value
      else if(token == "abs") {
        AddMethodCall(&Isis::Calculator::AbsoluteValue);
      }
  
      // Square root
      else if(token == "sqrt") {
        AddMethodCall(&Isis::Calculator::SquareRoot);
      }
  
      // Natural Log
      else if(token == "log" || token == "ln") {
        AddMethodCall(&Isis::Calculator::Log);
      }
  
      // Log base 10
      else if(token == "log10") {
        AddMethodCall(&Isis::Calculator::Log10);
      }
      
      // Pi
      else if(token == "pi") {
        AddDataPush(Isis::PI);
      }
  
      // e
      else if(token == "e") {
        AddDataPush(Isis::E);
      }
  
      // Sine
      else if(token == "sin") {
        AddMethodCall(&Isis::Calculator::Sine);
      }
      
      // Cosine
      else if(token == "cos") {
        AddMethodCall(&Isis::Calculator::Cosine);
      }
  
      // Tangent
      else if(token == "tan") {
        AddMethodCall(&Isis::Calculator::Tangent);
      }
  
      // Secant
      else if(token == "sec") {
        AddMethodCall(&Isis::Calculator::Secant);
      }
  
      // Cosecant
      else if(token == "csc") {
        AddMethodCall(&Isis::Calculator::Cosecant);
      }
  
      // Cotangent
      else if(token == "cot") {
        AddMethodCall(&Isis::Calculator::Cotangent);
      }
  
      // Arcsin
      else if(token == "asin") {
        AddMethodCall(&Isis::Calculator::Arcsine);
      }
  
      // Arccos
      else if(token == "acos") {
        AddMethodCall(&Isis::Calculator::Arccosine);
      }
  
      // Arctan
      else if(token == "atan") {
        AddMethodCall(&Isis::Calculator::Arctangent);
      }
  
      // Arctan2
      else if(token == "atan2") {
        AddMethodCall(&Isis::Calculator::Arctangent2);
      }
  
      // SineH
      else if(token == "sinh") {
        AddMethodCall(&Isis::Calculator::SineH);
      }
  
      // CosH
      else if(token == "cosh") {
        AddMethodCall(&Isis::Calculator::CosineH);
      }
  
      // TanH
      else if(token == "tanh") {
        AddMethodCall(&Isis::Calculator::TangentH);
      }
  
      // Less than
      else if(token == "<") {
        AddMethodCall(&Isis::Calculator::LessThan);
      }
      
      // Greater than
      else if(token == ">") {
        AddMethodCall(&Isis::Calculator::GreaterThan);
      }
  
      // Less than or equal
      else if(token == "<=") {
        AddMethodCall(&Isis::Calculator::LessThanOrEqual);
      }
  
      // Greater than or equal
      else if(token == ">=") {
        AddMethodCall(&Isis::Calculator::GreaterThanOrEqual);
      }
  
      // Equal
      else if(token == "==") {
        AddMethodCall(&Isis::Calculator::Equal);
      }
  
      // Not equal 
      else if(token == "!=") {
        AddMethodCall(&Isis::Calculator::NotEqual);
      }

      // Ignore empty token
      else if(token == "") { }
  
      else {
        string msg = "Unidentified operator [";
        msg += token + "]";
        throw Isis::iException::Message(Isis::iException::Math, msg, _FILEINFO_);
      }
    } // while loop
  }

  /** 
   * This is a conveinience method for PrepareCalculations(...).
   * This will cause RunCalculations(...) to execute this method in order.
   * 
   * @param method The method to call, i.e. &Isis::Calculator::Multiply
   */
  void CubeCalculator::AddMethodCall(void (Calculator::*method)( void )) {
    p_calculations.push_back(callNextMethod);
    p_methods.push_back(method);
  }

  /** 
   * This is a conveinience method for PrepareCalculations(...).
   * This will cause RunCalculations(...) to push on a single variable.
   * Currently, only line and band are supported.
   * 
   * @param type The variable type (line or band)
   */
  void CubeCalculator::AddDataPush(dataValue type) {
    if(type != line && type != band) {
      string msg = "Can not call CubeCalculator::AddDataPush(dataValue) with types ";
      msg += "other than line,band";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    std::vector<double> tmp;
    p_dataDefinitions.push_back(type);
    p_calculations.push_back(pushNextData);

    // This keeps the data definitions and data vectors in alignment
    p_data.push_back(tmp); 
  }

  /** 
   * This is a conveinience method for PrepareCalculations(...).
   * This will cause RunCalculations(...) to push on a constant value.
   * 
   * @param data A constant
   */
  void CubeCalculator::AddDataPush(const double &data) {
    std::vector<double> constVal;
    constVal.push_back(data);
    p_data.push_back(constVal);
    p_dataDefinitions.push_back(constant);
    p_calculations.push_back(pushNextData);
  }

  /** 
   * This is a conveinience method for PrepareCalculations(...).
   * This will cause RunCalculations(...) to push on an array of
   * constant values.
   * 
   * @param data An array of constant values
   */
  void CubeCalculator::AddDataPush(std::vector<double> &data) {
    p_data.push_back(data);
    p_dataDefinitions.push_back(constant);
    p_calculations.push_back(pushNextData);
  }

} // End of namespace Isis
