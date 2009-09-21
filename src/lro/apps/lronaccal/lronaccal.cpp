#include "Isis.h"
#include "ProcessByLine.h"
#include "SpecialPixel.h"
#include "Camera.h"
#include "iTime.h"
#include "iException.h"
#include "TextFile.h"
#include "LineManager.h"
#include "Brick.h"
#include "Table.h"
#include "Statistics.h"
#include <fstream>
#include <vector>

using namespace std;
using namespace Isis;

// Working functions and parameters
void ResetGlobals();
void CopyCubeIntoArray ( string &fileString, vector<double> &data );
void ReadTextDataFile ( string fileString, vector<double> &data );
void ReadTextDataFile ( string fileString, vector<vector<double> > &data );
void Calibrate ( Buffer &in, Buffer &out );
void RemoveMaskedOffset ( Buffer &line );
void CorrectDark ( Buffer &in );
void CorrectNonlinearity ( Buffer &in );
void CorrectFlatfield ( Buffer &in );
void RadiometricCalibration ( Buffer &in );

#define LINE_SIZE 5064
#define MAXNONLIN 600
#define CONVRAD_LEFT 18.056
#define CONVRAD_RIGHT 16.683
#define CONVIOF_LEFT 9426.5
#define CONVIOF_RIGHT 8632.3
#define SOLAR_RADIUS 695500
#define NUM_MASKED_PIXELS 24
#define NUM_MASKED_LEFT_PIXELS 15

const int ODD_MASKED_PIXELS[] = { 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 5045, 5047, 5049, 5051, 5053, 5055, 5057,
        5059, 5061 };
const int EVEN_MASKED_PIXELS[] = { 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 5044, 5046, 5048, 5050, 5052, 5054, 5056,
        5058, 5060 };
const int SUMMED_MASKED_PIXELS[] = { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 2522, 2523, 2524, 2525, 2526, 2527, 2528,
        2529, 2530 };

double g_exposure; // Exposure duration
double g_solarDistance; // average distance in [AU]

bool g_summed, g_masked, g_maskedLeft, g_dark, g_nonlinear, g_flatfield, g_withoutBumps, g_radiometric, g_iof, g_isLeftNac;

vector<double> g_averageDarkLine, g_linearOffsetLine, g_flatfieldLine;

vector<vector<double> > g_linearityCoefficients;

