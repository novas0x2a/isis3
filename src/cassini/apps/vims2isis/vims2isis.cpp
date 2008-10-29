#include "Isis.h"
#include "ProcessImportPds.h"
#include "ProcessByLine.h"
#include "Filename.h"
#include "UserInterface.h"
#include "Filename.h"
#include "Pvl.h"
#include "iException.h"
#include "iString.h"
#include "Brick.h"

#include <algorithm>
#include <stdio.h>

using namespace std;
using namespace Isis;

void ReadVimsBIL(std::string inFile, const PvlKeyword &suffixItems, std::string outFile);
void TranslateVisLabels (Pvl &pdsLab, Cube *viscube);
void TranslateIrLabels (Pvl &pdsLab, Cube *ircube);
void ProcessCube (Buffer &in, Buffer &out);

void IsisMain ()
{
  UserInterface &ui = Application::GetUserInterface();
  Filename in = ui.GetFilename("FROM");
  Filename outIr = ui.GetFilename("IR");
  Filename outVis = ui.GetFilename("VIS");
  Pvl lab(in.Expanded());

  //Checks if in file is rdr
  if( lab.HasObject("IMAGE_MAP_PROJECTION") ) {
    string msg = "[" + in.Name() + "] appears to be an rdr file.";
    msg += " Use pds2isis.";
    throw iException::Message(iException::User,msg, _FILEINFO_);
  }

  //Make sure it is a vims cube
  try {
    PvlObject qube(lab.FindObject("QUBE"));
    iString id;
    id = (string)qube["DATA_SET_ID"];
    id.ConvertWhiteSpace();
    id.Compress();
    id.Trim(" ");
    if ((id != "CO-J-VIMS-2-EDR-V1.0") &&
        (id != "CO-S-VIMS-2-EDR-V1.0")) {
      string msg = "Invalid DATA_SET_ID [" + id + "]";
      throw iException::Message(iException::Pvl,msg,_FILEINFO_);
    }
  }
  catch (iException &e) {
    string msg = "Input file [" + in.Expanded() + 
                 "] does not appear to be " +
                 "in VIMS EDR/RDR format";
    throw iException::Message(iException::Io,msg,_FILEINFO_);
  }

  Filename tempname = in.Basename() + ".bsq.cub";
  Pvl pdsLab(in.Expanded());

  // It's VIMS, let's figure out if it has the suffix data or not
  if ((int)lab.FindObject("QUBE")["SUFFIX_ITEMS"][0] == 0) {
    // No suffix data, we can use processimportpds
    ProcessImportPds p;

    p.SetPdsFile (in.Expanded(), "", pdsLab);
    // Set up the temporary output cube
    //The temporary cube is set to Real pixeltype, regardless of input pixel type
    Isis::CubeAttributeOutput outatt = CubeAttributeOutput("+Real");
    p.SetOutputCube(tempname.Name(),outatt);
    p.StartProcess();
    p.EndProcess();
  }
  else {
    // We do it the hard way
    ReadVimsBIL(in.Expanded(), lab.FindObject("QUBE")["SUFFIX_ITEMS"], tempname.Name());
  }

  //Now separate the cubes
  ProcessByLine l;
  CubeAttributeInput inattvis = CubeAttributeInput("+1-96");
  l.SetInputCube(tempname.Name(),inattvis);
  Cube *oviscube = l.SetOutputCube("VIS");
  l.StartProcess(ProcessCube);
  TranslateVisLabels(pdsLab,oviscube);
  l.EndProcess();

  CubeAttributeInput inattir = CubeAttributeInput("+97-352");
  l.SetInputCube(tempname.Name(),inattir);
  Cube *oircube = l.SetOutputCube("IR");
  l.StartProcess(ProcessCube);
  TranslateIrLabels(pdsLab,oircube);
  l.EndProcess();

  //Clean up
  string tmp(tempname.Expanded());
  remove(tmp.c_str());
}

/** 
 * We created a method to manually skip the suffix and corner data for this image
 *   to avoid implementing it in ProcessImport and ProcessImportPds. To fully support 
 *   this file format, we would have to re-implement the ISIS2 Cube IO plus add prefix
 *   data features to it. This is a shortcut; because we know these files have one sideplane
 *   and four backplanes, we know how much data to skip when. This should be fixed if we ever
 *   decide to fully support suffix and corner data, which would require extensive changes to
 *   ProcessImport/ProcessImportPds. Method written by Steven Lambright.
 * 
 * @param inFilename Filename of the input file
 * @param outFile Filename of the output file
 */
