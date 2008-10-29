#include "Isis.h"
#include "ProcessByBrick.h"

#include "SpecialPixel.h"
#include "iException.h"
#include "HiLab.h"

using namespace std; 
using namespace Isis;

void mult (vector<Buffer *> &in,
           vector<Buffer *> &out);
void sub (vector<Buffer *> &in,
          vector<Buffer *> &out);
void multSub (vector<Buffer *> &in,
          vector<Buffer *> &out);

void IsisMain() {
  // We will be processing by brick
  ProcessByBrick p;
  
  Isis::Cube *amatrixCube=NULL; 
  Isis::Cube *bmatrixCube=NULL; 

  // Setup the user input for the input/output files and the option
  UserInterface &ui = Application::GetUserInterface();

  // Setup the input HiRise cube
  Isis::Cube *icube = p.SetInputCube("FROM");

  if (icube->Bands() != 1) {
    std::string msg = "Only single-band HiRise cubes can be calibrated";
    throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
  }

  //Get pertinent label information to determine which band of matrix cube to use
  HiLab hilab(icube);

  int ccd = hilab.getCcd();

  int channel = hilab.getChannel();

  if (channel != 0  && channel != 1) {
    std::string msg = "Only unstitched cubes can be calibrated";
    throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
  }
  int band = 1 + ccd*2 + channel;

  string option = ui.GetString("OPTION");

  // Set attributes (input band number) for the matrix cube(s);
  CubeAttributeInput att("+" + iString(band));

  // Determine the file specification to the matrix file(s) if defaulted
  // and open 
  
  if (ui.WasEntered ("MATRIX") ) {
    if (option == "GAIN") {
      string matrixFile = ui.GetFilename("MATRIX");
      amatrixCube = p.SetInputCube(matrixFile, att);
    }
    else if (option == "OFFSET") {
      string matrixFile = ui.GetFilename("MATRIX");
      bmatrixCube = p.SetInputCube(matrixFile, att);
    }
    else { //(option == "BOTH")
      std::string 
        msg = "The BOTH option cannot be used if a MATRIX is entered";
      throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
    }
  }
  else {
    int tdi = hilab.getTdi();
    int bin = hilab.getBin();

    if (option == "OFFSET" || option == "BOTH") {
      std::string bmatrixFile = "$mro/calibration";
      bmatrixFile += "/B_matrix_tdi";
      bmatrixFile += iString(tdi) + "_bin" + iString(bin);
      bmatrixCube = p.SetInputCube(bmatrixFile, att);
    }
    if (option == "GAIN" || option == "BOTH") {
       std::string amatrixFile = "$mro/calibration";
       amatrixFile += "/A_matrix_tdi";
       amatrixFile += iString(tdi) + "_bin" + iString(bin);
       amatrixCube = p.SetInputCube(amatrixFile, att);
    }
  }

  // Open the output file and set processing parameters
  Cube *ocube = p.SetOutputCube ("TO");
  p.SetWrap (true);
  p.SetBrickSize ( icube->Samples(), 1, 1);

  // Add the radiometry group if it is not there yet.  Otherwise
  // read the current value of the keyword CalibrationParameters.
  // Then delete the keyword and rewrite it after appending the
  // new value to it.  Do it this way to avoid multiple Calibration
  // Parameter keywords.
  PvlGroup calgrp;
  PvlKeyword calKey;

  if (ocube->HasGroup("Radiometry")) {
    calgrp = ocube->GetGroup ("Radiometry");

    if (calgrp.HasKeyword("CalibrationParameters")) {
      calKey = calgrp.FindKeyword("CalibrationParameters");
      calgrp.DeleteKeyword( "CalibrationParameters" );
    }
    else {
      calKey.SetName ("CalibrationParameters");
    }
  }
  else {
    calgrp.SetName("Radiometry");
    calKey.SetName ("CalibrationParameters");
  }

  string keyValue = option;
  if (option == "GAIN") {
    keyValue += ":" + amatrixCube->Filename();
  }
  else if (option == "OFFSET") {
    keyValue += ":" + bmatrixCube->Filename();
  }
  else { // "BOTH"
    keyValue += ":"+bmatrixCube->Filename()+":"+amatrixCube->Filename();
  }

  calKey += keyValue;
  calgrp += calKey;
  ocube->PutGroup(calgrp);

  // Start the processing based on the option
  if (option == "GAIN") {
    p.StartProcess(mult);
  }
  else if (option == "OFFSET") {
    p.StartProcess(sub);
  }
  else { //(option == "BOTH") 
    p.StartProcess(multSub);
  }

  // Cleanup
  p.EndProcess();
}

// Mult routine 
void mult (vector<Buffer *> &in, vector<Buffer *> &out) {	
  Buffer &inp1 = *in[0];
  Buffer &inp2 = *in[1];
  Buffer &outp = *out[0];

  // Loop for each pixel in the line. 
  for (int i=0; i<inp1.size(); i++) {
// For hirise we will use the input file special pixel bef 
// matrix (??? ask Kris and/or Jeff).  If the input pixel is special, propagate
// it as per team request via Moses on 11/08/2006.
    if (IsSpecial (inp1[i])) {
      outp[i] = inp1[i];
    }
    else if (IsSpecial (inp2[i])) {
      outp[i] = inp2[i];
    }
    else {
      outp[i] = inp1[i] * inp2[i];
    }
  }
}

// Sub routine 
void sub (vector<Buffer *> &in, vector<Buffer *> &out) {	
  Buffer &inp1 = *in[0];
  Buffer &inp2 = *in[1];
  Buffer &outp = *out[0];

  // Loop for each pixel in the line. 
  for (int i=0; i<inp1.size(); i++) {
    if (IsSpecial (inp1[i])) {
       outp[i] = inp1[i];
     }
    else if (IsSpecial (inp2[i])) {
      outp[i] = inp2[i];
    }
    else {
      outp[i] = inp1[i] - inp2[i];
    }
  }
}

// multSub routine 
void multSub (vector<Buffer *> &in, vector<Buffer *> &out) {	
  Buffer &inp1 = *in[0];
  Buffer &inp2 = *in[1];
  Buffer &inp3 = *in[2];
  Buffer &outp = *out[0];

  // Loop for each pixel in the line. 
  for (int i=0; i<inp1.size(); i++) {
    if (IsSpecial (inp1[i])) {
      outp[i] = inp1[i];
     }
    else if (IsSpecial (inp2[i])) {
      outp[i] = inp2[i];
    }
    else if (IsSpecial (inp3[i])) {
      outp[i] = inp3[i];
     }
    else {
//      outp[i] = inp1[i] * inp2[i] - inp3[i];
      outp[i] = (inp1[i] - inp2[i]) * inp3[i];
    }
  }
}
