/**                                                                       
* @file                                                                  
* $Revision: 1.19 $                                                             
* $Date: 2009/05/06 18:04:59 $                                                                 
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
#include <vector>
#include "ImagePolygon.h"
#include "SpecialPixel.h"
#include "PolygonTools.h"
#include "geos/geom/Geometry.h"
#include "geos/geom/Polygon.h"
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/util/IllegalArgumentException.h"
#include "geos/util/TopologyException.h"
#include "geos/util/GEOSException.h"
#include "geos/io/WKTReader.h"
#include "geos/io/WKTWriter.h"
#include "geos/operation/distance/DistanceOp.h"

using namespace std;

namespace Isis {
  /**
   *  Constructs a Polygon object, setting the polygon name
   * 
   */
  ImagePolygon::ImagePolygon () : Blob ("Footprint","Polygon") {
    p_name = "Footprint";
    p_polygons = NULL;

  }

  //! Destroys the Polygon object
  ImagePolygon::~ImagePolygon() {
    if (p_polygons != 0) delete p_polygons;
  }

  /**
   * Create a Polygon from given cube
   * 
   * @param[in]  cube                 (Cube &)    Cube used to create polygon
   * 
   * @param[in]  ss    (Default=1)    (in)       Starting sample number
   * @param[in]  sl    (Default=1)    (in)       Starting Line number
   * @param[in]  ns    (Default=0)    (in)       Number of samples used to create 
   *       the polygon. Default of 0 will cause ns to be set to the number of
   *       samples in the cube.
   * @param[in]  nl    (Default=0)    (in)       Number of lines used to create 
   *       the polygon. Default of 0 will cause nl to be set to the number of
   *       lines in the cube.
   * @param[in]  band  (Default=1)    (in)       Image band number
   *  
   * @history 2008-04-28 Tracie Sucharski, When calculating p_pixInc, set 
   *                             to 1 if values calculated is 0.
   * @history 2008-12-30 Tracie Sucharski - If ground map returns pole make 
   *                        sure it is actually on the image. 
   */
  void ImagePolygon::Create (Cube &cube,int ss,int sl,int ns,int nl,
                             int band) {

    p_pts = new geos::geom::CoordinateArraySequence ();

    p_gMap = new UniversalGroundMap(cube);
    p_gMap->SetBand(band);

    p_cube = &cube;

    Camera *cam = NULL;
    Projection *proj = NULL;
    p_isProjected = false;

    try {
      cam = cube.Camera();
    } catch (iException &error) {
      try {
        proj = cube.Projection();
        error.Clear();
      } catch (iException &error) {
        std::string msg = "Can not create polygon, ";
        msg += "cube [" + cube.Filename();
        msg += "] is not a camera or map projection";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }
    }
    if (cam != NULL) p_isProjected = cam->HasProjection ();

    //  Create brick for use in SetImage
    p_brick = new Brick (1,1,1,cube.PixelType());

    /*------------------------------------------------------------------------
    /  Save cube number of samples and lines for later use.
    /-----------------------------------------------------------------------*/
    p_cubeSamps = cube.Samples ();
    p_cubeLines = cube.Lines ();

    try {
      WalkPoly();
    } catch (iException &e) {
      std::string msg = "Cannot find polygon for image [" + cube.Filename()+"]";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    /*------------------------------------------------------------------------
    /  If image contains 0/360 boundary, the polygon needs to be split up
    /  into multi polygons.
    /-----------------------------------------------------------------------*/
    Pvl defaultMap;
    cam->BasicMapping (defaultMap);

    if (cam->IntersectsLongitudeDomain(defaultMap)) {
      Fix360Poly ();
    } else {
      vector<geos::geom::Geometry *> *polys = new vector<geos::geom::Geometry *>;
      try {
        polys->push_back (globalFactory.createPolygon 
                          (globalFactory.createLinearRing (p_pts),NULL));
        p_polygons = globalFactory.createMultiPolygon (*polys);
        delete polys;
      } catch (iException &ie  ) {
        string msg = "Unable to create image footprint";
        ie.Message(iException::Programmer,msg,_FILEINFO_);
        throw;
      } catch (geos::util::IllegalArgumentException *geosIll) {
        std::string msg = "Unable to create image footprint due to geos illegal ";
        msg += "argument [" + iString(geosIll->what()) + "]";
        delete geosIll;
        throw iException::Message(iException::Programmer,msg,_FILEINFO_);
      } catch (geos::util::GEOSException *geosExc) {
        std::string msg = "Unable to create image footprint due to geos exception [";
        msg += iString(geosExc->what()) + "]";
        delete geosExc;
        throw iException::Message(iException::Programmer,msg,_FILEINFO_);
      }
    }
    if (p_brick != 0) delete p_brick;
  }

   /**
   * Finds the next point on the image using a left hand rule walking algorithm. To 
   * initiate the walk pass it the same point for both currentPoint and lastPoint.
   * 
   * @param[in] currentPoint  (geos::geom::Coordinate *currentPoint)   This is the 
   *       currentPoint in the path. You are looking for its successor.
   *  
   * @param[in] lastPoint  (geos::geom::Coordinate lastPoint)   This is the 
   *       lastPoint in the path, it allows the algorithm to calculate direction.
   *  
   * @param[in] recursionDepth  (int)   This optional parameter keeps track
   *       of how far it has walked around a point. By default it is zero.
   *  
   * throws Isis::iException::Programmer - Error walking the file
   * 
   */
  geos::geom::Coordinate ImagePolygon::FindNextPoint(geos::geom::Coordinate *currentPoint, geos::geom::Coordinate lastPoint, int recursionDepth){
    double x = lastPoint.x - currentPoint->x;
    double y = lastPoint.y - currentPoint->y;
    geos::geom::Coordinate result;

    // Check to see if depth is too deep (walked all the way around and found nothing)
    if (recursionDepth > 6) {
      return *currentPoint;
    }
    // Check and walk in appropriate direction
    if (x == 0.0 && y == 0.0) { // Find the starting point on an image
      for(int line = -1; line <= 1; line++) {
        for(int samp = -1; samp <= 1; samp++) {
          double s = currentPoint->x + samp;
          double l = currentPoint->y + line;
          if(!SetImage (s,l) || !p_gMap->Camera()->InCube()) {
            geos::geom::Coordinate next(s, l);
            return FindNextPoint(currentPoint, next);
          }
        }
      }

      std::string msg = "Unable to create image footprint. Starting point is not on the edge of the image.";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
      
    } else if ( x == -1.0 && y == -1.0) { // current is top left
      geos::geom::Coordinate next (currentPoint->x, currentPoint->y - 1);
      if(!SetImage (next.x ,next.y) || !p_gMap->Camera()->InCube()){
        result = FindNextPoint(currentPoint, next, recursionDepth+1);
      } else {
        result = next;
      }
    } else if ( x == 0.0 && y == -1.0) { // current is top
      geos::geom::Coordinate next (currentPoint->x + 1, currentPoint->y - 1);
      if(!SetImage (next.x ,next.y) || !p_gMap->Camera()->InCube()){
        result = FindNextPoint(currentPoint, next, recursionDepth+1);
      } else {
        result = next;
      }
    } else if ( x == 1.0 && y == -1.0) { // current is top right
      geos::geom::Coordinate next (currentPoint->x + 1, currentPoint->y);
      if(!SetImage (next.x ,next.y) || !p_gMap->Camera()->InCube()){
        result = FindNextPoint(currentPoint, next, recursionDepth+1);
      } else {
        result = next;
      }
    } else if ( x == 1.0 && y == 0.0) { // current is right
      geos::geom::Coordinate next (currentPoint->x + 1, currentPoint->y + 1);
      if(!SetImage (next.x ,next.y) || !p_gMap->Camera()->InCube()){
        result = FindNextPoint(currentPoint, next, recursionDepth+1);
      } else {
        result = next;
      }
    } else if ( x == 1.0 && y == 1.0) { // current is bottom right
      geos::geom::Coordinate next (currentPoint->x, currentPoint->y + 1);
      if(!SetImage (next.x ,next.y) || !p_gMap->Camera()->InCube()){
        result = FindNextPoint(currentPoint, next, recursionDepth+1);
      } else {
        result = next;
      }
    } else if ( x == 0.0 && y == 1.0) { // current is bottom
      geos::geom::Coordinate next (currentPoint->x - 1, currentPoint->y + 1);
      if(!SetImage (next.x ,next.y) || !p_gMap->Camera()->InCube()){
        result = FindNextPoint(currentPoint, next, recursionDepth+1);
      } else {
        result = next;
      }
    } else if ( x == -1.0 && y == 1.0) { // current is bottom left
      geos::geom::Coordinate next (currentPoint->x - 1, currentPoint->y);
      if(!SetImage (next.x ,next.y) || !p_gMap->Camera()->InCube()){
        result = FindNextPoint(currentPoint, next, recursionDepth+1);
      } else {
        result = next;
      }
    } else if ( x == -1.0 && y == 0.0) { // current is left
      geos::geom::Coordinate next (currentPoint->x - 1, currentPoint->y - 1);
      if(!SetImage (next.x ,next.y) || !p_gMap->Camera()->InCube()){
        result = FindNextPoint(currentPoint, next, recursionDepth+1);
      } else {
        result = next;
      }
    } else {
      std::string msg = "Unable to create image footprint. Error walking image.";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }
    return result;
  }

  /** 
  * Finds the first point that projects in an image
  * 
  * @return geos::geom::Coordinate A starting point that is on the edge of the 
  *         polygon.
  */
  geos::geom::Coordinate ImagePolygon::FindFirstPoint(){
    // @todo: Brute force method, should be improved
    for (int sample = 1; sample <= p_cubeSamps; sample++) {
      for (int line = 1; line <= p_cubeLines; line++) {
        if(SetImage (sample,line)) {
          return geos::geom::Coordinate (sample, line);
        }
      }
    }
    // Check to make sure a point was found
    std::string msg = "No lat/lon data found for image";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  /**
   * Walks the image finding its lon lat polygon and stores it to p_pts. 
   */
  void ImagePolygon::WalkPoly (){
    vector<geos::geom::Coordinate> *points = new vector<geos::geom::Coordinate>;
    double lat, lon, prevLat, prevLon;

    // Find the edge of the polygon
    geos::geom::Coordinate firstPoint = FindFirstPoint();
    points->push_back(firstPoint);
    //************************
    // Start walking the edge
    //************************

    // set up for intialization
    geos::geom::Coordinate currentPoint = firstPoint;
    geos::geom::Coordinate lastPoint = firstPoint;
    geos::geom::Coordinate tempPoint;

    do {
      tempPoint = FindNextPoint(&currentPoint, lastPoint);
      // Failed to find the next point
      if(tempPoint.equals(currentPoint)) {
        // Init vars for first run through the loop
        tempPoint = lastPoint;

        do {
          lastPoint = currentPoint;
          currentPoint = tempPoint;
          // remove last point from the list
          if (points->size()<2) {
            std::string msg = "Failed to find next point in the image.";
            throw iException::Message(iException::Programmer,msg,_FILEINFO_);
          }
          points->pop_back();

          tempPoint = FindNextPoint(&currentPoint, lastPoint, 1);

        } while(tempPoint.equals(points->at(points->size()-2)));
      }

      lastPoint = currentPoint;
      currentPoint = tempPoint;
      points->push_back(currentPoint);
      
    }while(!currentPoint.equals(firstPoint));

    prevLat = 0;
    prevLon = 0;
    // this vector stores crossing points, where the image crosses the 
    // meridian. It stores the first coordinate of the pair in its vector
    vector<geos::geom::Coordinate> *crossingPoints = new vector<geos::geom::Coordinate>;
    for(unsigned int i = 0; i<points->size(); i++) {
      geos::geom::Coordinate *temp = &(points->at(i));
      SetImage (temp->x, temp->y);
      lon = p_gMap->UniversalLongitude ();
      lat = p_gMap->UniversalLatitude ();
      if (abs(lon - prevLon) >= 180 && i!=0 ) {
        crossingPoints->push_back(geos::geom::Coordinate(prevLon, prevLat));
      }
      p_pts->add (geos::geom::Coordinate (lon,lat));
      prevLon = lon;
      prevLat = lat;
    }
    FixPolePoly(crossingPoints);
    delete crossingPoints;
   }

  /** 
  *  If the cube crosses the 0/360 boundary and contains a pole, Some  points are
  *  added to allow the polygon to unwrap properly. Throws an error if both poles
  *  are in the image. Returns if there is no pole in the image.
  *  
  */ 
  void ImagePolygon::FixPolePoly (vector<geos::geom::Coordinate> *crossingPoints) {
    // We currently do not support both poles in one image
    if (p_gMap->SetUniversalGround (90,0) && p_gMap->SetUniversalGround (-90,0)) {
      std::string msg = "Unable to create image footprint because image has both poles";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    } else if (!p_gMap->SetUniversalGround (90,0) && !p_gMap->SetUniversalGround (-90,0)) {
      // Nothing to do, no pole
      return;
    }
    geos::geom::Coordinate *closestPoint = &crossingPoints->at(0);
    geos::geom::Coordinate *pole;

    // Setup the right pole
    if(p_gMap->SetUniversalGround (90,0)){
      pole = new geos::geom::Coordinate(0,90);
    } else {
      pole = new geos::geom::Coordinate(0,-90);
    }

    // Find where the pole needs to be split
    double closestDistance = 999999999;
    for(unsigned int i = 0; i < crossingPoints->size(); i++) {
      geos::geom::Coordinate *temp = &crossingPoints->at(i);
      double tempDistance = Distance(temp, pole);
      if (tempDistance < closestDistance) {
        closestDistance = tempDistance;
        closestPoint = temp;
      }
    }

    if (closestDistance == 999999999) {
      std::string msg = "Image contains a pole but did not detect a meridian crossing!";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    // split image at the pole
    geos::geom::CoordinateSequence *new_points = new geos::geom::CoordinateArraySequence();
    for(unsigned int i = 0; i < p_pts->size(); i++) {
      geos::geom::Coordinate temp = p_pts->getAt(i);
      new_points->add(temp);
      if(temp.equals(*closestPoint)) {

        // Setup direction
        double fromLon, toLon;
        if ((p_pts->getAt(i+1).x - closestPoint->x) > 0) {
          fromLon = 0.;
          toLon = 360.;
        } else {
          fromLon = 360.;
          toLon = 0.;
        }
        new_points->add(geos::geom::Coordinate(fromLon, closestPoint->y));
        new_points->add(geos::geom::Coordinate(fromLon, pole->y));
        new_points->add(geos::geom::Coordinate(toLon, pole->y));
        new_points->add(geos::geom::Coordinate(toLon, closestPoint->y));
      }
    }
    delete p_pts;
    p_pts = new_points;
    delete pole;
  }

  /**
   * Sets the sample/line values of the cube to get lat/lon values.  This
   * method checks whether the image pixel is Null for level 2 images and
   * if so, it is considered an invalid pixel.
   * 
   * @param[in] sample   (const double)  Sample coordinate of the cube
   *
   * @param[in] line     (const double)  Line coordinate of the cube
   *
   * @return bool Returns true if the image was set successfully and false if it
   *              was not or if pixel of level 2 images is NULL.
   */

  bool ImagePolygon::SetImage (const double sample,const double line) {
    bool found = false;
    if (!p_isProjected) {
      found = p_gMap->SetImage (sample,line);
      if (found == false) {
        return false;
      } else {
        //  Make sure we can go back to image coordinates
        //  This is done because some camera models due to distortion, get
        //  a lat/lon for samp/line=1:1, but entering that lat/lon does
        //  not return samp/line =1:1. Ie.  moc WA global images
        double lat = p_gMap->UniversalLatitude();
        double lon = p_gMap->UniversalLongitude();
        return p_gMap->SetUniversalGround(lat,lon);
      }
    } else {
      // If projected, make sure the pixel DN is valid before worrying about
      //  geometry.
      p_brick->SetBasePosition ((int)sample,(int)line,1);
      p_cube->Read (*p_brick);
      if (Isis::IsNullPixel ( (*p_brick)[0] )) {
        return false;
      } else {
        return p_gMap->SetImage (sample,line);
      }
    }
  }


 /**
  * If the cube crosses the 0/360 boundary and does not include a pole,
  * the polygon is separated into two polygons, one on each side of the
  * boundary.  These two polygons are put into a geos Multipolygon. 
  */
  void ImagePolygon::Fix360Poly () {
    bool convertLon = false;
    bool negAdjust = false;
    bool newCoords = false;  //  coordinates have been adjusted
    geos::geom::CoordinateSequence *newPts = new geos::geom::CoordinateArraySequence ();
    double lon,lat;
    double lonOffset=0;
    double prevLon = p_pts->getAt(0).x;
    double prevLat = p_pts->getAt(0).y;

    newPts->add (geos::geom::Coordinate (prevLon, prevLat));
    for (unsigned int i=1; i<p_pts->getSize (); i++) {
      lon = p_pts->getAt(i).x;
      lat = p_pts->getAt(i).y;
      // check to see if you just crossed the Meridian
      if (abs(lon - prevLon) > 180 && ( prevLat != 90 && prevLat != -90 )) {
        newCoords = true;
        // if you were already converting then stop (crossed Meridian even number of times)
        if (convertLon) {
          convertLon = false;
          lonOffset = 0;
        } else { // Need to start converting again, deside how to adjust coordinates
          if ((lon - prevLon) > 0) {
            lonOffset = -360.;
            negAdjust = true;
          } else if ((lon - prevLon) < 0) {
            lonOffset = 360.;
            negAdjust = false;
          }
          convertLon = true;          
        }
      }

      // add coord
      newPts->add (geos::geom::Coordinate (lon + lonOffset, lat));

      // set current to old
      prevLon = lon;
      prevLat = lat;
    }

    // Nothing was done so return
    if (!newCoords) {
      geos::geom::Polygon *newPoly = globalFactory.createPolygon
                                     (globalFactory.createLinearRing(newPts),NULL);
      p_polygons = PolygonTools::MakeMultiPolygon(newPoly);
      delete newPts;
      return;
    }

    // bisect into seperate polygons
    try {
      geos::geom::Polygon *newPoly = globalFactory.createPolygon
                                     (globalFactory.createLinearRing(newPts),NULL);

      geos::geom::CoordinateSequence *pts = new geos::geom::CoordinateArraySequence ();
      geos::geom::CoordinateSequence *pts2 = new geos::geom::CoordinateArraySequence ();

      // Depending on direction of compensation bound accordingly
      //***************************************************
      // LOGIC IN NEXT FOR LOOP DEPENDS ON VALUES OF pts
      // please verify correct if you change these values
      //***************************************************
      if (negAdjust) {
        pts->add (geos::geom::Coordinate (0.,90.));
        pts->add (geos::geom::Coordinate (-360.,90.));
        pts->add (geos::geom::Coordinate (-360.,-90.));
        pts->add (geos::geom::Coordinate (0.,-90.));
        pts->add (geos::geom::Coordinate (0.,90.));
        pts2->add (geos::geom::Coordinate (0.,90.));
        pts2->add (geos::geom::Coordinate (360.,90.));
        pts2->add (geos::geom::Coordinate (360.,-90.));
        pts2->add (geos::geom::Coordinate (0.,-90.));
        pts2->add (geos::geom::Coordinate (0.,90.));
      } else {
        pts->add (geos::geom::Coordinate (360.,90.));
        pts->add (geos::geom::Coordinate (720.,90.));
        pts->add (geos::geom::Coordinate (720.,-90.));
        pts->add (geos::geom::Coordinate (360.,-90.));
        pts->add (geos::geom::Coordinate (360.,90.));
        pts2->add (geos::geom::Coordinate (360.,90.));
        pts2->add (geos::geom::Coordinate (0.,90.));
        pts2->add (geos::geom::Coordinate (0.,-90.));
        pts2->add (geos::geom::Coordinate (360.,-90.));
        pts2->add (geos::geom::Coordinate (360.,90.));
      }

      geos::geom::Polygon *boundaryPoly = globalFactory.createPolygon
                                          (globalFactory.createLinearRing(pts),NULL);
      geos::geom::Polygon *boundaryPoly2 = globalFactory.createPolygon
                                           (globalFactory.createLinearRing(pts2),NULL);
      /*------------------------------------------------------------------------
      /  Intersecting the original polygon (converted coordinates) with the 
      /  boundary polygons will create the multi polygons with the converted coordinates.
      /  These will need to be converted back to the original coordinates.
      /-----------------------------------------------------------------------*/

      geos::geom::Geometry *intersection = PolygonTools::Intersect(newPoly,boundaryPoly);
      geos::geom::MultiPolygon *convertPoly = PolygonTools::MakeMultiPolygon(intersection);
      delete intersection;

      intersection = PolygonTools::Intersect(newPoly,boundaryPoly2);
      geos::geom::MultiPolygon *convertPoly2 = PolygonTools::MakeMultiPolygon(intersection);
      delete intersection;

      /*------------------------------------------------------------------------
      / Adjust points created in the negative space or >360 space to be back in 
      / the 0-360 world.  This will always only need to be done on convertPoly. 
      / Then add geometries to finalpolys.
      /-----------------------------------------------------------------------*/
      vector<geos::geom::Geometry *> *finalpolys = new vector<geos::geom::Geometry *>;
      geos::geom::Geometry *newGeom = NULL;
 
      for (unsigned int i=0; i<convertPoly->getNumGeometries();i++) {
        newGeom = (convertPoly->getGeometryN(i))->clone();
        pts = convertPoly->getGeometryN(i)->getCoordinates();
        geos::geom::CoordinateSequence *newPts = new geos::geom::CoordinateArraySequence ();
          // fix the points
  
          for (unsigned int k=0; k < pts->getSize() ; k++) {
            double lon = pts->getAt(k).x;
            double lat = pts->getAt(k).y;
            if (negAdjust) {
              lon = lon + 360;
            } else {
              lon = lon - 360;
            }
            newPts->add(geos::geom::Coordinate(lon,lat), k);
          }
        // Add the points to polys
        finalpolys->push_back (globalFactory.createPolygon
                             (globalFactory.createLinearRing (newPts),NULL));
      }

      // This loop is over polygons that will always be in 0-360 space no need to convert
      for (unsigned int i=0; i<convertPoly2->getNumGeometries();i++) {
        newGeom = (convertPoly2->getGeometryN(i))->clone();
        finalpolys->push_back(newGeom);
      }
     
      p_polygons = globalFactory.createMultiPolygon (*finalpolys);

      delete finalpolys;
      delete newGeom;
      delete newPts;
      delete pts;
      delete pts2;
    } catch (geos::util::IllegalArgumentException *geosIll) {
      std::string msg = "Unable to create image footprint (Fix360Poly) due to ";
      msg += "geos illegal argument [" + iString(geosIll->what()) + "]";
      delete geosIll;
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    } catch (geos::util::GEOSException *geosExc) {
      std::string msg = "Unable to create image footprint (Fix360Poly) due to ";
      msg += "geos exception [" + iString(geosExc->what()) + "]";
      delete geosExc;
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    } catch (...) {
      std::string msg = "Unable to create image footprint (Fix360Poly) due to ";
      msg += "unknown exception";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    } 
  }

  /**
   * Reads Multipolygon from cube blob
   * 
   * @param[in] is  (std::fstream)   Input stream to read from
   * 
   * throws Isis::iException::Io - Error reading data from stream
   * 
   */

  void ImagePolygon::ReadData (std::fstream &is) {

    streampos sbyte = p_startByte - 1;
    is.seekg (sbyte,std::ios::beg);
    if (!is.good()) {
      string msg = "Error preparing to read data from " + p_type +
                   " [" + p_blobName + "]";
      throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
    }

    char *buf = new char[p_nbytes+1];
    is.read(buf,p_nbytes);
    p_polyStr = buf;
    delete []buf;

    if (!is.good()) {
      string msg = "Error reading data from " + p_type + " [" +
                   p_blobName + "]";
      throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
    }

    geos::io::WKTReader *wkt = new geos::io::WKTReader (&(globalFactory));
    p_polygons = (geos::geom::MultiPolygon *) wkt->read(p_polyStr);
    delete wkt;
  }


  //!  Initializes for writing polygon to cube blob
  void ImagePolygon::WriteInit () {
    geos::io::WKTWriter *wkt = new geos::io::WKTWriter ();
   
    // Check to see p_polygons is valid data
    if(!p_polygons) {
      string msg = "Cannot write a NULL polygon!";
      throw Isis::iException::Message(iException::Programmer,msg,_FILEINFO_);
    }
    p_polyStr = wkt->write(p_polygons);
    p_nbytes = p_polyStr.size();

    delete wkt;
  }

  /**
   * Writes polygon to cube blob
   *
   * @param[in] os   (std::fstream &)  Output steam blob data will be written to
   *
   * @throws Isis::iException::Io - Error writing data to stream
   */

  void ImagePolygon::WriteData (std::fstream &os) {
    os.write (p_polyStr.c_str(),p_nbytes);
  }

  /**
   * Calculates the distance squared between two coordinates.
   */
  double ImagePolygon::Distance(geos::geom::Coordinate *p1, geos::geom::Coordinate *p2){
    return ((p2->x - p1->x)*(p2->x - p1->x)) + ((p2->y - p1->y)*(p2->y - p1->y));
  }

} // end namespace isis

