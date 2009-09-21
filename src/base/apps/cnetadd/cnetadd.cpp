#include "Isis.h"

#include "UserInterface.h"
#include "FileList.h"
#include "Filename.h"
#include "Pvl.h"
#include "SerialNumber.h"
#include "SerialNumberList.h"
#include "CameraFactory.h"
#include "ControlNet.h"
#include "ControlPoint.h"
#include "ControlMeasure.h"
#include "iException.h"
#include "CubeManager.h"
#include "iTime.h"

using namespace std;
using namespace Isis;

void SetControlPointLatLon( const std::string &incubes, const std::string &cnet );

std::map< std::string, std::pair<double,double> > p_pointLatLon;

void IsisMain() {

  UserInterface &ui = Application::GetUserInterface();

  FileList list2 (ui.GetFilename("ADDLIST"));

  bool log = false;
  Filename logFile;
  if (ui.WasEntered("LOG")) {
    log = true;
    logFile = ui.GetFilename("LOG");
  }
  Pvl results;
  results.SetName("cnetadd_Results");
  PvlKeyword added ("FilesAdded");
  PvlKeyword omitted ("FilesOmitted");
  PvlKeyword pointsModified ("PointsModified");

  string retrievalOpt = ui.GetString("RETRIEVAL");
  if (retrievalOpt == "REFERENCE") {
    FileList list1 (ui.GetFilename("FROMLIST"));
    SerialNumberList addSerials (ui.GetFilename("ADDLIST"));
    SerialNumberList fromSerials (ui.GetFilename("FROMLIST"));

    //Check for duplicate files in the lists by serial number
    vector<string> duplicates;
    for (int i = 0; i < addSerials.Size(); i++) {

      // Check for duplicate SNs accross the lists
      if (fromSerials.HasSerialNumber(addSerials.SerialNumber(i))) {
        duplicates.push_back(addSerials.Filename(i));
      }

      // Check for duplicate SNs within the addlist
      for (int j = i+1; j < addSerials.Size(); j++) {
        if (addSerials.SerialNumber(i) == addSerials.SerialNumber(j)) {
          string msg = "Add list files [" + addSerials.Filename(i) + "] and [";
          msg += addSerials.Filename(j) + "] share the same serial number.";
          throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
        }
      }
    }
  
    // If duplicates throw error
    if (duplicates.size() > 0) {
      string msg = "The following files share serial numbers accross lists: [" + duplicates[0];
      for (unsigned int k = 1; k < duplicates.size(); k++) {
        msg += "," + duplicates[k];
      }
      msg += "]";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }
  
    SetControlPointLatLon( ui.GetFilename("FROMLIST"), ui.GetFilename("CNET") );

  }

  Filename outNet(ui.GetFilename("TO"));
  
  ControlNet inNet = ControlNet(ui.GetFilename("CNET"));
  inNet.SetUserName( Isis::Application::UserName() );
  //inNet.SetCreatedDate( Isis::Application::DateTime() );    //This should be done in ControlNet's Write fn
  inNet.SetModifiedDate( Isis::iTime::CurrentLocalTime() );
  
  Progress progress;
  progress.SetText("Adding Images");
  progress.SetMaximumSteps(list2.size());
  progress.CheckStatus();
  
  // loop through all the images
  vector<int> modPoints;
  for (unsigned int img = 0; img < list2.size(); img++) {
    Pvl cubepvl;
    bool imageAdded = false;
    cubepvl.Read(list2[img]);
    Camera *cam = CameraFactory::Create(cubepvl);
    
    //loop through all the control points
    for (int cp = 0; cp < inNet.Size(); cp++) {
      ControlPoint point( inNet[cp] );
      
      double latitude;
      double longitude;
      if (retrievalOpt == "REFERENCE") {
        // Get the lat/long coords from the existing reference measure
        latitude = p_pointLatLon[point.Id()].first;
        longitude = p_pointLatLon[point.Id()].second;
      }
      else {
        // Get the lat/long coords from the current control point
        latitude = point.UniversalLatitude();
        longitude = point.UniversalLongitude();
      }

      if (cam->SetUniversalGround(latitude, longitude)) {
      
        // Make sure the samp & line are inside the image
        if (cam->InCube()) {

          ControlMeasure newCm;
          newCm.SetCoordinate(cam->Sample(),cam->Line(),ControlMeasure::Estimated);
          newCm.SetCubeSerialNumber(SerialNumber::Compose(cubepvl));
          newCm.SetDateTime();
          newCm.SetChooserName("Application cnetadd");
          inNet[cp].Add(newCm);
      
          if (retrievalOpt == "POINT" && inNet[cp].Size() == 1) {
            inNet[cp].SetIgnore(false);
          }

          if (log) {

            // If we can't find this control point in the list of control points
            // that have already been modified, then add it to the list
            bool doesntContainPoint = true;
            for (unsigned int i = 0; i < modPoints.size() && doesntContainPoint; i++) {
              if (modPoints[i] == cp) doesntContainPoint = false;
            }
            if (doesntContainPoint) {
              modPoints.push_back(cp);
            }

            imageAdded = true;
          }
        }
      }
    }

    delete cam;
    cam = NULL;

    if (log) {
      if (imageAdded) added.AddValue(Isis::Filename(list2[img]).Basename());
      else omitted.AddValue(Isis::Filename(list2[img]).Basename());
    }

    progress.CheckStatus();
  }

  if (log) {

    // Shell sort the list of modified control points
    int increments[] = { 1391376, 463792, 198768, 86961, 33936, 13776, 4592, 1968,
                         861, 336, 112, 48, 21, 7, 3, 1 };
    for (unsigned int k = 0; k < 16; k++) {
      int inc = increments[k];
      for (unsigned int i = inc; i < modPoints.size(); i++) {
        int temp = modPoints[i];
        int j = i;
        while (j >= inc && modPoints[j - inc] > temp) {
          modPoints[j] = modPoints[j - inc];
          j -= inc;
        }
        modPoints[j] = temp;
      }
    }
  
    // Add the list of modified points to the output log file
    for (unsigned int i = 0; i < modPoints.size(); i++) {
      pointsModified += inNet[modPoints[i]].Id();
    }
   
    results.AddKeyword(added);
    results.AddKeyword(omitted);
    results.AddKeyword(pointsModified);
  
    results.Write(logFile.Expanded());
  }

  inNet.Write(outNet.Expanded());
}


