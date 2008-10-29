#include "Isis.h"
#include "ControlNet.h"

using namespace std;
using namespace Isis;

// Main program
void IsisMain() {
  // Get user parameters
  UserInterface &ui = Application::GetUserInterface();
  ControlNet c1( ui.GetFilename("FROM1") );
  ControlNet c2( ui.GetFilename("FROM2") );
  Filename outfile( ui.GetFilename("TO") );

  //Checks to make sure the ControlNets are valid to merge
  if( c1.Target() != c2.Target() ) {
    string msg = "Inputs do not target the same target.";
    msg += " FROM1 target = [" + c1.Target() + "]";
    msg += " FROM2 target = [" + c2.Target() + "]";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }
  else if( c1.NetworkId() == c2.NetworkId() ) {
    string msg = "Inputs have the same Network Id of [";
    msg += c1.NetworkId() + "] They are likely the same file.";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }
  else if( c1.Type() != c2.Type() ) {
    string msg = "Inputs are not of the same Network Type. FROM1=[";
    msg += iString(c1.Type()) + "] FROM2=[" + iString(c2.Type()) + "]";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  //If not force, then checks and throws an error if a ControlPoint Id is shared
  if( not ui.GetBoolean("FORCE") ) {
    for( int i=0; i<c2.Size(); i++ ) {
      if( c1.Exists(c2[i]) ) {
        string msg = "Inputs contain the same ControlPoint. [Id=";
        msg += c2[i].Id() + "] Set FORCE=true to force the merge.";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }
    }
  }

  //Set up the output ControlNet
  ControlNet cnet;
  cnet.SetNetworkId( ui.GetString("ID") );
  cnet.SetType( c1.Type() );
  cnet.SetTarget( c1.Target() );
  cnet.SetUserName( Isis::Application::UserName() );
  cnet.SetCreatedDate( Isis::Application::DateTime() );
  cnet.SetModifiedDate( Isis::iTime::CurrentLocalTime() );
  cnet.SetDescription( ui.GetString("DESCRIPTION") );

  //Adds c1 to the ControlNet
  for( int i=0; i<c1.Size(); i++ ) {
    c1[i].SetId( c1.NetworkId() + c1[i].Id() );
    cnet.Add( c1[i] );
  }
  //Adds c2 to the ControlNet
  for( int i=0; i<c2.Size(); i++ ) {
    c2[i].SetId( c2.NetworkId() + c2[i].Id() );
    cnet.Add( c2[i] );
  }

  cnet.Write( outfile.Expanded() );

}
