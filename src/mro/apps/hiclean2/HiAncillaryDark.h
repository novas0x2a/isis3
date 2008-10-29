#ifndef HiAncillaryDark_h
#define HiAncillaryDark_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2008/05/12 18:17:33 $
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
#include "Blobber.h"

namespace Isis {

/**                                                   
 * @brief HiRISE Ancillary Dark Pixels class
 *  
 * This class provides access and processing of HiRISE ancillary dark pixels
 * object data as stored in an ISIS BLOB (Table).  This data is used in the
 * radiometric calibration/reduction processing.
 * 
 * Note that the file provided must be a HiRISE ISIS cube file that is
 * freshly converted from a PDS compatable EDR (hi2isis).  It must contain
 * an ISIS Table named "HiRISE Ancillary".  From that table, data is extracted
 * from the "DarkPixels" field.
 * 
 * @ingroup MarsReconnaissanceOrbiter                                      
 *                                                    
 * @author 2005-10-03 Kris Becker                    
 * @see hi2isis
 * 
 * @internal
 *  @history 2008-05-12 Steven Lambright - Removed references to CubeInfo
 */
  class HiAncillaryDark : public Blobber {

    public:
      /**
       * @brief Default, mostly useless constructor
       */
      HiAncillaryDark() : Blobber() { }

      /**
       * @brief Constructor providing interface to an ISIS Cube object
       */
      HiAncillaryDark(Cube &cube) : 
        Blobber(cube,"HiRISE Ancillary","DarkPixels","AncillaryDark") { }
      /** Destructor */
      virtual ~HiAncillaryDark() { }
   };
};

#endif

