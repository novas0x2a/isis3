/**                                                                       
* @file                                                                  
* $Revision: 1.15 $                                                             
* $Date: 2009/01/28 16:30:07 $                                                                 
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
#include "geos/util/GEOSException.h"
#include "geos/io/WKTReader.h"
#include "geos/io/WKTWriter.h"

using namespace std;

namespace Isis {
  /**
   *  Constructs a Polygon object, setting the polygon name
   * 
   */
  ImagePolygon::ImagePolygon () : Blob ("Footprint","Polygon") {
    p_name = "Footprint";
    p_pole = 0;
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
   * @param[in]  pixInc (Default=0)   (in)       The pixel increment used to 
   *                                               calculate polygon vertices
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
  void ImagePolygon::Create (Cube &cube,int pixInc,int ss,int sl,int ns,int nl,
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

    p_iss = ss;
    p_isl = sl;
    p_ins = ns;
    p_inl = nl;
    if (p_ins == 0) p_ins = p_cubeSamps;
    if (p_inl == 0) p_inl = p_cubeLines;
    p_ies = p_iss + p_ins - 1;
    if (p_ies > p_cubeSamps) p_ies = p_cubeSamps;
    p_iel = p_isl + p_inl - 1;
    if (p_iel > p_cubeLines) p_iel = p_cubeLines;

    /*----------------------------------------------------------------------
    /  If pixInc not entered calculate based on longest edge.  This 
    /  might need to be refined.
    /---------------------------------------------------------------------*/
    if (pixInc != 0) {
      p_pixInc = pixInc;
    } else {
      p_pixInc = (int)(max(p_ins,p_inl) * .03);
    }
    if (p_pixInc == 0) p_pixInc = 1;
    /*-----------------------------------------------------------------------
    /  First find out if this image contains a pole and save off that info.
    /  2008-12-30  There have been case where we are getting a "false positive"
    /  on the pole-the pt being returned is actually off of the image.
    /  This check is added until it is dealt with in Isis lower level routines?
    /----------------------------------------------------------------------*/
    if ( (p_gMap->SetUniversalGround (90,0)) &&
         (p_gMap->Sample() >= 1 && p_gMap->Sample() <= p_ins &&
          p_gMap->Line() >= 1 && p_gMap->Line() <= p_inl) ) {
      p_pole = 90.;
    }
    if ( (p_gMap->SetUniversalGround (-90,0)) &&
         (p_gMap->Sample() >= 1 && p_gMap->Sample() <= p_ins &&
          p_gMap->Line() >= 1 && p_gMap->Line() <= p_inl) ) {
      p_pole = -90.;
    }
    try {
      FindPoly ();
    } catch (iException &e) {
      std::string msg = "Cannot find polygon for image [" + cube.Filename();
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    //  Close off polygon by adding the first point as the last point
    p_pts->add (p_pts->getAt(0));

    /*------------------------------------------------------------------------
    /  If image contains 0/360 boundary, the polygon needs to be split up
    /  into 2 polygons, one on each side of the boundary.
    /-----------------------------------------------------------------------*/
    Pvl defaultMap;
    cam->BasicMapping (defaultMap);
    bool fixed = false;
    if (cam->IntersectsLongitudeDomain(defaultMap)) {
      if (p_pole != 0) {
        fixed = FixPolePoly();
      } else {
        // If this is a sub-poly, it might not intersect the lon domain
        // save bool indicating if the pts had to be split up into multi-
        // polygons.  If not, the pts will still need to be put into a poly.
        fixed = Fix360Poly ();
      }
    }
    /*----------------------------------------------------------------------
    /  If this is a sub-poly and did not contain the boundary, copy polys
    /  to vector.
    /---------------------------------------------------------------------*/
    if (!fixed) {
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


  //!  Calculate the polygon vertices

  void ImagePolygon::FindPoly () {

    int ssamp,sline;
    int lastSamp,lastLine;
    int samp,line;
    double lat,lon;
    int sampPixInc = p_pixInc;
    int linePixInc = p_pixInc;

    /*------------------------------------------------------------------------
    /  Top edge
    /-----------------------------------------------------------------------*/

    //  First find the first valid pixel
    ssamp = p_iss;
    line = 0;
    while ( ssamp <= p_ies && !(line = FindFirstLine(ssamp,+1)) ) {
      ssamp++;
    }
    if (line != 0) {
      // Keep track of first pixel found, so that the last edge which is the
      // left edge will stop at this pixel.
      lastSamp = ssamp;
      lastLine = line;
      SetImage (ssamp,line);
      lon = p_gMap->UniversalLongitude ();
      lat = p_gMap->UniversalLatitude ();
      p_pts->add (geos::geom::Coordinate (lon,lat));
    } else {
      std::string msg = "No lat/lon data found for image";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    //  Now get the rest of the points on the top edge
    for (samp=ssamp+sampPixInc; samp<=p_ies; samp+=sampPixInc) {
      if ((line = FindFirstLine (samp,+1))) {
        if (abs(line - lastLine) > p_pixInc) {
          // reset loop samp to previous point and try new increment
          samp = samp - sampPixInc;
          sampPixInc = (int)((float)sampPixInc * 
                             ((float)sampPixInc / abs(line - lastLine)));
          //  If we can't converge, and we are not close to the right edge
          //  delete all points before current point and
          //  start edge from here.  The points deleted will be picked up
          //  while processing the left edge.
          if (sampPixInc == 0) {
            if (samp > (p_ins/2)) break;
            while (!p_pts->isEmpty()) p_pts->deleteAt(0);
            // Could not converge, start at new point
            samp = samp + p_pixInc;
            line = FindFirstLine (samp,+1);
            lon = p_gMap->UniversalLongitude ();
            lat = p_gMap->UniversalLatitude ();
            p_pts->add (geos::geom::Coordinate(lon,lat));
            lastSamp = samp;
            lastLine = line;
            sampPixInc = p_pixInc;
          }
          continue;
        }

        lon = p_gMap->UniversalLongitude ();
        lat = p_gMap->UniversalLatitude ();
        p_pts->add (geos::geom::Coordinate (lon,lat));
        lastSamp = samp;
        lastLine = line;
        sampPixInc = p_pixInc;
      } else {
        break;
      }
    }
    //  If top right corner is valid, include
    if (SetImage (p_ies,p_isl)) {
      lon = p_gMap->UniversalLongitude ();
      lat = p_gMap->UniversalLatitude ();
      p_pts->add (geos::geom::Coordinate (lon,lat));
      lastSamp = p_ies;
      lastLine = p_isl;
    }

    //  Set the starting point so we know where to end the polygon
    p_gMap->SetUniversalGround (p_pts->getAt(0).y,p_pts->getAt(0).x);

    int startPolyLine = (int)(p_gMap->Line() + .5);

    /*------------------------------------------------------------------------
    /  Right edge
    /-----------------------------------------------------------------------*/
    linePixInc = p_pixInc;

    //  First find the first valid pixel
    samp = 0;
    sline = lastLine + 1;
    while ( sline <= p_iel && !(samp = FindFirstSamp(sline,-1)) ) {
      sline++;
    }
    if (samp != 0) {
      lastSamp = samp;
      lastLine = sline;
      SetImage (samp,sline);
      lon = p_gMap->UniversalLongitude ();
      lat = p_gMap->UniversalLatitude ();
      p_pts->add (geos::geom::Coordinate (lon,lat));

      //  Now get the rest of the points on the right edge
      for (line=sline+linePixInc; line<=p_iel; line+=linePixInc) {
        if ((samp = FindFirstSamp (line,-1))) {
          if (abs(samp - lastSamp) > p_pixInc) {
            //  Reset loop line to previous point and try new increment
            line = line - linePixInc;
            linePixInc = (int)((float)linePixInc * 
                               ((float)linePixInc / abs(samp - lastSamp)));
            //  If we can't converge, abandon right edge, go to bottom edge
            if (linePixInc == 0) break;
            continue;
          }

          lon = p_gMap->UniversalLongitude ();
          lat = p_gMap->UniversalLatitude ();
          p_pts->add (geos::geom::Coordinate (lon,lat));
          lastSamp = samp;
          lastLine = line;
          linePixInc = p_pixInc;
        } else {
          break;
        }
      }
      //  If bottom corner is valid, include
      if (SetImage (p_ies,p_iel)) {
        lon = p_gMap->UniversalLongitude ();
        lat = p_gMap->UniversalLatitude ();
        p_pts->add (geos::geom::Coordinate (lon,lat));
        lastSamp = p_ies;
        lastLine = p_iel;
      }
    }



    /*------------------------------------------------------------------------
    /  Bottom edge
    /-----------------------------------------------------------------------*/
    sampPixInc = p_pixInc;

    //  First find the first valid pixel on edge
    ssamp = lastSamp - 1;
    line = 0;
    while ( ssamp >= p_iss && !(line = FindFirstLine(ssamp,-1)) ) {
      ssamp--;
    }
    if (line != 0) {
      lastSamp = ssamp;
      lastLine = line;
      SetImage (ssamp,line);
      lon = p_gMap->UniversalLongitude ();
      lat = p_gMap->UniversalLatitude ();
      p_pts->add (geos::geom::Coordinate (lon,lat));

      //  Now get the rest of the points on the top edge
      for (samp=ssamp-sampPixInc; samp>=p_iss; samp-=sampPixInc) {
        if ((line = FindFirstLine (samp,-1))) {
          if (abs(line - lastLine) > p_pixInc) {
            // reset loop samp to previous point and try new increment
            samp = samp + sampPixInc;
            sampPixInc = (int)((float)sampPixInc * 
                               ((float)sampPixInc / abs(line - lastLine)));
            //  If we can't converge, abandon bottom edge, go to left edge
            if (sampPixInc == 0) break;
            continue;
          }

          lon = p_gMap->UniversalLongitude ();
          lat = p_gMap->UniversalLatitude ();
          p_pts->add (geos::geom::Coordinate (lon,lat));
          lastSamp = samp;
          lastLine = line;
          sampPixInc = p_pixInc;
        } else {
          break;
        }
      }
      //  If left bottom corner is valid, include
      if (SetImage (p_iss,p_iel)) {
        lon = p_gMap->UniversalLongitude ();
        lat = p_gMap->UniversalLatitude ();
        p_pts->add (geos::geom::Coordinate (lon,lat));
        lastSamp = p_iss;
        lastLine = p_iel;
      }
    }

    /*------------------------------------------------------------------------
    /  Left edge
    /-----------------------------------------------------------------------*/
    linePixInc = p_pixInc;

    //  First find the first valid pixel on edge
    samp = 0;
    sline = lastLine - 1;
    while ( sline >= p_isl && !(samp = FindFirstSamp(sline,+1)) ) {
      sline--;
    }
    if (samp != 0) {
      lastSamp = samp;
      lastLine = sline;
      SetImage (samp,sline);
      lon = p_gMap->UniversalLongitude ();
      lat = p_gMap->UniversalLatitude ();
      p_pts->add (geos::geom::Coordinate (lon,lat));

      //  Now get the rest of the points on the left edge
      for (line=sline-linePixInc; line>startPolyLine; line-=linePixInc) {
        if ((samp = FindFirstSamp (line,+1))) {
          if (abs(samp - lastSamp) > p_pixInc) {
            //  Reset loop line to previous point and try new increment
            line = line + linePixInc;
            linePixInc = (int)((float)linePixInc * 
                               ((float)linePixInc / abs(samp - lastSamp)));
            //  If we can't converge, abandon right edge, go to bottom edge
            if (linePixInc == 0) break;
            continue;
          }

          lon = p_gMap->UniversalLongitude ();
          lat = p_gMap->UniversalLatitude ();
          p_pts->add (geos::geom::Coordinate (lon,lat));
          lastSamp = samp;
          lastLine = line;
          linePixInc = p_pixInc;
        } else {
          break;
        }
      }
    }
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
   * Finds the first valid image sample on given line working forward
   * or backward depending on the value given in the direction parameter.
   * 
   * @param[in] line      (int)   Line to search
   * 
   * @param[in] direction (int)   Direction to search,
   *                              1=search forward starting at sample 1,
   *                              2=search in reverse starting at the last sample
   *
   * @return int  Returns the first valid sample found on this line.  Returns
   *              a 0 if no valid data is found on this line.
   */

  int ImagePolygon::FindFirstSamp (int line,int direction) {

    int samp;
    if (direction == 1) {
      samp = p_iss;
    } else {
      samp = p_ies;
    }

    while ((samp >= p_iss && samp <= p_ies) && !SetImage (samp,line)) {
      samp += direction;
    }

    //  Nothing found, return 0
    if (samp < p_iss || samp > p_ies) {
      return 0;
    } else {
      return samp;
    }
  }

  /**
   * Finds the line in the given column with the first valid pixel.  Searches
   * forward or backward depending on the value given in the direction parameter.
   * 
   * @param[in] samp      (int)   Sample column to search
   * 
   * @param[in] direction (int)   Direction to search,
   *                              1=search forward starting at line 1,
   *                              2=search in reverse starting at the last line
   *
   * @return int  Returns the first valid line found on this sample column.
   */

  int ImagePolygon::FindFirstLine (int samp,int direction) {

    int line;
    if (direction == 1) {
      line = p_isl;
    } else {
      line = p_iel;
    }

    while ((line >= p_isl && line <= p_iel) && !SetImage (samp,line)) {
      line += direction;
    }

    //  Nothing found, return 0
    if (line < p_isl || line > p_iel) {
      return 0;
    } else {
      return line;
    }
  }

  /** 
   * If the cube crosses the 0/360 boundary and contains a pole, Some  points are
   * added to allow the polygon to unwrap properly. Throws an error if both poles 
   * are in the image. 
   * 
   * Returns True if points were added.
   */ 
  bool ImagePolygon::FixPolePoly(){
    // We currently do not support both poles in one image
    if (p_gMap->SetUniversalGround (90,0) && p_gMap->SetUniversalGround (-90,0)) {
      std::string msg = "Unable to create image footprint because image has both poles";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    /*------------------------------------------------------------------------
   /  When the points cross the 0/360 boundary, convert coordinates to
   /  either negative values if going from 0 to 360, or convert to values
   /  greater than 360 if going from 360 to 0.  This conversion is not
   /  done if the image contains a pole.  ??? Should the pole handling
   /  code be put into a different method ???
   /-----------------------------------------------------------------------*/
    bool poleAdded = false;  // only add pole 1x
    bool convertLon = false;
    bool newCoords = false;  //  coordinates have been adjusted
    geos::geom::CoordinateSequence *newPts = new geos::geom::CoordinateArraySequence ();
    double lon,lat;
    double fromLon=0,toLon=0;
    double prevLon = p_pts->getAt(0).x;
    double prevLat = p_pts->getAt(0).y;
    newPts->add (geos::geom::Coordinate (prevLon,prevLat));
    for (unsigned int i=1; i<p_pts->getSize (); i++) {
      lon = p_pts->getAt(i).x;
      lat = p_pts->getAt(i).y;
      if (abs(lon - prevLon) > 180) {
        newCoords = true;
        if (convertLon) {
          convertLon = false;
        } else {
          if ((lon - prevLon) > 0) {
            fromLon = 0.;
            toLon = 360.;
          } else if ((lon - prevLon) < 0) {
            fromLon = 360.;
            toLon = 0.;
          }

          if (!poleAdded) {
            newPts->add (geos::geom::Coordinate (fromLon,prevLat));
            newPts->add (geos::geom::Coordinate (fromLon,p_pole));
            newPts->add (geos::geom::Coordinate (toLon,p_pole));
            newPts->add (geos::geom::Coordinate (toLon,prevLat));
            poleAdded = true;
          }
        }
      }

      prevLon = lon;
      prevLat = lat;

      /*----------------------------------------------------------------------
      /  Meridian images-convert lon so that all are in same coordinate
      /  sequence.
      /---------------------------------------------------------------------*/
      if (convertLon) {
        if (fromLon == 360.) lon = 360. + abs(lon);
        if (fromLon == 0.) lon = lon - 360.;
      }

      newPts->add (geos::geom::Coordinate (lon,lat));
      /*----------------------------------------------------------------------
      /  Meridian images-convert lon back to original value.
      /---------------------------------------------------------------------*/
      if (convertLon) {
        if (fromLon == 360.) lon = lon - 360.;
        if (fromLon == 0.) lon = lon + 360.;
      }
    }

    /*----------------------------------------------------------------------
    /  If this is a sub-polygon, this particular poly might not have the
    /  boundary or pole, so just return the input poly.
    /---------------------------------------------------------------------*/
    if (!newCoords) {
      delete newPts;
      return false;
    }

    /*-----------------------------------------------------------------------
    /  Create polygon with the new converted points.
    /----------------------------------------------------------------------*/
    try {
      vector<geos::geom::Geometry *> *polys = new vector<geos::geom::Geometry *>;
      geos::geom::CoordinateSequence *pts = new geos::geom::CoordinateArraySequence ();

      polys->push_back (globalFactory.createPolygon
                        (globalFactory.createLinearRing (newPts),NULL));

      p_polygons = globalFactory.createMultiPolygon (*polys);

      delete polys;
      delete newPts;
      delete pts;
    } catch (geos::util::IllegalArgumentException *geosIll) {
      std::string msg = "Unable to create image footprint (FixPolePoly) due to ";
      msg += "geos illegal argument [" + iString(geosIll->what()) + "]";
      delete geosIll;
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    } catch (geos::util::GEOSException *geosExc) {
      std::string msg = "Unable to create image footprint (FixPolePoly) due to ";
      msg += "geos exception [" + iString(geosExc->what()) + "]";
      delete geosExc;
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    return true;
  }

  /**
  * If the cube crosses the 0/360 boundary and does not include a pole,
  * the polygon is separated into two polygons, one on each side of the
  * boundary.  These two polygons are put into a geos Multipolygon.  If
  * the image includes a pole, the polygon is left as a single polygon in
  * the geos Multipolygon.
  * 
  * Returns True if pts have changed,  if input is a sub-poly and doesn't
  *  contain the boundary, there is no change.
  */
  bool ImagePolygon::Fix360Poly () {
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
      if (abs(lon - prevLon) > 180) {
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

    // Nothing was done so return false
    if (!newCoords) {
      delete newPts;
      return false;
    }
    // bisect into seperate polygons
    try {
      geos::geom::Polygon *newPoly = globalFactory.createPolygon
                                     (globalFactory.createLinearRing(newPts),NULL);
      vector<geos::geom::Geometry *> *polys = new vector<geos::geom::Geometry *>;
      geos::geom::CoordinateSequence *pts = new geos::geom::CoordinateArraySequence ();
      geos::geom::CoordinateSequence *pts2 = new geos::geom::CoordinateArraySequence ();

      // Depending on direction of compensation bound accordingly
      if (negAdjust) {
        pts->add (geos::geom::Coordinate (0.,90.));
        pts->add (geos::geom::Coordinate (-180.,90.));
        pts->add (geos::geom::Coordinate (-180.,-90.));
        pts->add (geos::geom::Coordinate (0.,-90.));
        pts->add (geos::geom::Coordinate (0.,90.));
        pts2->add (geos::geom::Coordinate (0.,90.));
        pts2->add (geos::geom::Coordinate (180.,90.));
        pts2->add (geos::geom::Coordinate (180.,-90.));
        pts2->add (geos::geom::Coordinate (0.,-90.));
        pts2->add (geos::geom::Coordinate (0.,90.));
      } else {
        pts->add (geos::geom::Coordinate (360.,90.));
        pts->add (geos::geom::Coordinate (540.,90.));
        pts->add (geos::geom::Coordinate (540.,-90.));
        pts->add (geos::geom::Coordinate (360.,-90.));
        pts->add (geos::geom::Coordinate (360.,90.));
        pts2->add (geos::geom::Coordinate (360.,90.));
        pts2->add (geos::geom::Coordinate (180.,90.));
        pts2->add (geos::geom::Coordinate (180.,-90.));
        pts2->add (geos::geom::Coordinate (360.,-90.));
        pts2->add (geos::geom::Coordinate (360.,90.));
      }
      geos::geom::Polygon *boundaryPoly = globalFactory.createPolygon
                                          (globalFactory.createLinearRing(pts),NULL);
      geos::geom::Polygon *boundaryPoly2 = globalFactory.createPolygon
                                           (globalFactory.createLinearRing(pts2),NULL);

      /*------------------------------------------------------------------------
        /  Intersecting the original polygon (converted coordinates) with the 
        /  boundary polygon will create
        /  the right side polygon with the converted coordinates.  These will need
        /  to be converted back to the original coordinates.
        /-----------------------------------------------------------------------*/

      geos::geom::MultiPolygon *convertPoly = 
      PolygonTools::MakeMultiPolygon( newPoly->intersection(boundaryPoly));

      geos::geom::MultiPolygon *convertPoly2 = 
      PolygonTools::MakeMultiPolygon( newPoly->intersection(boundaryPoly2));

      /*------------------------------------------------------------------------
      /  Get rid of any points on the other side of the boundary due to
      /  constructed points from geos intersections.  These points are created
      /  from intersections between line segments in the edges of the polygons.
      /  Constructed points are not constructed exactly because of precision
      /  problems.  So if fromLon = 0 , check for points in the positive x
      /  for the convertPoly and check for points in the negative x
      /  for the polygon left after subtraction.
      /  If fromLon = 360, check for points less than 360 in x for the convertPoly
      /  and for points greater than 360 for the polygon left after subtraction.
      /-----------------------------------------------------------------------*/
      vector<geos::geom::Geometry *> *finalpolys = new vector<geos::geom::Geometry *>;
      for (unsigned int i=0; i<convertPoly->getNumGeometries();i++) {
        geos::geom::Geometry *newGeom = (convertPoly->getGeometryN(i))->clone();
        if (newGeom->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
          polys->push_back(newGeom);
        } else {
          std::string msg = "Should only contain polygons";
          throw iException::Message(iException::Programmer,msg,_FILEINFO_);
        }
      }
      for (unsigned int i=0; i<convertPoly2->getNumGeometries();i++) {
        geos::geom::Geometry *newGeom = (convertPoly2->getGeometryN(i))->clone();
        if (newGeom->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
          polys->push_back(newGeom);
        } else {
          std::string msg = "Should only contain polygons";
          throw iException::Message(iException::Programmer,msg,_FILEINFO_);
        }
      }
      // Fix the points
      geos::geom::CoordinateSequence *newPts = new geos::geom::CoordinateArraySequence ();
      for (unsigned int i = 0; i< polys->size() ; i++) {
        pts = polys->at(i)->getCoordinates();
        newPts = new geos::geom::CoordinateArraySequence ();
        for (unsigned int k=0; k < pts->getSize() ; k++) {
          lon = pts->getAt(k).x;
          // Make sure to check boundary cases to see what side it should be on
          if (negAdjust) {
            if (lon <= 0) {
              if (lon != 0) {
                lon = lon + 360;
              } else {
                // setup a direction to search through polygon
                // (to see what side of the meridian this point belongs on)
                int delta = k-(pts->getSize()/2);
                if (delta > 0) {
                  delta = -1;
                } else {
                  delta = 1;
                }
                unsigned int m = k;
                // should exit while loop after a few iterations
                while (m < pts->getSize() && m >=0) {
                  if (pts->getAt(m).x < 0) {
                    lon = lon + 360;
                    break;
                  } else if (pts->getAt(m).x > 0) {
                    break;
                  }
                  // if search point == 0 keep searching
                  m = m + delta;
                }
              }

            }
          } else {
            if (lon >= 360) {
              if (lon != 360) {
                lon = lon - 360;
              } else {
                // setup a direction to search through polygon
                // (to see what side of the meridian this point belongs on)
                int delta = k-(pts->getSize()/2);
                if (delta > 0) {
                  delta = -1;
                } else {
                  delta = 1;
                }
                unsigned int m = k;
                // should exit while loop after a few iterations
                while (m < pts->getSize() && m >=0) {
                  if (pts->getAt(m).x < 360) {
                    break;
                  } else if (pts->getAt(m).x > 360) {
                    lon = lon - 360;
                    break;
                  }
                  // if search point == 360 keep searching
                  m = m + delta;
                }
              }
            }
          }

          geos::geom::Coordinate newPt;
          newPt.x = lon;
          newPt.y = pts->getAt(k).y;
          newPts->add(newPt,k);
        }
        // Add the points to polys
        finalpolys->push_back (globalFactory.createPolygon
                               (globalFactory.createLinearRing (newPts),NULL));
      }

      p_polygons = globalFactory.createMultiPolygon (*finalpolys);

      delete finalpolys;
      delete polys;
      delete pts;
      delete pts2;
      delete newPts;
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
    }

    // Convert points back to what they were

    return true;
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

} // end namespace isis

