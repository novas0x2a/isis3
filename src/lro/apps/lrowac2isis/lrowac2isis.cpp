#include "Isis.h"

#include <cstdio>
#include <string>

#include "ProcessImportPds.h"
#include "LineManager.h"
#include "OriginalLabel.h"
#include "Brick.h"
#include "History.h"
#include "Stretch.h"

using namespace std; 
using namespace Isis;

vector<Cube *> outputCubes;
vector<int> frameletLines;
void separateFramelets(Buffer &in);
void writeNullsToFile();
void TranslateLabels (Pvl &pdsLab, Pvl &isis3VisEven, Pvl &isis3VisOdd, 
                      Pvl &isis3UvEven, Pvl &isis3UvOdd);

std::vector<int> padding;
int colorOffset = 0;

// Output UV Files
Cube *uveven = NULL;
Cube *uvodd = NULL;

// Output VIS Files
Cube *viseven = NULL;
Cube *visodd = NULL;

Stretch lookupTable;

bool flip = false;

void IsisMain () {
  colorOffset = 0;
  frameletLines.clear();
  outputCubes.clear();

  uveven = NULL;
  uvodd = NULL;
  viseven = NULL;
  visodd = NULL;

  ProcessImportPds p;
  Pvl pdsLab;

  UserInterface &ui = Application::GetUserInterface();
  string fromFile = ui.GetFilename("FROM");

  flip = ui.GetBoolean("FLIP");

  p.SetPdsFile (fromFile, "", pdsLab);

  lookupTable = Stretch();

  // read the lut if the option is on
  if(ui.GetBoolean("UNLUT") && pdsLab["LRO:LOOKUP_TABLE_TYPE"][0] == "STORED") {
    PvlKeyword lutKeyword = pdsLab["LRO:LOOKUP_CONVERSION_TABLE"];

    for (int i = 0; i < lutKeyword.Size(); i ++) {
      iString lutPair = lutKeyword[i];
      lutPair.ConvertWhiteSpace();
      lutPair.Remove("() ");
      iString outValueMin = lutPair.Token(" ,");
      iString outValueMax = lutPair.Token(" ,");
      lookupTable.AddPair(i, ((double)outValueMin + (double)outValueMax) / 2.0);
    }
  }

  string instModeId = pdsLab["INSTRUMENT_MODE_ID"];

  // this will be used to convert num input lines to num output lines,
  //   only changed for when both uv and vis exist (varying summing)
  double visOutputLineRatio = 1.0;
  double uvOutputLineRatio = 1.0;

  int numFilters = 0;
  if(ui.GetBoolean("COLOROFFSET")) {
    colorOffset = ui.GetInteger("COLOROFFSETSIZE");
  }

  // Determine our band information based on 
  // INSTRUMENT_MODE_ID - FILTER_NUMBER is
  // only going to be used for BW images
  if(instModeId == "COLOR") {
    numFilters = 7;
    frameletLines.push_back(4);
    frameletLines.push_back(4);
    frameletLines.push_back(14);
    frameletLines.push_back(14);
    frameletLines.push_back(14);
    frameletLines.push_back(14);
    frameletLines.push_back(14);

    uveven  = new Cube();
    uvodd   = new Cube();
    viseven = new Cube();
    visodd  = new Cube();

    // 14 output lines (1 framelet) from 5vis/2uv lines
    visOutputLineRatio = 14.0 / (14.0*5.0 + 4.0*2.0);

    // 4 output lines (1 framelet) from 5vis/2uv lines
    uvOutputLineRatio  = 4.0 / (14.0*5.0 + 4.0*2.0);
  }
  else if(instModeId == "VIS") {
    numFilters = 5;

    frameletLines.push_back(14);
    frameletLines.push_back(14);
    frameletLines.push_back(14);
    frameletLines.push_back(14);
    frameletLines.push_back(14);

    viseven = new Cube();
    visodd  = new Cube();

    // 14 output lines (1 framelet) from 5vis/2uv lines
    visOutputLineRatio = 14.0 / (14.0*5.0);
  }
  else if(instModeId == "UV") {
    numFilters = 2;

    frameletLines.push_back(4);
    frameletLines.push_back(4);

    uveven = new Cube();
    uvodd  = new Cube();

    // 4 output lines (1 framelet) from 2uv lines
    uvOutputLineRatio = 4.0 / (4.0*2.0);
  }
  else if(instModeId == "BW") {
    numFilters = 1;

    frameletLines.push_back(14);

    viseven = new Cube();
    visodd  = new Cube();
  }
  
  padding.resize(numFilters);
  

  for(int filter = 0; filter < numFilters; filter++) {
    padding[filter] = (colorOffset * frameletLines[filter]) * filter;

    // dont count UV for VIS offsetting
    if(instModeId == "COLOR" && filter > 1) {
      padding[filter] -= 2 * colorOffset * frameletLines[filter];
    }
  }

  Filename baseFilename(ui.GetFilename("TO"));

  if(uveven && uvodd) {
    // padding[1] is max padding forUV 
    int numSamples = ((viseven)? p.Samples()/4 : p.Samples()); 
    numSamples     = 128;  // UV is alway sum 4 so it is 128 samples
    int numLines   = (int)(uvOutputLineRatio * p.Lines() + 0.5) + padding[1];
    int numBands   = 2;

    uveven->SetDimensions(numSamples, numLines, numBands);
    uveven->SetPixelType(Isis::Real);

    string filename = baseFilename.Path() + "/" + baseFilename.Basename() + ".uv.even.cub";
    uveven->Create(filename);

    uvodd->SetDimensions(numSamples, numLines, numBands);
    uvodd->SetPixelType(Isis::Real);

    filename = baseFilename.Path() + "/" + baseFilename.Basename() + ".uv.odd.cub";
    uvodd->Create(filename);
  }

  if(viseven && visodd) {
    // padding[size-1] is max padding for vis (padding[0] or padding[4] or padding[6])
    int numSamples = p.Samples(); 
    int numLines   = (int)(visOutputLineRatio * p.Lines() + 0.5) + padding[padding.size()-1];
    int numBands   = ((uveven)? padding.size() - 2 : padding.size());

    viseven->SetDimensions(numSamples, numLines, numBands);
    viseven->SetPixelType(Isis::Real);

    string filename = baseFilename.Path() + "/" + baseFilename.Basename() + ".vis.even.cub";
    viseven->Create(filename);

    visodd->SetDimensions(numSamples, numLines, numBands);
    visodd->SetPixelType(Isis::Real);

    filename = baseFilename.Path() + "/" + baseFilename.Basename() + ".vis.odd.cub";
    visodd->Create(filename);
  }

  Pvl isis3VisEvenLab,
      isis3VisOddLab,
      isis3UvEvenLab,
      isis3UvOddLab;
  TranslateLabels(pdsLab, isis3VisEvenLab, isis3VisOddLab,
                  isis3UvEvenLab, isis3UvOddLab);

  writeNullsToFile();
  p.StartProcess (separateFramelets);
  p.EndProcess();

  // Add original labels
  OriginalLabel origLabel(pdsLab);

  int numFramelets = padding.size();
  PvlKeyword numFrameletsKeyword("NumFramelets", numFramelets);

  if(uveven) {
    for(int grp = 0; grp < isis3UvEvenLab.Groups(); grp++) {
      uveven->PutGroup(isis3UvEvenLab.Group(grp));
    }

    History history("IsisCube");
    history.AddEntry();
    uveven->Write(history);
    uveven->Write(origLabel);

    uveven->Close();
    delete uveven;
    uveven = NULL;
  }

  if(uvodd) {
    for(int grp = 0; grp < isis3UvOddLab.Groups(); grp++) {
      uvodd->PutGroup(isis3UvOddLab.Group(grp));
    }

    History history("IsisCube");
    history.AddEntry();
    uvodd->Write(history);
    uvodd->Write(origLabel);

    uvodd->Close();
    delete uvodd;
    uvodd = NULL;
  }

  if(viseven) {
    for(int grp = 0; grp < isis3VisEvenLab.Groups(); grp++) {
      viseven->PutGroup(isis3VisEvenLab.Group(grp));
    }

    History history("IsisCube");
    history.AddEntry();
    viseven->Write(history);
    viseven->Write(origLabel);

    viseven->Close();
    delete viseven;
    viseven = NULL;
  }

  if(visodd) {
    for(int grp = 0; grp < isis3VisOddLab.Groups(); grp++) {
      visodd->PutGroup(isis3VisOddLab.Group(grp));
    }

    History history("IsisCube");
    history.AddEntry();
    visodd->Write(history);
    visodd->Write(origLabel);

    visodd->Close();
    delete visodd;
    visodd = NULL;
  }
}

