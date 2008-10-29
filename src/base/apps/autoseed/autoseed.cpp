#include "Isis.h"

//#include <time.h>
#include <map>

#include "Process.h"
#include "Pvl.h"
#include "SerialNumberList.h"
#include "FindImageOverlaps.h"
#include "ImageOverlap.h"
#include "ControlNet.h"
#include "UniversalGroundMap.h"
#include "PolygonSeederFactory.h"
#include "PolygonSeeder.h"
#include "PolygonTools.h"
#include "ID.h"
#include "iException.h"

#include "GridPolygonSeeder.h"
#include "geos/util/GEOSException.h"

using namespace std;
using namespace Isis;

void IsisMain() {

  try {

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
//    PolygonSeeder *seeder = new GridPolygonSeeder(seedDef);
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

    // Create a progress object for feedback to the user
    Progress progress;
    progress.SetText("Finding Overlaps");
    progress.SetMaximumSteps(1);
    progress.CheckStatus();

    // Find all the overlaps between the images in the FROMLIST
    // The overlap polygon coordinates are in Lon/Lat order
    FindImageOverlaps overlaps(serialNumbers);
    progress.CheckStatus();


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

    progress.SetText("Seeding Points");
    progress.SetMaximumSteps(overlaps.Size());
    progress.CheckStatus();

    // Process each overlap area
    //   Seed measurments into it
    //   Store the measurments in the control network

    for (int ov=0; ov<overlaps.Size(); ++ov) {
      if (overlaps[ov]->Size() == 1) {
        stats_noOverlap++;
        progress.CheckStatus();
        continue;
      }

      // Seed this overlap with points
      const geos::geom::MultiPolygon *mp = overlaps[ov]->Polygon();
      std::vector<geos::geom::Point*> seed;

      try {
        seed = seeder->Seed(mp, proj);
      }
      catch (geos::util::GEOSException *exc) {
//        for (int i=0; overlaps[ov]->Size(); ++i) {
//          cout << "Problem SN: " << (*overlaps[ov])[i] << endl;
//        }
        // cout <<"GEOS Exception: " << exc->what() << endl;
        cout << "Error #2" << endl;
        delete exc;
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
      }

      progress.CheckStatus();
    }

    // All done with the UGMs so delete them
    for (unsigned int sn=0; sn<gMaps.size(); ++sn) {
      UniversalGroundMap *gmap = gMaps[serialNumbers.SerialNumber(sn)];
      delete gmap;
    }
    gMaps.clear();

    // Write the control network out
    cnet.Write(ui.GetFilename("TO"));

  }
// REFORMAT THESE ERRORS INTO ISIS TYPES AND RETHROW
  catch (Isis::iException &e) {
    cout << "ERROR " << e.what() << endl;
    e.Report();
  }
  catch (geos::util::GEOSException *exc) {
    //cout <<"GEOS Exception: " << exc->what() << endl;
    cout << "Error #1" << endl;
    delete exc;
  }
  catch (std::exception const &se) {
    cout << "std::exception " << se.what() << endl;
  }
  catch (...) {
    cout << " Other error" << endl;
  }
}

