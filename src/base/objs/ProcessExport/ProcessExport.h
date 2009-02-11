#ifndef ProcessExport_h
#define ProcessExport_h
/**
 * @file
 * $Revision: 1.6 $
 * $Date: 2008/12/17 17:27:38 $
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
#include <string>
#include <fstream>
#include "Process.h"
#include "Buffer.h"
#include "SpecialPixel.h"
#include "Endian.h"
#include "EndianSwapper.h"
#include "Stretch.h"

namespace Isis {

/**                                                                       
 * @brief Process class for exporting cubes                
 *                                                                        
 * This class allows a programmer to develop applications which export Isis 
 * cubes into another format. For example, isis2jpg or isis2tif. It is highly 
 * recommended that this object be utilized when developing export applications 
 * to ensure a consistent look-and-feel for Isis users. The class operates by 
 * passing the programmer a line of cube data at a time. It is up to the 
 * programmer to write this data to the foreign output format. An ability exists
 * to stretch the data supplied to the programmer in one of three ways. Either 
 * an automatic linear stretch, an automatic piecewise stretch, or an manual 
 * linear stretch. There are various methods which specify how the input pixels
 * are to be stretched to an output range. You can examine the isis2jpg 
 * application code as a guide for writing an export program. Currently this 
 * class only allows for one band of a cube to be exported.
 *                                   
 * If you would like to see ProcessExport being used in implementation, 
 * see isis2jpg.cpp                                                
 *                                                                        
 * @ingroup HighLevelCubeIO                                                  
 *                                                                        
 * @author 2003-03-31 Jeff Anderson                                   
 *                                                                        
 * @internal                                                              
 *  @history 2003-04-03 Jeff Anderson - Added unit test
 *  @history 2003-04-04 Jeff Anderson - Updated documentation for SetInputRange 
 *                                      methods
 *  @history 2003-05-16 Stuart Sides - Modified schema from astrogeology... 
 *                                     isis.astrogeology...
 *  @history 2005-01-07 Stuart Sides - Added CreateWorldFile method                               
 *  @history 2005-02-08 Elizabeth Ribelin - Modified file to support Doxygen 
 *                                          documentation
 *  @history 2005-06-14 Drew Davidson - Overloaded StartProcess method to output
 *                                      directly to a stream.
 *  @history 2005-06-15 Drew Davidson - Updated to support multi-band output.
 * 
 *  @history 2006-02-27 Jacob Danton - Added Multiple input cube support
 *  @history 2006-05-08 Elizabeth Miller - Modified SetInputRange() to get the 
 *                                         min and max percent values from the 
 *                                         userinterface (0.5 and 99.5 are still
 *                                         the default values)
 *  @history 2006-05-15 Jeff Anderson - Fixed bug with multiple input cube
 *                                      support when the programmer didn't
 *                                      set a input minimum/maximum
 *
 * @history 2006-05-23 Jacob Danton - Added seperate MIN/MAX
 * values for each input channel
 *
 * @history 2006-08-30 Jeff Anderson - Fixed memory leak 
 * @history 2007-12-17 Christopher Austin - Added processes for BIL
 *           and BIP, leaving BSQ as the default, as well as
 *           fixed rounding accuracy.
 * @history 2008-05-12 Steven Lambright - Removed references to CubeInfo 
 * @history 2008-06-18 Steven Koechle - Fixed Documentation Errors 
 * @history 2008-08-14 Christopher Austin - Added the Destructor to fix 
 *           memory leaks, as well as changed the EndianSwapper::Float()
 *           call to EndianSwapper::ExportFloat() in isisOut32() to fix bad
 *           float casting.
 * @history 2008-12-17 Steven Lambright - Changed SetOutputRange calls to use 
 *           constants (i.e. instead of 65535 VALID_MAX2 is used).  
 *  
 *  @todo Modify Class to allow export of RGB
 *  @todo 2005-02-09 Stuart Sides - write documentation for CreateWorldFile 
 *                                  method
 *  @todo 2005-02-09 Jeff Anderson - add coded example to class file and 
 *                                   implementation examples
 */
  class ProcessExport : public Isis::Process {

    public:

      //! Storage order enumeration
      enum ExportFormat { BSQ, BIL, BIP };

      ProcessExport();
      ~ProcessExport();
      void StartProcess (void funct(Isis::Buffer &in));
      void StartProcess (void funct(std::vector<Isis::Buffer *> &in));
      void StartProcess (std::ofstream& fout);
      void SetOutputRange (const double minimum, const double maximum);
      void SetOutputNull (const double value);
      void SetOutputLis (const double value);
      void SetOutputLrs (const double value);
      void SetOutputHis (const double value);
      void SetOutputHrs (const double value);
      double OutputNull ();
      double OutputLis ();
      double OutputLrs ();
      double OutputHis ();
      double OutputHrs ();
      void SetInputRange ();
      void SetInputRange (const double minimum, const double maximum);
      void SetInputRange (const double minimum, const double maximum, const int index);
      void SetInputRange (const double minimum, const double middle, 
                          const double maximum);
      void SetInputRange (const double minimum, const double middle, 
                          const double maximum, const int index);
      void CreateWorldFile (const std::string &worldFile);
      void SetOutputEndian(enum ByteOrder endianness);
      void SetOutputType(Isis::PixelType pixelIn);

      //! Get the valid minimum pixel value for the Nth input cube
      double GetInputMinimum(const int n=0){return (p_inputMinimum[n]);};
      //! Get the valid maximum pixel value for the Nth input cube
      double GetInputMaximum(const int n=0){return (p_inputMaximum[n]);};
      //! Get the valid minimum pixel value to be written to the output file 
      double GetOutputMinimum(){return (p_outputMinimum);}; 
      //! Get the valid maximum pixel value to be written to the output file
      double GetOutputMaximum(){return (p_outputMaximum);}; 

      //! Sets the storage order of the output file
      void SetFormat( ExportFormat format ) { p_format = format; };

    protected:

      //! Current storage order
      ExportFormat p_format;

      void StartProcessBSQ (void funct(std::vector<Isis::Buffer *> &in));
      void StartProcessBIL (void funct(std::vector<Isis::Buffer *> &in));
      void StartProcessBIP (void funct(std::vector<Isis::Buffer *> &in));

      double p_outputMinimum; //!<Desired minimum pixel value in the Buffer
      double p_outputMiddle;  /**<Middle pixel value (minimum+maximun)/2.0 in 
                                  the Buffer */
      double p_outputMaximum; //!<Desired maximum pixel value in the Buffer

      std::vector<double> p_inputMinimum;  /**<Minimum pixel value in the input cube to be  
                                  mapped to the minimum value in the Buffer */
      std::vector<double> p_inputMiddle;   /**<Middle pixel value in the input cube to be  
                                  mapped to the (minimum+maximum)/2.0 value in 
                                  the Buffer */
      std::vector<double> p_inputMaximum;  /**<Maximum pixel value in the input cube to be  
                                  mapped to the maximum value in the Buffer */
      EndianSwapper *p_endianSwap; /**<Object to swap the endianness of the
                                  raw output to either MSB or LSB */      
      ByteOrder p_endianType; //! The byte order of the output file
      
      PixelType p_pixelType;  /**<The bits per pixel of the output image*/

      std::vector<Stretch *> p_str;       /**<Stretch object to ensure a reasonable range
                                    of pixel values in the output data**/
				          
      double p_Null; //! Holds the output NULL value
      double p_Lis;  //! Holds the output LIS value
      double p_Lrs;  //! Holds the output LRS value
      double p_His;  //! Holds the output HIS value
      double p_Hrs;  //! Holds the output HRS value

      bool p_Null_Set; //! Indicates if p_Null has been set or not
      bool p_Lis_Set; //! Indicates if p_Lis has been set or not
      bool p_Lrs_Set; //! Indicates if p_Lrs has been set or not
      bool p_His_Set; //! Indicates if p_His has been set or not
      bool p_Hrs_Set; //! Indicates if p_Hrs has been set or not

    private:
      //!Method for writing 8-bit unsigned pixel data to a file stream
      void isisOut8 (Buffer &in, std::ofstream &fout); 
      
      //!Method for writing 16-bit signed pixel data to a file stream
      void isisOut16s (Buffer &in, std::ofstream &fout);
      
      //!Method for writing 16-bit unsigned pixel data to a file stream
      void isisOut16u (Buffer &in, std::ofstream &fout);
      
      /**Method for writing 32-bit signed floating point pixels data to a 
      file stream*/ 
      void isisOut32 (Buffer &in, std::ofstream &fout); 
      
      /** Convenience method that checks to make sure the user is only using
      valid input to the StartProcess method. Also sets the cube up to be
      processed by performing the necessary stretches.*/  
      void InitProcess();  
      

  };
};

#endif