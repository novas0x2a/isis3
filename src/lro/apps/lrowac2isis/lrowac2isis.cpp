#include "Isis.h"

#include <cstdio>
#include <string>

#include "ProcessImportPds.h"
#include "LineManager.h"
#include "OriginalLabel.h"
#include "History.h"

using namespace std; 
using namespace Isis;

vector<Cube *> outputCubes;
int frameletLines;
void separateFrames(Buffer &in);

void TranslateLabels (Pvl &labelFile, Pvl &isis3);

void IsisMain () {
  outputCubes.clear();
  ProcessImportPds p;
  Pvl pdsLab;

  UserInterface &ui = Application::GetUserInterface();
  string fromFile = ui.GetFilename("FROM");

  p.SetPdsFile (fromFile, "", pdsLab);

  Pvl isis3Lab;
  TranslateLabels(pdsLab, isis3Lab);

  Cube *even = new Cube();
  Cube *odd = new Cube();

  even->SetDimensions(p.Samples(), p.Lines(), p.Bands());
  even->SetPixelType(Isis::Real);
  odd->SetDimensions(p.Samples(), p.Lines(), p.Bands());
  odd->SetPixelType(Isis::Real);

  string evenFile = ui.GetFilename("EVEN");
  string oddFile = ui.GetFilename("ODD");

  even->Create(evenFile);
  odd->Create(oddFile);

  frameletLines = 16;
  outputCubes.push_back(odd);
  outputCubes.push_back(even);

  p.StartProcess (separateFrames);
  p.EndProcess();

  // Add original labels
  OriginalLabel origLabel(pdsLab);


  for(int i = 0; i < (int)outputCubes.size(); i++) {
    int numFramelets = p.Lines() / frameletLines;
    isis3Lab.FindGroup("Instrument").AddKeyword(
        PvlKeyword("NumFramelets", numFramelets), Pvl::Replace
        );

    string frameletType = ((i == 0)? "Odd" : "Even");
    isis3Lab.FindGroup("Instrument").AddKeyword(
        PvlKeyword("Framelets", frameletType), Pvl::Replace
        );

    for(int grp = 0; grp < isis3Lab.Groups(); grp++) {
      outputCubes[i]->PutGroup(isis3Lab.Group(grp));
    }

    History history("IsisCube");
    history.AddEntry();

    outputCubes[i]->Write(history);
    outputCubes[i]->Write(origLabel);

    outputCubes[i]->Close();
    delete outputCubes[i];
  }

  outputCubes.clear();
}

//! Separates each of the individual WAC frames into their own file
void separateFrames(Buffer &in) {
  // (line-1)/frameletHeight % numOutImages
  int outputCube = (in.Line()-1)/frameletLines % outputCubes.size();
  LineManager mgr(*outputCubes[outputCube]);
  mgr.SetLine(in.Line(), in.Band());
  mgr.Copy(in);
  outputCubes[outputCube]->Write(mgr);

  // Null out every other cube
  for(int i = 0; i < (int)outputCubes.size(); i++) {
    if(i == outputCube) continue;

    LineManager mgr(*outputCubes[i]);
    mgr.SetLine(in.Line(), in.Band());

    for(int j = 0; j < mgr.size(); j++) {
      mgr[j] = Isis::Null;
    }

    outputCubes[i]->Write(mgr);
  }
}

void TranslateLabels(Pvl &pdsLab, Pvl &isis3) {
  // Get the directory where the MRO HiRISE translation tables are.
  PvlGroup dataDir (Preference::Preferences().FindGroup("DataDirectory"));
  iString transDir = (string) dataDir["Lro"] + "/translations/";

  // Translate the Instrument group
  Filename transFile (transDir + "lrowacInstrument.trn");
  PvlTranslationManager instrumentXlater (pdsLab, transFile.Expanded());
  instrumentXlater.Auto (isis3);

  // Translate the BandBin group
  transFile  = transDir + "lrowacBandBin.trn";
  PvlTranslationManager bandBinXlater (pdsLab, transFile.Expanded());
  bandBinXlater.Auto (isis3);

  // Translate the Archive group
  transFile  = transDir + "lrowacArchive.trn";
  PvlTranslationManager archiveXlater (pdsLab, transFile.Expanded());
  archiveXlater.Auto (isis3);

  // Create the Kernel Group
  PvlGroup kerns("Kernels");
  kerns += PvlKeyword("NaifIkCode", "-85620");
  isis3 += kerns;
}