/**
 * Get the framelet number line is in.
 * 
 * @param line input line number (starting at 1) 
 * @param even Set to true if even framelet, otherwise false 
 * 
 * @return int framelet this line belongs to
 */
int getFrameletNumber(int line, int &frameletSetNumber, int &frameletSetOffset, 
                      int &frameletLineOffset, bool &even) {
  int frameletNumber = -1;

  int frameletSetSize = 0;

  // framelet set = one capture of vis/uv data (1 to 7 framelets)
  for(unsigned int i = 0; i < frameletLines.size(); i++) {
    frameletSetSize += frameletLines[i];
  }

  frameletSetNumber = (line-1) / frameletSetSize;

  // frameletSetNumber is 0-based, but even is 1-based, so logic reversed from
  //   what would be intuitive. An odd frameletSetNumber means even framelet.
  even = ((frameletSetNumber % 2) == 1);

  // offset into set
  frameletSetOffset = line - (frameletSetSize * frameletSetNumber) - 1;

  int offset = frameletSetOffset;
  for(unsigned int framelet = 0; frameletNumber < 0; framelet++) {
    offset -= frameletLines.at(framelet);

    if(offset < 0) {
      frameletNumber = framelet;
      frameletLineOffset = offset + frameletLines.at(framelet);
    }
  }

  return frameletNumber;
}

