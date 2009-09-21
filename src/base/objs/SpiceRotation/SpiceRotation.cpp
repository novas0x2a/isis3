#include <string>
#include <algorithm>
#include <vector>
#include <cfloat>

#include <cmath>
#include <iomanip>

#include "SpiceRotation.h"
#include "Quaternion.h"
#include "LineEquation.h"
#include "BasisFunction.h"
#include "LeastSquares.h"
#include "BasisFunction.h"
#include "PolynomialUnivariate.h"
#include "iString.h"
#include "iException.h"
#include "Table.h"
#include "NaifStatus.h"

// Declarations for binding for Naif Spicelib routine refchg_ that does not have
// a wrapper
extern int refchg_(integer *frame1, integer *frame2, doublereal *et,
                   doublereal *rotate);
// Temporary declarations for bindings for Naif supportlib routines
// These three declarations should be removed once supportlib is in Isis3
extern int ck3sdn(double sdntol, bool avflag, int *nrec,
                   double *sclkdp, double *quats, double *avvs,
                   int nints, double *starts, double *dparr, 
                   int *intarr);

namespace Isis {
  /**
   * Construct an empty SpiceRotation class using a valid Naif frame code to
   * set up for getting rotation from Spice kernels.  See required reading
   * ftp://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/ascii/individual_docs/naif_ids.req
   *
   * @param frameCode Valid naif frame code.
   */
  SpiceRotation::SpiceRotation( int frameCode ) {
    p_frameCode = frameCode;
    p_timeBias = 0.0;
    p_source = Spice;
    p_RJ.resize(9);
    p_matrixSet = false;
    p_et = -DBL_MAX;
    p_degree = 2;
    p_degreeApplied = false;
    p_noOverride = true;
    p_axis1 = 3;
    p_axis2 = 1;
    p_axis3 = 3;
    p_minimizeCache = No;
  }

  /**
   * Construct an empty SpiceRotation class using valid Naif frame code and.
   * body code to set up for computing nadir rotation.  See required reading
   * ftp://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/ascii/individual_docs/naif_ids.req
   *
   * @param frameCode Valid naif frame code.
   * @param targetCode Valid naif body code.
   */
  SpiceRotation::SpiceRotation( int frameCode, int targetCode ) {
    NaifStatus::CheckErrors();

    p_frameCode = frameCode;
    p_targetCode = targetCode;
    p_timeBias = 0.0;
    p_source = Nadir;
    p_RJ.resize(9);
    p_matrixSet = false;
    p_et = -DBL_MAX;
    p_axisP = 3;
    p_degree = 2;
    p_degreeApplied = false;
    p_noOverride = true;
    p_axis1 = 3;
    p_axis2 = 1;
    p_axis3 = 3;
    p_minimizeCache = No;

    // Determine the axis for the velocity vector
    std::string key = "INS" + Isis::iString(frameCode) + "_TRANSX";
    SpiceDouble transX[2];
    SpiceInt number;
    SpiceBoolean found;
    //Read starting at element 1 (skipping element 0)
    gdpool_c ( key.c_str(), 1, 2, &number, transX, &found );

    if (!found) {
      std::string msg = "Cannot find [" + key + "] in text kernels";
      throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
    }

    p_axisV = 2;
    if (transX[0] < transX[1]) p_axisV = 1;

    NaifStatus::CheckErrors();
  }

  /** Apply a time bias when invoking SetEphemerisTime method.
   *
   * The bias is used only when reading from NAIF kernels.  It is added to the
   * ephermeris time passed into SetEphemerisTime and then the body
   * position is read from the NAIF kernels and returned.  When the cache
   * is loaded from
   * a table the bias is ignored as it is assumed to have already been
   * applied.  If this method is never called the default bias is 0.0
   * seconds.
   *
   * @param timeBias time bias in seconds
   */
  void SpiceRotation::SetTimeBias (double timeBias) {
    p_timeBias = timeBias;
  }

