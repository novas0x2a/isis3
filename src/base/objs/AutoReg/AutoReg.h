#if !defined(AutoReg_h)
#define AutoReg_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.3 $                                                             
 * $Date: 2008/08/13 23:44:24 $                                                                 
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

namespace Isis {
  /**
   * @brief Auto Registration class
   * 
   * Create AutoReg object.  Because this is a pure virtual class you can
   * not create an AutoReg class directly.  Instead, see the AutoRegFactory 
   * class.
   * 
   * @ingroup PatternMatching
   * 
   * @see AutoRegFactory MaximumCorrelation MinimumDifference
   *
   * @internal
   *   @history 2006-01-11 Jacob Danton Added idealFit variable, option for sub-pixel
   *                                     accuracy, new CompareFit method, an arbitrarily
   *                                     chosen EPSILON value, a minimum standard deviation
   *                                     cutoff for the pattern chip, and created a unitTest
   * 
   *   @history 2006-02-13 Jacob Danton Added shrinking pattern and sampling option for the pattern chip.
   *   @history 2006-05-15 Jeff Anderson Moved ZScoreMinimum from Algorith group
   *                                     to the PatternChip group
   *   @history 2006-05-22 Jacob Danton Added statistics reporting
   *   @history 2006-06-28 Brendan George Added copy constructor
   *            (private member function)
   *   @history 2008-06-23 Steven Lambright Fixed naming and some documentation
   *            fixes, added ZScores
   *   @history 2008-08-13 Noah Hilt Added two new optional arguments to AutoReg: WindowSize and 
        DistanceTolerance. These two arguments affect how AutoReg gathers and 
   *    compares data for surface modeling and accuracy and should be contained
   *    inside the group "SurfaceModel" in the Pvl. Also changed the Pvl returned
   *    by RegistrationStatistics so that group names do not contain spaces and
   *    the Pvl only contains groups.
   */
  class Chip;
  class Pvl;
  class AutoReg {
    public:
      AutoReg (Pvl &pvl);

      virtual ~AutoReg();

      enum RegisterStatus {
        Success, //!< Success
        PatternChipNotEnoughData, //!< Not enough valid data in pattern chip
        FitChipNoData, //!< Fit chip did not have any valid data
        FitChipToleranceNotMet, //!< Goodness of fit tolerance not satisfied
        SurfaceModelNotEnoughData, //!< Not enough points to fit a surface model for sub-pixel accuracy
        SurfaceModelSolutionInvalid, //!< Could not model surface for sub-pixel accuracy
        SurfaceModelDistanceInvalid, //!< Surface model moves registration more than one pixel
        PatternZScoreNotMet, //!< Pattern data max or min does not pass the z-score test
        SurfaceModelToleranceNotMet //!< Goodness of fit tolerance (with subpixel accuracy) not satisfied
      };

      //! Return pointer to pattern chip
      inline Chip *PatternChip () { return p_patternChip; };

      //! Return pointer to search chip
      inline Chip *SearchChip() { return p_searchChip; };

      void SetPatternValidPercent(const double percent);
      void SetPatternSampling (const double percent);
      void SetSearchSampling (const double percent);
      void SetTolerance(double tolerance);
      void SetPatternReduction(std::vector<int> samples, std::vector<int> lines);
      void SetMovementReductionPercent(std::vector<double> percents);

      //! Return pattern valid percent
      double PatternValidPercent() const { return p_patternValidPercent; };

      //! Return name of the matching algorithm
      inline std::string AlgorithmName () const { return p_algorithmName; };

      //! Return match algorithm tolerance
      inline double Tolerance () const { return p_tolerance; };

      AutoReg::RegisterStatus Register();

      //! Return the goodness of fit of the match algorithm
      inline double GoodnessOfFit () const { return p_goodnessOfFit; };

      inline bool IsIdeal(double fit);

      //! Return the search chip sample that best matched
      inline double ChipSample() const { return p_chipSample; };

      //! Return the search chip line that best matched
      inline double ChipLine() const { return p_chipLine; };

      //! Return the search chip cube sample that best matched
      inline double CubeSample() const { return p_cubeSample; };

      //! Return the search chip cube line that best matched
      inline double CubeLine() const { return p_cubeLine; };

      /**
       * Get the ZScores
       * 
       * @param score1 First Z Score
       * @param score2 Second Z Score
       */
      void ZScores(double &score1, double &score2) const { 
        score1=p_ZScore1; 
        score2=p_ZScore2; 
      }

      Pvl RegistrationStatistics();

    protected:
      double p_idealFit;
      double p_patternSamplingPercent;

      void Parse(Pvl &pvl);
      virtual double MatchAlgorithm (Chip &pattern, Chip &subsearch) = 0;
      virtual bool CompareFits(double fit1, double fit2);
      int ModelSurface(std::vector<double> &x, std::vector<double> &y,
                       std::vector<double> &z);

    private:
      AutoReg (const AutoReg &original){};

      Chip *p_patternChip;
      Chip *p_searchChip;

      bool p_subpixelAccuracy;

      int p_Total;
      int p_Success;
      std::vector<int> p_PatternChipNotEnoughData;
      std::vector<int> p_PatternZScoreNotMet;
      std::vector<int> p_FitChipNoData;
      std::vector<int> p_FitChipToleranceNotMet;
      int p_SurfaceModelNotEnoughData;
      int p_SurfaceModelSolutionInvalid;
      int p_SurfaceModelToleranceNotMet;
      int p_SurfaceModelDistanceInvalid;
      double p_ZScore1;
      double p_ZScore2;

      double p_minimumPatternZScore;
      double p_patternValidPercent;
      double p_searchSamplingPercent;
      std::vector<int> p_reductionSamples, p_reductionLines;
      std::vector<double> p_reductionPercents;

      double p_chipSample, p_chipLine;
      double p_cubeSample, p_cubeLine;
      double p_goodnessOfFit;
      double p_tolerance;
      double EPSILON;

      std::string p_algorithmName;

      int p_windowSize;
      double p_distanceTolerance;
  };
};

#endif