//! Separates each of the individual WAC framelets into the right place
void separateFramelets(Buffer &in) {
  // this is true if uv is summed and mixed with unsummed vis
  bool extractMiddleSamples = false;
  // This is the framelet set the line belongs to
  int frameletSet = 0;
  // this is the offset into the set
  int frameletSetOffset = 0;
  // this is true if framelet belongs in an even cube
  bool even = false;
  // line # in current framelet
  int frameletLineOffset = 0;
  // this is the framelet number the current line belongs in
  int framelet = getFrameletNumber(in.Line(), frameletSet, frameletSetOffset, frameletLineOffset, even);
  // this is the output file the current line belongs in
  Cube *outfile = NULL;

  // uv and vis outputs
  if(viseven && uveven) {
    if(framelet < 2 && even) {
      outfile = uveven;
      extractMiddleSamples = true;
    }
    else if(framelet < 2) {
      outfile = uvodd;
      extractMiddleSamples = true;
    }
    else if(even) {
      outfile = viseven;
    }
    else {
      outfile = visodd;
    }
  }
  // vis output
  else if(viseven) {
    if(even) {
      outfile = viseven;
    }
    else {
      outfile = visodd;
    }
  }
  // uv output
  else {
    extractMiddleSamples = true;

    if(even) {
      outfile = uveven;
    }
    else {
      outfile = uvodd;
    }
  }

  // We know our output file now, so get a linemanager for writing
  LineManager mgr(*outfile);

  // line is framelet * frameletLineOffset + frameletSetOffset
  int outLine = 1;
  int outBand = framelet + 1;

  // if both vis & uv on, outLine is a calculation based on the current line
  //   being uv or vis and the general calculation (above) does not work
  if(viseven && uveven) {
    // uv file
    if(framelet < 2) {
      outLine = frameletSet * 4 + 1 + padding[framelet];
    }
    // vis file
    else {
      outLine = frameletSet * 14 + 1 + padding[framelet];
      outBand -= 2; // uv is not in vis file
    }
  }
  // only vis on
  else if(viseven) {
    outLine = frameletSet * 14 + 1 + padding[framelet];
  }
  // only uv on
  else {
    outLine = frameletSet * 4 + 1 + padding[framelet];
  }

  if(flip) {
    outLine = outfile->Lines() - (outLine - 1);
  }

  outLine += frameletLineOffset;

  mgr.SetLine(outLine, outBand);

  if(!extractMiddleSamples) {
    for(int i = 0; i < in.size(); i++) {
      if(i >= mgr.size()) {
        string msg = "The input file has an unexpected number of samples";
        throw iException::Message(iException::Programmer, msg, _FILEINFO_);
      }

      mgr[i] = lookupTable.Map(in[i]);
    }
  }
  else {
    // read middle of input...
    int startSamp = (in.size() / 2) - mgr.size() / 2;
    int endSamp = (in.size() / 2) + mgr.size() / 2;

    if(mgr.size() > in.size()) {
      string msg = "Output number of samples calculated is invalid";
      throw iException::Message(iException::Programmer, msg, _FILEINFO_);
    }

    for(int inputSamp = startSamp; inputSamp < endSamp; inputSamp++) {
      mgr[inputSamp-startSamp] = lookupTable.Map(in[inputSamp]);
    }
  }

  outfile->Write(mgr);
}

