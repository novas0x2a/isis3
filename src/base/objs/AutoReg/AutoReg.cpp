#include "Chip.h"
#include "Pvl.h"
#include "AutoReg.h"
#include "Plugin.h"
#include "iException.h"
#include "PolynomialBivariate.h"
#include "LeastSquares.h"
#include "Filename.h"
#include "Histogram.h"

namespace Isis {
  /**
   * Create AutoReg object.  Because this is a pure virtual class you can
   * not create an AutoReg class directly.  Instead, see the AutoRegFactory
   * class.
   *
   * @param pvl  A pvl object containing a valid AutoReg specification
   *
   * @see automaticRegistration.doc
   */
  AutoReg::AutoReg (Pvl &pvl) {
    // Set default parameters
    p_algorithmName = "Unknown";
    p_idealFit = 0.0;

    p_patternChip = new Chip(3,3);
    p_searchChip = new Chip(5,5);
    p_fitChip = new Chip(5,5);
    SetPatternSampling(100.0);
    SetSearchSampling(100.0);
    SetPatternValidPercent(50.0);
    SetPatternZScoreMinimum(1.0);
    SetTolerance(Isis::Null);
    SetFitChipSkewnessTolerance(0.5);
    SetSubPixelAccuracy(true);
    SetSurfaceModelDistanceTolerance(1.0);
    SetSurfaceModelWindowSize(3);

    // Clear statistics
    p_Total = 0;
    p_Success = 0;
    p_PatternChipNotEnoughValidData = 0;
    p_PatternZScoreNotMet = 0;
    p_FitChipNoData = 0;
    p_FitChipToleranceNotMet = 0;
    p_FitChipSkewnessNotMet = 0;
    p_SurfaceModelNotEnoughValidData = 0;
    p_SurfaceModelSolutionInvalid = 0;
    p_SurfaceModelToleranceNotMet = 0;
    p_SurfaceModelDistanceInvalid = 0;
    p_ZScore1 = Isis::Null;
    p_ZScore2 = Isis::Null;
    p_skewness = Isis::Null;

    Parse(pvl);
  }

  //! Destroy AutoReg object
  AutoReg::~AutoReg() {
    if (p_patternChip != NULL) delete p_patternChip;
    if (p_searchChip != NULL) delete p_searchChip;
    if (p_fitChip != NULL) delete p_fitChip;
  }

