#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>

#include "geos/operation/distance/DistanceOp.h"
#include "geos/util/IllegalArgumentException.h"
#include "geos/opOverlay.h"

#include "iException.h"
#include "Cube.h"
#include "SerialNumberList.h"
#include "PolygonTools.h"
#include "ImagePolygon.h"
#include "Filename.h"
#include "ImageOverlapSet.h"
#include "Progress.h"

using namespace std;

namespace Isis {

  /**
   * Create FindImageOverlaps object.
   * 
   * Create an empty FindImageOverlaps object.
   * 
   * @see automaticRegistration.doc
   */
  ImageOverlapSet::ImageOverlapSet(bool continueOnError) {
    p_continueAfterError = continueOnError;
  }


  /**
   * Delete this object.
   * 
   * Delete the FindImageOverlaps object. The stored ImageOverlaps
   * will be deleted as well.
   */
  ImageOverlapSet::~ImageOverlapSet() {
    for (int i=0; i<Size(); i++) {
      delete p_lonLatOverlaps[i]; 
    }
  };


  /**
   * Create polygons of overlap from the images specified in the serial number
   * list. All polygons create by this class will be deleted when it is
   * destroyed, so callers should not delete the polygons returned by various 
   * members. 
   *  
   * @param sns The serial number list to use when finding overlaps 
   *  
   * @param continueAfterError Causes the class to continue processing polygons 
   * even after polygon intersection and difference errors. 
   * 
   * @see automaticRegistration.doc
   */
  void ImageOverlapSet::FindImageOverlaps(SerialNumberList &sns) {
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

        HandleError(error, &sns, msg);
      }

      // Read the bounding polygon
      ImagePolygon *poly = new ImagePolygon();
      cube.Read(*poly);
      cube.Close();

      // Create a ImageOverlap with the serial number and the bounding
      // polygon and save it
      geos::geom::MultiPolygon *tmp = PolygonTools::MakeMultiPolygon(poly->Polys());

      delete poly;
      poly = NULL;

      geos::geom::MultiPolygon *mp = NULL;

      // If footprint is invalid throw exception
      if(!tmp->isValid()) {
        delete tmp;
        iString msg = "The image [" + sns.Filename(sns.SerialNumber(i)) + "] has an invalid footprint";
        throw iException::Message(iException::Programmer, msg, _FILEINFO_);
      }

      try {
        mp = PolygonTools::Despike (tmp);
      }
      catch (iException &e) {
        if(tmp->isValid()) {
          mp = tmp;
        }
        else {
          delete tmp;
          HandleError(e, &sns);
          continue;
        }
      }
      
      p_lonLatOverlaps.push_back(CreateNewOverlap(sns.SerialNumber(i), mp));

      if(mp) {
        delete mp;
        mp = NULL;
      }

