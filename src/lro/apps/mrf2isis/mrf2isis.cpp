#include "Isis.h"

#include <cstdio>
#include <string>

#include "ProcessImportPds.h"

#include "UserInterface.h"
#include "Filename.h"

using namespace std; 
using namespace Isis;

void IsisMain ()
{
  ProcessImportPds p;
  Pvl label;
  UserInterface &ui = Application::GetUserInterface();

  string labelFile = ui.GetFilename("FROM");
  Filename inFile = ui.GetFilename("FROM");
  iString id;

  try {
    Pvl lab(inFile.Expanded());
    id = (string) lab.FindKeyword ("DATA_SET_ID");
  }
  catch (iException &e) {
    string msg = "Unable to read [DATA_SET_ID] from input file [" +
                 inFile.Expanded() + "]";
    throw iException::Message(iException::Io,msg, _FILEINFO_);
  }

  id.ConvertWhiteSpace();
  id.Compress();
  id.Trim(" ");
  if (id != "CHAN1-L-MRFFR-5-CDR-MAP-V1.0" && id != "CHAN1-L-MRFFR-4-CDR-V1.0" &&
      id != "CH1-ORB-L-MRFFR-4-CDR-V1.0" && id != "CH1-ORB-L-MRFFR-5-CDR-MAP-V1.0" &&
      id != "CH1-ORB-L-MRFFR-5-CDR-MOSAIC-V1.0" &&
      id != "LRO-L-MRFLRO-3-CDR-V1.0" && id != "LRO-L-MRFLRO-5-CDR-MAP-V1.0" &&
      id != "LRO-L-MRFLRO-4-CDR-V1.0" && id != "LRO-L-MRFLRO-5-CDR-MOSAIC-V1.0") {
    string msg = "Input file [" + inFile.Expanded() + "] does not appear to be " +
                 "in CHANDRAYAAN-1 MINI-RF FORERUNNER level 1 or level 2 format " +
		 "or in LUNAR RECONNAISSANCE ORBITER MINI-RF LRO level 1 or " +
		 "level 2 format. " +
		 "DATA_SET_ID is [" + id + "]";
    throw iException::Message(iException::Io,msg, _FILEINFO_);
  }

  p.SetPdsFile (labelFile, "", label);
  Cube *outcube = p.SetOutputCube ("TO");

  p.SetOrganization(Isis::ProcessImport::BIP);
  p.StartProcess ();

  // Get the mapping labels
  Pvl otherLabels;
  p.TranslatePdsProjection(otherLabels);

  // Get the directory where the MiniRF level 2 translation tables are.
  PvlGroup dataDir (Preference::Preferences().FindGroup("DataDirectory"));
  iString transDir = (string) dataDir["Lro"] + "/translations/";

  if (id == "CHAN1-L-MRFFR-5-CDR-MAP-V1.0" || id == "LRO-L-MRFLRO-5-CDR-MAP-V1.0") {
    // Translate the BandBin group
    Filename transFile (transDir + "mrflev2BandBin.trn");
    PvlTranslationManager bandBinXlater (label, transFile.Expanded());
    bandBinXlater.Auto(otherLabels);

    // Translate the Archive group
    transFile = transDir + "mrflev2Archive.trn";
    PvlTranslationManager archiveXlater (label, transFile.Expanded());
    archiveXlater.Auto(otherLabels);

    // Write the BandBin, Archive, and Mapping groups to the output cube label
    outcube->PutGroup(otherLabels.FindGroup("BandBin"));
    outcube->PutGroup(otherLabels.FindGroup("Mapping"));
    outcube->PutGroup(otherLabels.FindGroup("Archive"));
  } else {
    // Translate the BandBin group
    Filename transFile (transDir + "mrflev1BandBin.trn");
    PvlTranslationManager bandBinXlater (label, transFile.Expanded());
    bandBinXlater.Auto(otherLabels);

    // Translate the Archive group
    transFile = transDir + "mrflev1Archive.trn";
    PvlTranslationManager archiveXlater (label, transFile.Expanded());
    archiveXlater.Auto(otherLabels);

    // Translate the Instrument group
    transFile = transDir + "mrflev1Instrument.trn";
    PvlTranslationManager instrumentXlater (label, transFile.Expanded());
    instrumentXlater.Auto(otherLabels);

    // Translate the Image group
    transFile = transDir + "mrflev1Image.trn";
    PvlTranslationManager imageXlater (label, transFile.Expanded());
    imageXlater.Auto(otherLabels);

    // Write the BandBin, Archive, Instrument, and ImageInfo groups
    // to the output cube label
    outcube->PutGroup(otherLabels.FindGroup("BandBin"));
    outcube->PutGroup(otherLabels.FindGroup("Archive"));
    outcube->PutGroup(otherLabels.FindGroup("Instrument"));
    outcube->PutGroup(otherLabels.FindGroup("ImageInfo"));
  }

  p.EndProcess ();
}
