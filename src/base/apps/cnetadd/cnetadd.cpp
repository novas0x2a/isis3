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

using namespace std;
using namespace Isis;

void SetControlPointLatLon( const std::string &incubes, const std::string &cnet );

std::map< std::string, std::pair<double,double> > p_pointLatLon;

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

  SetControlPointLatLon( ui.GetFilename("FROMLIST"), ui.GetFilename("CNET") );
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
  for( unsigned int img =0; img < list2.size(); img ++ ) {
    Pvl cubepvl;
    bool imageAdded = false;
    cubepvl.Read(list2[img]);
    Camera *cam = CameraFactory::Create(cubepvl);
    
    //loop through all the control points
    for( int cp = 0 ; cp < inNet.Size() ; cp ++ ) {
      ControlPoint point( inNet[cp] );
      ControlMeasure cmeasure( point[ inNet[cp].ReferenceIndex() ] );
      
      // Get the lat/long coords from the existing reference measure
      if ( cam->SetUniversalGround( p_pointLatLon[point.Id()].first, p_pointLatLon[point.Id()].second ) ) {

        // Make sure the samp & line are inside the image
        if( cam->InCube() ) {

          ControlMeasure newCm;
          newCm.SetCoordinate(cam->Sample(),cam->Line(),ControlMeasure::Estimated);
          newCm.SetCubeSerialNumber(SerialNumber::Compose(cubepvl));
          newCm.SetDateTime();
          newCm.SetChooserName("Application cnetadd");
          inNet[cp].Add(newCm);
          
          imageAdded = true;
        }
      }
    }

    delete cam;
    cam = NULL;

    if(imageAdded) {
      added.AddValue(Isis::Filename(list2[img]).Basename());
    }else{
      omitted.AddValue(Isis::Filename(list2[img]).Basename());
    }
    progress.CheckStatus();
  }

  results.AddKeyword(added);
  results.AddKeyword(omitted);

  if(log) {
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
  manager.SetNumOpenCubes( 50 ); //Should keep memory usage to around 1GB

  Progress progress;
  progress.SetText("Calculating Lat/Lon");
  progress.SetMaximumSteps(net.Size());
  progress.CheckStatus();

  for( int cp = 0 ; cp < net.Size() ; cp ++ ) {
    ControlPoint point( net[cp] );
    ControlMeasure cm( point[ net[cp].ReferenceIndex() ] );
    
    Cube * cube = manager.OpenCube( snl.Filename( cm.CubeSerialNumber() ) );
    try {
      cube->Camera()->SetImage( cm.Sample(), cm.Line() );
      p_pointLatLon[point.Id()].first = cube->Camera()->UniversalLatitude();
      p_pointLatLon[point.Id()].second = cube->Camera()->UniversalLongitude();
    }
    catch (Isis::iException &e) {
      std::string msg = "Unable to create camera for cube file [";
      msg += snl.Filename( cm.CubeSerialNumber() ) + "]";
      throw Isis::iException::Message(Isis::iException::System,msg,_FILEINFO_);
    }
    cube = NULL; //Do not delete, manager still has ownership

    progress.CheckStatus();
  }

  manager.CleanCubes();
}
