#include "Isis.h"

#include <cstdio>
#include <string>

#include "ProcessImportPds.h"

#include "UserInterface.h"
#include "Filename.h"
#include "System.h"
#include "iException.h"
#include "Pvl.h"
#include "iString.h"

using namespace std; 
using namespace Isis;

void TranslateVikingLabels (Pvl &pdsLabel, Cube *ocube);

void IsisMain ()
{
  // We should be processing a PDS file
  ProcessImportPds p;
  UserInterface &ui = Application::GetUserInterface();
  Filename in = ui.GetFilename("FROM");
  string tempName = "$TEMPORARY/" + in.Name() + ".img";
  Filename temp (tempName);

  bool tempFile = false;

  // If the input file is compressed, use vdcomp to decompress
  iString ext = iString(in.Extension()).UpCase();
  if (ext == "IMQ") {
    try {
      string command = "$ISISROOT/bin/vdcomp " + in.Expanded() + " " + temp.Expanded();
      System(command); 
      in = temp.Expanded();
      tempFile = true;
    }
    catch (iException &e) {
      e.Clear();
    }
  }
  else if (ext != "IMG") {
    string msg = "Input file [" + in.Name() + 
               "] does not appear to be a Viking PDS product";
    throw iException::Message(iException::User,msg, _FILEINFO_);
  }
  // Convert the pds file to a cube
  Pvl pdsLabel;
  try {
    p.SetPdsFile (in.Expanded(),"",pdsLabel);
  }
  catch (iException &e) {
    string msg = "Input file [" + in.Name() + 
               "] does not appear to be a Viking PDS product";
    throw iException::Message(iException::User,msg, _FILEINFO_);
  }

  Cube *ocube = p.SetOutputCube("TO");
  p.StartProcess ();
  TranslateVikingLabels(pdsLabel, ocube);
  p.EndProcess ();

  if (tempFile) remove (temp.Expanded().c_str());
  return;
}

void TranslateVikingLabels (Pvl &pdsLabel, Cube *ocube) {
  // Setup the archive group
  PvlGroup arch("Archive");
  arch += PvlKeyword("DataSetId",(string)pdsLabel["DATA_SET_ID"]);
  arch += PvlKeyword("ProductId",(string)pdsLabel["IMAGE_ID"]);
  arch += PvlKeyword("MissonPhaseName",(string)pdsLabel["MISSION_PHASE_NAME"]);
  arch += PvlKeyword("ImageNumber",(string)pdsLabel["IMAGE_NUMBER"]);
  arch += PvlKeyword("OrbitNumber",(string)pdsLabel["ORBIT_NUMBER"]);
  ocube->PutGroup(arch);

  // Setup the instrument group
  // Note SpacecraftClockCount used to be FDS_COUNT
  PvlGroup inst("Instrument");
  inst += PvlKeyword("SpacecraftName",(string)pdsLabel["SPACECRAFT_NAME"]);
  inst += PvlKeyword("InstrumentId",(string)pdsLabel["INSTRUMENT_NAME"]);
  inst += PvlKeyword("TargetName",(string)pdsLabel["TARGET_NAME"]);

  iString stime = (string) pdsLabel["IMAGE_TIME"];
  stime.Trim("Z");
  inst += PvlKeyword("StartTime",stime);

  inst += PvlKeyword("ExposureDuration",
                         (string)pdsLabel["EXPOSURE_DURATION"],"seconds");
  inst += PvlKeyword("SpacecraftClockCount",(string)pdsLabel["IMAGE_NUMBER"]);
  inst += PvlKeyword("FloodModeId",(string)pdsLabel["FLOOD_MODE_ID"]);
  inst += PvlKeyword("GainModeId",(string)pdsLabel["GAIN_MODE_ID"]);
  inst += PvlKeyword("OffsetModeId",(string)pdsLabel["OFFSET_MODE_ID"]);
  ocube->PutGroup(inst);

  // Setup the band bin group
  PvlGroup bandBin("BandBin");
  string filterName = pdsLabel["FILTER_NAME"];
  bandBin += PvlKeyword("FilterName",filterName);

  int filterId;
  if (filterName == "BLUE") filterId = 1;
  if (filterName == "MINUS_BLUE") filterId = 2;
  if (filterName == "VIOLET") filterId = 3;
  if (filterName == "CLEAR") filterId = 4;
  if (filterName == "GREEN") filterId = 5;
  if (filterName == "RED") filterId = 6;
  bandBin += PvlKeyword("FilterId",filterId);
  ocube->PutGroup(bandBin);

  // Setup the kernel group
  PvlGroup kern("Kernels");
  int spn;
  if ((string) pdsLabel["SPACECRAFT_NAME"] == "VIKING_ORBITER_1") {
    if ((string) pdsLabel["INSTRUMENT_NAME"] == 
         "VISUAL_IMAGING_SUBSYSTEM_CAMERA_A") {
      kern += PvlKeyword("NaifFrameCode",-27001);
    }
    else {
      kern += PvlKeyword("NaifFrameCode",-27002);
    }
    spn=1;
  }
  else {
    if ((string) pdsLabel["INSTRUMENT_NAME"] == 
         "VISUAL_IMAGING_SUBSYSTEM_CAMERA_A") {
      kern += PvlKeyword("NaifFrameCode",-30001);
    }
    else {
      kern += PvlKeyword("NaifFrameCode",-30002);
    }
    spn=2;
  }
  ocube->PutGroup(kern);

  // Set up the nominal reseaus group
  PvlGroup res("Reseaus");
  Pvl nomRes("$viking" + iString(spn) + "/reseaus/nominal.pvl");
  PvlKeyword samps,lines,type,valid;
  lines = PvlKeyword("Line");
  samps = PvlKeyword("Sample");
  type = PvlKeyword("Type");
  valid = PvlKeyword("Valid");
  string prefix;
  if ((string) pdsLabel["SPACECRAFT_NAME"] == "VIKING_ORBITER_1") {
    if ((string) pdsLabel["INSTRUMENT_NAME"] == 
         "VISUAL_IMAGING_SUBSYSTEM_CAMERA_A") {
      prefix = "VO1_VISA_";
    }
    else {
      prefix = "VO1_VISB_";
    }
  }
  else {
    if ((string) pdsLabel["INSTRUMENT_NAME"] == 
         "VISUAL_IMAGING_SUBSYSTEM_CAMERA_A") {
      prefix = "VO2_VISA_";
    }
    else {
      prefix = "VO2_VISB_";
    }
  }
  PvlKeyword key = nomRes.FindKeyword(prefix + "RESEAUS");
  int numRes = nomRes[prefix + "NUMBER_RESEAUS"];
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
  if (prefix == "VO1_VISA_") {
    res += PvlKeyword("Template","$viking1/reseaus/vo1.visa.template.cub");
  }
  else if (prefix == "VO1_VISB_") {
    res += PvlKeyword("Template","$viking1/reseaus/vo1.visb.template.cub");
  }
  else if (prefix == "VO2_VISA_") {
    res += PvlKeyword("Template","$viking2/reseaus/vo2.visa.template.cub");
  }
  else res += PvlKeyword("Template","$viking2/reseaus/vo2.visb.template.cub");
  res += PvlKeyword("Status","Nominal");
  ocube->PutGroup(res);
}