void ReadVimsBIL(std::string inFilename, const PvlKeyword &suffixItems, std::string outFile) {
  Isis::PvlGroup &dataDir = Isis::Preference::Preferences().FindGroup("DataDirectory");
  string transDir = (string) dataDir["Base"];

  Pvl pdsLabel(inFilename);
  Isis::Filename transFile (transDir + "/" + "translations/pdsQube.trn");
  Isis::PvlTranslationManager pdsXlater (pdsLabel, transFile.Expanded());

  Isis::iString str;
  str = pdsXlater.Translate ("CoreBitsPerPixel");
  int bitsPerPixel = str.ToInteger();
  str = pdsXlater.Translate ("CorePixelType");
  PixelType pixelType = Isis::Real;

  if ((str == "Real") && (bitsPerPixel == 32)) {
    pixelType = Isis::Real;
  }
  else if ((str == "Integer") && (bitsPerPixel == 8)) {
    pixelType = Isis::UnsignedByte;
  }
  else if ((str == "Integer") && (bitsPerPixel == 16)) {
    pixelType = Isis::SignedWord;
  }
  else if ((str == "Integer") && (bitsPerPixel == 32)) {
    pixelType = Isis::SignedInteger;
  }
  else if ((str == "Natural") && (bitsPerPixel == 8)) {
    pixelType = Isis::UnsignedByte;
  }
  else if ((str == "Natural") && (bitsPerPixel == 16)) {
    pixelType = Isis::UnsignedWord;
  }
  else if ((str == "Natural") && (bitsPerPixel == 16)) {
    pixelType = Isis::SignedWord;
  }
  else if ((str == "Natural") && (bitsPerPixel == 32)) {
    pixelType = Isis::UnsignedInteger;
  }
  else {
    string msg = "Invalid PixelType and BitsPerPixel combination [" + str +
                 ", " + Isis::iString(bitsPerPixel) + "]";
    throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
  }

  str = pdsXlater.Translate ("CoreByteOrder");
  Isis::ByteOrder byteOrder = Isis::ByteOrderEnumeration(str);

  str = pdsXlater.Translate ("CoreSamples", 0);
  int ns = str.ToInteger();
  str = pdsXlater.Translate ("CoreLines", 2);
  int nl = str.ToInteger();
  str = pdsXlater.Translate ("CoreBands", 1);
  int nb = str.ToInteger();

  std::vector<double> baseList;
  std::vector<double> multList;

  str = pdsXlater.Translate ("CoreBase");
  baseList.clear();
  baseList.push_back(str.ToDouble());
  str = pdsXlater.Translate ("CoreMultiplier");
  multList.clear();
  multList.push_back(str.ToDouble());

  Cube outCube;
  outCube.SetPixelType(Isis::Real);
  outCube.SetDimensions(ns,nl,nb);
  outCube.Create(outFile);

  // Figure out the number of bytes to read for a single line
  int readBytes = Isis::SizeOf (pixelType);
  readBytes = readBytes * ns;
  char *in = new char [readBytes];

  // Set up an Isis::EndianSwapper object
  Isis::iString tok(Isis::ByteOrderName(byteOrder));
  tok.UpCase();
  Isis::EndianSwapper swapper (tok);

  ifstream fin;
  // Open input file
  Isis::Filename inFile(inFilename);
  fin.open (inFilename.c_str(), ios::in|ios::binary);
  if (!fin.is_open()) {
    string msg = "Cannot open input file [" + inFilename + "]";
    throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
  }

  // Handle the file header
  streampos pos = fin.tellg();
  int fileHeaderBytes = (int)(iString)pdsXlater.Translate("DataFileRecordBytes") *
    ((int)(iString)pdsXlater.Translate ("DataStart", 0) - 1);

  fin.seekg (fileHeaderBytes, ios_base::beg);

  // Check the last io
  if (!fin.good ()) {
    string msg = "Cannot read file [" + inFilename + "]. Position [" +
                 Isis::iString((int)pos) + "]. Byte count [" +
                 Isis::iString(fileHeaderBytes) + "]" ;
    throw Isis::iException::Message(Isis::iException::Io,msg, _FILEINFO_);
  }

  // Construct a line buffer manager
  Isis::Brick out(ns, 1, 1, Isis::Real);

  // Loop for each line
  for (int line=0; line<nl; line++) {
    // Loop for each band 
    for (int band=0; band<nb; band++) {
      // Set the base multiplier
      double base,mult;
      if (baseList.size() > 1) {
        base = baseList[band];
        mult = multList[band];
      }
      else {
        base = baseList[0];
        mult = multList[0];
      }

      // Get a line of data from the input file
      pos = fin.tellg();
      fin.read (in, readBytes);
      
      if (!fin.good ()) {
        string msg = "Cannot read file [" + inFilename + "]. Position [" +
                     Isis::iString((int)pos) + "]. Byte count [" + 
                     Isis::iString(readBytes) + "]" ;
        throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
      }

      // Swap the bytes if necessary and convert any out of bounds pixels
      // to special pixels
      for (int samp=0; samp<ns; samp++) {
        switch (pixelType) {
          case Isis::UnsignedByte:
            out[samp] = (double) ((unsigned char *)in)[samp];
            break;
          case Isis::UnsignedWord:
            out[samp] = (double)swapper.UnsignedShortInt ((unsigned short int *)in+samp);
            break;
          case Isis::SignedWord:
            out[samp] = (double)swapper.ShortInt ((short int *)in+samp);
            break;
          case Isis::Real:
            out[samp] = (double)swapper.Float ((float *)in+samp);
            break;
          default:
            break;
        }

        // Sets out to isis special pixel or leaves it if valid
        out[samp] = TestPixel(out[samp]);

        if (Isis::IsValidPixel(out[samp])) {
          out[samp] = mult * out[samp] + base;
        }
      } // End sample loop

      //Set the buffer position and write the line to the output file
      out.SetBasePosition(1, line+1, band+1);
      outCube.Write (out);

      pos = fin.tellg();
      fin.seekg (4*(int)suffixItems[0], ios_base::cur);

      // Check the last io
      if (!fin.good ()) {
        string msg = "Cannot read file [" + inFilename + "]. Position [" +
                     Isis::iString((int)pos) + "]. Byte count [" +
                     Isis::iString(4) + "]" ;
        throw Isis::iException::Message(Isis::iException::Io,msg, _FILEINFO_);
      }
    } // End band loop

    int backplaneSize = (int)suffixItems[1] * (4*(ns + (int)suffixItems[0]));
    fin.seekg (backplaneSize, ios_base::cur);
  
    // Check the last io
    if (!fin.good ()) {
      string msg = "Cannot read file [" + inFilename + "]. Position [" +
                   Isis::iString((int)pos) + "]. Byte count [" +
                   Isis::iString(4 * (4*ns + 4)) + "]" ;
      throw Isis::iException::Message(Isis::iException::Io,msg, _FILEINFO_);
    }

  } // End line loop

  // Close the file and clean up
  fin.close ();
  outCube.Close();
  delete [] in;
}

