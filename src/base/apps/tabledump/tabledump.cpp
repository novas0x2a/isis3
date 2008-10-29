#define GUIHELPERS

#include "Isis.h"
#include "Table.h"
#include "Filename.h"
#include "iString.h"
#include <sstream>

using namespace Isis;
using namespace std;

void helperButtonGetTableList();

map <string,void*> GuiHelpers(){
  map <string,void*> helper;
  helper ["helperButtonGetTableList"] = (void*) helperButtonGetTableList;
  return helper;
}

void IsisMain(){
  //Gather parameters from the UserInterface
  UserInterface &ui = Application::GetUserInterface();
  Filename file = ui.GetFilename("FROM");
  string tableName = ui.GetString("NAME");
  Table table(tableName, file.Expanded());

  //Set the character to separate the entries
  string delimit;
  if (ui.GetString("DELIMIT") == "COMMA") {
    delimit = ",";
  }
  else if (ui.GetString("DELIMIT") == "SPACE") {
    delimit = " ";
  }
  else {
    delimit = ui.GetString("CUSTOM");
  }

  //Open the file and output the column headings
  stringstream ss (stringstream::in | stringstream::out);

  for (int i=0; i< table[0].Fields(); i++) {
    for (int j=0; j<table[0][i].Size(); j++) {
      string title = table[0][i].Name();
      if ( table[0][i].IsText() ) {
        j += table[0][i].Bytes();
      }
      else if (table[0][i].Size() > 1) {
        title += "(" + iString(j) + ")";
      }
      if (i == table[0].Fields()-1 && j == table[0][i].Size()-1) {
        //We've reached the last field, omit the delimiter
        ss << title;
      }
      else{
        ss << title + delimit;
      }
    }
  }

  //Loop through for each record
  for (int i=0; i<table.Records(); i++) {
    ss.put('\n');

    //Loop through each Field in the record
    for (int j=0; j<table[i].Fields(); j++) {
      //if there is only one entry in this field,
      //cast and output accordingly
      if (table[i][j].Size() == 1) {
        if (table[i][j].IsInteger()) {
          ss << iString((int)table[i][j]);
        }
        else if (table[i][j].IsDouble()) {
          ss << iString((double)table[i][j]);
        }
        else if (table[i][j].IsText()) {
          ss << (string)table[i][j];
        }
        if (j < table[i].Fields()-1) {
          ss << delimit;
        }
      }
      //Otherwise, build a vector to contain the entries,
      //and output them with the delimiter character between
      else {
        if ( table[i][j].IsText() ) {
          ss << (string)table[i][j] << delimit;
        }
        else if (table[i][j].IsInteger()) {
          vector<int> currField = table[i][j];
          for (int k=0; k<(int)currField.size(); k++) {
            //check to see that we aren't on either the last field, or
            //(if we are), we aren't on the last element of the field
            if (j < table[i].Fields()-1 || 
                k < (int)currField.size()-1) {
              ss << currField[k] << delimit;
            }
            else {
              ss << currField[k];
            }
          }
        }
        else if (table[i][j].IsDouble()) {
          vector<double> currField = table[i][j];
          for (int k=0; k<(int)currField.size(); k++) {
            //check to see that we aren't on either the last field, or
            //(if we are), we aren't on the last element of the field
            if (j < table[i].Fields()-1 || 
                k < (int)currField.size()-1) {
              ss << currField[k] << delimit;
            }
            else {
              ss << currField[k];
            }
          }
        }
      }
    } //End Field loop
  }//End Record loop

  if(ui.WasEntered("TO")) {
    string outfile(Filename(ui.GetFilename("TO")).Expanded());
    ofstream outFile(outfile.c_str());
    outFile << ss.str();
    outFile.close();
  }
  else if (ui.IsInteractive()){
    std::string log = ss.str();
    Application::GuiLog(log);
  }
  else {
    cout << ss.str() << endl;
  }
}

//Function to find the available table names and put them into the GUI
void helperButtonGetTableList() {
  string list;
  UserInterface &ui = Application::GetUserInterface();
  const Pvl label (Filename(ui.GetFilename("FROM")).Expanded());
  int tfound = 0;
  for (int i=0; i<label.Objects(); i++) {
    if (label.Object(i).Name() == "Table") {
      if (tfound>0) {
        list +=",";
      }
      list += label.Object(i)["Name"][0].c_str();
      tfound++;
    }
  }
  ui.Clear("NAME");
  ui.PutString("NAME",list);
}