/**
 * This calculates the output labels for each file, which are only valid if the 
 * output cubes have been created (uses ns/nl from each file if 
 * available). Otherwise the label calculated for that particular file is 
 * incomplete and invalid. 
 *  
 * One input file goes to 2 or 4 output file, so let's calculate everything we 
 * can here. 
 * 
 * @param pdsLab Input File Label 
 * @param isis3VisEven Even vis file output label
 * @param isis3VisOdd  Odd vis file output label
 * @param isis3UvEven  Even UV file output label
 * @param isis3UvOdd   Odd UV file output label
 */
void TranslateLabels (Pvl &pdsLab, Pvl &isis3VisEven, Pvl &isis3VisOdd, 
                      Pvl &isis3UvEven, Pvl &isis3UvOdd) {

  // Let's start by running through the generic translations.

  // Get the directory where the translation tables are.
  PvlGroup dataDir (Preference::Preferences().FindGroup("DataDirectory"));
  iString transDir = (string) dataDir["Lro"] + "/translations/";

  // Translate the in
  Filename transFile (transDir + "lrowacInstrument.trn");
  PvlTranslationManager instrumentXlater (pdsLab, transFile.Expanded());
  instrumentXlater.Auto (isis3VisEven);
  instrumentXlater.Auto (isis3VisOdd);
  instrumentXlater.Auto (isis3UvEven);
  instrumentXlater.Auto (isis3UvOdd);

  // Translate the Archive group
  transFile  = transDir + "lrowacArchive.trn";
  PvlTranslationManager archiveXlater (pdsLab, transFile.Expanded());

  archiveXlater.Auto (isis3VisEven);
  archiveXlater.Auto (isis3VisOdd);
  archiveXlater.Auto (isis3UvEven);
  archiveXlater.Auto (isis3UvOdd);

  // Get a couple of user parameters into the instrument groups
  UserInterface &ui = Application::GetUserInterface();

  vector<PvlKeyword> genericInstrument; 
  genericInstrument.push_back(PvlKeyword("DataFlipped", (ui.GetBoolean("FLIP")? "Yes" : "No")));
  genericInstrument.push_back(PvlKeyword("ColorOffset", colorOffset));
  genericInstrument.push_back(PvlKeyword("Decompanded", (ui.GetBoolean("UNLUT")? "Yes" : "No")));

  // We'll need the instrument groups a lot, get a reference right to them
  PvlGroup &visEvenInst = isis3VisEven.FindGroup("Instrument", Pvl::Traverse);
  PvlGroup &visOddInst  = isis3VisOdd.FindGroup("Instrument", Pvl::Traverse);
  PvlGroup &uvEvenInst = isis3UvEven.FindGroup("Instrument", Pvl::Traverse);
  PvlGroup &uvOddInst  = isis3UvOdd.FindGroup("Instrument", Pvl::Traverse);

  // **TEMPORARY. TargetName input keyword is wrong, hardcode it to moon
  visEvenInst.AddKeyword(PvlKeyword("TargetName", "Moon"), Pvl::Replace);
  visOddInst.AddKeyword(PvlKeyword("TargetName", "Moon"), Pvl::Replace);
  uvEvenInst.AddKeyword(PvlKeyword("TargetName", "Moon"), Pvl::Replace);
  uvOddInst.AddKeyword(PvlKeyword("TargetName", "Moon"), Pvl::Replace);

  // Add user parameters to the instrument group
  for(unsigned int key = 0; key < genericInstrument.size(); key++) {
    visEvenInst.AddKeyword(genericInstrument[key]);
    visOddInst.AddKeyword(genericInstrument[key]);
    uvEvenInst.AddKeyword(genericInstrument[key]);
    uvOddInst.AddKeyword(genericInstrument[key]);
  }

  // add labels unique to particular files
  if(viseven) {
    visEvenInst.AddKeyword(PvlKeyword("Framelets", "Even"));
    visEvenInst.AddKeyword(PvlKeyword("NumFramelets", viseven->Lines()/14));
    visEvenInst.AddKeyword(PvlKeyword("InstrumentId", "WAC-VIS"), Pvl::Replace);
    visEvenInst.AddKeyword(PvlKeyword("InstrumentModeId", (std::string)pdsLab["INSTRUMENT_MODE_ID"]));
  }

  if(visodd) {
    visOddInst.AddKeyword(PvlKeyword("Framelets", "Odd"));
    visOddInst.AddKeyword(PvlKeyword("NumFramelets", visodd->Lines()/14));
    visOddInst.AddKeyword(PvlKeyword("InstrumentId", "WAC-VIS"), Pvl::Replace);
    visOddInst.AddKeyword(PvlKeyword("InstrumentModeId", (std::string)pdsLab["INSTRUMENT_MODE_ID"]));
  }

  // **TEMPORARY. This should be done by a translation table.
  // Translate the BandBin group
  PvlGroup visBandBin("BandBin");
  PvlKeyword visFilterName("FilterName");
  PvlKeyword visFilterNum("FilterNumber");

  if(viseven && viseven->Bands() == 1) {
    visFilterName += pdsLab["FILTER_NAME"][0];
    visFilterNum  += pdsLab["FILTER_NUMBER"][0];
  }
  else {
    visFilterName += "415";
    visFilterName += "566";
    visFilterName += "604";
    visFilterName += "643";
    visFilterName += "689";
    visFilterNum += 3;
    visFilterNum += 4;
    visFilterNum += 5;
    visFilterNum += 6;
    visFilterNum += 7;
  }

  visBandBin += visFilterNum;
  visBandBin += visFilterName;

  isis3VisEven += visBandBin;
  isis3VisOdd += visBandBin;

  PvlGroup visKerns("Kernels");
  visKerns += PvlKeyword("NaifIkCode", "-85621");
  isis3VisEven += visKerns;
  isis3VisOdd += visKerns;

  if(uveven) {
    uvEvenInst.AddKeyword(PvlKeyword("Framelets", "Even"));
    uvEvenInst.AddKeyword(PvlKeyword("NumFramelets", uveven->Lines()/4));
    uvEvenInst.AddKeyword(PvlKeyword("InstrumentId", "WAC-UV"), Pvl::Replace);
    uvEvenInst.AddKeyword(PvlKeyword("InstrumentModeId", (std::string)pdsLab["INSTRUMENT_MODE_ID"]));
  }

  if(uvodd) {
    uvOddInst.AddKeyword(PvlKeyword("Framelets", "Odd"));
    uvOddInst.AddKeyword(PvlKeyword("NumFramelets", uvodd->Lines()/4));
    uvOddInst.AddKeyword(PvlKeyword("InstrumentId", "WAC-UV"), Pvl::Replace);
    uvOddInst.AddKeyword(PvlKeyword("InstrumentModeId", (std::string)pdsLab["INSTRUMENT_MODE_ID"]));
  }

  // Translate the BandBin group
  PvlGroup uvBandBin("BandBin");
  PvlKeyword uvFilterName("FilterName");
  PvlKeyword uvFilterNum("FilterNumber");

  uvFilterName += "321";
  uvFilterName += "360";
  uvFilterNum += 1;
  uvFilterNum += 2;

  uvBandBin += uvFilterNum;
  uvBandBin += uvFilterName;

  isis3UvEven += uvBandBin;
  isis3UvOdd  += uvBandBin;

  PvlGroup uvKerns("Kernels");
  uvKerns += PvlKeyword("NaifIkCode", "-85626");

  isis3UvEven += uvKerns;
  isis3UvOdd  += uvKerns;
}

