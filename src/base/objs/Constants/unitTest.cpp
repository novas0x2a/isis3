#include <iostream>
#include <iomanip>
#include "Constants.h"
#include "Preference.h"

using namespace std;

int main (int argc, char *argv[]) {
  Isis::Preference::Preferences(true);

  cout << "Unit test for IsisConstants" << endl;

  cout << setprecision(16) << setw(17) << endl;
  cout << Isis::PI << endl;
  cout << Isis::HALFPI << endl;
  cout << Isis::E << endl;

  cout << sizeof(Isis::BigInt) << endl;

}






