/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2007/01/30 22:12:22 $                                                                 
 *                                                                        
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for 
 *   intellectual property information,user agreements, and related information.
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software 
 *   and related material nor shall the fact of distribution constitute any such 
 *   warranty, and no responsibility is assumed by the USGS in connection 
 *   therewith.                                                           
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see 
 *   the Privacy &amp; Disclaimers page on the Isis website,              
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include "CameraFactory.h"
#include "Camera.h"
#include "Plugin.h"
#include "iException.h"
#include "Filename.h"

using namespace std;
namespace Isis {
 /**
  * Creates a Camera object using Pvl Specifications
  * 
  * @param lab Pvl label containing specifications for the Camera object
  * 
  * @return Camera* The Camera object created
  * 
  * @throws Isis::iException::System - Unsupported camera model, unable to find
  *                                    the plugin
  * @throws Isis::iException::Camera - Unable to initialize camera model
  */
  Camera *CameraFactory::Create(Isis::Pvl &lab) {
    // Try to load a plugin file in the current working directory and then
    // load the system file
    Plugin p;
    Filename localFile("Camera.plugin");
    if (localFile.Exists()) p.Read(localFile.Expanded());
    Filename systemFile("$ISISROOT/lib/Camera.plugin");
    if (systemFile.Exists()) p.Read(systemFile.Expanded());
  
    try {
      // First get the spacecraft and instrument and combine them
      PvlGroup &inst = lab.FindGroup("Instrument",Isis::Pvl::Traverse);
      iString spacecraft = (string) inst["SpacecraftName"];
      iString name = (string) inst["InstrumentId"];
      spacecraft.UpCase();
      name.UpCase();
      iString group = spacecraft + "/" + name;
      group.Remove(" ");
  
      // See if we have a camera model plugin
      void *ptr;
      try {
        ptr = p.GetPlugin(group);
      }
      catch (Isis::iException &e) {
        string msg = "Unsupported camera model, unable to find plugin for ";
        msg += "SpacecraftName [" + spacecraft + "] with InstrumentId [";
        msg += name + "]";
        throw Isis::iException::Message(Isis::iException::System,msg,_FILEINFO_);
      }
  
      // Now cast that pointer in the proper way
      Camera * (*plugin) (Isis::Pvl &lab);
      plugin = (Camera * (*)(Isis::Pvl &lab)) ptr;
      
      // Create the projection as requested
      return (*plugin)(lab);
    }
    catch (Isis::iException &e) {
      string message = "Unable to initialize camera model from group [Instrument]";
      throw Isis::iException::Message(Isis::iException::Camera,message,_FILEINFO_);
    }
  }
} // end namespace isis