/**
 * This initializes all of the files will NULL DN's.
 * 
 */
void writeNullsToFile() {
  // have output vis files? initialize files with nulls
  if(viseven && visodd) {
    LineManager evenLineMgr(*viseven);
    LineManager oddLineMgr(*visodd);
    evenLineMgr.SetLine(1, 1);
    oddLineMgr.SetLine(1, 1);

    for(int i = 0; i < evenLineMgr.size(); i++) {
      evenLineMgr[i] = Isis::Null;
      oddLineMgr[i]  = Isis::Null;
    }

    while(!evenLineMgr.end()) {
      viseven->Write(evenLineMgr);
      visodd->Write(oddLineMgr);
      evenLineMgr ++;
      oddLineMgr  ++;
    }
  }

  // have output uv files? initialize files with nulls
  if(uveven && uvodd) {
    LineManager evenLineMgr(*uveven);
    LineManager oddLineMgr(*uvodd);
    evenLineMgr.SetLine(1, 1);
    oddLineMgr.SetLine(1, 1);

    for(int i = 0; i < evenLineMgr.size(); i++) {
      evenLineMgr[i] = Isis::Null;
      oddLineMgr[i]  = Isis::Null;
    }

    while(!evenLineMgr.end()) {
      uveven->Write(evenLineMgr);
      uvodd->Write(oddLineMgr);
      evenLineMgr ++;
      oddLineMgr  ++;
    }
  }
}
