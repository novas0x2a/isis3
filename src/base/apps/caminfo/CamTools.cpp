/* @file                                                                  
 * $Revision: 1.7 $
 * $Date: 2009/01/06 22:22:28 $
 * $Id: CamTools.cpp,v 1.7 2009/01/06 22:22:28 skoechle Exp $
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
#include <cmath>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "CamTools.h"
#include "CameraFactory.h"
#include "Pvl.h"
#include "SpecialPixel.h"
#include "iTime.h"
#include "iString.h"
#include "iException.h"
#include "Statistics.h"
#include "SpiceUsr.h"
#include "ProjectionFactory.h"
#include "PolygonTools.h"

#include "geos/geom/GeometryFactory.h"
#include "geos/geom/Geometry.h"
#include "geos/geom/Point.h"

using namespace std;


inline double SetRound(double value, const int precision) {
    double scale = pow(10.0, precision);
    value = round(value * scale) / scale;
    return (value);
}

namespace Isis {

bool BandGeometry::isPointValid(const double &sample, const double &line, 
                                const Camera *camera) const {
  int nl(_nLines), ns(_nSamps);
  if ( camera != 0 ) {
    nl = camera->Lines();
    ns = camera->Samples();
  } 

  if (line < 0.5) return (false);
  if (line > (nl+0.5)) return (false);
  if (sample < 0.5) return (false);
  if (sample > (ns+0.5)) return (false);
  return (true);
}

bool BandGeometry::hasCenterGeometry() const {
   BandPropertiesListConstIter b;
   for ( b = _gBandList.begin() ; b != _gBandList.end() ; ++b ) {
     if ( !IsSpecial(b->centerLatitude) ) return (true);
   }
   // No valid center exists
   return (false);
}


void BandGeometry::destruct() {
  // Ensure this can be applied for reentrant operations
  _gBandList.clear();
   for_each(_polys.begin(), _polys.end(), DeleteObject());
   delete _combined;
   _combined = 0;
   _radius = 1.0;
}

void BandGeometry::collect(Camera &camera, Cube &cube) {
  destruct();

  _nLines  = cube.Lines();
  _nSamps  = cube.Samples();
  _nBands  = cube.Bands();

  //  Compute average planetary radius in meters.  This is used as a fallback
  //  to compute surface area if no geoemetry has a center intersect point.
  double radii[3];
  camera.Radii(radii);
  _radius = (radii[0] + radii[1] + radii[2]) / 3.0 * 1000.0;

  double cLine = _nLines;
  double cSamp = _nSamps;
  double centerLine = cLine / 2.0;
  double centerSamp = cSamp / 2.0;

  // Check to determine if geometry is band independant
  _isBandIndependent = camera.IsBandIndependent();
  _hasCenterGeom = false;
  int nbands = (_isBandIndependent ? 1 : _nBands);
  for ( int band = 0 ; band < nbands ; band++ ) {
    //  Compute band geometry properties
    GProperties g;
    g.lines = _nLines;
    g.samples = _nSamps;
    g.bands = _nBands;
    g.band = band + 1;
    camera.SetBand(band+1);
    g.realBand = cube.PhysicalBand(band+1);


    g.target = camera.Target();

    iTime t1(camera.CacheStartTime());
    g.startTime = t1.UTC();
    iTime t2(camera.CacheEndTime());
    g.endTime = t2.UTC();

    g.centerLine = centerLine;
    g.centerSamp = centerSamp;

    // Now compute elements for the center pixel
    if (camera.SetImage(centerSamp,centerLine)) {
      _hasCenterGeom = true;
      g.centerLatitude  = camera.UniversalLatitude();
      g.centerLongitude = camera.UniversalLongitude();
      g.radius = camera.LocalRadius();

      g.rightAscension = camera.RightAscension();
      g.declination    = camera.Declination();

      g.sampRes = camera.SampleResolution();
      g.lineRes = camera.LineResolution();

      g.solarLongitude = camera.SolarLongitude();
      g.northAzimuth = camera.NorthAzimuth();
      g.offNader = camera.OffNadirAngle();
      g.subSolarAzimuth = camera.SunAzimuth();
      g.subSpacecraftAzimuth = camera.SpacecraftAzimuth();
      g.localSolartime = camera.LocalSolarTime();
      g.targetCenterDistance = camera.TargetCenterDistance();
      g.slantDistance = camera.SlantDistance();

      camera.SubSolarPoint(g.subSolarLatitude,g.subSolarLongitude);
      g.subSolarGroundAzimuth = camera.GroundAzimuth(g.centerLatitude, 
                                                     g.centerLongitude,
                                                     g.subSolarLatitude,
                                                     g.subSolarLongitude);
      camera.SubSpacecraftPoint(g.subSpacecraftLatitude,g.subSpacecraftLongitude);
      g.subSpacecraftGroundAzimuth = camera.GroundAzimuth(g.centerLatitude, 
                                                          g.centerLongitude,
                                                          g.subSpacecraftLatitude, 
                                                          g.subSpacecraftLongitude);


        //  solve for the parallax and shadow stuff
      g.phase = camera.PhaseAngle();
      g.emi   = camera.EmissionAngle();
      g.inc   = camera.IncidenceAngle();

       //  Need some radian values
      if ( !IsSpecial(g.emi) && !IsSpecial(g.subSpacecraftAzimuth) ) {
        double emi_r = DegToRad(g.emi);
        g.parallaxx = RadToDeg(-tan(emi_r)*cos(DegToRad(g.subSpacecraftAzimuth)));
        g.parallaxy = RadToDeg(tan(emi_r)*sin(DegToRad(g.subSpacecraftAzimuth))); }

      if ( !IsSpecial(g.inc) && !IsSpecial(g.subSolarAzimuth) ) {
        double inc_r = DegToRad(g.inc);
        g.shadowx = RadToDeg(-tan(inc_r)*cos(DegToRad(g.subSolarAzimuth)));
        g.shadowy = RadToDeg(tan(inc_r)*sin(DegToRad(g.subSolarAzimuth)));
      }
    }
    //  OK...now get corner pixel geometry.  NOTE this resets image
    //  pixel location from center!!!
    if ( camera.SetImage(1.0, 1.0) ) {
      g.upperLeftLongitude = camera.UniversalLongitude();
      g.upperLeftLatitude =  camera.UniversalLatitude();
    }

    if ( camera.SetImage(1.0, cLine) ) {
      g.lowerLeftLongitude = camera.UniversalLongitude();
      g.lowerLeftLatitude =  camera.UniversalLatitude();
    }

    if ( camera.SetImage(cSamp, cLine) ) {
      g.lowerRightLongitude = camera.UniversalLongitude();
      g.lowerRightLatitude =  camera.UniversalLatitude();
    }

    if ( camera.SetImage(cSamp, 1.0) ) {
      g.upperRightLongitude = camera.UniversalLongitude();
      g.upperRightLatitude =  camera.UniversalLatitude();
    }

    double minRes = camera.LowestImageResolution();
    double maxRes = camera.HighestImageResolution();
    if ( !(IsSpecial(minRes) || IsSpecial(maxRes)) ) {
      g.grRes = (minRes+maxRes)/2.0;
    }

    Pvl camMap;
    camera.BasicMapping(camMap);
    _mapping = camMap;

    // Test for interesting intersections
    if (camera.IntersectsLongitudeDomain(camMap)) g.hasLongitudeBoundary = true;
    if (camera.SetUniversalGround(90.0, 0.0)) {
      if ( isPointValid(camera.Sample(), camera.Line(), &camera) ) {
        g.hasNorthPole = true;
      }
    }
    if (camera.SetUniversalGround(-90.0, 0.0)) {
      if ( isPointValid(camera.Sample(), camera.Line(), &camera) ) {
        g.hasSouthPole = true;
      }
    }

    // Now compute the the image polygon
    ImagePolygon poly;
    poly.Create(cube,0,1,1,0,0,band+1);
    geos::geom::MultiPolygon *multiP = poly.Polys();
    _polys.push_back(multiP->clone());
    if (_combined == 0) {
      _combined = multiP->clone();
    }
    else {
      //  Construct composite (union) polygon
      geos::geom::Geometry *old(_combined);
      _combined = old->Union(multiP);
      delete old;
    }

    Projection *sinu = ProjectionFactory::Create(camMap, true);
    geos::geom::MultiPolygon *sPoly = PolygonTools::LatLonToXY(*multiP, sinu);
    geos::geom::Point *center = sPoly->getCentroid();

    sinu->SetCoordinate(center->getX(), center->getY());
    g.centroidLongitude = sinu->UniversalLongitude();
    g.centroidLatitude  = sinu->UniversalLatitude();
    g.surfaceArea = sPoly->getArea() / (1000.0 * 1000.0);
    delete center;
    delete sPoly;
    delete sinu;

    if (camera.SetUniversalGround(g.centroidLatitude, g.centroidLongitude)) {
      g.centroidLine = camera.Line();
      g.centroidSample = camera.Sample();
      g.centroidRadius = camera.LocalRadius();
    }

    // Save off this band geometry property
    _gBandList.push_back(g);

  }

  //  Compute the remainder of the summary bands since some of the operations
  //  need the camera model
  _summary = getGeometrySummary();
  if ( size() != 1 ) {     
    Projection *sinu = ProjectionFactory::Create(_mapping, true);
    geos::geom::MultiPolygon *multiP = makeMultiPolygon(_combined);
    geos::geom::MultiPolygon *sPoly = PolygonTools::LatLonToXY(*multiP, sinu); 
    geos::geom::Point *center = sPoly->getCentroid();

    sinu->SetCoordinate(center->getX(), center->getY());
    _summary.centroidLongitude = sinu->UniversalLongitude();
    _summary.centroidLatitude  = sinu->UniversalLatitude();
    _summary.surfaceArea = sPoly->getArea() / (1000.0 * 1000.0);
    delete center;
    delete sPoly;
    delete multiP;
    delete sinu;

    if (camera.SetUniversalGround(_summary.centroidLatitude, 
                                  _summary.centroidLongitude)) { 
      _summary.centroidLine = camera.Line(); 
      _summary.centroidSample = camera.Sample();
      _summary.centroidRadius = camera.LocalRadius();
    }
  }

  return;
}


void BandGeometry::generateGeometryKeys(PvlObject &pband) {
  if ( size() <= 0 ) {
    std::string mess = "No Band geometry available!";
    iException::Message(iException::Programmer, mess, _FILEINFO_);
  }

  GProperties g = getGeometrySummary();

//geometry keywords for band output
  pband += PvlKeyword("BandsUsed", size());
  pband += PvlKeyword("ReferenceBand", g.band);
  pband += PvlKeyword("OriginalBand", g.realBand);

  pband += PvlKeyword("Target", g.target);

  pband += PvlKeyword("StartTime",g.startTime);
  pband += PvlKeyword("EndTime",g.endTime);

  pband += ValidateKey("CenterLine",g.centerLine);
  pband += ValidateKey("CenterSample",g.centerSamp);
  pband += ValidateKey("CenterLatitude",g.centerLatitude);
  pband += ValidateKey("CenterLongitude",g.centerLongitude);
  pband += ValidateKey("CenterRadius",g.radius);

  pband += ValidateKey("RightAscension",g.rightAscension);
  pband += ValidateKey("Declination",g.declination);

  pband += ValidateKey("CentroidLatitude",g.centroidLatitude);
  pband += ValidateKey("CentroidLongitude",g.centroidLongitude);
  pband += ValidateKey("CentroidLine",g.centroidLine);
  pband += ValidateKey("CentroidSample",g.centroidSample);
  pband += ValidateKey("CentroidRadius",g.centroidRadius);
  pband += ValidateKey("SurfaceArea",g.surfaceArea);

  pband += ValidateKey("UpperLeftLongitude",g.upperLeftLongitude);
  pband += ValidateKey("UpperLeftLatitude",g.upperLeftLatitude);
  pband += ValidateKey("LowerLeftLongitude",g.lowerLeftLongitude);
  pband += ValidateKey("LowerLeftLatitude",g.lowerLeftLatitude);
  pband += ValidateKey("LowerRightLongitude",g.lowerRightLongitude);
  pband += ValidateKey("LowerRightLatitude",g.lowerRightLatitude);
  pband += ValidateKey("UpperRightLongitude",g.upperRightLongitude);
  pband += ValidateKey("UpperRightLatitude",g.upperRightLatitude);

  pband += ValidateKey("PhaseAngle",g.phase);
  pband += ValidateKey("EmissionAngle",g.emi);
  pband += ValidateKey("IncidenceAngle",g.inc);

  pband += ValidateKey("NorthAzimuth",g.northAzimuth);
  pband += ValidateKey("OffNadir",g.offNader);
  pband += ValidateKey("SolarLongitude",g.solarLongitude);
  pband += ValidateKey("LocalTime",g.localSolartime);
  pband += ValidateKey("TargetCenterDistance",g.targetCenterDistance);
  pband += ValidateKey("SlantDistance",g.slantDistance);

  double aveRes(Null);
  if (!IsSpecial(g.sampRes) && !IsSpecial(g.lineRes)) {
    aveRes = (g.sampRes + g.lineRes) / 2.0;
  }

  pband += ValidateKey("SampleResolution",g.sampRes);
  pband += ValidateKey("LineResolution",g.lineRes);
  pband += ValidateKey("PixelResolution",aveRes);
  pband += ValidateKey("MeanGroundResolution",g.grRes);

  pband += ValidateKey("SubSolarAzimuth",g.subSolarAzimuth);
  pband += ValidateKey("SubSolarGroundAzimuth",g.subSolarGroundAzimuth);
  pband += ValidateKey("SubSolarLatitude", g.subSolarLatitude);
  pband += ValidateKey("SubSolarLongitude",g.subSolarLongitude);

  pband += ValidateKey("SubSpacecraftAzimuth", g.subSpacecraftAzimuth);
  pband += ValidateKey("SubSpacecraftGroundAzimuth", g.subSpacecraftGroundAzimuth);
  pband += ValidateKey("SubSpacecraftLatitude",g.subSpacecraftLatitude);
  pband += ValidateKey("SubSpacecraftLongitude",g.subSpacecraftLongitude);

  pband += ValidateKey("ParallaxX",g.parallaxx);
  pband += ValidateKey("ParallaxY",g.parallaxy);

  pband += ValidateKey("ShadowX",g.shadowx);
  pband += ValidateKey("ShadowY",g.shadowy);

  //  Determine if image crosses Longitude domain
  if ( g.hasLongitudeBoundary ) {
    pband += PvlKeyword("HasLongitudeBoundary","TRUE");
  }
  else {
    pband += PvlKeyword("HasLongitudeBoundary","FALSE");
  }

  //  Add test for North pole in image
  if (g.hasNorthPole) {
    pband += PvlKeyword("HasNorthPole", "TRUE");
  }
  else {
    pband += PvlKeyword("HasNorthPole", "FALSE");
  }

  //  Add test for South pole in image
  if (g.hasSouthPole) {
    pband += PvlKeyword("HasSouthPole", "TRUE");
  }
  else {
    pband += PvlKeyword("HasSouthPole", "FALSE");
  }

  return;
}

BandGeometry::GProperties BandGeometry::getGeometrySummary() const {
  if ( _isBandIndependent  || (size() == 1)) {
     return (_gBandList[0]);
  }

  //  Get the centroid point of the union polygon
  geos::geom::Point *center = _combined->getCentroid();
  double plon(center->getX()), plat(center->getY());
  delete center;

  GProperties bestBand;
  double centerDistance(DBL_MAX);

  GProperties corners;
  double ulDist(DBL_MIN), urDist(DBL_MIN), 
         lrDist(DBL_MIN), llDist(DBL_MIN);

  double radius = getRadius();

  BandPropertiesListConstIter b;
  for ( b = _gBandList.begin() ; b != _gBandList.end() ; ++b ) {
    double thisDist;
    bool isCloser = isDistShorter(centerDistance, plat, plon,
                                  b->centerLatitude,b->centerLongitude,
                                  radius, thisDist) ;
    if (isCloser) {
        bestBand = *b;
        centerDistance = thisDist;
      }

    //  Do upper left and right corners
    isCloser = isDistShorter(ulDist, plat, plon,
                             b->upperLeftLatitude, b->upperLeftLongitude,
                             radius, thisDist);
    if ( !isCloser ) {
      corners.upperLeftLatitude = b->upperLeftLatitude;
      corners.upperLeftLongitude = b->upperLeftLongitude;
      ulDist = thisDist;
    }

    isCloser = isDistShorter(urDist, plat, plon,
                             b->upperRightLatitude, b->upperRightLongitude,
                             radius, thisDist);
    if ( !isCloser ) {
      corners.upperRightLatitude = b->upperRightLatitude;
      corners.upperRightLongitude = b->upperRightLongitude;
      urDist = thisDist;
    }

   //  Do lower left and right corners
    isCloser = isDistShorter(llDist, plat, plon,
                             b->lowerLeftLatitude, b->lowerLeftLongitude,
                             radius, thisDist);
    if ( !isCloser ) {
      corners.lowerLeftLatitude = b->lowerLeftLatitude;
      corners.lowerLeftLongitude = b->lowerLeftLongitude;
      llDist = thisDist;
    }

    isCloser = isDistShorter(lrDist, plat, plon,
                             b->lowerRightLatitude, b->lowerRightLongitude,
                             radius, thisDist);
    if ( !isCloser ) {
      corners.lowerRightLatitude = b->lowerRightLatitude;
      corners.lowerRightLongitude = b->lowerRightLongitude;
      lrDist = thisDist;
    }
  }

  // Add the corners to the returning property
  bestBand.upperLeftLatitude = corners.upperLeftLatitude;
  bestBand.upperLeftLongitude = corners.upperLeftLongitude;
  bestBand.upperRightLatitude = corners.upperRightLatitude;
  bestBand.upperRightLongitude = corners.upperRightLongitude;
  bestBand.lowerLeftLatitude = corners.lowerLeftLatitude;
  bestBand.lowerLeftLongitude = corners.lowerLeftLongitude;
  bestBand.lowerRightLatitude = corners.lowerRightLatitude;
  bestBand.lowerRightLongitude = corners.lowerRightLongitude;
  return (bestBand);
}

void BandGeometry::generatePolygonKeys(PvlObject &pband) {
  if ( size() <= 0 ) {
    std::string mess = "No Band geometry available!";
    iException::Message(iException::Programmer, mess, _FILEINFO_);
  }

   // Compute surface area - already done in collection phase
  double radius = getRadius();
  double globalCoverage(Null);
  if ( !IsSpecial(radius) ) {
    double globalArea = 4.0 * pi_c() * (radius * radius) / (1000.0*1000.0);
    globalCoverage = _summary.surfaceArea / globalArea * 100.0;
    globalCoverage = SetRound(globalCoverage, 6);
  }
 
  pband += PvlKeyword("CentroidLatitude", _summary.centroidLatitude);
  pband += PvlKeyword("CentroidLongitude", _summary.centroidLongitude);
  pband += ValidateKey("Radius",radius,"meters");
  pband += ValidateKey("SurfaceArea",_summary.surfaceArea,"km^2");
  pband += ValidateKey("GlobalCoverage",globalCoverage,"percent");
  if ( _combined->getGeometryTypeId() != geos::geom::GEOS_MULTIPOLYGON ) {
    geos::geom::MultiPolygon *geom = makeMultiPolygon(_combined);
    pband += PvlKeyword("GisFootprint", geom->toString());
    delete geom;
  }
  else {
    pband += PvlKeyword("GisFootprint", _combined->toString());
  }

  return;
}

double BandGeometry::getRadius() const {
  Statistics polyRadius, centRadius;
  BandPropertiesListConstIter b;
  for ( b = _gBandList.begin() ; b != _gBandList.end() ; ++b ) {
    polyRadius.AddData(b->centroidRadius);
    centRadius.AddData(b->radius);
  }
  double radius = polyRadius.Average();
  if ( IsSpecial(radius) ) radius = centRadius.Average();
  if ( IsSpecial(radius) ) radius = _radius;
  return (radius);
}

double BandGeometry::getPixelResolution() const {
  Statistics groundRes, pixelRes;
   BandPropertiesListConstIter b;
   for ( b = _gBandList.begin() ; b != _gBandList.end() ; ++b ) {
     pixelRes.AddData(b->sampRes);
     pixelRes.AddData(b->lineRes);
     groundRes.AddData(b->grRes);
   }

   double res = groundRes.Average();
   if ( IsSpecial(res) ) res = pixelRes.Average();
   return (res);
}

double BandGeometry::getPixelsPerDegree(double pixres, 
                                        double radius) const {
  double circumference = 2.0  * pi_c() * radius;
  double metersPerDegree = circumference / 360.0;
  double pixelsPerDegree = metersPerDegree / pixres;
  return (pixelsPerDegree);
}


bool BandGeometry::isDistShorter(double bestDist, double lat1, double lon1, 
                                 double lat2, double lon2, double radius, 
                                 double &thisDist) const {
  if ( IsSpecial(lat1) ) return (false);
  if ( IsSpecial(lon1) ) return (false);
  if ( IsSpecial(lat2) ) return (false);
  if ( IsSpecial(lon2) ) return (false);
  if ( IsSpecial(radius) ) return (false);

  thisDist = Camera::Distance(lat1, lon1, lat2, lon2, radius);
  return (thisDist < bestDist);
}

geos::geom::MultiPolygon *BandGeometry::makeMultiPolygon(
                                           geos::geom::Geometry *g) const {
  vector<geos::geom::Geometry *> polys;
  polys.push_back(g);
  const geos::geom::GeometryFactory *gfactory = geos::geom::GeometryFactory::getDefaultInstance();
  return (gfactory->createMultiPolygon(polys));
}


} // namespace Isis
