#ifndef LimitPolygonSeeder_h
#define LimitPolygonSeeder_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2008/08/19 22:33:15 $                                                                 
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

#include "geos/geom/Point.h"
#include "geos/geom/MultiPolygon.h"
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/Polygon.h"
#include "PolygonSeeder.h"

namespace Isis {
  class Pvl;

  /**                                                                       
   * @brief Seed points using a grid
   *                                                                        
   * This class is used to construct a grid of points inside a polygon.
   *                                                                        
   * @ingroup PatternMatching
   *                                                                        
   * @author  2008-04-21 Steven Lambright 
   *  
   * @history 2008-08-18 Christopher Austin - Upgraded to geos3.0.0 
   * 
   * @internal
   */                                                                       
  class LimitPolygonSeeder : public PolygonSeeder {
    public:
      LimitPolygonSeeder (Pvl &pvl);

      //! Destructor
      virtual ~LimitPolygonSeeder() {};

      std::vector<geos::geom::Point*> Seed(const geos::geom::MultiPolygon *mp, 
                                           Projection *proj);
      double Spacing();

    protected:
      virtual void Parse(Pvl &pvl);

    private:
      geos::geom::Geometry *GetMultiPolygon(double dMinX, double dMinY, 
                                            double dMaxX, double dMaxY, 
                                            const geos::geom::MultiPolygon &orig);
      double p_minThickness; //<! area / max(Xextent,Yextent)**2
      double p_minArea; //<! Units are meters
      int p_majorAxisPts; //<! Number of points to place on major axis
      int p_minorAxisPts; //<! Number of points to place on minor axis
  };
};

#endif
