#include <iostream>

//#include <QMap>
//#include <QString>

#include "GroupedStatistics.h"
#include "Statistics.h"
#include "Preference.h"


using namespace std;
using namespace Isis;


int main ()
{
  Isis::Preference::Preferences(true);
  cout << "GroupedStatistics unitTest\n";

  // test constructor
  cout << "testing constructor...\n";
  GroupedStatistics * groupStats = new GroupedStatistics();
  
  // test AddStatistic
  cout << "testing AddStatistic...\n";
  groupStats->AddStatistic("Height", 71.5);
  
  // test copy constructor
  cout << "testing copy constructor...\n";
  GroupedStatistics * groupStats2 = new GroupedStatistics(*groupStats);
  
  // test GetStatistics
  cout << "testing GetStatistics...\n";
  Statistics stats = groupStats2->GetStatistics("Height");
  cout << stats.Average() << "\n";

  // test destructor
  delete groupStats;
  delete groupStats2;
  
  return 0;
}
