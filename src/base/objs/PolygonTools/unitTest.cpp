#include "UniversalGroundMap.h"
#include "ProjectionFactory.h"
#include "PolygonTools.h"
#include "Preference.h"
#include "geos/geom/CoordinateSequence.h"
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/LinearRing.h"
#include "geos/geom/Polygon.h"
#include "geos/util/GEOSException.h"

using namespace std; 
using namespace Isis;

int main () {
  Isis::Preference::Preferences(true);

  try {
    cout << "Unit test for PolygonTools" << endl << endl;

    // Create coordinate sequence for the first of two polygons
    geos::geom::CoordinateSequence *pts = new geos::geom::CoordinateArraySequence ();
    pts->add (geos::geom::Coordinate (0.0, 0.0));
    pts->add (geos::geom::Coordinate (0.0, 1.0));
    pts->add (geos::geom::Coordinate (1.0, 1.0));
    pts->add (geos::geom::Coordinate (1.0, 0.0));
    pts->add (geos::geom::Coordinate (0.0, 0.0));
    cout << "Coordinates of polygon 1:" << pts->toString() << endl << endl;
  
    // Create the first polygon
    vector<geos::geom::Geometry *> polys;
    polys.push_back (Isis::globalFactory.createPolygon (
                         Isis::globalFactory.createLinearRing (pts),NULL));
  
    // Create coordinate sequence for the second of two polygons
    geos::geom::CoordinateSequence *pts2 = new geos::geom::CoordinateArraySequence ();
    pts2->add (geos::geom::Coordinate (360.0, 1.0));
    pts2->add (geos::geom::Coordinate (359.0, 1.0));
    pts2->add (geos::geom::Coordinate (359.0, 0.0));
    pts2->add (geos::geom::Coordinate (360.0, 0.0));
    pts2->add (geos::geom::Coordinate (360.0, 1.0));
    cout << "Coordinates of polygon 2:" << pts2->toString() << endl << endl;

    // Create coordinate sequence for the hole in the second polygon
    geos::geom::CoordinateSequence *pts3 = new geos::geom::DefaultCoordinateSequence ();
    pts3->add (geos::geom::Coordinate (359.75, 0.75));
    pts3->add (geos::geom::Coordinate (359.25, 0.75));
    pts3->add (geos::geom::Coordinate (359.25, 0.25));
    pts3->add (geos::geom::Coordinate (359.75, 0.25));
    pts3->add (geos::geom::Coordinate (359.75, 0.75));
    cout << "Coordinates of hole for polygon 2:" << pts3->toString() << endl << endl;

    vector<geos::geom::Geometry *> *hole2 = new vector<geos::geom::Geometry *>;
    hole2->push_back(Isis::globalFactory.createLinearRing(pts3));
  
    // Create the second polygon
    polys.push_back (Isis::globalFactory.createPolygon (
                         Isis::globalFactory.createLinearRing (pts2), hole2));
  
    // Create a multipolygon from the two polygons
    geos::geom::MultiPolygon* mPolygon = Isis::globalFactory.createMultiPolygon (polys);

    // Create a copy of the multipolygon
    geos::geom::MultiPolygon* tmpMp = PolygonTools::CopyMultiPolygon(mPolygon);
    cout << "Copy of the multipolygon = " << tmpMp->toString() << endl << endl;

    Isis::Pvl lab;
    lab.AddGroup(Isis::PvlGroup("Mapping"));
    Isis::PvlGroup &mapGroup = lab.FindGroup("Mapping");
    mapGroup += Isis::PvlKeyword("EquatorialRadius", 1.0);
    mapGroup += Isis::PvlKeyword("PolarRadius", 1.0);
    mapGroup += Isis::PvlKeyword("LatitudeType", "Planetocentric");
    mapGroup += Isis::PvlKeyword("LongitudeDirection", "PositiveEast");
    mapGroup += Isis::PvlKeyword("LongitudeDomain", 360);
    mapGroup += Isis::PvlKeyword("CenterLatitude", 0);
    mapGroup += Isis::PvlKeyword("CenterLongitude", 0);
    mapGroup += Isis::PvlKeyword("ProjectionName", "Sinusoidal");

    Projection *proj = ProjectionFactory::Create(lab);

    cout << "Lon/Lat polygon = " << mPolygon->toString() << endl << endl;

    cout << "X/Y polygon radius (1) = " 
         << (PolygonTools::XYFromLonLat(*mPolygon, proj))->toString() << endl << endl;

    delete proj;

    // Change the radius and set a new projection
    // Then get the XYPolygon again
    mapGroup.DeleteKeyword("EquatorialRadius");
    mapGroup += Isis::PvlKeyword("EquatorialRadius", 10.0);
    mapGroup.DeleteKeyword("PolarRadius");
    mapGroup += Isis::PvlKeyword("PolarRadius", 10.0);

    proj = ProjectionFactory::Create(lab);
    cout << "X/Y polygon radius (10) = " 
         << (PolygonTools::XYFromLonLat(*mPolygon, proj))->toString() << endl << endl;

    // Convert a Lon/Lat poly to X/Y and then back to Lon/Lat
    cout << "Lat/Lon polygon from X/Y with radius (10) = "
         << PolygonTools::LonLatFromXY(*PolygonTools::XYFromLonLat(*mPolygon, proj), proj)->toString()
         << endl << endl;

    // Create a UniversalGroundMap so we can test the SampleLinePolygon stuff
    Pvl pvl("unitTest.lbl");
    UniversalGroundMap ugm = UniversalGroundMap(pvl);

    // Create coordinate sequence for the first of two polygons
    geos::geom::CoordinateSequence *llpts = new geos::geom::CoordinateArraySequence ();
    ugm.SetImage(1.0, 1.0);
    llpts->add (geos::geom::Coordinate (ugm.UniversalLongitude(), ugm.UniversalLatitude()));
    ugm.SetImage(1204.0, 1.0);
    llpts->add (geos::geom::Coordinate (ugm.UniversalLongitude(), ugm.UniversalLatitude()));
    ugm.SetImage(1204.0, 1056.0);
    llpts->add (geos::geom::Coordinate (ugm.UniversalLongitude(), ugm.UniversalLatitude()));
    ugm.SetImage(1.0, 1056.0);
    llpts->add (geos::geom::Coordinate (ugm.UniversalLongitude(), ugm.UniversalLatitude()));
    ugm.SetImage(1.0, 1.0);
    llpts->add (geos::geom::Coordinate (ugm.UniversalLongitude(), ugm.UniversalLatitude()));
    cout << "Coordinates of Lon/Lat polygon:" << llpts->toString() << endl << endl;

    // Create the L/L polygon
    vector<geos::geom::Geometry *> llpolys;
    llpolys.push_back (Isis::globalFactory.createPolygon (
                           Isis::globalFactory.createLinearRing (llpts),NULL));
    geos::geom::MultiPolygon* llmPolygon = Isis::globalFactory.createMultiPolygon (llpolys);

    geos::geom::MultiPolygon *slmPolygon = PolygonTools::SampleLineFromLonLat(*llmPolygon, &ugm);
    cout << "Coordinates of Sample/Line polygon:" << slmPolygon->toString() << endl;

    cout << endl;

    cout << "Well Knowen Text Polygon:" << endl;
    std::cout<<mPolygon->toString()<<std::endl;

    cout << endl;

    cout << "GML Ploygon:" << endl;
    std::string GMLpolygon = PolygonTools::ToGML(mPolygon, "test");
    cout << GMLpolygon << endl; 

  }
  catch (Isis::iException &e) {
    cout << "ERROR " << e.what() << endl;
    e.Report();
  }
  catch (geos::util::GEOSException *exc) {
    cout <<"GEOS Exception: " << exc->what() << endl;
    delete exc;
  }
  catch (std::exception const &se) {
    cout << "std::exception " << se.what() << endl;
  }
  catch (...) {
    cout << " Other error" << endl;
  }
}
