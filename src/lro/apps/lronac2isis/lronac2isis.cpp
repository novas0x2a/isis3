#include "Isis.h"
#include "ProcessImportPds.h"
#include "ProcessByLine.h"
#include "UserInterface.h"
#include "Filename.h"
#include "iException.h"
#include "Preference.h"
#include "iString.h"
#include "Pvl.h"
#include "Table.h"
#include "Stretch.h"
#include "TextFile.h"
#include "PixelType.h"

using namespace std;
using namespace Isis;

// Global variables for processing functions
Stretch stretch;

void FixDns8 (Buffer &buf);
void TranslateLrocNacLabels (Filename &labelFile, Cube *ocube);
bool fillGap;

int preStartPix = 0,
    preEndPix = 0,
    sufStartPix = 0,
    sufEndPix = 0;

void IsisMain (){
  ProcessImportPds p;

  //Check that the file comes from the right camera
  UserInterface &ui = Application::GetUserInterface();
  Filename inFile = ui.GetFilename("FROM");
  fillGap = false;
  iString id;
  int sumMode;
  bool projected;
  try {
    Pvl lab(inFile.Expanded());

    if (lab.HasKeyword("DATA_SET_ID"))
    	id = (string) lab.FindKeyword ("DATA_SET_ID");
    else {
        string msg = "Unable to read [DATA_SET_ID] from input file [" +
                     inFile.Expanded() + "]";
        throw iException::Message(iException::Io,msg, _FILEINFO_);
    }

    projected = lab.HasObject("IMAGE_MAP_PROJECTION");

    if (lab.HasKeyword("CROSSTRACK_SUMMING")) {
      sumMode = (int)lab.FindKeyword("CROSSTRACK_SUMMING");
    }
    else {
      sumMode = (int)lab.FindKeyword("SAMPLING_FACTOR");
    }

    // Create the stretch pairs
    stretch.ClearPairs();
    PvlKeyword lut = lab.FindKeyword ("LRO:LOOKUP_CONVERSION_TABLE");

    for (int i=0; i<lut.Size(); i++) {
    	iString pair = lut[i];
    	pair.ConvertWhiteSpace();
    	pair.Remove("() ");
    	iString inValue = pair.Token(" ,");
    	iString outValue = pair.Token(" ,");
    	stretch.AddPair(inValue, outValue);
    }

  }
  catch (iException &e) {
    string msg = "The PDS header is missing important keyword(s).";
    throw iException::Message(iException::Io,msg, _FILEINFO_);
  }

  //Checks if in file is rdr
  if( projected ) {
    string msg = "[" + inFile.Name() + "] appears to be an rdr file.";
    msg += " Use pds2isis.";
    throw iException::Message(iException::User,msg, _FILEINFO_);
  }

  id.ConvertWhiteSpace();
  id.Compress();
  id.Trim(" ");
  if (id != "LRO-L-LROC-2-EDR-V1.0") {
    string msg = "Input file [" + inFile.Expanded() + "] does not appear to be " +
                 "in LROC-NAC EDR format. DATA_SET_ID is [" + id + "]";
    throw iException::Message(iException::Io,msg, _FILEINFO_);
  }

  //Process the file
  Pvl pdsLab;
  p.SetPdsFile(inFile.Expanded(), "", pdsLab);

  p.SetDimensions(p.Samples(),p.Lines(),p.Bands());

  // Set the output bit type to Real
  CubeAttributeOutput &outAtt = ui.GetOutputAttribute("TO");
  outAtt.PixelType (Isis::SignedWord);
  outAtt.Minimum((double)VALID_MIN2);
  outAtt.Maximum((double)VALID_MAX2);
  Cube *ocube = p.SetOutputCube(ui.GetFilename("TO"), outAtt);

  // Translate the labels
  p.StartProcess();
  TranslateLrocNacLabels(inFile, ocube);
  p.EndProcess();

  // Do 8 bit to 12 bit conversion
  //fillGap = ui.GetBoolean("FILLGAP");
  fillGap = false;
  ProcessByLine p2;
  string ioFile = ui.GetFilename("TO");
  CubeAttributeInput att;
  p2.SetInputCube(ioFile, att, ReadWrite);
  p2.Progress()->SetText("Converting 8 bit pixels to 16 bit");
  p2.StartProcess(FixDns8);
  p2.EndProcess();
}

// The input buffer has a raw 16 bit buffer but the values are still 0 to 255
void FixDns8 (Buffer &buf) {
  for (int i=0; i<buf.size(); i++) {
    if(!fillGap || buf[i] != 0) {
      buf[i] = stretch.Map(buf[i]);
    }
    else {
      buf[i] = Isis::Null;
    }
  }
}

//Function to translate the labels
void TranslateLrocNacLabels(Filename &labelFile, Cube *ocube){

  //Pvl to store the labels
  Pvl outLabel;
  //Set up the directory where the translations are
  PvlGroup dataDir (Preference::Preferences().FindGroup("DataDirectory"));
  iString transDir = (string) dataDir["Lro"] + "/translations/";
  Pvl labelPvl (labelFile.Expanded());

  //Translate the Instrument group
  Filename transFile (transDir + "lronacInstrument.trn");
  PvlTranslationManager instrumentXlator (labelPvl, transFile.Expanded());
  instrumentXlator.Auto(outLabel);

  //Translate the Archive group
  transFile  = transDir + "lronacArchive.trn";
  PvlTranslationManager archiveXlater (labelPvl, transFile.Expanded());
  archiveXlater.Auto (outLabel);

  // Set up the BandBin groups
  PvlGroup bbin("BandBin");
  bbin += PvlKeyword("FilterName","BroadBand");
  bbin += PvlKeyword("Center",0.650,"micrometers");
  bbin += PvlKeyword("Width",0.150,"micrometers");

  Pvl lab(labelFile.Expanded());

  //Set up the Kernels group
  PvlGroup kern("Kernels");
  if (lab.FindKeyword("FRAME_ID")[0] == "LEFT")
	  kern += PvlKeyword("NaifFrameCode",-85600);
  else
	  kern += PvlKeyword("NaifFrameCode",-85610);

  int sumMode, startSamp;
  if (lab.HasKeyword("CROSSTRACK_SUMMING")) {
    sumMode = (int)lab.FindKeyword("CROSSTRACK_SUMMING");
  }
  else {
    sumMode = (int)lab.FindKeyword("SAMPLING_FACTOR");
  }

  startSamp = 0;
  PvlGroup inst = outLabel.FindGroup("Instrument", Pvl::Traverse);
  if (lab.FindKeyword("FRAME_ID")[0] == "LEFT") {
	  inst.FindKeyword("InstrumentId") = "NACL";
	  inst.FindKeyword("InstrumentName") = "LUNAR RECONNAISSANCE ORBITER NARROW ANGLE CAMERA LEFT";
  }
  else {
	  inst.FindKeyword("InstrumentId") = "NACR";
	  inst.FindKeyword("InstrumentName") = "LUNAR RECONNAISSANCE ORBITER NARROW ANGLE CAMERA RIGHT";
  }

  inst += PvlKeyword("SpatialSumming",sumMode);
  inst += PvlKeyword("SampleFirstPixel",startSamp);

  //Add all groups to the output cube
  ocube->PutGroup (inst);
  ocube->PutGroup (outLabel.FindGroup("Archive", Pvl::Traverse));
  ocube->PutGroup (bbin);
  ocube->PutGroup (kern);
}
