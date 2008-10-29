#ifndef HiImageClean_h
#define HiImageClean_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.5 $                                                             
 * $Date: 2008/05/13 23:14:22 $
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
#include <fstream>
#include <sstream>
#include <iomanip>

#include "HiCalibrationImage.h"
#include "HiCalibrationDark.h"
#include "HiCalibrationBuffer.h"
#include "HiAncillaryDark.h"
#include "HiAncillaryBuffer.h"
#include "SpecialPixel.h"
#include "Statistics.h"
#include "Buffer.h"
#include "Filename.h"

namespace Isis {


class PvlGroup;

/**                                                   
 * @brief Management class for HiRISE calibration
 * 
 * HiImageClean is an implementation of Mike Mellon's iclean_v2.1.pro IDL
 * program.  This rendering should produce very comparable results to his.
 * 
 * This class reads all the HiRISE BLOBs in the input cube file upon
 * instantiation and computes column-wise and row-wise statistics.
 * 
 * An exhaustive output summary of the statistics can be generated for
 * further analysis and/or derivation of a single set of parameters.
 * 
 * This implementation is not a complete solution for HiRISE radiometric
 * calibration.  It is known to work for calibration images acquired with
 * no binning (BINMODE=1) and no compression.  It is, however, intended to
 * be used for further analysis of other acquistion modes.
 *  
 * @ingroup MarsReconnaissanceOrbiter
 *                                                    
 * @author 2005-10-25 Kris Becker 
 * 
 * @internal
 *   @history 2006-03-10 Kris Becker - Modified to use median as opposed to 
 *                                     average;  Use of either is conditionally
 *                                     controlled during compiliation. The default
 *                                     is median, average can be used instead by
 *                                     adding 'ISISCPPFLAGS+=-DUSE_AVERAGE' to
 *                                     the make command.  Be sure to include the
 *                                     single quotes.
 *   @history 2008-05-12 Steven Lambright - Removed references to CubeInfo
 */
  class HiImageClean {

    private:
      enum {defaultWidthFmt=10, defaultPrecisionFmt=6 }; //!< Format defaults

    public:
      HiImageClean() { }
      HiImageClean(const std::string &fname);
      HiImageClean(Cube &cube);
      /** Destructor */
      virtual ~HiImageClean() { }

      friend std::ostream& operator<<(std::ostream& o, const HiImageClean &cs);
      /**
       * Returns the name of file from where the BLOBs came from
       */
      std::string fileName() const { return (_filename.Name()); }

      /**
       * @brief Returns the width of the calibration filter width
       * 
       * @return int Width of the (median/average) filter width
       */
      int getFilterWidth() const { return (_filterWidth); }


      void setLastGoodLine(int lastLine) { _lastGoodLine = lastLine - 1; }
      int getLastGoodLine() const { return (_lastGoodLine); }

      void propagateBlobs(Cube *cube);
      void cleanLine(Buffer &in, Buffer &out);
      void resetStats();

      BigInt TotalNulled() const { return (_totalMaskNulled+_totalDarkNulled); }
      BigInt TotalMaskNulled() const { return (_totalMaskNulled); }
      BigInt TotalDarkNulled() const { return (_totalDarkNulled); }

      PvlGroup getPvlForm(const std::string &name = "HiImageClean") const;
      void PvlImageStats(PvlGroup &grp) const;

      private:
      typedef TNT::Array1D<double> H1DBuf; //!< 1-D stats buffer
      typedef TNT::Array2D<double> H2DBuf; //!< 2-D data buffer

      struct DataBuffer {
        std::string name;
        H1DBuf data;
        DataBuffer() : name("_none"), data() { }
        DataBuffer(const std::string &id, const H1DBuf &buf) : name(id), 
                                                               data(buf) { }

      };
      
      static const std::string _version;       //!< Version compatability
      static const std::string _revision;      //!< Revision history 
      static const std::string _compatability; //!< iclean compatability
      static const int         _filterWidth;   //!< Calibration data

