/**
 * @file
 * $Revision: 1.2 $
 * $Date: 2007/07/19 22:49:55 $
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

#include <QLibrary>
#include "iException.h"
#include "Plugin.h"

using namespace std;
namespace Isis {
  //! Constructs a Plugin object.
  Plugin::Plugin () : Isis::Pvl() {
  }

/**
 * This method returns a void pointer to a C function (i.e., the plugin) It
 * does this by looking in itself (Pvl) for the keyword LIBRARY and the
 * keyword ROUTINE.  When you write this function make sure to place extern
 * "C" infront of it to eliminate C++ symbol mangling.
 *
 * @param group The group name.
 *
 * @return A void pointer to a C function (i.e., the plugin)
 */
  void *Plugin::GetPlugin (const std::string &group) {
  // Get the library and plugin to load
    Isis::PvlGroup &g = FindGroup(group);
    string library = g["Library"];
    string pluginName = g["Routine"];

    // Open the library, resolve the routine name, and return the function
    // address. The function will stay in memory until the application exists
    // so the scope of lib does not matter.
    QLibrary lib(library.c_str());
    void *plugin = lib.resolve(pluginName.c_str());
    if (plugin == 0) {
      string msg = "Unable to find plugin [" + pluginName +
                   "] in shared library [" + lib.fileName().toStdString() + "]";
      throw Isis::iException::Message(Isis::iException::System,msg,_FILEINFO_);
    }

    return plugin;
  }
} // end namespace isis

extern "C" int *PluginPlugin () {
  cout << "Hello world" << endl;
  static int k = 5;
  return &k;
}