#ifndef IntersectionStatistics_h
#define IntersectionStatistics_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2009/06/05 21:17:53 $                                                                 
 *                                                             
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for 
 *   intellectual property information,user agreements, and related information.
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software 
 *   and related material nor shall the fact of distribution constitute any such 
 *   warranty, and no responsibility is assumed by the USGS in connection 
 *   therewith.                                                           
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see 
 *   the Privacy &amp; Disclaimers page on the Isis website,              
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */

#include "Statistics.h"
#include "BasisFunction.h"
#include "LeastSquares.h"
#include "PvlObject.h"
#include "Pvl.h"

namespace Isis {

/**
 * @brief
 * @internal
 *   @history 2009-06-05 Mackenzie Boyd Fixed unittest to work on all systems
 */

  class IntersectionStatistics {
    public:
      IntersectionStatistics (std::vector<Statistics> statsList,
                              std::vector<std::string> idList);

      virtual ~IntersectionStatistics() {};
  
      enum AddStatus {
        Success,        
        NoOverlap,
        BadWeight,
        NoContrast
      };

      AddStatus AddIntersection (const Statistics &area1, const std::string id1,
                    const Statistics &area2, const std::string id2,
                    double weight=1.0);
  
      void SetHoldList (std::vector<std::string> idHoldList);
  
      enum SolutionType {
        Gain,
        Offset,
        Both
      };
  
      bool Solve (SolutionType type=Both);
  
      double GetGain (const std::string id) const;
      double GetOffset (const std::string id) const;

      inline PvlObject GetPvls () { return p_equ; };
  
    private:
      std::vector<Statistics> p_statsList;
      std::vector<std::string> p_idList;
      std::vector<std::string> p_idHoldList;
  
      class Intersection {
        public:
          Statistics area1,area2;
          std::string id1,id2;
      };
  
      std::vector<Intersection> intersectionList;
      std::vector<double> p_deltas;
      std::vector<double> p_weights;
  
      bool p_solved;
      SolutionType p_solutionType;
  
      std::vector<double> p_gains;   // Filled by Solve method
      std::vector<double> p_offsets; // Filled by Solve method
  
      Isis::BasisFunction *p_gainFunction;
      Isis::BasisFunction *p_offsetFunction;
  
      Isis::LeastSquares *p_gainLsq;
      Isis::LeastSquares *p_offsetLsq;

      Isis::PvlObject p_equ;
  };
};

#endif
