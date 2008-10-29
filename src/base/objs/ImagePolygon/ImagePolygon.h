#ifndef ImagePolygon_h
#define ImagePolygon_h
/**
 * @file
 * $Revision: 1.8 $
 * $Date: 2008/08/19 22:32:52 $
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

#include <string>
#include <sstream>

#include "iException.h"
#include "Cube.h"
#include "Brick.h"
#include "Camera.h"
#include "Projection.h"
#include "UniversalGroundMap.h"
#include "Blob.h"
#include "geos/geom/MultiPolygon.h"
#include "geos/geom/CoordinateSequence.h"

namespace Isis {

/**
 * @brief Create cube polygons, read/write polygons to blobs
 * 
 * This class creates polygons defining an image boundary, reads
 * the polygon from a cube blob, and writes a polygon to a cube
 * blob.  The GEOS (Geometry Engine Open Source) package is used
 * to create and manipulate the polygons. See
 * http://geos.refractions.net for information about this
 * package.
 * 
 * @ingroup Registration
 * 
 * @author 2005-11-22 Tracie Sucharski
 * 
 * @internal
 * 
 * @history 2006-05-17 Stuart Sides Renamed from PolygonTools and moved the 
 *                                  geos::GeometryFactory to the new PolygonTools
 * @history 2007-01-04 Tracie Sucharski, Fixed bug in Fix360Poly, on
 *                non-pole images after splitting at the 360 boundary,
 *                wasn't insuring that start and end points of polygon were
 *                the same.
 * @history 2007-01-05 Tracie Sucharski, Made a change to the SetImage
 *                method.  Cannot make the assumption that if a sample/line
 *                returns a lat/lon, that the lat/lon will return a sample/line.
 *                This was found using a Moc WA, global image, sample 1, line 1
 *                returned a lat/lon, but entering that lat/lon does not return a
 *                valid sample/line.
 * @history 2007-01-08 Tracie Sucharski, Fixed bugs in the ToImage and
 *                ToGround class for images that contain a pole.
 * 
 * @history 2007-01-19  Tracie Sucharski,  Comment out the ToGround method.
 * 
 * @history 2007-01-31  Tracie Sucharski,  Add WKT method to return WKT polygon
 *                            as a string.
 * @history 2007-02-06  Tracie Sucharski,  Added band parameter to Create
 *                            method and call UniversalGroundMap::SetBand.
 * @history 2007-05-04  Robert Sucharski, with Jeff, Stuart, and
 *                               Tracie's blessing, moved the WKT method to the
 *                               PolygonTools class.
 * @history 2007-07-26  Tracie Sucharski,  Added ss,sl,ns,nl to constructor
 *                            for sub-poly creation.
 * @history 2007-11-09  Tracie Sucharski,  Remove WKT method since the geos
 *                            package know has a method to return a WKT
 *                            string
 * @history 2008-03-17  Tracie Sucharski, Added try/catch block to Create 
 *                            and Fix360Poly to catch geos exceptions.
 *  
 *  @history 2008-06-18 Stuart Sides - Fixed doc error
 *  @history 2008-08-18 Steven Lambright - Updated to work with geos3.0.0
 *           instead of geos2. Mostly namespace changes.
 */

  class ImagePolygon : public Isis::Blob {

    public:
      ImagePolygon (std::string name);
      ~ImagePolygon ();

      void Create (Cube &cube,int pixInc=0,int ss=1,int sl=1,int ns=0,int nl=0,
                   int band=1);

      //!  Return a geos Multipolygon
      geos::geom::MultiPolygon *Polys () { return p_polygons; };

      //!  Return name of polygon
      std::string Name () { return p_name; };

  protected:
      void ReadData (std::fstream &is);
      void WriteInit ();
      void WriteData (std::fstream &os);

  private:
    //Please do not add new polygon manipulation methods to this class.
    //Polygon manipulation should be done in the PolygonTools class.
      bool SetImage (const double sample, const double line);

      void FindPoly ();
      int FindFirstSamp (int line,int direction);
      int FindFirstLine (int samp,int direction);

      bool Fix360Poly ();

      Cube *p_cube;
      bool p_isProjected;
      int p_band;

      Brick *p_brick;

      geos::geom::CoordinateSequence *p_pts;

      geos::geom::MultiPolygon *p_polygons;
      int p_pixInc;

      std::string p_uniqueImageId;
      std::string p_name;
      std::string p_polyStr;
      std::stringstream p_ss;

      UniversalGroundMap *p_gMap;
      int p_iss;
      int p_isl;
      int p_ies;
      int p_iel;
      int p_ins;
      int p_inl;

      int p_cubeSamps;
      int p_cubeLines;
      double p_pole;


  };
};

#endif

