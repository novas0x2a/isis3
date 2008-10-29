#include "Isis.h"
#include "ProcessByBoxcar.h"
#include "Pvl.h"
#include "SpecialPixel.h"
#include "LineManager.h"
#include "Filename.h"
#include "iException.h"

using namespace std; 
using namespace Isis;

void filter (Buffer &in, double &v);
double *coefs;
double weight;

void IsisMain() {

  ProcessByBoxcar p;
  p.SetInputCube ("FROM");
  p.SetOutputCube ("TO");

  //Get information from the input kernel
    //Set up the user interface
    UserInterface &ui = Application::GetUserInterface();
    //Get the name of the input kernel
    Pvl pvl(ui.GetFilename("KERNEL"));
    //Access the Kernel group section of the input file
    PvlGroup g= pvl.FindGroup("KERNEL");
    //Read the number of lines in the input kernel  
    int lines =  g["lines"];  //Read the number of lines in the input kernel
    //Read the number of samples in the input kernel
    int samples = g["samples"];
    //Read the weight in the input kernel
    weight = g["weight"];
    //Determine the size of the kernel
    int numCoef = samples * lines;
    //Reference a pointer to an array of kernel data values
    coefs = new double[numCoef]; 
    //Iterate through the input kernel's data values to fill the coefs array
    for (int i=0 ; i < numCoef ; i++){	
      coefs[i] = g["Data"][i];  
    }
  
  //Set the Boxcar size based on the input kernel
  p.SetBoxcarSize (samples,lines);
  
  
  p.StartProcess(filter); 
  p.EndProcess ();

}

void filter (Buffer &in, double &v){
  v = 0;
  
  for (int i= 0 ; i < in.size() ; i++){
    if (!IsSpecial(in[i])){
      v += in[i] * coefs[i] ;
    }
  }
  v *= weight;
}
