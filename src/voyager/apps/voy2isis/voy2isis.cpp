#include "Isis.h"

#include <cstdio>
#include <string>

#include "Filename.h"
#include "iException.h"
#include "iString.h"
#include "ProcessImportPds.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "PvlTranslationManager.h"
#include "System.h"
#include "UserInterface.h"

using namespace std; 
using namespace Isis;

void TranslateVoyagerLabels (Pvl &inputLabel, Cube *ocube);

void IsisMain ()
{
  // We should be processing a PDS file
  ProcessImportPds p;
  UserInterface &ui = Application::GetUserInterface();
  Filename in = ui.GetFilename("FROM");

  string tempName = "$TEMPORARY/" + in.Basename() + ".img";
  Filename temp (tempName);

  cout << temp.Expanded() << endl;

  bool tempFile = false;

  // input files are compressed, use vdcomp to decompress
  iString ext = iString(in.Extension()).UpCase();
  if (ext == "IMQ") {
    try {
      string command = "$ISISROOT/bin/vdcomp " + in.Expanded() + " " + temp.Expanded();
      System(command); 
      in = temp.Expanded();
      tempFile = true;
    }
    catch (iException &e) {
      throw iException::Message(iException::Io, 
                                "Unable to decompress input file ["
                                + in.Name() + "].", _FILEINFO_);
    }
  }
  else {
    string msg = "Input file [" + in.Name() + 
               "] does not appear to be a Voyager EDR";
    throw iException::Message(iException::User,msg, _FILEINFO_);
  }
  // Convert the pds file to a cube
  Pvl pdsLabel;
  try {
    p.SetPdsFile (in.Expanded(),"",pdsLabel);
  }
  catch (iException &e) {
    string msg = "Unable to set PDS file.  Decompressed input file [" 
                 + in.Name() + "] does not appear to be a PDS product";
    throw iException::Message(iException::User,msg, _FILEINFO_);
  }

  Cube *ocube = p.SetOutputCube("TO");
  p.StartProcess ();
  TranslateVoyagerLabels(pdsLabel, ocube);
  p.EndProcess ();

  if (tempFile) remove (temp.Expanded().c_str());
  return;
}

/**
 * @brief Translate labels into Isis3 
 * 
 * @param inputLabel Reference to pvl label from the input image
 * @param ocube Pointer to output cube
 * @internal 
 *   @history 2009-03-11 Jeannie Walldren - Original Version
 */
void TranslateVoyagerLabels (Pvl &inputLabel, Cube *ocube) {
  // Get the directory where the Voyager translation tables are
  PvlGroup &dataDir = Preference::Preferences().FindGroup("DataDirectory");
  iString missionDir = (string) dataDir[(string)inputLabel["SpacecraftName"]];
  Filename transFile = (missionDir + "/translations/voyager.trn");

  // Get the translation manager ready
  PvlTranslationManager labelXlater (inputLabel, transFile.Expanded());

  // Pvl output label
  Pvl *outputLabel = ocube->Label();
  labelXlater.Auto(*(outputLabel));

  // Add needed keywords that are not in the translation table
  PvlGroup &inst = outputLabel->FindGroup("Instrument",Pvl::Traverse);

  //Add units of measurement to keywords from translation table
  double exposureDuration = inst.FindKeyword("ExposureDuration");
  inst.FindKeyword("ExposureDuration").SetValue(exposureDuration, "seconds");

  // Setup the kernel group
  PvlGroup kern("Kernels");
  iString spn;
  iString instId = (string) inst.FindKeyword("InstrumentId");
  if ((string) inst.FindKeyword("SpacecraftName") == "VOYAGER_1") {
    spn = "1";
    if (instId == "NARROW_ANGLE_CAMERA") {
      kern += PvlKeyword("NaifFrameCode",-31001);
      instId = "issna";
    }
    else {
      kern += PvlKeyword("NaifFrameCode",-31002);
      instId = "isswa";
    }
  }
  else {
    spn = "2";
    if (instId == "NARROW_ANGLE_CAMERA") {
      kern += PvlKeyword("NaifFrameCode",-32001);
      instId = "issna";
    }
    else {
      kern += PvlKeyword("NaifFrameCode",-32002);
      instId = "isswa";
    }
  }
  ocube->PutGroup(kern);

  // Set up the nominal reseaus group
  PvlGroup res("Reseaus");
  Pvl nomRes("$voyager" + spn + "/reseaus/nominal.pvl");
  PvlKeyword samps,lines,type,valid;
  lines = PvlKeyword("Line");
  samps = PvlKeyword("Sample");
  type = PvlKeyword("Type");
  valid = PvlKeyword("Valid");

  PvlKeyword key = nomRes.FindKeyword("VG" + spn + "_" + instId.UpCase() + "_RESEAUS");
  int numRes = nomRes["VG" + spn + "_" + instId.UpCase() + "_NUMBER_RESEAUS"];
  for (int i=0; i<numRes*3; i+=3) {
    lines += key[i];
    samps += key[i+1];
    type += key[i+2];
    valid += 0;
  }
  res += lines;
  res += samps;
  res += type;
  res += valid;
  res += PvlKeyword("Template","$voyager" + spn + "/reseaus/vg" + spn + "." + instId.DownCase() + ".template.cub");
  res += PvlKeyword("Status","Nominal");
  ocube->PutGroup(res);
}
