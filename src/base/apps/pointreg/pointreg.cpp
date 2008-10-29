#include "Isis.h"
#include "UserInterface.h"
#include "Cube.h"
#include "Chip.h"
#include "Progress.h"
#include "iException.h"
#include "AutoReg.h"
#include "AutoRegFactory.h"
#include "ControlNet.h"
#include "ControlMeasure.h"
#include "SerialNumberList.h"
#include "iTime.h"

using namespace std;
using namespace Isis;

void IsisMain() {
  // Get user interface
  UserInterface &ui = Application::GetUserInterface();

  // Open the files list in a SerialNumberList for
  // reference by SerialNumber
  SerialNumberList files(ui.GetFilename("FILES"));

  // Create a ControlNet from the input file
  ControlNet cn(ui.GetFilename("CNET"));

  // Create an AutoReg from the template file
  Pvl pvl(ui.GetFilename("TEMPLATE"));
  AutoReg *ar = AutoRegFactory::Create(pvl);

  // Create the output ControlNet
  ControlNet output;
  output.SetType(cn.Type());
  output.SetUserName(Application::UserName());
  output.SetDescription(cn.Description());
  output.SetCreatedDate(iTime::CurrentLocalTime());
  output.SetTarget(cn.Target());
  output.SetNetworkId(cn.NetworkId());

  Progress progress;
  progress.SetMaximumSteps(cn.Size());
  progress.CheckStatus();

  int ignored=0, unmeasured=0, registered=0, unregistered=0, validated=0;

  // Register the points and create a new
  // ControlNet containing the refined measurements
  for (int i=0; i<cn.Size(); i++) {
    ControlPoint &cp = cn[i];
    // Check to see if the control point should be registered
    // if not, leave it alone and add it to the network
    if (cp.Ignore()) {
      ignored++;
      if (ui.GetBoolean("IGNORED")) output.Add(cp);
      continue;
    }

    ControlPoint point;
    point.SetType(cp.Type());
    point.SetId(cp.Id());
    point.SetUniversalGround(cp.UniversalLatitude(), cp.UniversalLongitude(), cp.Radius());
    point.SetHeld(cp.Held());

    ControlMeasure &patternCM = cp[cp.ReferenceIndex()];
    Cube patternCube;
    patternCube.Open(files.Filename(patternCM.CubeSerialNumber()));

    ar->PatternChip()->TackCube(patternCM.Sample(), patternCM.Line());
    ar->PatternChip()->Load(patternCube);

    if (patternCM.IsValidated()) validated++;
    if (!patternCM.IsMeasured()) continue;
    patternCM.SetReference(true);
    point.Add(patternCM);

    // Register all the unvalidated measurements
    for (int j=0; j<cp.Size(); j++) {
      // don't register the reference
      if (j == cp.ReferenceIndex()) continue;
      // if the measurement is valid, keep it as is
      if (cp[j].IsValidated()) {
        validated++;
        point.Add(cp[j]);
        continue;
      }
      // if the point is unmeasured,
      if (!cp[j].IsMeasured()) {
        unmeasured++;
        if (ui.GetBoolean("UNMEASURED")) point.Add(cp[j]);
        continue;
      }

      ControlMeasure &searchCM = cp[j];

      Cube searchCube;
      searchCube.Open(files.Filename(searchCM.CubeSerialNumber()));

      ar->SearchChip()->TackCube(searchCM.Sample(), searchCM.Line());

      try {
        ar->SearchChip()->Load(searchCube,*(ar->PatternChip()));

        // If the measurements were correctly registered
        // Write them to the new ControlNet
        AutoReg::RegisterStatus res = ar->Register();

        double score1, score2;
        ar->ZScores(score1, score2);
        searchCM.SetZScores(score1, score2);

        if(res == AutoReg::Success) {
          registered++;
          searchCM.SetType(ControlMeasure::Automatic);
          searchCM.SetError(searchCM.Sample() - ar->CubeSample(), searchCM.Line() - ar->CubeLine());
          searchCM.SetCoordinate(ar->CubeSample(),ar->CubeLine());
          searchCM.SetGoodnessOfFit(ar->GoodnessOfFit());
          point.Add(searchCM);
        }
        // Else use the original marked as "Estimated"
        else {
          unregistered++;
          searchCM.SetType(ControlMeasure::Estimated);

          if(res == AutoReg::FitChipToleranceNotMet) {
            searchCM.SetError(cp[j].Sample() - ar->CubeSample(), cp[j].Line() - ar->CubeLine());
            searchCM.SetGoodnessOfFit(ar->GoodnessOfFit());
          }

          point.SetIgnore(true);
          point.Add(searchCM);
        }
      } catch (iException &e) {
        e.Clear();
        unregistered++;
        searchCM.SetType(ControlMeasure::Estimated);
        point.SetIgnore(true);
        point.Add(searchCM);
      }

      searchCube.Close();
    }

    output.Add(point);

    patternCube.Close();
    progress.CheckStatus();
  }

  PvlGroup pLog("Points");
  pLog+=PvlKeyword("Ignored", ignored);
  Application::Log(pLog);

  PvlGroup mLog("Measures");
  mLog+=PvlKeyword("Validated", validated);
  mLog+=PvlKeyword("Registered", registered);
  mLog+=PvlKeyword("Unregistered", unregistered);
  mLog+=PvlKeyword("Unmeasured", unmeasured);
  Application::Log(mLog);

  output.Write(ui.GetFilename("TO"));

  delete ar;
}
