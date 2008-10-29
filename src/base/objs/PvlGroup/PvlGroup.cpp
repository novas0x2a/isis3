/**                                                                       
 * @file                                                                  
 * $Revision: 1.3 $                                                             
 * $Date: 2008/07/10 15:00:45 $                                                                 
 *                                                                        
 *   Unless noted otherwise, the portions of Isis written by the USGS are public domain. See
 *   individual third-party library and package descriptions for intellectual property information,
 *   user agreements, and related information.                            
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or implied, is made by the
 *   USGS as to the accuracy and functioning of such software and related material nor shall the
 *   fact of distribution constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.                                        
 *                                                                        
 *   For additional information, launch $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include "iException.h"
#include "PvlFormat.h"
#include "PvlGroup.h"

using namespace std;
namespace Isis {
  //! Creates a blank PvlGroup object.
  PvlGroup::PvlGroup() : Isis::PvlContainer("Group", "") {}

  /**
   * Creates a PvlGroup object with a name.
   * @param name The group name.
   */
  PvlGroup::PvlGroup(const std::string &name) : 
    Isis::PvlContainer("Group",name) {
  }


  /**
   * Creates a PvlGroup object using a vector of tokens and an iterator.
   * @param tokens The vector of PvlTokens to use.
   * @param pos The iterator to use.
   * @throws iException::Message
   */
  PvlGroup::PvlGroup (std::vector<Isis::PvlToken> &tokens, 
                      std::vector<Isis::PvlToken>::iterator &pos) :
                Isis::PvlContainer("Group") {
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
  
      // Are we at the groups end
      if (PvlKeyword::StringEqual (key, "ENDGROUP")) {
        pos++;
        return;
      }
  
      // Don't allow groups inside of groups
      if (PvlKeyword::StringEqual (key, "GROUP")) {
        string message = "Nested groups are not allowed [Group=" + 
                         pos->GetValue() +
                         "] is inside [Group=" + Name() + "]";
        throw Isis::iException::Message(Isis::iException::Pvl,message,_FILEINFO_);
      }
  
      // Don't allow objects inside of groups
      if (PvlKeyword::StringEqual (key, "OBJECT")) {
        string message = "Objects can not be nested in groups [Group=" + 
                         pos->GetValue() +
                         "] is inside [Group=" + Name() + "]";
        throw Isis::iException::Message(Isis::iException::Pvl,message,_FILEINFO_);
      }
  
      // Don't allow reserved keyword endobject inside of a group
      if (PvlKeyword::StringEqual (key, "ENDOBJECT")) {
        string message = "Reserved word [EndObject] found inside [Group=" 
                         + Name() + "]";
        throw Isis::iException::Message(Isis::iException::Pvl,message,_FILEINFO_);
      }
  
      // See if we have a comment
      if (PvlKeyword::StringEqual (key, "_COMMENT_")) {
        pos++;
      }
      else {
        Isis::PvlKeyword keyword(tokens,curPos);
        AddKeyword(keyword);
        pos = curPos;
      }
    } 
  
    // Fell out of the loop so we don't have an endgroup
    string message = "Ending delimiter [EndGroup] not found for [Group=" 
                     + Name() + "]";
    throw Isis::iException::Message(Isis::iException::Pvl,message,_FILEINFO_);
  }


  /**
   * Outputs the PvlGroup data to a specified output stream.
   * @param os The output stream to output to.
   * @param group The PvlGroup object to output.
   */
  ostream& operator<<(std::ostream &os, PvlGroup &group) {

    // Set up a Formatter
    bool removeFormatter = false;
    if (group.GetFormat() == NULL) {
      group.SetFormat(new PvlFormat());
      removeFormatter = true;
    }

    Isis::PvlGroup temp("DEFAULT");
    if (group.HasFormatTemplate()) temp = *(Isis::PvlGroup*)group.FormatTemplate();

    // Output comment from the template
    if (temp.Comments() > 0) {
      for (int k=0; k<temp.Comments(); k++) {
        for (int l=0; l<group.Indent(); l++) os << " ";
        os << temp.Comment(k) << endl;
      }
//      os << endl;
    }

    // Output the group comments and name
    os << group.GetNameKeyword() << endl;
    group.SetIndent(group.Indent()+2);

    // Output the keywords in this group
    if (group.Keywords() > 0) {
      os << (Isis::PvlContainer &) group << endl;
    }

    // Output the end of the group
    group.SetIndent(group.Indent()-2);
    for (int i=0; i<group.Indent(); i++) os << " ";
    os << group.GetFormat()->FormatEnd("End_Group", group.GetNameKeyword());

    if (removeFormatter) {
      delete group.GetFormat();
      group.SetFormat(NULL);
    }

    return os;
  }
} // end namespace isis
