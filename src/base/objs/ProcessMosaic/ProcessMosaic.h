#ifndef ProcessMosaic_h
#define ProcessMosaic_h
/**
 * @file
 * $Revision: 1.3 $
 * $Date: 2008/10/03 21:46:43 $
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

#include "Process.h"
#include "Buffer.h"

enum MosaicPriority {
  input, mosaic
};

namespace Isis {
/**                                                                       
 * @brief Mosaic two cubs together                
 *                                                                        
 * This class allows a programmer to develop a program which merges two cubes 
 * together. The application sets the position where input (child) cube will be
 * placed in the mosaic (parent) cube and priority. The the Mosaic object will 
 * merge the overlapping area.                                                 
 *                                                                        
 * @ingroup HighLevelCubeIO                                                  
 *                                                                        
 * @author 2003-04-28 Stuart Sides                                    
 *                                                                        
 * @internal
 *  @history 2003-04-28 Stuart Sides - Modified unitTest.cpp to do a better test
 *  @history 2003-09-04 Jeff Anderson - Added SetInputWorkCube method
 *  @history 2005-02-11 Elizabeth Ribelin - Modified file to support Doxygen 
 *                                          documentation
 *  @history 2006-09-01 Elizabeth Miller - Added BandBinMatch option to
 *                                         propagate the bandbin group to the
 *                                         mosaic and make sure the input cube
 *                                         bandbin groups match the mosaics
 *                                         bandbin group
 *  @history 2006-10-20 Stuart Sides - Fixed bug BandBin group did not get
 *                                     copied to the output mosaic.
 *  @history 2008-10-03 Steven Lambright - Fixed problem where member variables
 *                                     could be corrupted
 *  
 *  
 *  @todo 2005-02-11 Stuart Sides - add coded example and implementation example 
 *                                  to class documentation                                                     
 */                                                                       

  class ProcessMosaic : public Isis::Process {
  
    public:

      //! Constructs a Mosaic object
      ProcessMosaic() { SetBandBinMatch(true); };

      //!  Destroys the Mosaic object. It will close all opened cubes.
      ~ProcessMosaic() {};
  
      // Line Processing method for one input and output cube
      void StartProcess (const int &outputSample, const int &outputLine,
                         const int &outputBand, const MosaicPriority &priority);
  
      Isis::Cube* SetInputCube (const std::string &parameter,
                                    const int ss=1, const int sl=1,
                                    const int sb=1,
                                    const int ns=0, const int nl=0,
                                    const int nb=0);
  
      Isis::Cube* SetInputCube (const std::string &fname,
                                    Isis::CubeAttributeInput &att,
                                    const int ss=1, const int sl=1,
                                    const int sb=1,
                                    const int ns=0, const int nl=0,
                                    const int nb=0);
  
      Isis::Cube* SetOutputCube (const std::string &parameter);

      /**
       * Sets the bandbin match parameter to the input boolean value
       * @param b The boolean value to set the bandbin match parameter to
       */
      void SetBandBinMatch(bool b) { p_bandbinMatch = b; };
  
    private:
      int p_iss; //!<The starting sample within the input cube 
      int p_isl; //!<The starting line within the input cube
      int p_isb; //!<The starting band within the input cube
      int p_ins; //!<The number of samples from the input cube
      int p_inl; //!<The number of lines from the input cube
      int p_inb; //!<The number of bands from the input cube

      /**
       * True/False value to determine whether to enforce the input cube 
       * bandbin matches the mosaic bandbin group
       */
      bool p_bandbinMatch;
  };
};

#endif

