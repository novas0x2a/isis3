#include "Isis.h"
#include "ProcessByLine.h"
#include "Pixel.h"
#include "LineManager.h"
#include "SpecialPixel.h"
#include "Cube.h"
#include "Table.h"
#include "AlphaCube.h"
#include "Projection.h"
#include "Pvl.h"
#include "PvlKeyword.h"

using namespace std; 
using namespace Isis;

int min_sample, max_sample, num_samples;
int min_line, max_line, num_lines;
int cur_band, num_bands;
bool crop_nulls, crop_hrs, crop_lrs, crop_his, crop_lis;
LineManager *in;
Cube cube;

void FindPerimeter (Buffer &in);
void NullRemoval (Buffer &out);


void IsisMain() {
  max_sample = 0;
  max_line = 0;
  cur_band = 1;  
  
  // Open the input cube
  UserInterface &ui = Application::GetUserInterface();
  string from = ui.GetAsString("FROM");
  CubeAttributeInput inAtt(from);
  cube.SetVirtualBands(inAtt.Bands());
  from = ui.GetFilename("FROM");
  cube.Open(from);

  crop_nulls = ui.GetBoolean("NULL");
  crop_hrs = ui.GetBoolean("HRS");
  crop_lrs = ui.GetBoolean("LRS");
  crop_his = ui.GetBoolean("HIS");
  crop_lis = ui.GetBoolean("LIS");

  min_sample = cube.Samples() + 1;
  min_line = cube.Lines() + 1;
  num_bands = cube.Bands();
  
  // Setup the input cube
  ProcessByLine p1;
  p1.SetInputCube("FROM");
      
  // Start the first pass
  p1.StartProcess(FindPerimeter);
  p1.EndProcess();

  if (min_sample == cube.Samples() + 1) {
    cube.Close();
    string msg = "There are no non-null pixels in the [FROM] cube";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  num_samples = max_sample - (min_sample - 1);
  num_lines = max_line - (min_line - 1);
  
  // Setup the output cube
  ProcessByLine p2;
  p2.SetInputCube("FROM");
  p2.PropagateTables(false);
  Cube *ocube = p2.SetOutputCube("TO", num_samples, num_lines, num_bands);
  p2.ClearInputCubes();  
  
  // propagate tables manually
  Pvl &inLabels = *cube.Label();
  
  // Loop through the labels looking for object = Table
  for(int labelObj = 0; labelObj < inLabels.Objects(); labelObj++) {
    PvlObject &obj = inLabels.Object(labelObj);
  
    if(obj.Name() != "Table") continue;
  
    // Read the table into a table object
    Table table(obj["Name"], from);
  
    ocube->Write(table);
  }

  // Construct a label with the results
  PvlGroup results("Results");
  results += PvlKeyword ("InputLines", cube.Lines());
  results += PvlKeyword ("InputSamples", cube.Samples());
  results += PvlKeyword ("StartingLine", min_line);
  results += PvlKeyword ("StartingSample", min_sample);
  results += PvlKeyword ("EndingLine", max_line);
  results += PvlKeyword ("EndingSample", max_sample);
  results += PvlKeyword ("OutputLines", num_lines);
  results += PvlKeyword ("OutputSamples", num_samples); 
  
  /*
  Pvl &outLabels = *ocube->Label();
  if(outLabels.FindObject("IsisCube").HasGroup("Kernels")) {
    PvlGroup &kerns = outLabels.FindObject("IsisCube").FindGroup("Kernels");
  
    string tryKey = "NaifIkCode";
    if(kerns.HasKeyword("NaifFrameCode")) {
      tryKey = "NaifFrameCode";
    }
  
    if(kerns.HasKeyword(tryKey)) {
      PvlKeyword ikCode = kerns[tryKey];
      kerns = PvlGroup("Kernels");
      kerns += ikCode;
    }
  }
  */
  
  // Create a buffer for reading the input cube
  in = new LineManager(cube);
  
  // Start the second pass
  p2.StartProcess(NullRemoval);

  // Adjust the upper corner x,y values if the cube is projected
  try {
    // Try to create a projection & set the x,y position
    Projection *proj = cube.Projection();
    proj->SetWorld(min_sample-0.5,min_line-0.5);

    // Set the new x,y coords in the cube label
    PvlGroup &mapgrp = cube.Label()->FindGroup("Mapping",Pvl::Traverse);
    mapgrp.AddKeyword(PvlKeyword("UpperLeftCornerX",proj->XCoord()), 
                       Pvl::Replace);
    mapgrp.AddKeyword(PvlKeyword("UpperLeftCornerY",proj->YCoord()),
                                      Pvl::Replace);
    ocube->PutGroup(mapgrp);
  }
  // Only write the alpha group if the cube is not projected
  catch (iException &e) {
    // Clear the exception
    e.Clear();

    // Add and/or update the alpha group
    AlphaCube aCube(cube.Samples(),cube.Lines(),
                        ocube->Samples(),ocube->Lines(),
                        min_sample-0.5,min_line-0.5,num_samples+0.5,num_lines+0.5);
    aCube.UpdateGroup(*ocube->Label());
  }

  p2.EndProcess(); 
  cube.Close();  

  /*
  // Write the results to the output file if the user specified one
  string outFile = Filename(ui.GetFilename("TO")).Expanded();
  ofstream os;
  bool writeHeader = false;
  //write the results in the requested format.
  //if (ui.GetString("FORMAT") == "PVL") {
  Pvl temp;
  temp.AddGroup(results);
  temp.Write(outFile);
  //} else {
    //if the format was not PVL, write out a flat file.
  //  os.open(outFile.c_str(),ios::out);
  //    writeHeader = true;
  //} 
  if(writeHeader) {
    for(int i = 0; i < results.Keywords(); i++) {
      os << results[i].Name();
      if(i < results.Keywords()-1) {
        os << ",";
      }
    }
    os << endl;
  }
  for(int i = 0; i < results.Keywords(); i++) {
    os << (string)results[i];
    if(i < results.Keywords()-1) {
      os << ",";
    }
  }
  os << endl;
  */

  delete in;
  in = NULL;
  // Write the results to the log
  Application::Log(results);
}

// Process each line to find the min and max lines and samples
void FindPerimeter (Buffer &in) {
  for (int i = 0; i < in.size(); i++) {    
    // Do nothing until we find a valid pixel, or a pixel we do not want to crop off
    if ((Pixel::IsValid(in[i])) || (Pixel::IsNull(in[i]) && !crop_nulls) || 
        (Pixel::IsHrs(in[i]) && !crop_hrs) || (Pixel::IsLrs(in[i]) && !crop_lrs) || 
        (Pixel::IsHis(in[i]) && !crop_his) || (Pixel::IsLis(in[i]) && !crop_lis)) {
      // The current line has a valid pixel and is greater than max, so make it the new max line
      if (in.Line() > max_line) max_line = in.Line();

      // This is the first line to contain a valid pixel, so it's the min line
      if (in.Line() < min_line) min_line = in.Line();      

      int cur_sample = i + 1;

      // We process by line, so the min sample is the valid pixel with the lowest index in all line arrays
      if (cur_sample < min_sample) min_sample = cur_sample;

      // Conversely, the max sample is the valid pixel with the highest index
      if (cur_sample > max_sample) max_sample = cur_sample;
    }   
  }
}

// Using the min and max values, create a new cube without the extra nulls
void NullRemoval (Buffer &out) {
  int iline = min_line + (out.Line() - 1);
  in->SetLine(iline, cur_band);
  cube.Read(*in);

  // Loop and move appropriate samples
  for (int i = 0; i < out.size(); i++) {
    out[i] = (*in)[(min_sample - 1) + i];
  }

  if (out.Line() == num_lines) cur_band++;
}
