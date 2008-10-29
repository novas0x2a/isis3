/**
 * @file
 * $Revision: 1.9 $
 * $Date: 2008/08/05 21:49:56 $
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

#include "PvlKeyword.h"
#include "iException.h"
#include "Message.h"
#include "iString.h"
#include "PvlSequence.h"
#include "PvlFormat.h"

using namespace std;
namespace Isis {
  //! Constructs a blank PvlKeyword object.
  PvlKeyword::PvlKeyword() {
    Init();
  }

  /**
   * Constructs a PvlKeyword object with a name.
   * @param name The keyword name.
   */
  PvlKeyword::PvlKeyword(const std::string &name) {
    Init();
    SetName(name);
  }

  /**
   * Constructs a PvlKeyword object with a name, value and units. 
   * Defaults to unit="". 
   * @param name The keyword name.
   * @param value The keyword values.
   * @param unit The units the values are given in.
   */
  PvlKeyword::PvlKeyword(const std::string &name, const Isis::iString value, 
                         const std::string unit) {
    Init();
    SetName(name);
    AddValue(value,unit);
  }


  /**
   * Destructs a PvlKeyword object.
   */
  PvlKeyword::~PvlKeyword() {}


  //! Clears all PvlKeyword data.
  void PvlKeyword::Init() {
    Clear();
    ClearComments();
    SetName("");
    p_width = 0;
    p_indent = 0;
    p_formatter = NULL;
  }

  /**
   * Decides whether a value is null or not at a given index. 
   * Defaults to index = 0. 
   * @param index The value index
   * @return <B>bool</B> True if the value is null, false if it's 
   *         not.
   */
  bool PvlKeyword::IsNull(const int index) const {
    if (Size() == 0) return true;
    if (index < 0 || index >= (int)p_values.size()) {
      string msg = Isis::Message::ArraySubscriptNotInRange (index);
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    if (StringEqual("NULL",p_values[index])) return true;
    if (StringEqual("",p_values[index])) return true;
    if (StringEqual("\"\"",p_values[index])) return true;
    if (StringEqual("''",p_values[index])) return true;
    return false; 
  }

  /**
   * Sets the keyword name.
   * @param name The new keyword name.
   */
  void PvlKeyword::SetName(const std::string &name) {
    iString final(name);
    final.Trim("\n\r\t\f\v\b ");
    if (final.find_first_of("\n\r\t\f\v\b ") != std::string::npos) {
      string msg ="[" + name + "] is invalid. Keyword name cannot ";
      msg += "contain whitespace.";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }
    p_name = final;
  }

  /**
   * @brief Sets new values. 
   * If no current value exists, this method sets the given value 
   * to the PvlKeyword.  Otherwise, it clears any existing values 
   * and resets to the value given using AddValue(). Defaults to 
   * unit = "" (empty string). 
   *  
   * @param value New value to be assigned.
   * @param unit Units of measurement corresponding to the value. 
   *  
   * @see AddValue()
   * @see operator=
   * @see operator+=
   */
  void PvlKeyword::SetValue(const Isis::iString value, const std::string unit) {
    Clear();
    AddValue(value,unit);
  }

  /** @brief Sets new values.
   * Overwrites the '=' operator to add a new value using 
   * AddValue(). Like SetValue(), this method clears any 
   * previously existing values and resets to the given value with 
   * unit = "" (empty string). 
   *  
   * @param value The value to be added. 
   * @return <B>PvlKeyword&</B> Reference to PvlKeyword object.
   *  
   * @see AddValue()
   * @see SetValue()
   * @see operator+=
   */
  PvlKeyword& PvlKeyword::operator=(const Isis::iString value) {
    Clear();
    AddValue(value);
    return *this;
  }

  /**
   * @brief Adds a value with units. 
   *  
   * If no current value exists, this method sets the given value. 
   * Otherwise, it retains any current values and adds the value 
   * given to the array of values for this PvlKeyword object. 
   * Defaults to unit = "" (empty string). 
   *  
   * @param value New value to be assigned.
   * @param unit Units of measurement corresponding to the value. 
   *  
   * @see SetValue()
   * @see operator=
   * @see operator+=
   */
  void PvlKeyword::AddValue(const Isis::iString value, const std::string unit) {
    p_values.push_back(value);
    p_units.push_back(unit);
  }

  /** 
   * @brief  Adds a value. 
   * Overwrites the '+=' operators to add a new value. Like 
   * AddValue(), this method keeps any previously existing values 
   * and adds the new value with unit = "" (empty string) to the 
   * array of values for this PvlKeyword object. 
   *  
   * @param value The new value. 
   * @return <B>PvlKeyword&</B> Reference to PvlKeyword object.
   *  
   * @see AddValue()
   * @see SetValue()
   * @see operator=
   */
  PvlKeyword& PvlKeyword::operator+=(const Isis::iString value) {
    AddValue(value);
    return *this;
  }

  //! Clears all values and units for this PvlKeyword object.
  void PvlKeyword::Clear() {
    p_values.clear();
    p_units.clear();
  }

  /** 
   * @brief Gets value for this object at specified index. 
   * Overrides the '[]' operator to return the element in the 
   * array of values at the specified index. 
   * 
   * @param index The index of the value.
   * @return <B>iString</B> The value at the index.
   * @throws iException ArraySubscriptNotInRange (index) Index out of bounds. 
   * @see const operator[] 
   */
  Isis::iString &PvlKeyword::operator[](const int index) {
    if (index < 0 || index >= (int)p_values.size()) {
      string msg = (Isis::Message::ArraySubscriptNotInRange (index)) + 
                    "for Keyword [" + p_name + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    return p_values[index];
  }

  /**
   * @brief Gets value for this object at specified index. 
   * Overrides the '[]' operator to return the element in the 
   * array of values at the specified index. 
   * 
   * @param index The index of the value.
   * @return <b>iString</b> The value at the index.
   * @throws iException ArraySubscriptNotInRange (index) Index out of bounds.
   * @see operator[] 
   */
  const Isis::iString &PvlKeyword::operator[](const int index) const {
    if (index < 0 || index >= (int)p_values.size()) {
      string msg = Isis::Message::ArraySubscriptNotInRange (index);
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    return p_values[index];
  }

  /** 
   * @brief Get units. 
   * Returns the units of measurement of the element of the array 
   * of values for the object at the specified index. Defaults to 
   * index = 0. 
   * @param index The index of the unit.
   * @return <B>string</B> The unit at the index.
   * @throws iException ArraySubscriptNotInRange (index) Index out of bounds.
   */
  string PvlKeyword::Unit(const int index) const {
    if (index < 0 || index >= (int)p_units.size()) {
      string msg = Isis::Message::ArraySubscriptNotInRange (index);
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    return p_units[index];
  }

  /**
   * Add a comment to the PvlKeyword.
   * @param comment The new comment. 
   * @see AddCommentWrapped() 
   * @see AddComments() 
   * @see ClearComments() 
   */
  void PvlKeyword::AddComment(const std::string &comment) {
    if (comment.size() == 0) {
      p_comments.push_back("#");
    }
    if (comment[0] == '#') {
      p_comments.push_back(comment);
    }
    else if (comment.size() == 1) {
      p_comments.push_back("# "+comment);
    }
    else if ((comment[0] == '/') && (comment[1] == '*')) {
      p_comments.push_back(comment);
    }
    else if ((comment[0] == '/') && (comment[1] == '/')) {
      p_comments.push_back(comment);
    }
    else {
      p_comments.push_back("# "+comment);
    }
  }

  /**
   * Automatically wraps and adds long comments to the PvlKeyword
   * @param comment The new comment to add 
   * @see AddComment() 
   * @see AddComments() 
   * @see ClearComments() 
   */
  void PvlKeyword::AddCommentWrapped(const std::string &comment) {
    iString cmt = comment;
    string token = cmt.Token(" ");
    while (cmt != "") {
      string temp = token;
      token = cmt.Token(" ");
      int length = temp.size() + token.size() + 1;
      while ((length < 72) && (token.size() > 0)) {
        temp += " " + token;
        token = cmt.Token(" ");
        length = temp.size() + token.size() + 1;
      }
      AddComment(temp);
    }
    if (token.size() != 0) AddComment(token);
  }

  //! Clears the current comments.
  void PvlKeyword::ClearComments() {
    p_comments.clear();
  }
  
  /**
   * Return a comment at the specified index.
   * @param index The index of the comment.
   * @return <B>string</B> The comment at the index.
   * @throws iException ArraySubscriptNotInRange (index) Index out of bounds.
   */
  string PvlKeyword::Comment (const int index) const { 
    if (index < 0 || index >= (int)p_comments.size()) {
      string msg = Isis::Message::ArraySubscriptNotInRange (index);
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    return p_comments[index]; 
  };
  
  /**
   * Checks if the value needs to be converted to PVL or iPVL and returns it in
   * the correct format.
   * @param value The value to be converted.
   * @return <B>string</B> The value in its proper format (iPVL or
   *         PVL).
   */
  string PvlKeyword::Reform (const std::string &value) const {
  #if 0
    static bool firstTime = true;
    static bool iPVL = true;
    if (firstTime) {
      firstTime = false;
      Isis::PvlGroup &g = Isis::Preference::Preferences().FindGroup("UserInterface",
                                                                    Isis::Pvl::Traverse);
      Isis::iString s = (string) g["PvlFormat"];
      s.UpCase();
      if (s == "PVL") iPVL = false;
    }
  
    if (iPVL) return ToIPvl(value);
  #endif
    return ToPvl(value);
  }

  /**
   * Converts a value to iPVL format.
   * @param value The value to be converted.
   * @return <B>string</B> The value in iPVL format.
   */
  string PvlKeyword::ToIPvl (const std::string &value) const {
    string out;
    bool upcase = true;
    bool lastlower = true;
    for (unsigned int i=0; i<value.size(); i++) {
      if ((lastlower) && (isupper(value[i]))) upcase = true;
      if (value[i] == '_') {
        upcase = true;
      }
      else if (upcase) {
        out += toupper(value[i]);
        lastlower = false;
        upcase = false;
      }
      else {
        out += tolower(value[i]);
        if (islower(value[i])) lastlower = true;
        upcase = false;
      }
    }
    return out;
  }

  /**
   * Converts a value to PVL format.
   * @param value The value to be converted.
   * @return <B>string</B> The value in PVL format.
   */
  string PvlKeyword::ToPvl (const std::string &value) const {
    string out;
    bool lastlower = false;
    for (unsigned int i=0; i<value.size(); i++) {
      if ((lastlower) && (isupper(value[i]))) out += "_";
      if (value[i] == '_') {
        out += "_";
        lastlower = false;
      }
      else {
        out += toupper(value[i]);
        if (islower(value[i])) lastlower = true;
      }
    }
    return out;
  }

  /**
   * Add a comment from a vector, using an interator to keep track.
   * @param tokens The vector of PvlTokens.
   * @param pos The counter (vector index).
   * @see AddComment() 
   * @see AddCommentWrapped() 
   * @see ClearComments() 
   */
  void PvlKeyword::AddComments(std::vector<Isis::PvlToken> &tokens, 
                               std::vector<Isis::PvlToken>::iterator &pos) {
    while (pos != tokens.end() && StringEqual(pos->GetKey(),"_COMMENT_")) {
      AddComment(pos->GetValue());
      pos++;
    }
  }

  /**
   * Construct a PvlKeyword object with a name, values with units, and comments.
   * @param tokens The vector of PvlTokens.
   * @param pos The counter (vector index).
   */
  PvlKeyword::PvlKeyword(std::vector<Isis::PvlToken> &tokens, 
                         std::vector<Isis::PvlToken>::iterator &pos) {
    // Set the comments and keyword name
    Init();
    AddComments(tokens,pos);
    SetName(pos->GetKey());
  
    // Move in the values
    for (int i=0; i<pos->ValueSize(); i++) {
      string value = pos->GetValue(i);
      std::string::size_type p1, p2;
      if (((p1=value.find("<")) != string::npos) &&
          ((p2=value.find(">")) != string::npos)) {
        AddValue(value.substr(0,p1),value.substr(p1+1,p2-p1-1));
      }
      else {
        AddValue(value);
      }
    }
    pos++;
    
    // Take a look at the next keyword to see if it is a unit
    if (pos != tokens.end()) {
      string key = pos->GetKey();
      if (key[0] == '<' && key[key.length()-1] == '>') {
        for (unsigned int i=0; i<p_units.size(); i++) {
          if (p_units[i] != "") {
            string message = "Duplicate units for keyword not allowed [" 
                             + Name() + "]";
            throw Isis::iException::Message(Isis::iException::Pvl,message,_FILEINFO_);
          }
          p_units[i] = key.substr(1,key.size()-2);
        }
        pos++;
      }
    }
  }
  
  /**
   * Checks to see if two strings are equal. Each is converted to uppercase
   * and removed of underscores and whitespaces.
   * @param string1 The first string
   * @param string2 The second string
   * @return <B>bool</B> True or false, depending on whether 
   *          the string values are equal.
   */
  bool PvlKeyword::StringEqual (const std::string &string1,
                                const std::string &string2) {
    Isis::iString s1(string1);
    Isis::iString s2(string2);
  
    s1.ConvertWhiteSpace();
    s2.ConvertWhiteSpace();
  
    s1.Remove(" _");
    s2.Remove(" _");
  
    s1.UpCase();
    s2.UpCase();
  
    if (s1 == s2) return true;
    return false;
  }

  /**
   * Checks to see if a value with a specified index is equivalent to another
   * string.
   * @param string1 The string to compare the value to.
   * @param index The index of the existing value.
   * @return <B>bool</B> True if the two strings are equivalent, 
   *         false if they're not.
   * @throws iException ArraySubscriptNotInRange (index) Index out of bounds.
   */
  bool PvlKeyword::IsEquivalent (const std::string &string1, int index) const {
    if (index < 0 || index >= (int)p_values.size()) {
      string msg = Isis::Message::ArraySubscriptNotInRange (index);
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  
    return StringEqual(p_values[index],string1);
  }

  /**
   * Add values and units from a PvlSequence. (Clears current values and units)
   * @param seq The PvlSequence to add from. 
   * @return <B>PvlKeyword&</B> Reference to PvlKeyword object.
   */
  PvlKeyword& PvlKeyword::operator=(Isis::PvlSequence &seq) {
    this->Clear();
    for (int i=0; i<seq.Size(); i++) {
      string temp = "(";
      for (int j=0; j<(int)seq[i].size(); j++) {
        string val = seq[i][j];
        if (val.find(" ") != std::string::npos) {
          temp += "\"" + val + "\"";
        }
        else {
          temp += val;
        }
        if (j < (int) seq[i].size()-1) temp += ", ";
      }
      temp += ")";
      this->operator+=(temp);
    }
    return *this;
  }

  /**
   * Wraps output so that length doesn't exceed 80. 
   * Used as a helper method for output of PvlKeyword.
   * @param os Designated output stream
   * @param texttowrite The text to be written
   * @param startColumn The starting column after the "=" sign.
   * @param charsLeft Counts the number of characters left. 
   * @return <B>ostream&</B> Reference to ostream.
   * @see operator<< 
   */
  ostream& PvlKeyword::WriteWithWrap(std::ostream &os, const std::string &texttowrite, 
                                     int startColumn, int &charsLeft) const {
    // Position set
    int pos = charsLeft - 1;
    string tempText = texttowrite;

    //If there's less than 1 character left, go ahead and skip to a new line.
    if (charsLeft < 1) {
      os << endl;
      for (int k=0; k < startColumn; ++k) {
        os << " "; 
      }
      charsLeft = 78 - startColumn;
    }

    //If it's quoted...
    if (tempText[0] == '\"') {
      while ((int) tempText.length() > charsLeft) {
        //look for these symbols
        while (pos > 0 && tempText[pos] != ' ' && tempText[pos] != ',') {
          --pos;
        }
        if (pos <= 0) break;

        os << tempText.substr(0, (pos+1));
        os << endl;
        for (int k=0; k < startColumn; ++k) {
          os << " "; 
        }

        tempText = tempText.substr(pos+1, tempText.length()-(pos+1));
        charsLeft = 78 - startColumn;
        pos = charsLeft - 1;
      }
    } 

    // Should we linefeed?
    if ((charsLeft != 78 - startColumn) && 
        ((int)tempText.length() > charsLeft)) {
      os << endl;
      for (int k=0; k < startColumn; ++k) {
        os << " "; 
      }
      charsLeft = 78 - startColumn;
    }

    // Write the remaining text
    os << tempText;
    charsLeft -= tempText.length();

    return os;
  }


  /**
   * 
   * Set the PvlFormatter used to format the keyword name and value(s)
   * 
   * @param formatter A pointer to the formatter to be used
   */
  void PvlKeyword::SetFormat (PvlFormat *formatter) {
    p_formatter = formatter;
  }


  /**
   * 
   * Get the current PvlFormat or create one 
   * @return <B>PvlFormat*</B> Pointer to PvlFormat.
   * 
   */
  PvlFormat* PvlKeyword::GetFormat() {
    return p_formatter;
  };


  /**
   * Write out the keyword. 
   *  
   * @param os The output stream.
   * @param keyword The PvlKeyword object to output. 
   * @return <B>ostream&</B> Reference to ostream.
   * @see WriteWithWrap()
   */
  ostream& operator<<(std::ostream &os, const Isis::PvlKeyword &keyword) {
    // Set up a Formatter
    PvlFormat *tempFormat = keyword.p_formatter;
    bool removeFormatter = false;
    if (tempFormat == NULL) {
      tempFormat = new PvlFormat();
      removeFormatter = true;
    }

    // Write out the comments
    for (int i=0; i<keyword.Comments(); i++) {
      for (int j=0; j<keyword.Indent(); j++) os << " ";
      os << keyword.Comment(i) << endl;
    }

    // Write the keyword name & add length to startColumn.
    int startColumn = 0;
    for (int i=0; i<keyword.Indent(); i++) {
       os << " ";
       ++startColumn;
    }
    string keyname = tempFormat->FormatName(keyword);
    os << keyname;
    startColumn += keyname.length();

    // Add padding and then write equal sign.
    for (int i=0; i<keyword.Width()-(int)keyname.size(); ++i) {
        os << " ";
        ++startColumn;
    }
    os << " = ";
    startColumn += 3;

    // If it has no value then write a NULL 
    if (keyword.Size() == 0) {
      os << tempFormat->FormatValue(keyword);
    }

    // Loop and write each array value
    int charsLeft = 78 - startColumn;
    for (int i=0; i<keyword.Size(); i++) {
      string val = tempFormat->FormatValue(keyword, i);
      keyword.WriteWithWrap(os, val, startColumn, charsLeft);
      if ((i==0) && keyword.Size() > 1) startColumn++;
//cout << " I: " << i;
    }

    if (removeFormatter) delete tempFormat;

    return os;
  }
} 
