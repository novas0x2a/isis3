#include <cmath>
#include "Pvl.h"
#include "NumericalMethods.h"
#include "iException.h"
#include "Filename.h"

#define MAX(x,y) (((x) > (y)) ? (x) : (y))

namespace Isis {
  /**
   * This routine performs polynomial interpolation or extrapolation.
   * Given arrays xa[0..n-1] and ya[0..n-1] and given a value x, this
   * routine returns a value y and an error estimate dy.
   * 
   * @param xa  Array of independent polynomial variables
   * @param ya  Array of dependent polynomial variables
   * @param n  Number of array values in xa and ya
   * @param x  Independent variable for which the polynomial will be
   *           evaluated
   * @param y  Value returned from interpolation or extrapolation
   * @param dy  Estimate of error in polynomial interpolation or 
   *            extrapolation
   * 
   */
  void NumericalMethods::r8polint (double *xa, double *ya, int n,
                             double x, double *y, double *dy) 
  {
    int ns;
    double den,dif,dift,c[10],d[10],ho,hp,w;

    ns = 1;
    dif = fabs(x - xa[0]);
    for (int i=0; i<n; i++) {
      dift = fabs(x - xa[i]);
      if (dift < dif) {
        ns = i + 1;
	dif = dift;
      }
      c[i] = ya[i];
      d[i] = ya[i];
    }
    ns = ns - 1;
    *y = ya[ns];
    for (int m=1; m<n; m++) {
      for (int i=1; i<=n-m; i++) {
        ho = xa[i-1] - x;
	hp = xa[i+m-1] - x;
	w = c[i] - d[i-1];
	den = ho - hp;
	if (den == 0.0) {
          std::string msg = "Not able to obtain integral of ";
          msg += "function"; 
          throw iException::Message(iException::Math,msg,_FILEINFO_);
        }
	den = w / den;
	d[i-1] = hp * den;
	c[i-1] = ho * den;
      }
      if (2*ns < n-m) {
        *dy = c[ns];
      } else {
	ns = ns - 1;
        *dy = d[ns];
      }
      *y = *y + *dy;
    }
  }

  /**
   * This routine performs cubic spline interpolation.
   * 
   * @param x  Array of independent variables of function
   * @param y  Array of dependent variables of function
   * @param n  Number of array values in x and y
   * @param yp1  First derivative of the function at point 0 
   * @param ypn  First derivative of the function at point n-1
   * @param y2  Array containing the second derivatives of the 
   *            function at the points in the x array
   * 
   */
  void NumericalMethods::r8spline (double *x, double *y, int n,
                             double yp1, double ypn, double *y2) 
  {
    double u[500];
    double p,sig,qn,un;

    if (yp1 > 0.99e30) {
      y2[0] = 0.0;
      u[0] = 0.0;
    } else {
      y2[0] = -0.5;
      u[0] = (3.0 / (x[1] - x[0])) * ((y[1] - y[0]) / (x[1] - x[0]) - yp1);
    }
    for (int i=1; i<n-1; i++) {
      sig = (x[i] - x[i-1]) / (x[i+1] - x[i-1]);
      p = sig * y2[i-1] + 2.0;
      y2[i] = (sig - 1.0) / p;
      u[i] = (6.0 * ((y[i+1] - y[i]) / (x[i+1] - x[i]) - (y[i] - y[i-1]) / 
             (x[i] - x[i-1])) / (x[i+1] - x[i-1]) - sig * u[i-1]) / p;
    }
    if (ypn > 0.99e30) {
      qn = 0.0;
      un = 0.0;
    } else {
      qn = 0.5;
      un = (3.0 / (x[n-1] - x[n-2])) * (ypn - (y[n-1] - y[n-2]) / 
           (x[n-1] - x[n-2]));
    }
    y2[n-1] = (un - qn * u[n-2]) / (qn * y2[n-2] + 1.0);
    for (int i=n-2; i>=0; i--) {
      y2[i] = y2[i] * y2[i+1] + u[i];
    }
  }

  /**
   * This routine performs cubic spline interpolation.
   * Given arrays xa[0..n-1] and ya[0..n-1] which tabulate
   * a function (with the xa[i]'s in order), and given the
   * array y2a[0..n-1] which is the output from the r8spline
   * function, and given a value x, this routine returns
   * a cubic-spline interpolated value y.
   * 
   * @param xa  Array of independent variables of function
   * @param ya  Array of dependent variables of function
   * @param y2a  Array containing the second derivatives of the 
   *            function at the points in the x array (this is
   *            created by a previous call to the r8spline
   *            function)
   * @param n  Number of array values in x and y
   * @param x  Independent variable for which the polynomial will be
   *           evaluated
   * @param y  Value returned from interpolation or extrapolation
   * 
   */
  void NumericalMethods::r8splint (double *xa, double *ya, 
                             double *y2a, int n, double x,
                             double *y) 
  {
    int k;
    int klo;
    int khi;
    double h;
    double a;
    double b;

    klo = 0;
    khi = n - 1;
    while (khi-klo > 1) {
      k = (khi + klo) / 2;
      if (xa[k] > x) {
        khi = k;
      } else {
        klo = k;
      }
    }

    h = xa[khi] - xa[klo];
    if (h == 0.0) {
      std::string msg = "Cubic spline cannot be done because ";
      msg += "endpoints of the input function are the same";
      throw iException::Message(iException::Programmer,msg,
          _FILEINFO_);
    }
    a = (xa[khi] - x) / h;
    b = (x - xa[klo]) / h;
    *y = a * ya[klo] + b * ya[khi] + ((pow(a,3.0) - a) *
        y2a[klo] + (pow(b,3.0) - b) * y2a[khi]) *
	pow(h,2.0) / 6.0;
  }

