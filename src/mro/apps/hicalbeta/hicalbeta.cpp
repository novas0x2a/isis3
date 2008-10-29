//  $Id: hicalbeta.cpp,v 1.12 2008/07/15 15:37:08 skoechle Exp $
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
#include "PvlGroup.h"
#include "iString.h"
#include "HiCalConf.h"
#include "CollectorMap.h"
#include "HiCalTypes.h"
#include "HiCalUtil.h"
#include "HiCalData.h"
#include "SplineFillComp.h"
#include "LowPassFilterComp.h"
#include "DriftBuffer.h"
#include "DriftCorrect.h"
#include "OffsetCorrect.h"
#include "DarkSubtractComp.h"
#include "GainVLineComp.h"
#include "FlatFieldComp.h"


using namespace Isis;
using namespace std;

//!< Define the matrix container for systematic processing
typedef CollectorMap<std::string, HiVector, NoCaseStringCompare> MatrixList;

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
  const HiVector &Zd    = calVars->get("Zd");
  const HiVector &Zz    = calVars->get("Zz");
  const HiVector &Zb    = calVars->get("Zb");
  const HiVector &Zg    = calVars->get("Zg");
  const HiVector &Zgg   = calVars->get("Zgg");
  const HiVector &Za    = calVars->get("Za");
  const HiVector &Ziof  = calVars->get("Ziof");
  const double   sed    = (calVars->get("ScanExposureDuration"))[0];

  //  Set current line (index)
  int line(in.Line()-1);
  if (calVars->exists("LastGoodLine")) {
    int lastline = ((int) (calVars->get("LastGoodLine"))[0]) - 1;
    if ( line > lastline ) { line = lastline; }
  }

  //  Apply correction
  for (int i = 0 ; i < in.size() ; i++) {
    if (IsSpecial(in[i])) {
      out[i] = in[i];
    }
    else {
      double hdn;
      hdn = (in[i] - Zd[line] - Zz[i] - Zb[i]) / sed; // Drift, Offset, Dark
      hdn = hdn / Zg[line] * Zgg[i] * Za[i];    // GainVLine, Gain, FlatField
      out[i] = hdn / Ziof[0];                   // I/F
    }
  }
  return;
}


