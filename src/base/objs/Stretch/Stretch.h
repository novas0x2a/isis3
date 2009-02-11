#ifndef Stretch_h
#define Stretch_h
/**
 * @file
 * $Revision: 1.4 $
 * $Date: 2008/11/13 15:21:04 $
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

#include <vector>
#include <string>
#include "Pvl.h"

namespace Isis {
/**                                                                       
 * @brief Pixel value mapper               
 *                                                                        
 * This class is used to stretch or remap pixel values. For example, it can be
 * used to apply contrast stretches, color code stretches, or remap from a
 * double range to 8-bit (0 to 255). The methodology used is straightforward. 
 * The program must set up a list of stretch pairs, input-to-output mappings, 
 * using the AddPair method. For example, (0,0) and (1,255) are two pairs which 
 * would cause an input of 0 to be mapped to 0, 0.5 would be mapped to 127.5 
 * and 1 would be mapped to 255. More than two pairs can be used which
 * generates piece-wise linear mappings. Special pixels are mapped to themselves 
 * unless overridden with methods such as SetNull. Input values outside the 
 * minimum and maximum input pair values are mapped to LRS and HRS respectively.
 * 
 * If you would like to see Stretch being used in implementation, 
 * see stretch.cpp
 * 
 * @ingroup Utility                                                  
 *                                                                        
 * @author 2002-05-15 Jeff Anderson                                                                       
 *                                                                        
 * @internal                                                              
 *  @history 2002-09-17 Jeff Anderson - Added Parse method
 *  @history 2003-05-16 Stuart Sides - Modified schema from astrogeology... 
 *                                     isis.astrogeology...
 *  @history 2005-02-16 Elizabeth Ribelin - Modified file to support Doxygen 
 *                                          documentation
 *  @history 2005-03-11 Elizabeth Ribelin - Modified unitTest to test all 
 *                                          methods in the class
 *  @history 2006-05-25 Jacob Danton - Fixed typo in documentation
 *  @history 2007-03-02 Elizabeth Miller - Added Load and Save
 *                                       methods
 *  @history 2008-11-12 Steven Lambright - Changed search algorithm into a
 *                                 binary search replacing a linear search.
 * 
 */
  class Stretch {
    private:
      std::vector<double> p_input;   //!< Array for input side of stretch pairs
      std::vector<double> p_output;  //!< Array for output side of stretch pairs
      int p_pairs;                   //!< Number of stretch pairs
  
      double p_null; /**<Mapping of input NULL values go to this value
                         (default NULL)*/
      double p_lis;  /**<Mapping of input LIS values go to this value
                         (default LIS)*/
      double p_lrs;  /**<Mapping of input LRS values go to this value
                         (default LRS)*/
      double p_his;  /**<Mapping of input HIS values go to this value
                         (default HIS)*/
      double p_hrs;  /**<Mapping of input HRS values go to this value
                         (default HRS)*/
      double p_minimum; //!<By default this value is set to p_lrs
      double p_maximum; //!<By default this value is set to p_hrs
  
    public:
      Stretch ();

      //! Destroys the Stretch object
      ~Stretch () {};
  
      void AddPair (const double input, const double output);
      
      void SetNull (const double value);
      void SetLis (const double value);
      void SetLrs (const double value);
      void SetHis (const double value);
      void SetHrs (const double value);
      void SetMinimum (const double value);
      void SetMaximum (const double value);

      void Load(Pvl &pvl, std::string &grpName);
      void Save(Pvl &pvl, std::string &grpName);
      void Load(std::string &file, std::string &grpName);
      void Save(std::string &file, std::string &grpName);
  
      double Map (const double value) const;
  
      void Parse (const std::string &pairs);
      std::string Text () const;

      //! Returns the number of stretch pairs
      int Pairs () const { return p_pairs; };

      double Input (const int index) const;
      double Output (const int index) const;

      //! Clears the stretch pairs
      void ClearPairs() { p_pairs = 0; p_input.clear(); p_output.clear(); };
  
  };
};
  
#endif

