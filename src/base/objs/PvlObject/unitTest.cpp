#include <iostream>
#include <sstream>
#include "PvlObject.h"
#include "PvlTokenizer.h"
#include "iException.h"
#include "Preference.h"

using namespace std;
int main () {
  Isis::Preference::Preferences(true);

  Isis::PvlObject o("Beasts");
  o += Isis::PvlKeyword("CAT","Meow");
  cout << o << endl;
  cout << endl;

  Isis::PvlGroup g("Fish");
  g += Isis::PvlKeyword("Trout","Brown");
  g += Isis::PvlKeyword("Bass","Large mouth");
  o += g;
  cout << o << endl;
  cout << endl;

  Isis::PvlGroup g2("Birds");
  g2 += Isis::PvlKeyword("Sparrow","House");
  g2 += Isis::PvlKeyword("Crow");
  o += g2;
  cout << o << endl;
  cout << endl;

  Isis::PvlObject o2("Snake");
  o2.AddComment("Are slimey");
  o2 += Isis::PvlKeyword("Rattler","DiamondBack");
  o += o2;
  cout << o << endl;
  cout << endl;

  o.FindObject("Snake").AddGroup(g);
  cout << o << endl;
  cout << endl;

  o.FindObject("Snake") += o2;
  cout << o << endl;
  cout << endl;

  cout << "New for PvlObjectFindKeyword" << endl;
  
  cout << o.HasKeyword("Trout", Isis::PvlObject::Traverse) << endl; 
  cout << o.FindKeyword("Trout", Isis::PvlObject::Traverse) << endl; 
  cout << o.HasKeyword("Crow",Isis::PvlObject::Traverse) << endl; 
  cout << o.FindKeyword("Crow",Isis::PvlObject::Traverse) << endl; 
  cout << o.HasKeyword("Rattler",Isis::PvlObject::Traverse) << endl; 
  cout << o.FindKeyword("Rattler",Isis::PvlObject::Traverse) << endl; 
  cout << o.HasKeyword("Cat",Isis::PvlObject::Traverse) << endl; 
  cout << o.FindKeyword("Cat",Isis::PvlObject::Traverse) << endl; 

  try {
    cout << o.FindKeyword("Trout", Isis::PvlObject::None) << endl; 
  }
  catch (Isis::iException &e) {
    e.Report(false);
  }
  try {
    cout << o.FindKeyword("Bus", Isis::PvlObject::Traverse) << endl; 
  }
  catch (Isis::iException &e) {
    e.Report(false);
  }
  cout << "Keyword Trout should not exist at top level " << o.HasKeyword("Trout", Isis::PvlObject::None) << endl; 
  cout << "Keyword Bus should dnot exit at top level " << o.HasKeyword("Bus", Isis::PvlObject::Traverse) << endl; 

  cout << "End new for PvlObjectFindKeyword" << endl;

  cout << "------------" << endl;
  o.FindObject("Snake").AddObject(o2);
  o.FindObject("Snake").FindObject("Snake") += 
                        Isis::PvlKeyword("Gopher","Constrictor");
  cout << o << endl;
  cout << endl;


  stringstream os;
  os << o;

  Isis::PvlTokenizer tizer;
  tizer.Load(os);
  vector<Isis::PvlToken> toks = tizer.GetTokenList();

  cout << "------------" << endl;
  vector<Isis::PvlToken>::iterator it = toks.begin();
  Isis::PvlObject o3(toks,it);
  cout << o3 << endl;


}
