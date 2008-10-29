#include "iException.h"
#include "Cube.h"
#include "SerialNumberList.h"
#include "PolygonTools.h"
#include "ImagePolygon.h"
#include "FindImageOverlaps.h"
#include "geos/geom/Point.h"
#include "geos/geom/Polygon.h"
#include "geos/geom/LinearRing.h"
#include "geos/geom/CoordinateSequence.h"
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/operation/distance/DistanceOp.h"
#include "geos/util/IllegalArgumentException.h"
#include "geos/util/GEOSException.h"

using namespace std;

namespace Isis {
  /**
   * Create FindImageOverlaps object.
   * 
   * Create an empty FindImageOverlaps object.
   * 
   * @see automaticRegistration.doc
   */
  FindImageOverlaps::FindImageOverlaps() {}


  /**
   * Create FindImageOverlaps object.
   * 
   * Create polygons of overlap from the images specified in the serial number
   * list. All polygons create by this class will be deleted when it is
   * destroyed, so callers should not delete the polygons returned by various 
   * members. 
   *  
   * @param sns The serial number list to use when finding overlaps
   * 
   * @see automaticRegistration.doc
   */
  FindImageOverlaps::FindImageOverlaps(SerialNumberList& sns) {

    // Create an ImageOverlap for each image boundary
    for (int i=0; i<sns.Size(); i++) {
      // Open the cube
      Cube cube;
      try {
        cube.Open(sns.Filename(i));
      }
      catch (iException &error) {
        std::string msg = "Unable to open cube for serial number [";
        msg += sns.SerialNumber(i) + "] filename [" + sns.Filename(i) + "]";
        throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
      }

      // Read the bounding polygon
      ImagePolygon *poly = new ImagePolygon(sns.SerialNumber(i));
      cube.Read(*poly);
      cube.Close();

      // Create a ImageOverlap with the serial number and the bounding
      // polygon and save it
      geos::geom::MultiPolygon *tmp = poly->Polys();
      geos::geom::MultiPolygon *mp = Despike (tmp);
      p_lonLatOverlaps.push_back(CreateNewOverlap(sns.SerialNumber(i), mp));

    }

    // Determine the overlap between each boundary polygon
    FindAllOverlaps ();
  }


  /**
   * Create FindImageOverlaps object.
   * 
   * Create polygons of overlap from the polygons specified. The serial numbers
   * and the polygons are assumed to be parallel arrays. The original polygons
   * passed as arguments are copied, so the ownership of the originals remains 
   * with the caller. 
   *  
   * @param sns The serial number list to use when finding overlaps 
   * @param polygons The polygons which are to be used when finding overlaps 
   * 
   * @see automaticRegistration.doc
   */
  FindImageOverlaps::FindImageOverlaps(std::vector<std::string> sns,
                                       std::vector<geos::geom::MultiPolygon*> polygons) {

    if (sns.size() != polygons.size()) {
      string message = "Invalid argument sizes. Sizes must match.";
      throw Isis::iException::Message(Isis::iException::Programmer,message,_FILEINFO_);      
    }

    // Create one ImageOverlap for each image sn
    for (unsigned int i=0; i<sns.size(); ++i) {
      p_lonLatOverlaps.push_back(CreateNewOverlap(sns[i], polygons[i]));
    }

    // Determine the overlap between each boundary polygon
    FindAllOverlaps ();

  }


  /**
   * Delete this object.
   * 
   * Delete the FindImageOverlaps object. The stored ImageOverlaps
   * will be deleted as well.
   */
  FindImageOverlaps::~FindImageOverlaps() {
    for (int i=0; i<Size(); i++) delete p_lonLatOverlaps[i]; 
  };


