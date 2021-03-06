#include "ImagePolygon.h"
#include "PolygonTools.h"
#include "Preference.h"
#include "geos/geom/MultiPolygon.h"

using namespace std; 
using namespace Isis;


int main () {
  Isis::Preference::Preferences(true);

/**
 * @brief Test ImagePolygon object for accuracy and correct behavior.
 * 
 * @author 2005-11-22 Tracie Sucharski
 * 
 * @history 2007-01-19  Tracie Sucharski, Removed ToGround method (for now)
 *          because of round off problems going back and forth between
 *          lat/lon,line/samp.
 * @history 2007-01-31  Tracie Sucharski,  Added WKT method to return polygon
 *                           in string as WKT.
 * @history 2007-11-09  Tracie Sucharski,  Remove WKT method, geos now has
 *                            a method to return a WKT string.
 * @history 2007-11-20  Tracie Sucharski,  Added test for sub-polys 
*/   

  //   simple MOC image
  string inFile = "/usgs/cpkgs/isis3/data/mgs/testData/ab102401.cub";
  //string inFile = "/work1/tsucharski/poly/I17621017RDR_lev2.cub";
  //  same MOC image, but sinusoidal projection
  //string inFile = "/farm/prog1/tsucharski/isis3/ab102401.lev2.cub";

  //  same MOC image, but sinusoidal projection , doctored left edge
  //string inFile = "/farm/prog1/tsucharski/isis3/ab102401.lev2.leftTrim.cub";

  //   MOC north pole image
  //string inFile = "/work1/tsucharski/isis3/poly/e0202226.lev1.cub";

  //  MOC image with 0/360 boundary
  //  orkin
  //string inFile = "/farm/prog1/tsucharski/isis3/cubes/m0101631.lev1.cub";
  // blackflag
  //string inFile = "/work1/tsucharski/isis3/poly/m0101631.lev1.cub";
  
  // galileo ssi image
  //string inFile = "/farm/prog1/tsucharski/isis3/6700r.cub";


  // Open the cube
  Cube cube;
  Cube cube1;
  cube.Open(inFile,"r");

  ImagePolygon poly;
  try {
    poly.Create(cube);
  }
  catch (iException &e) {
    std::string msg = "Cannot create polygon for [" + cube.Filename() + "]";
    throw iException::Message(iException::Programmer,msg,_FILEINFO_);
  }


  //  write poly as WKT
  std::cout<< poly.Polys()->toString()<<std::endl;

  //  Test sub-poly option
  try {
    poly.Create(cube,12,1,384,640,385);
  } 
  catch (iException &e) {
    std::string msg = "Cannot create sub-polygon for [" + cube.Filename() + "]";
    throw iException::Message(iException::Programmer,msg,_FILEINFO_);
  }
  //  write poly as WKT
  std::cout<< poly.Polys()->toString()<<std::endl;


  //  Test lower quality option
  try {
    poly.Create(cube,10,12,1,384,640,385);
  } 
  catch (iException &e) {
    std::string msg = "Cannot create lower quality polygon for [" + cube.Filename() + "]";
    throw iException::Message(iException::Programmer,msg,_FILEINFO_);
  }
  //  write poly as WKT
  std::cout<< poly.Polys()->toString()<<std::endl;



  cube.Close();
}