void IsisMain(){

  const std::string hical_program = "hicalbeta";
  const std::string hical_version = "3.3";
  const std::string hical_revision = "$Revision: 1.12 $";
  const std::string hical_runtime = Application::DateTime();

  UserInterface &ui = Application::GetUserInterface();

  string procStep("prepping phase");
  try {
//  The output from the last processing is the input into subsequent processing
    ProcessByLine p;

    Cube *hifrom = p.SetInputCube("FROM");
    int nsamps = hifrom->Samples();
    int nlines = hifrom->Lines();

//  Initialize the configuration file
    string conf(ui.GetAsString("CONF"));
    HiCalConf hiconf(*(hifrom->Label()), conf);
    DbProfile hiprof = hiconf.getMatrixProfile();

// Check for label propagation and set the output cube
    Cube *ocube = p.SetOutputCube("TO");
    if ( !IsTrueValue(hiprof,"PropagateTables", "TRUE") ) {
      RemoveHiBlobs(*(ocube->Label()));
    }

//  Set specified profile if entered by user
    if (ui.WasEntered("PROFILE")) {
      hiconf.selectProfile(ui.GetAsString("PROFILE"));
    }


//  Add OPATH parameter to profiles
    if (ui.WasEntered("OPATH")) {
      hiconf.add("OPATH",ui.GetAsString("OPATH"));
    }
    else {
      hiconf.add("OPATH",".");
    }

//  Do I/F output DN conversions
    bool doIOF = ui.GetBoolean("IOF");

    //  Allocate the calibration list
    calVars = new MatrixList;

//  Set up access to HiRISE ancillary data (tables, blobs) here.  Note it they
//  are gone, this will error out. See PropagateTables in conf file.
    HiCalData caldata(*hifrom);

////////////////////////////////////////////////////////////////////////////
//  FixGaps (Z_f) Get buffer pixels and compute coefficients for equation
//     y = a[0] + a[1]*x + a[2] * exp(a[3] * x)
//        where y is the average of the buffer pixel region,
//          and x is the time at each line in electrons/sec/pixel
    procStep = "Zf module";
    hiconf.selectProfile("Zf");
    hiprof = hiconf.getMatrixProfile();
    HiHistory ZfHist;
    ZfHist.add("Profile["+ hiprof.Name()+"]");
    if ( !SkipModule(hiprof) ) {
      DriftBuffer driftB(caldata, hiconf);
      calVars->add("Zf", driftB.ref());
      ZfHist = driftB.History();
      if ( hiprof.exists("DumpModuleFile") ) {
        driftB.Dump(hiconf.getMatrixSource("DumpModuleFile",hiprof));
      }
    }
    else {
      //  NOT RECOMMENDED!  This is required for the next step!
      //  SURELY must be skipped with Z_d step as well!
      calVars->add("Zf", HiVector(nlines, 0.0));
      ZfHist.add("Debug::SkipModule invoked!");
    }

/////////////////////////////////////////////////////////////////////
// DriftCorrect (Z_d)
//  Now compute the equation of fit
// 
    procStep = "Zd module";
    HiHistory ZdHist;
    hiconf.selectProfile("Zd");
    hiprof = hiconf.getMatrixProfile();
    ZdHist.add("Profile["+ hiprof.Name()+"]");
    if (!SkipModule(hiconf.getMatrixProfile("Zd")) ) { 
      DriftCorrect driftC(hiconf);
      calVars->add("Zd", driftC.Normalize(driftC.Solve(calVars->get("Zf"))));
      ZdHist = driftC.History();
      if ( hiprof.exists("DumpModuleFile") ) {
        driftC.Dump(hiconf.getMatrixSource("DumpModuleFile",hiprof));
      }
    }
    else {
      calVars->add("Zd", HiVector(nlines, 0.0));
      ZdHist.add("Debug::SkipModule invoked!");
    }


 ////////////////////////////////////////////////////////////////////
 //  ZeroCorrect (Z_z)  Get reverse clock 
    procStep = "Zz module";
    hiconf.selectProfile("Zz"); 
    hiprof = hiconf.getMatrixProfile();
    HiHistory ZzHist;
    ZzHist.add("Profile["+ hiprof.Name()+"]");
    if ( !SkipModule(hiprof) ) {
      OffsetCorrect zoff(caldata, hiconf);
      calVars->add("Zz", zoff.ref());
      ZzHist = zoff.History();
      if ( hiprof.exists("DumpModuleFile") ) {
        zoff.Dump(hiconf.getMatrixSource("DumpModuleFile",hiprof));
      }
    }
    else {
      calVars->add("Zz", HiVector(nsamps, 0.0));
      ZzHist.add("Debug::SkipModule invoked!");
    }

/////////////////////////////////////////////////////////////////
// DarkSubtract (Z_b) Remove dark current
// 
    procStep = "Zb module";
    hiconf.selectProfile("Zb");
    hiprof =  hiconf.getMatrixProfile();
    HiHistory ZbHist;
    ZbHist.add("Profile["+ hiprof.Name()+"]");
    if ( !SkipModule(hiprof) ) {
      DarkSubtractComp dark(hiconf);
      calVars->add("Zb", dark.ref());
      ZbHist = dark.History();
      if ( hiprof.exists("DumpModuleFile") ) {
        dark.Dump(hiconf.getMatrixSource("DumpModuleFile",hiprof));
      }
    }
    else {
      calVars->add("Zb", HiVector(nsamps, 0.0));
      ZbHist.add("Debug::SkipModule invoked!");
    }

////////////////////////////////////////////////////////////////////
// GainVLineCorrect (Z_g) Correct for gain-based drift
// 
    procStep = "Zg module";
    hiconf.selectProfile("Zg");
    hiprof = hiconf.getMatrixProfile();
    HiHistory ZgHist;
    ZgHist.add("Profile["+ hiprof.Name()+"]");
    if ( !SkipModule(hiprof) ) {
      GainVLineComp gainV(hiconf);
      calVars->add("Zg", gainV.ref());
      ZgHist = gainV.History();
      if ( hiprof.exists("DumpModuleFile") ) {
        gainV.Dump(hiconf.getMatrixSource("DumpModuleFile",hiprof));
      }
    }
    else {
      calVars->add("Zg", HiVector(nlines, 1.0));
      ZgHist.add("Debug::SkipModule invoked!");
    }


////////////////////////////////////////////////////////////////////
//  GainCorrect (Z_gg)  Correct for gain with the G matrix 
    procStep = "Zgg module";
    hiconf.selectProfile("Zgg"); 
    hiprof =  hiconf.getMatrixProfile();
    HiHistory ZggHist;
    ZggHist.add("Profile["+ hiprof.Name()+"]");
    if ( !SkipModule(hiprof) ) {
      double bin = ToDouble(hiprof("Summing"));
      double tdi = ToDouble(hiprof("Tdi"));
      double factor = 128.0 / tdi / (bin*bin);
      HiVector zgg =  hiconf.getMatrix("G", hiprof);
      for ( int i = 0 ; i < zgg.dim() ; i++ ) { zgg[i] *= factor; }
      calVars->add("Zgg", zgg);;
      ZggHist.add("LoadMatrix(G[" + hiconf.getMatrixSource("G",hiprof) +
                  "],Band[" + ToString(hiconf.getMatrixBand(hiprof)) + 
                  "],Factor[" + ToString(factor) + "])"); 
      if ( hiprof.exists("DumpModuleFile") ) { 
        Component zg("GMatrix", ZggHist);
        zg.Process(zgg);
        zg.Dump(hiconf.getMatrixSource("DumpModuleFile",hiprof));
      }
    }
    else {
      calVars->add("Zgg", HiVector(nsamps, 1.0));
      ZggHist.add("Debug::SkipModule invoked!");
    }

////////////////////////////////////////////////////////////////////
//  FlatField (Z_a)  Flat field correction with A matrix
    procStep = "Za module";
    hiconf.selectProfile("Za"); 
    hiprof =  hiconf.getMatrixProfile();
    HiHistory ZaHist;
    ZaHist.add("Profile["+ hiprof.Name()+"]");
    if ( !SkipModule(hiprof) ) {
      FlatFieldComp flat(hiconf);
      calVars->add("Za", flat.ref());
      ZaHist = flat.History();
      if ( hiprof.exists("DumpModuleFile") ) {
        flat.Dump(hiconf.getMatrixSource("DumpModuleFile",hiprof));
      }
    }
    else {
      calVars->add("Za", HiVector(nsamps, 1.0));
      ZaHist.add("Debug::SkipModule invoked!");
    }


////////////////////////////////////////////////////////////////////
//  I/FCorrect (Z_iof) Conversion to I/F
// 
    procStep = "Ziof module";
    hiconf.selectProfile("Ziof");
    hiprof = hiconf.getMatrixProfile();
    HiHistory ZiofHist;
    ZiofHist.add("Profile["+ hiprof.Name()+"]");
    if ( !SkipModule(hiprof) ) {
      if ( doIOF ) {
  //  Add solar I/F correction parameters
        double au = hiconf.sunDistanceAU();
        ZiofHist.add("SunDist[" + ToString(au) + " (AU)]");
        double suncorr =  1.5 / au;
        suncorr *= suncorr;

        double zbin  = ToDouble(hiprof("ZiofBinFactor"));
        ZiofHist.add("ZiofBinFactor[" + ToString(zbin) + "]");

        double zgain = ToDouble(hiprof("FilterGainCorrection"));
        ZiofHist.add("FilterGainCorrection[" + ToString(zgain) + "]");

        double ziof = (zbin * zgain) * (suncorr * 1.e-6); 

        calVars->add("Ziof", HiVector(1, ziof));
        ZiofHist.add("I/F_Factor[" + ToString(ziof) + "]");
        ZiofHist.add("Units[I/F Reflectance]");
        calVars->add("ScanExposureDuration", 
                 HiVector(1,ToDouble(hiprof("ScanExposureDuration"))));
      }
      else {
        calVars->add("Ziof", HiVector(1, 1.0));
        calVars->add("ScanExposureDuration", HiVector(1, 1.0));
        ZiofHist.add("I/F conversion not selected by user");
        ZiofHist.add("Units[DNs/microsecond]");
      }
    }
    else {
      calVars->add("Ziof", HiVector(1,1.0));
      calVars->add("ScanExposureDuration", HiVector(1, 1.0));
      ZiofHist.add("Debug::SkipModule invoked!");
    }

    //  Reset the profile selection to default
    hiconf.selectProfile();

//----------------------------------------------------------------------
// 
/////////////////////////////////////////////////////////////////////////
//  Call the processing function
    procStep = "calibration phase";
    p.StartProcess(calibrate);

    // Get the default profile for logging purposes
    hiprof = hiconf.getMatrixProfile();
    const std::string conf_file = hiconf.filepath(conf);

    // Quitely dumps parameter history to alternative format file.  This
    // is completely controlled by the configuration file
    if ( hiprof.exists("DumpHistoryFile") ) {
      procStep = "logging/reporting phase";
      Filename hdump(hiconf.getMatrixSource("DumpHistoryFile",hiprof));
      string hdumpFile = hdump.Expanded();
      ofstream ofile(hdumpFile.c_str(), ios::out);
      if (!ofile) {
        string mess = "Unable to open/create history dump file " + 
                      hdump.Expanded();
        iException::Message(iException::User, mess, _FILEINFO_).Report();
      }
      else {
        ofile << "Program:  " << hical_program << endl;
        ofile << "RunTime:  " << hical_runtime << endl;
        ofile << "Version:  " << hical_version << endl;
        ofile << "Revision: " << hical_revision << endl << endl;

        ofile << "FROM:     " << hifrom->Filename() << endl;
        ofile << "TO:       " << ocube->Filename()  << endl;
        ofile << "CONF:     " << conf_file  << endl << endl;

        ofile << "/* Hicalbeta application equation */" << endl
              << "/* hdn = (idn - Zd(Zf) - Zz - Zb)/ScanExposureDuration */"
              << endl << "/* odn = hdn / Zg * Zgg * Za / Ziof */" 
              << endl << endl;

        ofile << "****** PARAMETER GENERATION HISTORY *******" << endl;
        ofile << "\nZf   = " << ZfHist << endl;
        ofile << "\nZd   = " << ZdHist << endl;
        ofile << "\nZz   = " << ZzHist << endl;
        ofile << "\nZb   = " << ZbHist << endl;
        ofile << "\nZg   = " << ZgHist << endl;
        ofile << "\nZgg  = " << ZggHist << endl;
        ofile << "\nZa   = " << ZaHist << endl;
        ofile << "\nZiof = " << ZiofHist << endl;

        ofile.close();
      }
    }

//  Ensure the RadiometricCalibration group is out there
    const std::string rcalGroup("RadiometricCalibration");
    if (!ocube->HasGroup(rcalGroup)) {
      PvlGroup temp(rcalGroup);
      ocube->PutGroup(temp);
    }

    PvlGroup &rcal = ocube->GetGroup(rcalGroup);
    rcal += PvlKeyword("Program", hical_program);
    rcal += PvlKeyword("RunTime", hical_runtime);
    rcal += PvlKeyword("Version",hical_version);
    rcal += PvlKeyword("Revision",hical_revision);

    PvlKeyword key("Conf", conf_file);
    key.AddCommentWrapped("/* Hicalbeta application equation */");
    key.AddComment("/* hdn = (idn - Zd(Zf) - Zz - Zb)/ScanExposureDuration */");
    key.AddComment("/* odn = hdn / Zg * Zgg * Za / Ziof */");
    rcal += key;

    //  Record parameter generation history.  Controllable in configuration
    //  file.  Note this is optional because of a BUG!! in the ISIS label
    //  writer as this application was initially developed
    if ( IsEqual(ConfKey(hiprof,"LogParameterHistory",string("TRUE")),"TRUE")) { 
      rcal += ZfHist.makekey("Zf");
      rcal += ZdHist.makekey("Zd");
      rcal += ZzHist.makekey("Zz");
      rcal += ZbHist.makekey("Zb");
      rcal += ZgHist.makekey("Zg");
      rcal += ZggHist.makekey("Zgg");
      rcal += ZaHist.makekey("Za");
      rcal += ZiofHist.makekey("Ziof");
    }

    p.EndProcess();
  } 
  catch (iException &ie) {
    delete calVars;
    calVars = 0;
    string mess = "Failed in " + procStep;
    ie.Message(iException::User, mess.c_str(), _FILEINFO_);
    throw;
  }
  
// Clean up parameters
  delete calVars;
  calVars = 0;
}

