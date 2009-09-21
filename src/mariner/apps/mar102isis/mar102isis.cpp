//************************************************************************
// See Full documentation in raw2isis.xml
//************************************************************************
#include "Isis.h"

#include "Cube.h"
#include "Filename.h"
#include "iString.h"
#include "iTime.h"
#include "PixelType.h"
#include "ProcessImport.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "PvlKeyword.h"

#include <sstream>
#include <string>

using namespace std;
using namespace Isis;

string ebcdic2ascii (unsigned char *header);
void UpdateLabels (Cube *cube, const string &header);

void IsisMain () {
  ProcessImport p;

  // All mariner images from both cameras share this size
  p.SetDimensions(832,700,1);
  p.SetFileHeaderBytes(968);
  p.SaveFileHeader();
  p.SetPixelType(UnsignedByte);
  p.SetByteOrder(Lsb);
  p.SetDataSuffixBytes(136);

  UserInterface &ui = Application::GetUserInterface();
  p.SetInputFile (ui.GetFilename("FROM"));
  Cube *cube = p.SetOutputCube("TO");

  p.StartProcess ();
  unsigned char *header = (unsigned char *) p.FileHeader();
  string labels = ebcdic2ascii(header);

  UpdateLabels(cube, labels);

  p.EndProcess ();
}

//! Converts labels into standard pvl format and adds necessary 
//! information not included in original labels
void UpdateLabels (Cube *cube, const string &labels) {
  // Convert string to stream
  stringstream os;
  os << labels;
  // Ingest stream as Pvl
  Pvl pvl;
  os >> pvl;

  // Create the instrument group
  PvlGroup inst("Instrument");
  inst += PvlKeyword("SpacecraftName","Mariner10");
  inst += PvlKeyword("InstrumentId","Camera"+(string)pvl["CCAMERA"]);

  // Get the date
  int year = pvl["YR"];
  year += 1900;
  int jday = pvl["DAY"];
  iString date = iString(year) + "-" + iString(jday);

  // Get the time
  iString time = (string) pvl["GMT"];
  time = time.Replace("/",":") + ".000";

  // Construct the Start Time in yyyy-mm-ddThh:mm:ss format
  string fulltime = date + "T" + time;
  iTime startTime(fulltime);
  inst += PvlKeyword("StartTime",startTime.UTC());

  // Get exposure duration
  inst += PvlKeyword("ExposureDuration",(string)pvl["EXPOSURE"],"milliseconds");

  // Create the band bin group
  PvlGroup bandBin("BandBin");
  iString filter = (string)pvl["FILTER"];
  iString number = filter.Token("(");
  bandBin += PvlKeyword("FilterNumber",number);
  filter = filter.Replace(")","");
  bandBin += PvlKeyword("FilterName",filter);

  // Dates taken from ASU Mariner website - http://ser.ses.asu.edu/M10/TXT/encounters.html and
  // from Nasa website - http://www.jpl.nasa.gov/missions/missiondetails.cfm?mission_Mariner10
  // under fast facts. Mariner encountered the Moon, Venus, and Mercury three times.
  // Date used for all is two days before date of first encounter on website. Information
  // is important for nominal reseaus used and for Keyword encounter  
  string encounter = "";
  if (startTime < iTime("1974-2-3T12:00:00")) { 
    encounter = "$mariner10/reseaus/mar10MoonNominal.pvl";
    inst += PvlKeyword("Encounter", "Moon");
  } 
  //Disagreement on the below date between ASU website and NASA website, used earlier of the two - ASU
  else if (startTime < iTime("1974-3-22T12:00:00")) { 
    encounter = "$mariner10/reseaus/mar10VenusNominal.pvl";
    inst += PvlKeyword("Encounter", "Venus");
  }
  else if (startTime < iTime("1974-9-19T12:00:00")) {
    encounter = "$mariner10/reseaus/mar10Merc1Nominal.pvl";
    inst += PvlKeyword("Encounter", "Mercury_1");
  }
  // No specific date on ASU website, used NASA date
  else if (startTime < iTime("1975-3-14T12:00:00")) {
    encounter = "$mariner10/reseaus/mar10Merc2Nominal.pvl";
    inst += PvlKeyword("Encounter", "Mercury_2");
  }
  else {
    encounter = "$mariner10/reseaus/mar10Merc3Nominal.pvl";
    inst += PvlKeyword("Encounter", "Mercury_3");
  }
  
  // Open nominal positions pvl named by string encounter
  Pvl nomRx(encounter);

  // Allocate all keywords within reseaus groups well as the group its self
  PvlGroup rx("Reseaus");
  PvlKeyword line("Line");
  PvlKeyword sample("Sample");
  PvlKeyword type("Type");
  PvlKeyword valid("Valid");
  PvlKeyword templ("Template");
  PvlKeyword status("Status");  

  // All cubes will stay this way until findrx is run on them
  status = "Nominal";

  // Kernels group
  PvlGroup kernels("Kernels");
  PvlKeyword naif("NaifFrameCode");
  
  // Camera dependent information
  string camera = "";
  if (iString("CameraA") == inst["InstrumentId"][0]) {
    templ = "$mariner10/reseaus/mar10a.template.cub";
    naif += "-76110"; 
    camera = "M10_VIDICON_A_RESEAUS";
  } 
  else {
    templ = "$mariner10/reseaus/mar10b.template.cub";
    naif += "-76120";
    camera = "M10_VIDICON_B_RESEAUS";
  }

  // Add naif frame code
  kernels += naif;
  
  // Find the correct PvlKeyword corresponding to the camera for nominal positions
  PvlKeyword resnom = nomRx[camera];
  
  // This loop goes through the PvlKeyword resnom which contains data 
  // in the format: line, sample, type, on each line. There are 111 reseaus for 
  // both cameras. To put data away correctly, it must go through a total 333 items, 
  // all in one PvlKeyword.  
  int i = 0;
  while (i < 333) {
    line += resnom[i];
    i++;
    sample += resnom[i];
    i++;
    type += resnom[i];
    i++;
    valid += 0;
  }

  // Add all the PvlKeywords to the PvlGroup Reseaus
  rx += line;
  rx += sample;
  rx += type;
  rx += valid;
  rx += templ;
  rx += status;

  // Get the labels and add the updated labels to them
  Pvl *cubeLabels = cube->Label();
  cubeLabels->FindObject("IsisCube").AddGroup(inst);
  cubeLabels->FindObject("IsisCube").AddGroup(bandBin);
  cubeLabels->FindObject("IsisCube").AddGroup(kernels);
  cubeLabels->FindObject("IsisCube").AddGroup(rx);
}

