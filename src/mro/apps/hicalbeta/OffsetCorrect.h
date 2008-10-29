#if !defined(OffsetCorrect_h)
#define OffsetCorrect_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.1 $
 * $Date: 2008/06/13 22:28:55 $
 * $Id: OffsetCorrect.h,v 1.1 2008/06/13 22:28:55 kbecker Exp $
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
#include <iostream>
#include <sstream>


#include "iString.h"
#include "HiCalTypes.h"
#include "HiCalUtil.h"
#include "HiCalConf.h"
#include "Component.h"
#include "SplineFillComp.h"
#include "LowPassFilterComp.h"
#include "Statistics.h"
#include "SpecialPixel.h"
#include "iException.h"

namespace Isis {

  /**
   * @brief Loads and processes Reverse Clock calibration data
   * 
   * This class loads and processes the Reverse Clock data from a HiRISE image 
   * for offset correction purposes.   Additional processing may occur in 
   * subsequent modules. 
   *  
   * This module implements th Zz module. 
   *  
   * @ingroup Utility
   * 
   * @author 2008-06-13 Kris Becker
   */
  class OffsetCorrect : public Component {

    public: 
      //  Constructors and Destructor
      OffsetCorrect() : Component("OffsetCorrect") { }
      OffsetCorrect(HiCalData &cal, const HiCalConf &conf) : 
                  Component("OffsetCorrect") { 
        init(cal, conf);
      }

      /** Destructor */
      virtual ~OffsetCorrect() { }

      /** 
       * @brief Return statistics for raw Reverse Clock buffer
       * 
       * @return const Statistics&  Statistics class with all stats
       */
      const Statistics &Stats() const { return (_stats); }

    private:
      HiVector   _revClock;
      Statistics _stats;

      void init(HiCalData &cal, const HiCalConf &conf) {
        DbProfile prof = conf.getMatrixProfile();
        _history.clear();
        _history.add("Profile["+ prof.Name()+"]");

        int line0 = ConfKey(prof,"ZzFirstLine",0);
        int lineN = ConfKey(prof,"ZzLastLine",19);
        _revClock = averageLines(cal.getReverseClock(), line0, lineN);
      
        _history.add("AveLines(RevClock["+ToString(line0) + "," +
                     ToString(lineN)+"])");

        SplineFillComp spline(_revClock, _history);
        _data = spline.ref();
        _history = spline.History();

        //  Compute statistics and record to history
        _stats.Reset();
        for ( int i = 0 ; i < _revClock.dim() ; i++ ) {
            _stats.AddData(_revClock[i]);
        }
        _history.add("Statistics(Average["+ToString(_stats.Average())+
                     "],StdDev["+ToString(_stats.StandardDeviation())+"])"); 
        return;
      }

      virtual void printOn(std::ostream &o) const {
        o << "#  History = " << _history << std::endl;
        //  Write out the header
        o << std::setw(_fmtWidth)   << "RevClock"
          << std::setw(_fmtWidth+1) << "Processed\n";

        for (int i = 0 ; i < _data.dim() ; i++) {
          o << formatDbl(_revClock[i]) << " "
            << formatDbl(_data[i]) << std::endl;
        }
        return;
      }

  };

}     // namespace Isis
#endif

