#include "SsiCamera.h"
#include "CameraDetectorMap.h"
#include "CameraFocalPlaneMap.h"
#include "CameraDistortionMap.h"
#include "CameraGroundMap.h"
#include "CameraSkyMap.h"
#include "RadialDistortionMap.h"
#include "iString.h"

using namespace std;
using namespace Isis;
namespace Galileo {
  SsiCamera::SsiCamera (Pvl &lab) : Camera(lab) {
    // Get the camera characteristics
    SetFocalLength ();
    SetPixelPitch ();

    // Get the start time in et
    PvlGroup inst = lab.FindGroup ("Instrument",Pvl::Traverse);
    string stime = inst["StartTime"];
    double et; 
    str2et_c(stime.c_str(),&et);

    // Get summation mode
    double sumMode = inst["Summing"];

    // Setup detector map
    CameraDetectorMap *detectorMap = new CameraDetectorMap(this);
    detectorMap->SetDetectorSampleSumming(sumMode);
    detectorMap->SetDetectorLineSumming(sumMode);

    // Setup focal plane map
    CameraFocalPlaneMap *focalMap = new CameraFocalPlaneMap(this,NaifIkCode());

    focalMap->SetDetectorOrigin (Spice::GetDouble("INS" + (iString)(int)NaifIkCode() + "_BORESIGHT_SAMPLE"), 
                                 Spice::GetDouble("INS" + (iString)(int)NaifIkCode() + "_BORESIGHT_LINE"));

    // Setup distortion map
    double k1 = Spice::GetDouble("INS" + (iString)(int)NaifIkCode() + "_K1");
    new RadialDistortionMap(this, k1);

    // Setup the ground and sky map
    new CameraGroundMap(this);
    new CameraSkyMap(this);
  
    SetEphemerisTime(et);
    LoadCache();
  }
}

extern "C" Camera *SsiCameraPlugin(Pvl &lab) {
  return new Galileo::SsiCamera(lab);
}
