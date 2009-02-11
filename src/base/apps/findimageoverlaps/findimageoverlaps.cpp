#include "Isis.h"
#include "SerialNumberList.h"
#include "ImageOverlapSet.h"

using namespace std;
using namespace Isis;

void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();
  SerialNumberList serialNumbers(ui.GetFilename("FROMLIST"));
  ImageOverlapSet overlaps(true);
  overlaps.FindImageOverlaps(serialNumbers);
  overlaps.WriteImageOverlaps(Filename(ui.GetFilename("TO")).Expanded());

  // This will only occur when "CONTINUE" was true, so we can assume "ERRORS" was 
  //   an entered parameter.
  if(overlaps.Errors().size() != 0 && ui.WasEntered("ERRORS")) {
    Pvl outFile;

    bool filenamesOnly = !ui.GetBoolean("DETAILED");

    vector<PvlGroup> errorList = overlaps.Errors();

    for(unsigned int err = 0; err < errorList.size(); err++) {
      if(!filenamesOnly) {
        outFile += errorList[err];
      }
      else if(errorList[err].HasKeyword("Filenames")) {
        PvlGroup origError = errorList[err];
        PvlGroup err("ImageOverlapError");

        for(int keyword = 0; keyword < origError.Keywords(); keyword++) {
          if(origError[keyword].Name() == "Filenames") {
            err += origError[keyword];
          }
        }

        outFile += err;
      }
    }

    outFile.Write(Filename(ui.GetFilename("ERRORS")).Expanded());
  }

  PvlGroup results("Results");
  results += PvlKeyword("ErrorCount", (BigInt)overlaps.Errors().size());
  Application::Log(results);
}
