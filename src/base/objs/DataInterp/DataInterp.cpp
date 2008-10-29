/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2008/06/18 22:54:35 $                                                                 
 *                                                                        
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for 
 *   intellectual property information,user agreements, and related information.
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software 
 *   and related material nor shall the fact of distribution constitute any such 
 *   warranty, and no responsibility is assumed by the USGS in connection 
 *   therewith.                                                           
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see 
 *   the Privacy &amp; Disclaimers page on the Isis website,              
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

 
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>

using std::string;
using std::vector;
using std::make_pair;
using std::ostringstream;

#include "SpecialPixel.h"
#include "DataInterp.h"

namespace Isis {
/**
 * @brief Default data interpolator function constructor
 *
 * Sets the type to Cubic Spline by default
 */
DataInterp::DataInterp() {
  initInterp();
}

/**
 * @brief Constructor with interpolator type
 *
 * Sets the type to parameter.  Note that the type is validated and an exception
 * will be thrown if it is deemed unsupported or invalid. 
 *  
 * @param itype The type to be set
 */
DataInterp::DataInterp(const Isis::DataInterp::InterpType &itype) {
  initInterp();
//  Validates the parameter
  findFunctor(itype);
  _itype = itype;
}

/**
 * @brief Constructor with data and interpolator type
 * 
 * Provide data and interpolator type in constructor.  Note that in this case,
 * the interpolation is automatically computed, unlike the other constructors
 * (since there is no data provided).
 * 
 * Also, if the supplied type is no available or invalid, an exception will be
 * thrown.
 * 
 * @param n     Number of points to use in interpolation
 * @param x     X values for data
 * @param y     Y values at each X data point
 * @param itype Data interpolation function to use  (Default: Cubic)
 */
DataInterp::DataInterp(unsigned int n, double *x, double *y,
                       const Isis::DataInterp::InterpType itype) {
  initInterp();
  _itype = itype;
  for (unsigned int i = 0 ; i < n ; i++) {
    _x.push_back(x[i]);
    _y.push_back(y[i]);
  }
  Compute();
}

/**
 * @brief Copy constructor
 * 
 * Copying of class objects must be handled explicitly in order for it to be
 * completed properly.  The data and interpolator type is copied from the
 * supplied object.  It will be computed if and only if the copied object is
 * computed and it has the minimum number of points for the particular
 * interpolator function.
 * 
 * @param dint DataInterp object to gleen data from for creation of this object
 */
DataInterp::DataInterp(const DataInterp &dint) {
  initInterp();
  _itype = dint._itype;
  _x = dint._x;
  _y = dint._y;
  if ((dint.isComputed()) && (size() >= (unsigned int) minPoints())) Compute();
}

/**
 * @brief DataInterp assigmment operator
 * 
 * Assigning one object to the other requires a deallocation of the current
 * state of this object and reinitialization of data from the right hand object.
 * This may be a bit costly if the number of values is large.
 * 
 * @param dint DataInterp object to copy data from for initialization
 */
DataInterp::DataInterp &DataInterp::operator=(const DataInterp &dint) {
  if (&dint != this) {
    deallocate();
    _itype = dint._itype;
    _x = dint._x;
    _y = dint._y;
    if ((dint.isComputed()) && (size() >= (unsigned int) minPoints())) Compute();
  }
  return (*this);
}

/**
 * @brief Destructor
 *
 * Explicit operations are required to free resources of the interpolation
 */
DataInterp::~DataInterp() {
  deallocate();
}

/**
 * @brief Get name of interpolating function
 * 
 * This method returns the name of the interpolating function currently active.
 * This many be called without adding any points and reflects the name of the
 * chosen one at instantiation.
 * 
 * @return std::string name Name of the interpolation function used/to be used.
 */
std::string DataInterp::name() const {
  return (string(findFunctor(_itype)->name));
}

/**
 * @brief Get name of interpolating function
 * 
 * This method returns the name of the interpolating function for a specific
 * type of interpolating function.  This many be called without adding any
 * points and reflects the name of the chosen one at instantiation.
 * 
 * @param itype Specify the type of interpolation to retrieve the name of
 * @return std::string name Name of the interpolation function used/to be used.
 */
std::string DataInterp::name(const Isis::DataInterp::InterpType itype) const {
  return (string(findFunctor(itype)->name));
}

/**
 * @brief Minimum number of points required by interpolating function
 * 
 * This method returns the minimum number of points that are required by the
 * interpolating function in order for it to be applied to a data set.  It
 * returns the number of the of the currently selected/active interpolation
 * function.
 * 
 * @return int Minimum number of data points required for the interpolation
 *         function
 */
int DataInterp::minPoints() const {
  return (findFunctor(_itype)->min_size);
}

/**
 * @brief Minimum number of points required by interpolating function
 * 
 * This method returns the minimum number of points that are required by the
 * interpolating function in order for it to be applied to a data set.  It
 * returns the minimum number for the specifed interpolation function as
 * specified by the caller.
 * 
 * @param  itype Type of interpolation function to return minimum points for.
 * @return int Minimum number of data points required for the interpolation
 *         function
 */
int DataInterp::minPoints(const Isis::DataInterp::InterpType itype) const {
  return (findFunctor(itype)->min_size);
}

/**
 * @brief Input data domain minimum value
 * 
 * This method returns the minimum X (domain) value provided by the caller used
 * to compute the interpolation interval.  It is only valid if the Compute()
 * method has been invoked.
 * 
 * @return double Minimum X domain value if computed, otherwise returns Null
 * @see isComputed()
 */
double DataInterp::domainMinimum() const {
  if (!isComputed()) return (NULL8);
  return (_interp->interp->xmin);
}

/**
 * @brief Input data domain maximum value
 * 
 * This method returns the maximum X (domain) value provided by the caller used
 * to compute the interpolation interval.  It is only valid if the Compute()
 * method has been invoked.
 * 
 * @return double Maximum X domain value if computed, otherwise returns Null
 * @see isComputed()
 */
double DataInterp::domainMaximum() const {
  if (!isComputed()) return (NULL8);
  return (_interp->interp->xmax);
}

/**
 * @brief Add a datapoint to the set
 * 
 * This method allows the user to add a point to the set of data in preparation
 * for interpolation over an interval.  The caller must call the Compute()
 * method after all the desired points are added to the data set.
 * 
 * Note that points can be added after an interpolation is computed, but they
 * have not effect until the Compute() method is called.  Behavior is undefined
 * if the domain is not sorted in ascending or descending order.
 * 
 * @param x X (domain) value to add
 * @param y Y (range) value to add
 */
void DataInterp::addPoint(const double x, const double y) {
  _x.push_back(x);
  _y.push_back(y);
  return;
}

/**
 *@brief Computes/solves the interpolation over an interval of supplied points
 *
 * This method must be called when all the data points have been added to the
 * object for the data set.  It will compute the interval of interpolated Y
 * (range) values over the given domain.
 *
 * @throws iException When the minimum number of points is not satisfied for the
 *                    interpolating function selected.
 * @see Compute(itype)
 */
void DataInterp::Compute() throw (iException &) {
  if (size() <= (unsigned int) minPoints()) {
    ostringstream msg;
    msg << name() << " interpolation requires at least " << minPoints() 
        << " points - currently have " << size() << "." << std::endl;
    throw iException::Message(iException::Programmer,msg.str(),_FILEINFO_);
  }
  allocate(size());
  integrityCheck(gsl_spline_init(_interp, &_x[0], &_y[0], size()), _FILEINFO_);
  return;
}

/**
 *@brief Computes/solves the interpolation over an interval of supplied points
 *
 * This method must be called when all the data points have been added to the
 * object for the data set.  It will compute the interval of interpolated Y
 * (range) values over the given domain.
 *
 * It differs from the method with the same name and no arguments so as the user
 * can select a different interpolation function with the same data points.
 * This is achieved by deallocating any previous interpolation and it
 * regenerates the new interpolation using the existing data points.
 * 
 * @param itype The type to be set
 *
 * @throws iException When the minimum number of points is not satisfied for the
 *                    interpolating function selected.
 * @see Compute(itype)
 */
void DataInterp::Compute(Isis::DataInterp::InterpType itype) throw (iException &) {
  _itype = itype;
  Compute();
  return;
}

/**
 *@brief Computes interpolated value for given domain value
 *
 * This method returns the Y (range) interpolated value given a valid X (domain)
 * value.  If the given x value exceeds the domain range, the last valid range
 * of the minumum or maximum domain value, whichever end of the domain the value
 * x is outside of, will be returned.
 * 
 * @param x The valid domain value
 *
 * @return double Interpolated Y (range) value at the given X (domain) value
 * @throws iException When the the interpolation solution has not been computed
 *                    using Compute().
 */
double DataInterp::Evaluate(const double x) const throw (iException &) {
  if (!isComputed()) reportError("Evaluate",
                               "Interpolation not computed after points added",
                               _FILEINFO_);
//  Go ahead and get the point
  double value;
  integrityCheck(gsl_spline_eval_e(_interp, x, _acc, &value), _FILEINFO_);
  return (value);
}

/**
 *@brief Computes first derivative value for given domain value
 *
 * This method returns the first derivative value given a valid X (domain)
 * value.
 *
 * @param x X (domain) value to determine first derivative for
 * @return double First derivative for the given domain value if valid, 0
 *         otherwise
 * @throws iException When the the interpolation solution has not been computed
 *                    using Compute().
 */
double DataInterp::firstDerivative(const double x) const throw (iException &) {
  if (!isComputed()) reportError("Evaluate",
                               "Interpolation not computed after points",
                               _FILEINFO_);
//  Go ahead and get the point
  double value;
  integrityCheck(gsl_spline_eval_deriv_e(_interp, x, _acc, &value), _FILEINFO_);
  return (value);
}

/**
 *@brief Computes second derivative value for given domain value
 *
 * This method returns the second derivative value given a valid X (domain)
 * value.
 *
 * @param x X (domain) value to determine second derivative for
 * @return double Second derivative for the given domain value if valid, 0
 *         otherwise
 * @throws iException When the the interpolation solution has not been computed
 *                    using Compute().
 */
double DataInterp::secondDerivative(const double x) const throw (iException &) {
  if (!isComputed()) reportError("Evaluate",
                               "Interpolation not computed after points",
                               _FILEINFO_);
//  Go ahead and get the point
  double value;
  integrityCheck(gsl_spline_eval_deriv2_e(_interp, x, _acc, &value), 
                 _FILEINFO_);
  return (value);

}

/**
 * @brief Resets the state of the object
 *
 * This method will deallocate (inactivate) the internal state of the object and
 * set it up for receiving a completely new dataset.
 */
void DataInterp::Reset() {
  deallocate();
  _x.clear();
  _y.clear();
  return;
}

/**
 * @brief Initializes the object upon instantiation
 * 
 * This method sets up the initial state of the object, typically at
 * instantiation.  It sets internal pointer to their defaults (0) and populates
 * the interpolation function table identifying which options are available to
 * the users of this class.
 * 
 * It sets the default option type to Cubic.
 * 
 * GSL error handling is turned off - upon repeated instantiation of this
 * object.  The GSL default error handling, termination of the application via
 * an abort() is unacceptable.  This calls adopts an alternative policy provided
 * by the GSL whereby error checking must be done by the calling environment.
 * This has an unfortunate drawback in that it is not enforceable in an object
 * oriented environment that utilizes the GSL in disjoint classes.
 */
void DataInterp::initInterp() {
  _acc = 0;
  _interp = 0;
  _itype = Cubic;
  _interpFunctors.insert(make_pair(Linear,gsl_interp_linear));
  _interpFunctors.insert(make_pair(Polynomial,gsl_interp_polynomial));
  _interpFunctors.insert(make_pair(Cubic,gsl_interp_cspline));
  _interpFunctors.insert(make_pair(Cubic_Periodic,gsl_interp_cspline_periodic));
  _interpFunctors.insert(make_pair(Akima,gsl_interp_akima));
  _interpFunctors.insert(make_pair(Akima_Periodic,gsl_interp_akima_periodic));

  // Turn all GSL error handling off...repeatedly, every time this routine is
  // called.
  gsl_set_error_handler_off();
}

/**
 * @brief Search for an interpolation function
 * 
 * This method searches the supported options table for a given interpolation
 * function as requested by the caller.  If it is not found, an exception will
 * be thrown indicating the erroneous request.
 * 
 * @param itype  Type of the interpolator to find
 * 
 * @return DataInterp::InterpFunctor Pointer to the spline interpolator
 *         construct.
 * @throws iException When the given option is not found
 */
DataInterp::InterpFunctor DataInterp::findFunctor(Isis::DataInterp::InterpType itype) 
                                                  const throw (iException &) {
  FunctorConstIter fItr = _interpFunctors.find(itype);
  if (fItr == _interpFunctors.end()) {
    ostringstream msg;
    msg << "Unable to find interpolator with id = " << static_cast<int> (itype)
        << "!" << std::endl;
    throw iException::Message(iException::Programmer,msg.str(),_FILEINFO_);
  }
  return (fItr->second);
}

/**
 * @brief Allocates an interpolation function
 * 
 * This methid is called to allocate an interpolation function of the type as
 * maintained internal to this object.  If it deemed invalid, an exception will
 * be thrown.
 * 
 * @param npoints Number of points to allocate for the interpolator
 */
void DataInterp::allocate(int npoints) {
  deallocate();
  _acc = gsl_interp_accel_alloc();
  _interp = gsl_spline_alloc(findFunctor(_itype), npoints);
  return;
}

/**
 * @brief Deallocate interpolator resources
 *
 * If an interpolator function has been allocated, this routine will free its
 * resources and reset internal pointers to reflect this state and provide a
 * mechanism to test its state.
 */
void DataInterp::deallocate() {
  if (_interp) gsl_spline_free(_interp);
  if (_acc) gsl_interp_accel_free(_acc);
  _acc = 0;
  _interp = 0;
  return;
}

/**
 * @brief Checks the status of the interpolation operations
 * 
 * This method takes a return status from a GSL call and determines if it is
 * completed successfully.  This implementation currently allows the GSL_DOMAIN
 * error to propagate through sucessfully as the domain can be checked by the
 * caller if they deem this necessary.
 * 
 * @param gsl_status Return status of a GSL function call
 * @param src Name of the calling source invoking the check.  This allows more
 *            precise determination of the error.
 * @param line Line of the calling source that invoked the check
 */
void DataInterp::integrityCheck(int gsl_status, char *src, int line) 
                                const throw (iException &) {
  if (gsl_status != GSL_SUCCESS) {
    if (gsl_status != GSL_EDOM) {
      string msg = "GSL error occured: " + string(gsl_strerror(gsl_status));
      throw iException::Message(iException::Programmer,msg.c_str(),src,line);
    }
  }
  return;
}

/**
 * @brief Generalized error report generator
 * 
 * This method is used throughout this class to standardize error reporting as a
 * convenience to its implementor.
 * 
 * @param src  Name of calling source error was called in
 * @param error Error context string provided by the caller
 * @param filesrc Name of the file the error occured in.
 * @param lineno Line number of the calling source where the error occured.
 */
void DataInterp::reportError(const std::string &src, const std::string &error,
                             char *filesrc, int lineno) const 
                             throw (iException &) {
  string msg = "Error: " + src + " - " + error;
  throw iException::Message(iException::Programmer,msg.c_str(),filesrc,lineno);
  return;
}

};
