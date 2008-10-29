#include "Isis.h"

#include <map>

#include "Process.h"
#include "Pvl.h"
#include "TextFile.h"
#include "FileList.h"
#include "SerialNumberList.h"
#include "ControlNet.h"
#include "ID.h"
#include "iException.h"

using namespace std;
using namespace Isis;

std::map <int,string> snMap;
//std::map <string,int> fscMap;

void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();

  // The following steps can take a significant amount of time, so 
  // set up a progress object to keep the user informed
  Progress progress;
  progress.SetText("Initializing");
  progress.SetMaximumSteps(3);
  progress.CheckStatus();

  // Prepare the ISIS2 list of file names
  FileList list2(ui.GetFilename("LIST2"));
  progress.CheckStatus();

  // Prepare the ISIS3 SNs
  SerialNumberList snl(ui.GetFilename("LIST3"));
  progress.CheckStatus();

  if (list2.size() != (unsigned)snl.Size()) {
    iString msg = "Invalid input file number of lines. The ISIS 2 file list [";
    msg += ui.GetFilename("LIST2") + "] must contain the same number of lines ";
    msg += "as the ISIS 3 file list [" + ui.GetFilename("LIST3") + "]";
    throw Isis::iException::Message (Isis::iException::User, msg, _FILEINFO_);
  }

  // Setup a map between ISIS2 image number (fsc) and ISIS3 sn
  for (unsigned int f=0; f<list2.size(); f++) {
    iString currFile (list2[f]);
    Pvl lab (currFile);
    PvlObject qube (lab.FindObject("QUBE"));
  
  	iString fsc;
  	if(qube.HasKeyword("IMAGE_NUMBER")) {
      fsc = qube.FindKeyword("IMAGE_NUMBER")[0];
  	}
  	else if(qube.HasKeyword("IMAGE_ID")) {
  	  fsc = qube.FindKeyword("IMAGE_ID")[0];
  	}
  	else {
  	  throw iException::Message(iException::Pvl, 
			    "Can not find required keyword IMAGE_NUMBER or IMAGE_ID in [" + currFile + "]",
          _FILEINFO_);
  	}

    iString sn (snl.SerialNumber(f));
    snMap.insert(std::pair<int,string>((int)fsc, sn));
  }
  progress.CheckStatus();

  // Create a new control network
  ControlNet cnet;
  cnet.SetType (ControlNet::ImageToGround);
  cnet.SetTarget (ui.GetString("TARGET"));
  cnet.SetNetworkId(ui.GetString("NETWORKID"));
  cnet.SetUserName(Isis::Application::UserName());
  cnet.SetDescription(ui.GetString("DESCRIPTION"));
  cnet.SetCreatedDate(Isis::Application::DateTime());

  // Open the match point file
  TextFile mpFile (ui.GetFilename("MATCH"));
  iString currLine;

  // Read the first line with the number of measurments
  mpFile.GetLine (currLine);
  currLine.Token("=");
  currLine.Token(" ");
  int inTotalMeas = int(currLine);

  // Read line 2, the column header line and ignore it
  mpFile.GetLine (currLine);

  // Reset the progress object for feedback about conversion processing
  progress.SetText("Converting");
  progress.SetMaximumSteps(inTotalMeas);

  int line = 2;
  while (mpFile.GetLine(currLine)) {
    line ++;
    string origLine = currLine;

    // Update the Progress object
    try {
      progress.CheckStatus();
    } catch ( Isis::iException e ) {
      e.Clear();
      string msg = "The \"Matchpoint total\" keyword at the top of the Matchpoint file [";
      msg += ui.GetFilename("MATCH") + "] equals [" + iString(inTotalMeas);
      msg += "], and is likely incorrect.";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }
    

    // Declare the Point and Measure
    ControlPoint *cpoint;
    ControlMeasure cmeasure;

    // Section the match point line into the important pieces
    currLine.ConvertWhiteSpace();
    currLine.Compress();
    string pid = currLine.Token(" "); //The ID of the point
    iString fsc = currLine.Token(" "); //The FSC of the Isis 2 cube
    string lineNum = currLine.Token(" ");
    string sampNum = currLine.Token(" ");
    string matClass = currLine.Token(" "); //The Match Point Class
    string diam = currLine.Token(" "); //Diameter, in case of a crater

    // Set the coordinate and serial number for this measure
    cmeasure.SetCoordinate(iString::ToDouble(sampNum),
                           iString::ToDouble(lineNum));

    int index = (int)fsc;
    cmeasure.SetCubeSerialNumber(snMap[index]);

    if(snMap[index].empty()) {      
      std::string msg = "The FSC specified in the Match Point File [";
      msg += ui.GetFilename("MATCH") + "] Line [" + iString(line);
      msg += "] does not exist. ";
      msg += "None of the images specified in [" + ui.GetFilename("LIST2");
      msg += "] have an IMAGE_NUMBER or IMAGE_ID that matches the FSC [" + iString(index);
      msg += "]";
      throw iException::Message(iException::User, msg, _FILEINFO_);
    }

    //Set the Measure Type
    if (iString::Equal(matClass,"T")) {
      cmeasure.SetReference(true);
      cmeasure.SetType(ControlMeasure::ValidatedManual);
    }
    else if (iString::Equal(matClass,"M")) {
      cmeasure.SetType(ControlMeasure::ValidatedManual);
    }
    else if (iString::Equal(matClass,"S")) {
      cmeasure.SetType(ControlMeasure::ValidatedAutomatic);
    }
    else if (iString::Equal(matClass,"U") && 
             (cmeasure.Sample() != 0.0) && (cmeasure.Line() != 0.0)) {
      cmeasure.SetType(ControlMeasure::Estimated);
    }
    else if (iString::Equal(matClass,"U")) {
      cmeasure.SetType(ControlMeasure::Unmeasured);
    }
    else {
      iString msg = "Unknown measurment type [" + matClass + "] ";
      msg += "in match point file [" + ui.GetFilename("MATCH") + "] ";
      msg += "at line [" + iString(line) + "]";
      throw Isis::iException::Message (Isis::iException::User, msg, _FILEINFO_);
    }

    //Set Diameter
    try {
      //Check to see if the column was, in fact, a double
      cmeasure.SetDiameter(iString::ToDouble(diam));
    } catch (iException &e) {
      //If we get here, it was not, but the error is not important otherwise
      e.Clear();
    }

    //Find the point that matches the PointID, create it if it does not exist
    try {
      cpoint = cnet.Find(pid);
    } catch (iException &e) {
      e.Clear();
      cnet.Add(ControlPoint(pid));
      cpoint = cnet.Find(pid);
    }

    //Add the measure
    try {
      cpoint->Add(cmeasure);
    } catch (iException &e) {
      iString msg = "Unable to add measurment to control network from line [";
      msg += iString(line);
      msg += "] of match point file [";
      msg += ui.GetFilename("MATCH");
      msg += "]";
      throw Isis::iException::Message (Isis::iException::User, msg, _FILEINFO_);
    }
  }

  // Update the Progress object
  try {
      progress.CheckStatus();
    } catch ( Isis::iException e ) {
      e.Clear();
      string msg = "The \"Matchpoint total\" keyword at the top of the Matchpoint file [";
      msg += ui.GetFilename("MATCH") + "] equals [" + iString(inTotalMeas);
      msg += "], and is likely incorrect.";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }


  // Write the control network out
  cnet.Write(ui.GetFilename("CONTROL"));
}