  /**
   * Find the overlaps between all the existing PolygonsOverlapItems
   */
  void FindImageOverlaps::FindAllOverlaps () {
    // Compare each polygon with all of the others
    for (unsigned int outside=0; outside<p_lonLatOverlaps.size()-1; ++outside) {
      // Intersect the current polygon (from the outside loop) with all others
      // below it that existed when we started this inside loop. In other words
      // don't intersect the current polyon with any new polygons that were
      // created in this pass of the inside loop
      unsigned int end = p_lonLatOverlaps.size();
      for (unsigned int inside=outside+1; inside<end; ++inside) {
        const geos::geom::MultiPolygon *poly1 = p_lonLatOverlaps.at(outside)->Polygon();
        const geos::geom::MultiPolygon *poly2 = p_lonLatOverlaps.at(inside)->Polygon();

        // First check to see if the two poygons are equivalent.
        // If they are, then we can get rid of one of them
        if (poly1->equals(poly2)) {
          AddSerialNumbers (p_lonLatOverlaps[outside],p_lonLatOverlaps[inside]);
          p_lonLatOverlaps.erase(p_lonLatOverlaps.begin()+inside);
          --end;
          continue;
        }

        // Get and process the intersection of the two polygons
        try {
          geos::geom::Geometry *tempGeometry = poly1->intersection(poly2);

          // We are only interested in overlaps that result in polygon(s)
          // and not any that are lines or points, so create a new multipolygon
          // with only the polygons of overlap

          geos::geom::MultiPolygon *overlap = Despike(MakeMultiPolygon (tempGeometry));

          if (!overlap->isEmpty()) {
            if (poly1->equals(overlap)) {
              AddSerialNumbers (p_lonLatOverlaps[outside], p_lonLatOverlaps[inside]);

              p_lonLatOverlaps.at(inside)->
                  SetPolygon(*(Despike(MakeMultiPolygon(poly2->difference(poly1)))));
            }
            else if (poly2->equals(overlap)) {
              AddSerialNumbers (p_lonLatOverlaps[inside], p_lonLatOverlaps[outside]);
              p_lonLatOverlaps.at(outside)->
                  SetPolygon(*(Despike(MakeMultiPolygon(poly1->difference(poly2)))));
            }
            else {
              ImageOverlap *po = new ImageOverlap();
              po->SetPolygon(overlap);
              AddSerialNumbers (po, p_lonLatOverlaps[outside]);
              AddSerialNumbers (po, p_lonLatOverlaps[inside]);
              p_lonLatOverlaps.push_back(po);

              // Create a temp multipolygon to hold the original poly1
              vector<geos::geom::Geometry*> polys;
              for (unsigned int i=0; i<poly1->getNumGeometries(); ++i) {
                polys.push_back((poly1->getGeometryN(i))->clone());
              }
              geos::geom::MultiPolygon *poly1Copy = Isis::globalFactory.createMultiPolygon (polys);
              
              p_lonLatOverlaps.at(outside)->
                  SetPolygon(Despike(MakeMultiPolygon(poly1->difference(poly2))));
              p_lonLatOverlaps.at(inside)->
                  SetPolygon(Despike(MakeMultiPolygon(poly2->difference(poly1Copy))));
            }
          }
        }
        // Collections are illegal as intersection argument
        catch (geos::util::IllegalArgumentException *ill) {
          std::string msg = "Unable to find overlap due to geos exception 1 [";
          msg += iString(ill->what()) + "]";
          delete ill;
          throw iException::Message(iException::Programmer,msg,_FILEINFO_);
        }
        catch (geos::util::GEOSException *exc) {
          std::string msg = "Unable to find overlap due to geos exception 2 [";
          msg += iString(exc->what()) + "]";
          delete exc;
          throw iException::Message(iException::Programmer,msg,_FILEINFO_);
        }
      }
    }
  }