// FYI, mariner10 original labels are stored in ebcdic, a competitor with ascii,
// a conversion table is necessary then to get the characters over to ascii. For
// more info: http://en.wikipedia.org/wiki/Extended_Binary_Coded_Decimal_Interchange_Code
//! Converts ebsidic Mariner10 labels to ascii
string ebcdic2ascii (unsigned char *header) {
  // Table to convert ebcdic to ascii
  unsigned char xlate[] = {
    0x00,0x01,0x02,0x03,0x9C,0x09,0x86,0x7F,0x97,0x8D,0x8E,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x9D,0x85,0x08,0x87,0x18,0x19,0x92,0x8F,0x1C,0x1D,0x1E,0x1F,
    0x80,0x81,0x82,0x83,0x84,0x0A,0x17,0x1B,0x88,0x89,0x8A,0x8B,0x8C,0x05,0x06,0x07,
    0x90,0x91,0x16,0x93,0x94,0x95,0x96,0x04,0x98,0x99,0x9A,0x9B,0x14,0x15,0x9E,0x1A,
    0x20,0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xD5,0x2E,0x3C,0x28,0x2B,0x7C,
    0x26,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0x21,0x24,0x2A,0x29,0x3B,0x5E,
    0x2D,0x2F,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xE5,0x2C,0x25,0x5F,0x3E,0x3F,
    0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xC0,0xC1,0xC2,0x60,0x3A,0x23,0x40,0x27,0x3D,0x22,
    0xC3,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,
    0xCA,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,
    0xD1,0x7E,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0xD2,0xD3,0xD4,0x5B,0xD6,0xD7,
    0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,0xE0,0xE1,0xE2,0xE3,0xE4,0x5D,0xE6,0xE7,
    0x7B,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,
    0x7D,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,0x51,0x52,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,
    0x5C,0x9F,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
  };

  // Mariner 10 has 360 bytes of header information
  for (int i=0; i<360; i++) {
    header[i] = xlate[header[i]];
  }

  // Put in a end of string mark and return
  header[215] = 0;
  return string((const char *)header);
}