      if(tmp) {
        delete tmp;
        tmp = NULL;
      }
    }

    // Determine the overlap between each boundary polygon
    FindAllOverlaps (&sns);
  }


  /**
   * Create polygons of overlap from the polygons specified. The serial numbers
   * and the polygons are assumed to be parallel arrays. The original polygons
   * passed as arguments are copied, so the ownership of the originals remains 
   * with the caller. 
   *  
   * @param sns The serial number list to use when finding overlaps 
   *  
   * @param polygons The polygons which are to be used when finding overlaps 
   *  
   * @param continueAfterError Causes the class to continue processing polygons 
   * even after polygon intersection and difference errors. 
   * 
   * @see automaticRegistration.doc
   */
  void ImageOverlapSet::FindImageOverlaps(std::vector<std::string> sns,
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
   * Create polygons of overlap from the file specified.
   *  
   * @param filename The file to read the image overlaps from
   */
  void ImageOverlapSet::ReadImageOverlaps(const std::string &filename) {
    iString file = Filename(filename).Expanded();

    try {
      // Let's get an istream pointed at our file
      std::ifstream inStream;
      inStream.open(file.c_str(), fstream::in);
  
      while(!inStream.eof()) {
        p_lonLatOverlaps.push_back(new ImageOverlap(inStream));
      }
  
      inStream.close();
    }
    catch(...) {
      iString msg = "The overlap file [" + filename + "] does not contain a valid list of image overlaps";
      throw iException::Message(iException::User, msg, _FILEINFO_);
    }
  }


  /**
   * Write polygons of overlap to the file specified.
   *  
   * @param filename The file to write the image overlaps to
   */
  void ImageOverlapSet::WriteImageOverlaps(const std::string &filename) {
    iString file = Filename(filename).Expanded();
    bool failed = false;

    try {
      // Let's get an ostream pointed at our file
      std::ofstream outStream;
      outStream.open(file.c_str(), fstream::out | fstream::trunc);
  
      failed |= outStream.fail();

      bool overlapWritten = false;
      for(unsigned int overlap = 0; !failed && overlap < p_lonLatOverlaps.size(); overlap++) {
        if(p_lonLatOverlaps[overlap]->Polygon()->getNumPoints() != 0) {
          if(overlapWritten) {
            outStream << std::endl;
          }
  
          p_lonLatOverlaps[overlap]->Write(outStream);
          overlapWritten = true;
        }
      }

      failed |= outStream.fail();
  
      outStream.close();

      failed |= outStream.fail();
    }
    catch(...) {
      failed = true;
    }

    if(failed) {
      iString msg = "Unable to write the image overlap list to [" + filename + "]";
      throw iException::Message(iException::Io, msg, _FILEINFO_);
    }
  }


  /**
   * Find the overlaps between all the existing PolygonsOverlapItems 
   *  
   * @param continueAfterError Causes the class to continue processing polygons 
   * even after polygon intersection and difference errors. 
   */
  void ImageOverlapSet::FindAllOverlaps (SerialNumberList *snlist) {
    if(p_lonLatOverlaps.size() <= 1) return;

    Progress p;
    p.SetText("Calculating Image Overlaps");
    p.SetMaximumSteps( p_lonLatOverlaps.size() - 1 );
    p.CheckStatus();

    // Compare each polygon with all of the others
    for (unsigned int outside=0; outside<p_lonLatOverlaps.size()-1; ++outside) {
      
      // Intersect the current polygon (from the outside loop) with all others
      // below it that existed when we started this inside loop. In other words
      // don't intersect the current polyon with any new polygons that were
      // created in this pass of the inside loop
      unsigned int end = p_lonLatOverlaps.size();
      for (unsigned int inside=outside+1; inside<end; ++inside) {
        // If the same SN appears in both overlaps then the intersection has
        // already been done and will create a spike, so don't do it.
        if (p_lonLatOverlaps.at(outside)->HasAnySameSerialNumber(*p_lonLatOverlaps.at(inside))) {
          continue;
        }

        // We know these are valid because they were filtered early on
        const geos::geom::MultiPolygon *poly1 = p_lonLatOverlaps.at(outside)->Polygon();
        const geos::geom::MultiPolygon *poly2 = p_lonLatOverlaps.at(inside)->Polygon();

        // Check to see if the two poygons are equivalent.
        // If they are, then we can get rid of one of them
        if (poly1->equals(poly2)) {
          AddSerialNumbers (p_lonLatOverlaps[outside],p_lonLatOverlaps[inside]);
          p_lonLatOverlaps.erase(p_lonLatOverlaps.begin()+inside);
          end --;
          inside --;
          continue;
        }

        // Get and process the intersection of the two polygons
        // TODO: The error handling on this needs improved, erasing
        //   both polygons is not a good thing to be doing.
        try {
          geos::geom::Geometry *intersected = NULL;
          try {
            intersected = PolygonTools::Intersect(poly1, poly2);
          }
          catch (iException &e) {
            HandleError(e, snlist, "Intersection of overlaps failed. All affected overlaps will be removed.", inside, outside);
            intersected = NULL;

            p_lonLatOverlaps.erase(p_lonLatOverlaps.begin()+inside);
            p_lonLatOverlaps.erase(p_lonLatOverlaps.begin()+outside);
            end -= 2;
            inside = outside;

            continue;
          }

          if(intersected->isEmpty()) {
            delete intersected;
            intersected = NULL;
            continue;
          }

          // We are only interested in overlaps that result in polygon(s)
          // and not any that are lines or points, so create a new multipolygon
          // with only the polygons of overlap
          geos::geom::MultiPolygon *overlap = NULL;
          try {
            overlap = PolygonTools::Despike(intersected);

            delete intersected;
            intersected = NULL;
          }
          catch (iException &e) {
            if(!intersected->isValid()) {
              delete intersected;
              intersected = NULL;

              HandleError(e, snlist, "", inside, outside);
              continue;
            }
            else {
              overlap = PolygonTools::MakeMultiPolygon(intersected);
              e.Clear();

              delete intersected;
              intersected = NULL;
            }
          }
          catch (geos::util::GEOSException *exc) {
            delete intersected;
            intersected = NULL;
            HandleError(exc, snlist, "", inside, outside);
            continue;
          }

          if (!overlap->isValid()) {
            delete overlap;
            overlap = NULL;
            HandleError(snlist, "Intersection produced invalid overlap area", inside, outside);
            continue;
          }

          // Process the overlap if it contains a usable area
          if (!overlap->isEmpty() && overlap->getArea() > 1.0e-14) {
            // poly1 is completly inside poly2
            if (poly1->equals(overlap)) {
              AddSerialNumbers (p_lonLatOverlaps[outside], p_lonLatOverlaps[inside]);

              geos::geom::Geometry *tmpGeom = NULL;
              try {
                tmpGeom = PolygonTools::Difference(poly2, poly1);
              }
              catch (geos::util::GEOSException *exc) {
                HandleError(exc, snlist, "Differencing overlap polygons failed", inside, outside);
                continue;
              }

              try {
                geos::geom::MultiPolygon *despiked = PolygonTools::Despike(tmpGeom);
                p_lonLatOverlaps.at(inside)->SetPolygon(despiked);

                delete despiked;
                despiked = NULL;
                delete tmpGeom;
                tmpGeom = NULL;
              } 
              catch (iException &e) {
                if(tmpGeom->isValid()) {
                  geos::geom::MultiPolygon *tmpPoly = PolygonTools::MakeMultiPolygon(tmpGeom);
                  p_lonLatOverlaps.at(inside)->SetPolygon(tmpPoly);

                  delete tmpPoly;
                  tmpPoly = NULL;
                  delete tmpGeom;
                  tmpGeom = NULL;
                }
                else {
                  HandleError(e, snlist, "", inside, outside);
                }
              }

              delete overlap;
            }

            // poly2 is completly inside poly1
            else if (poly2->equals(overlap)) {
              AddSerialNumbers (p_lonLatOverlaps[inside], p_lonLatOverlaps[outside]);
              geos::geom::Geometry *tmpGeom = NULL;
              try {
                tmpGeom = PolygonTools::Difference(poly1, poly2);
              }
              catch (iException &e) {
                HandleError(e, snlist, "Differencing overlap polygons failed", inside, outside);
                continue;
              }

              try {
                geos::geom::MultiPolygon *tmpPoly = PolygonTools::MakeMultiPolygon(tmpGeom);
                p_lonLatOverlaps.at(outside)->SetPolygon(tmpPoly);

                delete tmpGeom;
                tmpGeom = NULL;

                delete tmpPoly;
                tmpPoly = NULL;
              } 
              catch (iException &e) {
                if(tmpGeom->isValid()) {
                  geos::geom::MultiPolygon *tmpPoly = PolygonTools::MakeMultiPolygon(tmpGeom);
                  p_lonLatOverlaps.at(outside)->SetPolygon(tmpPoly);

                  delete tmpPoly;
                  tmpPoly = NULL;
                  delete tmpGeom;
                  tmpGeom = NULL;
                }
                else {
                  HandleError(e, snlist, "", inside, outside);
                }
              }

              delete overlap;
            }

            // There is partial overlap
            else {
              ImageOverlap *po = new ImageOverlap();
              po->SetPolygon(overlap);
              AddSerialNumbers (po, p_lonLatOverlaps[outside]);
              AddSerialNumbers (po, p_lonLatOverlaps[inside]);
              p_lonLatOverlaps.push_back(po);
              p.AddSteps(1);

              // ImageOverlap does a copy, so delete the overlap now that we're done with it
              delete overlap;
              overlap = NULL;

              geos::geom::MultiPolygon *poly1Copy = PolygonTools::MakeMultiPolygon(poly1);

              // Subtract poly2 from poly1 and set poly1 to the result
              geos::geom::Geometry *tmpGeom = NULL;
              try {
                tmpGeom = PolygonTools::Difference(poly1, poly2);
              }
              catch (iException &e) {
                HandleError(e, snlist, "Differencing overlap polygons failed", inside, outside);
                tmpGeom = NULL;
                continue;
              }

              try {
                geos::geom::MultiPolygon *tmpPoly = PolygonTools::MakeMultiPolygon(tmpGeom);
                p_lonLatOverlaps.at(outside)->SetPolygon(tmpPoly);

                delete tmpGeom;
                tmpGeom = NULL;

                delete tmpPoly;
                tmpPoly = NULL;
              } 
              catch (iException &e) {
                HandleError(e, snlist, "", inside, outside);
              }

              // Subtract poly1 from poly2 and set poly2 to the result
              geos::geom::Geometry *tmpGeom2 = NULL;
              try {
                tmpGeom2 = PolygonTools::Difference(poly2, poly1Copy);
              }
              catch (iException &e) {
                HandleError(e, snlist, "Differencing overlap polygons failed", inside, outside);
                continue;
              }

              delete poly1Copy;
              poly1Copy = NULL;

              try {
                geos::geom::MultiPolygon *tmpPoly = PolygonTools::Despike(tmpGeom2);

                p_lonLatOverlaps.at(inside)->SetPolygon(tmpPoly);

                delete tmpGeom2;
                tmpGeom2 = NULL;

                delete tmpPoly;
                tmpPoly = NULL;
              } 
              catch (iException &e) {
                if(tmpGeom2->isValid()) {
                  geos::geom::MultiPolygon *tmpPoly = PolygonTools::MakeMultiPolygon(tmpGeom2);
                  p_lonLatOverlaps.at(inside)->SetPolygon(tmpPoly);

                  delete tmpPoly;
                  tmpPoly = NULL;
                  delete tmpGeom;
                  tmpGeom = NULL;
                }
                else {
                  HandleError(e, snlist, "", inside, outside);
                  continue;
                }
              }
            } // End of partial overlap else
          } // Usable area of overlap
          else {
            delete overlap;
          }
        }
        // Collections are illegal as intersection argument
        catch (geos::util::IllegalArgumentException *ill) {
          HandleError(NULL, snlist, "Unable to find overlap", inside, outside);
        }
        catch (geos::util::GEOSException *exc) {
          HandleError(exc, snlist, "Unable to find overlap", inside, outside);
        }
        catch (...) {
          HandleError(snlist, "Unknown Error: Unable to find overlap", inside, outside);
        }
      }

      p.CheckStatus();
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
  void ImageOverlapSet::AddSerialNumbers (ImageOverlap *to, ImageOverlap *from) {
    for (int i=0; i<from->Size(); i++) {
      string s = (*from)[i];
      to->Add(s);
    }
  }


  /**
   * Create an overlap item to hold the overlap poly and its SN
   *
   * @param serialNumber The serial number 
   * 
   * @param latLonPolygon The object to copy the serial numbers from
   * 
   */
  ImageOverlap* ImageOverlapSet::CreateNewOverlap(std::string serialNumber,
      geos::geom::MultiPolygon* latLonPolygon) {

    return new ImageOverlap(serialNumber, *latLonPolygon);
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
  vector<ImageOverlap*> ImageOverlapSet::operator[](string serialNumber) {

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

  /**
   * If a problem occurred when searching for image overlaps, 
   * this method will handle it. 
   * 
   * @param e Isis Exception representing the problem
   */
  void ImageOverlapSet::HandleError(iException &e, SerialNumberList *snlist, iString msg, int overlap1, int overlap2) {
    PvlGroup err("ImageOverlapError");

    if(overlap1 >= 0 && (unsigned)overlap1 < p_lonLatOverlaps.size()) {
      PvlKeyword serialNumbers("PolySerialNumbers");
      PvlKeyword filename("Filenames");

      for (int i=0; i< p_lonLatOverlaps.at(overlap1)->Size(); i++) {
        serialNumbers += (*p_lonLatOverlaps.at(overlap1))[i];

        if(snlist != NULL) {
          filename += snlist->Filename((*p_lonLatOverlaps.at(overlap1))[i]);
        }
      }

      err += serialNumbers;
 
      if(filename.Size() != 0) {
        err += filename;
      }
    }

    if(overlap2 >= 0 && (unsigned)overlap1 < p_lonLatOverlaps.size() && (unsigned)overlap2 < p_lonLatOverlaps.size()) {
      PvlKeyword serialNumbers("PolySerialNumbers");
      PvlKeyword filename("Filenames");

      for (int i=0; i<p_lonLatOverlaps.at(overlap2)->Size(); i++) {
        serialNumbers += (*p_lonLatOverlaps.at(overlap2))[i];

        if(snlist != NULL) {
          filename += snlist->Filename((*p_lonLatOverlaps.at(overlap2))[i]);
        }
      }

      err += serialNumbers;

      if(filename.Size() != 0) {
        err += filename;
      }
    }

    err += PvlKeyword("Error", e.what());

    if(!msg.empty()) {
      err += PvlKeyword("Description", msg);
    }

    p_errorLog.push_back(err);

    if (!p_continueAfterError) throw;

    e.Clear();
  }

  /**
   * If a problem occurred when searching for image overlaps, 
   * this method will handle it. 
   * 
   * @param exc Geos exception representing the problem
   */
  void ImageOverlapSet::HandleError(geos::util::GEOSException *exc, SerialNumberList *snlist, iString msg, int overlap1, int overlap2) {
    PvlGroup err("ImageOverlapError");

    if(overlap1 >= 0 && (unsigned)overlap1 < p_lonLatOverlaps.size()) {
      PvlKeyword serialNumbers("PolySerialNumbers");
      PvlKeyword filename("Filenames");

      for (int i=0; i<p_lonLatOverlaps.at(overlap1)->Size(); i++) {
        serialNumbers += (*p_lonLatOverlaps.at(overlap1))[i];

        if(snlist != NULL) {
          filename += snlist->Filename((*p_lonLatOverlaps.at(overlap1))[i]);
        }
      }

      err += serialNumbers;

      if(filename.Size() != 0) {
        err += filename;
      }
    }

    if(overlap2 >= 0 && (unsigned)overlap1 < p_lonLatOverlaps.size() && (unsigned)overlap2 < p_lonLatOverlaps.size()) {
      PvlKeyword serialNumbers("PolySerialNumbers");
      PvlKeyword filename("Filenames");

      for (int i=0; i<p_lonLatOverlaps.at(overlap2)->Size(); i++) {
        serialNumbers += (*p_lonLatOverlaps.at(overlap2))[i];

        if(snlist != NULL) {
          filename += snlist->Filename((*p_lonLatOverlaps.at(overlap2))[i]);
        }
      }

      err += serialNumbers;

      if(filename.Size() != 0) {
        err += filename;
      }
    }

    err += PvlKeyword("Error", iString(exc->what()));

    if(!msg.empty()) {
      err += PvlKeyword("Description", msg);
    }

    p_errorLog.push_back(err);

    delete exc;

    if (!p_continueAfterError) {
      throw iException::Message(iException::Programmer, err["Description"], _FILEINFO_);
    }
  }


  /**
   * If a problem occurred when searching for image overlaps, 
   * this method will handle it. 
   * 
   */
  void ImageOverlapSet::HandleError(SerialNumberList *snlist, iString msg, int overlap1, int overlap2) {
    PvlGroup err("ImageOverlapError");

    if(overlap1 >= 0 && (unsigned)overlap1 < p_lonLatOverlaps.size()) {
      PvlKeyword serialNumbers("PolySerialNumbers");
      PvlKeyword filename("Filenames");

      for (int i=0; i<p_lonLatOverlaps.at(overlap1)->Size(); i++) {
        serialNumbers += (*p_lonLatOverlaps.at(overlap1))[i];

        if(snlist != NULL) {
          filename += snlist->Filename((*p_lonLatOverlaps.at(overlap1))[i]);
        }
      }

      err += serialNumbers;

      if(filename.Size() != 0) {
        err += filename;
      }
    }

    if(overlap2 >= 0 && (unsigned)overlap1 < p_lonLatOverlaps.size() && (unsigned)overlap2 < p_lonLatOverlaps.size()) {
      PvlKeyword serialNumbers("PolySerialNumbers");
      PvlKeyword filename("Filenames");

      for (int i=0; i<p_lonLatOverlaps.at(overlap2)->Size(); i++) {
        serialNumbers += (*p_lonLatOverlaps.at(overlap2))[i];

        if(snlist != NULL) {
          filename += snlist->Filename((*p_lonLatOverlaps.at(overlap2))[i]);
        }
      }

      err += serialNumbers;

      if(filename.Size() != 0) {
        err += filename;
      }
    }

    err += PvlKeyword("Description", msg);

    p_errorLog.push_back(err);

    if (!p_continueAfterError) {
      throw iException::Message(iException::Programmer, err["Description"], _FILEINFO_);
    }
  }
}
