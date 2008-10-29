#include "Application.h"
#include "UserInterface.h"

using namespace std; 
using namespace Isis;

void interface ();

int main (int argc, char *argv[]) {
  Application app(argc-1,&argv[1]);
  return app.Exec(interface);
}

void interface () {
  PvlObject hist = Isis::iApp->History();
  PvlGroup up = hist.FindGroup("UserParameters");
  Pvl pvl;
  pvl.AddGroup(up);
  cout << pvl << endl;
}
