#ifndef MocLabels_h
#define MocLabels_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.4 $                                                             
 * $Date: 2008/06/18 21:52:30 $                                                                 
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

#include <map>
#include "Pvl.h"
#include "Filename.h"

namespace Isis {
  namespace Mgs {
    /**                                              
     * @author 2007-01-30 Author Unknown
     *                                                                        
     * @internal                                                              
     *  @history 2008-04-30  Steven Lambright corrected infinite loop
     *  @history 2008-05-29  Steven Lambright Fixed binary search indexing,
     *          bad calls to std::string::_cstr() references
     *  @history 2008-06-18  Steven Koechle - Fixed Documentation Errors
     */  
    class MocLabels {
      public:
        MocLabels (Isis::Pvl &lab);
        MocLabels (const std::string &file);
        ~MocLabels () {};
        
        inline bool IsNarrowAngle () const { return p_mocNA; };
        inline bool IsWideAngle () const { return !p_mocNA; };
        inline bool IsWideAngleRed () const { return p_mocRedWA; };
        inline bool IsWideAngleBlue () const { return p_mocBlueWA; };
        inline int CrosstrackSumming () const { return p_crosstrackSumming; };
        inline int DowntrackSumming () const { return p_downtrackSumming; };
        inline int FirstLineSample () const { return p_startingSample; };
        inline double FocalPlaneTemperature () const { return p_focalPlaneTemp; };
        inline double LineRate () const { return p_trueLineRate; };
        inline double ExposureDuration() const { return p_exposureDuration; };
        inline std::string StartTime() const { return p_startTime; };    
        inline int Detectors() const { return p_mocNA ? 2048 : 3456; };
        int StartDetector(int sample) const;
        int EndDetector(int sample) const;
        double Sample(int detector) const;
        double EphemerisTime(double line) const;
        double Gain(int line=1);
        double Offset(int line=1);
    
      private:
        void Init (Isis::Pvl &lab);
        void ReadLabels (Isis::Pvl &lab);
        void ValidateLabels ();
        void Compute();
    
        int p_crosstrackSumming;
        int p_downtrackSumming;
        int p_startingSample;
        int p_orbitNumber;
        double p_exposureDuration;
        double p_trueLineRate;
        double p_focalPlaneTemp;
        bool p_mocNA;
        bool p_mocRedWA;
        bool p_mocBlueWA;
        std::string p_instrumentId;
        std::string p_filter;
        std::string p_clockCount;
        std::string p_gainModeId;
        int p_offsetModeId;
        std::string p_startTime;
        std::string p_dataQuality;
        double p_etStart;
        double p_etEnd;
    
        void InitGainMaps();
        std::map<std::string,double> p_gainMapNA;
        std::map<std::string,double> p_gainMapWA;
        double p_gain;
        double p_offset;
    
        int p_nl;
        int p_ns;
        int p_startDetector[3456];
        int p_endDetector[3456];
        double p_sample[3456];
        void InitDetectorMaps();
    
        typedef struct WAGO {
          double et;
          double gain;
          double offset;
          inline bool operator<(const WAGO &w) const { return (et < w.et); };
          inline bool operator==(const WAGO &w) const { return (et == w.et); };
        };
        std::vector<WAGO> p_wagos;
        void InitWago();
    
        Isis::Filename p_lsk;
        Isis::Filename p_sclk;
    };
  };
};

#endif
