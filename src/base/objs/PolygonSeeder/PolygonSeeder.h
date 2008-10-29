#ifndef PolygonSeeder_h
#define PolygonSeeder_h
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

#include <string>
#include <vector>

#include "geos/geom/Point.h"
#include "geos/geom/MultiPolygon.h"

#include "Projection.h"

namespace Isis {
  class Pvl;
  class PolygonTools;

  /**
   * This class is used as the base class for all PolygonSeeder objects. The
   * class is pure virtual.
   * 
   * @ingroup PatternMatching
   *
   * @author 2006-01-20 Stuart Sides 
   *  
   * @history 2008-08-18 Christopher Austin - Upgraded to geos3.0.0, removed 
   *          Chip.h include, fixed ifndef
   */
  class PolygonSeeder {
    public:
      PolygonSeeder(Pvl &pvl);
      virtual ~PolygonSeeder();

      virtual std::vector<geos::geom::Point*> Seed(const geos::geom::MultiPolygon *mp,
                                                   Projection *proj) = 0;

      std::string Name();
      double MinimumThickness();
      double MinimumArea();

    protected:
      virtual void Parse(Pvl &pvl);
      std::string StandardTests(const geos::geom::MultiPolygon *xymp,
                                const geos::geom::Envelope *xyBoundBox);

    private:
      std::string p_algorithmName;
      double p_minimumThickness;
      double p_minimumArea;

  };
};

#endif
