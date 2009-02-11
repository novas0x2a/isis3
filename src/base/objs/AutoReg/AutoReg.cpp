#include "Chip.h"
#include "Pvl.h"
#include "AutoReg.h"
#include "Plugin.h"
#include "iException.h"
#include "PolynomialBivariate.h"
#include "LeastSquares.h"
#include "Filename.h"
#include "Statistics.h"

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
    p_algorithmName = "Unknown";
    p_idealFit = 0.0;
    p_subpixelAccuracy = true;
    p_minimumPatternZScore = 1;
    p_patternChip = new Chip(3,3);
    p_searchChip = new Chip(5,5);
    SetPatternSampling(100.0);
    SetSearchSampling(100.0);
    SetPatternValidPercent(50.0);
    std::vector<int> temp;
    SetPatternReduction(temp,temp);
    SetTolerance(Isis::Null);
    EPSILON = 0.0001;

    // Clear statistics
    p_Total = 0;
    p_Success = 0;
    p_PatternChipNotEnoughValidData.clear();
    p_PatternZScoreNotMet.clear();
    p_FitChipNoData.clear();
    p_FitChipToleranceNotMet.clear();
    p_SurfaceModelNotEnoughValidData = 0;
    p_SurfaceModelSolutionInvalid = 0;
    p_SurfaceModelToleranceNotMet = 0;
    p_SurfaceModelDistanceInvalid = 0;
    p_ZScore1 = Isis::Null;
    p_ZScore2 = Isis::Null;
    p_distanceTolerance = 1.0;
    p_windowSize = 3;

    Parse(pvl);
  }

  //! Destroy AutoReg object
  AutoReg::~AutoReg() {
    if (p_patternChip != NULL) delete p_patternChip;
    if (p_searchChip != NULL) delete p_searchChip;
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

      if (algo.HasKeyword("SubpixelAccuracy"))
        p_subpixelAccuracy = ((std::string)algo["SubpixelAccuracy"] == "True");

      // Setup the pattern chip
      PvlGroup &pchip = pvl.FindGroup("PatternChip",Pvl::Traverse);
      PatternChip()->SetSize((int)pchip["Samples"],(int)pchip["Lines"]);

      double minimum = -DBL_MAX;
      double maximum = DBL_MAX;
      if (pchip.HasKeyword("ValidMinimum")) minimum = pchip["ValidMinimum"];
      if (pchip.HasKeyword("ValidMaximum")) maximum = pchip["ValidMaximum"];
      PatternChip()->SetValidRange(minimum,maximum);

      if (pchip.HasKeyword("MinimumZScore")) {
        p_minimumPatternZScore = pchip["MinimumZScore"];
      }

      // Get the shrinking pattern dimensions
      std::vector<int> samps;
      for (int i=0; i<pchip["Samples"].Size(); i++) {
        samps.push_back((int)pchip["Samples"][i]);
      }

      std::vector<int> lines;
      for (int i=0; i<pchip["Lines"].Size(); i++) {
        lines.push_back((int)pchip["Lines"][i]);
      }
      SetPatternReduction(samps,lines);

      // and movement restrictions
      std::vector<double> percents;
      if (pchip.HasKeyword("FloatPercent")) {
        for (int i=0; i<pchip["FloatPercent"].Size(); i++) {
          percents.push_back((double)pchip["FloatPercent"][i]);
        }
      } else percents.push_back(1.0); // todo: replace with const var
      SetMovementReductionPercent(percents);

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
          p_distanceTolerance = (double)smodel["DistanceTolerance"];
  
          if(p_distanceTolerance <= 0.0) {
            std::string msg = "DistanceTolerance must be greater than 0";
            throw iException::Message(iException::User,msg,_FILEINFO_);
          }
        }

        if(smodel.HasKeyword("WindowSize")) {
          p_windowSize = (int)smodel["WindowSize"];
    
          if(p_windowSize % 2 != 1 || p_windowSize < 3) {
            std::string msg = "WindowSize must be an odd number greater than or equal to 3";
            throw iException::Message(iException::User,msg,_FILEINFO_);
          }
        }
      }

    } catch (iException &e) {
      std::string msg = "Improper format for AutoReg PVL ["+pvl.Filename()+"]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    return;
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
   * Walk the pattern chip through the search chip to find the best registration
   *
   * @return  Returns the status of the registration.  The following conditions
   *          can occur
   *          0=Success,
   *          1=Not enough valid data in pattern chip,
   *          2=Fit chip did not have any valid data
   *          3=Goodness of fit tolerance not satisfied,
   *          4=Not enough points to fit a surface model for sub-pixel accuracy
   *          5=Could not model surface for sub-pixel accuracy
   *          6=Surface model moves registration more than one pixel
   *          7=Pattern data max or min does not pass the z-score test
   *
   * @todo implement search chip sampling, pattern chip sampling, and pattern
   * chip reduction
   */
  AutoReg::RegisterStatus AutoReg::Register() {
    // The search chip must be bigger than the pattern chip by two pixels in
    // both directions for a successful surface model
    if (p_searchChip->Samples() < p_patternChip->Samples() + 2) {
      std::string msg = "Search chips samples [";
      msg += iString(p_searchChip->Samples()) + "] must be at ";
      msg += "least two pixels wider than the pattern chip [";
      msg += iString(p_patternChip->Samples()) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    if (p_searchChip->Lines() < p_patternChip->Lines() + 2) {
      std::string msg = "Search chips lines [";
      msg += iString(p_searchChip->Lines()) + "] must be at ";
      msg += "least two pixels taller than the pattern chip [";
      msg += iString(p_patternChip->Lines()) + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_ZScore1 = Isis::Null;
    p_ZScore2 = Isis::Null;

    p_Total++;

    int bestSamp = 0;
    int bestLine = 0;
    double bestFit = Isis::Null;
    Chip fitChip(p_searchChip->Samples(),p_searchChip->Lines());
    // For each Shrinking pattern
    for (unsigned int sp=0; sp<p_reductionSamples.size(); sp++) {
      // Prep for walking the search chip by computing the starting sample
      // and line for the search.  Also compute the sample and linc
      // increment for sparse walking
      int ss,sl, es, el;
      if (sp==0) {
        ss = (p_patternChip->Samples() - 1) / 2 + 1;
        sl = (p_patternChip->Lines() - 1) / 2 + 1;
        es = p_searchChip->Samples()-ss+1;
        el = p_searchChip->Lines()-sl+1;
      } 
      else {
        ss = bestSamp - (int)(p_reductionPercents[sp-1]*p_reductionSamples[sp-1]/2);
        sl = bestLine - (int)(p_reductionPercents[sp-1]*p_reductionLines[sp-1]/2);
        es = bestSamp + (int)(p_reductionPercents[sp-1]*p_reductionSamples[sp-1]/2);
        el = bestLine + (int)(p_reductionPercents[sp-1]*p_reductionLines[sp-1]/2);
      }

      // Make sure we are going to register
      // more than one point
      if (ss==es && sl==el) break;

      // Ok create a fit chip whose size is the same as the search chip
      // and then fill it with nulls
      for (int line=1; line<=fitChip.Lines(); line++) {
        for (int samp=1; samp<=fitChip.Samples(); samp++) {
          fitChip(samp,line) = Isis::Null;
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

      // Ok, now extract the shrinking pattern size subchip
      Chip subpattern = p_patternChip->Extract(p_reductionSamples[sp],
                                                   p_reductionLines[sp],
                                                   (p_patternChip->Samples()+1)/2,
                                                   (p_patternChip->Lines()+1)/2);

      // See if the pattern chip has enough good data
      if (!subpattern.IsValid(p_patternValidPercent)) {
        p_PatternChipNotEnoughValidData[sp]++;
        return PatternChipNotEnoughValidData;
      }
  
        // See if the pattern passes the z-score test
      Statistics stats;
      for (int i=0; i<subpattern.Samples(); i++) {
        double pixels[subpattern.Lines()];
        for (int j=0; j<subpattern.Lines(); j++) {
          pixels[j] = subpattern(i+1,j+1);
        }
        stats.AddData(pixels, subpattern.Lines());
      }

      p_ZScore1 = stats.ZScore(stats.Minimum());
      p_ZScore2 = stats.ZScore(stats.Maximum());

      // If it does not pass, return
      if (p_ZScore2 < p_minimumPatternZScore ||
          p_ZScore1 > -1*p_minimumPatternZScore) {
        p_PatternZScoreNotMet[sp]++;
        return PatternZScoreNotMet;
      }
  
      // Walk the search chip and find the best fit
      for (int line=sl; line<=el; line+=linc) {
        for (int samp=ss; samp<=es; samp+=sinc) {
          // Extract the sub search chip and make sure it has enough valid
          // data
          Chip subsearch = p_searchChip->Extract(p_reductionSamples[sp],
                                                 p_reductionLines[sp],
                                                 samp,line);
          if (!subsearch.IsValid(p_patternValidPercent*p_patternSamplingPercent/100.0)) continue;
  
          // and try to match the two subchips
          double fit = MatchAlgorithm(subpattern, subsearch);
          if (fit != Isis::Null) {
            fitChip(samp,line) = fit;
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
        p_FitChipNoData[sp]++;
        return FitChipNoData;
      }
  
      // Now see if we satisified the tolerance
      if (!CompareFits(bestFit,Tolerance())) {
        p_goodnessOfFit = bestFit;
        p_searchChip->SetChipPosition(bestSamp, bestLine);
        p_cubeSample = p_searchChip->CubeSample();
        p_cubeLine   = p_searchChip->CubeLine();
        p_FitChipToleranceNotMet[sp]++;
        return FitChipToleranceNotMet;
      }
    }

    if (p_subpixelAccuracy && !IsIdeal(bestFit)) {
      // Ok try to fit a surface for sub-pixel accuracy using the window size
      std::vector<double> samps,lines,fits;
      for (int line=bestLine-p_windowSize/2; line<=bestLine+p_windowSize/2; line++) {
        if (line < 1) continue;
        if (line > fitChip.Lines()) continue;
        for (int samp=bestSamp-p_windowSize/2; samp<=bestSamp+p_windowSize/2; samp++) {
          if (samp < 1) continue;
          if (samp > fitChip.Samples()) continue;
          if (fitChip(samp,line) == Isis::Null) continue;
          samps.push_back((double) samp);
          lines.push_back((double) line);
          fits.push_back(fitChip(samp,line));
        }
      }
      if (samps.size() < 7) { 
        p_SurfaceModelNotEnoughValidData++;
        return SurfaceModelNotEnoughValidData;
      }
      if (ModelSurface(samps,lines,fits)) {
        p_SurfaceModelSolutionInvalid++;
        return SurfaceModelSolutionInvalid;
      }
      if (fabs(bestSamp - p_chipSample) > p_distanceTolerance ||
          fabs(bestLine - p_chipLine) > p_distanceTolerance) {
        p_SurfaceModelDistanceInvalid++;
        return SurfaceModelDistanceInvalid;
      }
      if (!CompareFits(p_goodnessOfFit, Tolerance())) {
        p_SurfaceModelToleranceNotMet++;
        return SurfaceModelToleranceNotMet;
      }

      // Ok we have subpixel fits in chip space so convert to cube space
      p_searchChip->SetChipPosition(p_chipSample,p_chipLine);
      p_cubeSample = p_searchChip->CubeSample();
      p_cubeLine   = p_searchChip->CubeLine();
    }
    // If sub-pixel accuracy is not used or an ideal fit was found
    // don't model a surface
    else {
      p_goodnessOfFit = bestFit;
      p_searchChip->SetChipPosition(bestSamp, bestLine);
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
    return( std::abs(p_idealFit - fit) < EPSILON );
  }

  /**
   * This method set up shrinking pattern chips.  After the Register algorithm
   * traverses the search chip and finds a match, it will reduce the pattern
   * chip and attempt to rematch in a local area to improve sub-pixel
   * accuracy
   *
   * @param samples   a vector of decreasing pattern samples
   * @param lines     a vector of decreasing pattern lines
   */
  void AutoReg::SetPatternReduction(std::vector<int> samples,
                                    std::vector<int> lines) {
    if (samples.size() != lines.size()) {
      std::string msg = "Pattern reduction samples and lines must";
      msg += " be the same size";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_reductionSamples = samples;
    p_reductionLines = lines;

    int n = samples.size();

    p_PatternChipNotEnoughValidData.clear();
    p_PatternChipNotEnoughValidData.resize(n, 0);
    p_PatternZScoreNotMet.clear();
    p_PatternZScoreNotMet.resize(n, 0);
    p_FitChipNoData.clear();
    p_FitChipNoData.resize(n, 0);
    p_FitChipToleranceNotMet.clear();
    p_FitChipToleranceNotMet.resize(n, 0);
  }

  /**
   * This method set up shrinking pattern chips.  After the Register algorithm
   * traverses the search chip and finds a match, it will reduce the pattern
   * chip and attempt to rematch in a local area to improve sub-pixel
   * accuracy
   *
   * @param percents   a vector of decreasing percents
   */
  void AutoReg::SetMovementReductionPercent(std::vector<double> percents) {
    if (percents.size() !=1 && percents.size() != p_reductionSamples.size()-1) {
      std::string msg = "The number of elements in the reduction movement ";
      msg += "array must be one less than the number of sample/line points.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    // If percent reduction has a single value,
    // duplicate it. Otherwise, copy directly from the pvl.
    std::vector<double> p;
    for (unsigned int i=0; i<p_reductionSamples.size()-1; i++) {
      if (i<percents.size()) p.push_back(percents[i]/100.0);
      else p.push_back(percents[0]/100.0);
    }

    p_reductionPercents = p;
  }

  Pvl AutoReg::RegistrationStatistics() {
    Pvl pvl;
    PvlGroup stats("AutoRegStatistics");
    stats += Isis::PvlKeyword("Total", p_Total);
    stats += Isis::PvlKeyword("Successful", p_Success);
    stats += Isis::PvlKeyword("Failure", p_Total - p_Success);
    pvl.AddGroup(stats);

    for (int i=0; i< (int)p_PatternChipNotEnoughValidData.size(); i++) {
      PvlGroup grp("PatternChip"+iString(i+1));
      grp += PvlKeyword("PatternNotEnoughValidData", p_PatternChipNotEnoughValidData[i]);
      grp += PvlKeyword("PatternZScoreNotMet", p_PatternZScoreNotMet[i]);
      grp += PvlKeyword("FitChipNoData", p_FitChipNoData[i]);
      grp += PvlKeyword("FitChipToleranceNotMet", p_FitChipToleranceNotMet[i]);
      pvl.AddGroup(grp);
    };

    PvlGroup model("SurfaceModel");
    model += PvlKeyword("SurfaceModelNotEnoughValidData", p_SurfaceModelNotEnoughValidData);
    model += PvlKeyword("SurfaceModelSolutionInvalid", p_SurfaceModelSolutionInvalid);
    model += PvlKeyword("SurfaceModelToleranceNotMet", p_SurfaceModelToleranceNotMet);
    model += PvlKeyword("SurfaceModelDistanceInvalid", p_SurfaceModelDistanceInvalid);

    pvl.AddGroup(model);


    return pvl;
  }
}