  /** Return the J2000 to reference frame quaternion at given time.
   *
   * This method returns the J2000 to reference frame rotational matrix at a
   * given et in seconds.  The quaternion is obtained from either valid NAIF ck
   * and/or fk, or alternatively from an internal cache loaded from an ISIS
   * Table object.  In the first case, the kernels must contain the rotation
   * for the frame specified in the constructor at the given time (as well as
   * all the intermediate frames going from the reference frame to J2000) and
   * they must be loaded using the SpiceKernel class.
   *
   * @param et   ephemeris time in seconds
   */
  void SpiceRotation::SetEphemerisTime(double et) {
    NaifStatus::CheckErrors();

    // Save the time
    if (p_et == et) return;
    p_et = et;

    SpiceInt j2000 = 1;
    // namfrm_c("J2000", &j2000);   // Trust naif not to change it

    // Read from the cache
    if (p_source == Memcache) {
      // If the cache has only one position set it
      if (p_cache.size() == 1) {
/*        p_quaternion = p_cache[0];*/
        p_RJ = p_cache[0];
//        p_RJ = p_quaternion.ToMatrix();
      }

      else {
        // Otherwise determine the interval to interpolate
        std::vector<double>::iterator pos;
        pos = upper_bound(p_cacheTime.begin(),p_cacheTime.end(),p_et);

        int cacheIndex;
        if (pos != p_cacheTime.end()) {
          cacheIndex = distance(p_cacheTime.begin(),pos);
          cacheIndex--;
        } else {
          cacheIndex = p_cacheTime.size() - 2;
        }

        if (cacheIndex < 0) cacheIndex = 0;

// Interpolate the rotation
        double mult = (p_et - p_cacheTime[cacheIndex]) /
                      (p_cacheTime[cacheIndex+1] - p_cacheTime[cacheIndex]);
/*        Quaternion Q2 (p_cache[cacheIndex+1]);
        Quaternion Q1 (p_cache[cacheIndex]);*/
        std::vector<double> RJ2( p_cache[cacheIndex+1] );
        std::vector<double> RJ1 (p_cache[cacheIndex] );
        SpiceDouble J2J1[3][3];
        mtxm_c ((SpiceDouble (*)[3]) &RJ2[0], (SpiceDouble (*)[3]) &RJ1[0], J2J1);
        SpiceDouble axis[3];
        SpiceDouble angle;
        raxisa_c (J2J1, axis, &angle);
        SpiceDouble delta[3][3];
        axisar_c (axis, angle*(SpiceDouble)mult, delta);
        mxmt_c ( (SpiceDouble *) &RJ1[0], delta, (SpiceDouble (*) [3]) &p_RJ[0] );
      }
    }

    // Apply coefficients defining a function for each of the three camera angles
    else if (p_source == Function) {
      Isis::PolynomialUnivariate function1( p_degree );
      Isis::PolynomialUnivariate function2( p_degree );
      Isis::PolynomialUnivariate function3( p_degree );

      // Load the functions with the coefficients
      function1.SetCoefficients ( p_coefficients[0] );
      function2.SetCoefficients ( p_coefficients[1] );
      function3.SetCoefficients ( p_coefficients[2] );
      
      std::vector<double> rtime;
      rtime.push_back((et - p_baseTime) / p_timeScale);
      double angle1 = function1.Evaluate (rtime);
      double angle2 = function2.Evaluate (rtime);
      double angle3 = function3.Evaluate (rtime);

      // Get the first angle back into the range Naif expects [180.,180.]
      if (angle1 < -1*pi_c() ) {
        angle1 += twopi_c();
      }
      else if (angle1 > pi_c()) {
        angle1 -= twopi_c();
      }

      eul2m_c ( (SpiceDouble) angle3, (SpiceDouble) angle2, (SpiceDouble) angle1,
                 p_axis3,             p_axis2,              p_axis1,
                 (SpiceDouble (*)[3]) &p_RJ[0]);
    }
    // Read from the kernel
    else if (p_source == Spice) {
      // Retrieve the J2000 (code=1) to reference rotation matrix
      integer toFrameCode = p_frameCode;
      SpiceDouble time = p_et + p_timeBias;
      refchg_ ( &j2000, &toFrameCode, &time, (doublereal *) &p_RJ[0] );

      if ( failed_c() ) {
        char naifstr[64];
        getmsg_c ("SHORT", sizeof(naifstr), naifstr);
        reset_c();  // Reset Naif error system to allow caller to recover

        if ( eqstr_c( naifstr, "SPICE(UNKNOWNFRAME)" )) {
          Isis::iString msg = Isis::iString( (int) p_frameCode) + " is an unrecognized " + 
            "reference frame code.  Has the mission frames kernel been loaded?";
          throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
        }
        else {
          Isis::iString msg = "No pointing available at requested time [" +
                           Isis::iString(p_et+p_timeBias) + "] for frame code [" +
                           Isis::iString( (int) p_frameCode) + "]";
          throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
        }
      }

      // Transpose to obtain row-major order
      xpose_c ( (SpiceDouble (*)[3]) &p_RJ[0], (SpiceDouble (*)[3]) &p_RJ[0]);
    }

    // Compute from Nadir
    else {
      // TODO what about spk time bias and mission setting of light time corrections
      //      That information has only been passed to the SpicePosition class and
      //      is not available to this class, but probably should be applied to the
      //      spkez call.
      SpiceDouble stateJ[6];  // Position and velocity vector in J2000
      SpiceDouble lt;
      SpiceInt spkCode = p_frameCode / 1000;
      spkez_c ( (SpiceInt) spkCode, p_et, "J2000", "LT+S",
                (SpiceInt) p_targetCode, stateJ, &lt );
      // reverse the position to be relative to the spacecraft.  This may be
      // a mission dependent value and possibly the sense of the velocity as well.
      SpiceDouble sJ[3],svJ[3];
      vpack_c ( -1*stateJ[0], -1*stateJ[1], -1*stateJ[2], sJ);
      vpack_c ( stateJ[3], stateJ[4], stateJ[5], svJ);
      twovec_c ( sJ,
                 p_axisP,
                 svJ,
                 p_axisV,
                 (SpiceDouble (*)[3]) &p_RJ[0]);
    }


    // Set the quaternion for this rotation
 //   p_quaternion.Set ( p_RJ );
    NaifStatus::CheckErrors();
  }

