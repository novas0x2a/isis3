#include "Isis.h"

#include <sstream>

#include "FileList.h"
#include "ControlNet.h"
#include "Progress.h"
#include "iTime.h"

using namespace std;
using namespace Isis;

// Main program
void IsisMain() {
  // Get user parameters
  UserInterface &ui = Application::GetUserInterface();
  FileList filelist;
  if( ui.GetString("INPUTTYPE") == "LIST" ) {
    filelist = ui.GetFilename("FROMLIST");
  }
  else if ( ui.GetString("INPUTTYPE") == "CNETS" ) {
    filelist.push_back( ui.GetFilename("FROM1") );
    filelist.push_back( ui.GetFilename("FROM2") );
  }
  Filename outfile( ui.GetFilename("TO") );

  //Creates a Progress
  Progress progress;
  progress.SetMaximumSteps( filelist.size() );
  progress.CheckStatus();

  //Set up the output ControlNet with the first Control Net in the list
  ControlNet cnet( Filename(filelist[0]).Expanded() );
  cnet.SetNetworkId( ui.GetString("ID") );
  cnet.SetUserName( Isis::Application::UserName() );
  cnet.SetCreatedDate( Isis::Application::DateTime() );
  cnet.SetModifiedDate( Isis::iTime::CurrentLocalTime() );
  cnet.SetDescription( ui.GetString("DESCRIPTION") );

  progress.CheckStatus();

  ofstream ss;
  if( ui.WasEntered("REPORT") ) {
    string report = ui.GetFilename("REPORT");
    ss.open(report.c_str(),ios::out);
  }

  for ( int f = 1; f < (int)filelist.size(); f ++ ) {

    ControlNet currentnet( Filename(filelist[f]).Expanded() );

    //Checks to make sure the ControlNets are valid to merge
    if ( cnet.Target() != currentnet.Target() ) {
      string msg = "Input [" + currentnet.NetworkId() + "] does not target the ";
      msg += "same target as other Control Nets.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    /*else if( cnet.NetworkId() == currentnet.NetworkId() ) {
      string msg = "Inputs have the same Network Id of [";
      msg += cnet.NetworkId() + "] They are likely the same file.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }*/    

    //ERROR Merge
    if ( ui.GetString("DUPLICATEPOINTS") == "ERROR" ) {

      //Throws an error if there is a duplicate Control Point
      for ( int cp=0; cp<currentnet.Size(); cp++ ) {
        if ( cnet.Exists(currentnet[cp]) ) {
          string msg = "Inputs contain the same ControlPoint. [Id=";
          msg += currentnet[cp].Id() + "] Set DUPLICATEPOINTS=RENAME to force the";
          msg += " merge. Or, set DUPLICATEPOINTS=REPLACE to replace duplicate";
          msg += " Control Points.";
          throw iException::Message(iException::User,msg,_FILEINFO_);
        }
        //Adds the point to the output ControlNet
        cnet.Add( currentnet[cp] );
      }

    }
    //RENAME Merge
    else if ( ui.GetString("DUPLICATEPOINTS") == "RENAME" ) {

      //Adds the Control Net and gets the number of duplicate Control
      // Points to Log
      for ( int cp=0; cp<currentnet.Size(); cp++ ) {
        try {
          cnet.Find( currentnet[cp].Id() ); //This is the failing line
          if( ui.WasEntered("REPORT") ) {
            ss << "Control Point " << currentnet[cp].Id() << " from ";
            ss << currentnet.NetworkId() << " was renamed ";
            ss << currentnet.NetworkId() << currentnet[cp].Id() << endl;
          }
          currentnet[cp].SetId( currentnet.NetworkId() + currentnet[cp].Id() );
        } catch ( ... ) {
          //then currentnet[i] was not found and no renaming took place
        }
        //Adds the point to the output ControlNet
        cnet.Add( currentnet[cp] );
      }

    }
    //REPLACE Merge
    else if ( ui.GetString("DUPLICATEPOINTS") == "REPLACE" ) {

      //Adds currentnet to the ControlNet if it does not exist in cnet
      for ( int cp=0; cp<currentnet.Size(); cp++ ) {
        try {
          cnet.Find( currentnet[cp].Id() ); //This is the failing line
          if( ui.WasEntered("REPORT") ) {
            ss << "Control Point " << currentnet[cp].Id() << " was replaced from ";
            ss << currentnet.NetworkId() << endl;
          }
          cnet.Delete( currentnet[cp].Id() );
        } catch ( ... ) {
          //then currentnet[i] was not found as was not deleted
        }
        //Adds the point to the output ControlNet
        cnet.Add( currentnet[cp] );
      }

    }

    progress.CheckStatus();
  }

  //Writes out the final Control Net
  cnet.Write( outfile.Expanded() );

}
