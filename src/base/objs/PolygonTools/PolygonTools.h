#ifndef Polygontools_h
#define Polygontools_h
/**
 * @file
 * $Revision: 1.5 $
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

#include "geos/geom/GeometryFactory.h"
#include "geos/geom/MultiPolygon.h"
#include "geos/geom/CoordinateSequence.h"

namespace Isis {

/**
 * @brief General tools for manipulating polygons
 * 
 * Tools for manipulating polygons within Isis. These include
 * converting a Lon/Lat polygon to X/Y using a projection.
 * 
 * @ingroup Utility
 * 
 * @author 2006-06-22 Stuart Sides 
 *  
 * @internal 
 *   @history 2008-08-18 Steven Lambright Updated to work with geos3.0.0 
 * 
 */

  class UniversalGroundMap;
  class Projection;

  static geos::geom::GeometryFactory globalFactory;

/**                                                                       
 * @brief Provides various tools to work with geos multipolygons
 *                                                                        
 * This class provides methods to that work with geos multipolygons. This
 * includes functions to convert from one coordinate system to another and to
 * copy multipolygons.
 *                                                                        
 * @ingroup Utility
 *                                                                        
 * @author 2006-08-07 Stuart Sides
 *                                                                                                             
 * @internal                                                                                                                        
 *   @history 2006-08-07 Stuart Sides - Original version
 * 
 *   @history 2007-05-04 Robert Sucharski - Moved the method to
 *            output WKT from ImagePlygon class to this class.
 *            Also added method to output GML format.
 * 
 *   @history 2007-11-09 Tracie Sucharski - Remove ToWKT method, geos
 *             now has a method (toString) to return a WKT string.
 *             Added To180 method which converts polygon coordinates from
 *             0/360 system to -180/180 system.  If polygon was split because
 *             it crossed the 0/360 seam, the two polys coordinates are
 *             converted then merged.
 *   @history 2008-06-18 Steven Koechle - Fixed Documentation Errors
 *   @history 2008-08-18 Steven Lambright - Updated to work with geos3.0.0
 *            instead of geos2. Mostly namespace changes.
 */                                                                       

  class PolygonTools {

    public:
      PolygonTools ();
      ~PolygonTools ();

      static geos::geom::MultiPolygon *XYFromLonLat(
          const geos::geom::MultiPolygon &lonLatPoly, Projection *proj);

      static geos::geom::MultiPolygon *LonLatFromXY(
          const geos::geom::MultiPolygon &xYPoly, Projection *proj);

      static geos::geom::MultiPolygon *SampleLineFromLonLat(
          const geos::geom::MultiPolygon &lonLatPoly, UniversalGroundMap *ugm);

      // Return a deep copy of a multpolygon
      static geos::geom::MultiPolygon* CopyMultiPolygon(const geos::geom::MultiPolygon *mpolygon);
      static geos::geom::MultiPolygon* CopyMultiPolygon(const geos::geom::MultiPolygon &mpolygon);

      //  Return polygon in -180/180 coordinated system and merge split polys
      static geos::geom::MultiPolygon *To180 (geos::geom::MultiPolygon *poly360);

      //Return a polygon in GML format
      static std::string ToGML(const geos::geom::MultiPolygon *mpolygon, std::string idString="0");

  private:
      geos::geom::MultiPolygon *p_polygons;

  };
};

#endif

