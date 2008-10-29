#include "MaximumCorrelation.h"
#include "Chip.h"
#include "MultivariateStatistics.h"

namespace Isis {
  double MaximumCorrelation::MatchAlgorithm (Chip &pattern, Chip &subsearch) {
    // calculate the sampling information
    double percent = sqrt(p_patternSamplingPercent/100.0);
    double numLines = (int)(percent*pattern.Lines());
    if (numLines < 1.0) numLines = 1.0;
    double linc = pattern.Lines()/numLines;
    double numSamples = (int)(percent*pattern.Samples());
    if (numSamples < 1.0) numSamples = 1.0;
    double sinc = pattern.Samples()/numSamples;
    if (linc < 1.0) linc = 1.0;
    if (sinc < 1.0) sinc = 1.0;

    MultivariateStatistics mv;
    for (double l=1.0; l<=pattern.Lines(); l+=linc) {
      for (double s=1.0; s<=pattern.Samples(); s+=sinc) {
        int line = (int)l;
        int samp = (int)s;

        double pdn = pattern(samp,line);
        double sdn = subsearch(samp,line);
        mv.AddData(&pdn,&sdn,1);
      }
    }

    double percentValid = (double) mv.ValidPixels() / 
                     (numLines * numSamples);
    if (percentValid * 100.0 < this->PatternValidPercent()) return Isis::Null;

    double r = mv.Correlation();
    if (r == Isis::Null) return Isis::Null;
    return fabs(r);
  }

  /**
   * This virtual method must return if the 1st fit is equal to or better 
   * than the second fit.
   * 
   * @param fit1  1st goodness of fit 
   * @param fit2  2nd goodness of fit
   */
  bool MaximumCorrelation::CompareFits (double fit1, double fit2) {
    return (fit1 >= fit2);
  }
}

extern "C" Isis::AutoReg *MaximumCorrelationPlugin (Isis::Pvl &pvl) {
  return new Isis::MaximumCorrelation(pvl);
}

