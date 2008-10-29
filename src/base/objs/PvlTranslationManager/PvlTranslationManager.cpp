/**
 * @file
 * $Revision: 1.8 $
 * $Date: 2008/07/10 17:57:36 $
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

#include "iString.h"
#include "Message.h"
#include "iException.h"
#include "PvlTranslationManager.h"

using namespace std;
namespace Isis {

  PvlTranslationManager::PvlTranslationManager (const std::string &transFile) {
    AddTable (transFile);
  }

  /** 
   * Constructs and initializes a TranslationManager object
   * 
   * @param inputLabel The Pvl holding the input label.
   * 
   * @param transFile The translation file to be used to tranlate keywords in  
   *                  the input label.
   */
   PvlTranslationManager::PvlTranslationManager (Isis::Pvl &inputLabel,
                                                  const std::string &transFile) {
    p_fLabel = inputLabel;

    // Internalize the translation table
    AddTable (transFile);
  }
  
 /** 
  * Constructs and initializes a TranslationManager object
  * 
  * @param inputLabel The Pvl holding the input label.
  * 
  * @param transStrm A stream containing the tranlation table to be used to  
  *                  tranlate keywords in the input label.
  */
  PvlTranslationManager::PvlTranslationManager (Isis::Pvl &inputLabel,
                                                std::istream &transStrm) {
    p_fLabel = inputLabel;
  
    // Internalize the translation table
    AddTable (transStrm);
  }
  
   /** 
    * Returns a translated value. The output name is used to find the input  
    * group, keyword, default and tranlations in the translation table. If the 
    * keyword does not exist in the input label, the input default if 
    * available will be used as the input value. This input value
    * is then used to search all of the translations. If a match is
    * found the translated value is returned.
    *
    * @param nName The output name used to identify the input keyword to be 
    *              translated.
    * 
    * @param findex The index into the input keyword array.  Defaults to 0
    * 
    * @return string
    */
    string PvlTranslationManager::Translate (const std::string nName, int findex) {
    Isis::PvlContainer *con;
    int inst = 0;
    string grp;
    while ((grp = InputGroup(nName, inst++)) != "") {
      if ((con = GetContainer(grp)) != NULL) {
    //while ((con = GetContainer(InputGroup(nName, inst++))) != NULL) {
//    if ((con = GetContainer (InputGroup(nName))) != NULL) {
        if (con->HasKeyword(InputKeyword(nName))) {
          return Isis::PvlTranslationTable::Translate(nName,
                 (*con)[InputKeyword(nName)][findex]);
        }
      }
    }
  
    return Isis::PvlTranslationTable::Translate(nName, "");
  }
  
 /** 
  * Translate the requested output name to output values using the input name  
  * and values or default value
  * 
  * @param nName The output name used to identify the input keyword to be 
  *              translated.
  * 
  * @param dummy  
  * 
  * @return Isis::PvlKeyword
  */
  Isis::PvlKeyword PvlTranslationManager::Translate (const std::string nName, double dummy) {
    Isis::PvlContainer *con = NULL;
    Isis::PvlKeyword key;

    int inst = 0;
    string grp;
    while ((grp = InputGroup(nName, inst++)) != "") {
      if ((con = GetContainer(grp)) != NULL) {
        //while ((con = GetContainer(InputGroup(nName, inst++))) != NULL) {
        //if ((con = GetContainer (InputGroup(nName))) != NULL) {
        // If the label contains the correct keyword, translate all values
        if (con->HasKeyword(InputKeyword(nName))) {
          //if (InputHasKeyword(nName)) {
          key.SetName(OutputName(nName));
          for (int v=0; v<(*con)[(InputKeyword(nName))].Size(); v++) {
            //key.AddValue(Isis::PvlTranslationTable::Translate(nName, InputValue(nName, v)),
            //             InputUnits(nName, v));
            key.AddValue(Isis::PvlTranslationTable::Translate(nName, (*con)[InputKeyword(nName)][v]),
                         (*con)[InputKeyword(nName)].Unit(v));
          }
          return key;
        }
      }
    }
    return Isis::PvlKeyword (OutputName(nName),
                           Isis::PvlTranslationTable::Translate(nName, ""));
  }
  
  
  
  // Automatically translate all the output names found in the translation table
  // If a output name does not translate an error will be thrown by one
  // of the support members
  // Store the translated key, value pairs in the argument pvl
  void PvlTranslationManager::Auto (Isis::Pvl &outputLabel) {
    // Attempt to translate every group in the translation table
   for (int i=0; i<TranslationTable().Groups(); i++) {
      Isis::PvlGroup &g = TranslationTable().Group(i);
      if (IsAuto(g.Name())) {
        try {
          Isis::PvlContainer *con = CreateContainer(g.Name(), outputLabel);
          (*con) += Translate (g.Name(),  0.5);
        }
        catch (iException &e){
          if (IsOptional(g.Name())) {
            e.Clear();
          }
          else {
//            e.Report();
            throw e;
          }
        }
      }
    }
  }
  
 /** 
  * Returns the ith input value assiciated with the output name argument. 
  * 
  * @param nName The output name used to identify the input keyword.
  * 
  * @param findex The index into the input keyword array.  Defaults to 0
  * 
  * @throws Isis::iException::Programmer
  */
  string PvlTranslationManager::InputValue (const std::string nName,
                                            const int findex) {
    Isis::PvlContainer *con;
    int inst = 0;
    //while ((con = GetContainer(InputGroup(nName, inst++))) != NULL) {
    //if ((con = GetContainer (InputGroup(nName))) != NULL) {

    string grp;
    bool grpFound = false;
    while ((grp = InputGroup(nName, inst++)) != "") {
      if ((con = GetContainer(grp)) != NULL) {

        if (con->HasKeyword(InputKeyword(nName))) {
          return (*con)[InputKeyword(nName)][findex];
        }
        grpFound = true;
      }
    }

    if(grpFound) {
      string msg = "Unable to find input keyword [" + InputKeyword(nName) +
                   "] for output name [" + nName + "] in file [" + TranslationTable().Filename() +"]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg, _FILEINFO_);
    }
    else {
      string msg = "Unable to find input group [" + InputGroup(nName) +
                   "] for output name [" + nName + "] in file [" + TranslationTable().Filename() +"]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg, _FILEINFO_);
    }
  }
  
 /** 
  * Returns the ith input units assiciated with the output name argument.  
  * 
  * @param nName The output name used to identify the input keyword.
  * 
  * @param findex The index into the input keyword array.  Defaults to 0
  * 
  * @throws Isis::iException::Programmer
  */
  string PvlTranslationManager::InputUnits (const std::string nName,
                                              const int findex) {
    Isis::PvlContainer *con;
    int inst = 0;
    //while ((con = GetContainer(InputGroup(nName, inst++))) != NULL) {
    //if ((con = GetContainer (InputGroup(nName))) != NULL) {

    string grp;
    bool grpFound = false;
    while ((grp = InputGroup(nName, inst++)) != "") {
      if ((con = GetContainer(grp)) != NULL) {

        if (con->HasKeyword(InputKeyword(nName))) {
          return (*con)[InputKeyword(nName)].Unit(findex);
        }
        grpFound = true;

      }
    }
    if(grpFound) {
      string msg = "Unable to find input keyword [" + InputKeyword(nName) +
                   "] for output name [" + nName + "] in file [" + TranslationTable().Filename() +"]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg, _FILEINFO_);
    }
    else {
      string msg = "Unable to find input group [" + InputGroup(nName) +
                   "] for output name [" + nName + "] in file [" + TranslationTable().Filename() +"]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg, _FILEINFO_);
    }  
  }
  
 /** 
  * Returns the number of values in foriegn keyword assiciated with the output 
  * name argument.
  * 
  * @param nName The output name used to identify the input keyword.
  * 
  * @throws Isis::iException::Programmer
  */
  int PvlTranslationManager::InputSize (const std::string nName) { 
    Isis::PvlContainer *con;
    int inst = 0;
    //while ((con = GetContainer(InputGroup(nName, inst++))) != NULL) {
    //if ((con = GetContainer (InputGroup(nName))) != NULL) {

    string grp;
    bool grpFound = false;
    while ((grp = InputGroup(nName, inst++)) != "") {
      if ((con = GetContainer(grp)) != NULL) {
        if (con->HasKeyword(InputKeyword(nName))) {
          return (*con)[(InputKeyword(nName))].Size();
        }
        grpFound = true;
      }
    }
    if(grpFound) {
      string msg = "Unable to find input keyword [" + InputKeyword(nName) +
                   "] for output name [" + nName + "] in file [" + TranslationTable().Filename() +"]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg, _FILEINFO_);
    }
    else {
      string msg = "Unable to find input group [" + InputGroup(nName) +
                   "] for output name [" + nName + "] in file [" + TranslationTable().Filename() +"]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg, _FILEINFO_);
    } 
  }
  
 /** 
  * Indicates if the input keyword corresponding to the output name exists in 
  * the label
  *  
  * @param nName The output name used to identify the input keyword.    
  */
  bool PvlTranslationManager::InputHasKeyword (const std::string nName) {
  
    // Set the current position in the input label pvl
    // by finding the input group corresponding to the output group
    Isis::PvlContainer *con;
    int inst = 0;
    //while ((con = GetContainer(InputGroup(nName, inst++))) != NULL) {
    //if ((con = GetContainer (InputGroup(nName))) != NULL) {

    string grp;
    while ((grp = InputGroup(nName, inst++)) != "") {
      if ((con = GetContainer(grp)) != NULL) {  
        if (con->HasKeyword(InputKeyword(nName))) return true;
      }
    }
    return false;
  }
  
 /* 
  * Indicates if the input group corresponding to the output name exists in
  * the label 
  * 
  * @param nName The output name used to identify the input keyword. 
  
  bool PvlTranslationManager::InputHasGroup (const std::string nName) {
  
    if (GetContainer (InputGroup(nName)) != NULL) {
      return true;
    }
  
    return false;
  }
*/  
  
  // Return a container from the input label according tund
  Isis::PvlContainer *PvlTranslationManager::GetContainer(const std::string &fGroup) {
    Isis::iString groups(fGroup);
    Isis::iString g;
    Isis::PvlObject *obj = &p_fLabel;

    // Return the root container if "ROOT" is the ONLY thing in the list
    if (groups.UpCase() == "ROOT") {
      return (Isis::PvlContainer *) obj;
    }

    while ((g = groups.Token(",").Trim(" \n\t\v\b\f\r")).length() != 0) {
      if (obj->HasObject(g)) {
        obj = &obj->FindObject(g);
        continue;
      }
      else if (obj->HasGroup(g)) {
        return (Isis::PvlContainer *) &obj->FindGroup(g);
      }
      else {
        return NULL;
      }
    }
  
    return (Isis::PvlContainer *) obj;
  }
  
  
  // Create the requsted container and any containers above it and
  // return a reference to the container
  // list is an Isis::PvlKeyword with an array of container types an their names
  Isis::PvlContainer *PvlTranslationManager::CreateContainer(const std::string nName,
                                                            Isis::Pvl &pvl) {
    
    // Get the array of Objects/Groups from the OutputName keyword
    Isis::PvlKeyword np = OutputPosition(nName);
  
    Isis::PvlObject *obj = &pvl;
  
    // Look at every pair in the output position
    for (int c=0; c<np.Size(); c+=2) {
      // If this pair is an object
      if (np[c].UpCase() == "OBJECT") {
        // If the object doesn't exist create it
        if (!obj->HasObject(np[c+1])) {
          obj->AddObject (np[c+1]);
        }
        obj = &(obj->FindObject(np[c+1]));
      }
      // If this pair is a group
      else if (np[c].UpCase() == "GROUP") {
        // If the group doesn't exist create it
        if (!obj->HasGroup (np[c+1])) {
          obj->AddGroup (np[c+1]);
        }
        return (Isis::PvlContainer *) &(obj->FindGroup(np[c+1]));
        
      }
    }
  
    return (Isis::PvlContainer *) obj;
  }
} // end namespace isis
