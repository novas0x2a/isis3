#include <algorithm>
#include <cfloat>

#include "SpicePosition.h"
#include "iException.h"
#include "LineEquation.h"
#include "BasisFunction.h"
#include "LeastSquares.h"
#include "PolynomialUnivariate.h"
#include "NaifStatus.h"

namespace Isis {
  /**
   * Construct an empty SpicePosition class using valid body codes.
   * See required reading
   * ftp://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/ascii/individual_docs/naif_ids.req
   *
   * @param targetCode Valid naif body name.
   * @param observerCode Valid naif body name.
   */
  SpicePosition::SpicePosition(int targetCode, int observerCode) {
    p_targetCode = targetCode;
    p_observerCode = observerCode;
    p_timeBias = 0.0;
    p_aberrationCorrection = "LT+S";
    p_cached = false;
    p_coordinate.resize(3);
    p_velocity.resize(3);
    p_et = -DBL_MAX;
    p_noOverride = true;
    p_hasVelocity = false;
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
  void SpicePosition::SetTimeBias (double timeBias) {
    p_timeBias = timeBias;
  }

  /** Set the aberration correction (light time).
   * See NAIF required reading for more information on this correction at
   * ftp://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/ascii/individual_docs/spk.req
   *
   * @param correction  This value must be one of the following: "NONE", "LT",
   * "LT+S", where LT is a correction for planetary aberration (light time) and
   * S a correction for stellar aberration.  If never called the default is
   * "LT+S".
   */
  void SpicePosition::SetAberrationCorrection(const std::string &correction) {
    if (correction == "NONE" || correction == "LT" || correction == "LT+S") {
      p_aberrationCorrection = correction;
    }
    else {
      std::string msg = "Invalid abberation correction [" + correction + "]";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  }

  /** Return J2000 coordinate at given time.
   *
   * This method returns the J2000 coordinates (x,y,z) of the body at a given
   * et in seconds.  The coordinates are obtained from either a valid NAIF spk
   * kernel, or alternatively from an internal cache loaded from an ISIS Table
   * object.  In the first case, the SPK kernel must contain positions for
   * the body code specified in the constructor at the given time and it must
   * be loaded using the SpiceKernel class.
   *
   * @param et   ephemeris time in seconds
   */
  const std::vector<double> &SpicePosition::SetEphemerisTime(double et) {
    NaifStatus::CheckErrors();

    // Save the time
    if (et == p_et) return p_coordinate;
    p_et = et;

    // Read from the cache
    if (p_cached) {
      // If the cache has only one position return it
      if (p_cache.size() == 1) {
        p_coordinate[0] = p_cache[0][0];
        p_coordinate[1] = p_cache[0][1];
        p_coordinate[2] = p_cache[0][2];

        if (p_hasVelocity) {
          p_velocity[0] = p_cacheVelocity[0][0];
          p_velocity[1] = p_cacheVelocity[0][1];
          p_velocity[2] = p_cacheVelocity[0][2];
        }
      }

      else {
        // Otherwise determine the interval to interpolate
        std::vector<double>::iterator pos;
        pos = upper_bound(p_cacheTime.begin(),p_cacheTime.end(),p_et);

        int cacheIndex;
        if (pos != p_cacheTime.end()) {
          cacheIndex = distance(p_cacheTime.begin(),pos);
          cacheIndex--;
        }
        else {
          cacheIndex = p_cacheTime.size() - 2;
        }

        if (cacheIndex < 0) cacheIndex = 0;

        // Interpolate the coordinate
        double mult = (p_et - p_cacheTime[cacheIndex]) /
                      (p_cacheTime[cacheIndex+1] - p_cacheTime[cacheIndex]);
        std::vector<double> p2 = p_cache[cacheIndex+1];
        std::vector<double> p1 = p_cache[cacheIndex];

        p_coordinate[0] = (p2[0] - p1[0]) * mult + p1[0];
        p_coordinate[1] = (p2[1] - p1[1]) * mult + p1[1];
        p_coordinate[2] = (p2[2] - p1[2]) * mult + p1[2];

        if (p_hasVelocity) {
          p2 = p_cacheVelocity[cacheIndex+1];
          p1 = p_cacheVelocity[cacheIndex];
          p_velocity[0] = (p2[0] - p1[0]) * mult + p1[0];
          p_velocity[1] = (p2[1] - p1[1]) * mult + p1[1];
          p_velocity[2] = (p2[2] - p1[2]) * mult + p1[2];
        }
      }
    }

    // Read from the kernel
    else {
      SpiceDouble j[6], lt;
      // First try getting the entire state (including the velocity vector)
      spkez_c ((SpiceInt)p_targetCode,
                (SpiceDouble)(p_et + p_timeBias),
                "J2000",
                p_aberrationCorrection.c_str(),
                (SpiceInt)p_observerCode,
                j,
                &lt);
      // If Naif fails attempting to get the entire state, assume the velocity vector is
      // not available and just get the position.  First turn off Naif error reporting and
      // return any error without printing them.
      SpiceBoolean spfailure = failed_c();
      reset_c();                   // Reset Naif error system to allow caller to recover
      if ( !spfailure ) {
        p_velocity[0] = j[3];
        p_velocity[1] = j[4];
        p_velocity[2] = j[5];
        p_hasVelocity = true;
      }
      else {
        spkezp_c ((SpiceInt)p_targetCode,
                (SpiceDouble)(p_et + p_timeBias),
                "J2000",
                p_aberrationCorrection.c_str(),
                (SpiceInt)p_observerCode,
                j,
                &lt);
      }
      p_coordinate[0] = j[0];
      p_coordinate[1] = j[1];
      p_coordinate[2] = j[2];
    }

    // Return the coordinate
    return p_coordinate;

    NaifStatus::CheckErrors();
    }

  /** Cache J2000 position over a time range.
   *
   * This method will load an internal cache with coordinates over a time
   * range.  This prevents
   * the NAIF kernels from being read over-and-over again and slowing a
   * application down due to I/O performance.  Once the
   * cache has been loaded then the kernels can be unloaded from the NAIF
   * system.
   *
   * @param startTime   Starting ephemeris time in seconds for the cache
   * @param endTime     Ending ephemeris time in seconds for the cache
   * @param size        Number of coordinates/positions to keep in the cache
   *
   */
  void SpicePosition::LoadCache (double startTime, double endTime, int size) {
    // Make sure cache isn't alread loaded
    if (p_cached) {
      std::string msg = "A SpicePosition cache has already been created";
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


    // Loop and load the cache
    double cacheSlope = 0.0;
    if (size > 1) cacheSlope = (endTime - startTime) / (double) (size - 1);

    for (int i=0; i<size; i++) {
      double et = startTime + (double) i * cacheSlope;
      SetEphemerisTime(et);
      p_cache.push_back(p_coordinate);
      if (p_hasVelocity) p_cacheVelocity.push_back(p_velocity);
      p_cacheTime.push_back(p_et);
    }
    p_cached = true;
  }

  /** Cache J2000 position for a time.
   *
   * This method will load an internal cache with coordinates for a single
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
  void SpicePosition::LoadCache (double time) {
    LoadCache(time,time,1);
  }

  /** Cache J2000 positions using a table file.
   *
   * This method will load an internal cache with coordinates from an
   * ISIS table file.  The table must have 4 columns, or 7 (if velocity)
   * is included, and at least one row. The 4 columns contain the
   * following information, body position x,y,z in J2000 and the
   * ephemeris time of that position. If there are multiple rows it is
   * assumed you can interpolate position at times in between the rows.
   *
   * @param table   An ISIS table blob containing valid J2000 coordinate/time
   *                values
   */
  void SpicePosition::LoadCache(Table &table) {
    // Make sure cache isn't alread loaded
    if (p_cached) {
      std::string msg = "A SpicePosition cache has already been created";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

    // Loop through and move the table to the cache
    for (int r=0; r<table.Records(); r++) {
      TableRecord &rec = table[r];
      if (rec.Fields() == 7) {
        p_hasVelocity = true;
      }
      else if (rec.Fields() == 4) {
        p_hasVelocity = false;
      }
      else  {
        std::string msg = "Expecting four or seven fields in the SpicePosition table";
        throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
      }

      std::vector<double> j2000Coord;
      j2000Coord.push_back((double)rec[0]);
      j2000Coord.push_back((double)rec[1]);
      j2000Coord.push_back((double)rec[2]);
      int inext = 3;

      p_cache.push_back(j2000Coord);
      if (p_hasVelocity) {
        std::vector<double> j2000Velocity;
        j2000Velocity.push_back((double)rec[3]);
        j2000Velocity.push_back((double)rec[4]);
        j2000Velocity.push_back((double)rec[5]);
        inext = 6;

        p_cacheVelocity.push_back(j2000Velocity);
      }
      p_cacheTime.push_back((double)rec[inext]);
    }

    p_cached = true;
  }

  /** Return a table with J2000 positions.
   *
   * Return a table containg the cached coordinates with the given name.  The
   * table will have four or seven columns, J2000 x,y,z (optionally vx,vy,vx)
   * and the ephemeris time.
   *
   * @param tableName    Name of the table to create and return
   */
  Table SpicePosition::Cache(const std::string &tableName) {
    TableField x("J2000X",TableField::Double);
    TableField y("J2000Y",TableField::Double);
    TableField z("J2000Z",TableField::Double);
    TableRecord record;
    record += x;
    record += y;
    record += z;

    if ( p_hasVelocity ) {
      TableField vx("J2000XV",TableField::Double);
      TableField vy("J2000YV",TableField::Double);
      TableField vz("J2000ZV",TableField::Double);
      record += vx;
      record += vy;
      record += vz;
    }
    TableField t("ET",TableField::Double);
    record += t;

    Table table(tableName,record);
    int inext=0;

    for (int i=0; i<(int)p_cache.size(); i++) {
      record[inext++] = p_cache[i][0];
      record[inext++] = p_cache[i][1];
      record[inext++] = p_cache[i][2];
      if (p_hasVelocity) {
        record[inext++] = p_cacheVelocity[i][0];
        record[inext++] = p_cacheVelocity[i][1];
        record[inext++] = p_cacheVelocity[i][2];
      }
      record[inext] = p_cacheTime[i];
      table += record;

      inext = 0;

    }

    return table;
  }



  /** Cache J2000 rotation over existing cached time range using polynomials
   *
   * This method will reload an internal cache with positions
   * formed from coordinates fit to polynomials over a time
   * range.
   *
   * @param function1   The first polynomial function used to
   *                    find the position coordinates
   * @param function2   The second polynomial function used to
   *                    find the position coordinates
   * @param function3   The third polynomial function used to
   *                    find the position coordinates
   */
  void SpicePosition::ReloadCache (Isis::PolynomialUnivariate &function1,
                                 Isis::PolynomialUnivariate &function2,
                                 Isis::PolynomialUnivariate &function3){
   // Make sure cache is already loaded
    if ( !p_cached ) {
      std::string msg = "A SpicePosition cache has not been created yet";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }

   // Clear existing coordinates from cache
    p_cache.clear();


    // Set velocity vector to false since it is not being updated
    p_hasVelocity = false;

//    std::cout <<"time cache size is " << p_cacheTime.size() << std::endl;



    // Calculate new postition coordinates from polynomials fit to coordinates & 
    // fill cache
//    std::cout << "Before" << std::endl;
//    double savetime = p_cacheTime.at(0);
//    SetEphemerisTime(savetime);
//    std::cout << "     at time " << savetime << std::endl;
//    for (int i=0; i<3; i++) {
//      std::cout << p_coordinate[i] << std::endl;
//    }

    for (std::vector<double>::size_type pos=0;pos < p_cacheTime.size();pos++) {
      double et = p_cacheTime.at(pos);
      std::vector<double> rtime;
      rtime.push_back(et - p_baseTime);
      p_coordinate[0] = function1.Evaluate (rtime);
      p_coordinate[1] = function2.Evaluate (rtime);
      p_coordinate[2]  = function3.Evaluate (rtime);

      p_cache.push_back( p_coordinate );
    }

    double et = p_et;
    p_et = -DBL_MAX;
    SetEphemerisTime(et);

/*    std::cout << "After" << std::endl;
    std::cout << "     at time " << et << std::endl;
    for (int i=0; i<3; i++) {
      std::cout << p_coordinate[i] << std::endl;
    }*/
    return;
  }



  /** Set the coefficients of a polynomial (parabola) fit to each
   * of the components (X, Y, Z) of the position vector for the time period covered
   * by the cache, component = a + bt + ct**2, where t = time - p_baseTime.
   *
   */
  void SpicePosition::SetPolynomial () {
    int degree=2;
    Isis::PolynomialUnivariate function1(degree);       //!< Basis function fit to X
    Isis::PolynomialUnivariate function2(degree);       //!< Basis function fit to Y
    Isis::PolynomialUnivariate function3(degree);       //!< Basis function fit to Z
                                    //
    LeastSquares *fitX = new LeastSquares ( function1 );
    LeastSquares *fitY = new LeastSquares ( function2 );
    LeastSquares *fitZ = new LeastSquares ( function3 );

    // Compute the base time
    ComputeBaseTime ();
    std::vector<double> time;
    std::vector<double> XC,YC,ZC;

    if (p_cache.size() == 1) {
      double t = p_cacheTime.at(0);
      SetEphemerisTime( t );
      XC.push_back(p_coordinate[0]);
      XC.push_back(0.0);
      XC.push_back(0.0);
      YC.push_back(p_coordinate[1]);
      YC.push_back(0.0);
      YC.push_back(0.0);
      ZC.push_back(p_coordinate[2]);
      ZC.push_back(0.0);
      ZC.push_back(0.0);
    }
    else if (p_cache.size() == 2) {
// Load the times and get the corresponding rotation angles
      double t1 = p_cacheTime.at(0);
      SetEphemerisTime( t1 );
      std::vector<double> coord1 = p_coordinate;
      t1 -= p_baseTime;
      double t2 = p_cacheTime.at(1);
      SetEphemerisTime( t2 );
      std::vector<double> coord2 = p_coordinate;
      t2 -= p_baseTime;
      double slope[3];
      double intercept[3];

// Compute the linear equation for each coordinate and save them
      for (int cIndex=0; cIndex < 3; cIndex++) {
        Isis::LineEquation posline(t1, coord1[cIndex], t2, coord2[cIndex]);
        slope[cIndex] = posline.Slope();
        intercept[cIndex] = posline.Intercept();
      }
      XC.push_back(intercept[0]);
      XC.push_back(slope[0]);
      XC.push_back(0.0);
      YC.push_back(intercept[1]);
      YC.push_back(slope[1]);
      YC.push_back(0.0);
      ZC.push_back(intercept[2]);
      ZC.push_back(slope[2]);
      ZC.push_back(0.0);
    }
    else {
    // Load the known values to compute the fit equation

      for (std::vector<double>::size_type pos=0;pos < p_cacheTime.size();pos++) {
        double t = p_cacheTime.at(pos);
        time.push_back( t - p_baseTime );
        SetEphemerisTime( t );
        std::vector<double> coord = p_coordinate;

        fitX->AddKnown ( time, coord[0] );
        fitY->AddKnown ( time, coord[1] );
        fitZ->AddKnown ( time, coord[2] );
        time.clear();
      }
      //Solve the equations for the coefficients
      fitX->Solve();
      fitY->Solve();
      fitZ->Solve();

      // Delete the least squares objects now that we have all the coefficients
      delete fitX;
      delete fitY;
      delete fitZ;

      // For now assume all three coordinates are fit to a parabola.  Later they may
      // each be fit to a unique basis function.
      // Fill the coefficient vectors

      for ( int i = 0;  i < function1.Coefficients(); i++) {
        XC.push_back( function1.Coefficient( i ) );
        YC.push_back( function2.Coefficient( i ) );
        ZC.push_back( function3.Coefficient( i ) );
      }

    }

    // Now that the coefficients have been calculated set the polynomial with them
    SetPolynomial ( XC, YC, ZC );
    return;
  }



  /** Set the coefficients of a polynomial (parabola) fit to
   * each of the three coordinates of the position vector for the
   * time period covered by the cache, coord = a + bt + ct**2,
   * where t = time - p_baseTime.
   *
   * @param [in] XC Coefficients of fit to X coordinate
   * @param [in] YC Coefficients of fit to Y coordinate
   * @param [in] ZC Coefficients of fit to Z coordinate
   *
   */
  void SpicePosition::SetPolynomial ( const std::vector<double>& XC,
                                      const std::vector<double>& YC,
                                      const std::vector<double>& ZC ) {
    Isis::PolynomialUnivariate function1( 2 );
    Isis::PolynomialUnivariate function2( 2 );
    Isis::PolynomialUnivariate function3( 2 );

    // Load the functions with the coefficients
    function1.SetCoefficients ( XC );
    function2.SetCoefficients ( YC );
    function3.SetCoefficients ( ZC );

    // Compute the base time
    ComputeBaseTime ();


//    std::cout << "Basetime=" << p_baseTime << std::endl;

    // Reload the cache from the functions and the currently cached time
    ReloadCache ( function1, function2, function3 );

    // Save the current coefficients
    p_coefficients[0] = XC;

//    std::cout << "Saved coef0="<<p_coefficients[0][0]<<" "<<p_coefficients[0][1]
//              <<p_coefficients[0][2]<<std::endl;

    p_coefficients[1] = YC;

//    std::cout << "Saved coef1="<<p_coefficients[1][0]<<" "<<p_coefficients[1][1]
//              <<p_coefficients[1][2]<<std::endl;

    p_coefficients[2] = ZC;
    return;
  }



  /**
   *  Return the coefficients of a polynomial (parabola) fit to each of the
   *  three coordinates of the position for the time period covered by the cache,
   *  angle = a + bt + ct**2, where t = time - p_basetime.
   *
   * @param [out] XC Coefficients of fit to first coordinate of position
   * @param [out] YC Coefficients of fit to second coordinate of position
   * @param [out] ZC Coefficients of fit to third coordinate of position
   *
   */
  void SpicePosition::GetPolynomial ( std::vector<double>& XC,
                                      std::vector<double>& YC,
                                      std::vector<double>& ZC ) {
    XC = p_coefficients[0];
    YC = p_coefficients[1];
    ZC = p_coefficients[2];

    return;
  }



  //! Compute the base time using cached times
  void SpicePosition::ComputeBaseTime () {
    if (p_noOverride) {
      p_baseTime = (p_cacheTime.at(0) + p_cacheTime.at(p_cacheTime.size()-1))/ 2.;
    }
    else {
      p_baseTime = p_overrideBaseTime;
    }
    return;
  }


  /**
   * Set an override base time to be used with observations on scanners to allow all
   * images in an observation to use the save base time and polynomials for the positions.
   * 
   * @param [in] baseTime The baseTime to use and override the computed base time
   */
  void SpicePosition::SetOverrideBaseTime( double baseTime ) {
    p_overrideBaseTime = baseTime;
    p_noOverride = false;
    return;
  }



  /** Set the coefficients of a polynomial (parabola) fit to each of the
   *  three coordinates of the position vector for the time period covered by the cache,
   *  coordinate = A + B*t + C*t**2, where t = time - p_basetime.
   *
   * @param partialVar     Variable output derivative vector is to be with respect to
   * @return               Derivative of j2000 vector calculated with polynomial 
   *                              with respect to partialVar
   *
   */
  std::vector<double> SpicePosition::CoordinatePartial( SpicePosition::PartialType partialVar) {
    // Start with a zero vector since the derivative of the other coordinates with
    // respect to the partial var will be 0.
    std::vector<double> coordinate(3,0);

    // Get the index of the coordinate to update with the partial derivative
    int coordIndex = partialVar / 3;

    // Get the coefficient being solved for
    int coef = partialVar%3;

    // Reset the coordinate to its derivative
    coordinate[coordIndex] = DPolynomial ( (Coefficient)  coef );
    return coordinate;
  }


  /**
   *  Evaluate the derivative of the fit polynomial (parabola) defined by the
   *  given coefficients with respect to the coefficient at the given index, at
   *  the current time.
   *
   * @param coeffIndex Index of coefficient to differentiate with respect
   *                         to
   * @return The derivative evaluated at the current time
   *
   */
  double SpicePosition::DPolynomial ( const Coefficient coeffIndex ) {
    double derivative=1.;
    double time = p_et - p_baseTime;

//    std::cout << "coeff index = " << coeffIndex << std::endl;

    switch (coeffIndex) {
    case C:
      derivative = time;
    case B:
      derivative *= time;
    case A:
      derivative *= 1.;
      break;
    }
    return derivative;
  }

  /** Return the velocity vector if available.
   *
   * @return The velocity vector evaluated at the current time
   */
  const std::vector<double> &SpicePosition::Velocity() {
    if (p_hasVelocity) {
      return p_velocity;
    }
    else {
      std::string msg = "No velocity vector available";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    }
  }
}
