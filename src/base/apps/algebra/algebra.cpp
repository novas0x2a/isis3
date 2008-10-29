#include "Isis.h"
#include "ProcessByLine.h"
#include "SpecialPixel.h"
#include "iException.h"

using namespace std; 
using namespace Isis;

void add (vector<Buffer *> &in,
          vector<Buffer *> &out);
void sub (vector<Buffer *> &in,
          vector<Buffer *> &out);
void mult (vector<Buffer *> &in,
           vector<Buffer *> &out);
void div  (vector<Buffer *> &in,
           vector<Buffer *> &out);
void unary (Buffer &in, Buffer &out);

double a,b,c,d,e;

void IsisMain() {
  // We will be processing by line
  ProcessByLine p;

  // Setup the input and output files
  UserInterface &ui = Application::GetUserInterface(); 
  // Setup the input and output cubes
  p.SetInputCube("FROM1");
  if (ui.WasEntered ("FROM2")) p.SetInputCube("FROM2");
  p.SetOutputCube ("TO");

  // Get the coefficients
  a = ui.GetDouble ("A");
  b = ui.GetDouble ("B");
  c = ui.GetDouble ("C");
  d = ui.GetDouble ("D");
  e = ui.GetDouble ("E");
  
  // Start the processing based on the operator
  string op = ui.GetString ("OPERATOR");
  if (op == "ADD" ) p.StartProcess(add);
  if (op == "SUBTRACT") p.StartProcess(sub);  
  if (op == "MULTIPLY") p.StartProcess(mult);
  if (op == "DIVIDE") p.StartProcess(div);
  if (op == "UNARY") p.StartProcess(unary);

  // Cleanup
  p.EndProcess();
}

// Add routine 
void add (vector<Buffer *> &in, vector<Buffer *> &out) {	
  Buffer &inp1 = *in[0];
  Buffer &inp2 = *in[1];
  Buffer &outp = *out[0];

  // Loop for each pixel in the line. 
  for (int i=0; i<inp1.size(); i++) {
    if (IsSpecial(inp1[i])) {
      outp[i] = inp1[i];
    }
    else if (IsSpecial(inp2[i])) {
      outp[i] = NULL8;
    }
    else {
      outp[i] = ((inp1[i] - d) * a) + ((inp2[i] - e) * b) + c;
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
    if (IsSpecial(inp1[i])) {
      outp[i] = inp1[i];
    }
    else if (IsSpecial(inp2[i])) {
      outp[i] = NULL8;
    }
    else {
      outp[i] = ((inp1[i] - d) * a) - ((inp2[i] - e) * b) + c;
    }
  }
}

// Sub routine 
void mult (vector<Buffer *> &in, vector<Buffer *> &out) {	
  Buffer &inp1 = *in[0];
  Buffer &inp2 = *in[1];
  Buffer &outp = *out[0];

  // Loop for each pixel in the line. 
  for (int i=0; i<inp1.size(); i++) {
    if (IsSpecial (inp1[i])) {
      outp[i] = inp1[i];
    }
    else if (IsSpecial (inp2[i])) {
      outp[i] = NULL8;
    }
    else {
      outp[i] = ((inp1[i] - d) * a) * ((inp2[i] - e) * b) + c;
    }
  }
}

// Div routine 
void div (vector<Buffer *> &in, vector<Buffer *> &out) {	
  Buffer &inp1 = *in[0];
  Buffer &inp2 = *in[1];
  Buffer &outp = *out[0];

  // Loop for each pixel in the line. 
  for (int i=0; i<inp1.size(); i++) {
    if (IsSpecial (inp1[i])) {
      outp[i] = inp1[i];
    }
    else if (IsSpecial (inp2[i])) {
      outp[i] = NULL8;
    }
    else {
      if ((inp2[i] - e) * b  == 0.0) {
        outp[i] = NULL8; 
      }
      else {
        outp[i] = ((inp1[i] - d) * a) / ((inp2[i] - e) * b) + c;
      }
    }
  }
}

// Unary routine 
void unary (Buffer &in, Buffer &out) {	
  // Loop for each pixel in the line. 
  for (int i=0; i<in.size(); i++) {
    if (IsSpecial (in[i])) {
      out[i] = in[i];
    }
    else {
      out[i] = in[i] * a + c;
    }
  }
}
