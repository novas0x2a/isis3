#include "Isis.h"

#include <string>

#include "ProcessImport.h"

#include "Filename.h"
#include "iException.h"
#include "iTime.h"

using namespace std; 
using namespace Isis;

void IsisMain ()
{
  // Make sure they want something done
  Process p;
  UserInterface &ui = Application::GetUserInterface();
  if (!ui.GetBoolean("INGESTION") && 
      !ui.GetBoolean("CALIBRATION") &&
      !ui.GetBoolean("MAPPING")) {
    string m = "Please pick at least one of [INGESTION, CALIBRATION, MAPPING]";
    throw iException::Message(iException::User,m,_FILEINFO_);
  }
 

  // Get the input filename
  Filename mainInput = Filename (ui.GetFilename ("FROM"));

  // Find out what the final level of proecesing will be
  int finalLevel = 2;
  if (!ui.GetBoolean("MAPPING")) finalLevel = 1;
  if (!ui.GetBoolean("CALIBRATION")) finalLevel = 0;

  // Get the final (last stage) output filename
  Filename finalOutput;
  // Use the specified output filename
  if (ui.WasEntered("TO")) {
    finalOutput = ui.GetFilename("TO");
  }
  // Create the output filename using the input file as a template
  else {
    finalOutput = mainInput.Basename() + ".lev" + iString(finalLevel) + ".cub";
  }

  // Create filenames to hold the temporary output of each app and the input
  // to the next app
  Filename output, input;
  output = mainInput.Expanded();


  //---------------------------------------------------------------------------
  // Run the ingestion processing apps if requested

  if (ui.GetBoolean("INGESTION")) {
    // Run moc2isis
    input = output.Expanded();
    // Set up the temporary filename for the output of moc2isis
    if (finalLevel == 0) {
      output = finalOutput.Expanded();
    }
    else {
      output.Temporary (mainInput.Basename() + "_moc2isis_", "cub");
    }

    string parameters = "FROM=" + input.Expanded() +  
                        " TO=" + output.Expanded();
    Isis::iApp->Exec("moc2isis",parameters);

    // Run spiceinit
    input = output.Expanded();
    parameters = "FROM=" + input.Expanded();
    parameters += " SHAPE=" + ui.GetString("SHAPE");
    if (ui.GetString("SHAPE") == "USER") {
      parameters +=  " MODEL=" + ui.GetFilename("MODEL");
    }
    if (ui.WasEntered("CK")) {
      parameters += " CK=" + ui.GetFilename("CK");
    }
    if (ui.WasEntered("PCK"))  {
      parameters += " PCK=" + ui.GetFilename("PCK");
    }
    if (ui.WasEntered("SPK")) {
      parameters += " SPK=" + ui.GetFilename("SPK");
    }

    parameters += iString(" CKNADIR=") + iString((ui.GetBoolean("CKNADIR"))? "true" : "false");

    Isis::iApp->Exec("spiceinit",parameters);
  } // end of ingestion if


  //---------------------------------------------------------------------------
  // Run the calibration processing apps if requested
  if (ui.GetBoolean("CALIBRATION")) {
    // Get some information for decisions later in this section
    Process tmpP;
    CubeAttributeInput att;
    Cube *icube = tmpP.SetInputCube (output.Expanded(), att);

    PvlGroup inst = icube->GetGroup("Instrument");
    iString id = (string) inst["InstrumentId"];
    id.UpCase();
    int crosstrackSumming = inst["CrosstrackSumming"];

    PvlGroup bandBin = icube->GetGroup("BandBin");
    iString filterName = (string) bandBin["FilterName"];
    filterName.UpCase();

    tmpP.EndProcess();

    // Run moccal
    input = output.Expanded();
    string inputExpanded = input.Expanded();
    // Set up the temporary filename for the output of moccal

    if (crosstrackSumming != 1 && finalLevel == 1) output = finalOutput.Expanded();
    else output.Temporary (mainInput.Basename() + "_moccal_", "cub");

    // ****NOTE****
    // take the "+32bit" off the extension below when filenames/attributes settle down
    string parameters = "FROM=" + input.Expanded() + 
                        " TO=" + output.Expanded()+"+32BIT";
    Isis::iApp->Exec("moccal",parameters);
    remove (inputExpanded.c_str());

    // Run mocnoise50 if:
    //  the instrument id is "MOC_NA"
    //  and the crosstrack summing is 1 (one)

    if (id == "MOC_NA" && crosstrackSumming == 1) {
      input = output.Expanded();
      string inputExpanded = input.Expanded();
      // Set up the temporary filename for the output of mocnoise50
      output.Temporary (mainInput.Basename() + "_mocnoise50_", "cub");
      parameters = "FROM=" + input.Expanded() + 
                   " TO=" + output.Expanded();
      Isis::iApp->Exec("mocnoise50",parameters);
      remove (inputExpanded.c_str());
    }

    // Run mocevenodd if:
    //   the crosstrackSumming is 1 (one)
    if (crosstrackSumming == 1) {
      input = output.Expanded();
      string inputExpanded = input.Expanded();
      // Set up the temporary filename for the output of mocevenodd
      if (finalLevel == 1) {
        output = finalOutput.Expanded();
      }
      else {
        output.Temporary (mainInput.Basename() + "_mocevenodd_", "cub");
      }

      parameters = "FROM=" + input.Expanded() + 
                   " TO=" + output.Expanded();
      Isis::iApp->Exec("mocevenodd",parameters);
      remove (inputExpanded.c_str());
    }

  } // end of calibration if


  //---------------------------------------------------------------------------
  // Run all mapping apps if requested
  if (ui.GetBoolean("MAPPING")) {

    // Run cam2map
    input = output.Expanded();
    string inputExpanded = input.Expanded();
    string parameters = "FROM=" + input.Expanded();
    parameters += " TO=" + finalOutput.Expanded();
    parameters += " DEFAULTRANGE = CAMERA";
    if (ui.WasEntered("MAP")) {
      parameters +=  " MAP=" + ui.GetFilename("MAP");
      Pvl m(ui.GetFilename("MAP"));
      if (m.FindGroup("Mapping").HasKeyword("PixelResolution")
            && !ui.WasEntered("PIXRES")) {
        parameters += " PIXRES=MAP";
      }
    }
    if (ui.WasEntered("PIXRES")) {
      parameters += " PIXRES=MPP RESOLUTION=" + ui.GetAsString("PIXRES");
    }

    Isis::iApp->Exec("cam2map",parameters);
    remove (inputExpanded.c_str());

  } // end of mapping if

  return;
}
