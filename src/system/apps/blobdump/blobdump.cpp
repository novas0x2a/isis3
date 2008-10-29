#include "Isis.h"

#include <fstream>

#include "Blob.h"
#include "Filename.h"

using namespace std;
using namespace Isis;

void IsisMain(){
  UserInterface &ui = Application::GetUserInterface();
  Filename file = ui.GetFilename("FROM");
  string blobname = ui.GetString("NAME");
  string blobtype = ui.GetString("TYPE");
  Blob blob( blobname, blobtype, file.Expanded());
  Filename outfname = ui.GetFilename("TO");
  blob.Write(outfname.Expanded());
}
