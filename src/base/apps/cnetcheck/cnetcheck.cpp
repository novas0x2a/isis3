#include "Isis.h"

#include <set>
#include <stack>
#include <sstream>

#include "UserInterface.h"
#include "ControlNet.h"
#include "Filename.h"
#include "FileList.h"
#include "SerialNumber.h"
#include "Cube.h"
#include "Constants.h"
#include "CameraFactory.h"
#include "ProjectionFactory.h"

using namespace std; 
using namespace Isis;

map< string, set<string> > constructPointSets(set<string> &index, ControlNet &innet);
vector< set<string> > findIslands(set<string> &index, map< string, set<string> > &adjCubes);

// Main program
void IsisMain() {
  Progress progress;
  UserInterface &ui = Application::GetUserInterface();
  ControlNet innet(ui.GetFilename("NETWORK"));
  iString prefix(ui.GetString("PREFIX"));

  //Sets up the list of serial numbers for #4
  FileList inlist(ui.GetFilename("FROMLIST"));
  vector<iString> innums;
  vector<iString> listednums;
  map< string, string > num2cube;

  if (inlist.size() > 0) {
    progress.SetText("Initializing");
    progress.SetMaximumSteps(inlist.size());
    progress.CheckStatus();
  }

  for(int i=0; i < (int)inlist.size(); i++) {
    iString st = SerialNumber::Compose(inlist[i]);
    innums.push_back(st);
    //Used with nonlistednums
    listednums.push_back(st);
    //Used as a SerialNumber to Cube index
    num2cube[st] = inlist[i];
    progress.CheckStatus();
  }

  vector<iString> nonlistednums;
  set<string> singleMeasureControlPoints;
  set<string> serialNumDupe;
  set<string> noLatLonCubes;
  map< string, set<string> > noLatLonCPoints;
  set<string> latlonchecked;

  if (innet.Size() > 0) {
    progress.SetText("Calculating");
    progress.SetMaximumSteps(innet.Size());
    progress.CheckStatus();
  }

  for(int i=0; i < innet.Size(); i++) {
    ControlPoint controlpt(innet[i]);

    //Checks for lat/Lon production
    if( ui.GetBoolean("NOLATLON") ) {
      for (int j=0; j < controlpt.Size(); j++) {
        if ( !num2cube[controlpt[j].CubeSerialNumber()].empty()  &&
             latlonchecked.find(controlpt[j].CubeSerialNumber()) == latlonchecked.end() ) {
          latlonchecked.insert( controlpt[j].CubeSerialNumber() );

          Pvl pvl( num2cube[controlpt[j].CubeSerialNumber()] );
          Camera *camera( CameraFactory::Create( pvl ) );
          if (camera == NULL) {
            try {
              // Checking if the Projection can be made
              delete ProjectionFactory::Create( pvl );
            } catch ( iException &e ) {
              noLatLonCubes.insert(controlpt[j].CubeSerialNumber());
              noLatLonCPoints[controlpt[j].CubeSerialNumber()].insert( controlpt.Id() );
              e.Clear();
            }
          }
          else if(!camera->SetImage(controlpt[j].Sample(), controlpt[j].Line())) {
            noLatLonCubes.insert(controlpt[j].CubeSerialNumber());
            noLatLonCPoints[controlpt[j].CubeSerialNumber()].insert( controlpt.Id() );
          }
          delete camera;
        }
      }
    }

    //Checks of the ControlPoint has only 1 Measure #2
    if(controlpt.Size() == 1) {
      singleMeasureControlPoints.insert(controlpt[0].CubeSerialNumber());
    }
    else {
      //Checks for duplicate Measures for the same SerialNumber #3
      vector<ControlMeasure> controlMeasures;
      for(int j=0; j < controlpt.Size(); j++) {
        iString currentsnum = controlpt[j].CubeSerialNumber();
        controlMeasures.push_back(controlpt[j]);
        //Compares previous ControlMeasure SerialNumbers with the current,
        //  while removing used cubes from inlist and innums
        for(int k=0; k < (int)controlMeasures.size(); k++) {
          iString checksnum = controlMeasures[k].CubeSerialNumber();
          if(j != k  &&  checksnum == currentsnum) {
            serialNumDupe.insert(currentsnum); //serial number duplication #3
          }
          else {
            //Removes from the serial number list, cubes that are included in the cnet
            vector<iString>::iterator measureIt = innums.begin();
            while(measureIt != innums.end()) {
              //Removes SerialNums that exist from the inlist
              if(*measureIt == checksnum) {
                innums.erase(measureIt);
              }
              else {
                measureIt ++;
              }
            }
          }
        }

        //Records if the currentsnum is not in the input cube list
        bool contains = false;
        for(int l=0; l < (int)listednums.size(); l++) {
          if(currentsnum == listednums[l]) {
            contains = true;
            break;
          }
        }

        if(!contains) {
          nonlistednums.push_back(currentsnum);
        }
      }
    }

    progress.CheckStatus();
  }


  //Checks/detects islands
  set<string> index;
  map< string, set<string> > adjCubes = constructPointSets(index, innet);
  vector< set<string> > islands = findIslands(index, adjCubes);

  //Output islands in the file-by-file format
  // Islands that have no cubes listed in the input list will
  // not be shown.
  for(int i=0; i < (int)islands.size(); i++) {
    string name(Filename(prefix + "Island." + iString(i+1)).Expanded());
    ofstream out_stream;
    out_stream.open(name.c_str(), std::ios::out);
    out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file

    bool hasList = false;
    for(set<string>::iterator island = islands[i].begin();
         island != islands[i].end(); island++) {
      if (((string)num2cube[*island]).compare("") != 0) {
        if (hasList) {
          out_stream << "\n";
        }
        out_stream << Filename(num2cube[*island]).Name() << " " << *island;
        hasList = true;
      }
    }

    out_stream.close();

    if (!hasList) {
      remove(name.c_str());
    }
  }

  //Output the results to screen and files accordingly

  stringstream ss (stringstream::in | stringstream::out);

  ss << endl << "--------------------------------------------------------------------------------" << endl;
  if(islands.size() == 1) {
    ss << "The cubes are fully connected by the Control Net." << endl;
  }
  else if(islands.size() == 0) {
    ss << "There are no control points in the provided Control Measure [";
    ss << Filename(ui.GetFilename("NETWORK")).Name() << "]" << endl;
  }
  else {
    ss << "The cubes are NOT fully connected by the Control Net." << endl;
    ss << "There are " << islands.size() << " disjoint sets of cubes." << endl;
  }
  if(ui.GetBoolean("SINGLE")  &&  singleMeasureControlPoints.size() > 0) {
    string name(Filename(prefix + "SinglePointCubes.txt").Expanded());
    ofstream out_stream;
    out_stream.open(name.c_str(), std::ios::out);
    out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file

    for(set<string>::iterator measure = singleMeasureControlPoints.begin();
         measure != singleMeasureControlPoints.end(); measure++) {
      if (measure != singleMeasureControlPoints.begin()) {
        out_stream << "\n";
      }
      out_stream << *measure;
    }

    out_stream.close();

    ss << "--------------------------------------------------------------------------------" << endl;
    ss << "There are " << singleMeasureControlPoints.size();
    ss << " cubes in Control Points with only a single";
    ss << " Control Measure." << endl;
    ss << "The serial numbers of these measures are listed in [";
    ss <<  Filename(name).Name() + "]" << endl;
  }

  if(ui.GetBoolean("DUPLICATE")  &&  serialNumDupe.size() > 0) {
    string name(Filename(prefix + "DuplicateMeasures.txt").Expanded());
    ofstream out_stream;
    out_stream.open(name.c_str(), std::ios::out);
    out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file

    for(set<string>::iterator dupes = serialNumDupe.begin();
         dupes != serialNumDupe.end(); dupes++) {
      if (dupes != serialNumDupe.begin()) {
        out_stream << "\n";
      }
      out_stream << *dupes;
    }

    out_stream.close();

    ss << "--------------------------------------------------------------------------------" << endl;
    ss << "There are " << serialNumDupe.size();
    ss << " duplicate Control Measures in the";
    ss << " Control Net." << endl;
    ss << "The serial numbers of these duplicate Control Measures";
    ss << " are listed in [" + Filename(name).Name() + "]" << endl;
  }

  if(ui.GetBoolean("NOLATLON")  &&  noLatLonCubes.size() > 0) {
    string name(Filename(prefix + "NoLatLon.txt").Expanded());
    ofstream out_stream;
    out_stream.open(name.c_str(), std::ios::out);
    out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file

    for( set<string>::iterator latlon = noLatLonCubes.begin();
         latlon != noLatLonCubes.end(); latlon++ ) {

      if (latlon != noLatLonCubes.begin()) {
        out_stream << "\n";
      }

      out_stream << *latlon;

      for( set<string>::iterator cpoint = noLatLonCPoints[*latlon].begin();
           cpoint != noLatLonCPoints[*latlon].end(); cpoint++ ) {
        out_stream << " " << *cpoint;
      }
    }

    out_stream.close();

    ss << "--------------------------------------------------------------------------------" << endl;
    ss << "There are " << noLatLonCubes.size();
    ss << " serial numbers in the Control Net which are listed in the";
    ss << " input list and cannot compute latitude and longitudes." << endl;
    ss << "The Control Point IDs in which the serial number cannot compute";
    ss << " the latitudes and longitudes in are listed after the serial";
    ss << " number." << endl;
    ss << "These serial numbers and control points are listed in [";
    ss << Filename(name).Name() + "]" << endl;
  }

  // At this point, innums is the list of cubes NOT included in the
  // ControlNet, and innums are their those cube's serial numbers. #4 cont
  if(ui.GetBoolean("NOMATCH") && innums.size() > 0) {
    string name(Filename(prefix + "NoMatch.txt").Expanded());
    ofstream out_stream;
    out_stream.open(name.c_str(), std::ios::out);
    out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file
    
    for(int i=0; i < (int)innums.size(); i++) {
      if(i != 0) {
        out_stream << "\n";
      }

      out_stream << Filename(num2cube[innums[i]]).Name();
    }
    
    out_stream.close();
    
    ss << "--------------------------------------------------------------------------------" << endl;
    ss << "There are " << innums.size();
    ss << " cubes in the input list [" << Filename(ui.GetFilename("FROMLIST")).Name();
    ss << "] which do not exist in the Control Net [";
    ss << Filename(ui.GetFilename("NETWORK")).Name() << "]" << endl;
    ss << "These cubes are listed in [" + Filename(name).Name() + "]" << endl;
  }

  // In addition, nonlistednums should be the SerialNumbers of
  // ControlMeasures in the ControlNet that do not have a correlating
  // cube in the input list. #6
  if(ui.GetBoolean("NOCUBE")  &&  nonlistednums.size() > 0) {
    string name(Filename(prefix + "NoCube.txt").Expanded());
    ofstream out_stream;
    out_stream.open(name.c_str(), std::ios::out);
    out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file

    for(int i=0; i < (int)nonlistednums.size(); i++) {

      if (i != 0) {
        out_stream << "\n";
      }

      out_stream << nonlistednums[i];
    }

    out_stream.close();

    ss << "--------------------------------------------------------------------------------" << endl;
    ss << "There are " << nonlistednums.size();
    ss << " serial numbers in the Control Net [" << Filename(ui.GetFilename("NETWORK")).Basename();
    ss << "] which do not exist in the input list [";
    ss << Filename(ui.GetFilename("FROMLIST")).Name() << "]" << endl;
    ss << "These serial numbers are listed in [" + Filename(name).Name() + "]" << endl;
  }


  ss << "--------------------------------------------------------------------------------" << endl << endl;
  std::string log = ss.str();

  if (ui.IsInteractive()) {
    Application::GuiLog(log);
  }
  else {
    cout << ss.str();
  }

}


