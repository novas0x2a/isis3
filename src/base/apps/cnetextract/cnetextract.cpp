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

void ExtractPointList( ControlNet & outNet, PvlKeyword & nonListedPoints );
void ExtractLatLonRange( ControlNet & outNet, PvlKeyword & noLatLonPoints,
                         PvlKeyword & cannotGenerateLatLonPoints, map<iString,iString> sn2filename );
bool NotInLatLonRange( double lat, double lon, double minlat,
                       double maxlat, double minlon, double maxlon );
void WriteCubeOutList( ControlNet cnet, map<iString,iString> sn2file );

// Main program
void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();

  if( !ui.WasEntered("FROMLIST") && ui.WasEntered("TOLIST") ) {
    std::string msg = "To create a [TOLIST] the [FROMLIST] parameter must be provided.";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  // Gets the input parameters
  ControlNet outNet( ui.GetFilename("CNET") );
  FileList inList;
  if( ui.WasEntered("FROMLIST") ) {
    inList = ui.GetFilename("FROMLIST");
  }

  bool noIgnore          = ui.GetBoolean("NOIGNORE");
  bool noHeld            = ui.GetBoolean("NOHELD");
  bool noSingleMeasure   = ui.GetBoolean("NOSINGLEMEASURES");
  bool noMeasureless     = ui.GetBoolean("NOMEASURELESS");
  bool noTolerancePoints = ui.GetBoolean("TOLERANCE");
  bool reference         = ui.GetBoolean("REFERENCE");
  bool ground            = ui.GetBoolean("GROUND");
  bool cubePoints        = ui.WasEntered("CUBEPOINTS");
  bool cubeMeasures      = ui.GetBoolean("CUBEMEASURES");
  bool pointsEntered     = ui.WasEntered("POINTLIST");
  bool latLon            = ui.GetBoolean("LATLON");

  if( !(noIgnore || noHeld || noSingleMeasure || noMeasureless || noTolerancePoints ||
        reference || ground || cubePoints || pointsEntered || latLon) ) {
    std::string msg = "At least one filter must be selected [";
    msg += "NOIGNORE,NOHELD,NOSINGLEMEASURE,TOLERANCE,REFERENCE,GROUND,CUBEPOINTS,";
    msg += "CUBEMEASURES,POINTLIST,LATLON]";
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
  progress.SetMaximumSteps(outNet.Size());
  progress.CheckStatus();

  // Set up verctor records of how points/measures are removed
  PvlKeyword ignoredPoints( "IgnoredPoints" );
  PvlKeyword ignoredMeasures( "IgnoredMeasures" );
  PvlKeyword heldPoints( "HeldPoints" );
  PvlKeyword singleMeasurePoints( "SingleMeasurePoints" );
  PvlKeyword measurelessPoints( "MeasurelessPoints" );
  PvlKeyword tolerancePoints( "TolerancePoints" );
  PvlKeyword nonReferenceMeasures( "NonReferenseMeasures" );
  PvlKeyword nonGroundPoints( "NonGroundPoints" );
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

  double tolerance = 0.0;
  if( noTolerancePoints ) {
    tolerance = ui.GetDouble("PIXELTOLERANCE");
  }

  // Set up extracted network values
  if( ui.WasEntered("NETWORKID") )
    outNet.SetNetworkId( ui.GetString("NETWORKID") );

  outNet.SetUserName( Isis::Application::UserName() );
  outNet.SetDescription( ui.GetString("DESCRIPTION") );


  for(int cp=outNet.Size()-1; cp >= 0; cp --) {
    progress.CheckStatus();

    // Do preliminary exclusion checks
    if( noIgnore && outNet[cp].Ignore() ) {
      ignoredPoints += outNet[cp].Id();
      outNet.Delete(cp);
      continue;
    }
    if( noHeld && outNet[cp].Held() ) {
      heldPoints += outNet[cp].Id();
      outNet.Delete(cp);
      continue;
    }
    if( ground && !(outNet[cp].Type() == ControlPoint::Ground) ) {
      nonGroundPoints += outNet[cp].Id();
      outNet.Delete(cp);
      continue;
    }

    if( noSingleMeasure ) {
      bool invalidPoint = false;
      invalidPoint |= noIgnore && (outNet[cp].NumValidMeasures() < 2);
      invalidPoint |= outNet[cp].Size() < 2 && (outNet[cp].Type() != ControlPoint::Ground);

      if( invalidPoint ) {
        singleMeasurePoints += outNet[cp].Id();
        outNet.Delete(cp);
        continue;
      }
    }

    // Change the current point into a new point by manipulation of its control measures
    ControlPoint & newPoint = outNet[cp];

    for( int cm = newPoint.Size()-1; cm >= 0; cm --) {
      if(noIgnore && newPoint[cm].Ignore()) {
        ignoredMeasures += "(" + newPoint.Id() + "," + newPoint[cm].CubeSerialNumber() + ")";
        newPoint.Delete( cm );
      }
      else if( reference && !newPoint[cm].IsReference() ) {
        nonReferenceMeasures += "(" + newPoint.Id() + "," + newPoint[cm].CubeSerialNumber() + ")";
        newPoint.Delete( cm );
      }
      else if( cubeMeasures ) {
        bool hasSerialNumber = false;

        for( unsigned int sn = 0; sn < serialNumbers.size() && !hasSerialNumber; sn ++) {
          if(serialNumbers[sn] == newPoint[cm].CubeSerialNumber()) hasSerialNumber = true;
        }

        if( !hasSerialNumber ) { 
          nonCubeMeasures += "(" + newPoint.Id() + "," + newPoint[cm].CubeSerialNumber() + ")";
          newPoint.Delete( cm );
        }
      }
    }

    // Check for line/sample errors above provided tolerance
    if( noTolerancePoints ) {
      bool hasLowTolerance = true;

      for( int cm = 0; cm < newPoint.Size() && hasLowTolerance; cm ++ ) {
        if( newPoint[cm].SampleError() >= tolerance ||
            newPoint[cm].LineError() >= tolerance ) {
          hasLowTolerance = false;
        }
      }

      if( hasLowTolerance ) {
        tolerancePoints += newPoint.Id();
        outNet.Delete(cp);
        continue;
      }
    }

    // Do not add outPoint if it has too few measures
    if( noSingleMeasure ) {
      bool invalidPoint = false;
      invalidPoint |= noIgnore && (newPoint.NumValidMeasures() < 2);
      invalidPoint |= newPoint.Size() < 2 && newPoint.Type() != ControlPoint::Ground;

      if( invalidPoint ) {
        singleMeasurePoints += outNet[cp].Id();
        outNet.Delete(cp);
        continue;
      }
    }

    // Do not add outPoint if it does not have a cube in CUBEPOINTS as asked
    if( cubePoints && !cubeMeasures ) {
      bool hasSerialNumber = false;

      for( int cm = 0; cm < newPoint.Size() && !hasSerialNumber; cm ++) {
        for( unsigned int sn = 0; sn < serialNumbers.size() && !hasSerialNumber; sn ++) {
          if(serialNumbers[sn] == newPoint[cm].CubeSerialNumber()) hasSerialNumber = true;
        }
      }

      if( !hasSerialNumber ) {
        nonCubePoints += newPoint.Id();
        outNet.Delete(cp);
        continue;
      }
    }

    if( noMeasureless && newPoint.Size() == 0 ) {
      noMeasurePoints += newPoint.Id();
      outNet.Delete(cp);
      continue;
    }
  } //! Finished with simple comparisons


  /**
   * Use another pass to check for Ids
   */
  if( pointsEntered ) {
    ExtractPointList( outNet, nonListedPoints );
  }


  /** 
   *  Use another pass on outNet, because this is by far the most time consuming
   *  process, and time could be saved by using the reduced size of outNet
   */
  if( latLon ) {
    ExtractLatLonRange( outNet, nonLatLonPoints, cannotGenerateLatLonPoints, sn2filename );
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
    summary.AddKeyword( PvlKeyword( "SingleMeasurePoints", singleMeasurePoints.Size() ) ); 
    results.AddKeyword( singleMeasurePoints );
  }
  if( noMeasureless ) {
    summary.AddKeyword( PvlKeyword( "MeasurelessPoints", measurelessPoints.Size() ) ); 
    results.AddKeyword( measurelessPoints );
  }
  if( noTolerancePoints ) {
    summary.AddKeyword( PvlKeyword( "TolerancePoints", tolerancePoints.Size() ) ); 
    results.AddKeyword( tolerancePoints );
  }
  if( reference ) {
    summary.AddKeyword( PvlKeyword( "NonReferenceMeasures", nonReferenceMeasures.Size() ) ); 
    results.AddKeyword( nonReferenceMeasures );
  }
  if( ground ) {
    summary.AddKeyword( PvlKeyword( "NonGroundPoints", nonGroundPoints.Size() ) ); 
    results.AddKeyword( nonGroundPoints );
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
 * Removes control points not listed in POINTLIST
 * 
 * @param outNet The output control net being removed from
 * @param nonListedPoints The keyword recording all of the control points 
 *                        removed due to not being listed
 */
void ExtractPointList( ControlNet & outNet, PvlKeyword & nonListedPoints ) {
  UserInterface &ui = Application::GetUserInterface();

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
 * Removes control points not in the lat/lon range provided in the unput 
 * parameters. 
 * 
 * @param outNet The output control net being removed from
 * @param noLanLonPoint The keyword recording all of the control points removed
 *                      due to the provided lat/lon range
 * @param noLanLonPoint The keyword recording all of the control points removed
 *                      due to the inability to calculate the lat/lon for that
 *                      point
 */
void ExtractLatLonRange( ControlNet & outNet, PvlKeyword & nonLatLonPoints,
                         PvlKeyword & cannotGenerateLatLonPoints,  map<iString,iString> sn2filename ) {
  if( outNet.Size() == 0 ) { return; }

  UserInterface &ui = Application::GetUserInterface();

  // Get the lat/lon and fix the range for the internal 0/360
  double minlat = ui.GetDouble("MINLAT");
  double maxlat = ui.GetDouble("MAXLAT");
  double minlon = ui.GetDouble("MINLON");
  if( minlon < 0.0 ) { minlon += 360; }
  double maxlon = ui.GetDouble("MAXLON");
  if( maxlon < 0.0 ) { minlon += 360; }

  bool useNetwork = ui.GetBoolean("USENETWORK");

  Progress progress;
  progress.SetText("Calculating lat/lon");
  progress.SetMaximumSteps(outNet.Size());
  progress.CheckStatus();

  CubeManager manager;
  manager.SetNumOpenCubes( 50 ); //Should keep memory usage to around 1GB

  for( int cp = outNet.Size()-1; cp >= 0; cp --) {
    progress.CheckStatus();

    // If the Contorl Network takes priority, use it
    double pointLat = outNet[cp].UniversalLatitude();
    double pointLon = outNet[cp].UniversalLongitude();
    bool useControlNet = useNetwork && pointLat > -1000 && pointLon > -1000;
    if( outNet[cp].Type() == Isis::ControlPoint::Ground || useControlNet ) {
      if( NotInLatLonRange( outNet[cp].UniversalLatitude(),
                            outNet[cp].UniversalLongitude(),
                            minlat, maxlat, minlon, maxlon ) ) {
        nonLatLonPoints += outNet[cp].Id();
        outNet.Delete( cp );
      }
    }

    /** 
     * If the lat/lon cannot be determined from the point, then we need to calculate
     * lat/lon on our own 
     */
    else if( ui.WasEntered("FROMLIST") ) {

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
    else {
      cannotGenerateLatLonPoints += outNet[cp].Id();
      outNet.Delete( cp );
    }

  }

  manager.CleanCubes();
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

  if( ui.WasEntered("TOLIST") ) {

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
}