// Main moccal routine
void IsisMain () {
    ResetGlobals();
    // We will be processing by line
    ProcessByLine p;

    // Setup the input and make sure it is a ctx file
    UserInterface &ui = Application::GetUserInterface();

    g_masked = ui.GetBoolean("MASKED");
    g_maskedLeft = (ui.GetString("MASKEDTYPE") == "LEFT");
    g_dark = ui.GetBoolean("DARK");
    g_nonlinear = ui.GetBoolean("NONLINEARITY");
    g_flatfield = ui.GetBoolean("FLATFIELD");
    g_withoutBumps = (ui.GetString("FLATFIELDTYPE") == "WITHOUTBUMPS");
    g_radiometric = ui.GetBoolean("RADIOMETRIC");
    g_iof = (ui.GetString("RADIOMETRICTYPE") == "IOF");

    Isis::Pvl lab(ui.GetFilename("FROM"));
    Isis::PvlGroup &inst = lab.FindGroup("Instrument", Pvl::Traverse);

    std::string instId = inst["InstrumentId"];
    if (instId != "NACL" && instId != "NACR") {
        string msg = "This is not a NAC image.  lrocnaccal requires a NAC image.";
        throw iException::Message(iException::User, msg, _FILEINFO_);
    }
    else if (instId == "NACL")
        g_isLeftNac = true;
    else
        g_isLeftNac = false;

    if ((int) inst["SpatialSumming"] == 1)
        g_summed = false;
    else
        g_summed = true;

    g_exposure = inst["LineExposureDuration"];

    Cube *icube = p.SetInputCube("FROM", OneBand);

    string darkFile, flatFile, offsetFile, coefficientFile;

    if (g_dark) {
        darkFile = "$lro/calibration/" + instId + "_AverageDarks";
        if (g_summed)
            darkFile += "_Summed";
        darkFile += ".????.cub";
        CopyCubeIntoArray(darkFile, g_averageDarkLine);
    }
    if (g_nonlinear) {
        offsetFile = "$lro/calibration/" + instId + "_LinearizationOffsets";
        if (g_summed)
            offsetFile += "_Summed";
        offsetFile += ".????.cub";
        CopyCubeIntoArray(offsetFile, g_linearOffsetLine);
        coefficientFile = "$lro/calibration/" + instId + "_LinearizationCoefficients.????.txt";
        ReadTextDataFile(coefficientFile, g_linearityCoefficients);
    }
    if (g_flatfield) {
        flatFile = "$lro/calibration/" + instId + "_Flatfield";
        if (g_withoutBumps)
            flatFile += "_WithoutBumps";
        if (g_summed)
            flatFile += "_Summed";
        flatFile += ".????.cub";
        CopyCubeIntoArray(flatFile, g_flatfieldLine);
    }
    if (g_radiometric) {
        if (g_iof) {
            try {
                g_solarDistance = (icube->Camera())->SolarDistance();
            }
            catch (iException &e) {
                string msg = "Spiceinit must be run before converting to IOF";
                throw iException::Message(iException::User, msg, _FILEINFO_);
            }
        }
    }

    // Setup the output cube
    Cube *ocube = p.SetOutputCube("TO");

    // Start the line-by-line calibration sequence
    p.StartProcess(Calibrate);
    PvlGroup calgrp("Radiometry");
    if (g_dark)
        calgrp += PvlKeyword("DarkFile", darkFile);
    if (g_flatfield)
        calgrp += PvlKeyword("FlatFile", flatFile);
    if (g_radiometric)
        calgrp += PvlKeyword("SolarDistance", g_solarDistance);
    if (g_nonlinear) {
        calgrp += PvlKeyword("NonlinearOffset", offsetFile);
        calgrp += PvlKeyword("LinearizationCoefficients", coefficientFile);
    }
    ocube->PutGroup(calgrp);
    p.EndProcess();
}

void ResetGlobals() {
    g_exposure = 1.0; // Exposure duration
    g_solarDistance = 1.01; // average distance in [AU]

    g_summed = true;
    g_masked = true;
    g_dark = true;
    g_nonlinear = true;
    g_flatfield = true;
    g_withoutBumps = true;
    g_radiometric = true;
    g_iof = true;
    g_isLeftNac = true;
    g_averageDarkLine.clear();
    g_linearOffsetLine.clear();
    g_flatfieldLine.clear();
    g_linearityCoefficients.clear();
}

// Line processing routine
void Calibrate ( Buffer &in, Buffer &out ) {
    for (int i = 0; i < in.size(); i++)
        out[i] = in[i];

    if (g_masked)
        RemoveMaskedOffset(out);

    if (g_dark)
        CorrectDark(out);

    if (g_nonlinear)
        CorrectNonlinearity(out);

    if (g_flatfield)
        CorrectFlatfield(out);

    if (g_radiometric)
        RadiometricCalibration(out);
}

void CopyCubeIntoArray ( string &fileString, vector<double> &data ) {
    Cube cube;
    Filename filename(fileString);
    filename.HighestVersion();
    cube.Open(filename.Expanded());
    Brick brick(cube.Samples(), cube.Lines(), cube.Bands(), cube.PixelType());
    brick.SetBasePosition(1, 1, 1);
    cube.Read(brick);
    data.clear();
    for (int i = 0; i < cube.Samples(); i++)
        data.push_back(brick[i]);

    fileString = filename.Expanded();
}

void ReadTextDataFile ( string fileString, vector<double> &data ) {
    Filename filename(fileString);
    filename.HighestVersion();
    TextFile file(filename.Expanded());
    iString lineString;
    unsigned int line = 0;
    while (file.GetLine(lineString)) {
        data.push_back((lineString.Token(" ,;")).ToDouble());
        line++;
    }
}

void ReadTextDataFile ( string fileString, vector<vector<double> > &data ) {
    Filename filename(fileString);
    filename.HighestVersion();
    TextFile file(filename.Expanded());
    iString lineString;
    while (file.GetLine(lineString)) {
        vector<double> line;
        lineString.ConvertWhiteSpace();
        lineString.Compress();
        lineString.TrimHead(" ,");
        while (lineString.size() > 0) {
            line.push_back((lineString.Token(" ,")).ToDouble());
        }

        data.push_back(line);
    }
}

