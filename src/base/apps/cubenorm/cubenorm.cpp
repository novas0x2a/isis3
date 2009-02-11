#include "Isis.h"

// system include files go first
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

// Isis specific include files go next
#include "ProcessByTile.h"
#include "ProcessByLine.h"
#include "SpecialPixel.h"
#include "iException.h"
#include "Pvl.h"
#include "Statistics.h"

//Application specific includes
#include "staticStats.h"

using namespace std;
using namespace Isis;
using namespace PIRL;

// These global vectors are used to keep track of info for columns or rows
// of image data.  For example, a 100 sample x 200 line x 2 band cube will
// have a vectors of 200 columns when processing in the column direction
// (100 samples x 2 bands).  Likewise, the vectors will have 400 rows when
// processing in the line direction (200 lines x 2 bands)
static vector<StaticStats> st;
static vector<int> band;
static vector<int> element;
static vector<double> median;
static vector<double> average;
static vector<double> normalizer;
static int rowcol;                    // how many rows or cols per band
static bool normalizeUsingAverage;    // mult/sub using average or median?

// Size of the cube
static int totalLines;
static int totalSamples;
static int totalBands;

string direction;

// function prototypes
void getStats(Buffer &in);
void multiply(Buffer &in, Buffer &out);
void subtract(Buffer &in, Buffer &out);
void pvlOut(const string &pv);
void tableOut(const string &pv);
void PVLIn(const Isis::Filename & filename);
void tableIn(const Isis::Filename & filename);
void subSame();
void multSame();

