#include "Isis.h"

#include <map>
#include <set>
#include <sstream>

#include "CameraFactory.h"
#include "ControlNet.h"
#include "ControlPoint.h"
#include "CubeManager.h"
#include "FileList.h"
#include "iException.h"
#include "iString.h"
#include "ProjectionFactory.h"
#include "Pvl.h"
#include "SerialNumber.h"
#include "UserInterface.h"

using namespace std; 
using namespace Isis;

bool NotInLatLonRange( double lat, double lon, double minlat,
                       double maxlat, double minlon, double maxlon );
void WriteCubeOutList( ControlNet cnet, map<iString,iString> sn2file );

// Main program
void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();

  // Gets the input parameters
  FileList inList( ui.GetFilename("FROMLIST") );
  ControlNet inNet( ui.GetFilename("CNET") );

  bool noIgnore        = ui.GetBoolean("NOIGNORE");
  bool noHeld          = ui.GetBoolean("NOHELD");
  bool noSingleMeasure = ui.GetBoolean("NOSINGLEMEASURE");
  bool reference       = ui.GetBoolean("REFERENCE");
  bool cubePoints      = ui.WasEntered("CUBEPOINTS");
  bool cubeMeasures    = ui.GetBoolean("CUBEMEASURES");
  bool pointsEntered   = ui.WasEntered("POINTLIST");
  bool latLon          = ui.GetBoolean("LATLON");

  if( !(noIgnore || noHeld || noSingleMeasure || reference || cubePoints || pointsEntered || latLon) ) {
    std::string msg = "At least one filter must be selected [";
    msg += "NOIGNORE,NOHELD,NOSINGLEMEASURE,REFERENCE,CUBEPOINTS,CUBEMEASURES,";
    msg += "POINTLIST,LATLON]";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }
  else if( cubeMeasures  &&  !cubePoints ) {
    std::string msg = "When CUBEMEASURES is selected, CUBEPOINTS must be given";
    msg += " a list of cubes.";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  // Set up the Serial Number to Filename mapping
  map<iString,iString> sn2filename;
  for( int cubeIndex=0; cubeIndex < (int)inList.size(); cubeIndex ++ ) {
    iString sn = SerialNumber::Compose( inList[cubeIndex] );
    sn2filename[sn] = inList[cubeIndex];
  }


  Progress progress;
  progress.SetMaximumSteps(inNet.Size());
  progress.CheckStatus();

  // Set up verctor records of how points/measures are removed
  PvlKeyword ignoredPoints( "IgnoredPoints" );
  PvlKeyword ignoredMeasures( "IgnoredMeasures" );
  PvlKeyword heldPoints( "HeldPoints" );
  PvlKeyword singlePoints( "SingleMeasurePoints" );
  PvlKeyword nonReferenceMeasures( "NonReferenseMeasures" );
  PvlKeyword nonCubePoints( "NonCubePoints" );
  PvlKeyword nonCubeMeasures( "NonCubeMeasures" );
  PvlKeyword noMeasurePoints( "NoMeasurePoints" );
  PvlKeyword nonListedPoints( "NonListedPoints" );
  PvlKeyword nonLatLonPoints( "LatLonOutOfRangePoints" );
  PvlKeyword cannotGenerateLatLonPoints( "NoLatLonPoints" );

  // Set up comparison data
  vector<iString> serialNumbers;
  if( cubePoints ) {
    FileList cubeList( ui.GetFilename("CUBEPOINTS") );
    for( int cubeIndex=0; cubeIndex < (int)cubeList.size(); cubeIndex ++ ) {
      iString sn = SerialNumber::Compose( cubeList[cubeIndex] );
      serialNumbers.push_back( sn );
    }
  }

  // Set up output values
  ControlNet outNet;
  outNet.SetType( inNet.Type() );
  outNet.SetTarget( inNet.Target() );

  if( ui.WasEntered("NETWORKID") )
    outNet.SetNetworkId( ui.GetString("NETWORKID") );
  else
    outNet.SetNetworkId( inNet.NetworkId() );

  outNet.SetUserName( Isis::Application::UserName() );
  outNet.SetDescription( ui.GetString("DESCRIPTION") );

  for(int cp=0; cp < inNet.Size(); cp ++) {
    progress.CheckStatus();

    // Do preliminary exclusion checks
    if( noIgnore && inNet[cp].Ignore() ) {
      ignoredPoints += inNet[cp].Id();
      continue;
    }
    else if( noHeld && inNet[cp].Held() ) {
      heldPoints += inNet[cp].Id();
      continue;
    }
    else if( noSingleMeasure ) {
      bool invalidPoint = false;
      invalidPoint |= noIgnore && (inNet[cp].NumValidMeasures() < 2);
      invalidPoint |= inNet[cp].Size() < 2 && (inNet[cp].Type() != ControlPoint::Ground);

      if( invalidPoint ) {
        singlePoints+= inNet[cp].Id();
        continue;
      }
    }

    // Begin constructing the new Control Point
    ControlPoint outPoint = inNet[cp];

    // Build the new Control Point
    for( int cm = outPoint.Size()-1; cm >= 0; cm --) {
      if(noIgnore && outPoint[cm].Ignore()) {
        ignoredMeasures += "(" + outPoint.Id() + "," + outPoint[cm].CubeSerialNumber() + ")";
        outPoint.Delete( cm );
      }
      else if( reference && !outPoint[cm].IsReference() ) {
        nonReferenceMeasures += "(" + outPoint.Id() + "," + outPoint[cm].CubeSerialNumber() + ")";
        outPoint.Delete( cm );
      }
      else if( cubeMeasures ) {
        bool hasSerialNumber = false;

        for( unsigned int sn = 0; sn < serialNumbers.size() && !hasSerialNumber; sn ++) {
          if(serialNumbers[sn] == outPoint[cm].CubeSerialNumber()) hasSerialNumber = true;
        }

        if( !hasSerialNumber ) { 
          nonCubeMeasures += "(" + outPoint.Id() + "," + outPoint[cm].CubeSerialNumber() + ")";
          outPoint.Delete( cm );
        }
      }
    }


    // Do not add outPoint if it has too few measures
    if( noSingleMeasure ) {
      bool invalidPoint = false;
      invalidPoint |= noIgnore && (outPoint.NumValidMeasures() < 2);
      invalidPoint |= outPoint.Size() < 2 && outPoint.Type() != ControlPoint::Ground;

      if( invalidPoint ) {
        singlePoints += inNet[cp].Id();
        continue;
      }
    }

    // Do not add outPoint if it does not have a cube in CUBEPOINTS as asked
    if( cubePoints && !cubeMeasures ) {
      bool hasSerialNumber = false;

      for( int cm = 0; cm < outPoint.Size() && !hasSerialNumber; cm ++) {
        for( unsigned int sn = 0; sn < serialNumbers.size() && !hasSerialNumber; sn ++) {
          if(serialNumbers[sn] == outPoint[cm].CubeSerialNumber()) hasSerialNumber = true;
        }
      }

      if( !hasSerialNumber ) {
        nonCubePoints += outPoint.Id();
        continue;
      }
    }

    if( outPoint.Size() == 0 ) {
      noMeasurePoints += outPoint.Id();
      continue;
    }

    outNet.Add( outPoint );
  }


  /**
   * Use another pass to check for Ids, since string comparisons are expensive
   */
  if( pointsEntered ) {
    FileList listedPoints( ui.GetFilename("POINTLIST") );

    for( int cp = outNet.Size()-1; cp >= 0; cp -- ) {
      bool isInList = false;
      for( int pointId = 0; pointId < (int)listedPoints.size()  &&  !isInList; pointId ++ ) {
        isInList = outNet[cp].Id().compare( listedPoints[pointId] ) == 0;
      }

      if( !isInList ) {
        nonListedPoints += outNet[cp].Id();
        outNet.Delete( cp );
      }
    }
  }


  /** 
   *  Use another pass on outNet, because this is by far the most time consuming
   *  process, and time could be saved by using the usually smaller outNet instead
   *  of inNet
   */
  if( latLon ) {
    double minlat = ui.GetDouble("MINLAT");
    double maxlat = ui.GetDouble("MAXLAT");
    double minlon = ui.GetDouble("MINLON");
    double maxlon = ui.GetDouble("MAXLON");

    Progress progress;
    progress.SetText("Calculating lat/lon");
    progress.SetMaximumSteps(outNet.Size());
    progress.CheckStatus();

    CubeManager manager;
    manager.SetNumOpenCubes( 50 ); //Should keep memory usage to around 1GB

    for( int cp = outNet.Size()-1; cp >= 0; cp --) {
      progress.CheckStatus();

      if( outNet[cp].Type() == Isis::ControlPoint::Ground ) {
        if( NotInLatLonRange( outNet[cp].UniversalLatitude(),
                              outNet[cp].UniversalLongitude(),
                              minlat, maxlat, minlon, maxlon ) ) {
          nonLatLonPoints += outNet[cp].Id();
          outNet.Delete( cp );
        }
      }

      /** 
       * If the point is not a Ground Point then we need to calculate lat/lon on our own
       */
      else {

        // Find a cube in the Control Point to get the lat/lon from
        int cm = 0;
        iString sn = "";
        double lat = 0.0;
        double lon = 0.0;
        double radius = 0.0;
  
        // First check the reference Measure
        if( outNet[cp].HasReference() ) {
          cm = outNet[cp].ReferenceIndex();
          if( !sn2filename[outNet[cp][cm].CubeSerialNumber()].empty() ) {
            sn = outNet[cp][cm].CubeSerialNumber();
          }
        }
  
        // Search for other Control Measures if needed
        if( sn.empty() ) {
          // Find the Serial Number if it exists
          for( int cm = 0; (cm < outNet[cp].Size()) && sn.empty(); cm ++ ) {
            if( !sn2filename[outNet[cp][cm].CubeSerialNumber()].empty() ) {
              sn = outNet[cp][cm].CubeSerialNumber();
            }
          }
        }
  
        // Connot fine a cube to get the lat/lon from
        if( sn.empty() ) {
          cannotGenerateLatLonPoints += outNet[cp].Id();
          outNet.Delete( cp );
        }
        
        // Calculate the lat/lon and check for validity
        else {
          bool remove = false;

          Cube *cube = manager.OpenCube( sn2filename[sn] );
          Camera *camera = cube->Camera();

          if (camera == NULL) {
            try {
              Projection *projection = ProjectionFactory::Create( (*(cube->Label())) );

              if(!projection->SetCoordinate(outNet[cp][cm].Sample(),outNet[cp][cm].Line())) {
                nonLatLonPoints += outNet[cp].Id();
                remove = true;
              }

              lat = projection->Latitude();
              lon = projection->Longitude();
              radius = projection->LocalRadius();

              delete projection;
              projection = NULL;
            } catch ( iException &e ) {
              remove = true;
              e.Clear();
            }
          }
          else {
            if(!camera->SetImage(outNet[cp][cm].Sample(),outNet[cp][cm].Line())) {
              nonLatLonPoints += outNet[cp].Id();
              remove = true;
            }

            lat = camera->UniversalLatitude();
            lon = camera->UniversalLongitude();
            radius = camera->LocalRadius();

            camera = NULL;
          }

          cube = NULL;
  
          if( remove  ||  NotInLatLonRange( lat, lon, minlat, maxlat, minlon, maxlon ) ) {
            nonLatLonPoints += outNet[cp].Id();
            outNet.Delete( cp );
          }
          else { // Add the reference lat/lon/radius to the Control Point
            outNet[cp].SetUniversalGround( lat, lon, radius );
          }
        }
      }

    }

    manager.CleanCubes();
  }



  // Write the filenames associated with outNet
  WriteCubeOutList( outNet, sn2filename );

  Progress outProgress;
  outProgress.SetText("Writing Control Network");
  outProgress.SetMaximumSteps( 5 );
  outProgress.CheckStatus();

  // Create the points included file
  PvlGroup included("NewControlNet");
  included.AddKeyword( PvlKeyword( "Size", outNet.Size() ) );
  PvlKeyword newPoints( "Points" );
  for( int cp = 0; cp < outNet.Size(); cp ++ ) {
    newPoints.AddValue( outNet[cp].Id() );
  }
  included.AddKeyword( newPoints );

  outProgress.CheckStatus();

  // Write the extracted Control Network
  outNet.Write( ui.GetFilename("OUTNET") );

  outProgress.CheckStatus();

  // Adds the remove history to the summary and results group
  PvlGroup summary("ResultSummary");
  PvlGroup results("Results");

  if( noIgnore ) {
    summary.AddKeyword( PvlKeyword( "IgnoredPoints", ignoredPoints.Size() ) ); 
    results.AddKeyword( ignoredPoints );
    summary.AddKeyword( PvlKeyword( "IgnoredMeasures", ignoredMeasures.Size() ) ); 
    results.AddKeyword( ignoredMeasures );
  }
  if( noHeld ) {
    summary.AddKeyword( PvlKeyword( "HeldPoints", heldPoints.Size() ) ); 
    results.AddKeyword( heldPoints );
  }
  if( noSingleMeasure ) {
    summary.AddKeyword( PvlKeyword( "SingleMeasurePoints", singlePoints.Size() ) ); 
    results.AddKeyword( singlePoints );
  }
  if( reference ) {
    summary.AddKeyword( PvlKeyword( "NonReferenceMeasures", nonReferenceMeasures.Size() ) ); 
    results.AddKeyword( nonReferenceMeasures );
  }
  if( cubePoints ) {
    summary.AddKeyword( PvlKeyword( "NonCubePoints", nonCubePoints.Size() ) ); 
    results.AddKeyword( nonCubePoints );
  }
  if( noMeasurePoints.Size() != 0 ) {
    summary.AddKeyword( PvlKeyword( "NonCubeMeasure", noMeasurePoints.Size() ) ); 
    results.AddKeyword( noMeasurePoints );
  }
  if( cubeMeasures ) {
    summary.AddKeyword( PvlKeyword( "NoMeasurePoints", nonCubeMeasures.Size() ) ); 
    results.AddKeyword( nonCubeMeasures );
  }
  if( pointsEntered ) {
    summary.AddKeyword( PvlKeyword( "NonListedPoints", nonListedPoints.Size() ) ); 
    results.AddKeyword( nonListedPoints );
  }
  if( latLon ) {
    summary.AddKeyword( PvlKeyword( "LatLonOutOfRange", nonLatLonPoints.Size() ) ); 
    results.AddKeyword( nonLatLonPoints );
    summary.AddKeyword( PvlKeyword( "NoLatLonPoints", cannotGenerateLatLonPoints.Size() ) ); 
    results.AddKeyword( cannotGenerateLatLonPoints );
  }

  outProgress.CheckStatus();
  
  // Log Control Net results
  Application::Log(included);
  Application::Log(summary);
  results.AddComment( "Each keyword represents a filter parameter used." \
                      " Check the documentation for specific keyword descriptions." );
  Application::Log(results);

  outProgress.CheckStatus();

}



