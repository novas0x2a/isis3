#include "Isis.h"

#include <set>
#include <stack>
#include <sstream>

#include "UserInterface.h"
#include "ControlNet.h"
#include "Filename.h"
#include "FileList.h"
#include "SerialNumber.h"
#include "CameraFactory.h"
#include "ProjectionFactory.h"
#include "PvlGroup.h"
#include "PvlKeyword.h"
#include "iString.h"

using namespace std; 
using namespace Isis;

map< string, set<string> > constructPointSets(set<string> &index,
                                              ControlNet &innet);
vector< set<string> > findIslands(set<string> &index, map< string,
                                            set<string> > &adjCubes);
void WriteOutput( map<string,string> num2cube, string suffix,
                  set<string> &sns, map< string, set<string> > &cps );

// Main program
void IsisMain() {
  Progress progress;
  UserInterface &ui = Application::GetUserInterface();
  ControlNet innet( ui.GetFilename("CNET"), NULL, true );
  iString prefix(ui.GetString("PREFIX"));
  bool ignore = ui.GetBoolean("IGNORE");

  // Sets up the list of serial numbers for
  FileList inlist(ui.GetFilename("FROMLIST"));
  set<iString> inListNums;
  map<iString,int> netSerialNumCount;
  vector<iString> listedSerialNumbers;
  map< string, string > num2cube;

  if (inlist.size() > 0) {
    progress.SetText("Initializing");
    progress.SetMaximumSteps(inlist.size());
    progress.CheckStatus();
  }

  for(int index = 0; index < (int)inlist.size(); index ++) {
    iString st = SerialNumber::Compose(inlist[index]);
    inListNums.insert(st);
    listedSerialNumbers.push_back(st);  // Used with nonListedSerialNumbers
    num2cube[st] = inlist[index];  // Used as a SerialNumber to Cube index
    progress.CheckStatus();
  }

  vector<iString> nonListedSerialNumbers;

  set<string> singleMeasureSerialNumbers;
  map< string, set<string> > singleMeasureControlPoints;

  set<string> duplicateSerialNumbers;
  map< string, set<string> > duplicateControlPoints;

  set<string> noLatLonSerialNumbers;
  map< string, set<string> > noLatLonControlPoints;
  set<string> latlonchecked;

  map< string, int > cubeMeasureCount;

  if (innet.Size() > 0) {
    progress.SetText("Calculating");
    progress.SetMaximumSteps(innet.Size());
    progress.CheckStatus();
  }

  for(int cp = 0; cp < innet.Size(); cp ++) {
    if(ignore && innet[cp].Ignore()) continue;

    ControlPoint controlpt(innet[cp]);

    // Checks for lat/Lon production
    if( ui.GetBoolean("NOLATLON") ) {
      for (int cm = 0; cm < controlpt.Size(); cm ++) {
        ControlMeasure controlms= controlpt[cm];

        // Make sure a cube is not checked twice
        if ( !num2cube[controlms.CubeSerialNumber()].empty()  &&
             latlonchecked.find(controlms.CubeSerialNumber()) == latlonchecked.end() ) {
          latlonchecked.insert( controlms.CubeSerialNumber() );

          Pvl pvl( num2cube[controlms.CubeSerialNumber()] );
          Camera *camera( CameraFactory::Create( pvl ) );
          if (camera == NULL) {
            try {
              // Checking if the Projection can be made
              delete ProjectionFactory::Create( pvl );
            } catch ( iException &e ) {
              noLatLonSerialNumbers.insert(controlms.CubeSerialNumber());
              noLatLonControlPoints[controlms.CubeSerialNumber()].insert( controlpt.Id() );
              e.Clear();
            }
          }
          else if(!camera->SetImage(controlms.Sample(), controlms.Line())) {
            noLatLonSerialNumbers.insert(controlms.CubeSerialNumber());
            noLatLonControlPoints[controlms.CubeSerialNumber()].insert( controlpt.Id() );
          }
          delete camera;
        }
      }
    }

    // Checks of the ControlPoint has only 1 Measure
    if(controlpt.NumValidMeasures() == 1) {
      string sn = controlpt[0].CubeSerialNumber();
      singleMeasureSerialNumbers.insert(sn);
      singleMeasureControlPoints[sn].insert( controlpt.Id() );

      // Records how many times a cube is in the ControlNet
      cubeMeasureCount[sn] ++;
    }
    else {
      // Checks for duplicate Measures for the same SerialNumber
      vector<ControlMeasure> controlMeasures;
      for(int cm = 0; cm < controlpt.Size(); cm ++) {
        if( ignore  &&  controlpt[cm].Ignore() ) continue;

        controlMeasures.push_back(controlpt[cm]);
        iString currentsn = controlpt[cm].CubeSerialNumber();

        // Records how many times a cube is in the ControlNet
        cubeMeasureCount[currentsn] ++;

        // Compares previous ControlMeasure SerialNumbers with the current
        for(int pre_cm = controlMeasures.size()-1-1; pre_cm >= 0; pre_cm --) {
          if( controlMeasures[pre_cm].CubeSerialNumber() == currentsn ) {
            duplicateSerialNumbers.insert(currentsn); //serial number duplication
            duplicateControlPoints[currentsn].insert( controlpt.Id() );
          }
        }

        // Removes from the serial number list, cubes that are included in the cnet
        inListNums.erase( currentsn );
        netSerialNumCount[currentsn] ++;

        // Records if the currentsnum is not in the input cube list
        bool contains = false;
        for(int sn = 0; sn < (int)listedSerialNumbers.size()  &&  !contains; sn ++) {
          if(currentsn == listedSerialNumbers[sn]) {
            contains = true;
          }
        }
        // Check if already added
        for(int sn = 0; sn < (int)nonListedSerialNumbers.size()  &&  !contains; sn ++) {
          if(currentsn == nonListedSerialNumbers[sn]) {
            contains = true;
          }
        }

        if(!contains) {
          nonListedSerialNumbers.push_back(currentsn);
        }
      }
    }

    progress.CheckStatus();

  }


  // Checks/detects islands
  set<string> index;
  map< string, set<string> > adjCubes = constructPointSets(index, innet);
  vector< set<string> > islands = findIslands(index, adjCubes);

  // Output islands in the file-by-file format
  //  Islands that have no cubes listed in the input list will
  //  not be shown.
  for(int i=0; i < (int)islands.size(); i++) {
    string name(Filename(prefix + "Island." + iString(i+1)).Expanded());
    ofstream out_stream;
    out_stream.open(name.c_str(), std::ios::out);
    out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file

    bool hasList = false;
    for(set<string>::iterator island = islands[i].begin();
         island != islands[i].end(); island++) {
      if (((string)num2cube[*island]).compare("") != 0) {
        out_stream << Filename(num2cube[*island]).Name() << " " << *island;
        out_stream << "\n";

        hasList = true;
      }
    }

    out_stream.close();

    if (!hasList) {
      remove(name.c_str());
    }
  }


  // Output the results to screen and files accordingly

  PvlGroup results("Results");

  stringstream ss (stringstream::in | stringstream::out);

  results.AddKeyword( PvlKeyword("Islands",iString((BigInt)islands.size())) );
  ss << endl << "----------------------------------------" \
                "----------------------------------------" << endl;
  if(islands.size() == 1) {
    ss << "The cubes are fully connected by the Control Network." << endl;
  }
  else if(islands.size() == 0) {
    ss << "There are no control points in the provided Control Network [";
    ss << Filename(ui.GetFilename("CNET")).Name() << "]" << endl;
  }
  else {
    ss << "The cubes are NOT fully connected by the Control Network." << endl;
    ss << "There are " << islands.size() << " disjoint sets of cubes." << endl;
  }

  if(ui.GetBoolean("SINGLEMEASURE")  &&  singleMeasureSerialNumbers.size() > 0) {
    results.AddKeyword( 
      PvlKeyword("SingleMeasure",iString((BigInt)singleMeasureSerialNumbers.size())) );

    string name(Filename(prefix + "SinglePointCubes.txt").Expanded());
    WriteOutput( num2cube, name,
                 singleMeasureSerialNumbers, singleMeasureControlPoints );

    ss << "----------------------------------------" \
          "----------------------------------------" << endl;
    ss << "There are " << singleMeasureSerialNumbers.size();
    ss << " cubes in Control Points with only a single";
    ss << " Control Measure." << endl;
    ss << "The serial numbers of these measures are listed in [";
    ss <<  Filename(name).Name() + "]" << endl;
  }

  if(ui.GetBoolean("DUPLICATE")  &&  duplicateSerialNumbers.size() > 0) {
    results.AddKeyword(
      PvlKeyword("DuplicateMeasures",iString((BigInt)duplicateSerialNumbers.size())) );

    string name(Filename(prefix + "DuplicateMeasures.txt").Expanded());
    WriteOutput( num2cube, name,
                 duplicateSerialNumbers, duplicateControlPoints );

    ss << "----------------------------------------" \
          "----------------------------------------" << endl;
    ss << "There are " << duplicateSerialNumbers.size();
    ss << " duplicate Control Measures in the";
    ss << " Control Net." << endl;
    ss << "The serial numbers of these duplicate Control Measures";
    ss << " are listed in [" + Filename(name).Name() + "]" << endl;
  }

  if(ui.GetBoolean("NOLATLON")  &&  noLatLonSerialNumbers.size() > 0) {
    results.AddKeyword(
      PvlKeyword("NoLatLonCubes",iString((BigInt)noLatLonSerialNumbers.size())) );

    string name(Filename(prefix + "NoLatLon.txt").Expanded());
    WriteOutput( num2cube, name,
                 noLatLonSerialNumbers, noLatLonControlPoints );

    ss << "----------------------------------------" \
          "----------------------------------------" << endl;
    ss << "There are " << noLatLonSerialNumbers.size();
    ss << " serial numbers in the Control Network which are listed in the";
    ss << " input list and cannot compute latitude and longitudes." << endl;
    ss << "These serial numbers, filenames, and control points are listed in [";
    ss << Filename(name).Name() + "]" << endl;
  }

  // At this point, inListNums is the list of cubes NOT included in the
  //  ControlNet, and inListNums are their those cube's serial numbers.
  if(ui.GetBoolean("NOCONTROL") && !inListNums.empty() ) {
    results.AddKeyword( PvlKeyword("NoControl",iString((BigInt)inListNums.size())) );

    string name(Filename(prefix + "NoControl.txt").Expanded());
    ofstream out_stream;
    out_stream.open(name.c_str(), std::ios::out);
    out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file
    
    for( set<iString>::iterator sn = inListNums.begin();
         sn != inListNums.end();
         sn ++) {
      out_stream << (*sn);
      out_stream << "\t" << Filename(num2cube[(*sn)]).Name();
      out_stream << "\n";
    }
    
    out_stream.close();
    
    ss << "----------------------------------------" \
          "----------------------------------------" << endl;
    ss << "There are " << inListNums.size();
    ss << " cubes in the input list [" << Filename(ui.GetFilename("FROMLIST")).Name();
    ss << "] which do not exist or are ignored in the Control Network [";
    ss << Filename(ui.GetFilename("CNET")).Name() << "]" << endl;
    ss << "These cubes are listed in [" + Filename(name).Name() + "]" << endl;
  }

  // In addition, nonListedSerialNumbers should be the SerialNumbers of
  //  ControlMeasures in the ControlNet that do not have a correlating
  //  cube in the input list.
  if(ui.GetBoolean("NOCUBE")  &&  nonListedSerialNumbers.size() > 0) {
    results.AddKeyword(
      PvlKeyword("NoCube",iString((BigInt)nonListedSerialNumbers.size())) );

    string name(Filename(prefix + "NoCube.txt").Expanded());
    ofstream out_stream;
    out_stream.open(name.c_str(), std::ios::out);
    out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file

    for(int sn=0; sn < (int)nonListedSerialNumbers.size(); sn++) {
      out_stream << nonListedSerialNumbers[sn];
      out_stream << "\n";
    }

    out_stream.close();

    ss << "----------------------------------------" \
          "----------------------------------------" << endl;
    ss << "There are " << nonListedSerialNumbers.size();
    ss << " serial numbers in the Control Net [";
    ss << Filename(ui.GetFilename("CNET")).Basename();
    ss << "] which do not exist in the input list [";
    ss << Filename(ui.GetFilename("FROMLIST")).Name() << "]" << endl;
    ss << "These serial numbers are listed in [";
    ss << Filename(name).Name() + "]" << endl;
  }

  // At this point cubeMeasureCount should be equal to the number of
  //  ControlMeasures associated with each serial number.
  if( ui.GetBoolean("SINGLECUBE") ) {
    set<string> singleMeasureCubes;
    for( map<string,int>::iterator cube = cubeMeasureCount.begin();
         cube != cubeMeasureCount.end();
         cube ++ ) {
      if( cube->second == 1 ) { singleMeasureCubes.insert( cube->first ); }
    }

    if( singleMeasureCubes.size() > 0 ) {
      results.AddKeyword(
        PvlKeyword("SingleCube",iString((BigInt)singleMeasureCubes.size())) );

      string name(Filename(prefix + "SingleCube.txt").Expanded());
      ofstream out_stream;
      out_stream.open(name.c_str(), std::ios::out);
      out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file

      for( set<string>::iterator sn = singleMeasureCubes.begin();
           sn != singleMeasureCubes.end();
           sn ++ ) {
        out_stream << (*sn);
        out_stream << "\t" << Filename(num2cube[(*sn)]).Name();
        out_stream << "\n";
      }

      out_stream.close();

      ss << "----------------------------------------" \
            "----------------------------------------" << endl;
      ss << "There are " << singleMeasureCubes.size();
      ss << " serial numbers in the Control Net [";
      ss << Filename(ui.GetFilename("CNET")).Basename();
      ss << "] which only exist in one Control Measure." << endl;
      ss << "These serial numbers are listed in [";
      ss << Filename(name).Name() + "]" << endl;
    }
  }

  ss << "----------------------------------------" \
        "----------------------------------------" << endl << endl;
  std::string log = ss.str();

  Application::Log(results);

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

  bool ignore = Application::GetUserInterface().GetBoolean("IGNORE");
  for(int cp=0; cp < innet.Size(); cp++) {

    if(ignore && innet[cp].Ignore()) continue;

    if(innet[cp].NumValidMeasures() < 2) continue;

    ControlPoint controlpt = innet[cp];
    // Map SerialNumbers together based on ControlMeasures
    for(int cm1=0; cm1 < controlpt.Size(); cm1++) {
      if(ignore && controlpt.Ignore()) continue;

      std::string sn = controlpt[cm1].CubeSerialNumber();
      index.insert(sn);
      for(int cm2=0; cm2 < controlpt.Size(); cm2++) {
        if(ignore && controlpt[cm2].Ignore()) continue;

        if(cm1 != cm2) {
          adjPoints[ sn ].insert( controlpt[cm2].CubeSerialNumber() );
        }
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

    // Depth search
    while(true) {
      index.erase(str_stack.top());
      connectedSet.insert(str_stack.top());
            
      // Find the first connected unvisited node
      std::string nextNode = "";
      set<string> neighbors = adjCubes[str_stack.top()];
      for (set<string>::iterator i = neighbors.begin(); i != neighbors.end(); i++) {
        if (index.count(*i) == 1) {
          nextNode = *i;
          break;
        }
      }
            
      if (nextNode != "") {
        // Push the unvisited node
        str_stack.push(nextNode);
      }
      else {
        // Pop the visited node
        str_stack.pop();

        if (str_stack.size() == 0) break;
      }
    }

    islands.push_back(connectedSet);
  }

  return islands;
}


// Writes the list of cubes [ SerialNumber, Filename, ControlPoints ] to the output file
void WriteOutput( map<string,string> num2cube, string filename,
                  set<string> &sns, map< string, set<string> > &cps ) {

  // Set up the output file for writing
  ofstream out_stream;
  out_stream.open(filename.c_str(), std::ios::out);
  out_stream.seekp(0,std::ios::beg); //Start writing from beginning of file

  for( set<string>::iterator sn = sns.begin();
                      sn != sns.end(); sn++ ) {
    // Serial Number of cube
    out_stream << *sn;

    // Filename of cube if given
    if( !num2cube[*sn].empty() ) {
      out_stream << "\t" << Filename(num2cube[*sn]).Name();
    }
    else {
      out_stream << "\tUnknownFilename";
    }

    // Control Points where the cube was found to have the issue
    for( set<string>::iterator cp = cps[*sn].begin();
         cp != cps[*sn].end(); cp++ ) {
      out_stream << "\t" << *cp;
    }

    out_stream << "\n";
  }

  out_stream.close();
}
