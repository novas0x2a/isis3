#include "Isis.h"
#include "Preference.h"

#include <iostream>

using namespace std;
void IsisMain () {
  Isis::Preference::Preferences(true);
  cout << "That's all folks" << endl;
}
