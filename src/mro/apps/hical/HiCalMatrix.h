#if !defined(HiCalMatrix_h)
#define HiCalMatrix_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $
 * $Date: 2007/07/10 01:30:29 $
 * $Id: HiCalMatrix.h,v 1.2 2007/07/10 01:30:29 kbecker Exp $
 * 
 *   Unless noted otherwise, the portions of Isis written by the USGS are 
 *   public domain. See individual third-party library and package descriptions 
 *   for intellectual property information, user agreements, and related  
 *   information.                                                         
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or   
 *   implied, is made by the USGS as to the accuracy and functioning of such 
 *   software and related material nor shall the fact of distribution     
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.                                        
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html                
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include <string>
#include <vector>
#include <iostream>
#include "Pvl.h"
#include "DbAccess.h"
#include "tnt_array1d.h"
#include "iException.h"

namespace Isis {

  /**
   *  @brief HiCalMatrix manages HiRISE calibration matrices
   * that alter some or all of the
   * parameters contained in the Object section of the file.
   *  This object accepts a configuration file that contains a generic format of
   *  HiRISE Calibration matrices and loads the appropriate set based upon TDI,
   *  BIN, and channel number minimally.
   * 
   *  Below is the basic unprofiled version of the configuration (database)
   *  file:
   * 
   *  @code
   *  Object = Database
   *    A = "$mro/calibration/matrices/A_TDI{TDI}_BIN{BIN}_????.cub"
   *    B = "$mro/calibration/matrices/B_TDI{TDI}_BIN{BIN}_????.cub"
   *    G = "$mro/calibration/matrices/G_TDI{TDI}_BIN{BIN}_????.cub"
   *  EndObject
   * 
   * @endcode 
   * 
   * The \b {TDI} and \b {BIN} will be replaced with the textual translation of
   * the TDI (128, 64, 32, 16, 8) and BIN (1, 2, 3, 4, 8, 16) numbers.  This
   * makes for easy defaulting to set up specific profiling of combinations.
   * 
   * Additionally, you can add profiles for a TDI/BIN mode combination should
   * the need arise. Simply add one or more \a Profile groups that specify
   * different files for the observation conditions.
   * @endcode 
   * 
   * This object will retain B, G and IF references from the main Database
   * object but replace A with the one specified in the "TDI128/BIN2" Profile
   * called "MyNew_A_Matrix.cub" for all HiRISE images that were acquired with
   * TDI128 and BIN2 imaging parameters.
   * @code 
   *  Object = Database
   *     Matrices = ("A", "B", "G")
   * 
   *     A = "$mro/calibration/matrices/A_TDI{TDI}_BIN{BIN}_????.cub" 
   *     B ="$mro/calibration/matrices/B_TDI{TDI}_BIN{BIN}_????.cub" 
   *     G = "$mro/calibration/matrices/G_TDI{TDI}_BIN{BIN}_????.cub" 
   * 
   *    Group = Profile
   *      Name = "TDI128/BIN2"
   *      A = "MyNew_A_Matrix.cub"
   *    EndGroup
   *  EndObject
   * @ingroup Utility
   * @author 2007-06-27 Kris Becker
   */
  class HiCalMatrix : public DbAccess { 
    public:
      typedef TNT::Array1D<double> HiMatrix;
      typedef enum { Matrix, Scalar, Keyword } CalType;

    public: 
      //  Constructors and Destructor
      HiCalMatrix();
      HiCalMatrix(Pvl &label);
      HiCalMatrix(Pvl &label, const std::string &conf);

      /** Destructor ensures everything is cleaned up properly */
      virtual ~HiCalMatrix () { }

      void setLabel(Pvl &label);
      PvlKeyword &getKey(const std::string &key, const std::string &group = "");

      std::string filepath(const std::string &fname) const;
      void setConf(const std::string &conf);
      void selectProfile(const std::string &profile);

      std::string getProfileName() const;
      std::string getMatrixSource(const std::string &name) const;
      std::vector<std::string> getMatrixList() const;
      std::vector<std::string> getScalarList() const;
      std::vector<std::string> getKeywordList() const;
      HiMatrix getMatrix(const std::string &name, int expected_size = 0) const;
      HiMatrix getScalar(const std::string &name, int expected_size = 0) const;
      HiMatrix getKeyword(const std::string &name, int expected_size = 0);
      int getMatrixBand() const;

      double sunDistanceAU();

    private:
      static bool  _naifLoaded;  //!< Ensures one instance of NAIF kernels
      std::string  _profName;    //!< Specified name of profile
      Pvl          _label;       //!< Hold label for future references
      int          _ccd;         //!< CCD Number
      int          _channel;     //!< Channel number
      int          _tdi;         //!< TDI mode of operation
      int          _binMode;     //!< Binning mode
      std::string  _filter;      //!< Filter set name (RED, IR, BG)
      

      void init();
      void init(Pvl &label);
      void loadNaifTiming();
      std::vector<std::string> getList(const DbProfile &profile, 
                                       const std::string &key) const;
      DbProfile getMatrixProfile() const;
      std::string parser(const std::string &s) const;
                              

  };

}     // namespace Isis
#endif