// Links cubes to other cubes it shares control points with
map< string, set<string> > constructPointSets(set<string> &index,
                                               ControlNet &innet) {
  map< string, set<string > > adjPoints;

  for(int i=0; i < innet.Size(); i++) {
    ControlPoint controlpt = innet[i];
    for(int j=0; j < controlpt.Size(); j++) {
      index.insert(controlpt[j].CubeSerialNumber());
      for(int k=0; k < controlpt.Size(); k++) {
        if(j != k) adjPoints[ controlpt[j].CubeSerialNumber() ].insert(
                                             controlpt[k].CubeSerialNumber());
      }
    }
  }

  return adjPoints;
}


// Uses a depth-first search to construct the islands
vector< set<string> > findIslands(set<string> &index,
                                   map< string, set<string> > &adjCubes) {
  vector< set<string> > islands;

  while(index.size() != 0) {
    set<string> connectedSet;

    stack<string> str_stack;
    set<string>::iterator first = index.begin();
    str_stack.push(*first);

    //Depth search
    while(true) {
      index.erase(str_stack.top());
      connectedSet.insert(str_stack.top());
            
      //Find the first connected unvisited node
      std::string nextNode = "";
      set<string> neighbors = adjCubes[str_stack.top()];
      for (set<string>::iterator i = neighbors.begin(); i != neighbors.end(); i++) {
        if (index.count(*i) == 1) {
          nextNode = *i;
          break;
        }
      }
            
      if (nextNode != "") {
        //Push the unvisited node
        str_stack.push(nextNode);
      }
      else {
        //Pop the visited node
        str_stack.pop();

        if (str_stack.size() == 0) break;
      }
    }

    islands.push_back(connectedSet);
  }

  return islands;
}
