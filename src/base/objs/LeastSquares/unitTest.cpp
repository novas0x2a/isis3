#include <iostream>
#include "iException.h"
#include "LeastSquares.h"
#include "Preference.h"

int main () {
  Isis::Preference::Preferences(true);
  try {
  std::cout << "Unit Test for LeastSquares:" << std::endl;
  Isis::BasisFunction b("Linear",2,2);

  Isis::LeastSquares lsq(b);

  std::vector<double> one;
  one.push_back(1.0);
  one.push_back(1.0);

  std::vector<double> two;
  two.push_back(-2.0);
  two.push_back(3.0);

  std::vector<double> tre;
  tre.push_back(2.0);
  tre.push_back(-1.0);

  lsq.AddKnown(one,3.0);
  lsq.AddKnown(two,1.0);
  lsq.AddKnown(tre,2.0);

  lsq.Solve();

  std::cout << "Number of Knowns = " << lsq.Knowns() << std::endl;
  std::cout << "one = " << lsq.Evaluate(one) << std::endl;
  std::cout << "  residual = " << lsq.Residual(0) << std::endl;
  std::cout << "two = " << lsq.Evaluate(two) << std::endl;
  std::cout << "  residual = " << lsq.Residual(1) << std::endl;
  std::cout << "three = " << lsq.Evaluate(tre) << std::endl;
  std::cout << "  residual = " << lsq.Residual(2) << std::endl;
  std::cout << "---" << std::endl;
  std::cout << "Test from Linear Algebra with Applications, 2nd Edition" << std::endl;
  std::cout << "Steven J. Leon, page 191, 83/50=1.66 71/50=1.42" << std::endl;
  std::cout << b.Coefficient(0) << std::endl;
  std::cout << b.Coefficient(1) << std::endl ; 
  std::cout << "---" << std::endl;

  lsq.Weight(1,5);
  lsq.Solve();
  std::cout << "Solution with unknown two weighted" << std::endl;
  std::cout << "one = " << lsq.Evaluate(one) << std::endl;
  std::cout << "two = " << lsq.Evaluate(two) << std::endl;
  std::cout << "three = " << lsq.Evaluate(tre) << std::endl;
  }
  catch (Isis::iException &e) {
    e.Report();
  }

  return 0;
}