void RemoveMaskedOffset ( Buffer &in ) {
    unsigned int numberOfMaskedPixels;
    if (g_maskedLeft)
        numberOfMaskedPixels = NUM_MASKED_LEFT_PIXELS;
    else
        numberOfMaskedPixels = NUM_MASKED_PIXELS;

    if (g_summed) {
        Statistics stats;
        for (unsigned int i = 0; i < numberOfMaskedPixels; i++)
            if (g_isLeftNac)
                stats.AddData(in[SUMMED_MASKED_PIXELS[i]]);
            else
                // if it is a right frame, flip it
                stats.AddData(in[LINE_SIZE/2 - 1 - SUMMED_MASKED_PIXELS[i]]);

        double mean = stats.Average();

        for (int i = 0; i < in.size(); i++)
            in[i] -= mean;
    }
    // Even and Odd seperately
    else {

        Statistics evenStats, oddStats;

        for (unsigned int i = 0; i < numberOfMaskedPixels; i++) {
            if (g_isLeftNac) {
                evenStats.AddData(in[EVEN_MASKED_PIXELS[i]]);
                oddStats.AddData(in[ODD_MASKED_PIXELS[i]]);
            }
            else {
                evenStats.AddData(in[LINE_SIZE - 1 - EVEN_MASKED_PIXELS[i]]);
                oddStats.AddData(in[LINE_SIZE - 1 - ODD_MASKED_PIXELS[i]]);
            }
        }

        double evenMean = evenStats.Average();
        double oddMean = oddStats.Average();

        for (int i = 0; i < in.size(); i++)
            if (i % 2 == 0)
                in[i] -= evenMean;
            else
                in[i] -= oddMean;
    }
}

void CorrectDark ( Buffer &in ) {
    for (int i = 0; i < in.size(); i++) {
        if (!IsSpecial(in[i]))
            in[i] -= g_averageDarkLine[i];
        else
            in[i] = Isis::Null;
    }
}

void CorrectNonlinearity ( Buffer &in ) {
    for (int i = 0; i < in.size(); i++) {
        if (!IsSpecial(in[i])) {
            in[i] += g_linearOffsetLine[i];

            if (in[i] < MAXNONLIN) {
                if (g_summed)
                    in[i] -= (1.0 / (g_linearityCoefficients[2* i ][0] * pow(g_linearityCoefficients[2* i ][1], in[i])
                            + g_linearityCoefficients[2* i ][2]) + 1.0 / (g_linearityCoefficients[2* i + 1][0] * pow(
                            g_linearityCoefficients[2* i + 1][1], in[i]) + g_linearityCoefficients[2* i + 1][2])) / 2;
                else
                    in[i] -= 1.0 / (g_linearityCoefficients[i][0] * pow(g_linearityCoefficients[i][1], in[i])
                            + g_linearityCoefficients[i][2]);
            }
        }
        else
            in[i] = Isis::Null;
    }
}

void CorrectFlatfield ( Buffer &in ) {
    for (int i = 0; i < in.size(); i++) {
        if (!IsSpecial(in[i]) && g_flatfieldLine[i] > 0)
            in[i] /= g_flatfieldLine[i];
        else
            in[i] = Isis::Null;
    }
}

void RadiometricCalibration ( Buffer &in ) {
    for (int i = 0; i < in.size(); i++) {
        if (!IsSpecial(in[i])) {
            in[i] /= g_exposure;
            if (g_iof) {
                if (g_isLeftNac)
                    in[i] = in[i] * pow(g_solarDistance, 2) / CONVIOF_LEFT;
                else
                    in[i] = in[i] * pow(g_solarDistance, 2) / CONVIOF_RIGHT;
            }
            else {
                if (g_isLeftNac)
                    in[i] = in[i] / CONVRAD_LEFT;
                else
                    in[i] = in[i] / CONVRAD_RIGHT;
            }
        }
        else
            in[i] = Isis::Null;
    }
}
