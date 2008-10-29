#include "Isis.h"
#include "PvlObject.h"
#include "PvlGroup.h"
#include "UserInterface.h"
#include "iException.h"

using namespace std;
using namespace Isis;

void IsisMain() {
  // Get user interface
  UserInterface &ui = Application::GetUserInterface();

  Pvl algos("$ISISROOT/lib/AutoReg.plugin");
  Pvl p;
  string algoName = ui.GetString("ALGORITHM");

  // Give the user a list of possible algorithms if the algoName="Unknown"
  if (algoName == "Unknown") {
    string msg = "Unknown Algorithm Name [" + algoName + "] " + 
      "Possible Algorithms are: ";
    for (int i=0; i<algos.Groups(); i++) {
      msg += algos.Group(i).Name();
      if (i<algos.Groups()-1) msg += ", ";
    }
    throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
  }

  // Everything seems to be okay, continue creating the mapping group
  else {
    PvlObject autoreg("AutoRegistration");    

    // Make sure the entered algorithm name is valid
    if (!algos.HasGroup(algoName)) {
      string msg = "Invalid Algorithm Name [" + algoName + "]";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }

    // Make algorithm group
    PvlGroup algorithm("Algorithm");
    algorithm += PvlKeyword("Name",algoName);

    // Set the tolerance
    if (ui.WasEntered("TOLERANCE")) {
      double tol = ui.GetDouble("TOLERANCE");
      algorithm += PvlKeyword("Tolerance",iString(tol));
    }

    // Set the sampling if the user entered it
    if (ui.WasEntered("SAMPLING")) {
      algorithm += PvlKeyword("Sampling",iString(ui.GetInteger("SAMPLING")),
                              "percent");
    }

    if (ui.GetBoolean("SUBPIXELACCURACY")) algorithm += PvlKeyword("SubpixelAccuracy","True");
    else algorithm += PvlKeyword("SubpixelAccuracy","False");



    // Add algorithm group to the autoreg object
    autoreg.AddGroup(algorithm);

    // Get pattern and search chip size values for error testing
    int psamp = ui.GetInteger("PSAMP");
    int pline = ui.GetInteger("PLINE");
    int ssamp = ui.GetInteger("SSAMP");
    int sline = ui.GetInteger("SLINE");

    // Make sure the pattern chip is not just one pixel
    if (psamp + pline < 3) {
      string msg = "The Pattern Chip must be larger than one pixel for the ";
      msg += "autoregistration to work properly";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }
    // Make sure the pattern chip is smaller than the search chip
    if (ssamp < psamp || sline < pline) {
      string msg = "The Pattern Chip must be smaller than the Search Chip";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }
    // Make sure the pattern chip spans at least a 3x3 window in the search chip
    if (psamp +2 > ssamp || pline + 2 > sline) {
      string msg = "The Pattern Chip must span at least a 3x3 window in the ";
      msg += "Search Chip";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }

    // Set up the pattern chip group
    PvlGroup patternChip("PatternChip");
    patternChip += PvlKeyword("Samples",iString(psamp));
    patternChip += PvlKeyword("Lines",iString(pline));
    if (ui.WasEntered("PMIN")) {
      patternChip += PvlKeyword("ValidMinimum",iString(ui.GetInteger("PMIN")));
    }
    if (ui.WasEntered("PMAX")) {
      patternChip += PvlKeyword("ValidMaximum",iString(ui.GetInteger("PMAX")));
    }
    if (ui.WasEntered("PPERCENT")) {
      patternChip += PvlKeyword("ValidPercent",iString(ui.GetInteger("PPERCENT")));
    }
    if (ui.WasEntered("MINIMUMPATTERNZSCORE")) {
      double minsd = ui.GetDouble("MINIMUMPATTERNZSCORE");
      patternChip += PvlKeyword("MinimumZScore",iString(minsd));
    }

    // Set up the search chip group
    PvlGroup searchChip("SearchChip");
    searchChip += PvlKeyword("Samples",iString(ssamp));
    searchChip += PvlKeyword("Lines",iString(sline));
    if (ui.WasEntered("SMIN")) {
      searchChip += PvlKeyword("ValidMinimum",iString(ui.GetInteger("SMIN")));
    }
    if (ui.WasEntered("SMAX")) {
      searchChip += PvlKeyword("ValidMaximum",iString(ui.GetInteger("SMAX")));
    }
    if (ui.WasEntered("SPERCENT")) {
      searchChip += PvlKeyword("ValidPercent",iString(ui.GetInteger("SPERCENT")));
    }
    int sinc = ui.GetInteger("SINC");
    int linc = ui.GetInteger("LINC");
    if (sinc != 1 || linc !=1) {
      string den = "(" + iString(sinc) + "," + iString(linc) + ")";
      searchChip += PvlKeyword("Density",den);
    }

    // Add groups to the autoreg object
    autoreg.AddGroup(patternChip);
    autoreg.AddGroup(searchChip);

    // Add autoreg group to Pvl
    p.AddObject(autoreg);
  }

  // Write the autoreg group pvl to the output file
  string output = ui.GetFilename("TO");
  p.Write(output);
}
