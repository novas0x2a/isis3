#include "Isis.h"
#include "Application.h"

using namespace std; 
using namespace Isis;

void IsisMain() {

  // Open the input cube
  UserInterface &ui = Application::GetUserInterface();
  Filename fname = ui.GetFilename("FROM");
  string name = fname.Basename();
  bool rmv = ui.GetBoolean("REMOVE");

  // Run viknosalt on the cube to remove the salt 
  string inFile = ui.GetFilename("FROM");
  string outFile = name + ".nosalt.cub";
  string parameters = "FROM=" + inFile + " TO=" + outFile;
  Isis::iApp ->Exec("viknosalt",parameters);

  // Run vikfixtrx on the cube to remove the tracks
  inFile = outFile;
  outFile = name + ".fixtrx.cub";
  parameters = "FROM=" + inFile + " TO=" + outFile;
  Isis::iApp ->Exec("vikfixtrx", parameters);
  if (rmv) remove(inFile.c_str());

  // Run findrx on the cube to find the nominal position of the reseaus
  inFile = outFile;
  parameters = "FROM=" + inFile;
  Isis::iApp ->Exec("findrx",parameters);

  // Run viknopepper on the cube to remove the pepper
  outFile = name + ".nopepper.cub";
  parameters = "FROM=" + inFile + " TO=" + outFile;
  Isis::iApp ->Exec("viknopepper",parameters);
  if (rmv) remove(inFile.c_str());

  // Run remrx on the cube to remove the reseaus
  inFile = outFile;
  outFile = name + ".remrx.cub";
  parameters = "FROM=" + inFile + " TO=" + outFile + " LDIM=" + 
    iString(ui.GetInteger("LDIM")) + " SDIM=" + iString(ui.GetInteger("SDIM"));
  Isis::iApp -> Exec("remrx",parameters);
  if (rmv) remove(inFile.c_str());

  // Run vikfixfly on the cube
  inFile = outFile;
  outFile = ui.GetFilename("TO");
  parameters = "FROM=" + inFile + " TO=" + outFile;
  Isis::iApp ->Exec("viknobutter",parameters);
  if (rmv) remove(inFile.c_str());       
}

