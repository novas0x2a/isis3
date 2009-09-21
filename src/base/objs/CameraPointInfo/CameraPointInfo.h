/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2009/09/14 05:51:06 $                                                                 
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
#ifndef CameraPointInfo_h
#define CameraPointInfo_h

#include <string>

namespace Isis {
  class CubeManager;
  class Cube;
  class Camera;
  class PvlGroup;


  /**
   * @brief CameraPointInfo provides quick access to the majority of 
   *        information avaliable from a camera on a point.
   *  
   * CameraPointInfo provides the functionality which was a part of 
   * campt in class form. This functionality is access to the 
   * majoirty of information avaliable on any given point on an 
   * image. The main difference is the use of a CubeManager within
   * CameraPointInfo for effeciency when working with control nets and 
   * the opening of cubes several times. 
   * 
   * @author 2009-08-25 Mackenzie Boyd
   *
   * @internal
   *   @history 2009-09-13 Mackenzie Boyd - Added methods SetCenter, SetSample
   *                                        and SetLine to support campt
   *                                        functionality. Added CheckCube
   *                                        private method to check
   *                                        currentCube isn't NULL.
   */
  class CameraPointInfo {

    public:
      CameraPointInfo();
      virtual ~CameraPointInfo();

      void SetCube(const std::string & cubeFilename);
      PvlGroup * SetImage(const double sample, const double line);
      PvlGroup * SetCenter();
      PvlGroup * SetSample(double sample);
      PvlGroup * SetLine(double line);
      PvlGroup * SetGround(const double latitude, const double longitude);
      void AllowOutside(const bool outsideAccepted);
      bool AllowOutside();

    private:
      bool CheckCube();
      void CheckConditions();
      PvlGroup * GetPointInfo();

      CubeManager * usedCubes;
      bool outsideAccepted;
      Cube * currentCube;
      Camera * camera;
  };
};

#endif
