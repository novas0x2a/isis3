/**                                                                       
 * @file                                                                  
 * $Revision: 1.5 $                                                             
 * $Date: 2008/08/19 22:32:52 $                                                                 
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
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

#include "ProjectionFactory.h"
#include "UniversalGroundMap.h"
#include "geos/geom/Geometry.h"
#include "geos/geom/Polygon.h"
#include "geos/geom/Coordinate.h"
#include "geos/geom/CoordinateSequence.h"
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/LineString.h"
#include "geos/geom/LinearRing.h"
#include "PolygonTools.h"

namespace Isis {

  /**                                                                       
   * This method will return a geos::geom::MultiPolygon which contains the X/Y
   * coordinates of the LonLat polygon.
   * 
   * @param lonLatPolygon  A multipolygon in (Lon,Lat) order
   * @param projection     The projection to be used to convert the Lons and Lat
   *                       to X and Y
   * 
   * @return  Returns a multipolygon which is the result of converting the input
   *          multipolygon from (Lon,Lat) to (X,Y).
   */
  geos::geom::MultiPolygon *PolygonTools::XYFromLonLat(
      const geos::geom::MultiPolygon &lonLatPolygon, Projection *projection) {

    if (projection == NULL) {
      std::string msg = "Unable to convert Lon/Lat polygon to X/Y. ";
      msg += "No projection has was supplied";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    // Convert the Lat/Lon poly coordinates to X/Y coordinates
    if (lonLatPolygon.isEmpty()) {
      return globalFactory.createMultiPolygon();
    }
    else {
      std::vector<geos::geom::Geometry*> *xyPolys = new std::vector<geos::geom::Geometry*>;
      // Convert each polygon in this multi-polygon
      for (unsigned int g=0; g<lonLatPolygon.getNumGeometries(); ++g) {
        geos::geom::Polygon *poly = (geos::geom::Polygon*)(lonLatPolygon.getGeometryN(g));

        // Convert each hole inside this polygon
        std::vector<geos::geom::Geometry *> *holes = new std::vector<geos::geom::Geometry *>;
        for (unsigned int h=0; h<poly->getNumInteriorRing(); ++h) {
          geos::geom::CoordinateSequence *xycoords = new geos::geom::CoordinateArraySequence ();
          geos::geom::CoordinateSequence *llcoords =
              poly->getInteriorRingN(h)->getCoordinates();

          // Convert each coordinate in this hole
          for (unsigned int cord=0; cord < llcoords->getSize(); ++cord) {
            projection->SetGround(llcoords->getAt(cord).y,
                                    llcoords->getAt(cord).x);
            xycoords->add(geos::geom::Coordinate(projection->XCoord(),
                                           projection->YCoord()));
          } // end num coords in hole loop
          holes->push_back(globalFactory.createLinearRing(xycoords));
        } // end num holes in polygon loop

        // Convert the exterior ring of this polygon
        geos::geom::CoordinateSequence *xycoords = new geos::geom::CoordinateArraySequence ();
        geos::geom::CoordinateSequence *llcoords =
            poly->getExteriorRing()->getCoordinates();

        // Convert each coordinate in the exterior ring of this polygon
        for (unsigned int cord=0; cord < llcoords->getSize(); ++cord) {
          projection->SetGround(llcoords->getAt(cord).y,
                                  llcoords->getAt(cord).x);
          xycoords->add(geos::geom::Coordinate(projection->XCoord(),
                                         projection->YCoord()));
        } // end exterior ring coordinate loop

        xyPolys->push_back(globalFactory.createPolygon(
            globalFactory.createLinearRing(xycoords), holes));
      } // end num geometry in multi-poly

      // Create a new multipoly from all the new X/Y polygon(s)
      return globalFactory.createMultiPolygon(xyPolys);
    } // end else
  }


  /**                                                                       
   * This method will return a geos::geom::MultiPolygon which contains the LonLat
   * coordinates of the XY polygon.
   *
   * @param xYPolygon  A multipolygon in (X,Y) order
   * @param projection The projection to be used to convert the Xs and Ys to Lon
   *                   and Lats
   * 
   * @return  Returns a multipolygon which is the result of converting the input
   *          multipolygon from (X,Y) to (Lon,Lat).
   */
  geos::geom::MultiPolygon *PolygonTools::LonLatFromXY(
      const geos::geom::MultiPolygon &xYPolygon, Projection *projection) {

    if (projection == NULL) {
      std::string msg = "Unable to convert X/Y polygon to Lon/Lat. ";
      msg += "No projection was supplied";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    // Convert the X/Y poly coordinates to Lat/Lon coordinates
    if (xYPolygon.isEmpty()) {
      return globalFactory.createMultiPolygon();
    }
    else {
      std::vector<geos::geom::Geometry*> *llPolys = new std::vector<geos::geom::Geometry*>;
      // Convert each polygon in this multi-polygon
      for (unsigned int g=0; g<xYPolygon.getNumGeometries(); ++g) {
        geos::geom::Polygon *poly = (geos::geom::Polygon*)(xYPolygon.getGeometryN(g));

        // Convert each hole inside this polygon
        std::vector<geos::geom::Geometry *> *holes = new std::vector<geos::geom::Geometry *>;
        for (unsigned int h=0; h<poly->getNumInteriorRing(); ++h) {
          geos::geom::CoordinateSequence *llcoords = new geos::geom::CoordinateArraySequence ();
          geos::geom::CoordinateSequence *xycoords =
              poly->getInteriorRingN(h)->getCoordinates();

          // Convert each coordinate in this hole
          for (unsigned int cord=0; cord < xycoords->getSize(); ++cord) {
            projection->SetWorld(xycoords->getAt(cord).x,
                                   xycoords->getAt(cord).y);
            llcoords->add(geos::geom::Coordinate(projection->Longitude(),
                                           projection->Latitude()));
          } // end num coords in hole loop
          holes->push_back(globalFactory.createLinearRing(llcoords));
        } // end num holes in polygon loop

        // Convert the exterior ring of this polygon
        geos::geom::CoordinateSequence *llcoords = new geos::geom::DefaultCoordinateSequence ();
        geos::geom::CoordinateSequence *xycoords =
            poly->getExteriorRing()->getCoordinates();

        // Convert each coordinate in the exterior ring of this polygon
        for (unsigned int cord=0; cord < xycoords->getSize(); ++cord) {
          projection->SetWorld(xycoords->getAt(cord).x,
                                 xycoords->getAt(cord).y);
          llcoords->add(geos::geom::Coordinate(projection->Longitude(),
                                         projection->Latitude()));
        } // end exterior ring coordinate loop

        llPolys->push_back(globalFactory.createPolygon(
            globalFactory.createLinearRing(llcoords), holes));
      } // end num geometry in multi-poly

      // Create a new multipoly from all the new Lon/Lat polygon(s)
      return globalFactory.createMultiPolygon(llPolys);
    } // end else
  }


  /**                                                                       
   * This method will return a geos::geom::MultiPolygon which contains the sample/lin
   * coordinates of the Lon/Lat polygon.
   *
   * @param lonLatPolygon  A multipolygon in (Lon,Lat order)
   * @param ugm            The UniversalGroundMap to be used to convert the Lons
   *                       and Lat to Samples and Lines
   * 
   * @return  Returns a multipolygon which is the result of converting the input
   *          multipolygon from (Lon,Lat) to (Sample,Line).
   */
  geos::geom::MultiPolygon *PolygonTools::SampleLineFromLonLat(
      const geos::geom::MultiPolygon &lonLatPolygon, UniversalGroundMap *ugm) {

    if (ugm == NULL) {
      std::string msg = "Unable to convert Lon/Lat polygon to Sample/Line. ";
      msg += "No UniversalGroundMap was supplied";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  
    // Convert the Lon/Lat poly coordinates to Sample/Line coordinates
    if (lonLatPolygon.isEmpty()) {
      return globalFactory.createMultiPolygon();
    }
    else {
      std::vector<geos::geom::Geometry*> *slPolys = new std::vector<geos::geom::Geometry*>;
      // Convert each polygon in this multi-polygon
      for (unsigned int g=0; g<lonLatPolygon.getNumGeometries(); ++g) {
        geos::geom::Polygon *poly = (geos::geom::Polygon*)(lonLatPolygon.getGeometryN(g));
  
        // Convert each hole inside this polygon
        std::vector<geos::geom::Geometry *> *holes = new std::vector<geos::geom::Geometry *>;
        for (unsigned int h=0; h<poly->getNumInteriorRing(); ++h) {
          geos::geom::CoordinateSequence *slcoords = new geos::geom::DefaultCoordinateSequence ();
          geos::geom::CoordinateSequence *llcoords =
              poly->getInteriorRingN(h)->getCoordinates();
  
          // Convert each coordinate in this hole
          for (unsigned int cord=0; cord < llcoords->getSize(); ++cord) {
            ugm->SetUniversalGround(llcoords->getAt(cord).y,
                                                     llcoords->getAt(cord).x);
            slcoords->add(geos::geom::Coordinate(ugm->Sample(),
                                                 ugm->Line()));
          } // end num coords in hole loop
          holes->push_back(globalFactory.createLinearRing(slcoords));
        } // end num holes in polygon loop
  
        // Convert the exterior ring of this polygon
        geos::geom::CoordinateSequence *slcoords = new geos::geom::CoordinateArraySequence ();
        geos::geom::CoordinateSequence *llcoords =
            poly->getExteriorRing()->getCoordinates();
  
        // Convert each coordinate in the exterior ring of this polygon
        for (unsigned int cord=0; cord < llcoords->getSize(); ++cord) {
          ugm->SetUniversalGround(llcoords->getAt(cord).y,
                                                   llcoords->getAt(cord).x);
          slcoords->add(geos::geom::Coordinate(ugm->Sample(),
                                               ugm->Line()));
        } // end exterior ring coordinate loop
  
        slPolys->push_back(globalFactory.createPolygon(
            globalFactory.createLinearRing(slcoords), holes));
      } // end num geometry in multi-poly
  
      // Create a new multipoly from all the new X/Y polygon(s)
      return globalFactory.createMultiPolygon(slPolys);
    } // end else
  }


  /**                                                                       
   * This static method will create a deep copy of a geos::geom::MultiPolygon. The
   * caller assumes responsibility for the memory associated with the new
   * polygon.
   * 
   * @param mpolygon The multipolygon to be copied.
   * 
   * @return  Returns a pointer to a multipolygon which is a deep copy of the
   *          input multipolygon. This is necessary because at the time of
   *          writing the geos package does not create multipolygons when
   *          copying. It produdes geometryCollections
   */
  geos::geom::MultiPolygon* PolygonTools::CopyMultiPolygon(const geos::geom::MultiPolygon *mpolygon) {

    std::vector<geos::geom::Geometry*> *polys = new std::vector<geos::geom::Geometry*>;
    for (unsigned int i=0; i<mpolygon->getNumGeometries(); ++i) {
      polys->push_back((mpolygon->getGeometryN(i))->clone());
    }
    return globalFactory.createMultiPolygon (polys);
  }


  /**                                                                       
   * This static method will create a deep copy of a geos::geom::MultiPolygon. The
   * caller assumes responsibility for the memory associated with the new
   * polygon.
   * 
   * @param mpolygon The multipolygon to be copied.
   * 
   * @return  Returns a pointer to a multipolygon which is a deep copy of the
   *          input multipolygon. This is necessary because at the time of
   *          writing the geos package does not create multipolygons when
   *          copying. It produdes geometryCollections
   */
  geos::geom::MultiPolygon* PolygonTools::CopyMultiPolygon(const geos::geom::MultiPolygon &mpolygon) {

    std::vector<geos::geom::Geometry*> *polys = new std::vector<geos::geom::Geometry*>;
    for (unsigned int i=0; i<mpolygon.getNumGeometries(); ++i) {
      polys->push_back((mpolygon.getGeometryN(i))->clone());
    }
    return globalFactory.createMultiPolygon (polys);
  }


  /**
   * Write out the polygon with gml header
   * 
   * 
   * @param [in] mpolygon Polygon with lat/lon vertices
   * @param idString mpolygon's Id
   * @return std::istrean  Returns the polygon with lon,lat
   *         lon,lat format vertices and GML header
   */

  std::string PolygonTools::ToGML (const geos::geom::MultiPolygon *mpolygon, std::string idString) { 

    std::ostringstream os;

    //Write out the GML header  
    os << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>" << std::endl;
    os << "<ogr:FeatureCollection" << std::endl;
    os << "    xmlns:xsi=\"http://www.w3c.org/2001/XMLSchema-instance\""<< std::endl;
    os << "    xmlns:gml=\"http://www.opengis.net/gml\">" << std::endl;
    os << "  <gml:boundedBy>" << std::endl;
    os << "    <gml:Box>" << std::endl;
    os << "      <gml:coord><gml:X>0.0</gml:X><gml:Y>-90.0</gml:Y></gml:coord>" << std::endl;
    os << "      <gml:coord><gml:X>360.0</gml:X><gml:Y>90.0</gml:Y></gml:coord>" << std::endl;
    os << "    </gml:Box>" << std::endl;
    os << "  </gml:boundedBy>" << std::endl;
    os << "  <gml:featureMember>" << std::endl;
    os << "   <multi_polygon fid=\"0\">" << std::endl;
    os << "      <ID>"<< idString << "</ID>" << std::endl;
    os << "      <ogr:geometryProperty><gml:MultiPolygon><gml:polygonMember>" <<
                 "<gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>";


    for (unsigned int polyN=0; polyN<mpolygon->getNumGeometries(); polyN++) {
      geos::geom::CoordinateSequence *pts = mpolygon->getGeometryN(polyN)->getCoordinates();

      for (unsigned int i=0; i<pts->getSize(); i++) {
        double lon = pts->getAt(i).x;
        double lat = pts->getAt(i).y;

        os << std::setprecision(15) << lon << ","<< std::setprecision(15) << lat << " "; 
      }
    }

     os <<"</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs>"<<
     "</gml:Polygon></gml:polygonMember></gml:MultiPolygon>"<<
     "</ogr:geometryProperty>" << std::endl; 
     os << "</multi_polygon>" << std::endl;
     os << "</gml:featureMember>" << std::endl;
     os << "</ogr:FeatureCollection>" << std::endl;

    return os.str();
  }



  /**
   * Convert polygon coordinates from 360 system to 180.  If polygon is split
   * into 2 polygons due to crossing the 360 boundary, convert the polygon
   * less than 360 to 180, then union together to merge the polygons.
   * 
   * @param[in] poly360  (geos::geom::MultiPolygon)   polys split by 360 boundary
   * 
   * @return geos::geom::MultiPolygon  Returns a single polygon (Note: Longitude
   *                               coordinates will be less than 0.
   */

  geos::geom::MultiPolygon *PolygonTools::To180 (geos::geom::MultiPolygon *poly360) {

    geos::geom::CoordinateSequence *convPts;
    std::vector<geos::geom::Geometry *> polys;
    //  If single poly, simply convert coordinates to 180 system
    if (poly360->getNumGeometries() == 1) {
      convPts = poly360->getGeometryN(0)->getCoordinates();
    }
    else {
      //  Find poly that needs to be converted to 180 coordinates
      if ((poly360->getGeometryN(0)->getCoordinate()->x - 
           poly360->getGeometryN(1)->getCoordinate()->x) > 0) {
        polys.push_back(poly360->getGeometryN(1)->clone());
        convPts = poly360->getGeometryN(0)->getCoordinates();
      }
      else {
        polys.push_back(poly360->getGeometryN(0)->clone());
        convPts = poly360->getGeometryN(1)->getCoordinates();
      }
    }

    //  Convert poly to greater than 360
    geos::geom::CoordinateSequence *newPts = new geos::geom::CoordinateArraySequence();
    for (unsigned int i=0; i<convPts->getSize(); i++) {
      newPts->add (geos::geom::Coordinate(convPts->getAt(i).x - 360.,convPts->getAt(i).y));
    }

    polys.push_back (Isis::globalFactory.createPolygon
                    (Isis::globalFactory.createLinearRing(newPts),NULL));

    geos::geom::GeometryCollection *polyCollection = 
                Isis::globalFactory.createGeometryCollection(polys);
    geos::geom::Geometry *unionPoly = polyCollection->buffer(0);

    //  Make sure there is a single polygon
    if (unionPoly->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
      return (geos::geom::MultiPolygon *) unionPoly;
    }
    else {
      std::string msg = "Could not merge 0/360 footprints";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
  }


} // end namespace isis