  /**
   * Add the serial numbers from the second overlap to the first
   * 
   * Note: Need to check for existence of a SN before adding it
   *
   * @param to    The object to receive the new serial numbers
   * 
   * @param from  The object to copy the serial numbers from
   * 
   */
  void FindImageOverlaps::AddSerialNumbers (ImageOverlap *to,
                                            ImageOverlap *from) {
    for (int i=0; i<from->Size(); i++) {
      string s = (*from)[i];
      to->Add(s);
    }
  }


  /**
   * Create an overlap item to hold the overlap poly and its sn
   *
   * @param serialNumber The serial number 
   * 
   * @param latLonPolygon The object to copy the serial numbers from
   * 
   */
  ImageOverlap* FindImageOverlaps::CreateNewOverlap(std::string serialNumber,
      geos::geom::MultiPolygon* latLonPolygon) {

    return new ImageOverlap(serialNumber, *latLonPolygon);
  }
  

  // Take whatever geometry we were passed and create a multipolygon
  // from it. Note: it may be an empty multipolygon or already a multipolygon
  // NOTE: THIS MEMBER DELETES THE ORIGINAL GEOMETRY
  /**
   * Make a geos::geom::MultiPolygon out of the components of the argument
   *
   * Create a new geos::geom::MultiPolygon out of the general geometry that is
   * passed in. This can be useful after an intersection or some other
   * operator on two MultiPolygons. The result of the operator is often a
   * collection of different geometries such as points, lines, polygons...
   * This member extracts all polygons and multipolygons into a new
   * multipolygon. The original geometry is deleted.
   * 
   * @param geom The geometry to be converted into a multipolygon
   */
  geos::geom::MultiPolygon* FindImageOverlaps::MakeMultiPolygon (geos::geom::Geometry *geom) {

    if (geom->isEmpty()) {
      delete geom;
      return Isis::globalFactory.createMultiPolygon();
    }

    else if (geom->getGeometryTypeId() == geos::geom::GEOS_MULTIPOLYGON) {
      vector<geos::geom::Geometry*> polys;
      geos::geom::MultiPolygon* gm = (geos::geom::MultiPolygon*)geom;
      for (unsigned int i=0; i<gm->getNumGeometries(); ++i) {
        polys.push_back((gm->getGeometryN(i))->clone());
      }
      delete geom;
      return Isis::globalFactory.createMultiPolygon (polys);
    }

    else if (geom->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
      vector<geos::geom::Geometry*> polys;
      polys.push_back(geom);
      geos::geom::MultiPolygon *mp = Isis::globalFactory.createMultiPolygon(polys);
      return mp;
     }

    else if (geom->getGeometryTypeId() == geos::geom::GEOS_GEOMETRYCOLLECTION) {
      vector<geos::geom::Geometry*> polys;
      geos::geom::GeometryCollection *gc = (geos::geom::GeometryCollection*)geom;
      for (unsigned int i=0; i < gc->getNumGeometries(); ++i) {
        if (gc->getGeometryN(i)->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
          polys.push_back(gc->getGeometryN(i)->clone());
        }
      }
      delete geom;

      return Isis::globalFactory.createMultiPolygon (polys);
    }
    // All other geometry types are invalid so ignore them
    else {
      delete geom;
      return Isis::globalFactory.createMultiPolygon();
    }
  }


  /**
   * Create a new multipolygon without the spikes associated with
   * some versions of the geos package.
   *
   * @param multiPoly The original geos::geom::MultiPolygon to be despiked.
   * 
   */
  geos::geom::MultiPolygon* FindImageOverlaps::Despike (geos::geom::MultiPolygon *multiPoly) {
    // Despike each polygon in the multipolygon
    vector<geos::geom::Geometry*> *newPolys = new vector<geos::geom::Geometry*>;
    for (unsigned int g=0; g<multiPoly->getNumGeometries(); ++g) {
      geos::geom::Polygon *poly = (geos::geom::Polygon*)(multiPoly->getGeometryN(g));

      // Despike each hole inside this polygon
      vector<geos::geom::Geometry *> *holes = new vector<geos::geom::Geometry *>;
      for (unsigned int h=0; h<poly->getNumInteriorRing(); ++h) {
        geos::geom::LinearRing *lr = Despike(poly->getInteriorRingN(h));
        if (!lr->isEmpty()) {
          holes->push_back(lr);
        }
      }

      // Create a new polygon with the despiked vertices
      geos::geom::LinearRing* lr = Despike(poly->getExteriorRing());
      newPolys->push_back (Isis::globalFactory.createPolygon (lr, holes));
    }

    // Create a new multipoly from the polygon(s)
    geos::geom::MultiPolygon *mp = Isis::globalFactory.createMultiPolygon(newPolys);

  return mp;
  }