//Function to separate the cube into two parts
void ProcessCube (Buffer &in, Buffer &out){
  for (int i=0;i<in.size();i++) {
    out[i] = in[i];
  }
}

void TranslateVisLabels (Pvl &pdsLab, Cube *viscube){
  PvlObject qube(pdsLab.FindObject("Qube"));
  //Create the Instrument Group
  PvlGroup inst("Instrument");
  inst += PvlKeyword("SpacecraftName","Cassini-Huygens");
  inst += PvlKeyword("InstrumentId",(string) qube["InstrumentId"]);
  inst += PvlKeyword("Channel","VIS");
  inst += PvlKeyword("TargetName",(string) qube["TargetName"]);
  inst += PvlKeyword("SpacecraftClockStartCount",
                     (string) qube["SpacecraftClockStartCount"]);
  inst += PvlKeyword("SpacecraftClockStopCount",
                     (string) qube["SpacecraftClockStopCount"]);
  inst += PvlKeyword("StartTime",(string) iString((string)qube["StartTime"]).Trim("Z"));
  inst += PvlKeyword("StopTime",(string) iString((string)qube["StopTime"]).Trim("Z"));
  inst += PvlKeyword("NativeStartTime",(string) qube["NativeStartTime"]);
  inst += PvlKeyword("NativeStopTime",(string) qube["NativeStopTime"]);
  inst += PvlKeyword("InterlineDelayDuration",
                     (string) qube["InterlineDelayDuration"]);
  PvlKeyword expDuration("ExposureDuration");
  expDuration.AddValue(qube["ExposureDuration"][0],"IR");
  expDuration.AddValue(qube["ExposureDuration"][1],"VIS");
  inst += expDuration;
  inst += PvlKeyword("SamplingMode",(string) qube["SamplingModeId"][1]);
  inst += PvlKeyword("XOffset",(string) qube["XOffset"]);
  inst += PvlKeyword("ZOffset",(string) qube["ZOffset"]);
  inst += PvlKeyword("SwathWidth",(string) qube["SwathWidth"]);
  inst += PvlKeyword("SwathLength",(string) qube["SwathLength"]);

  viscube->PutGroup(inst);

  //Create the BandBin Group
  PvlGroup bandbin("BandBin");
  PvlKeyword originalBand("OriginalBand");
  for (int i=1; i<=viscube->Bands(); i++) {
    originalBand.AddValue(i);
  }
  bandbin += originalBand;
  PvlKeyword center("Center");
  PvlGroup bbin(qube.FindGroup("BandBin"));
  for (int i=0; i<96; i++) {
    center += (string) bbin["BandBinCenter"][i];
  }
  bandbin += center;

  viscube->PutGroup(bandbin);

  //Create the Kernels Group
  PvlGroup kern("Kernels");
  kern += PvlKeyword("NaifFrameCode",-82370);
  viscube->PutGroup(kern);
}

