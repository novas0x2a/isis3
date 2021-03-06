#include "Isis.h"
#include "SerialNumberList.h"
#include "ImageOverlapSet.h"
#include "FileList.h"
#include "SerialNumber.h"

using namespace std;
using namespace Isis;

void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();
  SerialNumberList serialNumbers(true);

  FileList images(ui.GetFilename("FROMLIST"));
  // list of sns/filenames sorted by serial number
  vector< pair<string, string> > sortedList;

  // We want to sort the input data by serial number so that the same
  //   results are produced every time this program is run with the same
  //   images. This is a modified insertion sort.
  for(unsigned int image = 0; image < images.size(); image++) {
    unsigned int insertPos = 0;
    string sn = SerialNumber::Compose(images[image]);

    for(insertPos = 0; insertPos < sortedList.size(); insertPos++) {
      if(sn.compare(sortedList[insertPos].first) < 0) break;
    }

    pair<string, string> newPair = pair<string, string>(sn, images[image]);
    sortedList.insert(sortedList.begin() + insertPos, newPair);
  }

  // Add the serial numbers in sorted order now
  for(unsigned int i = 0; i < sortedList.size(); i++) {
    serialNumbers.Add(sortedList[i].second);
  }

  // Now we want the ImageOverlapSet to calculate our overlaps
  ImageOverlapSet overlaps(true);
  overlaps.FindImageOverlaps(serialNumbers, Filename(ui.GetFilename("TO")).Expanded());

  // These call were the old way of calculating overlaps and writing them
  //overlaps.FindImageOverlaps(serialNumbers);
  //overlaps.WriteImageOverlaps(Filename(ui.GetFilename("TO")).Expanded());

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
