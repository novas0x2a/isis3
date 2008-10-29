//  $Id: hical.cpp,v 1.7 2008/05/14 21:07:21 slambright Exp $
#include "Isis.h"

#include <cstdio>
#include <string>
#include <vector> 
#include <algorithm>
#include <sstream>
#include <iostream>

#include "Filename.h"
#include "ProcessByLine.h"
#include "UserInterface.h"
#include "Pvl.h"
#include "SpecialPixel.h"
#include "PvlGroup.h"
#include "iString.h"
#include "HiCalMatrix.h"
#include "CollectorMap.h"

using namespace Isis;
using namespace std;

//!< Define the matrix container for systematic processing
typedef CollectorMap<std::string, HiCalMatrix::HiMatrix, NoCaseStringCompare>
                     MatrixList;

//  Calibration parameter container
MatrixList *calVars = 0;

/**
 * @brief Apply calibration to each HiRISE image line
 * 
 * This function applies the calbration equation to each input image line.  It
 * gets matrices and constants from the \b calVars container that is established
 * in the main with some user input via the configuration (CONF) parameter.
 * 
 * @param in  Input raw image line buffer
 * @param out Output calibrated image line buffer
 */
void calibrate(Buffer &in, Buffer &out) {
  const HiCalMatrix::HiMatrix &A = calVars->get("A");
  const HiCalMatrix::HiMatrix &B = calVars->get("B");
  const HiCalMatrix::HiMatrix &G = calVars->get("G");
  double scanExpDuration = (calVars->get("ScanExposureDuration"))[0];
  double IFCorrection = (calVars->get("IFCorrection"))[0];

  for (int i = 0 ; i < in.size() ; i++) {
    if (IsSpecial(in[i])) {
      out[i] = in[i];
    }
    else {
      out[i] = ((in[i]/scanExpDuration - B[i]) * A[i]) * G[i] / IFCorrection;
    }
  }
  return;
}


void IsisMain(){

  const std::string hical_version = "1.0";
  const std::string hical_revision = "$Revision: 1.7 $";

  UserInterface &ui = Application::GetUserInterface();
  Filename from(ui.GetFilename("FROM"));
  string conf(ui.GetAsString("CONF"));

// Run hiclean2
  string hc2Name = from.Basename() + "_hiclean2";
  Filename tempcal;
  tempcal.Temporary(hc2Name, "cub");
  hc2Name = tempcal.Expanded();
  string params = "from=" + from.Expanded()  + " to=" + hc2Name;

  try {
    iApp->Exec("hiclean2", params);
  } catch (iException &ie ) {
    remove (hc2Name.c_str());
    ie.Message(iException::User,"Failed to execute hiclean2", _FILEINFO_);
    throw;
  }

//  This section processes the hiclean2 output by applying the HipiCal part
  try {

//  The output from the last processing is the input into subsequent processing
    ProcessByLine p;

    CubeAttributeInput att;
    Cube *icube = p.SetInputCube(hc2Name, att);
    Cube *ocube = p.SetOutputCube("TO");
    int nsamps = icube->Samples();

//  Initialize the configuration file
    HiCalMatrix matrices(*(icube->Label()), conf);

//  Set specified profile if entered by user
    if (ui.WasEntered("PROFILE")) {
      matrices.selectProfile(ui.GetAsString("PROFILE"));
    }

    //  Allocate the calibration list
    calVars = new MatrixList;

//  Load all matrix calibrations
    vector<string> calList = matrices.getMatrixList();
    for (unsigned int i = 0 ; i < calList.size() ; i++) {
      calVars->add(calList[i], matrices.getMatrix(calList[i], nsamps));
    }

    calList = matrices.getScalarList();
    for (unsigned int i = 0 ; i < calList.size() ; i++) {
      calVars->add(calList[i], matrices.getScalar(calList[i], 1));
    }

    calList = matrices.getKeywordList();
    for (unsigned int i = 0 ; i < calList.size() ; i++) {
      calVars->add(calList[i], matrices.getKeyword(calList[i], 1));
    }

//  Add solar I/F correction parameters
    double au = matrices.sunDistanceAU();
    double suncorr =  1.5 / au;
    suncorr *= suncorr;
    double ifCorrection = ((calVars->get("IFTdiBinFactor"))[0]) *
                          ((calVars->get("FilterGainCorrection"))[0]) *
                          (suncorr * 1.e-6);
    calVars->add("IFCorrection", HiCalMatrix::HiMatrix(1, ifCorrection));

//  Call the processing function
    p.StartProcess(calibrate);

//  Ensure the RadiometricCalibration group is out there
    const std::string rcalGroup("RadiometricCalibration");
    if (!ocube->HasGroup(rcalGroup)) {
      PvlGroup temp(rcalGroup);
      ocube->PutGroup(temp);
    }

    PvlGroup &rcal = ocube->GetGroup(rcalGroup);
    PvlKeyword key("ProgramName", "hical");
    key.AddCommentWrapped("/* Hical application parameters */");
    key.AddComment("/* odn = ((idn/ScanExposureDuration - B) * A) * G / IFCorrection */");
    rcal += key;

    rcal += PvlKeyword("HicalVersion",hical_version);
    rcal += PvlKeyword("HicalRevision",hical_revision);
    rcal += PvlKeyword("HicalConfiguration", matrices.filepath(conf));
    rcal += PvlKeyword("Profile", matrices.getProfileName());

    //  Add band information
    iString calBand = matrices.getMatrixBand();
    calList = matrices.getMatrixList();
    for (unsigned int i = 0 ; i < calList.size() ; i++) {
      string mtxFile = matrices.getMatrixSource(calList[i]) + "+" + calBand;
      rcal += PvlKeyword(calList[i], mtxFile);
    }

    // Add Scalar Information
    calList = matrices.getScalarList();
    for (unsigned int i = 0 ; i < calList.size() ; i++) {
      rcal += PvlKeyword(calList[i], matrices.getScalar(calList[i])[0]);
    }

// Add keywords used
    calList = matrices.getKeywordList();
    for (unsigned int i = 0 ; i < calList.size() ; i++) {
      rcal += PvlKeyword(calList[i], matrices.getKeyword(calList[i])[0]);
    }

    //  Add constants as computed in hical
    rcal += PvlKeyword("SunDistance", au, "AU");
    rcal += PvlKeyword("SolarConstant", suncorr);
    rcal += PvlKeyword("IFCorrection", ifCorrection);

    p.EndProcess();

  } 
  catch (iException &ie) {
    remove (hc2Name.c_str());
    delete calVars;
    calVars = 0;
    throw;
  }
  
// Always remove the temp hiclean2 file
  remove (hc2Name.c_str());
  delete calVars;
  calVars = 0;
}

