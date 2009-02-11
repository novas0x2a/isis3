#include <sstream>
#include "PvlTranslationTable.h"
#include "Preference.h"
#include "iException.h"
#include "iString.h"
#include "Preference.h"

using namespace std;
int main (void) {
  Isis::Preference::Preferences(true);

  try {
    stringstream in;
    in << "Group = DataFileName" << endl;
    in << "  InputKey = ^IMAGE" << endl;
    in << "  Translation = (*,*)" << endl;
    in << "EndGroup" << endl;
    in << "Group = DataFileRecordBytes" << endl;
    in << "  InputKey = RECORD_BYTES" << endl;
    in << "  Translation = (*,*)" << endl;
    in << "EndGroup" << endl;
    in << "Group = CoreLines" << endl;
    in << "  InputGroup = IMAGE" << endl;
    in << "  InputKey = LINES" << endl;
    in << "  Translation = (*,*)" << endl;
    in << "EndGroup" << endl;
    in << "Group = CoreBitsPerPixel" << endl;
    in << "  Auto" << endl;
    in << "  OutputName = BitsPerPixel" << endl;
    in << "  OutputPosition = (\"Object\",\"IsisCube\", \"Object\",\"Core\",";
    in <<                      "\"Group\",\"Pixels\")" << endl;
    in << "  InputGroup = IMAGE" << endl;
    in << "  InputKey = SAMPLE_BITS" << endl;
    in << "  InputDefault = 8" << endl;
    in << "  Translation = (8,8)" << endl;
    in << "  Translation = (16,16)" << endl;
    in << "  Translation = (32,32)" << endl;
    in << "EndGroup" << endl;
    in << "Group = CorePixelType" << endl;
    in << "  InputGroup = IMAGE" << endl;
    in << "  InputKey = SAMPLE_TYPE" << endl;
    in << "  InputDefault = LSB_INTEGER" << endl;
    in << "  Translation = (Integer,LSB_INTEGER)" << endl;
    in << "  Translation = (Integer,MSB_INTEGER)" << endl;
    in << "  Translation = (Integer,PC_INTEGER)" << endl;
    in << "  Translation = (Integer,MAC_INTEGER)" << endl;
    in << "  Translation = (Integer,SUN_INTEGER)" << endl;
    in << "  Translation = (Natural,UNSIGNED_INTEGER)" << endl;
    in << "  Translation = (Unknown,*)" << endl;
    in << "EndGroup" << endl;
    in << "Group = CoreByteOrder" << endl;
    in << "  InputGroup = IMAGE" << endl;
    in << "  InputKey = SAMPLE_TYPE" << endl;
    in << "  InputDefault = LSB_INTEGER" << endl;
    in << "  Translation = (LittleEndian,LSB_INTEGER)" << endl;
    in << "  Translation = (BigEndian,MSB_INTEGER)" << endl;
    in << "  Translation = (LittleEndian,PC_INTEGER)" << endl;
    in << "  Translation = (BigEndian,MAC_INTEGER)" << endl;
    in << "  Translation = (BigEndian,SUN_INTEGER)" << endl;
    in << "  Translation = (LittleEndian,UNSIGNED_INTEGER)" << endl;
    in << "EndGroup" << endl;
    in << "End" << endl;

    Isis::PvlTranslationTable table(in);

    string group,key;

    cout << "Unit test for Isis::PvlTranslationTable" << endl << endl;

    cout << "  Test InputGroup :" << endl;

    cout << "    InputGroup (\"DataFileName\") = " <<
            table.InputGroup ("DataFileName") << endl;
    cout << "    InputGroup (\"CoreLines\") = " <<
            table.InputGroup ("CoreLines") << endl;
    try {
      table.InputGroup ("tttt1");
    }
    catch (Isis::iException &e) {
      cerr << "    ";
      e.Report(false);
      cerr << endl;
    }

    cout << "  Test InputKey :" << endl;

    cout << "    InputKeyword (\"DataFileName\") = " <<
            table.InputKeyword ("DataFileName") << endl;
    cout << "    InputKeyword (\"CoreLines\") = " <<
            table.InputKeyword ("CoreLines") << endl;
    try {
      table.InputKeyword ("tttt2");
    }
    catch (Isis::iException &e) {
      cerr << "    ";
      e.Report(false);
      cerr << endl;
    }

    cout << "  Test InputDefault :" << endl;

    cout << "    InputDefault (\"DataFileName\") = " <<
            table.InputDefault ("DataFileName") << endl;
    cout << "    InputDefault (\"CoreBitsPerPixel\") = " <<
            table.InputDefault ("CoreBitsPerPixel") << endl;
    try {
      table.InputDefault ("tttt3");
    }
    catch (Isis::iException &e) {
      cerr << "    ";
      e.Report(false);
      cerr << endl;
    }

    cout << "  Test Translate :" << endl;

    cout << "    Translate (\"DataFilename\", \"tttt4\") = " <<
            table.Translate ("DataFilename", "tttt4") << endl;
    cout << "    Translate (\"CoreByteOrder\",\"MSB_INTEGER\") = " <<
            table.Translate ("CoreByteOrder","MSB_INTEGER") << endl;
    cout << "    Translate (\"CorePixelType\") = " <<
            table.Translate ("CorePixelType") << endl;
    cout << "    Translate (\"CorePixelType\") = " <<
            table.Translate ("CorePixelType","baddata") << endl;
    try {
      table.Translate ("tttt6");
    }
    catch (Isis::iException &e) {
      cerr << "    ";
      e.Report(false);
    }

    try {
      table.Translate ("DataFileRecordBytes");
    }
    catch (Isis::iException &e) {
      cerr << "    ";
      e.Report(false);
    }

    try {
      table.Translate ("CoreBitsPerPixel", "31");
    }
    catch (Isis::iException &e) {
      cerr << "    ";
      e.Report(false);
      cerr << endl;
    }

    cout << "  Test AddTable :" << endl;

    in << "Group = CoreLineSuffixBytes" << endl;
    in << "  InputKey = LINE_SUFFIX_BYTES" << endl;
    in << "  Translation = (*,*)" << endl;
    in << "EndGroup" << endl;
    in << "Group = CoreLinePrefixBytes" << endl;
    in << "  InputKey = LINE_PREFIX_BYTES" << endl;
    in << "  Translation = (*,*)" << endl;
    in << "EndGroup" << endl;
    table.AddTable(in);

    cout << "    Translate (\"CoreLinePrefixBytes\", \"128\") = " <<
            table.Translate ("CoreLinePrefixBytes", "128") << endl << endl;


    class protectedTester : public Isis::PvlTranslationTable {
      public:
        void tester () {
          stringstream in2;
          in2 << "Group = DataFileName" << endl;
          in2 << "  InputKey = ^IMAGE" << endl;
          in2 << "  Translation = (*,*)" << endl;
          in2 << "EndGroup" << endl;
          in2 << "Group = CoreBitsPerPixel" << endl;
          in2 << "  Auto" << endl;
          in2 << "  Optional" << endl;
          in2 << "  OutputName = BitsPerPixel" << endl;
          in2 << "  OutputPosition = (\"Object\",\"IsisCube\", \"Object\",\"Core\",";
          in2 <<                      "\"Group\",\"Pixels\")" << endl;
          in2 << "  InputGroup = IMAGE" << endl;
          in2 << "  InputKey = SAMPLE_BITS" << endl;
          in2 << "  InputDefault = 8" << endl;
          in2 << "  Translation = (8,8)" << endl;
          in2 << "  Translation = (16,16)" << endl;
          in2 << "  Translation = (32,32)" << endl;
          in2 << "EndGroup" << endl;
          in2 << "End" << endl;
      
          AddTable(in2);

          cout << "  Test IsAuto :" << endl;
          cout << "    IsAuto (\"DataFileName\") = " <<
                  IsAuto("DataFileName") << endl;
          cout << "    IsAuto (\"CoreBitsPerPixel\") = " <<
                  IsAuto("CoreBitsPerPixel") << endl << endl;

          cout << "  Test IsOptional :" << endl;
          cout << "    IsAuto (\"DataFileName\") = " <<
                  IsOptional("DataFileName") << endl;
          cout << "    IsAuto (\"CoreBitsPerPixel\") = " <<
                  IsOptional("CoreBitsPerPixel") << endl << endl;

          cout << "  Test OutputPosition :" << endl;
          cout << "    OutputPosition (\"CoreBitsPerPixel\") yields : " <<
                  OutputPosition("CoreBitsPerPixel") << endl << endl;

          cout << "  Test OutputName :" << endl;
          cout << "    OutputName (\"CoreBitsPerPixel\") = " <<
                  OutputName("CoreBitsPerPixel") << endl << endl;

      };
    };
    protectedTester t;
    t.tester();

  }
  catch (Isis::iException &e) {
    e.Report();
  }

  return 0;
}