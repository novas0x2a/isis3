#include <string>
#include <algorithm>
#include <vector>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <iomanip>

#include "PixelOffset.h"
#include "TextFile.h"
#include "LineEquation.h"
#include "LeastSquares.h"
#include "BasisFunction.h"
#include "PolynomialUnivariate.h"
#include "iString.h"
#include "iException.h"


namespace Isis {
  /**
   * Construct a Pixel Offset class loading the offsets in the input file into
   * the offset caches.
   *
   * @param tableList Ascii table list of sample, line offsets and their corresponding time.
   * @param fl Focal length of instrument in mm.
   * @param pixpitch Pixel pitch of instrument in mm/pixel.
   */
  PixelOffset::PixelOffset( const std::string &tableList, double fl, double pixPitch, double baseTime, 
                            double timeScale, int degree ) {
    p_focalLen = fl;
    p_pixPitch = pixPitch;
//    p_RJ.resize(9);
    p_et = -DBL_MAX;
    p_degree = degree;
    p_baseTime = baseTime;
    p_timeScale = timeScale;
    p_angleScale = p_pixPitch/p_focalLen;
    std::vector<std::string> lines;
    Isis::TextFile inputFile(tableList, "input", lines);
    std::vector<std::string> fields;
    p_I.push_back(1.0);
    p_I.push_back(0.0);
    p_I.push_back(0.0);
    p_I.push_back(0.0);
    p_I.push_back(1.0);
    p_I.push_back(0.0);
    p_I.push_back(0.0);
    p_I.push_back(0.0);
    p_I.push_back(1.0);

    for (size_t iline=0; iline<lines.size(); iline++) {
      int num = iString::Split( ' ', iString::ConvertWhiteSpace(lines[iline]), fields, false);
      if (num != 3) {
        Isis::iString msg = "Three fields are required:  sample, line, and ephemeris time.";
        throw Isis::iException::Message(Isis::iException::Io,msg,_FILEINFO_);
      }
      p_samples.push_back(iString::ToDouble(fields[0]));
      p_lines.push_back(iString::ToDouble(fields[1]));
      p_times.push_back(iString::ToDouble(fields[2]));
    }

    // Compute scalers   test1
    double sMin=p_samples[0];
    double sMax=p_samples[0];
    double lMin=p_lines[0];
    double lMax=p_lines[0];

    for (int i=1; i<(int) p_samples.size(); i++) {
      if (p_samples[i]<sMin) sMin = p_samples[i];
      if (p_samples[i]>sMax) sMax = p_samples[i];
      if (p_lines[i]<lMin) lMin = p_lines[i];
      if (p_lines[i]>lMax) lMax = p_lines[i];
    }
    p_sampScale = (sMax - sMin)/2.;
    p_sampOff = sMax - p_sampScale;
    p_lineScale = (lMax - lMin)/2.;
    p_lineOff = lMax - p_lineScale;
  }

