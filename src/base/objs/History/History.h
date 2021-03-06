/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2006/12/11 16:50:04 $                                                                 
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
#if !defined(History_h)
#define History_h

#include "Blob.h"

namespace Isis {
/**
 * 
 * @history 2006-12-11 Kris Becker Fixed bug in WriteInit method using a
 *                                 temporary string to reference a char pointer
 *                                 to its contents.  The string remain after the
 *                                 reference or its toast.
 * 
 * @internal
 * @todo This class needs documentation.
*/
  class History : public Isis::Blob {
    public:
      History(const std::string &name);
      History(const std::string &name, const std::string &file);
      ~History();

      void AddEntry();
      void AddEntry(Isis::PvlObject &obj);
      Pvl ReturnHist();
      void Read (Isis::Pvl &pvl, std::fstream &is);

    protected:
      void WriteInit ();

    private:
      Pvl p_history;
  };
};

#endif

