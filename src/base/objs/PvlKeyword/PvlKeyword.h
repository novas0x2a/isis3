#ifndef PvlKeyword_h
#define PvlKeyword_h
/**
 * @file
 * $Revision: 1.8 $
 * $Date: 2008/10/01 01:12:19 $
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

#include <vector>
#include <map>
#include <iostream>
#include "iString.h"
#include "PvlToken.h"
#include "Constants.h"

namespace Isis {
/**
 * @brief A single keyword-value pair.
 * 
 * This class is used to create a single PVL keyword-value pair. PvlContainer
 * can combine PvlKeyword objects and organize them so they look clean on 
 * output.
 * 
 * @ingroup Parsing                                                  
 *                                                                        
 * @author 2002-10-11 Jeff Anderson
 *                                                                        
 * @internal
 *  @history 2005-04-08 Leah Dahmer - wrote class documentation.
 *  @history 2005-04-08 Leah Dahmer - added the WriteWithWrap() method so 
 *                                    keyword values will now be wrapped when 
 *                                    the length exceeds 80 characters.
 *  @history 2005-05-18 Jeff Anderson - Fixed minor problems with wrapping code
 *  @history 2006-04-05 Elizabeth Miller - Added AddCommentWrapped method
 *  @history 2006-09-05 Stuart Sides - Added ability to format keywords in
 *                                     different ways using the PvlFormat class
 *  @history 2007-08-20 Brendan George - Added checking to ensure Keyword Name
 *                                       contains no whitespace
 *  @history 2008-02-08 Christopher Austin - Altered
 *           WriteWithWrap to not bomb when 2 or more single
 *           statement lines in a row are over 78 characters
 *           long.
 *  @history 2008-07-03 Steven Lambright - Added const functionality
 *  @history 2008-07-10 Steven Lambright - StringEqual is now static, all
 *           AddComments methods are public
 *  @history 2008-09-30 Christopher Austin - replaced all std::endl in the <<
 *           operator as well as WriteWithWrap() with PvlFormat.FormatEOL(), and
 *           formatted wraps accordingly
 */                                                                       
  class PvlSequence;
  class PvlFormat;

  class PvlKeyword {
    friend std::ostream& operator<<(std::ostream &os, const PvlKeyword &keyword);

    public:
      PvlKeyword();
      PvlKeyword(const std::string &name);
      PvlKeyword(const std::string &name, const Isis::iString value, 
                     const std::string unit="");
      PvlKeyword(std::vector<Isis::PvlToken> &token, 
                     std::vector<Isis::PvlToken>::iterator &pos);
      ~PvlKeyword();
  
      void SetName(const std::string &name);

      /**
       * Returns the keyword name.
       * @return The name of the keyword.
       */
      std::string Name() const { return p_name; };
      /**
       * Determines whether two PvlKeywords have the same name or not.
       * @param name The name of the keyword to compare with this one.
       * @return True if the names are equal, false if not.
       */
      bool IsNamed(const std::string &name) const { return StringEqual(name,Name()); };
      
      void SetValue(const Isis::iString value, const std::string unit="");
      PvlKeyword& operator=(const Isis::iString value);
  
      void AddValue(const Isis::iString value, const std::string unit="");
      PvlKeyword& operator+=(const Isis::iString value);
      
      int Size() const { return p_values.size(); };
      bool IsNull(const int index = 0) const;
      void Clear();
  
      operator double() const { return (double) operator[](0); };
      operator int() const { return (int) operator[](0); };
      operator Isis::BigInt() const { return (Isis::BigInt) operator[](0); };
      operator std::string() const { return (std::string) operator[](0); };

      const Isis::iString &operator[](const int index) const;
      Isis::iString &operator[](const int index);
      std::string Unit(const int index = 0) const;
      
      void AddComment(const std::string &comment);
      void AddCommentWrapped(const std::string &comment);

      void AddComments(std::vector<Isis::PvlToken> &token, 
                       std::vector<Isis::PvlToken>::iterator &pos);

      inline int Comments () const { return p_comments.size(); };
      std::string Comment (const int index) const;
      void ClearComments();
  
      bool operator==(const PvlKeyword &key) const {
        return (StringEqual(p_name,key.p_name));
      };
  
      bool IsEquivalent (const std::string &string1, int index=0) const;
  
      void SetWidth(int width) { p_width = width; };
      void SetIndent(int indent) { p_indent = indent; };
      int Width () const { return p_width; };
      int Indent () const { return p_indent; };

      PvlKeyword& operator=(Isis::PvlSequence &seq);

      void SetFormat (PvlFormat *formatter);
      PvlFormat* GetFormat();

      static bool StringEqual (const std::string &string1,
                        const std::string &string2);

    protected:
      std::string Reform(const std::string &value) const;
      std::string ToPvl(const std::string &value) const;
      std::string ToIPvl(const std::string &value) const;
      std::ostream& WriteWithWrap(std::ostream &os, const std::string &texttowrite, 
                                  int startColumn, int &charsLeft,
                                  PvlFormat *tempFormat) const;

      PvlFormat *p_formatter;

    private:
      //! The keyword name
      std::string p_name;
      //! A vector of values for the keyword.
      std::vector<Isis::iString> p_values;
      //! The units for the values.
      std::vector<std::string> p_units;
      //! The comments for the keyword.
      std::vector<std::string> p_comments;

      void Init();

      

      /**
       * The width of the longest keyword. This is used for spacing out the
       * equals signs on output.
       */
      int p_width;
      /**
       * The number of indentations to make. This is based on whether the 
       * keyword is in a group, etc.
       */
      int p_indent;
  };
};

#endif

