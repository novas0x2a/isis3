#include "Isis.h"

#include "Pvl.h"
#include "FileList.h"
#include "Filename.h"
#include "SerialNumber.h"
#include "SerialNumberList.h"
#include "CameraFactory.h"
#include "ControlNet.h"
#include "ControlPoint.h"
#include "ControlMeasure.h"
#include "iException.h"

using namespace std;
using namespace Isis;

void SetImages (const std::string &imageListFile);

std::map<std::string,Isis::Camera *> p_cameraMap;    //!< A map from serialnumber to camera
ControlNet inNet;

void IsisMain() {

  UserInterface &ui = Application::GetUserInterface();

  FileList list1(ui.GetFilename("FROMLIST"));
  FileList list2(ui.GetFilename("FROMLIST2"));

  bool log = false;
  Filename logFile;
  if(ui.WasEntered("LOG")) {
    log = true;
    logFile = ui.GetFilename("LOG");
  }
  Pvl results;
  results.SetName("cnetadd_Results");
  PvlKeyword added("FilesAdded");
  PvlKeyword omitted("FilesOmitted");

  vector<string> duplicates;

  //Check for duplicate files in the lists
  for(unsigned int i = 0; i < list2.size() ;i++) {
    for(unsigned int j = 0; j < list1.size() ;j++) {
      if(Filename(list1[j]).Expanded().compare(Filename(list2[i]).Expanded())==0) {
        duplicates.push_back(list1[j]);
      }
    }
  }

  // If duplicates throw error
  if(duplicates.size() > 0) {
    string msg = "The following files are duplicates in both lists: [" + duplicates[0];
    for(unsigned int k = 1; k< duplicates.size();k++) {
      msg += "," + duplicates[k];
    }
    msg += "]";
    throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
  }

  inNet = ControlNet(ui.GetFilename("NETWORK"));
  SetImages(ui.GetFilename("FROMLIST"));
  Filename outNet(ui.GetFilename("TO"));

  Progress progress;
  progress.SetText("Adding Images");
  progress.SetMaximumSteps(list2.size());

  // loop through all the images
  for(unsigned int i =0; i < list2.size(); i++) {
    Pvl cubepvl;
    bool imageAdded = false;
    cubepvl.Read(list2[i]);
    Camera *cam = CameraFactory::Create(cubepvl);
    //loop through all the control points
    for(int j=0; j< inNet.Size();j++) {
      ControlPoint &cp = inNet[j];
      ControlMeasure &cm = cp[cp.ReferenceIndex()];
      // Get the lat/long coords from the existing reference measure
      Camera *cmCam = p_cameraMap[cm.CubeSerialNumber()];
      cmCam->SetImage(cm.Sample(), cm.Line());
      cam->SetUniversalGround(cmCam->UniversalLatitude(),cmCam->UniversalLongitude());
      // Make sure the samp & line are inside the image
      if(cam->Sample() >= 0.5 && cam->Sample() <= cam->Samples()+0.5) {
        if(cam->Line() >= 0.5 && cam->Line() <= cam->Lines()+0.5) {
          ControlMeasure newCm;
          newCm.SetCoordinate(cam->Sample(),cam->Line());
          newCm.SetCubeSerialNumber(SerialNumber::Compose(cubepvl));
          newCm.SetDateTime();
          newCm.SetChooserName("Application cnetadd");
          cp.Add(newCm);
          
          imageAdded = true;
        }
      }    
    }
    if(!imageAdded) {
      omitted.AddValue(Isis::Filename(list2[i]).Basename());
    }else{
      added.AddValue(Isis::Filename(list2[i]).Basename());
    }
    progress.CheckStatus();
  }

  progress.CheckStatus();
  results.AddKeyword(added);
  results.AddKeyword(omitted);

  if(log) {
    results.Write(logFile.Expanded());
  }

  inNet.Write(outNet.Expanded());
}

  /**
   * Creates the ControlNet's image cameras based on an input file
   * 
   * @param imageListFile The list of images
   */
  void SetImages (const std::string &imageListFile) {
    SerialNumberList list(imageListFile);
    // Open the camera for all the images in the serial number list
    for (int i=0; i<list.Size(); i++) {
      std::string serialNumber = list.SerialNumber(i);
      std::string filename = list.Filename(i);
      Pvl pvl(filename);

      try {
        Isis::Camera *cam = CameraFactory::Create(pvl);
        p_cameraMap[serialNumber] = cam;
      }
      catch (Isis::iException &e) {
        std::string msg = "Unable to create camera for cube file ";
        msg += filename;
        throw Isis::iException::Message(Isis::iException::System,msg,_FILEINFO_);
      }
    }
  }
