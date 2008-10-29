#if !defined(Resolver_h)
#define Resolver_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.3 $
 * $Date: 2008/07/11 22:28:54 $
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
#include <iostream> 
#include <iomanip> 

#include "iString.h"
#include "DbPvlMapUtil.h"
#include "MapFunctor.h"
#include "DbMapParameter.h"
#include "Parser.hpp"


namespace Isis {

using namespace boost::spirit;
using namespace phoenix;

struct Resolver : public DbResource {
  Resolver(Pvl &source, Variables &vars, FunctionList &funcs) : 
           DbResource( source, vars, funcs), _value() { 
    init();
  }
  ~Resolver() { }

  bool resolve(const std::string &mapkey) {
    try {
      DbMapParameter *parm = 0;
      parameter parms(functions_p);
//    std::cout << "\nParsing <" << mapkey << ">\n";
      parse_info<> info = parse(mapkey.c_str(), parms[var(parm) = arg1]); 
      if (info.full) {
//      std::cout << "Successful!\n";
        if (parm->Evaluate(*this, _value)) {
          delete parm;
          return (true);
        }
      }

      // Returned but parsing failed
//    std::cout << "Failed!\n";
      delete  parm;
      _value.clear();
      return (false);
    } 
    catch (iException &ie) {
      std::string mess = "Error encountered parsing: " + mapkey;
      ie.Message(iException::User,mess.c_str(), _FILEINFO_);
    }
    return (false);
  }

  bool isValid() const { return (!_value.empty()); }
  std::string value() const { return (_value); }

  private:
//  Function symbol table used to validate parsing of these
//  occurances in the DM map file.
    boost::spirit::symbols<MapFunctor *> functions_p;

    std::string _value;

    void init() {
      for (int i = 0 ; i < _functors.size() ; i++) {
        MapFunctor *f = _functors.getNth(i);
        string fName(iString::DownCase(f->name()));
        functions_p.add(fName.c_str(), f);
      }
    }
};

}  // namespace Isis

#endif