/**
 * Calculates the lat/lon of the ControlNet.
 * 
 * @param incubes The filename of the list of cubes in the ControlNet
 * @param cnet    The filename of the ControlNet
 */
void SetControlPointLatLon( const std::string &incubes, const std::string &cnet ) {
  SerialNumberList snl( incubes );
  ControlNet net( cnet );

  CubeManager manager;
  manager.SetNumOpenCubes(50); //Should keep memory usage to around 1GB

  Progress progress;
  progress.SetText("Calculating Lat/Lon");
  progress.SetMaximumSteps(net.Size());
  progress.CheckStatus();

  for (int cp = 0; cp < net.Size(); cp++) {
    ControlPoint point( net[cp] );
    ControlMeasure cm( point[ net[cp].ReferenceIndex() ] );

    Cube *cube = manager.OpenCube( snl.Filename( cm.CubeSerialNumber() ) );
    try {
      cube->Camera()->SetImage( cm.Sample(), cm.Line() );
      p_pointLatLon[point.Id()].first = cube->Camera()->UniversalLatitude();
      p_pointLatLon[point.Id()].second = cube->Camera()->UniversalLongitude();
    } catch (Isis::iException &e) {
      std::string msg = "Unable to create camera for cube file [";
      msg += snl.Filename( cm.CubeSerialNumber() ) + "]";
      throw Isis::iException::Message(Isis::iException::System,msg,_FILEINFO_);
    }
    cube = NULL; //Do not delete, manager still has ownership

    progress.CheckStatus();
  }

  manager.CleanCubes();
}
