/**
 * @file
 * $Revision: 1.11 $
 * $Date: 2008/07/11 22:18:20 $
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

#include <fstream>
#include <sstream>

#include "iString.h"
#include "iException.h"
#include "iException.h"
#include "Message.h"
#include "Pvl.h"
#include "PvlTranslationTable.h"

using namespace std;
namespace Isis {

 /** 
  * Constructs and initializes a PvlTranslationTable object 
  * 
  * @param transFile The translation file to be used 
  * 
  * @throws Isis::iException::Io
  */
  PvlTranslationTable::PvlTranslationTable (Isis::Filename transFile) {
  
    ifstream inStm;
    string transFilename(transFile.Expanded());
    inStm.open(transFilename.c_str(),std::ios::in);
    if (!inStm) {
      string message = Isis::Message::FileOpen(transFilename.c_str());
      throw Isis::iException::Message(Isis::iException::Io,message,_FILEINFO_);
    }

    inStm >> p_trnsTbl;
    inStm.close();
  }
  
 /** 
  * Constructs and initializes a PvlTranslationTable object  
  * 
  * @param istr The translation stream to be used to translate values 
  */
  PvlTranslationTable::PvlTranslationTable (std::istream &istr) {
    istr >> p_trnsTbl;
  }
  
  //! Construct an empty PvlTranslationTable
  PvlTranslationTable::PvlTranslationTable () {
  }
  
  /** 
   * Adds the contents of a translation table to the searchable groups/keys
   * 
   * @param transFile The name of the translation file to be added.
   * 
   * @throws Isis::iException::Io
   */
   void PvlTranslationTable::AddTable (const std::string &transFile) {
  
    Isis::Filename temp(transFile);
  
    ifstream transStm;
    string tempName(temp.Expanded());
    transStm.open(tempName.c_str(),std::ios::in);
    if (!transStm) {
      string message = Isis::Message::FileOpen(tempName.c_str());
      throw Isis::iException::Message(Isis::iException::Io,message,_FILEINFO_);
    }
  
    AddTable (transStm);
    transStm.close();
  }
  
 /** 
  * Adds the contents of a translation table to the searchable groups/keys
  * Also performs a verification, to ensure that the translation table
  * is valid
  * 
  * @param transStm The stream to be added.
  */
  void PvlTranslationTable::AddTable (std::istream &transStm) {
    transStm >> p_trnsTbl;

// The following was commented out for now. Restore these tests when the
// translation files have been cleaned of "Nativexxx" and "Foreignxxx" keywords.
//    for (int i=0; i<p_trnsTbl->Groups(); i++) {
//      PvlGroup currGrp = p_trnsTbl->Group(i);
//      if (!currGrp.HasKeyword("Translation")) {
//        string message = "Unable to find Translation for group ["
//                         + currGrp.Name() + "] in file [" +
//                         p_trnsTbl->Filename();
//        throw
//        Isis::iException::Message(Isis::iException::User,message,_FILEINFO_);
//      }
//      if (!currGrp.HasKeyword("InputKey")) {
//        string message = "Unable to find InputKey for group ["
//                         + currGrp.Name() + "] in file [" +
//                         p_trnsTbl->Filename() + "]";
//        throw
//        Isis::iException::Message(Isis::iException::User,message,_FILEINFO_);
//      }
//      for (int j=0; j<currGrp.Keywords(); j++) {
//        PvlKeyword currKey = currGrp[j];
//        if (currKey.Name() != "Translation" && currKey.Name() != "OutputName"
//        &&
//            currKey.Name() != "InputGroup" && currKey.Name() !=
//            "OutputPosition" &&
//            currKey.Name() != "Auto" && currKey.Name() != "Optional" &&
//            currKey.Name() != "InputKey" && currKey.Name() != "InputDefault")
//            {
//          string message = "Keyword [" + currKey.Name() +"] is not a valid
//          keyword." +
//                           " Error in file [" + p_trnsTbl->Filename() +"]" ;
//          throw
//          Isis::iException::Message(Isis::iException::User,message,_FILEINFO_);
//        }
//      }
//    }
  }
  
 /** 
  * Translates the output name and input value.
  * 
  * @param nName The output name to be used to search the translation table.
  * 
  * @param fValue The input value to be translated
  * 
  * @return string The translated string
  *
  * @throws Isis::iException::Programmer
  */
  string PvlTranslationTable::Translate (const std::string nName,
                                         const std::string fValue) const {
  
    if (!p_trnsTbl.HasGroup(nName)) {
      string msg = "Unable to find translation group [" + nName + "] in file [" + p_trnsTbl.Filename() + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    Isis::PvlGroup tgrp = p_trnsTbl.FindGroup(nName);
  
    // If no input value was passed in search using the input default
    string tmpFValue = fValue;
    if (tmpFValue.length() == 0) {
      if (tgrp.HasKeyword("InputDefault")) {
        tmpFValue = (string) tgrp["InputDefault"];
      }
      else {
        string msg = "No value or default value to translate for translation group [" +
                     nName + "] in file [" + p_trnsTbl.Filename() + "]";
        throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
      }
    }
  
    // Search the Translation keywords for a match to the input value
    Isis::Pvl::PvlKeywordIterator it = tgrp.Begin();
    //  while ((it = tgrp.FindKeyword("Translation",it,tgrp.End()) != tgrp.End()) {
    while (it != tgrp.End()) {
      it = tgrp.FindKeyword("Translation",it,tgrp.End());
      if (it == tgrp.End()) break; 
      Isis::PvlKeyword &key = *it;
      if ((string) key[1] == tmpFValue) {
        return key[0];
      }
      else if ((string) key[1] == "*") {
        if ((string) key[0] == "*") {
          return tmpFValue;
        }
        else {
          return key[0];
        }
      }
      it++;
    }
  
    string msg = "Unable to find a translation value for [" +
                   nName +  ", " + Isis::iString(fValue) + "] in file [" + p_trnsTbl.Filename() + "]";
    throw Isis::iException::Message(Isis::iException::Programmer,msg, _FILEINFO_);
  }
  
   /** 
    * Returns the input group name from the translation table corresponding to 
    * the output name argument.
    *
    * @param nName The output name to be used to search the translation table.
    * @param inst The occurence number of the "InputGroup" keyword
    *             (first one is zero)
    * 
    * @return string The input group name
    * 
    * @throws Isis::iException::Programmer
    */
    string PvlTranslationTable::InputGroup (const std::string nName, const int inst) const {
  
    if (!p_trnsTbl.HasGroup(nName)) {
      string msg = "Unable to find translation group [" + nName + "] in file [" + p_trnsTbl.Filename() + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  
    Isis::PvlGroup tgrp = p_trnsTbl.FindGroup(nName);

    // If the translation group has the InputGroup keyword, then return the ith occurrence of it
    if (tgrp.HasKeyword("InputGroup")) {
      Isis::Pvl::PvlKeywordIterator it = tgrp.Begin() + inst;
      while (it != tgrp.End()) {
        it = tgrp.FindKeyword("InputGroup",it,tgrp.End());
        if (it == tgrp.End()) break;
        return (*it)[0]; 
      }
    }
    // Default to the ROOT container if no containers were listed
    else if (inst == 0) {
      return p_trnsTbl.Name();
    }

    //if (tgrp.HasKeyword("InputGroup")) return tgrp["InputGroup"];
  
    return "";
  }
  
 /** 
  * Returns the input keyword name from the translation table corresponding to 
  * the output name argument.
  *
  * @param nName The output name to be used to search the translation table.
  * 
  * @return string The input keyword name
  * 
  * @throws Isis::iException::Programmer
  */
  string PvlTranslationTable::InputKeyword (const std::string nName) const {
  
    if (!p_trnsTbl.HasGroup(nName)) {
      string msg = "Unable to find translation group [" + nName + "] in file [" + p_trnsTbl.Filename() + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  
    Isis::PvlGroup tgrp = p_trnsTbl.FindGroup(nName);
    if (tgrp.HasKeyword("InputKey")) return tgrp["InputKey"];
      
    return "";
  }
  
 /** 
  * Returns the input default value from the translation table corresponding 
  * to the output name argument.
  * 
  * @param nName The output name to be used to search the translation table.
  * 
  * @return string The input default value
  *
  * @throws Isis::iException::Programmer
  */
  string PvlTranslationTable::InputDefault (const std::string nName) const {
  
    if (!p_trnsTbl.HasGroup(nName)) {
      string msg = "Unable to find translation group [" + nName + "] in file [" + p_trnsTbl.Filename() + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  
    Isis::PvlGroup tgrp = p_trnsTbl.FindGroup(nName);
    if (tgrp.HasKeyword("InputDefault")) return tgrp["InputDefault"];
  
    return "";
  }
  
  bool PvlTranslationTable::IsAuto (const std::string nName) {
    if (!p_trnsTbl.HasGroup(nName)) {
      string msg = "Unable to find translation group [" + nName + "] in file [" + p_trnsTbl.Filename() + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  
    Isis::PvlGroup &tgrp = p_trnsTbl.FindGroup(nName);
    if (tgrp.HasKeyword("Auto")) return true;
  
    return false;
  }

  bool PvlTranslationTable::IsOptional (const std::string nName) {
    if (!p_trnsTbl.HasGroup(nName)) {
      string msg = "Unable to find translation group [" + nName + "] in file [" + p_trnsTbl.Filename() + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    Isis::PvlGroup &tgrp = p_trnsTbl.FindGroup(nName);
    if (tgrp.HasKeyword("Optional")) return true;

    return false;
  }
  
  Isis::PvlKeyword &PvlTranslationTable::OutputPosition (const std::string nName) {
    if (!p_trnsTbl.HasGroup(nName)) {
      string msg = "Unable to find translation group [" + nName + "] in file [" + p_trnsTbl.Filename() + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  
    Isis::PvlGroup &tgrp = p_trnsTbl.FindGroup(nName);
    if (!tgrp.HasKeyword("OutputPosition")) {
      string msg = "Unable to find translation keyword [OutputPostion] in [" + 
                   nName + "] in file [" + p_trnsTbl.Filename() + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
  
    }

    return tgrp["OutputPosition"];
  }
  
  
  string PvlTranslationTable::OutputName (const std::string nName) {
    if (!p_trnsTbl.HasGroup(nName)) {
      string msg = "Unable to find translation group [" + nName + "] in file [" + p_trnsTbl.Filename() + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  
    Isis::PvlGroup tgrp = p_trnsTbl.FindGroup(nName);
    if (tgrp.HasKeyword("OutputName")) {
      return tgrp["OutputName"];
    }
  
    return "";
  }
} // end namespace isis

