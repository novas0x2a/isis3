#include <iostream>
#include "UserInterface.h"
#include "iException.h"
#include "Preference.h"
#include "Filename.h"

using namespace std;

int main (int argc, char *argv[]) {
  Isis::Preference::Preferences(true);


  cout << "Unit test for Isis::UserInterface ..." << endl;

  try {
  int myArgc = 2;
  char *myArgv[2] = {"unitTest","from=input.cub to=output.cub"};

  Isis::iString unitTestXml = Isis::Filename("unitTest.xml").Expanded();
  Isis::UserInterface ui(unitTestXml,myArgc,myArgv);
  cout << "FROM:    " << ui.GetAsString("FROM") << endl;
  cout << "TO:      " << ui.GetAsString("TO") << endl;
  cout << "GUI:     " << ui.IsInteractive() << endl;
  cout << endl;

  string highpass = Isis::Filename("$ISISROOT/src/base/apps/highpass/highpass.xml").Expanded();
  char *myArgv2[2] = {"highpass","from=dog to=biscuit line=3 samp=3"};
  Isis::UserInterface ui2(highpass,myArgc,myArgv2);
  cout << "FROM:    " << ui2.GetAsString("FROM") << endl;
  cout << "TO:      " << ui2.GetAsString("TO") << endl;
  cout << "GUI:     " << ui2.IsInteractive() << endl;
  cout << endl;

  myArgc = 1;
  char *myArgv3[1] = {"unitTest"};
  Isis::UserInterface ui3(unitTestXml,myArgc,myArgv3);
  cout << "FROM:    " << ui3.GetAsString("FROM") << endl;
  cout << "TO:      " << ui3.GetAsString("TO") << endl;
  cout << "GUI:     " << ui3.IsInteractive() << endl;
  cout << endl;

  myArgc = 1;
  char *myArgv5[1] = {"unitTest"};
  Isis::UserInterface ui5(unitTestXml,myArgc,myArgv5);
  cout << "GUI:     " << ui5.IsInteractive() << endl;
  cout << endl;

  myArgc = 1;
  char *myArgv6[1] = {"./unitTest"};
  Isis::UserInterface ui6(unitTestXml,myArgc,myArgv6);
  cout << "GUI:     " << ui6.IsInteractive() << endl;
  cout << endl;

  cout << "Starting Batchlist Test" << endl;
  myArgc = 2;
  char *myArgv8[2] = {"unitTest","from= $1 to= $2 -batchlist=unitTest.lis"};
  Isis::UserInterface ui8(unitTestXml,myArgc,myArgv8);
  for (int i=0; i< ui8.BatchListSize(); i++) {
    ui8.SetBatchList(i);
    cout << "FROM:    " << ui.GetAsString("FROM") << endl;
    cout << "TO:      " << ui.GetAsString("TO") << endl;
    cout << "GUI:     " << ui.IsInteractive() << endl;
    cout << endl;
  }
  cout << "Finished Batchlist Test" << endl;
  cout << endl;

  try {
  myArgc = 2;
  char *myArgv7[2] = {"./unitTest","-restore=unitTest.par"};
  Isis::UserInterface ui7(unitTestXml,myArgc,myArgv7);
  cout << "FROM:    " << ui7.GetAsString("FROM") << endl;
  cout << "TO:      " << ui7.GetAsString("TO") << endl;
  cout << "GUI:     " << ui7.IsInteractive() << endl;
  cout << endl;
  }
  catch (Isis::iException &e) {
    e.Report(false);
    cout << endl;
  }

  try {
  myArgc = 2;
  char *myArgv3[2] = {"$ISISROOT/src/base/apps/highpass/highpass","bogus=parameter"};
  Isis::UserInterface stupid(highpass,myArgc,myArgv3);
  }
  catch (Isis::iException &e) {
    e.Report(false);
    cout << endl;
  }

  try {
  myArgc = 2;
  char *myArgv7[2] = {"$ISISROOT/src/base/apps/highpass/highpass","-restore=junk.par"};
  Isis::UserInterface stupid(highpass,myArgc,myArgv7);
  }
  catch (Isis::iException &e) {
    e.Report(false);
  }
  }
  catch (Isis::iException &e) {
    e.Report(false);
  }
}
