#ifndef DataInterp_h
#define DataInterp_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $                                                             
 * $Date: 2008/06/18 22:54:35 $                                                                 
 *                                                                        
 *   Unless noted otherwise, the portions of Isis written by the USGS are 
 *   public domain. See individual third-party library and package descriptions 
 *   for intellectual property information, user agreements, and related  
 *   information.                                                         
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or   
 *   implied, is made by the USGS as to the accuracy and functioning of such 
 *   software and related material nor shall the fact of distribution     
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.                                        
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html                
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include <string>
#include <vector>
#include <map>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include "iException.h"

namespace Isis {

  /**                                          
   * @brief DataInterp provides several options for x, y data interpolation
   * 
   * DataInterp provides 4 distinct types of data interpolation routines.  These
   * routines act on x, y pairs of data points.  The following forms of data
   * interpolation are supported:
   * 
   * <table>
   * <caption>
   *   Supported Interpolation Functions</caption>
   *   <tr>
   *     <th>Name</th>
   *     <th>Type (enum)</th>
   *     <th>MinPoints</th>
   *     <th>Description</th>
   *   </tr>
   *   <tr>
   *      <td>Linear</td>
   *      <td>Linear</td>
   *      <td>2</td>
   *      <td>
   *         Linear interpolation between points.  There are abrupt
   *         discontinuities at the supplied data points
   *      </td>
   *   </tr>
   *   <tr>
   *      <td>Polynomial</td>
   *      <td>Polynomial</td>
   *      <td>3</td>
   *      <td>
   *         Computes a polynomial interpolation.  This method should only be used
   *         for interpolating small numbers of points because polynomial
   *         interpolation introduces large oscillations, even for well-behaved
   *         datasets.  The number of terms in the interpolating polynomial is
   *         equal to the number of points.
   *      </td>
   *   </tr>
   *   <tr>
   *      <td>Cubic Spline</td>
   *      <td>Cubic</td>
   *      <td>3</td>
   *      <td>
   *        Cubic spline with natural boundary conditions.  Th resulting curve is
   *        piecewise cubic on each interval with matching first and second
   *        derivatives at the supplied data points.  The second derivative is
   *        chosen to be zero at the first point and last point.
   *      </td>
   *   </tr>
   *   <tr>
   *      <td>Periodic Cubic Spline</td>
   *      <td>Cubic_Periodic</td>
   *      <td>2</td>
   *      <td>
   *        Cubic spline with periodic boundary conditions.  Th resulting curve is
   *        piecewise cubic on each interval with matching first and second
   *        derivatives at the supplied data points.  The derivatives at the first
   *        and last point are also matched.  Note that the last point in the data
   *        must have the same y-value as the first point, otherwise the resulting
   *        periodic interpolation will have a discontinuity at the boundary.
   *      </td>
   *   </tr>
   *   <tr>
   *      <td>Akima</td>
   *      <td>Akima</td>
   *      <td>5</td>
   *      <td>
   *        Non-rounded Akima spline with natural boundary conditions.  This
   *        method uses non-rounded corner algorithm of Wodicka.
   *      </td>
   *   </tr>
   *   <tr>
   *      <td>Periodic Akima</td>
   *      <td>Akima_Periodic</td>
   *      <td>5</td>
   *      <td>
   *        Non-rounded Akima spline with periodic boundary conditions.  This
   *        method uses non-rounded corner algorithm of Wodicka.
   *      </td>
   *   </tr>
   * </table>
   * 
   * Below is an example demonstating its use:
   * @code
   *    inline double xValue(double ndx) { return (ndx + 0.5 * sin (ndx); }
   *    inline double yValue(double ndx) { return (ndx + cos ((ndx * ndx))); }
   * 
   *    DataInterp spline(DataInterp::Cubic);
   *    for (int i = 0; i < 200 ; i++) {
   *      spline.addPoint(xValue(i), yValue(i));
   *    }
   * 
   *    spline.Compute();
   *    for (int i = 0; i < 200 ; i+=10) {
   *      double yinterp = spline.Evaluate(xValue(i));
   *    }
   * @endcode
   * 
   * To compute the same data set using the Akima spline, just use the following:
   * @code 
   * spline.Compute(DataInterp::Akima);
   * @endcode
   * 
   * <h1>Caveats</h1>
   * When using this class, there are some important details that requires
   * consideration.  This class uses the GSL library.  As such, the spirit
   * of @b interpolation is strictly adhered to.  This class does not handle
   * @b extrapolation well.  The input X domain must be within the minimum
   * and maximum X values of the original data points.  This implementation
   * handles this issue by allowing the such extrapolations.  The last Y value in
   * the data range at the appropriate end is returned when the input X (domain)
   * value exceeds the domain range. First and second derivatives are zero for
   * this condition as well.
   * 
   * In addition, the GSL library default error handling scheme has implications
   * when used in C++ objects.  The default behavior is to terminate the
   * application when an error occurs.  Options for developers are to turn off
   * error trapping within the GSL, which means users of the library must do
   * their own error checking, or a specific handler can be specified to replace
   * the default mode.  Neither option is well suited in object-oriented
   * implementations because they apply globally, to all users of the library.
   * The approach used here is to turn off error handling and require users
   * to handle their own error checking.  This is not entirely safe as any
   * user that does not follow this strategy could change this behavior at
   * runtime.  Other options should be explored. An additional option is that
   * the default behavior can be altered prior to building GSL but this is also
   * problematic since this behavior cannot be guaranteed universally.
   * 
   * @ingroup Math                      
   * @author 2006-06-14 Kris Becker 
   *  
   * @history 2008-06-18 Christopher Austin - Fixed documentation
   */                                                                       
  class DataInterp {
    public:
      /**
       * This enum defines the types of interpolation supported in this class
       */
      enum InterpType { Linear,         //!< Linear interpolation
                        Polynomial,     //!< Polynomial interpolation
                        Cubic,          //!< Cubic Spline interpolation
                        Cubic_Periodic, //!< Periodic Cubic Spline interpolation
                        Akima,          //!< Akima Spline interpolation
                        Akima_Periodic  //!< Periodic Akima Spline interpolation
                      };