  /** Cache J2000 rotation quaternion over a time range.
   *
   * This method will load an internal cache with frames over a time
   * range.  This prevents the NAIF kernels from being read over-and-over
   * again and slowing an application down due to I/O performance.  Once the
   * cache has been loaded then the kernels can be unloaded from the NAIF
   * system.
   *
   * @param startTime   Starting ephemeris time in seconds for the cache
   * @param endTime     Ending ephemeris time in seconds for the cache
   * @param size        Number of frames to keep in the cache
   *
   */
  void SpiceRotation::LoadCache (double startTime, double endTime, int size) {

    // Check for valid arguments
    if (size <= 0) {
      std::string msg = "Argument cacheSize must not be less or equal to zero";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    if (startTime > endTime) {
      std::string msg = "Argument startTime must be less than or equal to endTime";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    if ((startTime != endTime) && (size == 1)) {
      std::string msg = "Cache size must be more than 1 if startTime endTime differ";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    // Make sure cache isn't already loaded
    if (p_source == Memcache) {
      std::string msg = "A SpiceRotation cache has already been created";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    // Save full cache parameters
    p_fullCacheStartTime = startTime;
    p_fullCacheEndTime = endTime;
    p_fullCacheSize = size;

    LoadTimeCache();
    int cacheSize = p_cacheTime.size();

    // Loop and load the cache
    for (int i=0; i<cacheSize; i++) {
      double et = p_cacheTime[i];
      SetEphemerisTime(et);
      p_cache.push_back( p_RJ );
    }
    p_source = Memcache;

// Downsize already loaded caches (both time and quats)
    if (p_minimizeCache == Yes  &&  cacheSize > 5) {
      LoadTimeCache();
    }
  }

  /** Cache J2000 to frame rotation for a time.
   *
   * This method will load an internal cache with a rotation for a single
   * time (e.g. useful for framing cameras). This prevents
   * the NAIF kernels from being read over-and-over again and slowing a
   * application down due to I/O performance.  Once the
   * cache has been loaded then the kernels can be unloaded from the NAIF
   * system.  This calls the LoadCache(stime,etime,size) method using the
   * time as both the starting and ending time with a size of 1.
   *
   * @param time   single ephemeris time in seconds to cache
   *
   */
  void SpiceRotation::LoadCache (double time) {
    LoadCache(time,time,1);
  }

  /** Cache J2000 rotations using a table file.
   *
   * This method will load either an internal cache with rotations (quaternions) 
   * or coefficients (for 3 polynomials defining the camera angles) from an
   * ISIS table file.  In the first case, the table must have 5 columns and 
   * at least one row. The 5 columns contain the following information, J2000 
   * to reference quaternion (q0, q1, q2, q3)  and the ephemeris time of that 
   * position. If there are multiple rows, it is assumed the quaternions between 
   * the rows can be interpolated.  In the second case, the table must have 
   * three columns and at least two rows.  The three columns contain the 
   * coefficients for each of the three camera angles.  Row one of the 
   * table contains coefficient 0 (constant term) for angles 1, 2, and 3. 
   * If the degree of the fit equation is greater than 1, row 2 contains 
   * coefficient 1 (linear) for each of the three angles.  Row n contains 
   * coefficient n-1 and the last row contains the time parameters, base time, 
   * and time scale, and the degree of the polynomial. 
   *
   * @param table   An ISIS table blob containing valid J2000 to reference
   *                quaternion/time values
   */
  void SpiceRotation::LoadCache(Table &table) {
    // Make sure cache isn't already loaded
    if (p_source == Memcache  ||  p_source == Function) {
      std::string msg = "A SpiceRotation cache has already been created";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    int recFields = table[0].Fields();

    // Loop through and move the table to the cache.  Retrieve the first record to 
    // establish the type of cache and then use the appropriate loop.
    
    // list table
    if (recFields == 5) {
      for (int r=0; r<table.Records(); r++) {
        TableRecord &rec = table[r];

        if (rec.Fields() != recFields) {
          // throw and error
        }

        std::vector<double> j2000Quat;
        j2000Quat.push_back((double)rec[0]);
        j2000Quat.push_back((double)rec[1]);
        j2000Quat.push_back((double)rec[2]);
        j2000Quat.push_back((double)rec[3]);

        Quaternion q(j2000Quat);
        std::vector<double> RJ = q.ToMatrix();
        p_cache.push_back(RJ);
        p_cacheTime.push_back((double)rec[4]);
      }
      p_source = Memcache;
    }

      // coefficient table
    else if (recFields == 3) {
      std::vector<double> coeffAng1,coeffAng2,coeffAng3;

      for (int r=0; r<table.Records()-1; r++) {
        TableRecord &rec = table[r];

        if (rec.Fields() != recFields) {
          // throw and error
        }
        coeffAng1.push_back((double)rec[0]);
        coeffAng2.push_back((double)rec[1]);
        coeffAng3.push_back((double)rec[2]);
      }

      // Take care of time parameters
      TableRecord &rec = table[table.Records()-1];
      double baseTime = (double)rec[0];
      double timeScale = (double)rec[1];
      double degree = (double)rec[2];
      SetPolynomialDegree ((int) degree);
      SetOverrideBaseTime (baseTime, timeScale);
      SetPolynomial(coeffAng1, coeffAng2, coeffAng3);
      p_source = Function;
    }
    else  {
      std::string msg = "Expecting either three or five fields in the SpiceRotation table";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  }



  /** Cache J2000 rotation over existing cached time range using polynomials
   *
   * This method will reload an internal cache with matrices
   * formed from rotation angles fit to polynomials over a time
   * range.
   *
   * @param function1   The first polynomial function used to
   *                    find the rotation angles
   * @param function2   The second polynomial function used to
   *                    find the rotation angles
   * @param function3   The third polynomial function used to
   *                    find the rotation angles
   */
  void SpiceRotation::ReloadCache (Isis::PolynomialUnivariate &function1,
                                 Isis::PolynomialUnivariate &function2,
                                 Isis::PolynomialUnivariate &function3){
    NaifStatus::CheckErrors();

   // Make sure cache is already loaded
    if (p_source != Memcache) {
      std::string msg = "A SpiceRotation cache has not been created yet";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

   // Clear existing matrices from cache
    p_cache.clear();

    //    std::cout<<std::setprecision(24);
    //    std::cout<<"PolyAng1  PolyAng2  PolyAng3  time"<<std::endl;

    for (std::vector<double>::size_type pos=0;pos < p_cacheTime.size();pos++) {
      double et = p_cacheTime.at(pos);
      std::vector<double> rtime;
      rtime.push_back((et - p_baseTime) / p_timeScale);
      double angle1 = function1.Evaluate (rtime);
      double angle2 = function2.Evaluate (rtime);
      double angle3 = function3.Evaluate (rtime);

      //      std::cout<<angle1<<" "<<angle2<<" "<<angle3<<" "<<et<<std::endl;


// Get the first angle back into the range Naif expects [180.,180.]
      if (angle1 < -1*pi_c() ) {
        angle1 += twopi_c();
      }
      else if (angle1 > pi_c()) {
        angle1 -= twopi_c();
      }

      eul2m_c ( (SpiceDouble) angle3, (SpiceDouble) angle2, (SpiceDouble) angle1,
                 p_axis3,             p_axis2,              p_axis1,
                 (SpiceDouble (*)[3]) &p_RJ[0]);
      p_cache.push_back( p_RJ );
    }

    double et = p_et;
    p_et = -DBL_MAX;
    SetEphemerisTime(et);


    NaifStatus::CheckErrors();
  }



  /** Return a table with J2000 to reference rotations.
   *
   * Return a table containing the cached pointing with the given
   * name. The table will have either five columns (for a list cache) 
   * of J2000 to reference quaternions and times, or three columns 
   * (for a coefficient cache), of J2000 to reference frame rotation 
   * angles defined by coefficients of a polynomial function (see 
   * SetPolynommial).  In the coefficient cache the last row of 
   * the table is the base time, time scale, and polynomial degree. 
   *
   * @param tableName    Name of the table to create and return
   */
  Table SpiceRotation::Cache(const std::string &tableName) {
    // Load the list of rotations and their corresponding times
    if (p_source == Memcache) {
      TableField q0("J2000Q0",TableField::Double);
      TableField q1("J2000Q1",TableField::Double);
      TableField q2("J2000Q2",TableField::Double);
      TableField q3("J2000Q3",TableField::Double);
      TableField t("ET",TableField::Double);

      TableRecord record;
      record += q0;
      record += q1;
      record += q2;
      record += q3;
      record += t;

      Table table(tableName,record);

      for (int i=0; i<(int)p_cache.size(); i++) {
        Quaternion q(p_cache[i]);
        std::vector<double> v = q.GetQuaternion();
        record[0] = v[0];
        record[1] = v[1];
        record[2] = v[2];
        record[3] = v[3];
        record[4] = p_cacheTime[i];
        table += record;
      }
      return table;
    }
    // Load the coefficients for the curves fit to the 3 camera angles
    else if (p_source == Function) {
      TableField angle1("J2000Ang1",TableField::Double);
      TableField angle2("J2000Ang2",TableField::Double);
      TableField angle3("J2000Ang3",TableField::Double);

      TableRecord record;
      record += angle1;
      record += angle2;
      record += angle3;

      Table table(tableName,record);

      for (int cindex=0; cindex<p_degree+1; cindex++) {
        record[0] = p_coefficients[0][cindex];
        record[1] = p_coefficients[1][cindex];
        record[2] = p_coefficients[2][cindex];
        table += record;
      }

      // Load one more table entry with the time adjustments for the fit equation
      // t = (et - baseTime)/ timeScale
      record[0] = p_baseTime;
      record[1] = p_timeScale;
      record[2] = (double) p_degree;

      table += record;
      return table;
    }
    else {
      // throw an error -- should not get here -- invalid Spice Source
      std::string msg = "To create table source of data must be either Memcache or Function";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

  }


  /** Return the camera angles (right ascension, declination, and twist) for
   *
   * the matrix
   *
   */
  std::vector<double> SpiceRotation::Angles( int axis3, int axis2, int axis1 )
  {
    NaifStatus::CheckErrors();

    SpiceDouble ang1,ang2,ang3;
    m2eul_c ((SpiceDouble *) &p_RJ[0],axis3, axis2, axis1, &ang3,&ang2, &ang1 );

    std::vector<double> angles;
    angles.push_back(ang1);
    angles.push_back(ang2);
    angles.push_back(ang3);

    NaifStatus::CheckErrors();
    return angles;
  }

  /** Given a direction vector in the reference frame, return a J2000 direction.
   *
   * @param [in]  rVec A direction vector in the reference frame
   *
   * @return (vector<double>)   A direction vector in J2000 frame
   */
  std::vector<double> SpiceRotation::J2000Vector( const std::vector<double>& rVec) {
    NaifStatus::CheckErrors();

    std::vector<double> Jvec(3);
    mtxv_c( (SpiceDouble *) &p_RJ[0], (SpiceDouble *) &rVec[0],
            (SpiceDouble *) &Jvec[0]);

    NaifStatus::CheckErrors();
    return ( Jvec );
  }


  /** Given a direction vector in J2000, return a reference frame direction.
   *
   * @param [in] jVec A direction vector in J2000
   *
   * @return (vector<double>)   A direction vector in reference
   *         frame
   */
  std::vector<double>
  SpiceRotation::ReferenceVector( const std::vector<double>& jVec ) {
    NaifStatus::CheckErrors();

    std::vector<double> Rvec(3);
    mxv_c ( (SpiceDouble *) &p_RJ[0], (SpiceDouble *) &jVec[0],
            (SpiceDouble *) &Rvec[0] );

    NaifStatus::CheckErrors();
    return ( Rvec );
  }


  /** Set the coefficients of a polynomial fit to each
   * of the three camera angles for the time period covered by the
   * cache, angle = a + bt + ct**2, where t = (time - p_baseTime)/ p_timeScale.
   *
   */
  void SpiceRotation::SetPolynomial () {

    // Rotation is already stored as a polynomial -- throw an error
    if (p_source == Function) {
      // Nothing to do
      return;
//      std::string msg = "Rotation already fit to a polynomial -- spiceint first to refit";
//      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }
    Isis::PolynomialUnivariate function1(p_degree);       //!< Basis function fit to 1st rotation angle
    Isis::PolynomialUnivariate function2(p_degree);       //!< Basis function fit to 2nd rotation angle
    Isis::PolynomialUnivariate function3(p_degree);       //!< Basis function fit to 3rd rotation angle
                                    //
    LeastSquares *fitAng1 = new LeastSquares ( function1 );
    LeastSquares *fitAng2 = new LeastSquares ( function2 );
    LeastSquares *fitAng3 = new LeastSquares ( function3 );

    // Compute the base time
    ComputeBaseTime ();
    std::vector<double> time;
    std::vector<double> coeffAng1,coeffAng2,coeffAng3;

    //    std::cout<<"Axes of rotation for axes 1, 2, and 3"<<p_axis1<<" "<<p_axis2<<" "<<p_axis3<<std::endl;


    if (p_cache.size() == 1) {
      p_degree = 0;
      double t = p_cacheTime.at(0);
      SetEphemerisTime( t );
      std::vector<double> angles = Angles ( p_axis3, p_axis2, p_axis1);
      coeffAng1.push_back(angles[0]);
      coeffAng2.push_back(angles[1]);
      coeffAng3.push_back(angles[2]);
    }
    else if (p_cache.size() == 2) {
// Load the times and get the corresponding rotation angles
      p_degree = 1;
      double t1 = p_cacheTime.at(0);
      SetEphemerisTime( t1 );
      t1 -= p_baseTime;
      t1 = t1/p_timeScale;
      std::vector<double> angles1 = Angles ( p_axis3, p_axis2, p_axis1);
      double t2 = p_cacheTime.at(1);
      SetEphemerisTime( t2 );
      t2 -= p_baseTime;
      t2 = t2/p_timeScale;
      std::vector<double> angles2 = Angles ( p_axis3, p_axis2, p_axis1 );
      angles2[0] = WrapAngle ( angles1[0], angles2[0]);
      angles2[2] = WrapAngle ( angles1[2], angles2[2]);
      double slope[3];
      double intercept[3];

// Compute the linear equation for each angle and save them
      for (int angleIndex=0; angleIndex < 3; angleIndex++) {
        Isis::LineEquation angline(t1, angles1[angleIndex], t2, angles2[angleIndex]);
        slope[angleIndex] = angline.Slope();
        intercept[angleIndex] = angline.Intercept();
      }
      coeffAng1.push_back(intercept[0]);
      coeffAng1.push_back(slope[0]);
      coeffAng2.push_back(intercept[1]);
      coeffAng2.push_back(slope[1]);
      coeffAng3.push_back(intercept[2]);
      coeffAng3.push_back(slope[2]);
    }
    else {
    // Load the known values to compute the fit equation
      double start1=0.;  // value of 1st angle1 in cache
      double start3=0.;  // value of 1st angle1 in cache

      //      std::cout<<"Actual CI angles"<<std::endl;
      //      std::cout<<std::setprecision(24);
      //      std::cout<<"Time range:  "<<p_cacheTime.at(0)<<" "<<p_cacheTime.at(p_cacheTime.size()-1)<<std::endl;


      for (std::vector<double>::size_type pos=0;pos < p_cacheTime.size();pos++) {
        double t = p_cacheTime.at(pos);
        time.push_back( (t - p_baseTime) / p_timeScale );
        SetEphemerisTime( t );
        std::vector<double> angles = Angles ( p_axis3, p_axis2, p_axis1);

// Fix 180/-180 crossovers on angles 1 and 3 before doing fit.
        if (pos == 0) {
          start1 = angles[0];
          start3 = angles[2];
        }
        else {
          angles[0] = WrapAngle ( start1, angles[0]);
          angles[2] = WrapAngle ( start3, angles[2]);
        }

        fitAng1->AddKnown ( time, angles[0] );
        fitAng2->AddKnown ( time, angles[1] );
        fitAng3->AddKnown ( time, angles[2] );
        time.clear();

        //        std::cout<<angles[0]<<" "<<angles[1]<<" "<<angles[2]<<" "<<t<<std::endl;

      }
      //Solve the equations for the coefficients
      fitAng1->Solve();
      fitAng2->Solve();
      fitAng3->Solve();

      // Delete the least squares objects now that we have all the coefficients
      delete fitAng1;
      delete fitAng2;
      delete fitAng3;

      // For now assume all three angles are fit to a polynomial.  Later they may
      // each be fit to a unique basis function.
      // Fill the coefficient vectors

      for ( int i = 0;  i < function1.Coefficients(); i++) {
        coeffAng1.push_back( function1.Coefficient( i ) );
        coeffAng2.push_back( function2.Coefficient( i ) );
        coeffAng3.push_back( function3.Coefficient( i ) );
      }

    }

    // Now that the coefficients have been calculated set the polynomial with them
    SetPolynomial ( coeffAng1, coeffAng2, coeffAng3 );

    return;
  }



  /** Set the coefficients of a polynomial fit to each of the
   * three camera angles for the time period covered by the
   * cache, angle = c0 + c1*t + c2*t**2 + ... + cn*t**n,
   * where t = (time - p_baseTime) / p_timeScale, and n = p_degree.
   *
   * @param [in] coeffAng1 Coefficients of fit to Angle 1
   * @param [in] coeffAng2 Coefficients of fit to Angle 2
   * @param [in] coeffAng3 Coefficients of fit to Angle 3
   *
   */
  void SpiceRotation::SetPolynomial ( const std::vector<double>& coeffAng1,
                                      const std::vector<double>& coeffAng2,
                                      const std::vector<double>& coeffAng3 ) {

    Isis::PolynomialUnivariate function1( p_degree );
    Isis::PolynomialUnivariate function2( p_degree );
    Isis::PolynomialUnivariate function3( p_degree );

    // Load the functions with the coefficients
    function1.SetCoefficients ( coeffAng1 );
    function2.SetCoefficients ( coeffAng2 );
    function3.SetCoefficients ( coeffAng3 );

/*    std::cout << "coeffAng1=";
    for (int i=0; i<(int) coeffAng1.size(); i++) std::cout <<coeffAng1[i]<<" ";
    std::cout << std::endl << "coeffAng2=";
    for (int i=0; i<(int) coeffAng2.size(); i++) std::cout <<coeffAng2[i]<<" ";
    std::cout << std::endl << "coeffAng3=";
    for (int i=0; i<(int) coeffAng3.size(); i++) std::cout <<coeffAng3[i]<<" ";
    std::cout << std::endl;*/


    // Compute the base time
    ComputeBaseTime ();


//    std::cout << "Basetime=" << p_baseTime << std::endl;

    // Reload the cache from the functions and the currently cached time
    // Commented out on 05-06-2009
//    ReloadCache ( function1, function2, function3 );

    // Save the current coefficients
    p_coefficients[0] = coeffAng1;

//    std::cout << "Saved coef0="<<p_coefficients[0][0]<<" "<<p_coefficients[0][1]
//              <<p_coefficients[0][2]<<std::endl;

    p_coefficients[1] = coeffAng2;

//    std::cout << "Saved coef1="<<p_coefficients[1][0]<<" "<<p_coefficients[1][1]
//              <<p_coefficients[1][2]<<std::endl;

    p_coefficients[2] = coeffAng3;

    // Set the flag indicating p_degree has been applied to the camera angles, the 
    // coefficients of the polynomials have been saved, and the cache reloaded from
    // the polynomials
    p_degreeApplied = true;
    p_source = Function;

    // Update the current rotation
    double et = p_et;
    p_et = -DBL_MAX;
    SetEphemerisTime(et);

    return;
  }



  /**
   *  Return the coefficients of a polynomial (parabola) fit to each of the
   *  three camera angles for the time period covered by the cache, angle =
   *  c0 + c1*t + c2*t**2 + ... + cn*t**n, where t = (time - p_basetime) / p_timeScale
   *  and n = p_degree.
   *
   * @param [out] coeffAng1 Coefficients of fit to Angle 1
   * @param [out] coeffAng2 Coefficients of fit to Angle 2
   * @param [out] coeffAng3 Coefficients of fit to Angle 3
   *
   */
  void SpiceRotation::GetPolynomial ( std::vector<double>& coeffAng1,
                                      std::vector<double>& coeffAng2,
                                      std::vector<double>& coeffAng3 ) {
    coeffAng1 = p_coefficients[0];
    coeffAng2 = p_coefficients[1];
    coeffAng3 = p_coefficients[2];

    return;
  }



  //! Compute the base time using cached times
  void SpiceRotation::ComputeBaseTime () {
    if (p_noOverride) {
      p_baseTime = (p_cacheTime.at(0) + p_cacheTime.at(p_cacheTime.size()-1))/ 2.;
      p_timeScale = p_baseTime - p_cacheTime.at(0);
      // Take care of case where 1st and last times are the same
      if (p_timeScale == 0)  p_timeScale = 1.0;
    }
    else {
      p_baseTime = p_overrideBaseTime;
      p_timeScale = p_overrideTimeScale;
    }

    return;
  }


  /**
   * Set an override base time to be used with observations on scanners to allow all
   * images in an observation to use the save base time and polynomials for the angles.
   * 
   * @param [in] baseTime The baseTime to use and override the computed base time
   */
  void SpiceRotation::SetOverrideBaseTime( double baseTime, double timeScale ) {
    p_overrideBaseTime = baseTime;
    p_overrideTimeScale = timeScale;
    p_noOverride = false;
    return;
  }



  /**
   *  Evaluate the derivative of the fit polynomial (parabola) defined by the
   *  given coefficients with respect to the coefficient at the given index, at
   *  the current time.
   *
   * @param coeffIndex The index of the coefficient to differentiate
   * @return The derivative evaluated at the current time
   *
   */
  double SpiceRotation::DPolynomial ( const int coeffIndex ) {
    double derivative;
    double time = (p_et - p_baseTime) / p_timeScale;

//    std::cout << "coeff index = " << coeffIndex << std::endl;

    if (coeffIndex > 0  && coeffIndex <= p_degree) {
      derivative = pow(time, coeffIndex);
    }
    else if (coeffIndex == 0) {
      derivative = 1;
    }
    else {
      Isis::iString msg = "Coeff index, " + Isis::iString(coeffIndex) + " exceeds degree of polynomial";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    return derivative;
  }



  /** Compute the derivative with respect to one of the coefficients in the
   *  angle polynomial fit equation of a vector rotated from J2000 to a
   *  reference frame.  The polynomial equation is of the form 
   *  angle = c0 + c1*t + c2*t**2 + ... cn*t**n, where t = (time - p_basetime) / p_timeScale
   *  and n = p_degree (the degree of the equation)
   *
   * @param [in]  lookJ       Look vector in J2000 frame
   * @param [in]  partialVar  Variable derivative is to be with respect to
   * @param [in]  coeffIndex  Coefficient index in the polynomial fit to the variable (angle) 
   * @return Vector rotated by derivative of J2000 to 
   *                              reference rotation
   *
   */
  std::vector<double>
    SpiceRotation::ToReferencePartial(std::vector<double>& lookJ,
                           SpiceRotation::PartialType partialVar, int coeffIndex) {
    NaifStatus::CheckErrors();
    //**TODO** To save time possibly save partial matrices

    // Get the rotation angles and form the derivative matrix for the partialVar

//    std::cout << "Partial call" << std::endl;

    std::vector<double> angles = Angles ( p_axis3, p_axis2, p_axis1 );
    int angleIndex = partialVar;
    int axes[3]={p_axis1,p_axis2,p_axis3};
    double angle = angles.at(angleIndex);

    double dmatrix[3][3];
    drotat_ (&angle, (integer *) axes+angleIndex, (doublereal *) dmatrix);
    // Transpose to obtain row-major order
    xpose_c ( dmatrix, dmatrix);

    // Get the derivative of the polynomial with respect to partialVar

    double dpoly = DPolynomial ( coeffIndex );

    // Multiply dpoly to complete dmatrix
    for ( int row = 0;  row < 3;  row++ ) {
      for ( int col = 0;  col < 3;  col++ ) {
        dmatrix[row][col] *= dpoly;
      }
    }
    // Apply the other 2 angles and chain them all together
    double dRJ[3][3];
    switch (angleIndex) {
    case 0:
      rotmat_c (dmatrix, angles[1], axes[1], dRJ );
      rotmat_c (dRJ, angles[2], axes[2], dRJ );
      break;
    case 1:
      rotate_c (angles[0], axes[0], dRJ);
      mxm_c ( dmatrix, dRJ, dRJ );
      rotmat_c (dRJ, angles[2], axes[2], dRJ );
      break;
    case 2:
      rotate_c (angles[0], axes[0], dRJ);
      rotmat_c (dRJ, angles[1], axes[1], dRJ );
      mxm_c ( dmatrix, dRJ, dRJ );
      break;
    }
     // Finally rotate the J2000 vector with the derivative matrix, dRJ
    std::vector<double> lookdR(3);

    mxv_c ( dRJ, (const SpiceDouble *) &lookJ[0], (SpiceDouble *) &lookdR[0] );

    NaifStatus::CheckErrors();
    return lookdR;
  }


  /** Wrap the input angle to keep it within 2pi radians of the
   *  angle to compare.
   *
   * @param [in]  compareAngle Look vector in J2000 frame
   * @param [in]  angle Angle to be wrapped if needed
   * @return double Wrapped angle
   *
   */
  double SpiceRotation::WrapAngle(double compareAngle, double angle) {
    NaifStatus::CheckErrors();
    double diff1 = compareAngle - angle;

    if ( diff1 < -1*pi_c() ){
      angle -= twopi_c();
    } 
    else if (diff1 > pi_c() ) {
      angle += twopi_c();
    }

    NaifStatus::CheckErrors();
    return angle;
  }

  /** Set the degree of the polynomials to be fit to the
   * three camera angles for the time period covered by the
   * cache, angle = c0 + c1*t + c2*t**2 + ... + cn*t**n,
   * where t = (time - p_baseTime) / p_timeScale, and n = p_degree.
   *
   * @param [in] degree Degree of the polynomial to be fit
   *
   */
  void SpiceRotation::SetPolynomialDegree( int degree) {

    // If polynomials have not been applied yet then simply set the degree and return
    if (!p_degreeApplied) {
      p_degree = degree;
    }

    // Otherwise the existing polynomials need to be either expanded ...
    else if (p_degree < degree) {  // (increase the number of terms)
      std::vector<double> coefAngle1(p_coefficients[0]),
                          coefAngle2(p_coefficients[1]),
                          coefAngle3(p_coefficients[2]);

      for (int icoef = p_degree+1;  icoef <= degree; icoef++) {
        coefAngle1.push_back(0.);
        coefAngle2.push_back(0.);
        coefAngle3.push_back(0.);
      }
      p_degree = degree;
      SetPolynomial (coefAngle1, coefAngle2, coefAngle3);
    }
    // ... or reduced (decrease the number of terms)
    else if (p_degree > degree) {
      std::vector<double> coefAngle1(degree + 1),
                          coefAngle2(degree + 1),
                          coefAngle3(degree + 1);

      for (int icoef = 0;  icoef <=degree;  icoef++) {
        coefAngle1.push_back(p_coefficients[0][icoef]);
        coefAngle2.push_back(p_coefficients[1][icoef]);
        coefAngle3.push_back(p_coefficients[2][icoef]);
      }
      SetPolynomial (coefAngle1, coefAngle2, coefAngle3);
      p_degree = degree;
    }
  }


  /** Set the axes of rotation for decomposition of a rotation 
   *  matrix into 3 angles.
   *
   * @param [in]  axis1 Axes of rotation of first angle applied (right rotation)
   * @param [in]  axis2 Axes of rotation of second angle applied (center rotation)
   * @param [in]  axis3 Axes of rotation of third angle applied (left rotation)
   * @return double Wrapped angle
   *
   */
  void SpiceRotation::SetAxes(int axis1, int axis2, int axis3) {
    if (axis1 < 1  ||  axis2 < 1  || axis3 < 1  || axis1 > 3  || axis2 > 3  || axis3 > 3) {
      std::string msg = "A rotation axis is outside the valid range of 1 to 3";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
    p_axis1 = axis1;
    p_axis2 = axis2;
    p_axis3 = axis3;
  }

  /** Load the time cache.  This method should works with the LoadCache(startTime, endTime, size) method
   *  to load the time cache.
   *  
   */
  void SpiceRotation::LoadTimeCache() {
    int count=0;

    double observStart  =  p_fullCacheStartTime + p_timeBias;
    double observEnd  = p_fullCacheEndTime + p_timeBias;
    bool timeLoaded = false;

    // Get number of ck loaded for this rotation.  This method assumes only one SpiceRotation
    // object is loaded.
    NaifStatus::CheckErrors();
    ktotal_c ("ck", (SpiceInt *) &count);

    // Downsize the loaded cache
    if (p_source == Memcache  && p_minimizeCache == Yes) {
      // Multiple ck case and type 5 ck case final step -- downsize loaded cache and reload

      if (p_fullCacheSize != (int) p_cache.size()) {

        Isis::iString msg = "Full cache size does NOT match cache size in LoadTimeCache -- should never happen";
        throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
      }

      SpiceDouble timeSclkdp[p_fullCacheSize];
      SpiceDouble quats[p_fullCacheSize][4];
      SpiceInt spcode = p_frameCode/1000;

      for (int r=0; r<p_fullCacheSize; r++) {
        double et = p_cacheTime[r];
        sce2c_c ( spcode, et, timeSclkdp+r);
        SpiceDouble RJ[9] = {p_cache[r][0],p_cache[r][1],p_cache[r][2],
                             p_cache[r][3],p_cache[r][4],p_cache[r][5],
                             p_cache[r][6],p_cache[r][7],p_cache[r][8]};
        m2q_c(RJ, quats[r]);
      }

      double cubeStarts=timeSclkdp[0]; //,timsSclkdp[ckBlob.Records()-1] };
      double radTol = 0.000000017453; //.000001 degrees  Make this instrument dependent TODO
      SpiceInt avflag = 0;            // Don't use angular velocity for now
      double avvs[3];                 // Not used 
      SpiceInt nints = 1;             // Always make an observation a single interpolation interval
      double dparr[p_fullCacheSize];    // Double precision work array
      SpiceInt intarr[p_fullCacheSize]; // Integer work array
      SpiceInt sizOut=p_fullCacheSize;  // Size of downsized cache
      ck3sdn(radTol, avflag, (int *) &sizOut, timeSclkdp, (doublereal *) quats, avvs, nints, &cubeStarts, dparr, (int *) intarr);

      // Clear full cache and load with downsized version
      p_cacheTime.clear();
      p_cache.clear();

      for (int r=0; r<sizOut; r++) {
        SpiceDouble et;
        sct2e_c ( spcode, timeSclkdp[r], &et);
        p_cacheTime.push_back(et);
        std::vector<double> RJ(9);
        q2m_c (quats[r], (SpiceDouble (*)[3]) &RJ[0]);
        p_cache.push_back(RJ);
      }

      timeLoaded = true;
      p_minimizeCache = Done;
    }
    else if (count == 1  && p_minimizeCache == Yes) {
      // case of a single ck -- read instances and data straight from kernel for given time range
      SpiceInt handle;

      // Define some Naif constants
      int FILESIZ = 128;
      int TYPESIZ = 32;
      int SOURCESIZ = 128;
      double DIRSIZ = 100;

      SpiceChar file[FILESIZ];
      SpiceChar filtyp[TYPESIZ];
      SpiceChar source[SOURCESIZ];

      SpiceBoolean found;

      double segStartEt;
      double segStopEt;

      kdata_c (0, "ck", FILESIZ, TYPESIZ, SOURCESIZ, file, filtyp, source, &handle, &found);
      dafbfs_c ( handle );
      daffna_c ( &found );
      int spCode = ( (int) (p_frameCode/1000) ) * 1000;

      while ( found ) {
        double sum[10];   // daf segment summary
        double dc[2];     // segment starting and ending times in tics
        SpiceInt ic[6];   // segment summary values:  
                          // instrument code for platform, 
                          // reference frame code, 
                          // data type, 
                          // velocity flag, 
                          // offset to quat 1, 
                          // offset to end.
        dafgs_c ( sum );
        dafus_c ( sum, (SpiceInt) 2, (SpiceInt) 6, dc, ic);

        // Don't read type 5 ck here
        if (ic[2] == 5) break;
        if (ic[2] != 3) {
          std::string msg = "Time fetching method only works on type 3 and 5 ck";
          throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
        }

        // Check times for type 3 ck segment if spacecraft matches
        if (ic[0] == spCode) {
          sct2e_c((int) spCode/1000, dc[0], &segStartEt);
          sct2e_c((int) spCode/1000, dc[1], &segStopEt);
          NaifStatus::CheckErrors();
          double et;

          // Get times for this segment
          if (observStart > segStartEt  &&  observStart < segStopEt) {

            if (observEnd > segStopEt) {
              std::string msg = "Observation crosses segment boundary--unable to interpolate pointing";
              throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
            }

            // Extract necessary header parameters
            int dovelocity = ic[3];
            int end = ic[5];
            double val[2];
            dafgda_c (handle, end-1, end, val);
            int nints = (int) val[0];
            int ninstances = (int) val[1];
            int numvel  =  dovelocity * 3;
            int quatnoff  =  ic[4] + (4 + numvel) * ninstances - 1;
            int nrdir = (int) (( ninstances - 1 ) / DIRSIZ); /* sclkdp directory records */
            int sclkdp1off  =  quatnoff + 1;
            int sclkdpnoff  =  sclkdp1off + ninstances - 1;
            int start1off = sclkdpnoff + nrdir + 1;
            int startnoff = start1off + nints - 1;
            int sclkSpCode = spCode/1000;

            // Now get the times
            std::vector<double> sclkdp(ninstances);
            dafgda_c ( handle, sclkdp1off, sclkdpnoff, (SpiceDouble *) &sclkdp[0]);

            // Check for a gap in the time coverage by making sure the time span of the observation does not
            // cross an interpolation interval boundary 
            std::vector<double> starts(nints);
            dafgda_c ( handle, start1off, startnoff, (SpiceDouble *) &starts[0]);
            int interval = 0;
            sct2e_c (sclkSpCode, starts[interval], &et);

            // Find the start of the interval following the beginning of the observation
            while (interval < (nints-1)  &&  et < observStart) {
              interval++;
              sct2e_c ( sclkSpCode, starts[interval], &et);
            }

            // Make sure the observation has ended before the next interval start. The last interval was 
            // checked by the segment check above.  TODO Add a catch to get this error.
            if (interval < (nints-1)  &&  observEnd > et) {
                std::string msg = "Observation crosses interpolation interval boundary--unable to interpolate pointing";
                throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
            }

            int instance = 0;
            sct2e_c( sclkSpCode, sclkdp[0], &et);

            while (instance < (ninstances-1)  &&  et < observStart) {
              instance++;
              sct2e_c( sclkSpCode, sclkdp[instance], &et);
            }

            instance--;
            sct2e_c(sclkSpCode, sclkdp[instance], &et);

            while (et < observEnd) {
              p_cacheTime.push_back(et-p_timeBias);
              instance++;

              if (instance >= ninstances) {
                std::string msg = "Programmer error reading times from ck";
                throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
              }
              sct2e_c(sclkSpCode, sclkdp[instance], &et);
            }
            p_cacheTime.push_back(et-p_timeBias);
            timeLoaded = true;
            p_minimizeCache = Done;
          }
        }
        dafcs_c ( handle );  // Continue search in daf last searched
        daffna_c ( &found ); // Find next forward array in current daf
      }
    }
    else if (count == 0  &&  p_source != Nadir  &&  p_minimizeCache == Yes) {
      std::string msg = "No camera kernels loaded...Unable to determine time cache to downsize";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }

    // Load times according to cache size (body rotations) -- handle first round of type 5 ck case and multiple ck case -- 
    // Load a time for every line scan line and downsize later
    if (!timeLoaded) {
      double cacheSlope = 0.0;
      if (p_fullCacheSize > 1)
        cacheSlope = (p_fullCacheEndTime - p_fullCacheStartTime) / (double) (p_fullCacheSize - 1);
      for (int i=0; i<p_fullCacheSize; i++) 
        p_cacheTime.push_back(p_fullCacheStartTime + (double) i * cacheSlope);
      if (p_source == Nadir) p_minimizeCache = No;
    }
  }

  /** Return full listing (cache) of original time coverage
   * requested.
   */
  std::vector<double> SpiceRotation::GetFullCacheTime() {

    // No time cache was initialized -- throw an error
    if (p_fullCacheSize < 1) {
      std::string msg = "Time cache not available -- rerun spiceinit";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }

    std::vector<double> fullCacheTime;
    double cacheSlope = 0.0;
    if (p_fullCacheSize > 1)  cacheSlope = (p_fullCacheEndTime - p_fullCacheStartTime) / (double) (p_fullCacheSize - 1);

    for (int i=0; i<p_fullCacheSize; i++) 
      fullCacheTime.push_back(p_fullCacheStartTime + (double) i * cacheSlope);

    return fullCacheTime;
  }

}
