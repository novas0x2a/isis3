using namespace std;

#include "ThemisIrCamera.h"
#include "LineScanCameraDetectorMap.h"
#include "CameraFocalPlaneMap.h"
#include "ThemisIrDistortionMap.h"
#include "LineScanCameraGroundMap.h"
#include "LineScanCameraSkyMap.h"

namespace Isis {
  namespace Odyssey {
    ThemisIrCamera::ThemisIrCamera (Isis::Pvl &lab) : Isis::Camera(lab) {
      // Set up the camera characteristics
      // LoadFrameMounting("M01_SPACECRAFT","M01_THEMIS_IR");
      SetFocalLength(203.9);
      SetPixelPitch(0.05);

      // Get the start time
      Isis::PvlGroup &inst = lab.FindGroup ("Instrument",Isis::Pvl::Traverse);
      string stime = inst["SpacecraftClockCount"];
      scs2e_c (NaifSclkCode(),stime.c_str(),&p_etStart);
      double offset = inst["SpacecraftClockOffset"];
      p_etStart += offset;

      // Get the keywords from labels
      p_tdiMode = (string) inst["TimeDelayIntegration"];
      Isis::PvlGroup &bandBin = lab.FindGroup("BandBin",Isis::Pvl::Traverse);
      Isis::PvlKeyword &orgBand = bandBin["OriginalBand"];
      for (int i=0; i<orgBand.Size(); i++) {
        p_originalBand.push_back(orgBand[i]);
      }

      int sumMode = 1;
      if (inst.HasKeyword("SpatialSumming")) {
        sumMode = inst["SpatialSumming"];
      }

      // Setup detector map
      p_lineRate = 33.2804 / 1000.0 * sumMode;
      LineScanCameraDetectorMap *detectorMap = new LineScanCameraDetectorMap(this,p_etStart,p_lineRate);
      detectorMap->SetDetectorSampleSumming(sumMode);
      detectorMap->SetDetectorLineSumming(sumMode);

      // Setup focal plane map
      CameraFocalPlaneMap *focalMap = new CameraFocalPlaneMap(this,NaifIkCode());
      focalMap->SetDetectorOrigin(164.25,0.0);

      // Setup distortion map
      new ThemisIrDistortionMap(this);

      // Setup the ground and sky map
      new LineScanCameraGroundMap(this);
      new LineScanCameraSkyMap(this);

      LoadCache();
    }

    void ThemisIrCamera::SetBand (const int vband) {
      // Constants
      int bandFirstRow[] = {1,17,43,69,95,121,147,173,198,224};
      int bandLastRow[] = {16,32,58,84,110,136,162,188,213,239};
      int bandRowTdiOffset[] = {9,24,52,77,102,129,155,181,206,232};
      double cxDistortion = -2.54;
      double cyDistortion[] = {1.2562,1.0636,0.6351,0.2397,0.0,-0.1207,-0.2136,
                               -0.2183,-0.2228,-0.2275};

      // Lookup the original band from the band bin group.  Unless there is
      // a reference band which means the data has all been aligned in the
      // band dimension
      int band = p_originalBand[vband-1];
      if (HasReferenceBand()) {
        band = ReferenceBand();
      }

      // Compute factors for applying corrections for optical distortions
      // for the sample (x) direction
      double epsilon = (cxDistortion/320.0) *
                       (
                       ( ((double)(bandFirstRow[band-1] + bandLastRow[band-1])/2.0)
                       - ((double)(bandFirstRow[5-1] + bandLastRow[5-1])/2.0) )
                       /
                       ( ((double)(bandFirstRow[9-1] + bandLastRow[9-1])/2.0)
                       - ((double)(bandFirstRow[1-1] + bandLastRow[1-1])/2.0) )
                              ) ;
      double cx1 = 1.0 + epsilon;
      double cy = cyDistortion[band-1];

      ThemisIrDistortionMap *distortionMap =
        (ThemisIrDistortionMap *) this->DistortionMap();
      distortionMap->SetDistortion(cx1,cy);

      // Figure out the detector line in the CCD
      double detectorLine = (double) bandRowTdiOffset[band-1];
      if (p_tdiMode == "ENABLED") {
        // Average of detector lines used to accumulate an image line
        detectorLine = (double)(bandFirstRow[band-1] + bandLastRow[band-1]) / 2.0;
      }

      // Compute the time offset for this detector line
      double exposureDuration = 16.6402 / 1000.0;      // seconds
      p_bandTimeOffset = (detectorLine - 1.0) * p_lineRate -
                          exposureDuration / 2.0;
      LineScanCameraDetectorMap *detectorMap =
        (LineScanCameraDetectorMap *) this->DetectorMap();
      detectorMap->SetStartTime(p_etStart + p_bandTimeOffset);

      // Compute the along track offset at this detector line
      double alongtrackOffset = 109.5 - detectorLine;
      this->FocalPlaneMap()->SetDetectorOffset(0.0,alongtrackOffset);
    }
  }
}

// Plugin
extern "C" Isis::Camera *ThemisIrCameraPlugin (Isis::Pvl &lab) {
  return new Isis::Odyssey::ThemisIrCamera(lab);
}