  /** Compute the angular equivalents for the offsets at a given time.
   *
   * This method computes the angular equivalents in radians for the offsets 
   * at a given et in seconds.  The pixel offsets are interpolated from the
   * offsets input in the table using a cubic interpolation and converted to
   * angles based on the focal length and the pixel pitch.
   *
   * @param et   ephemeris time in seconds
   */
  void PixelOffset::SetEphemerisTime(double et) {
    // Save the time
    if (p_et == et) return;
    p_et = et;

    // Otherwise determine the interval to interpolate
    std::vector<double>::iterator pos;
    pos = upper_bound(p_times.begin(),p_times.end(),p_et); // returns the nearest endpoint if outside range
    
    int index=0;
    double soc0, soc1, soc2, soc3, loc0, loc1, loc2, loc3;
    soc0=soc1=soc2=soc3=loc0=loc1=loc2=loc3=0.;
    if (pos != p_times.end() && pos > (p_times.begin()+1) ) {
      index = distance(p_times.begin(),pos);
      index--;  //lower bound
      soc3 = p_samples[index+2] - p_samples[index+1] - p_samples[index-1] + p_samples[index];
      soc2 = p_samples[index-1] - p_samples[index] - soc3;
      soc1 = p_samples[index+1] - p_samples[index-1];
      soc0 = p_samples[index];
      loc3 = p_lines[index+2] - p_lines[index+1] - p_lines[index-1] + p_lines[index];
      loc2 = p_lines[index-1] - p_lines[index] - loc3;
      loc1 = p_lines[index+1] - p_lines[index-1];
      loc0 = p_lines[index];
//    } else if (pos == p_times.begin()+1) {
    } else if (pos <= p_times.begin()+1) { // Extrapolates
      // Since values for index cacheIndex-1 do not exist, make up one based on slope from segment
      // at index to index+1.  Replace p_samps[index-1] with 
      // p_samps[index] - (p_samps[index+1 - p_samps[index] )
//      index = distance(p_times.begin(),pos);
//      index--;
      index = 0;
/*      soc3 = p_samples[index+2] - p_samples[index];
      soc2 = p_samples[index] - p_samples[index+1] - soc3;
      soc1 = 2.*(p_samples[index+1] - p_samples[index]);
      soc0 = p_samples[index];
      loc3 = p_lines[index+2] - p_lines[index];
      loc2 = p_lines[index] - p_lines[index+1] - loc3;
      loc1 = 2.*(p_lines[index+1] - p_lines[index]);
      loc0 = p_lines[index];*/
      soc3 = p_samples[2] - p_samples[0];
      soc2 = 2*p_samples[0] - p_samples[1] - p_samples[2];
      soc1 = 2.*(p_samples[1] - p_samples[0]);
      soc0 = p_samples[0];
      loc3 = p_lines[2] - p_lines[0];
      loc2 = 2.*p_lines[0] - p_lines[1] - p_lines[2];
      loc1 = 2.*(p_lines[1] - p_lines[0]);
      loc0 = p_lines[0];
    }
/*    else if (pos == p_times.end()) {
    // Since values for index cacheIndex+2 do not exist, make up one based on slope from segment
    // at index to index+1.  Replace p_samps[index+2] with 
    // p_samps[index] - (p_samps[index+1 - p_samps[index] )
    index = distance(p_times.begin(),pos);
    index--;
    soc3 = 3.*p_samples[index] - 2.*p_samples[index+1] - p_samples[index-1];
    soc2 = p_samples[index-1] - p_samples[index] - soc3;
    soc1 = p_samples[index+1] - p_samples[index-1];
    soc0 = p_samples[index];
    loc3 = p_lines[index+2] - p_lines[index+1] - p_lines[index-1] + p_lines[index];
    loc3 = 3.*p_lines[index] - 2.*p_lines[index+1] - p_lines[index-1];
    loc2 = p_lines[index-1] - p_lines[index] - loc3;
    loc1 = p_lines[index+1] - p_lines[index-1];
    loc0 = p_lines[index];
    }*/
    else if (pos == p_times.end()) {
    // Since values for index cacheIndex+2 do not exist, make up one based on slope from segment
    // at index to index+1.  Replace p_samps[index+2] with 
    // p_samps[index] - (p_samps[index+1 - p_samps[index] )
    index = p_times.size() - 2;
    soc3 = p_samples[index+1] - p_samples[index-1];
    soc2 = 2.*p_samples[index-1] - p_samples[index] - p_samples[index+1];
    soc1 = p_samples[index+1] - p_samples[index-1];
    soc0 = p_samples[index];
    loc3 = p_lines[index+1] - p_lines[index-1];
    loc2 = 2.*p_lines[index-1] - p_lines[index] - p_lines[index+1];
    loc1 = p_lines[index+1] - p_lines[index-1];
    loc0 = p_lines[index];
    }
/*    else if (pos == p_times.begin()) {
      index = 0;
      soc3 = soc2 = soc1 = 0.;
      soc0 = p_samples[index];
      loc3 = loc2 = loc1 = 0.;
      loc0 = p_lines[index];
    }*/
//    else if (WHAT DOES upper_bound DO IF VALUE IS GREATER THAN TOP OF INTERVAL?????) {
//    }
    if (index < 0) index = 0;

// Interpolate the offsets
    double mult = (p_et - p_times[index]) /
      (p_times[index+1] - p_times[index]);
    double mult2 = mult*mult;
    double sampleOff = soc3*mult*mult2 + soc2*mult2 + soc1*mult + soc0;
    double lineOff = loc3*mult*mult2 + loc2*mult2 + loc1*mult + loc0;
//
//    std::cout<<"    "<<sampleOff<<"   "<<lineOff<<"   "<<p_et<<std::endl;
//
    // Convert pixel offsets to angles
    p_angle1 = -sampleOff;
    p_angle2 = -lineOff;
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
  void PixelOffset::LoadAngles ( std::vector<double> cacheTime ) {
    p_cacheTime = cacheTime;
    // Make sure angles aren't already loaded TBD
/*    if () {
      std::string msg = "An angle cache has already been created";
      throw Isis::iException::Message(Isis::iException::Programmer,msg,_FILEINFO_);
    } */

    // Loop and load the angle caches
    for (size_t i=0; i<p_cacheTime.size(); i++) {
      double et = p_cacheTime[i];
      SetEphemerisTime(et);
      p_cacheAngle1.push_back( (p_angle1 - p_sampOff)/p_sampScale );
      p_cacheAngle2.push_back( (p_angle2 - p_lineOff)/p_lineScale);

//      std::cout<<p_angle1<<"     "<<p_angle2<<"     "<<et <<std::endl;
    }
  }



  /** Set the coefficients of a polynomial fit to each
   * of the three camera angles for the time period covered by the
   * cache, angle = a + bt + ct**2, where t = (time - p_baseTime)/ p_timeScale.
   *
   */
  void PixelOffset::SetPolynomial () {
    Isis::PolynomialUnivariate function1(p_degree);       //!< Basis function fit to 1st rotation angle
    Isis::PolynomialUnivariate function2(p_degree);       //!< Basis function fit to 2nd rotation angle
                                    //
    LeastSquares *fitAng1 = new LeastSquares ( function1 );
    LeastSquares *fitAng2 = new LeastSquares ( function2 );

    std::vector<double> time;

    // Load the known values to compute the fit equation
    for (std::vector<double>::size_type pos=0;pos < p_cacheTime.size();pos++) {
      double t = p_cacheTime.at(pos);

// Base fit on extent of coverage in the input offset file
      if (t >= p_times[0] && t <= p_times[p_times.size()-1]) {
        time.push_back( (t - p_baseTime) / p_timeScale );
        SetEphemerisTime( t );
        fitAng1->AddKnown ( time, p_cacheAngle1[pos] );
        fitAng2->AddKnown ( time, p_cacheAngle2[pos] );
        time.clear();
      }
    }

    if (fitAng1->Knowns() == 0) {
      std::string msg;
      msg = "Cube time range is not covered by jitter file";
      throw iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }

    //Solve the equations for the coefficients
    fitAng1->Solve();
    fitAng2->Solve();

    // Delete the least squares objects now that we have all the coefficients
    delete fitAng1;
    delete fitAng2;

    // For now assume both angles are fit to a polynomial.  Later they may
    // each be fit to a unique basis function.
    // Fill the coefficient vectors

    for ( int i = 0;  i < function1.Coefficients(); i++) {
      p_ang1Coefficients.push_back( function1.Coefficient( i ) );
      p_ang2Coefficients.push_back( function2.Coefficient( i ) );
    }

//    std::cout<<"Angle1 coeff="<<p_ang1Coefficients[0]<<" "<<p_ang1Coefficients[1]<<" "<<p_ang1Coefficients[2]<<std::endl;
//    std::cout<<"Angle2 coeff="<<p_ang2Coefficients[0]<<" "<<p_ang2Coefficients[1]<<" "<<p_ang2Coefficients[2]<<std::endl;

    }



  /** Set ephemeris time for the high pass filtered rotation
   * from the instrument frame to the true (corrected) instrument
   * frame.  [TC] = [angle2 - Pangle2(t)]  [angle1 - Pangle1(t)]
   *                                  2                   1
   * where t = (time - p_baseTime) / p_timeScale, and n = p_degree.
   *
   * @param [in] et Ephemeris time
   *
   */
  std::vector<double> PixelOffset::SetEphemerisTimeHPF ( const double et ) {

    Isis::PolynomialUnivariate function1( p_degree );
    Isis::PolynomialUnivariate function2( p_degree );

    //If outside the range of the offsets just return the identity matrix
    if (et < p_times[0] || et > p_times[p_times.size()-1]) {
      return (p_I);
    }

    // Load the functions with the coefficients
    function1.SetCoefficients ( p_ang1Coefficients );
    function2.SetCoefficients ( p_ang2Coefficients );

    // Compute polynomial approximations to angles, pangle1 and pangle2
    std::vector<double> rtime;
    rtime.push_back((et - p_baseTime) / p_timeScale);
    double pangle1 = p_sampOff + p_sampScale*function1.Evaluate (rtime);
    double pangle2 = p_lineOff + p_lineScale*function2.Evaluate (rtime);

    // Compute p_angles for this time
    SetEphemerisTime(et);
    double angle1 = p_angleScale*(p_angle1 - pangle1);
    double angle2 = p_angleScale*(p_angle2 - pangle2);


//    std::cout<<"  "<<p_angle1*p_angleScale<<"  "<<pangle1*p_angleScale<<"  "<<angle1<<"  "<<et<<std::endl;
//   std::cout<<"  "<<p_angle2*p_angleScale<<"  "<<pangle2*p_angleScale<<"  "<<angle2<<"  "<<et<<std::endl;

    std::vector<double> TC(9);

    eul2m_c ( (SpiceDouble) 0., (SpiceDouble) angle2, (SpiceDouble) angle1,
                             3,                    2,                    1,
               (SpiceDouble(*) [3]) &TC[0]);

    return (TC);
  }



  /** Wrap the input angle to keep it within 2pi radians of the
   *  angle to compare.
   *
   * @param [in]  compareAngle
   * @param [in]  angle Angle to be wrapped if needed
   * @return double Wrapped angle
   *
   */
  double PixelOffset::WrapAngle(double compareAngle, double angle) {
    double diff1 = compareAngle - angle;

    if ( diff1 < -1*pi_c() ){
      angle -= twopi_c();
    } 
    else if (diff1 > pi_c() ) {
      angle += twopi_c();
    }
    return angle;
  }

}
