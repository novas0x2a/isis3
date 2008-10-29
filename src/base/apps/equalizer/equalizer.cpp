#include "Isis.h"
#include "ProcessByLine.h"
#include "SpecialPixel.h"
#include "FileList.h"
#include "LeastSquares.h"
#include "Filename.h"
#include "OverlapStatistics.h"
#include "LineManager.h"
#include "MultivariateStatistics.h"

#include "CubeAttribute.h"
#include "tnt_array2d.h"
#include "jama_svd.h"
#include <cmath>

using namespace Isis;

void apply(Buffer &in, Buffer &out);

std::vector<std::vector<double> > offset, gain, avg;
int imageNum;

void IsisMain() {
  // Get the list of cubes to mosaic
  FileList imageList;
  UserInterface &ui = Application::GetUserInterface();
  imageList.Read(ui.GetFilename("FROMLIST"));
  if (imageList.size() < 1) {
    std::string msg = "The list file [" + ui.GetFilename("FROMLIST") +
                 "] does not contain any data";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  // Read hold list if one was entered
  std::vector<int> hold;
  if (ui.WasEntered("HOLD")) {
    FileList holdList;
    holdList.Read(ui.GetFilename("HOLD"));

    // Make sure each file in the holdlist matches a file in the fromlist
    for (int i=0; i<(int)holdList.size(); i++) {
      bool matched = false;
      for (int j=0; j<(int)imageList.size(); j++) {
        if (holdList[i] == imageList[j]) {
          matched = true;
          hold.push_back(j);
          break;
        }
      }
      if (!matched) {
        std::string msg = "The hold list file [" + holdList[i] +
                     "] does not match a file in the from list";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }
    }
  }

  // Get the rest of the user input parameters
  bool applyopt = ui.GetBoolean("APPLY");
  std::string mode = ui.GetString("CMODE");
  std::string adjust = ui.GetString("ADJUST");
  bool wtopt = ui.GetBoolean("WEIGHT");
  int mincnt = ui.GetInteger("MINCOUNT");

  // Make sure the user enters a "TO" file if the apply option is not selected
  if (!applyopt) {
    if (!ui.WasEntered("TO")) {
      std::string msg = "If the apply option is not selected, you must enter a";
      msg += " TO file";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
  }

  PvlObject equ("EqualizationInformation");

  // Loop through all input cubes, making sure they all have the same number of
  // bands, and calculating statistics for each cube to use later
  avg.resize(imageList.size());
  PvlGroup a("ImageAverages");
  for (int i=0; i<(int)imageList.size(); i++){
    Process p;
    const CubeAttributeInput att;
    const std::string inp = imageList[i];
    Cube *icube = p.SetInputCube(inp, att);
    avg[i].resize(icube->Bands());
    for (int b=1; b<=icube->Bands(); b++) {
      avg[i][b-1] = icube->Statistics(1,
                     "Calculating Statistics for " + imageList[i])->Average();
      a += PvlKeyword(imageList[i], avg[i][b-1]);
    }
    if (i > 0) {
      if (icube->Bands() != (int)avg[i-1].size()) {
        std::string msg = "The number of bands must be the same for all input";
        msg += " images";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }
    }
    p.EndProcess();
  }
  equ.AddGroup(a);

  // We will loop through each input cube and get statistics needed for .
  // equalizing
  std::vector<OverlapStatistics> overlapList;
  std::vector<int> oIndex1;
  std::vector<int> oIndex2;
  for (unsigned int i=0; i<imageList.size(); i++) {
    Cube cube1;
    cube1.Open(imageList[i]);


    for (unsigned int j=(i+1); j<imageList.size(); j++) {
      Cube cube2;
      cube2.Open(imageList[j]);

      // Get overlap statistics for cubes
      OverlapStatistics oStats(cube1,cube2);

      // Only push the stats onto the oList vector if there is an overlap in at
      // least one of the bands
      if (oStats.HasOverlap()) {
        overlapList.push_back(oStats);
        oIndex1.push_back(i);
        oIndex2.push_back(j);
      }
    }
  }

  // Check to make sure that all input images overlap another image
  std::vector<bool> hasOverlap;
  hasOverlap.resize(imageList.size());
  for (int img=0; img<(int)imageList.size(); img++) hasOverlap[img] = false;
  for (int o=0; o<(int)overlapList.size(); o++) {
    hasOverlap[oIndex1[o]] = true;
    hasOverlap[oIndex2[o]] = true;
  }

  // Make sure that there is at least one overlap
  if (overlapList.size() == 0) {
      std::string msg = "None of the input images overlap";
      throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  // Print error if one of the images does not overlap another
  for (int img=0; img<(int)imageList.size(); img++) {
    bool allOverlap = true;
    std::string msg = "File(s) ";
    if (!hasOverlap[img]) {
       msg += "[" + imageList[img] + "] ";
    }
    msg += "does not overlap any other input images";
    if (!allOverlap) throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  // Resize arrays of gain and offset values and initialize the offset array to
  // 0's and the gain array to 1's
  gain.resize(imageList.size());
  offset.resize(imageList.size());
  for (int i=0; i<(int)imageList.size(); i++) {
    gain[i].resize(overlapList[0].Bands());
    offset[i].resize(overlapList[0].Bands());
    for (int b=0; b<overlapList[0].Bands(); b++) {
      offset[i][b] = 0.0;
      gain[i][b] = 1.0;
    }
  }

  // Loop through each band making all necessary calculations

  for (int band=1; band<=overlapList[0].Bands(); band++) {
    PvlGroup d("DeltaValues");
    std::vector<double> delta, wt;
    std::vector<int> validOverlaps;
    for (int o=0; o<(int)overlapList.size(); o++) {

      // Pull necessary data out of stats object
      double avgX = overlapList[o].GetMStats(band).X().Average();
      double avgY = overlapList[o].GetMStats(band).Y().Average();

      // Test band for overlap & valid data
      if (!overlapList[o].HasOverlap(band)) continue;
      if (overlapList[o].GetMStats(band).ValidPixels() < mincnt) continue;
      if (avgX == 0.0) continue;
      if (avgY == 0.0) continue;

      validOverlaps.push_back(o);
      delta.push_back(avgY - avgX);
      d += PvlKeyword(iString(o+1),delta[o]);

      // Fill wt vector with 1's if the overlaps are not to be weighted, or
      // fill the vector with the number of valid pixels in each overlap
      if (!wtopt) wt.push_back(1.0);
      else wt.push_back(overlapList[o].GetMStats(band).ValidPixels());
    }
    equ.AddGroup(d);

    // Make sure the number of valid overlaps + hold images is greater than the
    // number of input images (otherwise the least squares equation will be
    // unsolvable due to having more unknowns than knowns)
    if (validOverlaps.size() + hold.size() < imageList.size()) {
      std::string msg = "The number of overlaps and holds must be greater than";
      msg += " the number of input images";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    // Calculate offset
    if (adjust != "CONTRAST") {
      //Set up the least squares for offset
      BasisFunction basis("BasisFunction",imageList.size(),imageList.size());
      LeastSquares lsq(basis);

      // Add knowns to least squares for each overlap
      for (int overlap=0; overlap<(int)validOverlaps.size(); overlap++) {
        std::vector<double> input;
        input.resize(imageList.size());
        for (int i=0; i<(int)input.size(); i++) input[i] = 0.0;
        input[oIndex1[validOverlaps[overlap]]] = 1.0;
        input[oIndex2[validOverlaps[overlap]]] = -1.0;
        lsq.AddKnown(input,delta[overlap],wt[overlap]);
      }

      // Add a known to the least squares for each hold image
      for (int h=0; h<(int)hold.size(); h++) {
        std::vector<double> input;
        input.resize(imageList.size());
        for (int i=0; i<(int)input.size(); i++) input[i] = 0.0;
        input[hold[h]] = 1.0;
        lsq.AddKnown(input,0.0);
      }

      // Solve the least squares and get the offset coefficients to apply to the
      // images
      lsq.Solve(Isis::LeastSquares::QRD);
      PvlGroup b("Base");
      for (int i=0; i<basis.Coefficients(); i++) {
        offset[i][band-1] = basis.Coefficient(i);
        b += PvlKeyword(imageList[i],offset[i][band-1]);
      }
      equ.AddGroup(b);
    }

    // Calculate Gain
    if (adjust != "BRIGHTNESS") {

      //Set up the least squares for offset
      BasisFunction basis("BasisFunction",imageList.size(),imageList.size());
      LeastSquares lsq(basis);

      // Add knowns to least squares for each overlap
      PvlGroup p21("P21Values");
      for (int overlap=0; overlap<(int)validOverlaps.size(); overlap++) {
        std::vector<double> input;
        input.resize(imageList.size());
        for (int i=0; i<(int)input.size(); i++) input[i] = 0.0;
        input[oIndex1[validOverlaps[overlap]]] = 1.0;
        input[oIndex2[validOverlaps[overlap]]] = -1.0;

        double tanp;

        //@todo This contrast mode does not seem to be working correctly!!!!
        if (mode == "PCA") {
          // Get Eigenvectors for the overlap by using a matrices of variances
          // This is used to get the angle of rotation (theta) which will then
          // allow us to calculate the desired ratio of contrast (tanp)
          TNT::Array2D<double> a(2,2);
          a[0][0] = overlapList[overlap].GetMStats(band).X().Variance();
          a[0][1] = overlapList[overlap].GetMStats(band).Covariance();
          a[1][0] = overlapList[overlap].GetMStats(band).Covariance();
          a[1][1] = overlapList[overlap].GetMStats(band).Y().Variance();

          JAMA::SVD<double> svd(a);

          TNT::Array2D<double> v(2,2);
          svd.getV(v);
          double theta = acos(v[0][0]);

          // This would only happen if the overlaps were 100% uncorrelated
          if (theta == 90.0) {
            std::string msg = "Unexpected Programmer Error";
            throw iException::Message(iException::Programmer,msg,_FILEINFO_);
          }
          tanp = tan(theta);
        }
        else {
          if (overlapList[overlap].GetMStats(band).X().StandardDeviation() == 0.0) {
            tanp = 0.0;    //Set gain to 1.0
          }
          else {
            tanp = overlapList[overlap].GetMStats(band).Y().StandardDeviation()
                / overlapList[overlap].GetMStats(band).X().StandardDeviation();
          }
        }
        p21 += PvlKeyword(iString(overlap+1),tanp);
        if (tanp > 0.0) {
          lsq.AddKnown(input,log(tanp),wt[overlap]);
        }
        else {
          lsq.AddKnown(input,0.0,1e30);   //Set gain to 1.0
        }
      }
      equ.AddGroup(p21);

      // Add a known to the least squares for each hold image
      for (int h=0; h<(int)hold.size(); h++) {
        std::vector<double> input;
        input.resize(imageList.size());
        for (int i=0; i<(int)input.size(); i++) input[i] = 0.0;
        input[hold[h]] = 1.0;
        lsq.AddKnown(input,0.0);
      }

      //Solve the least squares and get the gain coefficients to apply to the
      // images
      lsq.Solve(Isis::LeastSquares::QRD);
      PvlGroup m("Multiplier");
      for (int i=0; i<basis.Coefficients(); i++) {
        gain[i][band-1] = exp(basis.Coefficient(i));
        m += PvlKeyword(imageList[i], gain[i][band-1]);
      }
      equ.AddGroup(m);
    }
  }

  // Setup the output text file if the user requested one
  if (ui.WasEntered("TO")) {
    std::string out = Filename(ui.GetFilename("TO")).Expanded();
    std::ofstream os;
    os.open(out.c_str(),std::ios::app);
    for (int i=0; i<(int)overlapList.size(); i++) {
      os << overlapList[i] << std::endl;
    }
    Pvl p;
    p.AddObject(equ);
    os << p << std::endl;
  }

  // Apply the correction to the images if the user wants this done
  if (applyopt) {
    for (int img=0; img<(int)imageList.size(); img++) {
      imageNum = img;
      ProcessByLine p;
      p.Progress()->SetText("Equalizing Image " + imageList[img]);
      CubeAttributeInput att;
      const std::string inp = imageList[img];
      Cube *icube = p.SetInputCube(inp, att);
      Filename file = imageList[img];
      const std::string out = file.Basename() + ".equ." + file.Extension();
      CubeAttributeOutput outAtt;
      p.SetOutputCube(out,outAtt,icube->Samples(),icube->Lines(),icube->Bands());
      p.StartProcess(apply);
      p.EndProcess();
    }
  }
}

// Apply the equalization to the cubes
void apply(Buffer &in, Buffer &out) {
  for (int i=0; i<in.size(); i++) {
    if (Isis::IsSpecial(in[i])) {
      out[i] = in[i];
    }
    else {
      out[i] = (in[i] - avg[imageNum][0]) * gain[imageNum][0] + avg[imageNum][0]
        + offset[imageNum][0];
    }
  }
}

