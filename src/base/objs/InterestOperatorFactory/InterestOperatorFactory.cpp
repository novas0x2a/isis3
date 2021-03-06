/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2009/01/16 16:25:09 $                                                                 
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

#include "InterestOperatorFactory.h"
#include "InterestOperator.h"
#include "Plugin.h"
#include "iException.h"
#include "Filename.h"

namespace Isis {
  /** 
   * Create an InterestOperator object using a PVL specification.
   * An example of the PVL required for this is:
   * 
   * @code
   * Object = InterestOperator
   *   Group = Operator
   *     Name      = StandardDeviation
   *     Samples   = 21
   *     Lines     = 21
   *     Delta     = 50
   *   EndGroup
   * EndObject
   * @endcode
   * 
   * There are many other options that can be set via the pvl and are
   * described in other documentation (see below).
   * 
   * @param pvl The pvl object containing the specification
   * 
   * @see automaticRegistration.doc
   **/
  InterestOperator *InterestOperatorFactory::Create(Pvl &pvl) {
    // Get the algorithm name to create
    PvlGroup &op = pvl.FindGroup("Operator",Pvl::Traverse);
    std::string operatorName = op["Name"];

    // Open the factory plugin file
    Plugin p;
    Filename f("InterestOperator.plugin");
    if (f.Exists()) {
      p.Read("InterestOperator.plugin");
    }
    else {
      p.Read("$ISISROOT/lib/InterestOperator.plugin");
    }

    // Get the algorithm specific plugin and return it
    InterestOperator * (*plugin) (Pvl &pvl);
    plugin = (InterestOperator * (*)(Pvl &pvl)) p.GetPlugin(operatorName);
    return (*plugin)(pvl);
  }
} // end namespace isis