  /**
   * This routine evaluates the exponential integral En(x).
   * 
   * @param n  The exponential integral En(x) will be evaluated
   * @param x  The exponential integral En(x) will be evaluated
   * 
   */
  double NumericalMethods::r8expint (int n, double x)
  {
    int nm1;
    double result;
    double a,b,c,d,h;
    double del;
    double fact;
    double psi;
    double fpmin;
    double maxit;
    double eps;
    double euler;

    fpmin = 1.0e-30;
    maxit = 100;
    eps = 1.0e-7;
    euler = 0.5772156649;

    nm1 = n - 1;
    if (n < 0 || x < 0.0 || (x == 0.0 && (n == 0 || n == 1))) {
      std::string msg = "Invalid arguments input to r8expint ";
      msg += "function";
      throw iException::Message(iException::Programmer,msg,
          _FILEINFO_);
    } else if (n == 0) {
      result = exp(-x) / x;
    } else if (x == 0.0) {
      result = 1.0 / nm1;
    } else if (x > 1.0) {
      b = x + n;
      c = 1.0 / fpmin;
      d = 1.0 / b;
      h = d;
      for (int i=1; i<=maxit; i++) {
        a = -i * (nm1 + i);
	b = b + 2.0;
	d = 1.0 / (a * d + b);
	c = b + a / c;
	del = c * d;
	h = h * del;
	if (fabs(del-1.0) < eps) {
	  result = h * exp(-x);
	  return(result);
	}
      }
      std::string msg = "Unable to evaluate exponential integral ";
      msg += "in r8expint";
      throw iException::Message(iException::Math,msg,_FILEINFO_);
    } else {
      if (nm1 != 0) {
        result = 1.0 / nm1;
      } else {
        result = -log(x) - euler;
      }
      fact = 1.0;
      for (int i=1; i<=maxit; i++) {
        fact = -fact * x / i;
	if (i != nm1) {
	  del = -fact / (i - nm1);
	} else {
	  psi = -euler;
	  for (int ii=1; ii<=nm1; ii++) {
	    psi = psi + 1.0 / ii;
	  }
	  del = fact * (-log(x) + psi);
	}
	result = result + del;
	if (fabs(del) < fabs(result)*eps) {
	  return(result);
        }
      }
      std::string msg = "Unable to evaluate exponential integral ";
      msg += "in r8expint";
      throw iException::Message(iException::Math,msg,_FILEINFO_);
      return(0.0);
    }
    return(result);
  }

  /**
   * This routine computes the exponential integral Ei(x)
   * for x > 0.
   * 
   * @param x  The exponential integral Ei(x) will be computed
   * 
   */
  double NumericalMethods::r8ei (double x)
  {
    double result;
    double fpmin;
    double euler;
    double eps;
    int maxit;
    double sum,fact,term,prev;

    fpmin = 1.0e-30;
    maxit = 100;
    eps = 6.0e-8;
    euler = 0.57721566;

    if (x <= 0.0) {
      std::string msg = "Invalid argument to r8ei";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }
    if (x < fpmin) {
      result = log(x) + euler;
    } else if (x <= -log(eps)) {
      sum = 0.0;
      fact = 1.0;
      for (int k=1; k<=maxit; k++) {
        fact = fact * x / k;
	term = fact / k;
	sum = sum + term;
        if (term < eps*sum) {
	  result = sum + log(x) + euler;
	  return(result);
	}
      }
      std::string msg = "Unable to calculate exponential ";
      msg += "integral in r8ei";
      throw iException::Message(iException::Math,msg,_FILEINFO_);
    } else {
      sum = 0.0;
      term = 1.0;
      for (int k=1; k<=maxit; k++) {
        prev = term;
	term = term * k / x;
	if (term < eps) {
	  result = exp(x) * (1.0 + sum) / x;
	  return(result);
	} else {
	  if (term < prev) {
	    sum = sum + term;
          } else {
	    sum = sum - prev;
	    result = exp(x) * (1.0 + sum) / x;
	    return(result);
	  }
	}
      }
      result = exp(x) * (1.0 + sum) / x;
    }
    return(result);
  }

  /**
   * Perform Chandra and Van de Hulst's series approximation
   * for the g'11 function needed in second order scattering
   * theory
   *
   * @param tau  normal optical depth of atmosphere
   *
   */
  double NumericalMethods::G11Prime(double tau) {
    double sum;
    int icnt;
    double fac;
    double term;
    double tol;
    double fi;
    double elog;
    double eulgam;
    double e1_2;
    double result;

    tol = 1.0e-6;
    eulgam = 0.5772156;

    sum = 0.0;
    icnt = 1;
    fac = -tau;
    term = fac;
    while (fabs(term) > fabs(sum)*tol) {
      sum = sum + term;
      icnt = icnt + 1;
      fi = (double) icnt;
      fac = fac * (-tau) / fi;
      term = fac / (fi * fi);
    }
    elog = log(MAX(1.0e-30,tau)) + eulgam;
    e1_2 = sum + Isis::PI * Isis::PI / 12.0 + 0.5 *
        pow(elog,2.0);
    result = 2.0 * (r8expint(1,tau) + elog * r8expint(2,tau) -
        tau * e1_2);
    return(result);
  }

  /**
   * Obtain arccosine of input value. If the input value is outside
   * of the valid range (-1 to 1), then obtain the arccosine of the
   * closest valid value.
   *
   * @param cosang  input value to obtain arccosine of (in radians)
   *
   */
  double NumericalMethods::PhtAcos(double cosang)
  {
    double result;

    if (fabs(cosang) <= 1.0) {
      result = acos(cosang);
    }
    else {
      if (cosang < -1.0) {
        result = acos(-1.0);
      }
      else {
        result = acos(1.0);
      }
    }

    return result;
  }
}
