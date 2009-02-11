#include "Isis.h"

#include <iostream>
#include <sstream>
#include <set>

#include "SerialNumberList.h"
#include "ImageOverlapSet.h"
#include "ImageOverlap.h"
#include "Statistics.h"
#include "PvlGroup.h"
#include "Pvl.h"
#include "Projection.h"
#include "ProjectionFactory.h"
#include "PolygonTools.h"
#include "Progress.h"

using namespace std;
using namespace Isis;

void IsisMain() {

  UserInterface &ui = Application::GetUserInterface();
  SerialNumberList serialNumbers(ui.GetFilename("FROMLIST"));

  // Find all the overlaps between the images in the FROMLIST
  // The overlap polygon coordinates are in Lon/Lat order
  ImageOverlapSet overlaps;
  overlaps.ReadImageOverlaps(ui.GetFilename("OVERLAPLIST"));

  // Progress
  Progress progress;
  progress.SetMaximumSteps( overlaps.Size() );
  progress.CheckStatus();

  // Sets up the no overlap list
  set<string> nooverlap;
  for ( int i = 0; i < serialNumbers.Size(); i ++ ) {
    nooverlap.insert( serialNumbers.SerialNumber(i) );
  }

  // Extracts the stats of each overlap and adds to the table
  Statistics thickness;
  Statistics area;
  Statistics sncount;
  int overlapnum = 0;//Makes sure there are overlaps
  for ( int index = 0; index < overlaps.Size(); index ++ ) {

    if ( overlaps[index]->Size() > 1 ) {
      overlapnum++;

      // Removes the overlapping Serial Numbers for the nooverlap set
      for ( int i = 0; i < overlaps[index]->Size(); i ++ ) {
        nooverlap.erase( (*overlaps[index])[i] );
      }

      // Sets up the serial number stats
      sncount.AddData( overlaps[index]->Size() );

      // Sets up the thickness stats by doing A over E
      const geos::geom::MultiPolygon *mpLatLon = overlaps[index]->Polygon();

      // Construct a Projection for converting between Lon/Lat and X/Y
      Pvl cubeLab(serialNumbers.Filename(0));
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
      Projection *proj = Isis::ProjectionFactory::Create(maplab);

      // Sets up the thickness stats
      geos::geom::MultiPolygon *mpXY = PolygonTools::LatLonToXY( *mpLatLon, proj );
      thickness.AddData( PolygonTools::Thickness( mpXY ) );

      // Sets up the area stats
      area.AddData( mpXY->getArea() );

      delete mpXY;
    }

    progress.CheckStatus();

  }

  // Checks if there were overlaps to output results from
  if ( overlapnum == 0 ) {
    std::string msg = "The overlap file [";
    msg += Filename(ui.GetFilename("OVERLAPLIST")).Name();
    msg += "] does not contain any overlaps across the provided cubes [";
    msg += Filename(ui.GetFilename("FROMLIST")).Name() + "]";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  // Create the output

  stringstream output (stringstream::in | stringstream::out);
  output.precision(16);

  string delim = "";
  string pretty = ""; // Makes tab tables look pretty, ignored in CSV
  if ( ui.GetString("TABLETYPE") == "CSV" ) {
    delim = ",";
    output.setf(ios::scientific,ios::floatfield);
  }
  else {
    delim = "\t";
    pretty = "\t";
    output.setf(ios::showpoint);
  }

  // Add titles
  output << delim << pretty << pretty << "Thickness";
  output << delim << pretty << "Area";
  output << delim << pretty << pretty << "SN Count";

  // Add Minimum
  output << endl << "Minimum";
  output << delim << pretty << pretty << thickness.Minimum();
  output << delim << area.Minimum();
  output << delim << sncount.Minimum();

  // Add Maximum
  output << endl << "Maximum";
  output << delim << pretty << pretty << thickness.Maximum();
  output << delim << area.Maximum();
  output << delim << sncount.Maximum();

  // Add Average
  output << endl << "Average";
  output << delim << pretty << pretty << thickness.Average();
  output << delim << area.Average();
  output << delim << sncount.Average();

  // Add StandardDeviation
  output << endl << "StandardDeviation";
  output << delim << thickness.StandardDeviation();
  output << delim << area.StandardDeviation();
  output << delim << sncount.StandardDeviation();

  // Add Variance
  output << endl << "Variance";
  output << delim << pretty << thickness.Variance();
  output << delim << area.Variance();
  output << delim << sncount.Variance();

  // Add Sum
  output << endl << "Sum";
  output << delim << pretty << pretty << thickness.Sum();
  output << delim << area.Sum();
  output << delim << sncount.Sum();

  // Add non-overlapping cubes to the output
  if ( !nooverlap.empty() ) {
    output << endl << endl << "No Overlaps" << endl;
    for ( set<string>::iterator itt = nooverlap.begin(); itt != nooverlap.end(); itt ++ ) {
      output << serialNumbers.Filename(*itt) << delim;
      output << *itt << endl;
    }
  }

  // Display output
  string outname = ui.GetFilename("TO");
  std::ofstream outfile;
  outfile.open( outname.c_str() );
  outfile << output.str();
  outfile.close();

  if(outfile.fail()) {
    iString msg = "Unable to write the statistics to [" + ui.GetFilename("TO") + "]";
    throw iException::Message(iException::Io, msg, _FILEINFO_);
  }
}
