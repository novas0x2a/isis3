#include "Isis.h"
#include "ControlNet.h"
#include "SerialNumberList.h"

using namespace std;
using namespace Isis;

// Main program
void IsisMain() {

  // Get user parameters
  UserInterface &ui = Application::GetUserInterface();
  ControlNet cnet( ui.GetFilename("CNET") );

  SerialNumberList snl;
  if ( ui.WasEntered("IGNORELIST") ) {
    snl = ui.GetFilename("IGNORELIST");
  }

  // Delete ignored points and measures
  for ( int cp = cnet.Size()-1; cp >= 0; cp -- ) {

    if ( ui.WasEntered("IGNORELIST") ) {
      // Compare each Serial Number listed with the serial number in the
      // Control Measure for according exclusion
      for ( int cm = 0; cm < cnet[cp].Size(); cm ++ ) {
        for ( int sn = 0; sn < snl.Size(); sn ++ ) {
          if ( snl.SerialNumber(sn) == cnet[cp][cm].CubeSerialNumber() ) {
            cnet[cp][cm].SetIgnore( true );
            break;
          }
        }
      }
    }

    if ( ui.GetBoolean("DELETE") ) {
      if ( cnet[cp].Ignore() ) {
        cnet.Delete(cp);
      }
      else {
        for ( int cm = cnet[cp].Size()-1; cm >= 0; cm -- ) {
          if ( cnet[cp][cm].Ignore() ) {
            cnet[cp].Delete(cm);
          }

          // Check if there are too few measures in the point
          if ( ( cnet[cp].Size() < 2 && cnet[cp].Type() != ControlPoint::Ground )
                                   || cnet[cp].Size() == 0 ) {
            cnet.Delete(cp);
          }

        }
      }
    }

  }

  cnet.Write( ui.GetFilename("ONET") );

}
