#include "Isis.h"

#include <map>
#include <sstream>

#include "geos/util/GEOSException.h"
#include "Process.h"
#include "Pvl.h"
#include "SerialNumberList.h"
#include "ImageOverlapSet.h"
#include "ImageOverlap.h"
#include "ControlNet.h"
#include "UniversalGroundMap.h"
#include "PolygonSeederFactory.h"
#include "PolygonSeeder.h"
#include "PolygonTools.h"
#include "ID.h"
#include "iException.h"
#include "CameraFactory.h"

using namespace std;
using namespace Isis;

void IsisMain() {

  UserInterface &ui = Application::GetUserInterface();
  SerialNumberList serialNumbers(ui.GetFilename("FROMLIST"));

  // Get the AutoSeed PVL internalized
  Pvl seedDef(ui.GetFilename("SEEDDEF"));

  // Grab the labels from the first filename in the SerialNumberList to get
  // some info
  Pvl cubeLab(serialNumbers.Filename(0));

  // Construct a Projection for converting between Lon/Lat and X/Y
  // This is used inside the seeding algorithms.
  // Note: Should this be an option to include this in the program?
  PvlGroup inst = cubeLab.FindGroup("Instrument", Pvl::Traverse);
  string target = inst["TargetName"];
  PvlGroup radii = Projection::TargetRadii(target);
  Isis::Pvl maplab;
  maplab.AddGroup(Isis::PvlGroup("Mapping"));
  Isis::PvlGroup &mapGroup = maplab.FindGroup("Mapping");
  mapGroup += Isis::PvlKeyword("EquatorialRadius",(string)radii["EquatorialRadius"]);
  mapGroup += Isis::PvlKeyword("PolarRadius",(string)radii["PolarRadius"]);
  mapGroup += Isis::PvlKeyword("LatitudeType","Planetocentric");
  mapGroup += Isis::PvlKeyword("LongitudeDirection","PositiveEast");
  mapGroup += Isis::PvlKeyword("LongitudeDomain",360);
  mapGroup += Isis::PvlKeyword("CenterLatitude",0);
  mapGroup += Isis::PvlKeyword("CenterLongitude",0);
  mapGroup += Isis::PvlKeyword("ProjectionName","Sinusoidal");
  PolygonSeeder *seeder = PolygonSeederFactory::Create(seedDef);
  Projection *proj = Isis::ProjectionFactory::Create(maplab);

  // Create the control net to store the points in.
  ControlNet cnet;
  cnet.SetType (ControlNet::ImageToGround);
  cnet.SetTarget (target);
  string networkId = ui.GetString("NETWORKID");
  cnet.SetNetworkId(networkId);
  cnet.SetUserName(Isis::Application::UserName());
  cnet.SetDescription(ui.GetString("DESCRIPTION"));

  // Set up an automatic id generator for the point ids
  ID pointId = ID(ui.GetString("POINTID"));

  // Find all the overlaps between the images in the FROMLIST
  // The overlap polygon coordinates are in Lon/Lat order
  ImageOverlapSet overlaps;
  overlaps.ReadImageOverlaps(ui.GetFilename("OVERLAPLIST"));

  // Create a Universal Ground Map (UGM) for each image in the list
  int stats_noOverlap = 0;
  int stats_tolerance = 0;

  map<std::string, UniversalGroundMap*> gMaps;
  for (int sn=0; sn<serialNumbers.Size(); ++sn) {
    // Create the UGM for the cube associated with this SN
    Pvl lab = Pvl(serialNumbers.Filename(sn));
    gMaps.insert(std::pair<std::string, UniversalGroundMap*>
                 (serialNumbers.SerialNumber(sn), new UniversalGroundMap(lab)));
  }

  stringstream errors (stringstream::in | stringstream::out);
  int errorNum = 0;

  // Process each overlap area
  //   Seed measurments into it
  //   Store the measurments in the control network

  bool previousControlNet = ui.WasEntered("CNET");

  vector< geos::geom::Point *> points;
  if ( previousControlNet ) {

    ControlNet precnet( ui.GetFilename("CNET") );

    Progress progress;
    progress.SetText("Calculating Provided Control Net");
    progress.SetMaximumSteps(precnet.Size());
    progress.CheckStatus();
    
    for ( int i = 0 ; i < precnet.Size(); i ++ ) {
      ControlPoint cp = precnet[i];
      ControlMeasure cm = cp[0];
      if( cp.HasReference() ) {
        cm = cp[cp.ReferenceIndex()];
      }

      string c = serialNumbers.Filename( cm.CubeSerialNumber() );
      Pvl cubepvl( c );
      Camera *cam = CameraFactory::Create( cubepvl );
      cam->SetImage( cm.Sample(), cm.Line() );

      points.push_back( Isis::globalFactory.createPoint(
        geos::geom::Coordinate(cam->UniversalLongitude(),cam->UniversalLatitude()) ) );

      delete cam;
      cam = NULL;

      progress.CheckStatus();
    }

  }

  Progress progress;
  progress.SetText("Seeding Points");
  progress.SetMaximumSteps(overlaps.Size());
  progress.CheckStatus();

  for (int ov=0; ov<overlaps.Size(); ++ov) {
    if (overlaps[ov]->Size() == 1) {
      stats_noOverlap++;
      progress.CheckStatus();
      continue;
    }

    // Checks if this overlap was already seeded
    if ( previousControlNet ) {

      // Grabs the Multipolygon's Envelope for Lat/Lon comparison
      const geos::geom::MultiPolygon *lonLatPoly = overlaps[ov]->Polygon();

      bool overlapSeeded = false;
      for( unsigned int j = 0; j < lonLatPoly->getNumGeometries()  &&  !overlapSeeded; j ++ ) {
        const geos::geom::Geometry *lonLatGeom = lonLatPoly->getGeometryN( j );

        // Checks if Control Point is in the MultiPolygon using Lon/Lat
        for ( unsigned int i = 0 ; i < points.size()  &&  !overlapSeeded; i ++ ) {
          if ( lonLatGeom->contains(points[i]) ) overlapSeeded = true;
        }
      }

      if( overlapSeeded ) continue;
    }

    // Seed this overlap with points
    const geos::geom::MultiPolygon *mp = overlaps[ov]->Polygon();
    std::vector<geos::geom::Point*> seed;

    try {
      seed = seeder->Seed(mp, proj);
    }
    catch (iException &e) {

      if ( ui.WasEntered("ERRORS") ) {

        if ( errorNum > 0 ) {
          errors << endl;
        }
        errorNum ++;

        errors << e.PvlErrors().Group(0).FindKeyword("Message")[0];
        for (int serNum = 0; serNum < overlaps[ov]->Size(); serNum++) {
          if ( serNum == 0 ) {
            errors << ": ";
          }
          else {
            errors << ", ";
          }
          errors << (*overlaps[ov])[serNum];
        }
      }

      e.Clear();
      continue;
    }

    // No points were seeded in this polygon, so collect some stats and move on
    if (seed.size() == 0) {
      stats_tolerance++;
      progress.CheckStatus();
      continue;
    }

    //   Create a control point for each seeded point in this overlap
    for (unsigned int point=0; point<seed.size(); ++point) {

      ControlPoint control;
      control.SetId (pointId.Next());
      control.SetType (ControlPoint::Tie);

      // Create a measurment at this point for each image in the overlap area
      for (int sn=0; sn<overlaps[ov]->Size(); ++sn) {

        // Get the line/sample of the lat/lon for this cube
        UniversalGroundMap *gmap = gMaps[(*overlaps[ov])[sn]];
        gmap->SetUniversalGround(seed[point]->getY(), seed[point]->getX());

        // Put the line/samp into a measurment
        ControlMeasure measurment;
        measurment.SetCoordinate (gmap->Sample(), gmap->Line(),
                                  ControlMeasure::Estimated);
        measurment.SetType(ControlMeasure::Estimated);
        measurment.SetCubeSerialNumber((*(overlaps[ov]))[sn]);
        measurment.SetDateTime();
        measurment.SetChooserName("Application autoseed");
        if (sn == 0) measurment.SetReference(true);
        control.Add(measurment);
      }

      cnet.Add(control);
      delete seed[point];

    } // End of create control points loop

    progress.CheckStatus();
  } // End of seeding loop

  // All done with the UGMs so delete them
  for (unsigned int sn=0; sn<gMaps.size(); ++sn) {
    UniversalGroundMap *gmap = gMaps[serialNumbers.SerialNumber(sn)];
    delete gmap;
  }

  for ( unsigned int i = 0 ; i < points.size(); i ++ ) {
    delete points[i];
    points[i] = NULL;
  }

  gMaps.clear();

  // Write the control network out
  cnet.Write(ui.GetFilename("TO"));

  //Log the ERRORS file
  if( ui.WasEntered("ERRORS") ) {
    string errorname = ui.GetFilename("ERRORS");
    std::ofstream errorsfile;
    errorsfile.open( errorname.c_str() );
    errorsfile << errors.str();
    errorsfile.close();
  }

  // create SeedDef group and add to print.prt
  PvlGroup pluginInfo = seeder->PluginParameters("SeedDefinition");
  Application::Log(pluginInfo);

  delete proj;
  proj = NULL;

  delete seeder;
  seeder = NULL;

}

