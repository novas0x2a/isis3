#ifndef InterestOperator_h
#define InterestOperator_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.6 $                                                             
 * $Date: 2008/08/19 22:37:41 $                                                                 
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

#include "geos/geom/MultiPolygon.h"

namespace Isis {
  class Chip;
  class Pvl;
  class Cube;
  class PvlObject;

  /**
   * @brief Interest Operator class
   * 
   * Create InterestOperator object.  Because this is a pure virtual class you
   * can not create an InterestOperator class directly.  Instead, see the
   * InterestOperatorFactory class.
   * 
   * @see StandardDeviationOperator GradientOperator
   *
   * @internal
   *   @history 2006-02-11 Jacob Danton - Original Version
   *   @history 2007-08-02 Steven Koechle - Added better
   *            documentation to CompareInterests().
   *   @history 2007-08-02 Steven Koechle - Fixed looping error
   *            that caused subchip to go outside the chip to the
   *            left and top, and not check the bottom and right.
   *   @history 2007-08-14 Steven Koechle - Added virtual method
   *            Padding() which default returns 0.
   *   @history 2007-08-16 Steven Koechle - Fixed Looping error
   *            in Operate. Made the loops <= instead of just <.
   *            Changed from accepting one delta to accepting a
   *            deltaSamp and a deltaLine.  
   *   @history 2008-06-18 Stuart Sides - Fixed doc error
   *   @history 2008-08-19 Steven Koechle - Updated to work with Geos3.0.0
   */
  class InterestOperator {
    public:
      InterestOperator (Pvl &pvl);
      virtual ~InterestOperator();

      void SetPatternValidPercent(const double percent);
      void SetPatternSampling (const double percent);
      void SetSearchSampling (const double percent);
      void SetTolerance(double tolerance);
      void SetPatternReduction(std::vector<int> samples, std::vector<int> lines);

      //! Return name of the matching operator
      inline std::string operatorName () const { return p_operatorName; };

      bool Operate(Cube &cube, int sample, int line);

      //! Return the goodness of fit of the match operator
      inline double InterestAmount () const { return p_interestAmount; };

      //! Return the search chip cube sample that best matched
      inline double CubeSample() const { return p_cubeSample; };

      //! Return the search chip cube line that best matched
      inline double CubeLine() const { return p_cubeLine; };

      virtual bool CompareInterests(double int1, double int2);
      void AddGroup(Isis::PvlObject &obj);
      double WorstInterest() {return p_worstInterest;}
      void SetClipPolygon (const geos::geom::MultiPolygon &clipPolygon);

    protected:
      void Parse(Pvl &pvl);
      virtual double Interest (Chip &subCube) = 0;
      bool InRange(double value) {return (p_minimumDN<=value && value<=p_maximumDN);}

      double p_minimumDN, p_maximumDN;
      double p_worstInterest, p_interestAmount;
      virtual int Padding();

      geos::geom::MultiPolygon *p_clipPolygon; //!< clipping polygon set by SetClipPolygon (line,samp)

    private:
      double p_cubeSample, p_cubeLine;
      double p_minimumInterest;

      std::string p_operatorName;

      int p_deltaSamp, p_deltaLine, p_lines, p_samples;
  };
};

#endif
