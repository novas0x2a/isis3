/**
 * @file
 * $Revision: 1.7 $
 * $Date: 2008/06/18 18:44:55 $
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

#include "OverlapStatistics.h"
#include "MultivariateStatistics.h"
#include "Cube.h"
#include "Filename.h"
#include "Projection.h"
#include "ProjectionFactory.h"
#include "iException.h"
#include "Brick.h"
#include <cfloat>
#include <iomanip>

using namespace std;
namespace Isis {

 /**
  * Constructs and OverlapStatistics object.  Compares the two input cubes and
  * finds where they overlap.
  *
  * @param x The first input cube
  * @param y The second input cube
  *
  * @throws Isis::iException::User - All images must have the same number of
  *                                  bands
  */
  OverlapStatistics::OverlapStatistics(Isis::Cube &x, Isis::Cube &y) {
    // Extract filenames and band number from cubes
    p_xFile = x.Filename();
    p_yFile = y.Filename();

    // Make sure number of bands match
    if (x.Bands() != y.Bands()) {
      string msg = "Number of bands do not match between cubes [" +
                   p_xFile.Name() + "] and [" + p_yFile.Name() + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_bands = x.Bands();
    p_stats.resize(p_bands);

    //Create projection from each cube
    Projection *projX = x.Projection();
    Projection *projY = y.Projection();

    // Test to make sure projection parameters match
    if (*projX != *projY) {
      string msg = "Mapping groups do not match between cubes [" +
                   p_xFile.Name() + "] and [" + p_yFile.Name() + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    // Figure out the x/y range for both images to find the overlap
    double Xmin1 = projX->ToProjectionX(0.5);
    double Ymax1 = projX->ToProjectionY(0.5);
    double Xmax1 = projX->ToProjectionX(x.Samples()+0.5);
    double Ymin1 = projX->ToProjectionY(x.Lines()+0.5);

    double Xmin2 = projY->ToProjectionX(0.5);
    double Ymax2 = projY->ToProjectionY(0.5);
    double Xmax2 = projY->ToProjectionX(y.Samples()+0.5);
    double Ymin2 = projY->ToProjectionY(y.Lines()+0.5);

    // Find overlap
    if ((Xmin1<Xmax2) && (Xmax1>Xmin2) && (Ymin1<Ymax2) && (Ymax1>Ymin2)) {
      double minX = Xmin1 > Xmin2 ? Xmin1 : Xmin2;
      double minY = Ymin1 > Ymin2 ? Ymin1 : Ymin2;
      double maxX = Xmax1 < Xmax2 ? Xmax1 : Xmax2;
      double maxY = Ymax1 < Ymax2 ? Ymax1 : Ymax2;

      // Find Sample range of the overlap
      p_minSampX = (int)(projX->ToWorldX(minX) + 0.5);
      p_maxSampX = (int)(projX->ToWorldX(maxX) + 0.5);
      p_minSampY = (int)(projY->ToWorldX(minX) + 0.5);
      p_maxSampY = (int)(projY->ToWorldX(maxX) + 0.5);
      p_sampRange = p_maxSampX - p_minSampX;

      // Test to see if there was only sub-pixel overlap
      if (p_sampRange <= 0) return;

#if 0
      // Make sure the sample range of the overlap in the 2 projections are equal
      if (p_sampRange != (p_maxSampY - p_minSampY)) {
        string msg = "Sample overlap ranges do not match in [" +
                     p_xFile.Name() + "] and [" + p_yFile.Name() + "]";
        throw iException::Message(iException::Programmer,msg,_FILEINFO_);
      }
#endif

      // Find Line range of overlap
      p_minLineX = (int)(projX->ToWorldY(maxY) + 0.5);
      p_maxLineX = (int)(projX->ToWorldY(minY) + 0.5);
      p_minLineY = (int)(projY->ToWorldY(maxY) + 0.5);
      p_maxLineY = (int)(projY->ToWorldY(minY) + 0.5);
      p_lineRange = p_maxLineX - p_minLineX;

#if 0
      // Make sure the line range of the overlap in the 2 projections are equal
      if (p_lineRange != (p_maxLineY - p_minLineY)) {
        string msg = "Line overlap ranges do not match in [" +
                     p_xFile.Name() + "] and [" + p_yFile.Name() + "]";
        throw iException::Message(iException::Programmer,msg,_FILEINFO_);
      }
#endif

      for (int band=1; band<=p_bands; band++) {
        Brick b1(p_sampRange,1,band,x.PixelType());
        Brick b2(p_sampRange,1,band,y.PixelType());

        for (int i=0; i<p_lineRange; i++) {
          b1.SetBasePosition(p_minSampX,(i+p_minLineX),band);
          b2.SetBasePosition(p_minSampY,(i+p_minLineY),band);
          x.Read(b1);
          y.Read(b2);
          p_stats[band-1].AddData(b1.DoubleBuffer(), b2.DoubleBuffer(), p_sampRange);
        }
      }
    }
  }

 /**
  * Checks all bands of the cubes for an overlap, and will only return false
  * if none of the bands overlap
  *
  * @return bool Returns true if any of the bands overlap, and false if none
  *              of the bands overlap
  */
  bool OverlapStatistics::HasOverlap() {
    for (int b=0; b<p_bands; b++) {
      if (p_stats[b].ValidPixels()>0) return true;
    }
    return false;
  }

 /**
  * Creates a pvl of various useful data obtained by the overlap statistics
  * class.  The pvl is returned in an output stream
  *  
  * @param os The output stream to write to
  * @param stats The OverlapStatistics object to write to os
  * @return ostream Pvl of useful statistics
  */
  std::ostream& operator<<(std::ostream &os, Isis::OverlapStatistics &stats) {
    // Output the private variables
    try {
    for (int band=1; band<=stats.Bands(); band++) {
      if (stats.HasOverlap(band)) {
        PvlObject o("Stats");
        PvlGroup g1("Overlap1");
        PvlGroup g2("Overlap2");
        o += PvlKeyword("File1", stats.FilenameX().Name());
        o += PvlKeyword("File2", stats.FilenameY().Name());
        o += PvlKeyword("Width", stats.Samples());
        o += PvlKeyword("Height", stats.Lines());
        o += PvlKeyword("Band", band);
        g1 += PvlKeyword("Average",stats.GetMStats(band).X().Average());
        g2 += PvlKeyword("Average",stats.GetMStats(band).Y().Average());
        g1 += PvlKeyword("StandardDeviation", stats.GetMStats(band).X().StandardDeviation());
        g2 += PvlKeyword("StandardDeviation", stats.GetMStats(band).Y().StandardDeviation());
        g1 += PvlKeyword("Variance", stats.GetMStats(band).X().Variance());
        g2 += PvlKeyword("Variance", stats.GetMStats(band).Y().Variance());
        g1 += PvlKeyword("ValidPixels", stats.GetMStats(band).X().ValidPixels());
        g2 += PvlKeyword("ValidPixels", stats.GetMStats(band).Y().ValidPixels());
        g1 += PvlKeyword("TotalPixels", stats.GetMStats(band).X().TotalPixels());
        g2 += PvlKeyword("TotalPixels", stats.GetMStats(band).Y().TotalPixels());
        g1 += PvlKeyword("StartSample", stats.StartSampleX());
        g2 += PvlKeyword("StartSample", stats.StartSampleY());
        g1 += PvlKeyword("EndSample", stats.EndSampleX());
        g2 += PvlKeyword("EndSample", stats.EndSampleY());
        g1 += PvlKeyword("StartLine", stats.StartLineX());
        g2 += PvlKeyword("StartLine", stats.StartLineY());
        g1 += PvlKeyword("EndLine", stats.EndLineX());
        g2 += PvlKeyword("EndLine", stats.EndLineY());
        o.AddGroup(g1);
        o.AddGroup(g2);
        o += PvlKeyword("Covariance", stats.GetMStats(band).Covariance());
        o += PvlKeyword("Correlation", stats.GetMStats(band).Correlation());
        double a,b;
        PvlKeyword LinReg("LinearRegression");
        try {
          stats.GetMStats(band).LinearRegression(a,b);
          LinReg += a;
          LinReg += b;
        }
        catch (iException &e) {
          // It is possible one of the overlaps was constant and therefore
          // the regression would be a vertical line (x=c instead of y=ax+b)
          e.Clear();
        }
        o += LinReg;
        o += PvlKeyword("ValidPixels", stats.GetMStats(band).ValidPixels());
        o += PvlKeyword("InvalidPixels", stats.GetMStats(band).InvalidPixels());
        o += PvlKeyword("TotalPixels", stats.GetMStats(band).TotalPixels());
        os << o << endl;
      }
    }
    return os;
    }
    catch (iException &e) {
      string msg = "Trivial overlap between [" + stats.FilenameX().Name();
      msg += "] and [" + stats.FilenameY().Name() + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

  }


}