  /**
   * Create a new LinearRing from a LineString without the spikes associated
   * with some versions of the geos package.
   *
   * @param lineRing The original geos::geom::LinearRing to be despiked.
   * 
   */
  geos::geom::LinearRing* FindImageOverlaps::Despike (const geos::geom::LineString *lineRing) {
    geos::geom::CoordinateSequence *origVertices = lineRing->getCoordinates();
    geos::geom::CoordinateSequence *newVertices = new geos::geom::CoordinateArraySequence();

    int vertSize = origVertices->getSize();

    geos::geom::CoordinateSequence *coords = new geos::geom::CoordinateArraySequence();
    coords->add(origVertices->getAt(vertSize-2));
    coords->add(origVertices->getAt(0));

    geos::geom::LineString *line = Isis::globalFactory.createLineString(coords);

    // Test for a spike in the last segment with the second vertex
    geos::geom::Point *pt = Isis::globalFactory.createPoint(origVertices->getAt(1));
    if (geos::operation::distance::DistanceOp::distance(pt, line) > 0.00000000000001) {
      newVertices->add(origVertices->getAt(0));
    }

    delete pt;
    pt = NULL;
    delete coords;
    coords = NULL;

    for (int c=2; c<vertSize; c++) {
      pt = Isis::globalFactory.createPoint(origVertices->getAt(c));

      coords = new geos::geom::CoordinateArraySequence();
      coords->add(origVertices->getAt(c-2));
      coords->add(origVertices->getAt(c-1));
  
      line = Isis::globalFactory.createLineString(coords);

      // Test for a spike by testing the distance from each segment to the
      // following vertex
      if (geos::operation::distance::DistanceOp::distance(pt, line) > 0.00000000000001) {
        newVertices->add(origVertices->getAt(c-1));
      }

      delete pt;
      pt = NULL;
      delete line;
      line = NULL;
    }

    // Duplicate the first vertix as the last to close the polygon
    newVertices->add(newVertices->getAt(0));

    if (newVertices->getSize() < 4) {
      return Isis::globalFactory.createLinearRing(
          geos::geom::CoordinateArraySequence());
    }
    else {
      geos::geom::LinearRing *lr = Isis::globalFactory.createLinearRing(newVertices);
      return lr;
    }
  }



  /**
   * Return the overlaps that have a specific serial number.
   * 
   * Search the existing ImageOverlap objects for all that have the
   * serial numbers associated with them. Note: This could be costly when many
   * overlaps exist.
   *
   * @param serialNumber The serial number to be search for in all existing
   * ImageOverlaps
   */
  vector<ImageOverlap*> FindImageOverlaps::operator[](string serialNumber) {

    vector<ImageOverlap*> matches;

    // Look at all the ImageOverlaps we have and save off the ones that
    // have this sn
    for (unsigned int ov=0; ov<p_lonLatOverlaps.size(); ++ov) {
      for (int sn=0; sn<p_lonLatOverlaps[ov]->Size(); ++sn) {
        if ((*p_lonLatOverlaps[ov])[sn] == serialNumber) {
          matches.push_back(p_lonLatOverlaps[ov]);
        }
      }
    }
    return matches;
  }
}