void TranslateIrLabels (Pvl &pdsLab, Cube *ircube){
  PvlObject qube(pdsLab.FindObject("Qube"));
  //Create the Instrument Group
  PvlGroup inst("Instrument");
  inst += PvlKeyword("SpacecraftName","Cassini-Huygens");
  inst += PvlKeyword("InstrumentId",(string) qube["InstrumentId"]);
  inst += PvlKeyword("Channel","IR");
  inst += PvlKeyword("TargetName",(string) qube["TargetName"]);
  inst += PvlKeyword("SpacecraftClockStartCount",
                     (string) qube["SpacecraftClockStartCount"]);
  inst += PvlKeyword("SpacecraftClockStopCount",
                     (string) qube["SpacecraftClockStopCount"]);
  inst += PvlKeyword("StartTime",(string) iString((string)qube["StartTime"]).Trim("Z"));
  inst += PvlKeyword("StopTime",(string) iString((string)qube["StopTime"]).Trim("Z"));
  inst += PvlKeyword("NativeStartTime",(string) qube["NativeStartTime"]);
  inst += PvlKeyword("NativeStopTime",(string) qube["NativeStopTime"]);
  inst += PvlKeyword("InterlineDelayDuration",
                     (string) qube["InterlineDelayDuration"]);
  PvlKeyword expDuration("ExposureDuration");
  expDuration.AddValue(qube["ExposureDuration"][0],"IR");
  expDuration.AddValue(qube["ExposureDuration"][1],"VIS");
  inst += expDuration;
  inst += PvlKeyword("SamplingMode",(string) qube["SamplingModeId"][0]);
  inst += PvlKeyword("XOffset",(string) qube["XOffset"]);
  inst += PvlKeyword("ZOffset",(string) qube["ZOffset"]);
  inst += PvlKeyword("SwathWidth",(string) qube["SwathWidth"]);
  inst += PvlKeyword("SwathLength",(string) qube["SwathLength"]);

  ircube->PutGroup(inst);

  //Create the BandBin Group
  PvlGroup bandbin("BandBin");
  PvlKeyword originalBand("OriginalBand");
  for (int i=97; i<=352; i++) {
    originalBand.AddValue(i);
  }
  bandbin += originalBand;
  PvlKeyword center("Center");
  PvlGroup bbin(qube.FindGroup("BandBin"));
  for (int i=96; i<352; i++) {
    center += (string) bbin["BandBinCenter"][i];
  }
  bandbin += center;

  ircube->PutGroup(bandbin);

  //Create the Kernels Group
  PvlGroup kern("Kernels");
  kern += PvlKeyword("NaifFrameCode",-82371); 
  ircube->PutGroup(kern);
}
