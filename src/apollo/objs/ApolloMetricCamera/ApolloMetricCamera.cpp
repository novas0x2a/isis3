#include "ApolloMetricCamera.h"
#include "iString.h"
#include "iException.h"
#include "CameraDetectorMap.h"
#include "CameraFocalPlaneMap.h"
#include "ReseauDistortionMap.h"
#include "CameraGroundMap.h"
#include "CameraSkyMap.h"
#include "ApolloMetricDistortionMap.h"
#include "Filename.h"

using namespace std;

namespace Isis {
  namespace Apollo {
    // constructors
    ApolloMetricCamera::ApolloMetricCamera (Isis::Pvl &lab) : Isis::FramingCamera(lab) {
      // Get the camera characteristics
      SetFocalLength();
      SetPixelPitch();

      // Setup detector map
      new CameraDetectorMap(this);

      // Setup focal plane map
      CameraFocalPlaneMap *focalMap = new CameraFocalPlaneMap(this, NaifIkCode());
      focalMap->SetDetectorOrigin(ParentSamples()/2.0, ParentLines()/2.0);

      const PvlGroup &inst = lab.FindGroup("Instrument", Pvl::Traverse);

      if((string)inst["SpacecraftName"] == "APOLLO 15") {
        // these values are for Apollo 15
        new ApolloMetricDistortionMap(this, -0.006, -0.002, -0.13361854e-5, 0.52261757e-9, -0.50728336e-13, -0.54958195e-6, -0.46089420e-10, 2.9659070);
      }
      else if((string)inst["SpacecraftName"] == "APOLLO 16") {
        // these values are for Apollo 16
        new ApolloMetricDistortionMap(this, -0.010, -0.004, -0.13678194e-5, 0.53824020e-9, -0.52793282e-13, 0.12275363e-5, -0.24596243e-9, 1.8859721);
      }
      else {
        string msg = "Apollo 17 cubes not yet supported by the camera model";
        throw iException::Message(iException::Camera, msg, _FILEINFO_);
      }

      // Setup the ground and sky map
      new CameraGroundMap(this);
      new CameraSkyMap(this);

      // Create a cache and grab spice info since it does not change for
      // a framing camera (fixed spacecraft position and pointing)
      // Get the start time in et
      string stime = inst["StartTime"];
      double time;
      str2et_c(stime.c_str(), &time);
      SetEphemerisTime(time);
      LoadCache();
    }
  }
}

extern "C" Isis::Camera *ApolloMetricCameraPlugin (Isis::Pvl &lab) {
  return new Isis::Apollo::ApolloMetricCamera(lab);
}
