/**                                                                       
 * @file                                                                  
 * $Revision: 1.7 $                                                             
 * $Date: 2009/06/02 17:13:22 $                                                                 
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

#ifndef Chip_h
#define Chip_h

#include <vector>

#include "Affine.h"
#include "SpecialPixel.h"
#include "Pvl.h"
#include "geos/geom/MultiPolygon.h"

namespace Isis {
  class Cube;

  /**                                                   
   * @brief A small chip of data used for pattern matching                                                                                                                                              
   *  
   * A chip is a small rectangular area that can be used for pattern
   * matching.  Data can be loaded into the chip manually or by reading
   * directly from a cube.
   * 
   * @ingroup PatternMatching
   *                                                    
   * @author 2005-05-05 Jeff Anderson                    
   *                                                    
   * @internal
   *   @history 2006-07-11  Tracie Sucharski, Added reLoad method to use
   *                          p_cube instead of cube passed in.
   *   @history 2006-08-03  Tracie Sucharski, Added Load and ReLoad method
   *                          to apply scale factor to chip.
   *   @history 2006-08-04  Stuart Sides, Added SetClipPolygon method. If the
   *                        clip polygon is set all pixel values outside the
   *                        polygon will be set to NULL.
   *   @history 2007-10-01  Steven Koechle, Fixed inc in LoadChip to fix an
   *                        infinite loop problem when x.size() never grew
   *                        to be more than 3.
   *   @history 2009-01-19  Steven Koechle, Fixed memory leak
   *   @history 2009-06-02  Stacy Alley, Added a check in the
   *            SetSize() method to make sure the given samples
   *            and lines are not equal to or less than zero.
   * 
   * @see AutoReg
   * @see AutoRegFactory
   */                                                   
  class Chip {
    public:
      Chip ();
      Chip (const int samples, const int lines);
      virtual ~Chip();

      void SetSize (const int samples, const int lines);

      //! Return the number of samples in the chip
      inline int Samples () const { return p_chipSamples; };

      //! Return the number of lines in the chip
      inline int Lines () const { return p_chipLines; };

      //! Returns the expanded filename of the cube from
      //! which this chip was chipped.
      inline std::string Filename() const { return p_filename; };

      /**
       * Loads a Chip with a value.  For example, 
       * @code
       * Chip c(10,5);
       * c(1,1) = 1.1;
       * c(10,5) = 1.2;
       * @endcode
       * 
       * @param sample    Sample position to load (1-based)
       * @param line      Line position to load (1-based)
       */
      inline double &operator()(int sample,int line) {
        return p_buf[line-1][sample-1];
      }

     /** Get a value from a Chip.  For example, 
       * @code
       * Chip c(10,5);
       * cout << c[3,3] << endl;
       * @endcode
       * 
       * @param sample    Sample position to get (1-based)
       * @param line      Line position to get (1-based)
       */
      inline const double &operator()(int sample,int line) const {
        return p_buf[line-1][sample-1];
      }

      void TackCube (const double cubeSample, const double cubeLine);

      /** 
       * Return the fixed tack sample of the chip. That is, the middle of the 
       * chip. It is a chip coordinate not a cube coordinate. For a chip with 5 
       * samples, this will return 3, the middle pixel. For a chip with 4
       * samples it will return 2
       */
      inline int TackSample() const { return p_tackSample; };

      /** 
       * Return the fixed tack line of the chip. That is, the middle of the 
       * chip. It is a chip coordinate not a cube coordinate. For a chip with 5 
       * lines, this will return 3, the middle pixel. For a chip with 4 lines 
       * it will return 2
       */
      inline int TackLine() const { return p_tackLine; };

      void Load(Cube &cube, const double rotation=0.0, const double scale=1.0,
                const int band=1);
      void Load(Cube &cube, Chip &match, const double scale=1.0,
                const int band=1);

      void ReLoad(const double rotation=0.0, const double scale=1.0,
                  const int band=1) {
        Load(*p_cube,rotation,scale,band);
      };
      void ReLoad(Chip &match, const double scale=1.0, const int band=1) {
        Load(*p_cube,match,scale,band);
      };

      void SetChipPosition (const double sample, const double line);

      //! Returns cube sample after invoking SetChipPosition
      inline double CubeSample() const { return p_cubeSample; };

      //! Returns cube line after invoking SetChipPosition
      inline double CubeLine() const { return p_cubeLine; };

      void SetCubePosition (const double sample, const double line);

      //! Returns chip sample after invoking SetCubePosition
      double ChipSample() const { return p_chipSample; };

      //! Returns chip line after invoking SetCubePosition
      double ChipLine() const { return p_chipLine; };

      void SetValidRange (const double minimum = Isis::ValidMinimum,
                          const double maximum = Isis::ValidMaximum);
      //bool IsValid(int sample, int line);
      bool IsValid(double percentage);

      inline bool IsValid(int sample, int line) {
        double value = (*this)(sample,line);
        if (value < p_validMinimum) return false;
        if (value > p_validMaximum) return false;
        return true;
      }

      Chip Extract (int samples, int lines, int samp, int line);
      void Extract (int samp, int line, Chip &output);
      void Write (const std::string &filename);

      void SetClipPolygon (const geos::geom::MultiPolygon &clipPolygon);

    private:
      void Init (const int samples, const int lines);
      void Read (Cube &cube, const int band);

      int p_chipSamples;          //!< Number of samples in the chip
      int p_chipLines;            //!< Number of lines in the chip
      std::vector<std::vector <double> > p_buf;  //!< Chip buffer
      int p_tackSample;           //!< Middle sample of the chip
      int p_tackLine;             //!< Middle line of the chip

      Cube *p_cube;               //!< cube associated with chip after load
      double p_cubeTackSample;    //!< cube sample at the chip tack 
      double p_cubeTackLine;      //!< cube line at the chip tack

      double p_validMinimum;      //!< valid minimum chip pixel value
      double p_validMaximum;      //!< valid maximum chip pixel value

      double p_chipSample;        //!< chip sample set by SetChip/CubePosition
      double p_chipLine;          //!< chip line set by SetChip/CubePosition
      double p_cubeSample;        //!< cube sample set by SetCubePosition
      double p_cubeLine;          //!< cube line set by SetCubePosition
      geos::geom::MultiPolygon *p_clipPolygon; //!< clipping polygon set by SetClipPolygon (line,samp)

      Affine p_affine;            //!< Transform used to load cubes into chip
      std::string p_filename;
   };
};

#endif
