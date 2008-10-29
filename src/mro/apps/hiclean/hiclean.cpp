#include "Isis.h"

#include <iostream>
#include <sstream>

#include "ProcessByLine.h"
#include "UserInterface.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "iString.h" 
#include "HiImageClean.h"

using namespace Isis;

///  Declare the image cleaner object
static HiImageClean *iclean(0);

/**
 * @brief Scrub each image line from the input cube
 * 
 * This function is the ISIS main handler that manages input and output
 * buffers from the ISIS process manager.  After initializing with the
 * HiRISE ancillary data, this routine calls the method that applies the
 * radiometric calibration procedure.
 * 
 * @param[in] (Buffer) in Input line buffer
 * @param[out] (Buffer) out Output line buffer
 */
void cleanImage (Buffer &in , Buffer &out) {
  iclean->cleanLine(in, out);
  return;
}


void IsisMain(){

  UserInterface &ui = Application::GetUserInterface();

  ProcessByLine proc;
  Cube *cube = proc.SetInputCube("FROM");
  BigInt npixels(cube->Lines() * cube->Samples());
  
//  Initialize the cleaner routine
  try {
    delete iclean;
    iclean = new HiImageClean(*cube);
  } 
  catch (iException &ie) {
    std::string message = "Error attempting to initialize HiRISE cleaner object";
    throw (iException::Message(iException::Programmer,message,_FILEINFO_));
  }
  catch (...) {
    std::string message = "Unknown error occured attempting to initialize "
                          "HiRISE cleaner object";
    throw (iException::Message(iException::Programmer,message,_FILEINFO_));
  }

//  For IR10, channel 1 lets restrict the last 3100 lines of dark current
  PvlGroup &instrument = cube->GetGroup("Instrument");
  std::string ccd = (std::string) instrument["CcdId"];
  int channel = instrument["ChannelNumber"];
  if ((ccd == "IR10") && (channel  == 1)) {
    int bin = instrument["Summing"];
    int lastLine = cube->Lines() - ((3100/bin) + iclean->getFilterWidth()/2);
    if (lastLine > 1) { iclean->setLastGoodLine(lastLine); }
  }

#if defined(DEBUG)
  std::cout << "Lines: " << cube->Lines() << "   GoodLines: " 
            << iclean->getLastGoodLine() << std::endl;    
#endif

//  Get the output file reference for label update
  Cube *ocube = proc.SetOutputCube("TO");
  proc.StartProcess(cleanImage);
  iclean->propagateBlobs(ocube);
  proc.EndProcess();

//  Write statistics to file if requested
  if (ui.WasEntered("CLEANSTATS")) {
    std::string darkfile = ui.GetFilename("CLEANSTATS");
    std::ofstream dfile;
    dfile.open(darkfile.c_str(), std::ios::out | std::ios::trunc);
    dfile << *iclean;
    dfile.close();
  }

//  Dump stats to standard out
  Pvl p;
  PvlGroup grp;
  iclean->PvlImageStats(grp);
  p.AddGroup(grp);
  Application::Log(grp);
  BigInt nNulled = iclean->TotalNulled();

  delete iclean;
  iclean = 0;

// Check for calibration problems
  if (nNulled != 0) {
    double tpixels((double) nNulled / (double) npixels);
    std::ostringstream mess;
    mess << "There were " << nNulled << " of " << npixels << " ("
         << std::setw(6) << std::setprecision(2) << (tpixels * 100.0)
         << "%) due to insufficient calibration data (LUTTED or Gaps)" 
         << std::ends;
    throw (iException::Message(iException::Math,mess.str(),_FILEINFO_));
  }
}


