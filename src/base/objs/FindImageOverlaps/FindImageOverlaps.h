#ifndef FindImageOverlaps_h
#define FindImageOverlaps_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.3 $                                                             
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
 *   $ISISROOT/doc/documents/Disclaimers/Disclaimers.html                
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include <vector>
#include <string>

#include "ImageOverlap.h"
#include "geos/geom/MultiPolygon.h"
#include "geos/geom/LinearRing.h"

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
   */
  class FindImageOverlaps {
    public:
      FindImageOverlaps ();
      FindImageOverlaps (SerialNumberList &boundaries);
      FindImageOverlaps (std::vector<std::string> sns,
                         std::vector<geos::geom::MultiPolygon*> polygons);
      virtual ~FindImageOverlaps();

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

      geos::geom::MultiPolygon* Despike (geos::geom::MultiPolygon *multiPoly);
      geos::geom::LinearRing* Despike (const geos::geom::LineString *linearRing);

      std::vector<ImageOverlap*> operator[](std::string sn);

  protected:

      void FindAllOverlaps ();
      void AddSerialNumbers (ImageOverlap *to, ImageOverlap *from);
      geos::geom::MultiPolygon* MakeMultiPolygon (geos::geom::Geometry *geom);

    private:
      std::vector<ImageOverlap *> p_lonLatOverlaps; //!< The list of lat/lon overlaps

      ImageOverlap* CreateNewOverlap (std::string serialNumber,
                                      geos::geom::MultiPolygon* lonLatPolygon);
  };
};

#endif
