#include <iomanip>
#include "iException.h"
#include "Cube.h"
#include "OverlapStatistics.h"
#include "Statistics.h"
#include "IntersectionStatistics.h"
#include "Preference.h"

using namespace std;

int main (int argc, char *argv[]) {
  Isis::Preference::Preferences(true);

  try{
    cout << "UnitTest for Intersection Statistics" << endl;
    Isis::Cube cube1,cube2,cube3;
    cube1.Open("$odyssey/testData/I00824006RDR.lev2.cub");
    cube2.Open("$odyssey/testData/I01523019RDR.lev2.cub");
    cube3.Open("$odyssey/testData/I02609002RDR.lev2.cub");
    Isis::Statistics *stats1 = cube1.Statistics();
    Isis::Statistics *stats2 = cube2.Statistics();
    Isis::Statistics *stats3 = cube3.Statistics();

    vector<Isis::Statistics> statsList;
    statsList.push_back(*stats1);
    statsList.push_back(*stats2);
    statsList.push_back(*stats3); 
    cout << "statsList size: " << statsList.size() << endl;

    vector<string> idList;
    idList.push_back("I00824006RDR.lev2.cub");
    idList.push_back("I01523019RDR.lev2.cub");
    idList.push_back("I02609002RDR.lev2.cub");
    cout << "idList size: " << statsList.size() << endl;

    vector<string> idHoldList;
    idHoldList.push_back("I01523019RDR.lev2.cub");

    Isis::IntersectionStatistics iStat (statsList, idList);
    iStat.SetHoldList(idHoldList);
    cout << "iStat creation == SUCCESS" << endl;
    cout << setprecision(25);

    Isis::OverlapStatistics oStats1(cube1,cube2);
    Isis::OverlapStatistics oStats2(cube1,cube3);
    Isis::OverlapStatistics oStats3(cube2,cube3);

    Isis::MultivariateStatistics mStats1;
    Isis::MultivariateStatistics mStats2;
    Isis::MultivariateStatistics mStats3;
    mStats1 = oStats1.GetMStats(1);
    mStats2 = oStats2.GetMStats(1);
    mStats3 = oStats3.GetMStats(1);

    Isis::Statistics overlap11 = mStats1.X();
    Isis::Statistics overlap12 = mStats1.Y();
    iStat.AddIntersection(overlap11, "I00824006RDR.lev2.cub",
                          overlap12, "I01523019RDR.lev2.cub", overlap11.ValidPixels());
    Isis::Statistics overlap21 = mStats2.X();
    Isis::Statistics overlap22 = mStats2.Y();
    iStat.AddIntersection(overlap21, "I00824006RDR.lev2.cub",
                          overlap22, "I02609002RDR.lev2.cub", overlap21.ValidPixels());
    Isis::Statistics overlap31 = mStats3.X();
    Isis::Statistics overlap32 = mStats3.Y();
    iStat.AddIntersection(overlap31, "I01523019RDR.lev2.cub",
                          overlap32, "I02609002RDR.lev2.cub", overlap31.ValidPixels());

    iStat.Solve();

    cout << "I00824006RDR.lev2.cub : Gathered Offset: " 
      << iStat.GetOffset("I00824006RDR.lev2.cub") << endl;
    cout << "I01523019RDR.lev2.cub : Gathered Offset: " 
      << iStat.GetOffset("I01523019RDR.lev2.cub") << endl;
    cout << "I02609002RDR.lev2.cub : Gathered Offset: " 
      << iStat.GetOffset("I02609002RDR.lev2.cub") << endl;

    cout << "I00824006RDR.lev2.cub : Gathered Gain: " 
      << iStat.GetGain("I00824006RDR.lev2.cub") << endl;
    cout << "I01523019RDR.lev2.cub : Gathered Gain: " 
      << iStat.GetGain("I01523019RDR.lev2.cub") << endl;
    cout << "I02609002RDR.lev2.cub : Gathered Gain: " 
      << iStat.GetGain("I02609002RDR.lev2.cub") << endl;
  }
  catch (Isis::iException &e) {
   e.Report();
 }
}