      H2DBuf              _calimg;      //!<  Calibration Image BLOB data
      H2DBuf              _calbuf;      //!<  Calibration Buffer BLOB data
      H2DBuf              _caldark;     //!<  Calibration Dark pixel BLOB data
      H2DBuf              _ancbuf;      //!<  Buffer Pixel (sideplane) BLOB data
      H2DBuf              _ancdark;     //!<  Dark Pixel (sideplane) BLOB data
      
      Filename            _filename;     //!< Name of input file
      BigInt              _lines;        //!< Number lines in ima
      BigInt              _samples;      //!< Number samples in image
                                         
      int                 _binning;      //!< Binning mode 
      int                 _tdi;          //!< iTime Delay Integration
      int                 _cpmm;         //!< CPMM number of image
      int                 _channelNo;    //!< Channel number (0 or 1)
      int                 _lastGoodLine; //!< Last good line in calibration

      int                 _firstImageSample;  //!< Absolute first image sample
      int                 _firstImageLine;    //!< Absolute first image line
      int                 _firstBufferSample; //!< Absolute first buffer sample
      int                 _firstDarkSample;   //!< Absolute first dark sample
                                              // 
      int                 _firstMaskLine; //!< First line in mask region
      int                 _lastMaskLine;  //!< Last line in mask region
      H1DBuf              _premask;       //!< Mask correction w/out null fixes
      H1DBuf              _mask;          //!< Mask correction buffer
      Statistics          _maskStats;     //!< Mask region statistics

      H1DBuf              _predark;       //!< Dark correction w/out null fixes
      H1DBuf              _dark;          //!< Dark correction buffer
      Statistics          _darkStats;     //!< Dark region statistics
                                         
      BigInt              _totalMaskNulled;  //!< Total pixels nulled
      BigInt              _totalDarkNulled;  //!< Total pixels nulled

//  Private methods
      void init(Cube &cube);
      H2DBuf blobvert(const Blobber &blob) const;
      H2DBuf appendLines(const std::vector<H2DBuf> &buffers) const;
      H2DBuf appendSamples(const std::vector<H2DBuf> &buffers) const;
      H1DBuf slice(const H2DBuf &buffer, long sample) const;
      void cimage_mask();
      void cimage_dark();
      void computeStats();
      H2DBuf column_apply(const H2DBuf &data, const H1DBuf &cal, 
                          const int startStatNdx, BigInt &nNulled, 
                          double calmult = 1.0) const;
      H2DBuf row_apply(const H2DBuf &data, const H1DBuf &cal, 
                       const int startStatNdx, BigInt &nNulled, 
                       double calmult = 1.0) const;


      /**
       * @brief Properly format values that could be special pixels
       * 
       * This method applies ISIS special pixel value conventions to properly 
       * print pixel values.
       * 
       * @param[in] (double) value Input value to test for specialness and print
       *                           as requested by caller
       * @param[in] (int) width Width of field in which to print the value
       * @param[in] (int) prec  Precision used to format the value
       * @return (string) Formatted double value
       */
      inline std::string formValue(const double &value, 
                                   int width = defaultWidthFmt,
                                   int prec  = defaultPrecisionFmt) const {
        if (IsSpecial(value)) return PixelToString(value);

        // Its not special so format to callers specs
        std::ostringstream ostr;
        ostr << std::setw(width) << std::setprecision(prec) << value;
        return (std::string(ostr.str()));
      }

      DataBuffer createIndex(const std::string &name, BigInt count, 
                             BigInt firstIndex) const;
      std::ostream &dumpStatBufs(std::ostream &o, 
                                 const std::vector<DataBuffer> &stat,
                                 int width = defaultWidthFmt, 
                                 int prec = defaultPrecisionFmt) const;

      void PvlCalStats(PvlGroup &grp) const;
   };
};

#endif

