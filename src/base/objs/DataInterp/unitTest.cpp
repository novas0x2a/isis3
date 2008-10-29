#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include "DataInterp.h"
#include "Preference.h"

using namespace std;
using namespace Isis;

inline double xFunc(double ith) { return (ith + 0.5 * sin (ith)); }
inline double yFunc(double ith) { return (ith + cos (ith * ith)); }


int main(int argc, char *argv[]) {
  Isis::Preference::Preferences(true);

  try {

    DataInterp spline(DataInterp::Cubic);

    for (int i = 0 ; i < 6 ; i++) {
      cout << "Spline Name:    " 
           << spline.name(DataInterp::InterpType(i)) << endl;
      cout << "Minimum Points: " 
           << spline.minPoints(DataInterp::InterpType(i)) << endl;
    }

    cout << "Using Spline Named: " << spline.name() << endl;
    cout << "Minimum Points:     " << spline.minPoints() << endl;

//  This should throw an error since we have not added any points
    try {
      spline.Evaluate(1.0);
    } catch (iException &ie) {
      ie.Report(false);
    }

    cout.setf(std::ios::fixed);
    vector<double> x, y;
    x.push_back(xFunc(-1.0));
    y.push_back(yFunc(-1.0));
    unsigned int npts(200);
    for (unsigned int i = 0; i < npts; i++) {
        x.push_back(xFunc(i));
        y.push_back(yFunc(i));
        spline.addPoint(x.back(), y.back());
        cout << "(x,y) = " 
             << setprecision(4) << x.back() << ", " 
             << setprecision(4) << y.back() << endl;
      }

    spline.Compute();
    double dmin(spline.domainMinimum());
    double dmax(spline.domainMaximum());
    cout << "Domain Minimum: " << setprecision(4) << dmin << endl;
    cout << "Domain Maximum: " << setprecision(4) << dmax << endl;

    unsigned int npts2(50);
    double dinc(((dmax - dmin)) / (double) (npts2));
    for (double d(dmin) ; d <= dmax ; d += dinc) {
      cout << "Spline(x,y) = " 
           << setprecision(4) << d << ", " 
           << setprecision(4) << spline.Evaluate(d) 
           << ", 1stD: " << setprecision(4) << spline.firstDerivative(d) 
           << ", 2ndD: " << setprecision(4) << spline.secondDerivative(d) 
           << endl;
  }
  } catch (iException &ie) {
    ie.Report(false);
  }

  return (0);
}
