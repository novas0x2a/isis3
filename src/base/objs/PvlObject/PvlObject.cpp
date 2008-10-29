/**
 * @file
 * $Revision: 1.8 $
 * $Date: 2008/07/10 15:04:01 $
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

#include "Filename.h"
#include "Pvl.h"
#include "iException.h"
#include "iException.h"
#include "Message.h"
#include "PvlFormat.h"
#include "PvlObject.h"

using namespace std;
namespace Isis {

  //! Creates a blank PvlObject.
  PvlObject::PvlObject() : Isis::PvlContainer("Object") {
  }


  /**
   * Creates a PvlObject with a specified name. 
   *  
   * @param name The name of the PvlObject. 
   */
  PvlObject::PvlObject(const std::string &name) : 
  Isis::PvlContainer("Object",name) {
  }


  /**
   * Finds a group within the current PvlObject. 
   *  
   * @param name The name of the group to look for.
   * @param opts The FindOptions option (None or Traverse). 
   *  
   * @return The PvlGroup object sought for. 
   *  
   * @throws iException::Pvl 
   */
  Isis::PvlGroup &PvlObject::FindGroup(const std::string &name, PvlObject::FindOptions opts) {
    vector<PvlObject *> searchList;
    searchList.push_back(this);

    while (searchList.size() > 0) {
      PvlGroupIterator it = 
      searchList[0]->FindGroup(name,
                               searchList[0]->BeginGroup(),
                               searchList[0]->EndGroup());
      if (it != searchList[0]->EndGroup()) return *it;
      if (opts == Traverse) {
        for (int i=0; i<searchList[0]->Objects(); i++) {
          searchList.push_back(&searchList[0]->Object(i));
        }
      }
      searchList.erase(searchList.begin());
    }

    string msg = "Unable to find group [" + name + "]";
    if (p_filename.size() > 0) msg += " in file [" + p_filename + "]";
    throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
  }


  /**
   * Finds a group within the current PvlObject. 
   *  
   * @param name The name of the group to look for.
   * @param opts The FindOptions option (None or Traverse). 
   *  
   * @return The PvlGroup object sought for. 
   *  
   * @throws iException::Pvl 
   */
  const Isis::PvlGroup &PvlObject::FindGroup(const std::string &name, PvlObject::FindOptions opts) const {
    vector<const PvlObject *> searchList;
    searchList.push_back(this);

    while (searchList.size() > 0) {
      ConstPvlGroupIterator it = 
      searchList[0]->FindGroup(name,
                               searchList[0]->BeginGroup(),
                               searchList[0]->EndGroup());
      if (it != searchList[0]->EndGroup()) return *it;
      if (opts == Traverse) {
        for (int i=0; i<searchList[0]->Objects(); i++) {
          searchList.push_back(new PvlObject(searchList[0]->Object(i)));
        }
      }
      searchList.erase(searchList.begin());
    }

    string msg = "Unable to find group [" + name + "]";
    if (p_filename.size() > 0) msg += " in file [" + p_filename + "]";
    throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
  }


  /**
   * Finds a keyword in the current PvlObject, or deeper inside
   * other PvlObjects and PvlGroups within this one. Note: This
   * member has the same name as the PvlContainer and hides those
   * implementations, but with the using statement the parents
   * FindKeyword members ar made visible. Note: If more than one
   * occurance of a Keyword appears below this Object no
   * guarantee is made as to which one is returned. 
   *  
   * @param kname The name of the keyword to look for. 
   * @param opts The FindOptions option (None or Traverse). 
   *  
   * @return The keyword sought 
   *  
   * @throws iException::Pvl 
   */
  PvlKeyword &PvlObject::FindKeyword (const std::string &kname,
                                      FindOptions opts) {

    // Call the parent's version if they don't want to dig deeper
    if (opts == None) return FindKeyword (kname);

    // Search this PvlObject, and all PvlObjects and PvlContainers within
    // it for the first occurrence of the requested keyword.
    vector<PvlObject *> searchList;
    searchList.push_back(this);

    while (searchList.size() > 0) {
      PvlKeywordIterator it = 
          searchList[0]->FindKeyword(kname, searchList[0]->Begin(),
                                     searchList[0]->End());
      if (it != searchList[0]->End()) {
        return *it;
      }

      // See if the keyword is inside a Group of this Object
      for (int g=0; g<searchList[0]->Groups(); g++) {
        PvlKeywordIterator it = 
            searchList[0]->Group(g).FindKeyword(kname, searchList[0]->Group(g).Begin(),
                                                searchList[0]->Group(g).End());
        if (it != searchList[0]->Group(g).End()) {
          return *it;
        }
      }

      // It's not in this Object or any Groups in this Object, so
      // add all Objects inside this Object to the search list
      for (int i=0; i<searchList[0]->Objects(); i++) {
        searchList.push_back(&searchList[0]->Object(i));
      }

      // This Object has been searched to remove it from the list
      searchList.erase(searchList.begin());
    }

    // No where else to look for the Keyword so throw an error
    string msg = "Unable to find keyword [" + kname + "]";
    if (p_filename.size() > 0) msg += " in file [" + p_filename + "]";
    throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
  }


  /**
   * See if a keyword is in the current PvlObject, or deeper inside
   * other PvlObjects and PvlGroups within this one. Note: This
   * member has the same name as the PvlContainer and hides those
   * implementations, but with the using statement the parents
   * FindKeyword members ar made visible. 
   *  
   * @param kname The name of the keyword to look for.
   * @param opts The FindOptions option (None or Traverse). 
   *  
   * @return True if the Keyword exists False otherwise. 
   */
  bool PvlObject::HasKeyword (const std::string &kname,
                             FindOptions opts) const {

    // Call the parent's version if they don't want to dig deeper
    if (opts == None) return HasKeyword (kname);

    // Search this PvlObject, and all PvlObjects and PvlContainers within
    // it for the first occurrence of the requested keyword.
    vector<const PvlObject *> searchList;
    searchList.push_back(this);

    while (searchList.size() > 0) {
      ConstPvlKeywordIterator it = 
          searchList[0]->FindKeyword(kname, searchList[0]->Begin(),
                                     searchList[0]->End());
      if (it != searchList[0]->End()) {
        return true;
      }

      // See if the keyword is inside a Group of this Object
      for (int g=0; g<searchList[0]->Groups(); g++) {
        ConstPvlKeywordIterator it = 
            searchList[0]->Group(g).FindKeyword(kname, searchList[0]->Group(g).Begin(), 
                                                searchList[0]->Group(g).End());
        if (it != searchList[0]->Group(g).End()) {
          return true;
        }
      }

      // It's not in this Object or any Groups in this Object, so
      // add all Objects inside this Object to the search list
      for (int i=0; i<searchList[0]->Objects(); i++) {
        searchList.push_back(&searchList[0]->Object(i));
      }

      // This Object has been searched to remove it from the list
      searchList.erase(searchList.begin());
    }
    return false;
  }


  /**
   * Find an object within the current PvlObject. 
   *  
   * @param name The object name to look for.
   * @param opts The FindOptions option (None or Traverse). 
   *  
   * @return The PvlObject sought for. 
   *  
   * @throws iException::Pvl 
   */
  PvlObject &PvlObject::FindObject(const std::string &name, 
                                   PvlObject::FindOptions opts) {
    vector<PvlObject *> searchList;
    searchList.push_back(this);

    while (searchList.size() > 0) {
      PvlObjectIterator it = 
      searchList[0]->FindObject(name,
                                searchList[0]->BeginObject(),
                                searchList[0]->EndObject());
      if (it != searchList[0]->EndObject()) return *it;
      if (opts == Traverse) {
        for (int i=0; i<searchList[0]->Objects(); i++) {
          searchList.push_back(&searchList[0]->Object(i));
        }
      }
      searchList.erase(searchList.begin());
    }

    string msg = "Unable to find object [" + name + "]";
    if (p_filename.size() > 0) msg += " in file [" + p_filename + "]";
    throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
  }


  /**
   * Remove an object from the current PvlObject. 
   *  
   * @param name The name of the PvlObject to remove.
   *  
   * @throws iException::Pvl 
   */
  void PvlObject::DeleteObject (const std::string &name) {
    PvlObjectIterator key = FindObject(name,BeginObject(),EndObject());
    if (key == EndObject()) {
      string msg = "Unable to find object [" + name + "] in " + Type() +
                   " [" + Name() + "]";
      if (p_filename.size() > 0) msg += " in file [" + p_filename + "]";
      throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
    }

    p_objects.erase(key);
  }


  /**
   * Remove an object from the current PvlObject. 
   *  
   * @param index The index of the PvlObject to remove.
   *  
   * @throws iException::Pvl 
   */
  void PvlObject::DeleteObject (const int index) {
    if (index >= (int)p_objects.size() || index < 0) {
      string msg = "The specified index is out of bounds in " + Type() +
                   " [" + Name() + "]";
      if (p_filename.size() > 0) msg += " in file [" + p_filename + "]";
      throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
    }

    PvlObjectIterator key = BeginObject();
    for (int i=0; i<index; i++)  key++;

    p_objects.erase(key);
  }


  /**
   * Remove a group from the current PvlObject. 
   *  
   * @param name The name of the PvlGroup to remove.
   *  
   * @throws iException::Pvl 
   */
  void PvlObject::DeleteGroup (const std::string &name) {
    PvlGroupIterator key = FindGroup(name,BeginGroup(),EndGroup());
    if (key == EndGroup()) {
      string msg = "Unable to find group [" + name + "] in " + Type() +
                   " [" + Name() + "]";
      if (p_filename.size() > 0) msg += " in file [" + p_filename + "]";
      throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
    }

    p_groups.erase(key);
  }


  /**
   * Remove a group from the current PvlObject. 
   *  
   * @param index The index of the PvlGroup to remove.
   *  
   * @throws iException::Pvl 
   */
  void PvlObject::DeleteGroup (const int index) {
    if (index >= (int)p_groups.size() || index < 0) {
      string msg = "The specified index is out of bounds in " + Type() +
                   " [" + Name() + "]";
      if (p_filename.size() > 0) msg += " in file [" + p_filename + "]";
      throw Isis::iException::Message(Isis::iException::Pvl,msg,_FILEINFO_);
    }

    PvlGroupIterator key = BeginGroup();
    for (int i=0; i<index; i++)  key++;

    p_groups.erase(key);
  }


  /**
   * Return the group at the specified index. 
   *  
   * @param index The index of the group. 
   *  
   * @return The PvlGroup sought for.
   *  
   * @throws iException::Pvl 
   */
  Isis::PvlGroup &PvlObject::Group(const int index) {
    if (index < 0 || index >= (int)p_groups.size()) {
      string msg = Isis::Message::ArraySubscriptNotInRange (index);
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    return p_groups[index];
  }


  /**
   * Return the group at the specified index. 
   *  
   * @param index The index of the group. 
   *  
   * @return The PvlGroup sought for.
   *  
   * @throws iException::Pvl 
   */
  const Isis::PvlGroup &PvlObject::Group(const int index) const {
    if (index < 0 || index >= (int)p_groups.size()) {
      string msg = Isis::Message::ArraySubscriptNotInRange (index);
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    return p_groups[index];
  }

  /**
   * Return the object at the specified index. 
   *  
   * @param index The index of the object. 
   *  
   * @return The PvlObject sought for.
   *  
   * @throws iException::Programmer 
   */
  PvlObject &PvlObject::Object(const int index) {
    if (index < 0 || index >= (int)p_objects.size()) {
      string msg = Isis::Message::ArraySubscriptNotInRange (index);
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    return p_objects[index];
  }

  /**
   * Return the object at the specified index. 
   *  
   * @param index The index of the object. 
   *  
   * @return The PvlObject sought for.
   *  
   * @throws iException::Programmer 
   */
  const PvlObject &PvlObject::Object(const int index) const {
    if (index < 0 || index >= (int)p_objects.size()) {
      string msg = Isis::Message::ArraySubscriptNotInRange (index);
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    return p_objects[index];
  }

  /** 
   * Creates a PvlObject using a vector of tokens and an iterator. 
   *  
   * @param tokens The vector of tokens to use. 
   *  
   * @param pos The counter to use.
   *  
   * @throws iException::Pvl 
   */
  PvlObject::PvlObject (std::vector<Isis::PvlToken> &tokens, 
                        std::vector<Isis::PvlToken>::iterator &pos) :
  Isis::PvlContainer("Object") {
    // Grab any comments
    p_name.AddComments(tokens,pos);

    // Get the group name
    SetName(pos->GetValue());
    pos++;

    // Loop and parse
    string key;
    vector<Isis::PvlToken>::iterator curPos = pos;
    while (pos != tokens.end()) {
      // Grab the keyword
      key = pos->GetKey();

      // Are we at the objects end
      if (PvlKeyword::StringEqual (key, "ENDOBJECT")) {
        pos++;
        return;
      }

      // See if we have a group
      if (PvlKeyword::StringEqual (key, "GROUP")) {
        Isis::PvlGroup group(tokens,curPos);
        AddGroup(group);
        pos = curPos;
      }

      // See if we have an object
      else if (PvlKeyword::StringEqual (key, "OBJECT")) {
        PvlObject object(tokens,curPos);
        AddObject(object);
        pos = curPos;
      }

      // See if we have a comment otherwise its a keyword
      else if (PvlKeyword::StringEqual (key, "_COMMENT_")) {
        pos++;
      } else {
        Isis::PvlKeyword keyword(tokens,curPos);
        AddKeyword(keyword);
        pos = curPos;
      }
    } 

    // Fell out of the loop so we don't have an endobject
    string message = "Ending delimiter [EndObject] not found for [Object=" 
                     + Name() + "]";
    throw Isis::iException::Message(Isis::iException::Pvl,message,_FILEINFO_);
  } 


  /**
   * Outputs the PvlObject data to a specified output stream. 
   *  
   * @param os The output stream to write to. 
   * @param object The PvlObject to send to the output stream.
   */
  ostream& operator<<(std::ostream &os, PvlObject &object) {

    // Set up a Formatter
    bool removeFormatter = false;
    if (object.GetFormat() == NULL) {
      object.SetFormat(new PvlFormat());
      removeFormatter = true;
    }

    Isis::PvlObject outTemplate("DEFAULT");
    if (object.HasFormatTemplate()) outTemplate = *(Isis::PvlObject*)object.FormatTemplate();

    // Look for and process all include files and remove duplicates
    Isis::PvlObject newTemp(outTemplate.Name());

    // Include files take precedence to all other objects and groups
    for (int i=0; i<outTemplate.Keywords(); i++) {
      if (outTemplate[i].IsNamed("Isis:PvlTemplate:File")) {
        string filename = outTemplate[i];
        Isis::Filename file(filename);
        if (!file.Exists()) {
          string message = "Could not open the following template file: " + filename;
          throw Isis::iException::Message(Isis::iException::Io,message,_FILEINFO_);
        }
        Isis::Pvl include(file.Expanded());

        for (int j=0; j<include.Keywords(); j++) {
          if (!newTemp.HasKeyword(include[j].Name()))
            newTemp.AddKeyword(include[j]);
        }

        for (int j=0; j<include.Objects(); j++) {
          if (!newTemp.HasObject(include.Object(j).Name()))
            newTemp.AddObject(include.Object(j));
        }

        for (int j=0; j<include.Groups(); j++) {
          if (!newTemp.HasGroup(include.Group(j).Name()))
            newTemp.AddGroup(include.Group(j));
        }
      }
      // If it is not an include file keyword add it in place
      else if (!newTemp.HasKeyword(outTemplate[i].Name()))
        newTemp.AddKeyword(outTemplate[i]);
    }

    // Copy over the objects
    for (int i=0; i<outTemplate.Objects(); i++) {
      if (!newTemp.HasObject(outTemplate.Object(i).Name()))
        newTemp.AddObject(outTemplate.Object(i));
    }

    // Copy over the groups
    for (int i=0; i<outTemplate.Groups(); i++) {
      if (!newTemp.HasGroup(outTemplate.Group(i).Name()))
        newTemp.AddGroup(outTemplate.Group(i));
    }

    outTemplate = newTemp;

    // Write out comments for this Objects that were in the template
    if (outTemplate.Comments() > 0) {
      for (int k=0; k<outTemplate.Comments(); k++) {
        for (int l=0; l<object.Indent(); l++) os << " ";
        os << outTemplate.Comment(k) << endl;
      }
      os << endl;
    }

    // Output the object comments and name
    os << object.GetNameKeyword() << endl;
    object.SetIndent(object.Indent()+2);

    // Output the keywords in this Object
    if (object.Keywords() > 0) {
      os << (Isis::PvlContainer &) object << endl;
    }

    // This number keeps track of the number of objects have been written
    int numObjects = 0;

    // Output the Objects within this Object using the format template
    for (int i=0; i<outTemplate.Objects(); i++) {
      for (int j=0; j<object.Objects(); j++) {
        if (outTemplate.Object(i).Name() != object.Object(j).Name()) continue;
        if (j == 0 && object.Keywords() > 0) os << endl;
        object.Object(j).SetIndent(object.Indent());
        object.Object(j).SetFormatTemplate(outTemplate.Object(i));
        object.Object(j).SetFormat(object.GetFormat());
        os << object.Object(j) << endl;
        object.Object(j).SetFormat(NULL);
        object.Object(j).SetIndent(0);
        if (++numObjects < object.Objects()) os << endl;
      }
    }

    // Output the Objects within this Object that were not included in the
    // format template pvl
    for (int i=0; i<object.Objects(); i++) {
      if (outTemplate.HasObject(object.Object(i).Name())) continue;
      if (i == 0 && object.Keywords() > 0) os << endl;
      object.Object(i).SetIndent(object.Indent());
      object.Object(i).SetFormat(object.GetFormat());
      os << object.Object(i) << endl;
      object.Object(i).SetFormat(NULL);
      object.Object(i).SetIndent(0);
      if (++numObjects < object.Objects()) os << endl;
    }

    // This number keeps track of the number of Groups that have been written
    int numGroups = 0;

    // Output the Groups within this Object using the format template
    for (int i=0; i<outTemplate.Groups(); i++) {
      for (int j=0; j<object.Groups(); j++) {
        if (outTemplate.Group(i).Name() != object.Group(j).Name()) continue;
        if ((numGroups == 0) && 
            (object.Objects() > 0 || object.Keywords() > 0)) os << endl;
        object.Group(j).SetIndent(object.Indent());
        object.Group(j).SetFormatTemplate(outTemplate.Group(i));
        object.Group(j).SetFormat(object.GetFormat());
        os << object.Group(j) << endl;
        object.Group(j).SetFormat(NULL);
        object.Group(j).SetIndent(0);
        if (++numGroups < object.Groups()) os << endl;
      }
    }

    // Output the groups that were not in the format template
    for (int i=0; i<object.Groups(); i++) {
      if (outTemplate.HasGroup(object.Group(i).Name())) continue;
      if ((numGroups == 0) && 
          (object.Objects() > 0 || object.Keywords() > 0)) os << endl;
      object.Group(i).SetIndent(object.Indent());
      object.Group(i).SetFormat(object.GetFormat());
      os << object.Group(i) << endl;
      object.Group(i).SetFormat(NULL);
      object.Group(i).SetIndent(0);
      if (++numGroups < object.Groups()) os << endl;
    }

    // Output the end of the object
    object.SetIndent(object.Indent()-2);
    for (int i=0; i<object.Indent(); i++) os << " ";
    os << object.GetFormat()->FormatEnd("End_Object", object.GetNameKeyword());

    if (removeFormatter) {
      delete object.GetFormat();
      object.SetFormat(NULL);
    }

    return os;
  }

} // end namespace isis