      // Constructors and Destructor
      DataInterp();
      DataInterp(const Isis::DataInterp::InterpType &itype);
      DataInterp (unsigned int n, double *x, double *y,
                   const Isis::DataInterp::InterpType itype = Cubic);
      DataInterp(const DataInterp &dint);
      DataInterp &operator=(const DataInterp &dint);
      virtual ~DataInterp ();

      /**
       * @brief Returns the size of the points contained in the interpolation
       * 
       * @return unsigned int Number of points in the interpolation
       */
      inline unsigned int size() const { return(_x.size()); }
      /**
       * @brief Returns the enumerated type of interpolation chosen
       * 
       * Note that this can be selected after all the points are added in the
       * interpolation by using the Compute(InterpType) method.
       * 
       * @return InterpType Currently chosen interpolation type
       */
      inline InterpType type() const { return (_itype); }
      std::string name() const;
      std::string name(const Isis::DataInterp::InterpType itype) const;
      int minPoints() const;
      int minPoints(const Isis::DataInterp::InterpType itype) const;
      double domainMinimum() const;
      double domainMaximum() const;

      void addPoint(const double x, const double y);

      /**
       * @brief Indicates the spline has been computed
       * 
       * This method indicates that the spline has been computed, typically via
       * the Compute() method, and interpolated values are accessable via the
       * Evaluate() method.
       * 
       * @return bool True if the spline has been computed, false if not
       */
      inline bool isComputed() const { return ((_interp) && (_acc)); }
      void Compute() throw (iException &);
      void Compute(Isis::DataInterp::InterpType itype) throw (iException &);

      double Evaluate (const double x) const throw (iException &);
      double firstDerivative(const double x) const throw (iException &);
      double secondDerivative(const double x) const throw (iException &);

      void Reset();

    private:
      typedef const gsl_interp_type * InterpFunctor;  //!< Interpolation specs
      InterpType        _itype;             //!<  Interpolation type
      gsl_interp_accel *_acc;               //!<  Lookup accelorator
      gsl_spline       *_interp;            //!<  Currently active interpolator

      std::vector<double>  _x;              //!<  List of X values
      std::vector<double>  _y;              //!<  List of Y values

      //!  Set up a std::map of interpolator functors
      typedef std::map<InterpType, InterpFunctor> FunctorList;
      //!< List of function types

      typedef FunctorList::const_iterator FunctorConstIter; //!< Iterator
      FunctorList _interpFunctors;   //!< Maintains list of interpolator options

//  Private functions
      void initInterp();
      void allocate(int npoints);
      InterpFunctor findFunctor(Isis::DataInterp::InterpType itype) const throw (iException &);
      void deallocate();
      void integrityCheck(int gsl_status, char *src, int line) const
                          throw (iException &);
      void reportError(const std::string &src, const std::string &error,
                       char *filesrc, int lineno) const throw (iException &);

  };
}


#endif

