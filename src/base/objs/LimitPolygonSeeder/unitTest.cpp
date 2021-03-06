#include <iostream>
#include <cstdlib>
#include <cmath>
#include <iomanip>

#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/Geometry.h"
#include "geos/geom/Polygon.h"

#include "iException.h"
#include "PolygonTools.h"
#include "PolygonSeeder.h"
#include "PolygonSeederFactory.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "ProjectionFactory.h"
#include "GridPolygonSeeder.h"
#include "Preference.h"

using namespace std;
using namespace Isis;

int main () {
  Isis::Preference::Preferences(true);
  try {
    cout << "Test 1, create a seeder" << endl;

    PvlGroup alg("PolygonSeederAlgorithm");

    if(!alg.HasKeyword("Name")) {
      alg += PvlKeyword("Name","Limit");
      alg += PvlKeyword("MinimumThickness", 0.3);
      alg += PvlKeyword("MinimumArea", 5);
      alg += PvlKeyword("MajorAxisPoints", 2);
      alg += PvlKeyword("MinorAxisPoints", 2);
    }

    PvlObject o("AutoSeed");
    o.AddGroup(alg);

    Pvl pvl;
    pvl.AddObject(o);
    cout << pvl << endl << endl;

    PolygonSeeder *ps = PolygonSeederFactory::Create(pvl);

    std::cout << "Test to make sure Parse did it's job" << std::endl;
    std::cout << "MinimumThickness = " << ps->MinimumThickness() << std::endl;
    std::cout << "MinimumArea = " << ps->MinimumArea() << std::endl;


    cout << "Test 2, test a triangular polygon" << endl;
    try {
      // Call the seed member with a polygon
      geos::geom::CoordinateSequence *pts;
      vector<geos::geom::Geometry*> polys;
  
      // Create the A polygon
      pts = new geos::geom::CoordinateArraySequence ();
      pts->add (geos::geom::Coordinate (0, 0));
      pts->add (geos::geom::Coordinate (0, 1));
      pts->add (geos::geom::Coordinate (0.5, 0.5));
      pts->add (geos::geom::Coordinate (1, 0));
      pts->add (geos::geom::Coordinate (0, 0));
  
      polys.push_back (Isis::globalFactory.createPolygon (
                       Isis::globalFactory.createLinearRing(pts),NULL));
  
      geos::geom::MultiPolygon *mp = Isis::globalFactory.createMultiPolygon (polys);
  
      cout << "Lon/Lat polygon = " << mp->toString() << endl;
      // Create the projection necessary for seeding
      PvlGroup radii = Projection::TargetRadii("MARS");
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

      Projection *proj = Isis::ProjectionFactory::Create(maplab);

      vector<geos::geom::Point *> seedValues = ps->Seed(mp, proj);
      cout << setprecision(13);
      for (unsigned int i=0; i<seedValues.size(); i++) {
//        cout << "Point(" << i << ") = " << seedValues[i]->toString() << endl;
        cout << "  POINT (";
        cout << seedValues[i]->getX() << " " << seedValues[i]->getY() << ")" << endl;
      }
    }
    catch (iException &e) {
      e.Report();
    }

    cout << "Test 3, test for too thin" << endl;
    try {
      // Call the seed member with a polygon
      geos::geom::CoordinateSequence *pts;
      vector<geos::geom::Geometry*> polys;

      // Create the A polygon
      pts = new geos::geom::CoordinateArraySequence ();
      pts->add (geos::geom::Coordinate (0, 0));
      pts->add (geos::geom::Coordinate (0, 0.5));
      pts->add (geos::geom::Coordinate (0.0125, 0.5));
      pts->add (geos::geom::Coordinate (0.0125, 0));
      pts->add (geos::geom::Coordinate (0, 0));

      polys.push_back (Isis::globalFactory.createPolygon (
                       Isis::globalFactory.createLinearRing(pts),NULL));

      geos::geom::MultiPolygon *mp = Isis::globalFactory.createMultiPolygon (polys);

      cout << "Lon/Lat polygon = " << mp->toString() << endl;

      // Create the projection necessary for seeding
      PvlGroup radii = Projection::TargetRadii("MARS");
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
      Projection *proj = Isis::ProjectionFactory::Create(maplab);

      // NOTHING SHOULD BE PRINTED (the thickness test should not have been met)
      vector<geos::geom::Point *> seedValues = ps->Seed(mp, proj);
      for (unsigned int i=0; i<seedValues.size(); i++) {
        cout << "Point(" << i << ") = " << seedValues[i]->toString() << endl;
      }
    }
    catch (iException &e) {
      e.Report();
    }
  }
  catch (iException &e) {
    e.Report();
  }

  return 0;
}