// Main Program
void IsisMain() {
  // ERROR CHECK:  The user must specify at least the TO or STATS
  // parameters.
  UserInterface &ui = Application::GetUserInterface();
  if (!(ui.WasEntered("TO")) && !(ui.WasEntered("STATS"))) {
    string msg = "User must specify a TO and/or STATS file.";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  // We will be processing by tile.  This is nice since we change change the
  // shape of our buffer to be either a row or column
  ProcessByTile p;

  // Setup the input cube;
  // Obtain information from the input file
  Cube *icube = p.SetInputCube("FROM");
  totalSamples = icube->Samples();
  totalLines   = icube->Lines();
  totalBands   = icube->Bands();

  // Setup the tile size based on the direction of normalization
  direction = ui.GetString("DIRECTION");
  if (direction == "COLUMN") {
    p.SetTileSize(1, totalLines);
    rowcol = totalSamples;
  }
  else {
    p.SetTileSize(totalSamples, 1);
    rowcol = totalLines;
  }

  // Now get the statistics for each row or column
  normalizeUsingAverage = ui.GetString("NORMALIZER") == "AVERAGE";

  //gather statistics
  if (ui.GetString("STATSOURCE") == "CUBE"){
    p.StartProcess(getStats);
  }
  else if (ui.GetString("STATSOURCE") == "TABLE"){
    tableIn(ui.GetFilename("FROMSTATS"));
  }
  else{
    PVLIn(ui.GetFilename("FROMSTATS"));
  }
 
  //check to make sure the first vector has as many elements as the last
  // vector, and that there is a vector element for each row/col
  if ((band.size() != (unsigned int)(rowcol*totalBands)) || 
  	(st.size() != (unsigned int)(rowcol*totalBands))){
    string message = "You have entered an invalid input file " + 
      ui.GetFilename("FROMSTATS");
    throw  iException::Message(Isis::iException::Io, message,
    	 _FILEINFO_);
  }

  //If a STATS file was specified then create statistics file
  if (ui.WasEntered("STATS")) {
     string op = ui.GetString("FORMAT");
     if (op == "PVL")    pvlOut(ui.GetFilename("STATS"));
     if (op == "TABLE")  tableOut(ui.GetFilename("STATS"));
  }

  // If an output file was specified then normalize the cube
  if (ui.WasEntered ("TO")) {
    // Before creating a normalized cube check to see if there
    // are any column averages less than or equal to zero.
    if (ui.GetString("MODE") == "MULTIPLY") {
      for (unsigned int i=0; i<st.size(); i++) {
        if (IsValidPixel(normalizer[i]) && normalizer[i]<=0.0) {
          string msg = "Cube file can not be normalized with [MULTIPLY] ";
          msg += "option, some column averages <= 0.0";
          throw iException::Message(iException::User,msg,_FILEINFO_);
        }
      }
    }

    // Setup the output file and apply the coefficients by either
    // subtracting or multipling them
    p.SetOutputCube("TO");

    // Should we preserve the average/median of the input image???
    if (ui.GetBoolean("PRESERVE")) {
      if (ui.GetString("MODE") == "SUBTRACT") {
        subSame();
      }
      else {
        multSame();
      }
    }

    // Process based on the mode
    if (ui.GetString("MODE") == "SUBTRACT") {
      p.StartProcess(subtract);
    }
    else {
      p.StartProcess(multiply);
    }
  }

  // Cleanup
  p.EndProcess();
  st.clear();
  band.clear();
  element.clear();
  median.clear();
  average.clear();
  normalizer.clear();
}

//**********************************************************
// DOUSER - Get statistics on a column or row of pixels
//**********************************************************
void getStats(Buffer &in) {
  Statistics stats;
  stats.AddData(in.DoubleBuffer(),in.size());
  
  StaticStats newStat(stats.Average(), stats.StandardDeviation(), 
                      stats.ValidPixels(), stats.Minimum(), stats.Maximum());
  
  st.push_back(newStat);
  band.push_back(in.Band());
  if (direction == "COLUMN") {
    element.push_back(in.Sample());
  }
  else {
    element.push_back(in.Line());
  }

  // Sort the input buffer
  vector<double> pixels;
  for (int i=0; i<in.size(); i++) {
    if (IsValidPixel(in[i])) pixels.push_back(in[i]);
  }
  sort(pixels.begin(),pixels.end());

  // Now obtain the median value and store in the median vector
  int size = pixels.size();
  if (size != 0) {
    int med = size/2;
    if (size%2 == 0) {
      median.push_back((pixels[med-1]+pixels[med])/2.0);
    }
    else {
      median.push_back(pixels[med]);
    }
  }
  else {
    median.push_back(Isis::Null);
  }

  // Determine the normalizer
  if (normalizeUsingAverage) {
    normalizer.push_back(stats.Average());
  }
  else {
    normalizer.push_back(median[median.size()-1]);
  }
}

//********************************************************
// Create PVL output of statistics
//*******************************************************
void pvlOut(const string &StatFile) {
  PvlGroup results ("Results");
  for (unsigned int i=0; i <st.size(); i++) {
    results += PvlKeyword("Band",band[i]);
    results += PvlKeyword("RowCol",element[i]);
    results += PvlKeyword("ValidPixels",st[i].ValidPixels());
    if (st[i].ValidPixels() > 0) {
      results += PvlKeyword("Mean",st[i].Average());
      results += PvlKeyword("Median",median[i]);
      results += PvlKeyword("Std",st[i].StandardDeviation());
      results += PvlKeyword("Minimum",st[i].Minimum());
      results += PvlKeyword("Maximum",st[i].Maximum());
    }
    else {
      results += PvlKeyword("Mean",0.0);
      results += PvlKeyword("Median",0.0);
      results += PvlKeyword("Std",0.0);
      results += PvlKeyword("Minimum",0.0);
      results += PvlKeyword("Maximum",0.0);
    }
  }

  Pvl t;
  t.AddGroup(results);
  t.Write(StatFile);
}

//********************************************************
// Create Tabular output of statistics
//*******************************************************
void tableOut(const string &StatFile) {
  // Open output file
  // TODO check status and throw error
  ofstream out;
  out.open(StatFile.c_str(),std::ios::out);

  // Output a header
  out << std::setw(8)  << "Band";
  out << std::setw(8)  << "RowCol";
  out << std::setw(15) << "ValidPoints";
  out << std::setw(15) << "Average";
  out << std::setw(15) << "Median";
  out << std::setw(15) << "StdDev";
  out << std::setw(15) << "Minimum";
  out << std::setw(15) << "Maximum";
  out << endl;

  // Print out the table results
  for (unsigned int i=0; i<st.size(); i++) {
    out << std::setw(8)  << band[i];
    out << std::setw(8)  << element[i];
    out << std::setw(15) << st[i].ValidPixels();
    if (st[i].ValidPixels() > 0) {
      out << std::setw(15) << st[i].Average();
      out << std::setw(15) << median[i];
      //Make sure the table's SD is 0 for RowCols with 1 or less valid pixels
      if( st[i].ValidPixels() > 1 ) {
        out << std::setw(15) << st[i].StandardDeviation();
      }
      else {
        out << std::setw(15) << 0;
      }
      out << std::setw(15) << st[i].Minimum();
      out << std::setw(15) << st[i].Maximum();
    }
    else {
      out << std::setw(15) << 0;
      out << std::setw(15) << 0;
      out << std::setw(15) << 0;
      out << std::setw(15) << 0;
      out << std::setw(15) << 0;
    }
    out << endl;
  }
  out.close();
}

//********************************************************
// Gather statistics from a PVL input file
//*******************************************************
void PVLIn(const Isis::Filename &filename){
  Pvl pvlFileIn;
  pvlFileIn.Read(filename.Name());
  PvlGroup results = pvlFileIn.FindGroup("Results");
  PvlObject::PvlKeywordIterator itr = results.Begin();
  
  while (itr != results.End()){
    StaticStats newStat;  
    band.push_back((*itr)[0]);
    itr++;
    element.push_back((*itr)[0]);
    itr++;
    newStat.setValidPixels((*itr)[0]);
    itr++;
    newStat.setMean((*itr)[0]);
    itr++;
    median.push_back((*itr)[0]);
    itr++;
    newStat.setStandardDeviation((*itr)[0]);
    itr++;
    newStat.setMinimum((*itr)[0]);
    itr++;
    newStat.setMaximum((*itr)[0]);
    itr++;
    st.push_back(newStat);
  
  // Determine the normalizer
  if (normalizeUsingAverage) {
    normalizer.push_back(newStat.Average());
  }
  else {
      normalizer.push_back(median[median.size()-1]);
    }
  }
}

//********************************************************
// Gather statistics from a table input file
//*******************************************************
void tableIn(const Isis::Filename & filename){
  ifstream in;
  string expanded(filename.Expanded());
  in.open(expanded.c_str(),std::ios::in);
  
  
  if (!in){
    string message = "Error opening " + filename.Expanded();
    throw  iException::Message(Isis::iException::Io, message,
    	 _FILEINFO_);
  }
  
  //skip the header (106 bytes)
  in.seekg(106);
  

  //read it
  StaticStats newStat;
  iString inString;
  while (in >> inString){
    band.push_back(inString);
    in>> inString;
    element.push_back(inString);
    in>> inString;
    newStat.setValidPixels(inString);
    in >> inString;
    newStat.setMean(inString);
    in >> inString;
    median.push_back(inString);
    in >> inString;
    newStat.setStandardDeviation(inString);
    in >> inString;
    newStat.setMinimum(inString);
    in >> inString;
    newStat.setMaximum(inString);
    st.push_back(newStat);
    //Make sure Standard Deviation is not < 0 when reading in from a table
    if( newStat.StandardDeviation() < 0 ) {
      newStat.setStandardDeviation( 0 );
    }
    // Determine the normalizer
    if (normalizeUsingAverage) {
      normalizer.push_back(newStat.Average());
    }
    else {
      normalizer.push_back(median[median.size()-1]);
    }
  }
  in.close();
}

//********************************************************
// Compute coefficients such that when we subtract the average or median
// of the output image stays the same
void subSame() {
  // Loop for each band
  for (int iband=1; iband<=totalBands; iband++) {
    double sumAverage = 0.0;
    double sumValidPixels = 0;
    for (int i=0; i<rowcol; i++) {
      int index = (iband - 1) * rowcol + i;
      if (IsValidPixel(normalizer[index])) {
        sumAverage += normalizer[index] * st[index].ValidPixels();
        sumValidPixels += st[index].ValidPixels();
      }
    }

    // Neither sumValidPixels nor totalAverage will be zero
    // because of a test done earlier in IsisMain
    double totalAverage = sumAverage/sumValidPixels;
    for (int i=0; i<rowcol; i++) {
      int index = (iband - 1) * rowcol + i;
      if (IsValidPixel(normalizer[index])) {
        normalizer[index] = normalizer[index] - totalAverage;
      }
    }
  }
}

// Compute coefficients such that when we divide using the coefficient
// the average or median of the output image stays the same
void multSame() {
  // Loop for each band
  for (int iband=1; iband<=totalBands; iband++) {
    double sumAverage = 0.0;
    double sumValidPixels = 0;
    for (int i=0; i<rowcol; i++) {
      int index = (iband - 1) * rowcol + i;
      if (IsValidPixel(normalizer[index])) {
        sumAverage += normalizer[index] * st[index].ValidPixels();
        sumValidPixels += st[index].ValidPixels();
      }
    }

    // Neither sumValidPixels nor totalAverage will be zero
    // because of a test done earlier in IsisMain
    double totalAverage = sumAverage/sumValidPixels;

    for (int i=0; i<rowcol; i++) {
      int index = (iband - 1) * rowcol + i;
      if (IsValidPixel(normalizer[index])) {
        normalizer[index] = normalizer[index] / totalAverage;
      }
    }
  }
}

// Apply coefficients multiplicatively
void multiply(Buffer &in, Buffer &out) {
  // Compute the index into the normalizer array
  // We have to tweak the index based on the shape of the buffer
  // either a column or line
  int index;
  if (in.SampleDimension() == 1) {
    index = (in.Band() - 1) * totalSamples;   // Get to the proper band
    index += in.Sample() - 1;                 // Get to the proper column
  }
  else {
    index = (in.Band() - 1) * totalLines;     // Get to the proper band
    index += in.Line() - 1;                   // Get to the proper row
  }
  double coeff = normalizer[index];

  // Now loop and apply the coefficents
  for (int i=0; i<in.size(); i++) {
    if (IsSpecial(in[i])) {
      out[i] = in[i];
    }
    else {
      out[i] = Null;
      if (coeff != 0.0 && IsValidPixel(coeff)) {
        out[i] = in[i]/coeff;
      }
    }
  }
}

// Apply coefficients subtractively
void subtract(Buffer &in, Buffer &out) {
  // Compute the index into the normalizer array
  // We have to tweak the index based on the shape of the buffer
  // either a column or line
  int index;
  if (in.SampleDimension() == 1) {
    index = (in.Band() - 1) * totalSamples;   // Get to the proper band
    index += in.Sample() - 1;                 // Get to the proper column
  }
  else {
    index = (in.Band() - 1) * totalLines;     // Get to the proper band
    index += in.Line() - 1;                   // Get to the proper row
  }
  double coeff = normalizer[index];

  // Now loop and apply the coefficents
  for (int i=0; i<in.size(); i++) {
    if (IsSpecial(in[i])) {
      out[i] = in[i];
    }
    else {
      out[i] = Null;
      if (IsValidPixel(coeff)) {
        out[i] = in[i] - coeff;
      }
    }
  }
}