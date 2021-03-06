#include "Isis.h"
#include "ProcessByBrick.h"
#include "Cube.h"
#include <string>

using namespace std;
void twoInAndOut (vector<Isis::Buffer *> &ib, vector<Isis::Buffer *> &ob);

void IsisMain()
{
  Isis::Preference::Preferences(true);
  Isis::ProcessByBrick p;

  Isis::Cube *icube = p.SetInputCube("FROM");
  p.SetInputCube("FROM2");
  p.SetBrickSize(10, 10, 2);
  p.SetOutputCube("TO", icube->Samples(), icube->Lines(), icube->Bands());
  p.SetOutputCube("TO2", icube->Samples(), icube->Lines(), icube->Bands());
  p.StartProcess(twoInAndOut);
  p.EndProcess();

  Isis::Cube cube;
  cube.Open("/tmp/isisProcessByBrick_01");
  cube.Close(true);
  cube.Open("/tmp/isisProcessByBrick_02");
  cube.Close(true);
}

void twoInAndOut (vector<Isis::Buffer *> &ib, vector<Isis::Buffer *> &ob)
{
  static bool firstTime = true;
  if (firstTime) {
    firstTime = false;
    cout << "Testing two input and output cubes ... " << endl;
    cout << "Number of input cubes:   " << ib.size() << endl;
    cout << "Number of output cubes:  " << ob.size() << endl;
    cout << endl;
  }

  Isis::Buffer &inone = *ib[0];
  Isis::Buffer &intwo = *ib[1];
  Isis::Buffer &outone = *ob[0];
  Isis::Buffer &outtwo = *ob[1];
  
  cout << "Sample:  " << inone.Sample() << ":" << intwo.Sample() 
       << "  Line:  " << inone.Line() << ":" << intwo.Line()
       << "  Band:  " << inone.Band() << ":" << intwo.Band() << endl;

  if ((inone.Sample() != intwo.Sample()) || 
      (inone.Line() != intwo.Line())) {
    cout << "Bogus error #1" << endl;
  }

  if ((inone.Sample() != outone.Sample()) || 
      (inone.Line() != outone.Line()) ||
      (inone.Band() != outone.Band())) {
    cout << "Bogus error #2" << endl;
  }
  
  if ((outone.Sample() != outtwo.Sample()) || 
      (outone.Line() != outtwo.Line()) ||
      (outone.Band() != outtwo.Band())) {
    cout << "Bogus error #3" << endl;
  }
}
