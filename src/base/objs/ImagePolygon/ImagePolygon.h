#ifndef ImagePolygon_h
#define ImagePolygon_h
/**
 * @file
 * $Revision: 1.23 $
 * $Date: 2009/06/19 18:12:04 $
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
#include <vector>

#include "iException.h"
#include "Cube.h"
#include "Brick.h"
#include "Camera.h"
#include "Projection.h"
#include "UniversalGroundMap.h"
#include "Blob.h"
#include "geos/geom/Coordinate.h"
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
 *  @history 2008-12-10 Steven Koechle - Split pole code into seperate function,
 *           chanded the way FixPoly360 did its intersection to handle a
 *           multipolygon being returned.
 *  @history 2008-12-10 Steven Lambright - Fixed logical problem which could
 *           happen when the limb of the planet was on the left and was the
 *           first thing found in the FindPoly method.
 *  @history 2009-01-06 Steven Koechle - Changed Constructor, removed backwards
 *           compatibility for footprint name.
 *  @history 2009-01-28 Steven Lambright - Fixed memory leaks
 *  @history 2009-02-20 Steven Koechle - Fixed Fix360Poly to use the
 *           PolygonTools Intersect method.
 *  @history 2009-04-17 Steven Koechle - Rewrote most of the class. Implemented
 *           a left-hand rule AI walking algotithm in the walkpoly method. Pole
 *           and 360 boundary logic was more integrated and simplified. Method
 *           of finding the first point was seperated into its own method.
 *  @history 2009-05-06 Steven Koechle - Fixed Error where a NULL polygon was
 *           being written.
 *  @history 2009-05-28 Steven Lambright - This program will no longer generate
 *           invalid polygons and should use despike as a correction algorithm
 *           if it is necessary.
 *  @history 2009-05-28 Stuart Sides - Added a new argument to Create for
 *           controling the quality of the polygon.
 *  @history 2009-05-28 Steven Lambright - Moved the quality parameter in the
 *           argument list, improved speed and the edges of files (and subareas)
 *           should be checked if the algorithm reaches them now. Fixed the
 *           sub-area capabilities.
 *  @history 2009-06-16 Christopher Austin - Fixed WalkPoly() to catch large
 *           pixel increments that ( though snapping ) skip over the starting
 *           point. Also skip the first left turn in FindNextPoint() to prevent
 *           double point checking as well as errors with snapping.
 *  @history 2009-06-17 Christopher Austin - Removed p_name. Uses parent Name()
 *  @history 2009-06-18 Christopher Austin - Changes starting point skipping
 *           solution to a snap. Added fix for polygons that form
 *           self-intersecting polygons due to this first point snapping.
 *  @history 2009-06-19 Christopher Austin - Temporarily reverted for backport
 *           and fixed truthdata for SpiceRotation
 */

  class ImagePolygon : public Isis::Blob {

    public:
      ImagePolygon ();
      ~ImagePolygon ();

      void Create (Cube &cube,int quality=1, int ss=1,int sl=1,int ns=0,int nl=0,
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
      // Please do not add new polygon manipulation methods to this class.
      // Polygon manipulation should be done in the PolygonTools class.
      bool SetImage (const double sample, const double line);

      geos::geom::Coordinate FindFirstPoint ();
      void WalkPoly ();
      geos::geom::Coordinate FindNextPoint(geos::geom::Coordinate *currentPoint,
                                           geos::geom::Coordinate lastPoint,
                                           int recursionDepth=0);
      
      double Distance(geos::geom::Coordinate *p1, geos::geom::Coordinate *p2);

      void MoveBackInsideImage(double &sample, double &line, double sinc, double linc);
      bool InsideImage(double sample, double line);
      void Fix360Poly ();
      void FixPolePoly(std::vector<geos::geom::Coordinate> *crossingPoints);

      Cube *p_cube;       //!< The cube provided
      bool p_isProjected; //!< True when the provided cube is projected

      Brick *p_brick;     //!< Used to check for valid DNs

      geos::geom::CoordinateSequence *p_pts; //!< The sequence of coordinates that compose the boundry of the image

      geos::geom::MultiPolygon *p_polygons;  //!< The multipolygon of the image

      std::string p_name;    //!< Name of Blob
      std::string p_polyStr; //!< The string representation of the polygon

      UniversalGroundMap *p_gMap; //!< The cube's ground map

      int p_cubeStartSamp; //!< The the sample of the first valid point in the cube
      int p_cubeStartLine; //!< The the line of the first valid point in the cube
      int p_cubeSamps;     //!< The number of samples in the cube
      int p_cubeLines;     //!< The number of lines in the cube

      int p_quality; //!< The increment for walking along the edge of the polygon

  };
};

#endif

