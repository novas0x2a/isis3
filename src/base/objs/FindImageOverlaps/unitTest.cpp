#include <iostream>
#include <exception>

#include "iException.h"
#include "SerialNumberList.h"
#include "PolygonTools.h"
#include "FindImageOverlaps.h"
#include "Preference.h"
#include "geos/util/GEOSException.h"
#include "geos/geom/Polygon.h"
#include "geos/geom/LinearRing.h"
#include "geos/geom/CoordinateArraySequence.h"

using namespace std;

int main () {
  Isis::Preference::Preferences(true);
  void PrintImageOverlap (const Isis::ImageOverlap *poi);

  // Create 6 multi polygons
  //     01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
  // 10        +--------------------------+
  //           |              B           |
  // 09  +-----|--------+              +--|--------+
  //     | A   | a      |              |c |   C    |
  // 08  |     |     +--|--------------|--|-----+  |
  //     |     |     |g |     d        |h |     |  |
  // 07  |     +-----|--|--------------|--+  e  |  |
  //     |           |  |              |        |  |
  // 06  |           |b |    D       +-|-----+  |  | 
  //     |           |  |            | |  f  |  |  |
  // 05  |           |  |            | +-----|--|--+
  //     |           |  |            |   E   |  |  
  // 04  +-----------|--+            +-------+  |
  //                 |                          |
  // 03              |                          |
  //                 |                          |
  // 02  +--------+  +--------------------------+
  //     |   F    |
  // 01  +--------+
  //
  //  Name   Comes From
  //  A      A-a-b               <---First start
  //  B      B-a-c-d
  //  C      C-c-e-f
  //  D      D-b-d-e
  //  E      E-f
  //  F      F                   <---First stop
  //  a      AnB-g               <---Second start
  //  b      AnD-g
  //  c      BnC-h
  //  d      BnD-h
  //  e      CnD
  //  f      CnE
  //         DnE                * Equivalent to E so throw it away
  //  g      anb                <---Third start
  //  h      cnd                <-- Second stop
  //
  //-----------------------------------------------------------------

  cout << "Test 1" << endl;

  // Fill a vector of MultiPolygon* and serial numbers
  vector<geos::geom::MultiPolygon*> boundaries;
  vector<string> sns;

  // Reusable variables
  geos::geom::CoordinateSequence *pts;
  vector<geos::geom::Geometry*> polys;

  // Create the A polygon
  pts = new geos::geom::CoordinateArraySequence ();
  pts->add (geos::geom::Coordinate (1,9));
  pts->add (geos::geom::Coordinate (6,9));
  pts->add (geos::geom::Coordinate (6,4));
  pts->add (geos::geom::Coordinate (1,4));
  pts->add (geos::geom::Coordinate (1,9));

  polys.push_back (Isis::globalFactory.createPolygon (
                   Isis::globalFactory.createLinearRing(pts),NULL));

  boundaries.push_back(Isis::globalFactory.createMultiPolygon (polys));

  for (unsigned int i=0; i<polys.size(); ++i) delete polys[i];
  polys.clear();
  sns.push_back("A");

  // Create the B polygon
  pts = new geos::geom::DefaultCoordinateSequence ();
  pts->add (geos::geom::Coordinate (3,10));
  pts->add (geos::geom::Coordinate (12,10));
  pts->add (geos::geom::Coordinate (12,7));
  pts->add (geos::geom::Coordinate (3,7));
  pts->add (geos::geom::Coordinate (3,10));

  polys.push_back (Isis::globalFactory.createPolygon (
                    Isis::globalFactory.createLinearRing (pts),NULL));
  boundaries.push_back(Isis::globalFactory.createMultiPolygon (polys));

  for (unsigned int i=0; i<polys.size(); ++i) delete polys[i];
  polys.clear();
  sns.push_back("B");

  // Create the C polygon
  pts = new geos::geom::CoordinateArraySequence ();
  pts->add (geos::geom::Coordinate (11,5));
  pts->add (geos::geom::Coordinate (11,9));
  pts->add (geos::geom::Coordinate (15,9));
  pts->add (geos::geom::Coordinate (15,5));
  pts->add (geos::geom::Coordinate (11,5));

  polys.push_back (Isis::globalFactory.createPolygon (
                    Isis::globalFactory.createLinearRing (pts),NULL));
  boundaries.push_back(Isis::globalFactory.createMultiPolygon (polys));

  for (unsigned int i=0; i<polys.size(); ++i) delete polys[i];
  polys.clear();
  sns.push_back("C");

  // Create the D polygon
  pts = new geos::geom::CoordinateArraySequence ();
  pts->add (geos::geom::Coordinate (14,8));
  pts->add (geos::geom::Coordinate (14,2));
  pts->add (geos::geom::Coordinate (5,2));
  pts->add (geos::geom::Coordinate (5,8));
  pts->add (geos::geom::Coordinate (14,8));

  polys.push_back (Isis::globalFactory.createPolygon (
                    Isis::globalFactory.createLinearRing (pts),NULL));
  boundaries.push_back(Isis::globalFactory.createMultiPolygon (polys));

  for (unsigned int i=0; i<polys.size(); ++i) delete polys[i];
  polys.clear();
  sns.push_back("D");

  // Create the E polygon
  pts = new geos::geom::CoordinateArraySequence ();
  pts->add (geos::geom::Coordinate (10,6));
  pts->add (geos::geom::Coordinate (13,6));
  pts->add (geos::geom::Coordinate (13,4));
  pts->add (geos::geom::Coordinate (10,4));
  pts->add (geos::geom::Coordinate (10,6));

  polys.push_back (Isis::globalFactory.createPolygon (
                    Isis::globalFactory.createLinearRing (pts),NULL));
  boundaries.push_back(Isis::globalFactory.createMultiPolygon (polys));

  for (unsigned int i=0; i<polys.size(); ++i) delete polys[i];
  polys.clear();
  sns.push_back("E");

  // Create the F polygon
  pts = new geos::geom::CoordinateArraySequence ();
  pts->add (geos::geom::Coordinate (1,1));
  pts->add (geos::geom::Coordinate (1,2));
  pts->add (geos::geom::Coordinate (4,2));
  pts->add (geos::geom::Coordinate (4,1));
  pts->add (geos::geom::Coordinate (1,1));

  polys.push_back (Isis::globalFactory.createPolygon (
                    Isis::globalFactory.createLinearRing (pts),NULL));
  boundaries.push_back(Isis::globalFactory.createMultiPolygon (polys));

  for (unsigned int i=0; i<polys.size(); ++i) delete polys[i];
  polys.clear();
  sns.push_back("F");

  // Create a FindImageOverlaps object with the multipolys and sns from above
  Isis::FindImageOverlaps po(sns, boundaries);

  // Print each overlap area
  for (int i=0; i<po.Size(); i++) {
    PrintImageOverlap(po[i]);
  }

  // Test the Despike members

  // Spike is at coordinate 0 and is repeated in last coordinate
  cout << endl << "Test Despike(geos::LineString*)" << endl;
  try {
    pts = new geos::geom::CoordinateArraySequence ();
    pts->add (geos::geom::Coordinate (23.65635603023856,19.775890446582164));
    pts->add (geos::geom::Coordinate (23.61028465044545,20.17395284745577));
    pts->add (geos::geom::Coordinate (23.47383058370609,20.16532011634494));
    pts->add (geos::geom::Coordinate (23.474419802432088,20.161247812957285));
    pts->add (geos::geom::Coordinate (23.53120295719081,19.76362753213033));
    pts->add (geos::geom::Coordinate (23.01943117438372,19.73027518403705));
    pts->add (geos::geom::Coordinate (20.49535961958861,34.459983946486005));
    pts->add (geos::geom::Coordinate (20.49457592481588,34.46402366520344));
    pts->add (geos::geom::Coordinate (21.945734840009525,34.55586840742796));
    pts->add (geos::geom::Coordinate (23.65635603023856,19.775890446582164));
  
    geos::geom::LinearRing *lr = Isis::globalFactory.createLinearRing (pts);
    cout << "  Input  >>>> " << lr->toString() << endl;
  
    Isis::FindImageOverlaps fio2;
    geos::geom::LinearRing *lr2 = fio2.Despike(lr);
    cout << "  Output >>>> " << lr2->toString() << endl;
  }
  catch (std::exception const &se) {
    cout << "std::exception " << se.what() << endl;
  }


  // Spike is at coordinate 1
  cout << endl << "Test Despike(geos::LineString*)" << endl;
  try {
    pts = new geos::geom::CoordinateArraySequence ();
    pts->add (geos::geom::Coordinate (21.945734840009525,34.55586840742796));
    pts->add (geos::geom::Coordinate (23.65635603023856,19.775890446582164));
    pts->add (geos::geom::Coordinate (23.61028465044545,20.17395284745577));
    pts->add (geos::geom::Coordinate (23.47383058370609,20.16532011634494));
    pts->add (geos::geom::Coordinate (23.474419802432088,20.161247812957285));
    pts->add (geos::geom::Coordinate (23.53120295719081,19.76362753213033));
    pts->add (geos::geom::Coordinate (23.01943117438372,19.73027518403705));
    pts->add (geos::geom::Coordinate (20.49535961958861,34.459983946486005));
    pts->add (geos::geom::Coordinate (20.49457592481588,34.46402366520344));
    pts->add (geos::geom::Coordinate (21.945734840009525,34.55586840742796));

    geos::geom::LinearRing *lr = Isis::globalFactory.createLinearRing (pts);
    cout << "  Input  >>>> " << lr->toString() << endl;

    Isis::FindImageOverlaps fio2;
    geos::geom::LinearRing *lr2 = fio2.Despike(lr);
    cout << "  Output >>>> " << lr2->toString() << endl;
  }
  catch (std::exception const &se) {
    cout << "std::exception " << se.what() << endl;
  }

  // Spike is at coordinate 2
  cout << endl << "Test Despike(geos::LineString*)" << endl;
  try {
    pts = new geos::geom::CoordinateArraySequence ();
    pts->add (geos::geom::Coordinate (20.49457592481588,34.46402366520344));
    pts->add (geos::geom::Coordinate (21.945734840009525,34.55586840742796));
    pts->add (geos::geom::Coordinate (23.65635603023856,19.775890446582164));
    pts->add (geos::geom::Coordinate (23.61028465044545,20.17395284745577));
    pts->add (geos::geom::Coordinate (23.47383058370609,20.16532011634494));
    pts->add (geos::geom::Coordinate (23.474419802432088,20.161247812957285));
    pts->add (geos::geom::Coordinate (23.53120295719081,19.76362753213033));
    pts->add (geos::geom::Coordinate (23.01943117438372,19.73027518403705));
    pts->add (geos::geom::Coordinate (20.49535961958861,34.459983946486005));
    pts->add (geos::geom::Coordinate (20.49457592481588,34.46402366520344));

    geos::geom::LinearRing *lr = Isis::globalFactory.createLinearRing (pts);
    cout << "  Input  >>>> " << lr->toString() << endl;

    Isis::FindImageOverlaps fio2;
    geos::geom::LinearRing *lr2 = fio2.Despike(lr);
    cout << "  Output >>>> " << lr2->toString() << endl;
  }
  catch (std::exception const &se) {
    cout << "std::exception " << se.what() << endl;
  }


  // Spike is at coordinate last-1
  cout << endl << "Test Despike(geos::LineString*)" << endl;
  try {
    pts = new geos::geom::CoordinateArraySequence ();
    pts->add (geos::geom::Coordinate (23.61028465044545,20.17395284745577));
    pts->add (geos::geom::Coordinate (23.47383058370609,20.16532011634494));
    pts->add (geos::geom::Coordinate (23.474419802432088,20.161247812957285));
    pts->add (geos::geom::Coordinate (23.53120295719081,19.76362753213033));
    pts->add (geos::geom::Coordinate (23.01943117438372,19.73027518403705));
    pts->add (geos::geom::Coordinate (20.49535961958861,34.459983946486005));
    pts->add (geos::geom::Coordinate (20.49457592481588,34.46402366520344));
    pts->add (geos::geom::Coordinate (21.945734840009525,34.55586840742796));
    pts->add (geos::geom::Coordinate (23.65635603023856,19.775890446582164));
    pts->add (geos::geom::Coordinate (23.61028465044545,20.17395284745577));

    geos::geom::LinearRing *lr = Isis::globalFactory.createLinearRing (pts);
    cout << "  Input  >>>> " << lr->toString() << endl;

    Isis::FindImageOverlaps fio2;
    geos::geom::LinearRing *lr2 = fio2.Despike(lr);
    cout << "  Output >>>> " << lr2->toString() << endl;
  }
  catch (std::exception const &se) {
    cout << "std::exception " << se.what() << endl;
  }

  cout << endl;

  // Test despiking a multipoly
  cout << "Test despiking a multipoly" << endl;
  try {
    geos::geom::CoordinateSequence *polyPts, *holePts;
    polyPts = new geos::geom::CoordinateArraySequence ();
    polyPts->add (geos::geom::Coordinate(27.7950989560816204,16.3228992571194844));
    polyPts->add (geos::geom::Coordinate(25.0101496152692775,32.8722829418579536));
    polyPts->add (geos::geom::Coordinate(25.0093900378818716,32.8763261395400193));
    polyPts->add (geos::geom::Coordinate(30.5315280248255156,33.2275097622719997));
    polyPts->add (geos::geom::Coordinate(30.5375451430601927,33.2277523029661381));
    polyPts->add (geos::geom::Coordinate(32.4775075046510082,16.6365012909721024));
    polyPts->add (geos::geom::Coordinate(32.4779948144589596,16.6324220266198530));
    polyPts->add (geos::geom::Coordinate(27.7950989560816204,16.3228992571194844));

    holePts = new geos::geom::CoordinateArraySequence ();
    holePts->add (geos::geom::Coordinate(27.9224092129771897,28.0089127717497846));
    holePts->add (geos::geom::Coordinate(27.8895730772957329,28.2312500162354283));
    holePts->add (geos::geom::Coordinate(27.8895588231340668,28.2313465958941414));
    holePts->add (geos::geom::Coordinate(27.8272020915479530,28.2283175599580503));
    holePts->add (geos::geom::Coordinate(27.8270798315129291,28.2283115760908601));
    holePts->add (geos::geom::Coordinate(27.8600736860844300,28.0059792747464371));
    holePts->add (geos::geom::Coordinate(27.8600881608826967,28.0058826899724238));
    holePts->add (geos::geom::Coordinate(27.9224092129771897,28.0089127717497846));

    geos::geom::LinearRing *lr = Isis::globalFactory.createLinearRing(polyPts);

    vector<geos::geom::Geometry*> *holes = new vector<geos::geom::Geometry*>;
    holes->push_back (Isis::globalFactory.createLinearRing(holePts));

    vector<geos::geom::Geometry*> *polys = new vector<geos::geom::Geometry*>;
    polys->push_back (Isis::globalFactory.createPolygon (lr, holes));

    geos::geom::MultiPolygon *mp = Isis::globalFactory.createMultiPolygon (polys);
    cout << "  Input >>> " << mp->toString() << endl;

    Isis::FindImageOverlaps fio2;
    geos::geom::MultiPolygon *dsmp = fio2.Despike(mp);
    cout << "  Output >>> " << dsmp->toString() << endl;

    delete dsmp;
    delete mp;
  }
  catch (geos::util::GEOSException *ge) {
    cout <<"GEOS Exception: " <<  ge->what() << endl;
    delete ge;
  }
  catch (std::exception const &se) {
    cout << "std::exception " << se.what() << endl;
  }
  cout << endl;
}




// Print an ImageOverlap
void PrintImageOverlap (const Isis::ImageOverlap *poi) {

  // Write the wkt version of the multi polygon to the screen
  const geos::geom::MultiPolygon *mp = poi->Polygon();
  cout << "Well Known Text" << endl;
  cout << "  " << mp->toString() << endl;
  cout << "  Number of serial numbers: " << poi->Size() << endl;;
  cout << "  Serial numbers: " << endl;
  for (int i=0; i<poi->Size(); i++) {
    cout << "    " << (*poi)[i] << endl;
  }
  cout << endl;
  return;
}

