#include <iostream>
#include "iException.h"
#include "PvlKeyword.h"
#include "PvlSequence.h"
#include "Preference.h"

using namespace std;
int main () {
  Isis::Preference::Preferences(true);

  try {
    const Isis::PvlKeyword keyN("THE_INTERNET", 
                          "Seven thousand eight hundred forty three million seventy four nine seventy six forty two eighty nine sixty seven thirty five million jillion bajillion google six nine four one two three four five six seven eight nine ten eleven twelve thirteen fourteen", 
                          "terrabytes");
    cout << keyN <<endl;

    const Isis::PvlKeyword keyZ("BIG_HUGE_LONG_NAME_THAT_SHOULD_TEST_OUT_PARSING", 
                          "Seven thousand eight hundred forty three million seventy four", 
                          "bubble baths");
    cout << keyZ <<endl;

    Isis::PvlKeyword keyU("ARRAY_TEST", 5.87, "lightyears");
    keyU.AddValue(5465.6, "lightyears");
    keyU.AddValue(574.6, "lightyears");
    keyU.AddValue(42, "dogs");

    cout << keyU << endl;

    const Isis::PvlKeyword keyV("FIRST_100_DIGITS_OF_PI", "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679");

    cout << keyV << endl;


    const Isis::PvlKeyword keyJ("A", "XXXXXXXXXXxxxxxxxxxxXXXXXXXXXXxxxxxxxxxxXXXXXXXXXXxxxxxxxxxxXXXXXXXXXXxxxx");

    cout << keyJ << endl;

    Isis::PvlKeyword keyW("UGHHHHHHHHHHHH");
    keyW += 59999.0;
    keyW += 59999.0;
    keyW += 59999.0;
    keyW += 59999.0;
    keyW += 59999.0;
    keyW += 59999.0;
    keyW += 59999.0;
    keyW += 59999.0;
    keyW += 59999.0;
    keyW += 59999.0;
    keyW += 59999.0;
    keyW += 59999.0;
    cout << keyW << endl;

    const Isis::PvlKeyword key("NAME",5.2,"meters");

    cout << key << endl;

    Isis::PvlKeyword key2("KEY");
    cout << key2 << endl;

    key2 += 5;
    key2 += string("");
    key2.AddValue(3.3,"feet");
    key2.AddValue("Hello World!");
    string str = "Hello World! This is a really really long comment that needs to"
                 " be wrapped onto several different lines to make the PVL file "
                 "look really pretty!";
    key2.AddCommentWrapped(str);
    cout << key2 << endl;

    cout << key2[1] << endl;
    key2[1] = 88;
    cout << key2 << endl;

    class MyKeyword : public Isis::PvlKeyword {
    public:
      MyKeyword(vector<Isis::PvlToken> toks, vector<Isis::PvlToken>::iterator &it) :
      Isis::PvlKeyword(toks,it) {
      }
    };

    vector<Isis::PvlToken> toks;

    Isis::PvlToken t;
    t.SetKey("dog"); t.AddValue("Big");
    toks.push_back(t);

    t.SetKey("_COMMENT_");
    toks.push_back(t);

    t.SetKey("Cat"); t.AddValue("Tabby");
    cout << (const string)t.GetValue(0) << (const string)t.GetValue(1) << endl;
    toks.push_back(t);

    t.SetKey("<Fish>");
    toks.push_back(t);

    vector<Isis::PvlToken>::iterator it = toks.begin();

    cout << "1 ... " << endl;
    cout << MyKeyword(toks,it) << endl << endl;
    cout << "2 ... " << endl;
    cout << MyKeyword(toks,it) << endl << endl;

    Isis::PvlSequence seq;
    seq += "(a,b,c)";
    seq += "(\"Hubba Hubba\",\"Bubba\")";
    Isis::PvlKeyword k("key");
    k = seq;
    cout << k << endl;

    //Test casting operators
    cout << "----------------------------------------" << endl;
    cout << "Testing cast operators" << endl;
    Isis::PvlKeyword cast01 ("cast1", "I'm being casted");
    Isis::PvlKeyword cast02 ("cast2", "465721");
    Isis::PvlKeyword cast03 ("cast3", "131.2435");
    cout << "string     = " << (string)cast01 << endl;
    cout << "int     = " << (int)cast02 << endl;
    cout << "BigInt     = " << (Isis::BigInt)cast02 << endl;
    cout << "double     = " << (double)cast03 << endl;

  } catch (Isis::iException &e) {
    e.Report(false);
  } catch (...) {
    cout << "Unknown error" << endl;
  }
  try {
    Isis::PvlKeyword key(" Test_key_2 ","Might work");
    cout << key << endl;
    Isis::PvlKeyword key2("Bob is a name","Yes it is");
  } catch (Isis::iException &e) {
    e.Report(false);
  }

}
