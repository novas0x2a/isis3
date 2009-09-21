#include "Isis.h"
#include "Pvl.h"
#include "SerialNumberList.h"
#include "ControlNet.h"
#include "ControlPoint.h"
#include "ControlMeasure.h"
#include "UniversalGroundMap.h"
#include "PolygonTools.h"
#include "Progress.h"
#include "InterestOperatorFactory.h"
#include "InterestOperator.h"
#include "ImageOverlapSet.h"
#include "iException.h"

#include "geos/geom/Point.h"
#include "geos/geom/Coordinate.h"
#include "geos/geom/MultiPolygon.h"
#include "geos/util/GEOSException.h"

using namespace std;
using namespace Isis;

void IsisMain() {

  try {
    UserInterface &ui = Application::GetUserInterface();
    SerialNumberList serialNumbers(ui.GetFilename("FROMLIST"));

    // Find all the overlaps between the images in the FROMLIST
    // The overlap polygon coordinates are in Lon/Lat order
    ImageOverlapSet overlaps;

    overlaps.ReadImageOverlaps(Filename(ui.GetFilename("OVERLAPLIST")).Expanded());
    
    // Get the InterestOperator set up
    Pvl interestDef(ui.GetFilename("INTEREST"));
    InterestOperator *interest = InterestOperatorFactory::Create(interestDef);
  
    // Get the original control net internalized
    ControlNet origNet(ui.GetFilename("NETWORK"));
  
    // Set up progress object to report processing
    Progress status;
    status.SetMaximumSteps(origNet.Size());
    status.CheckStatus();

    // Create the new control net to store the points in.
    ControlNet newNet;
    newNet.SetType (origNet.Type());
    newNet.SetTarget (origNet.Target());
    string networkId = ui.GetString("NETWORKID");
    newNet.SetNetworkId(networkId);
    newNet.SetUserName(Isis::Application::UserName());
    newNet.SetDescription(ui.GetString("DESCRIPTION"));
  
    // Open each cube and create a Universal Ground Map (UGM) for each cube in the list
    map<std::string, UniversalGroundMap*> gMaps;
    map<std::string, Cube*> cubes;
    for (int sn=0; sn<serialNumbers.Size(); ++sn) {
      Cube *c = new Cube;
      c->Open(serialNumbers.Filename(sn));
      // SerialNumberList doesnt allow duplicate sn
      cubes.insert(std::pair<std::string, Cube*>
                   (serialNumbers.SerialNumber(sn), c));
      gMaps.insert(std::pair<std::string, UniversalGroundMap*>
                   (serialNumbers.SerialNumber(sn), new UniversalGroundMap(*c)));
    }
  
    // Process each existing control point in the network
    for (int point=0; point<origNet.Size(); ++point) {
      ControlPoint &origPnt = origNet[point];
  
      // Only perform the interest operation on points of type "Tie"
      if (origPnt.Type() == ControlPoint::Tie) {
  
        // Create a new control point 
        ControlPoint newPnt;
        newPnt.SetId (origPnt.Id());
        newPnt.SetType (origPnt.Type());
    
        // Find the overlap this point is inside of
        gMaps[origPnt[0].CubeSerialNumber()]->SetImage(origPnt[0].Sample(),
                                                       origPnt[0].Line());
        double lon = gMaps[origPnt[0].CubeSerialNumber()]->UniversalLongitude();
        double lat = gMaps[origPnt[0].CubeSerialNumber()]->UniversalLatitude();
        geos::geom::Point *pnt = globalFactory.createPoint(geos::geom::Coordinate(lon, lat));
  
        const geos::geom::MultiPolygon *overlapPoly;
        overlapPoly = NULL;
        for (int ov=0; ov<overlaps.Size(); ++ov) {
          const geos::geom::MultiPolygon *tmp = overlaps[ov]->Polygon();
          if (pnt->within(tmp)) overlapPoly = tmp;
        }
        delete pnt;
        if (overlapPoly == NULL) {
          string msg = "Unable to find overlap polygon for point [";
          msg += origPnt.Id() + "] in network [" + origNet.NetworkId();
          msg += "] from file [" + ui.GetString("NETWORKID") + "]";
          throw Isis::iException::Message(Isis::iException::Programmer, msg, _FILEINFO_); 
        }

        // Find the most interesting pixel for each measurment in this point
        vector<double> bestInterests;
        vector<double> bestSamples;
        vector<double> bestLines;
        for (int measure=0; measure<origPnt.Size(); ++measure) {
          ControlMeasure &origMsr = origPnt[measure];
          std::string sn = origMsr.CubeSerialNumber();
  
          // Set the clipping polygon for this point
          //   Convert the lon/lat overlap polygon to samp/line using the UGM for
          //   this image
          geos::geom::MultiPolygon *poly = PolygonTools::LatLonToSampleLine(*overlapPoly, gMaps[origPnt[measure].CubeSerialNumber()]);
          interest->SetClipPolygon(*poly);
          delete poly;

          // Run the interest operator on this measurment
          if (interest->Operate(*(cubes[sn]), (int)(origMsr.Sample()+0.5),
                                (int)(origMsr.Line()+0.5))) {
            bestInterests.push_back(interest->InterestAmount());
            bestSamples.push_back(interest->CubeSample());
            bestLines.push_back(interest->CubeLine());
          }
          // If it failes to meet specs set the measurment to the worst case
          else {
            bestInterests.push_back(interest->WorstInterest());
            bestSamples.push_back(origMsr.Sample());
            bestLines.push_back(origMsr.Line());
          }
        }
  
        // Decide on a reference image and get the lat/lon from that measurment
        double referenceLat;
        double referenceLon;
        std::string referenceSn;
        int best = 0;

        // Use the image with the most interesting point as the reference image
        if (bestInterests.size() > 0) {
          for (int measure=1; measure<origPnt.Size(); ++measure) {
            if (interest->CompareInterests(bestInterests[measure], bestInterests[best])) {
              best = measure;
            }
          }
          referenceSn = origPnt[best].CubeSerialNumber();
          gMaps[referenceSn]->SetImage(bestSamples[best], bestLines[best]);
          referenceLon = gMaps[referenceSn]->UniversalLongitude();
          referenceLat = gMaps[referenceSn]->UniversalLatitude();
        }
        // Use the first image in this point as the reference and get its lat/lon
        else {
          referenceSn = origPnt[0].CubeSerialNumber();
          gMaps[referenceSn]->SetImage(origPnt[0].Sample(), origPnt[0].Line());
          referenceLon = gMaps[referenceSn]->UniversalLongitude();        
          referenceLat = gMaps[referenceSn]->UniversalLatitude();
        }


    
        // Create a measurment for each image in this point using the reference
        // lat/lon.
        for (int measure=0; measure<origPnt.Size(); ++measure) {
          std::string sn = origPnt[measure].CubeSerialNumber();
          ControlMeasure newMsr;
          newMsr.SetCubeSerialNumber(sn);
          newMsr.SetDateTime();
          newMsr.SetChooserName("Application interest");
    
          // Initialize the UGM of this cube with the reference lat/lon
          if (gMaps[sn]->SetUniversalGround(referenceLat, referenceLon)) {
    
            // Put the corresponding line/samp into a newMsr
            newMsr.SetCoordinate (gMaps[sn]->Sample(), gMaps[sn]->Line(),
                                      ControlMeasure::Estimated);
            if (bestInterests[best] != interest->WorstInterest()) {
              newMsr.SetType(ControlMeasure::Estimated);
            }
            else {
              newMsr.SetType(ControlMeasure::Unmeasured);
              newPnt.SetIgnore(true);
            }
            if (sn == referenceSn) {
              newMsr.SetReference(true);
            }
            else {
              newMsr.SetReference(false);
            }
          }
          // The lat/lon could not be set in the UGM so output a bad line/samp
          else {
            newMsr.SetType(ControlMeasure::Unmeasured);
            newMsr.SetIgnore(true);
          }
          newPnt.Add(newMsr);
        }
    
        newNet.Add(newPnt);
  
      }
      // Process non Tie points
      else {
  
      } // End of if point is of type tie
      status.CheckStatus();
    } // Point loop
  
    // Write the control network out
    newNet.Write(ui.GetFilename("TO"));
  
    // Clean up
    map<std::string, UniversalGroundMap*>::iterator gmapIt = gMaps.begin();
    while(gmapIt != gMaps.end()) {
      delete gmapIt->second;
      gmapIt ++;
    }
    gMaps.clear();

    map<std::string, Cube*>::iterator cubeIt = cubes.begin();
    while(cubeIt != cubes.end()) {
      delete cubeIt->second;
      cubeIt ++;
    }
    cubes.clear();

    // add operator to print.prt
    PvlGroup opGroup = interest->Operator(); 
    Application::Log(opGroup); 

    delete interest;
  }
  // REFORMAT THESE ERRORS INTO ISIS TYPES AND RETHROW
  catch (Isis::iException &e) {
    throw;
  }
  catch (geos::util::GEOSException *exc) {
    string message = "GEOS Exception: " + (iString)exc->what();
    delete exc;
    throw Isis::iException::Message(Isis::iException::User,message, _FILEINFO_);
  }
  catch (std::exception const &se) {
    string message = "std::exception: " + (iString)se.what();
    throw Isis::iException::Message(Isis::iException::User,message, _FILEINFO_);
  }
  catch (...) {
    string message = "Other Error";
    throw Isis::iException::Message(Isis::iException::User,message, _FILEINFO_);
  }
}