  /**
   * Initialize parameters in the AutoReg class using a PVL specification.
   * An example of the PVL required for this is:
   *
   * @code
   * Object = AutoRegistration
   *   Group = Algorithm
   *     Name      = MaximumCorrelation
   *     Tolerance = 0.7
   *   EndGroup
   *
   *   Group = PatternChip
   *     Samples = 21
   *     Lines   = 21
   *   EndGroup
   *
   *   Group = SearchChip
   *     Samples = 51
   *     Lines = 51
   *   EndGroup
   * EndObject
   * @endcode
   *
   * There are many other options that can be set via the pvl and are
   * described in other documentation (see below).
   *
   * @see automaticRegistration.doc
   *
   * @param pvl The pvl object containing the specification
   **/
  void AutoReg::Parse(Pvl &pvl) {
    try {
      // Get info from Algorithm group
      PvlGroup &algo = pvl.FindGroup("Algorithm", Pvl::Traverse);
      p_algorithmName =  (std::string) algo["Name"];
      SetTolerance(algo["Tolerance"]);

      if (algo.HasKeyword("SubpixelAccuracy")) {
        SetSubPixelAccuracy((std::string)algo["SubpixelAccuracy"] == "True");
      }

      if (algo.HasKeyword("SkewnessTolerance")) {
        SetFitChipSkewnessTolerance((double)algo["SkewnessTolerance"]);
      }

      // Setup the pattern chip
      PvlGroup &pchip = pvl.FindGroup("PatternChip",Pvl::Traverse);
      PatternChip()->SetSize((int)pchip["Samples"],(int)pchip["Lines"]);

      double minimum = -DBL_MAX;
      double maximum = DBL_MAX;
      if (pchip.HasKeyword("ValidMinimum")) minimum = pchip["ValidMinimum"];
      if (pchip.HasKeyword("ValidMaximum")) maximum = pchip["ValidMaximum"];
      PatternChip()->SetValidRange(minimum,maximum);

      if (pchip.HasKeyword("MinimumZScore")) {
        SetPatternZScoreMinimum((double)pchip["MinimumZScore"]);
      }

      // Setup the search chip
      PvlGroup &schip = pvl.FindGroup("SearchChip",Pvl::Traverse);
      SearchChip()->SetSize((int)schip["Samples"],(int)schip["Lines"]);

      minimum = -DBL_MAX;
      maximum = DBL_MAX;
      if (schip.HasKeyword("ValidMinimum")) minimum = schip["ValidMinimum"];
      if (schip.HasKeyword("ValidMaximum")) maximum = schip["ValidMaximum"];
      SearchChip()->SetValidRange(minimum,maximum);

      // Should we sample the pattern chip
      if (pchip.HasKeyword("Sampling")) {
        SetPatternSampling((double)pchip["Sampling"]);
      }

      // Should we sample the search chip?
      if (schip.HasKeyword("Sampling")) {
        SetSearchSampling((double)schip["Sampling"]);
      }

      // What percentage of the pattern chip should be valid
      if (pchip.HasKeyword("ValidPercent")) {
        SetPatternValidPercent((double)pchip["ValidPercent"]);
      }

      if(pvl.HasGroup("SurfaceModel")) {
        PvlGroup &smodel = pvl.FindGroup("SurfaceModel", Pvl::Traverse);

        if(smodel.HasKeyword("DistanceTolerance")) {
          SetSurfaceModelDistanceTolerance((double)smodel["DistanceTolerance"]);
        }

        if(smodel.HasKeyword("WindowSize")) {
          SetSurfaceModelWindowSize((int)smodel["WindowSize"]);
        }
      }

    } catch (iException &e) {
      std::string msg = "Improper format for AutoReg PVL ["+pvl.Filename()+"]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    return;
  }

  /**
   * If the sub-accuracy is enable the Register() method will
   * attempt to match the pattern chip to the search chip at
   * sub-pixel accuracy, otherwise it will be registered at whole
   * pixel accuracy.
   *
   * @param on Set the state of registration accuracy.  The
   *           default is sub-pixel accuracy is on
   */
  void AutoReg::SetSubPixelAccuracy(bool on) {
    p_subpixelAccuracy = on;
  }

  /**
   * Set the amount of data in the pattern chip that must be valid.  For
   * example, a 21x21 pattern chip has 441 pixels.  If percent is 75 then
   * at least 330 pixels pairs must be valid in order for a comparision
   * between the pattern and search sub-region to occur.  That is, both
   * the pattern pixel and search pixel must be valid to be counted.  Pixels
   * are considered valid based on the min/max range specified on each of
   * the Chips (see Chip::SetValidRange method).
   *
   * If the pattern chip reduction option is used this percentage will
   * apply to all reduced patterns.  Additionally, the pattern sampling
   * effects the pixel count.  For example if pattern sampling is 50% then
   * only 220 pixels in the 21x21 pattern are considered so 165 must be
   * valid.
   *
   * @param percent   Percentage of valid data between 0 and 100,
   *                  default is 50% if never invoked
   */
  void AutoReg::SetPatternValidPercent(const double percent) {
    if ((percent <= 0.0) || (percent > 100.0)) {
      std::string msg = "Invalid value of PatternValidPercent [" +
                        iString(percent) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_patternValidPercent = percent;
  }

  /**
   * Set the pattern sampling rate.  This is used to speed the comparison
   * of the match algorithm between the pattern chip and the search sub-region
   * chip.  That is, only X% of the data will be compared when checking the fit
   * of the pattern vs search sub-region.
   *
   * @param percent   Percentage of data to check, default is 100%
   */
  void AutoReg::SetPatternSampling (const double percent) {
    if ((percent <= 0.0) || (percent > 100.0)) {
      std::string msg = "Invalid value of PatternSamplingPercent [" +
                        iString(percent) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_patternSamplingPercent = percent;
  }

  /**
   * Set the minimum pattern zscore.  This option is used to
   * ignore pattern chips which are bland (low standard
   * deviation). If the minimum or maximum pixel value in the
   * pattern chip does not meet the minimum zscore value (see a
   * statisitcs book for definition of zscore) then invalid
   * registration will occur.
   *
   * @param minimum The minimum zscore value for the pattern chip.
   *                 Default is 1.0
   */
  void AutoReg::SetPatternZScoreMinimum (double minimum) {
    if(minimum <= 0.0) {
      std::string msg = "MinimumZScore must be greater than 0";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_minimumPatternZScore = minimum;
  }

  /**
   * Set the search sampling rate.  This is used to reduce the number of
   * search sub-regions to consider.  The default is to check 100% of
   * the search area.  If reduced then the pattern will be sparsely checked
   * through the search area.  The search algorithm will then populate the
   * data around the best fit in order to refine the search.  This can be
   * used to speed the registration process especially for large search areas.
   *
   * @param percent   Percentage of search area to check, default 100%
   */
  void AutoReg::SetSearchSampling (const double percent) {
    if ((percent <= 0.0) || (percent > 100.0)) {
      std::string msg = "Invalid value of PatternSamplingPercent [" +
                        iString(percent) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_searchSamplingPercent = percent;
  }

  /**
   * Set the tolerance for an acceptable goodness of fit
   *
   * @param tolerance   This tolerance is used to test against the goodness
   *                    of fit returned by the MatchAlgorith method after
   *                    a surface model has been fit.  See TestGoodnessOfFit
   */
  void AutoReg::SetTolerance(const double tolerance) {
    p_tolerance = tolerance;
  }

  /**
   * Set the surface model window size. The pixels in this window
   * will be used to fit a surface model in order to compute
   * sub-pixel accuracy.  In some cases the default (3x3) and
   * produces erroneous sub-pixel accuracy values.
   *
   * @param size The size of the window must be three or greater
   *             and odd.
   */
  void AutoReg::SetSurfaceModelWindowSize (int size) {
    if(size % 2 != 1 || size < 3) {
      std::string msg = "WindowSize must be an odd number greater than or equal to 3";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_windowSize = size;
  }

  /**
   * Set a distance the surface model solution is allow to move
   * away from the best whole pixel fit in the fit chip.
   *
   * @param distance The distance allowed to move in pixels.  Must
   *                 be greater than zero.
   */
  void AutoReg::SetSurfaceModelDistanceTolerance (double distance) {
    if(distance <= 0.0) {
      std::string msg = "DistanceTolerance must be greater than 0";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_distanceTolerance = distance;
  }

  /**
   * For a registration to be considered successful.  The skewness
   * of the data in the fit chip is examinded.  Fit chips that do
   * not meet the skewness requirements are considered false
   * positives (have had a high goodness of fit) but often
   * misregister.  Fit chip histograms that positively skewed are
   * generally good solutions.  Normal distribution or negatively
   * skewed distributions often produce false positive goodness of
   * fit values.
   *
   * @param tolerance the skewness tolerance.  Numbers close to
   *                  zero a normal distributions while positive
   *                  numbers imply positive skewness.  The
   *                  default is 0.5
   */
  void AutoReg::SetFitChipSkewnessTolerance (double tolerance) {
    p_skewnessTolerance = tolerance;
  }

  /**
   * Walk the pattern chip through the search chip to find the best registration
   *
   * @return  Returns the status of the registration. 
   *
   * @todo implement search chip sampling, pattern chip sampling
   */
  AutoReg::RegisterStatus AutoReg::Register() {
    // The search chip must be bigger than the pattern chip by N pixels in
    // both directions for a successful surface model
    int N = p_windowSize / 2 + 1;

    if (p_searchChip->Samples() < p_patternChip->Samples() + N) {
      std::string msg = "Search chips samples [";
      msg += iString(p_searchChip->Samples()) + "] must be at ";
      msg += "least [" +iString(N) + "] pixels wider than the pattern chip samples [";
      msg += iString(p_patternChip->Samples()) + "] for successful surface modeling";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    if (p_searchChip->Lines() < p_patternChip->Lines() + N) {
      std::string msg = "Search chips lines [";
      msg += iString(p_searchChip->Lines()) + "] must be at ";
      msg += "least [" +iString(N) + "] pixels taller than the pattern chip lines [";
      msg += iString(p_patternChip->Lines()) + "] for successful surface modeling";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    // Set computed parameters to NULL so we don't use values from a previous
    // run
    p_ZScore1 = Isis::Null;
    p_ZScore2 = Isis::Null;
    p_skewness = Isis::Null;
    p_goodnessOfFit = Isis::Null;

    p_Total++;

    // Prep for walking the search chip by computing the starting sample
    // and line for the search.  Also compute the sample and linc
    // increment for sparse walking
    int ss = (p_patternChip->Samples() - 1) / 2 + 1;
    int sl = (p_patternChip->Lines() - 1) / 2 + 1;
    int es = p_searchChip->Samples()-ss+1;
    int el = p_searchChip->Lines()-sl+1;

    // Sanity check.  Should have been caught by the two previous tests
    if (ss==es && sl==el) {
      std::string msg = "Sanity check, this shouldn't happen!";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    // Ok create a fit chip whose size is the same as the search chip
    // and then fill it with nulls
    p_fitChip->SetSize(p_searchChip->Samples(),p_searchChip->Lines());
    for (int line=1; line<=p_fitChip->Lines(); line++) {
      for (int samp=1; samp<=p_fitChip->Samples(); samp++) {
        (*p_fitChip)(samp,line) = Isis::Null;
      }
    }

    // TODO:  Enable this code
    // Calculate the sampling info for the search chip
    #if 0
    double percent = sqrt(p_searchSamplingPercent / 100.0);
    double numLines = (int)(percent*p_searchChip->Lines());
    if (numLines < 1) numLines = 1;
    double linc = p_searchChip->Lines()/numLines;
    double numSamples = (int)(percent*p_searchChip->Samples());
    if (numSamples < 1) numSamples = 1;
    double sinc = p_searchChip->Samples()/numSamples;
    if (linc < 1).0 linc = 1.0;
    if (sinc < 1.0) sinc = 1.0;
    #endif
    int sinc = 1;
    int linc = 1;


    // See if the pattern chip has enough good data
    if (!p_patternChip->IsValid(p_patternValidPercent)) {
      p_PatternChipNotEnoughValidData++;
      return PatternChipNotEnoughValidData;
    }

    // Get statistics to see if the pattern passes the z-score test
    Statistics stats;
    for (int i=0; i<p_patternChip->Samples(); i++) {
      double pixels[p_patternChip->Lines()];
      for (int j=0; j<p_patternChip->Lines(); j++) {
        pixels[j] = (*p_patternChip)(i+1,j+1);
      }
      stats.AddData(pixels, p_patternChip->Lines());
    }

    // If it does not pass, return
    p_ZScore1 = stats.ZScore(stats.Minimum());
    p_ZScore2 = stats.ZScore(stats.Maximum());

    if (p_ZScore2 < p_minimumPatternZScore ||
        p_ZScore1 > -1*p_minimumPatternZScore) {
      p_PatternZScoreNotMet++;
      return PatternZScoreNotMet;
    }

    // Walk the search chip and find the best fit
    int bestSamp = 0;
    int bestLine = 0;
    double bestFit = Isis::Null;
    stats.Reset();

    for (int line=sl; line<=el; line+=linc) {
      for (int samp=ss; samp<=es; samp+=sinc) {
        // Extract the sub search chip and make sure it has enough valid
        // data
        Chip subsearch = p_searchChip->Extract(p_patternChip->Samples(),
                                               p_patternChip->Lines(),
                                               samp,line);
        if (!subsearch.IsValid(p_patternValidPercent*p_patternSamplingPercent/100.0)) continue;

        // Try to match the two subchips
        double fit = MatchAlgorithm(*p_patternChip, subsearch);

        // If we had a fit save off information about that fit
        if (fit != Isis::Null) {
          (*p_fitChip)(samp,line) = fit;
          stats.AddData(fit);
          if ((bestFit == Isis::Null) || CompareFits(fit,bestFit)) {
            bestFit = fit;
            bestSamp = samp;
            bestLine = line;
          }
        }
      }
    }

    // Check to see if we went through the fit chip and never got a fit at
    // any location.
    if (bestFit == Isis::Null) {
      p_FitChipNoData++;
      return FitChipNoData;
    }

    // We had a location in the fit chip.  Save the values even if they
    // may not meet tolerances.  This is also saves the value in the
    // event the user does not want a surface model fit
    p_goodnessOfFit = bestFit;
    p_searchChip->SetChipPosition(bestSamp, bestLine);
    p_cubeSample = p_searchChip->CubeSample();
    p_cubeLine   = p_searchChip->CubeLine();

    // Now see if we satisified the goodness of fit tolerance
    if (!CompareFits(bestFit,Tolerance())) {
      p_FitChipToleranceNotMet++;
      return FitChipToleranceNotMet;
    }

    // Gather a histogram to apply the skewness test
    Histogram hist(stats.Minimum(),stats.Maximum());
    for (int i=0; i<p_fitChip->Samples(); i++) {
      double pixels[p_fitChip->Lines()];
      for (int j=0; j<p_fitChip->Lines(); j++) {
        pixels[j] = (*p_fitChip)(i+1,j+1);
      }
      hist.AddData(pixels, p_fitChip->Lines());
    }

    // Determine if we should test for negative or positive skewness
    bool skewFactor = 1.0;
    if (CompareFits(stats.Minimum(),stats.Maximum())) skewFactor = -1.0;
    p_skewness = hist.Skew()*skewFactor;

    // See if the skewness meets the tolerance
/* ##########This test should not happen ############
      if ((p_skewness == Isis::Null) || (p_skewness < p_skewnessTolerance)) {
      p_FitChipSkewnessNotMet++;
      return FitChipSkewnessNotMet;
    }
*/
    // Try to fit a model for sub-pixel accuracy if necessary
    if (p_subpixelAccuracy && !IsIdeal(bestFit)) {
      std::vector<double> samps,lines,fits;
      for (int line=bestLine-p_windowSize/2; line<=bestLine+p_windowSize/2; line++) {
        if (line < 1) continue;
        if (line > p_fitChip->Lines()) continue;
        for (int samp=bestSamp-p_windowSize/2; samp<=bestSamp+p_windowSize/2; samp++) {
          if (samp < 1) continue;
          if (samp > p_fitChip->Samples()) continue;
          if ((*p_fitChip)(samp,line) == Isis::Null) continue;
          samps.push_back((double) samp);
          lines.push_back((double) line);
          fits.push_back((*p_fitChip)(samp,line));
        }
      }

      // Make sure we have enough data for a surface fit.  That is,
      // we are not too close to the edge of the fit chip
      if ((int)samps.size() < p_windowSize*p_windowSize * 2 / 3 + 1) {
        p_SurfaceModelNotEnoughValidData++;
        return SurfaceModelNotEnoughValidData;
      }

      // Have enough data, try to mode the surface
      if (ModelSurface(samps,lines,fits)) {
        p_SurfaceModelSolutionInvalid++;
        return SurfaceModelSolutionInvalid;
      }

      // See if the surface model solution moved too far from our whole pixel
      // solution
      if (fabs(bestSamp - p_chipSample) > p_distanceTolerance ||
          fabs(bestLine - p_chipLine) > p_distanceTolerance) {
        p_SurfaceModelDistanceInvalid++;
        return SurfaceModelDistanceInvalid;
      }

      // Does the surface model fit meet the user tolerance
      if (!CompareFits(p_goodnessOfFit, Tolerance())) {
        p_SurfaceModelToleranceNotMet++;
        return SurfaceModelToleranceNotMet;
      }

      // Ok we have subpixel fits in chip space so convert to cube space
      p_searchChip->SetChipPosition(p_chipSample,p_chipLine);
      p_cubeSample = p_searchChip->CubeSample();
      p_cubeLine   = p_searchChip->CubeLine();
    }

    p_Success++;
    return Success;
  }

  /**
   * We will model a 2-d surface as:
   *
   * z = a + b*x + c*y + d*x**2 + e*x*y + f*y**2
   *
   * Then the partial derivatives are two lines:
   *
   * dz/dx = b + 2dx + ey
   * dz/dy = c + ex + 2fy
   *
   * We will have a local min/max where dz/dx and dz/dy = 0.
   * Solve using that condition using linear algebra yields:
   *
   * xlocal = (ce - 2bf) / (4df - ee)
   * ylocal = (be - 2cd) / (4df - ee)
   *
   * These will be stored in p_chipSample and p_chipLine respectively.
   *
   * @param x   vector of x (sample) values
   * @param y   vector of y (line) values
   * @param z   vector of z (goodness-of-fit) values
   */
  int AutoReg::ModelSurface(std::vector<double> &x,
                            std::vector<double> &y,
                            std::vector<double> &z) {
    PolynomialBivariate p(2);
    LeastSquares lsq(p);
    for (int i=0; i<(int)x.size(); i++) {
      std::vector<double> xy;
      xy.push_back(x[i]);
      xy.push_back(y[i]);
      lsq.AddKnown(xy,z[i]);
    }
    try {
      lsq.Solve();
    } catch (iException &e) {
      e.Clear();
      return 1;
    }

    // Get coefficients (don't need a)
    double b = p.Coefficient(1);
    double c = p.Coefficient(2);
    double d = p.Coefficient(3);
    double e = p.Coefficient(4);
    double f = p.Coefficient(5);

    // Compute the determinant
    double det = 4.0*d*f - e*e;
    if (det == 0.0) return 2;

    // Compute our chip position to sub-pixel accuracy
    p_chipSample = (c*e - 2.0*b*f) / det;
    p_chipLine   = (b*e - 2.0*c*d) / det;
    std::vector<double> temp;
    temp.push_back(p_chipSample);
    temp.push_back(p_chipLine);
    p_goodnessOfFit = lsq.Evaluate(temp);
    return 0;
  }

  /**
   * This virtual method must return if the 1st fit is equal to or better
   * than the second fit.
   *
   * @param fit1  1st goodness of fit
   * @param fit2  2nd goodness of fit
   */
  bool AutoReg::CompareFits (double fit1, double fit2) {
    return(std::abs(fit1-p_idealFit) <= std::abs(fit2-p_idealFit));
  }

  // See if the fit is arbitrarily close
  // to the ideal value
  bool AutoReg::IsIdeal(double fit) {
    return( std::abs(p_idealFit - fit) < 0.00001 );
  }


  /**
   * This returns the cumulative registration statistics.  That
   * is, the Register() method accumulates statistics regard the
   * errors each time is called.  Invoking this method returns a
   * PVL summary of those statisitics
   *
   * @author janderson (3/26/2009)
   *
   * @return Pvl
   */
  Pvl AutoReg::RegistrationStatistics() {
    Pvl pvl;
    PvlGroup stats("AutoRegStatistics");
    stats += Isis::PvlKeyword("Total", p_Total);
    stats += Isis::PvlKeyword("Successful", p_Success);
    stats += Isis::PvlKeyword("Failure", p_Total - p_Success);
    pvl.AddGroup(stats);

    PvlGroup grp("PatternChipFailures");
    grp += PvlKeyword("PatternNotEnoughValidData", p_PatternChipNotEnoughValidData);
    grp += PvlKeyword("PatternZScoreNotMet", p_PatternZScoreNotMet);
    pvl.AddGroup(grp);

    PvlGroup fit("FitChipFailures");
    fit += PvlKeyword("FitChipNoData", p_FitChipNoData);
    fit += PvlKeyword("FitChipToleranceNotMet", p_FitChipToleranceNotMet);
    fit += PvlKeyword("FitChipSkewnessNotMet", p_FitChipSkewnessNotMet);
    pvl.AddGroup(fit);

    PvlGroup model("SurfaceModelFailures");
    model += PvlKeyword("SurfaceModelNotEnoughValidData", p_SurfaceModelNotEnoughValidData);
    model += PvlKeyword("SurfaceModelSolutionInvalid", p_SurfaceModelSolutionInvalid);
    model += PvlKeyword("SurfaceModelToleranceNotMet", p_SurfaceModelToleranceNotMet);
    model += PvlKeyword("SurfaceModelDistanceInvalid", p_SurfaceModelDistanceInvalid);
    pvl.AddGroup(model);

    return pvl;
  }
}
