#ifndef GridPolygonSeeder_h
#define GridPolygonSeeder_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.7 $                                                             
 * $Date: 2008/12/06 00:39:34 $                                                                 
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
   * @author  2006-01-20 Stuart Sides
   * 
   * @internal
   * @history 2007-05-09 Tracie Sucharski,  Changed a single spacing value
   *                            to a separate value for x and y.
   * @history 2008-02-29 Steven Lambright - Created SubGrid capabilities,
   *                            cleaned up Seed methods
   * @history 2008-06-18 Christopher Austin - Fixed documentation errors 
   * @history 2008-08-18 Christopher Austin - Upgraded to geos3.0.0
   * @history 2008-11-25 Steven Lambright - Added error checking 
   * @history 2008-12-05 Christopher Austin - capped the subgrid to 127x127 to 
   *          prevent segfaults on too high a precision
   */
  class GridPolygonSeeder : public PolygonSeeder {
    public:
      GridPolygonSeeder (Pvl &pvl);

      //! Destructor
      virtual ~GridPolygonSeeder() {};

      std::vector<geos::geom::Point*> Seed(const geos::geom::MultiPolygon *mp, 
                                     Projection *proj);

      const bool SubGrid() { return p_subGrid; }

    protected:
      virtual void Parse(Pvl &pvl);

    private:
      std::vector<geos::geom::Point*> SeedGrid(const geos::geom::MultiPolygon *mp, 
                                           Projection *proj);
      std::vector<geos::geom::Point*> SeedSubGrid(const geos::geom::MultiPolygon *mp, 
                                           Projection *proj);

      geos::geom::Point *CheckSubGrid(const geos::geom::MultiPolygon &, const double &,
                                      const double &, const int &);

      double p_Xspacing;
      double p_Yspacing;
      double p_minThickness; // area / max(Xextent,Yextent)**2
      double p_minArea; // Units are meters
      bool   p_subGrid;
  };
};

#endif
