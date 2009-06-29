#if !defined(AutoReg_h)
#define AutoReg_h
/**
 * @file
 * $Revision: 1.11 $
 * $Date: 2009/06/02 22:46:27 $
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

#include "Statistics.h"
#include "Chip.h"
//#include "AutoRegItem.h"

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
   *    @history 2008-11-18 Steven Koechle - Changed all keywords *NotEnoughData
   *             to *NotEnoughValidData
   *    @history 2009-03-17  Tracie Sucharski - Added method to return fit chip
   *    @history 2009-03-26  Jeff Anderson - Removed pattern chip
   *             reduction as it was broken, added skewness check
   *             for the fit chip, cleaned up the
   *             RegistrationStatistics method and added setters
   *             methods where appropriate
   *    @history 2009-03-30 Jeff Anderson - Modified code to reset
   *             parameters (goodness of fit and skewness) for
   *             successive calls to Register method.  Also check
   *             to see if skewness is null.
   *    @history 2009-05-08 Stacy Alley - Took out the skewness
   *             test and added the ellipse eccentricity test in
   *             the ModelSurface method.  Also added the 'reduce'
   *             option to speed up the pattern matching process.
   *    @history 2009-06-02 Stacy Alley - ModelSurface method now
   *             returns a bool instead of an int.  The p_status
   *             is set within this method now. The Match method
   *             now takes another arg... fChip, passed in from
   *             Register. Also took out a redundant test,
   *             'CompareFits' after the ModelSurface call.  Also
   *             changed all the Chips in this header file from
   *             pointers to non-pointers.
   *             Saved all the reduced chips and have
   *             methods to return them so they can be views from
   *             Qnet.
   *    @history 2009-06-02 Jeff Anderson - Added
   *             AdaptiveRegistration virtual methods
   *
   */
  class Pvl;
  class AutoRegItem;

  class AutoReg {
    public:
      AutoReg (Pvl &pvl);

      virtual ~AutoReg();

      enum RegisterStatus{
        Success, //!< Success
        PatternChipNotEnoughValidData, //!< Not enough valid data in pattern chip
        FitChipNoData, //!< Fit chip did not have any valid data
        FitChipToleranceNotMet, //!< Goodness of fit tolerance not satisfied
        SurfaceModelNotEnoughValidData, //!< Not enough points to fit a surface model for sub-pixel accuracy
        SurfaceModelSolutionInvalid, //!< Could not model surface for sub-pixel accuracy
        SurfaceModelDistanceInvalid, //!< Surface model moves registration more than one pixel
        PatternZScoreNotMet, //!< Pattern data max or min does not pass the z-score test
        SurfaceModelEccentricityRatioNotMet //!< Ellipse eccentricity of the surface model not satisfied
      };

      //! Return pointer to pattern chip
      inline Chip *PatternChip () { return &p_patternChip; };

      //! Return pointer to search chip
      inline Chip *SearchChip() { return &p_searchChip; };

      //! Return pointer to search chip
      inline Chip *FitChip() { return &p_fitChip; };

      //! Return pointer to reduced pattern chip
      inline Chip *ReducedPatternChip() { return &p_reducedPatternChip; };

      //! Return pointer to reduced search chip
      inline Chip *ReducedSearchChip() { return &p_reducedSearchChip; };

      //! Return pointer to reduced fix chip
      inline Chip *ReducedFitChip() { return &p_reducedFitChip; };

      void SetSubPixelAccuracy(bool on);
      void SetPatternValidPercent(const double percent);
      void SetTolerance(double tolerance);
      void SetSurfaceModelWindowSize (int size);
      void SetSurfaceModelDistanceTolerance (double distance);
      void SetReductionFactor (int reductionFactor);
      void SetPatternZScoreMinimum(double minimum);
      void SetSurfaceModelEccentricityRatio(double ratioTolerance);

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
       * Return the ZScores of the pattern chip
       *
       * @param score1 First Z Score
       * @param score2 Second Z Score
       */
      void ZScores(double &score1, double &score2) const {
        score1=p_ZScore1;
        score2=p_ZScore2;
      }

      Pvl RegistrationStatistics();

      /**
       * Return if the algorithm is an adaptive pattern matching
       * technique
       */
      virtual bool IsAdaptive() { return false; }

      virtual AutoReg::RegisterStatus AdaptiveRegistration(int startSamp,
                                                           int startLine);

    protected:
      double p_idealFit;
      double p_patternSamplingPercent;

      void Parse(Pvl &pvl);
      virtual double MatchAlgorithm (Chip &pattern, Chip &subsearch) = 0;
      virtual bool CompareFits(double fit1, double fit2);
      bool ModelSurface(std::vector<double> &x, std::vector<double> &y,
                       std::vector<double> &z);
      Chip Reduce(Chip &chip, int reductionFactor);
    private:
      AutoReg (const AutoReg &original){};
      void Match(Chip &sChip, Chip &pChip, Chip &fChip, int ss, int es, int sl, int el);
      bool ComputeChipZScore(Chip &chip);

      Chip p_patternChip;
      Chip p_searchChip;
      Chip p_fitChip;
      Chip p_reducedPatternChip;
      Chip p_reducedSearchChip;
      Chip p_reducedFitChip;

      bool p_subpixelAccuracy;

      int p_Total;
      int p_Success;
      int p_PatternChipNotEnoughValidData;
      int p_PatternZScoreNotMet;
      int p_FitChipNoData;
      int p_FitChipToleranceNotMet;
      int p_SurfaceModelNotEnoughValidData;
      int p_SurfaceModelSolutionInvalid;
      int p_SurfaceModelDistanceInvalid;
      int p_SurfaceModelEccentricityRatioNotMet;

      double p_ZScore1;
      double p_ZScore2;

      double p_minimumPatternZScore;
      double p_patternValidPercent;
      double p_searchSamplingPercent;

      double p_chipSample, p_chipLine;
      double p_cubeSample, p_cubeLine;
      double p_goodnessOfFit;
      double p_tolerance;

      std::string p_algorithmName;

      int p_windowSize;
      double p_distanceTolerance;

      double p_surfaceModelEccentricityTolerance;
      double p_surfaceModelEccentricity;
      double p_bestFit;
      int p_bestSamp;
      int p_bestLine;
      int p_reduceFactor;
      Isis::AutoReg::RegisterStatus p_status;

  };
};

#endif
