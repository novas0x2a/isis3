/**
 * @file
 * $Revision: 1.4 $
 * $Date: 2009/03/03 17:16:00 $
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
#ifndef CUBE_CALCULATOR_H_
#define CUBE_CALCULATOR_H_

#include "Calculator.h"
#include "Cube.h"

namespace Isis {

/**
 * @brief Calculator for arrays
 *
 * This class is a RPN calculator on cubes. The base Calculator class
 *   is used in conjunction with methods to retrieve data from a cube
 *   and perform calculations.
 *
 * @ingroup Math
 *
 * @author 2008-03-26 Steven Lambright
 *
 * @internal
 *  @history 2008-05-12 Steven Lambright - Removed references to CubeInfo    
 *  @history 2008-06-18 Steven Lambright - Fixed documentation     
 *  @history 2009-03-03 Steven Lambright - Added missing secant method call
 */
  class CubeCalculator : Calculator {
    public:
      CubeCalculator();

      /** 
       * This method completely resets the calculator. The prepared
       *   calculations will be erased when this is called.
       */
      void Clear();

      void PrepareCalculations(std::string postfix, std::vector<Cube *> &inCubes, Cube *outCube);

      std::vector<double> RunCalculations(std::vector<Buffer*> &cubeData, int line, int band);

    private:
      /**
       * This is used to define
       *   the overall action to perform in
       *   RunCalculations(..).
       */
      enum calculations {
        //! The calculation requires calling one of the methods
        callNextMethod,
        //! The calculation requires input data
        pushNextData
      };

      /**
       * This is used to tell what kind of data to
       *   push onto the RPN calculator. Values 
       *   >= cubeData all specify cubeData, the difference
       *   between (int)cubeData and the actual value determines
       *   which cube's data to use.
       */
      enum dataValue {
        constant, //!< a constant value
        line, //!< current line number
        band, //!< current band number
        cubeData //!< a brick of cube data
      };

      void AddMethodCall(void (Calculator::*method)( void ));
      void AddDataPush(dataValue type);
      void AddDataPush(const double &data);
      void AddDataPush(std::vector<double> &data);

      /** 
       * This is what RunCalculations(...) will loop over.
       *   The action to perform (push data or execute calculation)
       *   is defined in this vector.
       */
      std::vector< calculations > p_calculations;

      /** 
       * This stores the addresses to the methods RunCalculations(...) 
       * will call
       */
      std::vector< void (Calculator::*)( void ) > p_methods;

      /** 
       * This stores the addressed to the methods RunCalculations(...) 
       * will push (constants), along with placeholders for simplicity to
       * keep synchronized with the data definitions.
       */
      std::vector< std::vector<double> > p_data;

      /** 
       * This defines what kind of data RunCalculations(...) will push
       * onto the calculator. Constants will be taken from p_data, which
       * is synchronized (index-wise) with this vector.
       */
      std::vector< dataValue > p_dataDefinitions;
  };

}
#endif
