/**                                                                       
 * @file                                                                  
 * $Revision: 1.4 $                                                             
 * $Date: 2010/03/05 23:15:52 $                                 
 *                                                                        
 *   Unless noted otherwise, the portions of Isis written by the USGS are 
 *   public domain. See individual third-party library and package descriptions 
 *   for intellectual property information, user agreements, and related  
 *   information.                                                         
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or   
 *   implied, is made by the USGS as to the accuracy and functioning of such 
 *   software and related material nor shall the fact of distribution     
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.                                        
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html                
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       
#include "Brick.h"
#include "Camera.h"
#include "CameraPointInfo.h"
#include "CubeManager.h"
#include "iException.h"
#include "iTime.h"
#include "PvlGroup.h"

using namespace std;

namespace Isis {


  /**
   * Constructor, initializes CubeManager and other variables for
   * use.
   * 
   */
  CameraPointInfo::CameraPointInfo() {
    usedCubes = NULL;
    outsideAccepted = false;
    usedCubes = new CubeManager();
    usedCubes->SetNumOpenCubes(50);
    currentCube = NULL;
    camera = NULL;
  }

  /**
   * Destructor, deletes CubeManager object used.
   * 
   */
  CameraPointInfo::~CameraPointInfo() {
    if (usedCubes) {
      delete usedCubes;
      usedCubes = NULL;
    }
  }


  /**
   * SetCube opens the given cube in a CubeManager. The 
   * CubeManager is for effeciency when working with control nets 
   * where cubes are accesed multiple times. 
   * 
   * @param cubeFilename A cube filename
   */
  void CameraPointInfo::SetCube(const std::string  & cubeFilename) {
    currentCube = usedCubes->OpenCube(cubeFilename);
    camera = currentCube->Camera();
  }


  /**
   * SetImage sets a sample, line image coordinate in the camera 
   * so data can be accessed. 
   * 
   * @param sample A sample coordinate in or almost in the cube
   * @param line A line coordinate in or almost in the cubei 
   *  
   * @return PvlGroup* The pertinent data from the Camera class on 
   *         the point. Ownership is passed to caller.
   */
  PvlGroup * CameraPointInfo::SetImage(double sample, double line) {
    if (CheckCube()) {
      camera->SetImage(sample, line);

      return GetPointInfo();
    }
    return NULL;
  }
 
  /**
   * SetCenter sets the image coordinates to the center of the image.
   *
   * @return PvlGroup* The pertinent data from the Camera class on 
   *         the point. Ownership is passed to caller.
   */
  PvlGroup * CameraPointInfo::SetCenter() {
    if (CheckCube()) {
      camera->SetImage(currentCube->Samples()/2, currentCube->Lines()/2);

      return GetPointInfo();
    }
    return NULL;
  }


  /**
   * SetSample sets the image coordinates to the center line and the 
   * given sample.
   *
   * @return PvlGroup* The pertinent data from the Camera class on 
   *         the point. Ownership is passed to caller.
   */
  PvlGroup * CameraPointInfo::SetSample(double sample) {
    if (CheckCube()) {
      camera->SetImage(sample, currentCube->Lines()/2);

      return GetPointInfo();
    }
    return NULL;
  }


  /**
   * SetLine sets the image coordinates to the center sample and the
   * given line.
   *
   * @return PvlGroup* The pertinent data from the Camera class on 
   *         the point. Ownership is passed to caller.
   */
  PvlGroup * CameraPointInfo::SetLine(double line) {
    if (CheckCube()) {
      camera->SetImage(currentCube->Samples()/2, line);

      return GetPointInfo();
    }
    return NULL;
  }


  /**
   * SetGround sets a latitude, longitude grrund coordinate in the 
   * camera so data can be accessed. 
   * 
   * @param latitude A latitude coordinate in or almost in the 
   *                 cube
   * @param longitude A longitude coordinate in or almost in the 
   *                  cube
   * 
   * @return PvlGroup* The pertinent data from the Camera class on 
   *         the point. Ownership is passed to caller.
   */
  PvlGroup * CameraPointInfo::SetGround(const double latitude, const double longitude) {
    if (CheckCube()) {
      camera->SetUniversalGround(latitude, longitude);

      return GetPointInfo();
    }
    return NULL;
  }


  /**
   * CheckCube checks that a cube has been set before the data for 
   * a point is accessed.
   *
   * @return bool Whether or not a cube has been set, true if it has been.
   */ 
  bool CameraPointInfo::CheckCube() {
    if (currentCube == NULL) {
      string msg = "Please set a cube before setting parameters";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
      return false;
    }
    return true;
  }

  /**
   * GetPointInfo builds the PvlGroup containing all the important 
   * information derived from the Camera.  
   * 
   * @return PvlGroup* Data taken directly from the Camera and 
   *         drived from Camera information. Ownership passed.
   */
  PvlGroup * CameraPointInfo::GetPointInfo() {
    CheckConditions();

    Brick b(3,3,1,currentCube->PixelType());
    
    int intSamp = (int)(camera->Sample() + 0.5);
    int intLine = (int)(camera->Line() + 0.5);
    b.SetBasePosition(intSamp, intLine, 1);
    currentCube->Read(b);

    double pB[3], spB[3], sB[3];
    string utc;
    double ssplat, ssplon, sslat, sslon, pwlon, oglat;

    // Create group with ground position
    PvlGroup * gp = new PvlGroup("GroundPoint");
    {
      gp->AddKeyword(PvlKeyword("Filename",currentCube->Filename()));
      gp->AddKeyword(PvlKeyword("Sample",camera->Sample()));
      gp->AddKeyword(PvlKeyword("Line",camera->Line()));
      gp->AddKeyword(PvlKeyword("PixelValue",PixelToString(b[0])));
      gp->AddKeyword(PvlKeyword("RightAscension",camera->RightAscension()));
      gp->AddKeyword(PvlKeyword("Declination",camera->Declination()));
      gp->AddKeyword(PvlKeyword("PlanetocentricLatitude",
                                camera->UniversalLatitude()));
  
      // Convert lat to planetographic
      double radii[3];
      camera->Radii(radii);
      oglat = Isis::Projection::ToPlanetographic(camera->UniversalLatitude(),
                                                 radii[0],radii[2]);
      gp->AddKeyword(PvlKeyword("PlanetographicLatitude",oglat));
      
      gp->AddKeyword(PvlKeyword("PositiveEast360Longitude",
                                camera->UniversalLongitude()));
  
      //Convert lon to -180 - 180 range
      gp->AddKeyword(PvlKeyword("PositiveEast180Longitude",
                                Isis::Projection::To180Domain(
                                  camera->UniversalLongitude())));

      //Convert lon to positive west
      pwlon = Isis::Projection::ToPositiveWest(camera->UniversalLongitude(),
                                               360);
      gp->AddKeyword(PvlKeyword("PositiveWest360Longitude",pwlon));

      //Convert pwlon to -180 - 180 range
      gp->AddKeyword(PvlKeyword("PositiveWest180Longitude",
                                Isis::Projection::To180Domain(pwlon)));
  
      camera->Coordinate(pB);
      PvlKeyword coord("BodyFixedCoordinate");
      coord.AddValue(pB[0],"km");
      coord.AddValue(pB[1],"km");
      coord.AddValue(pB[2],"km");
      gp->AddKeyword(coord);
  
      gp->AddKeyword(PvlKeyword("LocalRadius",camera->LocalRadius(),"m"));
      gp->AddKeyword(PvlKeyword("SampleResolution",camera->SampleResolution(),"m"));
      gp->AddKeyword(PvlKeyword("LineResolution",camera->LineResolution(),"m"));
  
      camera->InstrumentPosition(spB);
      PvlKeyword spcoord("SpacecraftPosition");
      spcoord.AddValue(spB[0],"km");
      spcoord.AddValue(spB[1],"km");
      spcoord.AddValue(spB[2],"km");
      spcoord.AddComment("Spacecraft Information");
      gp->AddKeyword(spcoord);
  
  
      gp->AddKeyword(PvlKeyword("SpacecraftAzimuth",camera->SpacecraftAzimuth()));
      gp->AddKeyword(PvlKeyword("SlantDistance",camera->SlantDistance(),"km"));
      gp->AddKeyword(PvlKeyword("TargetCenterDistance",camera->TargetCenterDistance(),"km"));
      camera->SubSpacecraftPoint(ssplat,ssplon);
      gp->AddKeyword(PvlKeyword("SubSpacecraftLatitude",ssplat));
      gp->AddKeyword(PvlKeyword("SubSpacecraftLongitude",ssplon));
      gp->AddKeyword(PvlKeyword("SpacecraftAltitude",camera->SpacecraftAltitude(),"km"));
      gp->AddKeyword(PvlKeyword("OffNadirAngle",camera->OffNadirAngle()));
      double subspcgrdaz;
      subspcgrdaz = camera->GroundAzimuth(camera->UniversalLatitude(),camera->UniversalLongitude(),
        ssplat,ssplon);
      gp->AddKeyword(PvlKeyword("SubSpacecraftGroundAzimuth",subspcgrdaz));
  
      camera->SunPosition(sB);
      PvlKeyword scoord("SunPosition");
      scoord.AddValue(sB[0],"km");
      scoord.AddValue(sB[1],"km");
      scoord.AddValue(sB[2],"km");
      scoord.AddComment("Sun Information");
      gp->AddKeyword(scoord);
  
      gp->AddKeyword(PvlKeyword("SubSolarAzimuth",camera->SunAzimuth()));
      gp->AddKeyword(PvlKeyword("SolarDistance",camera->SolarDistance(),"AU"));
      camera->SubSolarPoint(sslat,sslon);
      gp->AddKeyword(PvlKeyword("SubSolarLatitude",sslat));
      gp->AddKeyword(PvlKeyword("SubSolarLongitude",sslon));
      double subsolgrdaz;
      subsolgrdaz = camera->GroundAzimuth(camera->UniversalLatitude(),camera->UniversalLongitude(),
        sslat,sslon);
      gp->AddKeyword(PvlKeyword("SubSolarGroundAzimuth",subsolgrdaz));
  
      PvlKeyword phase("Phase",camera->PhaseAngle());
      phase.AddComment("Illumination and Other");
      gp->AddKeyword(phase);
      gp->AddKeyword(PvlKeyword("Incidence",camera->IncidenceAngle()));
      gp->AddKeyword(PvlKeyword("Emission",camera->EmissionAngle()));
      gp->AddKeyword(PvlKeyword("NorthAzimuth",camera->NorthAzimuth()));
  
      PvlKeyword et("EphemerisTime",camera->EphemerisTime(),"seconds");
      et.AddComment("Time");
      gp->AddKeyword(et);
      iTime t(camera->EphemerisTime());
      utc = t.UTC();
      gp->AddKeyword(PvlKeyword("UTC",utc));
      gp->AddKeyword(PvlKeyword("LocalSolarTime",camera->LocalSolarTime(),"hour"));
      gp->AddKeyword(PvlKeyword("SolarLongitude",camera->SolarLongitude()));
    }
    return gp;
  }


  /**
   * Checks two possible failure conditions involving no surface
   * intersection and not being inside the cube.
   * 
   */
  void CameraPointInfo::CheckConditions() {
    if (!camera->HasSurfaceIntersection()) {
      string msg = "Requested position does not project in camera model, no surface intersection";
      throw iException::Message(iException::Camera, msg, _FILEINFO_);
    }
    if(!camera->InCube() && !outsideAccepted) {
      string msg = "Requested position does not project in camera model, not inside cube";
      throw iException::Message(iException::Camera ,msg, _FILEINFO_);
    } 
  }


  /** 
   * Sets boolean value if outside cube locations are acceptable. 
   *  
   * @param outside Boolean setting if locations outside are 
   *                acceptable.
   */
  void CameraPointInfo::AllowOutside(bool outside) {
    outsideAccepted = outside;
  }


  /**
   * Returns whether or not locations outside the cube are 
   * acceptable. 
   * 
   * @return bool Outside cube locations acceptable. 
   */
  bool CameraPointInfo::AllowOutside() {
    return outsideAccepted;
  }
}
