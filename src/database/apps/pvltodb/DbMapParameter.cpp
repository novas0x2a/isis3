/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $
 * $Date: 2007/06/06 00:29:28 $
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

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

#include "DbMapParameter.h"
#include "Pvl.h"
#include "iString.h"
#include "iException.h"

namespace Isis {

bool DbMapKeySpec::getIndex(const DbMapVar &var, const Variables &vref, 
                            int &index) const {
  index = 0;
  iString translator;
  switch (var.type()) {
    case D_Integer:
      translator = var.value();
      index = translator.ToInteger();
      return (true);
    case D_Variable:
      if (vref.exists(var.value())) {
        translator = vref.get(var.value());
        index = translator.ToInteger();
        return (true);
      }
      else {
        _lastError = "Reference variable " + var.value() + " does not exist!";
        return (false);
      }
      break;
    case D_Undefined:
      index = 0;
      return (true);
    default:
      ostringstream mess;
      mess << "Reference variable \"" << var.value() << "\", of type " 
           << var.type() << " is not a valid index - only integers or variable"
              " references are valid indexes";
      throw iException::Message(Isis::iException::Programmer, mess.str(), 
                                _FILEINFO_);
  }
  return (false);
}


bool DbMapKeySpec::findObject(PvlObject &source, const Variables &vars, 
                  KeyListConstIter current, KeyListConstIter end,
                  std::string &value) const {

  // This test for premature search termination
  if (current == end) {
    value = "";
    _lastError = "Search ended prematurely searching for an object!";
    return (false);
  }

  //  Check for object with the specified name
  if (source.HasObject(current->key.value())) {
    int index;
    if (!getIndex(current->index, vars, index)) {
      _lastError = "Index (" + current->index.value() + ") for " + 
                    current->key.value() + " is not valid!";
      value = "";
      return (false);
    }

    if (index == 0) {
      PvlObject &obj = source.FindObject(current->key.value());
      return (findObject(obj, vars, ++current, end, value));
    }
    else {
      //  Search for the nth object of the name
      std::string objName(current->key.value());
      int nth = 0;
      for (int i = 0 ; i < source.Objects() ; i++) {
        PvlObject &obj = source.Object(i);
        if (obj.IsNamed(objName)) {
          if (nth == index) {
            return (findObject(obj,vars,++current,end,value));
          }
          nth++;
        }
      }
    }
  }

  //  If we reach here, an object wasn't found so see if we can find a group
  return (findGroup(source,vars,current,end,value));
}


bool DbMapKeySpec::findGroup(PvlObject &source, const Variables &vars, 
                  KeyListConstIter current, KeyListConstIter end,
                  std::string &value) const {

  // This test for premature search termination  if (current == end) {
  if (current == end) {
    value = "";
    _lastError = "Search ended prematurely searching for an object!";
    return (false);
  }

  //  Check for object with the specified name
  if (source.HasGroup(current->key.value())) {
    int index;
    if (!getIndex(current->index, vars, index)) {
      _lastError = "Index (" + current->index.value() + ") for " + 
                    current->key.value() + " is not valid!";
      value = "";
      return (false);
    }

    if (index == 0) {
      PvlGroup &grp = source.FindGroup(current->key.value());
      return (findKeyword(grp, vars, ++current, end, value));
    }
    else {
      //  Search for the nth object of the name
      std::string grpName(current->key.value());
      int nth = 0;
      for (int i = 0 ; i < source.Groups() ; i++) {
        PvlGroup &grp = source.Group(i);
        if (grp.IsNamed(grpName)) {
          if (nth == index) {
            return (findKeyword(grp,vars,++current,end,value));
          }
          nth++;
        }
      }
    }
  }

  //  If we reach here, an object wasn't found so see if we can find a keyword
  return (findKeyword(source,vars,current,end,value));
}

bool DbMapKeySpec::findKeyword(PvlContainer &source, const Variables &vars, 
                     KeyListConstIter current, KeyListConstIter end,
                     std::string &value) const {
  // This test for premature search termination
  if (current == end) {
    value = "";
    _lastError = "Search ended prematurely searching for an object!";
    return (false);
  }

  //  Check for keyword with the specified name
  if (source.HasKeyword(current->key.value())) {

    PvlKeyword &kw = source.FindKeyword(current->key.value());

    //  Get the index of value
    int index;
    if (!getIndex(current->index, vars, index)) {
      _lastError = "Index (" + current->index.value() + ") for " + 
                    current->key.value() + " is not valid!";
      value = "";
      return (false);
    }

    if (index >= kw.Size()) {
      ostringstream mess;
      mess << "Resulting index (" << index << ") is not valid for a keyword \""
           << kw.Name() << "\" as it has " << kw.Size() << " values!";
      _lastError = mess.str();
      value = "";
      return (false);
    }

    value = kw[index];
    ++current;
  }

  //  If we reach here, either we did not find a keyword or found one and
  //  must check to ensure we don't have any elements left to search.  This
  // call acheives this.
  return (findNothing(source,vars,current,end,value));

}


bool DbMapKeySpec::findNothing(PvlContainer &source, const Variables &vars, 
                  KeyListConstIter current, KeyListConstIter end,
                  std::string &value) const {

  // This test for premature search termination
  if (current != end) {
    value = "";
    _lastError = "Keyword element \"" + current->key.value() + 
                 "\" not found or invalid!";
    return (false);
  }
  return (true);
}

}   //  namespace Isis


