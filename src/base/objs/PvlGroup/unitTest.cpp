#include <sstream>
#include <iostream>
#include "PvlGroup.h"
#include "PvlTokenizer.h"
#include "Preference.h"

using namespace std;
int main () {
  Isis::Preference::Preferences(true);

  Isis::PvlKeyword dog("DOG",5.2,"meters");
  Isis::PvlKeyword cat("CATTLE");
  cat = "Meow";
  cat.AddComment("Cats shed");

  Isis::PvlGroup ani("Animals");
  ani += dog;
  ani += cat;
  ani.AddComment("/* Pets are cool");

  cout << ani << endl; 

  cout << (double) ani["dog"] << endl;

  ani -= "dog";
  cout << ani << endl;

  ani -= ani[0];
  cout << ani << endl;

  stringstream os;
  os << "# Testing" << endl
     << "/* 123" << endl
     << "Group=POODLE "
     << "CAT=\"TABBY\" "
     << "BIRD=(PARROT) \0"
     << "REPTILE={SNAKE,LIZARD} \t"
     << "-VEGGIE \n"
     << " "
     << "    BOVINE    =   (   COW  ,  CAMEL  ) \n  "
     << "TREE = {   \"MAPLE\"   ,\n \"ELM\" \n, \"PINE\"   }"
     << "FLOWER = \"DAISY & \nTULIP \""
     << "# This is a comment\n"
     << "/* This is another comment\n"
     << "BIG = (\"  NOT  \",\"REALLY LARGE\")"
     << "EndGroup" << endl;

  Isis::PvlTokenizer tizer;
  tizer.Load (os);
  vector<Isis::PvlToken> toks = tizer.GetTokenList();

  class MyGroup : public Isis::PvlGroup {
    public:
      MyGroup (vector<Isis::PvlToken> &toks, vector<Isis::PvlToken>::iterator &it) :
               Isis::PvlGroup(toks,it) {};
  };

  cout << "------------" << endl;
  vector<Isis::PvlToken>::iterator it = toks.begin();
  MyGroup g(toks,it);
  cout << g << endl;

}
