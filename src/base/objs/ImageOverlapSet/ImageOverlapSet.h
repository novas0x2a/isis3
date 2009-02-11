#ifndef ImageOverlapSet_h
#define ImageOverlapSet_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.14 $                                                             
 * $Date: 2009/01/28 16:29:15 $
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
 *   $ISISROOT/doc/documents/Disclaimers/Disclaimers.html                
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include <vector>
#include <string>

#include "geos/geom/MultiPolygon.h"
#include "geos/geom/LinearRing.h"
#include "geos/util/GEOSException.h"

#include "ImageOverlap.h"
#include "iException.h"
#include "PvlGroup.h"

namespace Isis {

  // Forward declarations
  class SerialNumberList;

  /**
   * This class is used to find the overlaps between all the images in a list of
   * serial numbers. The overlaps are created in (Lon,Lat) coordinates of
   * geos::geom::MultiPolygons. Each overlap has an associated list of serial numbers
   * which are contained in that overlap.
   * 
   * @ingroup PatternMatching
   *
   * @author 2006-01-20 Stuart Sides 
   *  
   * @internal 
   *  @history 2008-06-18 Christopher Austin - Fixed documentation
   *  @history 2008-08-18 Steven Lambright - Updated to work with geos3.0.0
   *           instead of geos2. Mostly namespace changes.
   *  @history 2008-11-24 Steven Lambright - Improved upon error reporting. Added
   *           the Errors() method.
   *  @history 2008-11-25 Steven Koechle - Moved Despike Methods from
   *           ImageOverlapSet to PolygonTools
   *  @history 2008-12-10 Steven Koechle - Moved MakeMultiPolygon Method from
   *           ImageOverlapSet to PolygonTools
   *  @history 2008-12-05 Steven Lambright - Checking footprints for validity now,
   *           fixed an issue with the intersection operator where an invalid but
   *           repairable polygon is produced, and fixed a memory leak.
   *  @history 2008-12-15 Steven Koechle - Fixed to read new footprint blob naming
   *           scheme.
   *  @history 2009-01-06 Steven Koechle - Removed backwards compatibility for old
   *           footprint blob name. Added a throw if footprints are invalid in an
   *           image.
   *  @history 2009-01-07 Steven Lambright & Christopher Austin - Fixed handling
   *           of Despike(...) throwing errors on empty polygons
   *  @history 2009-01-13 Steven Lambright - Deletes both overlaps if an
   *           intersection fails for an unknown reason.
   *  @history 2009-01-28 Steven Lambright - Fixed memory leaks
   */
  class ImageOverlapSet {
    public:
      ImageOverlapSet (bool continueOnError = false);
      virtual ~ImageOverlapSet();

      void FindImageOverlaps(SerialNumberList &boundaries);
      void FindImageOverlaps(std::vector<std::string> sns,
                             std::vector<geos::geom::MultiPolygon*> polygons);

      void ReadImageOverlaps(const std::string &filename);
      void WriteImageOverlaps(const std::string &filename);

      /**
       * Returns the total number of latitude and longitude overlaps
       * 
       * @return int The number of lat/lon overlaps
       */
      int Size() { return p_lonLatOverlaps.size(); }

      /**
       * Returns the images which overlap at a given loverlap
       * 
       * @param index The index of the overlap
       * 
       * @return const ImageOverlap* The polygon and serial numbers which define the 
       *         indexed overlap.
       */
      const ImageOverlap* operator[](int index) {return p_lonLatOverlaps[index];};

      std::vector<ImageOverlap*> operator[](std::string sn);

      const std::vector<PvlGroup> &Errors() { return p_errorLog; } 

    protected:
      void FindAllOverlaps (SerialNumberList *snlist = NULL);
      void AddSerialNumbers (ImageOverlap *to, ImageOverlap *from);

      std::vector<PvlGroup> p_errorLog;

    private:
      std::vector<ImageOverlap *> p_lonLatOverlaps; //!< The list of lat/lon overlaps

      ImageOverlap* CreateNewOverlap (std::string serialNumber,
                                      geos::geom::MultiPolygon* lonLatPolygon);

      void HandleError(iException &e, SerialNumberList *snlist, iString msg = "", int overlap1 = -1, int overlap2 = -1);
      void HandleError(geos::util::GEOSException *exc, SerialNumberList *snlist, iString msg = "", int overlap1 = -1, int overlap2 = -1);
      void HandleError(SerialNumberList *snlist, iString msg, int overlap1 = -1, int overlap2 = -1);

      bool p_continueAfterError;
  };
};

#endif
