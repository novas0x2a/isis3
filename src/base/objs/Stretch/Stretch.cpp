/**
 * @file
 * $Revision: 1.7 $
 * $Date: 2009/04/29 17:00:34 $
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

#include <iostream>
#include "Stretch.h"
#include "iString.h"
#include "SpecialPixel.h"
#include "iException.h"
#include "Pvl.h"

using namespace std;
namespace Isis {

 /**
  * Constructs a Stretch object with default mapping of special pixel values to
  * themselves.
  */
  Stretch::Stretch () {
    p_null = Isis::NULL8;
    p_lis = Isis::LOW_INSTR_SAT8;
    p_lrs = Isis::LOW_REPR_SAT8;
    p_his = Isis::HIGH_INSTR_SAT8;
    p_hrs = Isis::HIGH_REPR_SAT8;
    p_minimum = p_lrs;
    p_maximum = p_hrs;
    p_pairs = 0;
  }

 /**
  * Adds a stretch pair to the list of pairs. Note that all input pairs must be
  * in ascending order.
  *
  * @param input Input value to map
  *
  * @param output Output value when the input is mapped
  *
  * @throws Isis::iException::Programmer - input pairs must be in descending
  *                                        order
  */
  void Stretch::AddPair (const double input, const double output) {
    if (p_pairs > 0) {
      if (input <= p_input[p_pairs-1]) {
        string msg = "Input pairs must be in ascending order";
        throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
      }
    }

    p_input.push_back(input);
    p_output.push_back(output);
    p_pairs++;
  }

 /**
  * Sets the mapping for NULL pixels. If not called the NULL pixels will be
  * mapped to NULL. Otherwise you can map NULLs to any double value.
  * For example, SetNull(0.0).
  *
  * @param value Value to map input NULLs
  */
  void Stretch::SetNull (const double value) {
    p_null = value;
  }

 /**
  * Sets the mapping for LIS pixels. If not called the LIS pixels will be mapped
  * to LIS. Otherwise you can map LIS to any double value. For example,
  * SetLis(0.0).
  *
  * @param value Value to map input LIS
  */
  void Stretch::SetLis (const double value) {
    p_lis = value;
  }

 /**
  * Sets the mapping for LRS pixels. If not called the LRS pixels will be mapped
  * to LRS. Otherwise you can map LRS to any double value. For example,
  * SetLrs(0.0).
  *
  * @param value Value to map input LRS
  */
  void Stretch::SetLrs (const double value) {
    p_lrs = value;
  }

 /**
  * Sets the mapping for HIS pixels. If not called the HIS pixels will be mapped
  * to HIS. Otherwise you can map HIS to any double value. For example,
  * SetHis(255.0).
  *
  * @param value Value to map input HIS
  */
  void Stretch::SetHis (const double value) {
    p_his = value;
  }

 /**
  * Sets the mapping for HRS pixels. If not called the HRS pixels will be mapped
  * to HRS. Otherwise you can map HRS to any double value. For example,
  * SetHrs(255.0).
  *
  * @param value Value to map input HRS
  */
  void Stretch::SetHrs (const double value) {
    p_hrs = value;
  }

  void Stretch::SetMinimum (const double value) {
    p_minimum = value;
  }

  void Stretch::SetMaximum (const double value) {
    p_maximum = value;
  }

 /**
  * Maps an input value to an output value based on the stretch pairs and/or
  * special pixel mappings.
  *
  * @param value Value to map
  *
  * @return double The mapped output value is returned by this method
  */
  double Stretch::Map (const double value) const {
    // Check special pixels first
    if (!Isis::IsValidPixel(value)) {
      if (Isis::IsNullPixel(value)) return p_null;
      if (Isis::IsHisPixel(value)) return p_his;
      if (Isis::IsHrsPixel(value)) return p_hrs;
      if (Isis::IsLisPixel(value)) return p_lis;
      return p_lrs;
    }

    // Check to see if we have any pairs
    if (p_input.size() == 0) return value;

    // Check to see if outside the minimum and maximum next
    if (value < p_input[0]) {
      if (!Isis::IsValidPixel(p_minimum)) {
        if (Isis::IsNullPixel(p_minimum)) return p_null;
        if (Isis::IsHisPixel(p_minimum)) return p_his;
        if (Isis::IsHrsPixel(p_minimum)) return p_hrs;
        if (Isis::IsLisPixel(p_minimum)) return p_lis;
        return p_lrs;
      }
      return p_minimum;
    }

    if (value > p_input[p_pairs-1]) {
      if (!Isis::IsValidPixel(p_maximum)) {
        if (Isis::IsNullPixel(p_maximum)) return p_null;
        if (Isis::IsHisPixel(p_maximum)) return p_his;
        if (Isis::IsHrsPixel(p_maximum)) return p_hrs;
        if (Isis::IsLisPixel(p_maximum)) return p_lis;
        return p_lrs;
      }
      return p_maximum;
    }

    // Check the end points
    if (value == p_input[0]) return p_output[0];
    if (value == p_input[p_pairs-1]) return p_output[p_pairs-1];

    // Ok find the surrounding pairs with a binary search
    int start = 0;
    int end = p_pairs-1;
    while (start != end) {
      int middle = (start + end) / 2;

      if(middle == start) {
        end = middle;
      }
      else if(value < p_input[middle]) {
        end = middle;
      }
      else {
        start = middle;
      }
    }

    end = start+1;

    // Apply the stretch
    double slope = (p_output[end] - p_output[start]) / (p_input[end] - p_input[start]);
    return slope * (value - p_input[end]) + p_output[end];
  }

 /**
  * Parses a string of the form "i1:o1 i2:o2...iN:oN" where each i:o
  * represents an input:output pair. Therefore, the user can enter a string in
  * this form and this method will parse the string and load the stretch pairs
  * into the object via AddPairs.
  *
  * @param pairs A string containing stretch pairs for example
  *              "0:0 50:0 100:255 255:255"
  *
  * @throws Isis::iException::User - invalid stretch pair
  */
  void Stretch::Parse (const std::string &pairs) {
    // Zero out the stretch arrays
    p_input.clear();
    p_output.clear();
    p_pairs = 0;

    // Create some working strings for parsing
    Isis::iString p(pairs);
    Isis::iString temp;
    double input,output;

    // Trim leading whitespace and then loop extracting pairs from
    // a string of the form "i1:o1 i2:o2...iN:oN"
    p.TrimHead(" \t\r\n\v\f");
    while (p.length() > 0) {
      // Get the input value before the :
      temp = p.Token(":");
      // Remove leading and trailing whitespace
      temp.Trim(" \t\r\n\v\f");
      // Convert to a double or fail
      try {
        input = temp.ToDouble();
      }
      catch (Isis::iException &e) {
        throw Isis::iException::Message(Isis::iException::User,"Invalid stretch pairs ["+pairs+"]",_FILEINFO_);
      }

      // Trim leading blanks
      p.TrimHead(" \t\r\n\v\f");
      // Make sure there is something to convert
      if (p.length() == 0) {
        throw Isis::iException::Message(Isis::iException::User,"Invalid stretch pairs ["+pairs+"]",_FILEINFO_);
      }
      // Get the output value before the next set of whitespace
      temp = p.Token(" \t\r\n\v\f");
      // Convert to a double or fail
      try {
        output = temp.ToDouble();
      }
      catch (Isis::iException &e) {
        throw Isis::iException::Message(Isis::iException::User,"Invalid stretch pairs ["+pairs+"]",_FILEINFO_);
      }

      // Push them on the stretch pair stack
      try {
        Stretch::AddPair(input,output);
      }
      catch (Isis::iException &e) {
        throw Isis::iException::Message(Isis::iException::User,"Invalid stretch pairs ["+pairs+"]",_FILEINFO_);
      }

      // Trim leading whitespace and loop for the next pair
      p.TrimHead(" \t\r\n\v\f");
    }
  }

  /**
   * Converts stretch pair to a string);
   *
   * @return string The stretch pair as a string
   */
  string Stretch::Text() const {

    if (p_pairs < 0) return "";

    Isis::iString p("");
    for (int i=0; i<p_pairs; i++) {
      p += Isis::iString(p_input[i]) + ":" + Isis::iString(p_output[i]) + " ";
    }
    return p.TrimTail (" ");
  }

 /**
  * Returns the value of the input side of the stretch pair at the specified
  * index. If the index number is larger than the number of stretch pairs, the
  * method returns -1
  *
  * @param index The index number to retrieve the input stretch pair value from
  *
  * @return double The input side of the stretch pair at the specified index
  */
  double Stretch::Input (int index) const {
    if (index+1 > p_pairs) return -1;
    return p_input[index];
  }

 /**
  * Returns the value of the output side of the stretch pair at the specified
  * index. If the index number is larger than the number of stretch pairs, the
  * method returns -1
  *
  * @param index The index number to retieve the output stretch pair value from
  *
  * @return double The output side of the stretch pair at the specified index
  */
  double Stretch::Output (int index) const {
    if (index+1 > p_pairs) return -1;
    return p_output[index];
  }

  /**
   * Loads the stretch pairs from the pvl file into the Stretch
   * object.  The file should look similar to this:
   * @code
   * Group = Pairs
   *   Input = (0,100,255)
   *   Output = (255,100,0)
   * EndGroup
   * @endcode
   * 
   * @param file - The input file containing the stretch pairs
   * @param grpName - The group name to get the input and output
   *                keywords from
   */
  void Stretch::Load(std::string &file, std::string &grpName) {
    Pvl pvl(file);
    Load(pvl,grpName);
  }

  /**
   * Loads the stretch pairs from the pvl file into the Stretch
   * object.  The pvl should look similar to this:
   * @code
   * Group = Pairs
   *   Input = (0,100,255)
   *   Output = (255,100,0)
   * EndGroup
   * @endcode
   * 
   * @param pvl - The pvl containing the stretch pairs
   * @param grpName - The group name to get the input and output
   *                keywords from
   */
  void Stretch::Load(Isis::Pvl &pvl, std::string &grpName) {
    PvlGroup grp = pvl.FindGroup(grpName,Isis::PvlObject::Traverse);
    PvlKeyword inputs = grp.FindKeyword("Input");
    PvlKeyword outputs = grp.FindKeyword("Output");

    if (inputs.Size() != outputs.Size()) {
      std::string msg = "Invalid Pvl file: The number of Input values must equal the number of Output values";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }
    for (int i=0; i<inputs.Size(); i++) {
      AddPair(inputs[i],outputs[i]);
    }
  }


  /**
   * Saves the stretch pairs in the Stretch object into the given
   * pvl file
   * 
   * @param file - The file that the stretch pairs will be written
   *             to
   * @param grpName - The name of the group to create and put the
   *                stretch pairs into.  The group will contain
   *                two keywords, Input, and Output.
   */
  void Stretch::Save(std::string &file, std::string &grpName) {
    Pvl p;
    Save(p,grpName);
    p.Write(file);
  }

  void Stretch::Save(Isis::Pvl &pvl, std::string &grpName) {
    PvlGroup *grp = new PvlGroup(grpName);
    PvlKeyword inputs("Input");
    PvlKeyword outputs("Output");
    for (int i=0; i<Pairs(); i++) {
      inputs.AddValue(Input(i));
      outputs.AddValue(Output(i));
    }
    grp->AddKeyword(inputs);
    grp->AddKeyword(outputs);
    pvl.AddGroup(*grp);
  }

  /**
   * Copies the stretch pairs from another Stretch object, but maintains special 
   * pixel values 
   * 
   * @param other - The Stretch to copy pairs from
   */
  void Stretch::CopyPairs(const Stretch &other) {
    this->p_pairs = other.p_pairs;
    this->p_input = other.p_input;
    this->p_output = other.p_output;
  }

} // end namespace isis


