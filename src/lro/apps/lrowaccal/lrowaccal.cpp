#include "Isis.h"
#include "SpecialPixel.h"
#include "CubeAttribute.h"
#include "Cube.h"
#include "Camera.h"
#include "Constants.h"
#include "Statistics.h"
#include "ProcessByBrick.h"
#include "Brick.h"

using namespace Isis;
using namespace std;

#define BW_SAMPLES 1024
#define BW_LINES 14
#define BW_BANDS 1
#define COLOR_SAMPLES 704
#define COLOR_LINES 14
#define COLOR_BANDS 5
#define UV_SAMPLES 128
#define UV_LINES 4
#define UV_BANDS 2

const double CONVIOF_UV[2] = {864.7660752105, 918.2472123715};
const double CONVIOF_COLOR[5] = {3727.1804643893, 3309.7124933295, 3377.0238719378, 3272.7518420845, 2612.1113451715};
const double CONVIOF_BW[1] = {3309.7124933295};
const double *CONVIOF;


const double CONVRAD_UV[2] = {1.663809762, 1.76670746};
const double CONVRAD_COLOR[5] = {7.171094495, 6.367886199, 6.497393279, 6.296773914, 5.025701725};
const double CONVRAD_BW[1] = {6.367886199};
const double *CONVRAD;

void Calibrate (Buffer &in, Buffer &out);
void CopyCubeIntoArray(string &fileString, double data[]);

bool dark = true,
     flatfield = true,
     radiometric = true,
     iof = false;

double exposure;     // Exposure duration
double solarDistance = 1.01; // average distance in [AU]

double darkCube[COLOR_SAMPLES*COLOR_LINES*COLOR_BANDS],
       flatCube[COLOR_SAMPLES*COLOR_LINES*COLOR_BANDS];

void IsisMain () {
  UserInterface &ui = Application::GetUserInterface();

  ProcessByBrick p;
  Cube *icube = p.SetInputCube("FROM");

  // Make sure it is a WAC cube
  Isis::PvlGroup &inst = icube->Label()->FindGroup("Instrument",Pvl::Traverse);
  iString instId = (string) inst["InstrumentId"];
  instId.UpCase();
  if (instId != "WAC-VIS" && instId != "WAC-UV") {
    string msg = "This program is intended for use on LROC WAC images only. [";
    msg += icube->Filename() + "] does not appear to be a WAC image.";
    throw iException::Message(iException::User,msg, _FILEINFO_);
  }

  dark = ui.GetBoolean("DARK");
  flatfield = ui.GetBoolean("FLATFIELD");
  radiometric = ui.GetBoolean("RADIOMETRIC");
  iof = (ui.GetString("RADIOMETRICTYPE") == "IOF");

  if (radiometric && iof) {
	  try {
		  solarDistance = (icube->Camera())->SolarDistance();
	  }
	  catch ( iException &e ) {
		  string msg = "Spiceinit must be run before converting to IOF";
		  throw iException::Message(iException::User,msg,_FILEINFO_);
	  }
  }

  // Determine the dark/flat files to use
  iString instModeId = (string) inst["InstrumentModeId"];
  instModeId.UpCase();

  if (instModeId == "COLOR" && (string)inst["InstrumentId"] == "WAC-UV")
      instModeId = "UV";

  string darkFile,flatFile;
  if (instId == "WAC-UV") {
    darkFile = "$lro/calibration/WAC_UV_Dark.????.cub";
    CopyCubeIntoArray(darkFile, darkCube);
    flatFile = "$lro/calibration/WAC_UV_Flatfield.????.cub";
    CopyCubeIntoArray(flatFile, flatCube);
  }
  else if (instModeId == "BW") {
      Isis::PvlGroup &bandBin = icube->Label()->FindGroup("BandBin",Pvl::Traverse);
      iString band = (string) bandBin["FilterName"];

      darkFile = "$lro/calibration/WAC_" + instModeId + "_" + band + "_Dark.????.cub";
      CopyCubeIntoArray(darkFile, darkCube);
      flatFile = "$lro/calibration/WAC_" + instModeId + "_" + band + "_Flatfield.????.cub";
      CopyCubeIntoArray(flatFile, flatCube);
   }
  else {
    darkFile = "$lro/calibration/WAC_" + instModeId + "_Dark.????.cub";
    CopyCubeIntoArray(darkFile, darkCube);
    flatFile = "$lro/calibration/WAC_" + instModeId + "_Flatfield.????.cub";
    CopyCubeIntoArray(flatFile, flatCube);
  }

  if (instModeId == "BW") {
	  p.SetBrickSize(BW_SAMPLES, BW_LINES, BW_BANDS);
	  CONVIOF = CONVIOF_BW;
	  CONVRAD = CONVRAD_BW;
  }
  else if (instModeId == "COLOR") {
  	  p.SetBrickSize(COLOR_SAMPLES, COLOR_LINES, COLOR_BANDS);
  	  CONVIOF = CONVIOF_COLOR;
  	CONVRAD = CONVRAD_COLOR;
  }
  else if (instModeId == "UV") {
  	  p.SetBrickSize(UV_SAMPLES, UV_LINES, UV_BANDS);
  	  CONVIOF = CONVIOF_UV;
  	  CONVRAD = CONVRAD_UV;
  }

  exposure = inst["ExposureDuration"];

  Cube *ocube = p.SetOutputCube("TO");
  p.StartProcess(Calibrate);

  // Add an output group with the appropriate information
  PvlGroup calgrp("Radiometry");
  if (dark) calgrp += PvlKeyword("DarkFile",darkFile);
  if(flatfield) calgrp += PvlKeyword("FlatFile",flatFile);
  if(radiometric) calgrp += PvlKeyword("SolarDistance",solarDistance);
  ocube->PutGroup(calgrp);

  p.EndProcess();
}

// Calibrate each framelet
void Calibrate (Buffer &inCube, Buffer &outCube) {
	for (int i=0; i< outCube.size(); i++)
		outCube[i] = inCube[i];

	if (dark) {
		for (int i=0; i< outCube.size(); i++)
			if (IsSpecial(darkCube[i]))
				outCube[i] = Isis::Null;
			else
				outCube[i] -= darkCube[i];
	}

	if (flatfield) {
		for (int i=0; i< outCube.size(); i++)
			if (flatCube[i] <= 0 || IsSpecial(flatCube[i]))
				outCube[i] = Isis::Null;
			else
				outCube[i] /= flatCube[i];
	}

	if (radiometric) {
		for (int i=0; i< outCube.size(); i++) {
		    if (IsSpecial(outCube[i]))
		        outCube[i] = Isis::Null;
		    else {
			    outCube[i] /= exposure;
			    if(iof)
			    	outCube[i] *= pow(solarDistance,2)/CONVIOF[outCube.Band()-1];
		    	else
			    	outCube[i] /= CONVRAD[outCube.Band()-1];
			 }
		}
	}

}

void CopyCubeIntoArray(string &fileString, double data[]) {
	Cube cube;
	Filename filename(fileString);
	filename.HighestVersion();
	cube.Open(filename.Expanded());
	Brick brick(cube.Samples(), cube.Lines(), cube.Bands(),cube.PixelType());
	brick.SetBasePosition(1,1,1);
	cube.Read(brick);
	for (int i=0; i<brick.size(); i++)
		data[i] = brick[i];

	fileString = filename.Expanded();
}