/**
 * Checks for correct lat/lon range, handling the meridian correctly
 *  
 * @param lat The latitude to check 
 * @param lon The longitude to check
 * @param minlat Minimum Latitude Minimum valid latitude
 * @param maxlat Maximum Latitude Maximum valid latitude
 * @param minlon Minimum Longitude Minimum valid longitude
 * @param maxlon Maximum Longitude Maximum valid longitude
 * 
 * @return bool True when the range is valid
 */
bool NotInLatLonRange( double lat, double lon, double minlat,
                       double maxlat, double minlon, double maxlon ) {
  bool inRange = true;

  // Check latitude range
  if( inRange && minlat > maxlat ) {
    inRange &= (lat <= maxlat || lat >= minlat);
  }
  else if( inRange ) {
    inRange &= (lat >= minlat && lat <= maxlat);
  }

  // Check longitude range
  if( inRange && minlon > maxlon ) {
    inRange &= (lon <= maxlon || lon >= minlon);
  }
  else if( inRange ){
    inRange &= (lon >= minlon && lon <= maxlon);
  }

  return !inRange;
}


/**
 * Finds and writes all input cubes contained within the given Control Network 
 * to the output file list 
 * 
 * @param cnet The Control Network to list the filenames contained within
 * @param sn2file The map for converting the Control Network's serial numbers 
 *                to filenames 
 */
void WriteCubeOutList( ControlNet cnet, map<iString,iString> sn2file ) {
  UserInterface &ui = Application::GetUserInterface();

  Progress p;
  p.SetText("Writing Cube List");
  try {
    p.SetMaximumSteps(cnet.Size());
    p.CheckStatus();
  } catch( iException &e ) {
    e.Clear();
    string msg = "The provided filters have resulted in an empty Control Network.";
    throw Isis::iException::Message(Isis::iException::User,msg, _FILEINFO_);
  }

  set<iString> outputsn;
  for( int cp = 0; cp < cnet.Size(); cp ++ ) {
    for( int cm = 0; cm < cnet[cp].Size(); cm ++ ) {
      outputsn.insert( cnet[cp][cm].CubeSerialNumber() );
    }
    p.CheckStatus();
  }

  std::string toList = ui.GetFilename("TOLIST");
  ofstream out_stream;
  out_stream.open(toList.c_str(), std::ios::out);
  out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file

  for( set<iString>::iterator sn = outputsn.begin(); sn != outputsn.end(); sn ++ ) {
    if( !sn2file[(*sn)].empty() ) {
      out_stream << sn2file[(*sn)] << endl;
    }
  }

  out_stream.close();
}

