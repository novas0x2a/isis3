#include <iostream>
#include "Pvl.h"
#include "iException.h"
#include "Preference.h"

using namespace std;

int main () { 
  Isis::Preference::Preferences(true);

  Isis::Pvl p;
  p += Isis::PvlKeyword("LongKeyword", "This is a very long keyword value which "
                        "was causing some problems when the Pvl was output."
                        " The fist couple of lines looked good, but after that "
                        "things went south. Some lines get nothing, others get"
                        " bad indenting, most were too short");

  Isis::PvlGroup g("Test");
  g += Isis::PvlKeyword("Keyword", "Value");
  p.AddGroup(g);

  p.SetTerminator("");
  p.Write("tmp.unitTest");
  p.Append("tmp.unitTest");

  Isis::Pvl p2;
  p2.Read("tmp.unitTest");
  cout << p2 << endl;

  remove("tmp.unitTest");
}
