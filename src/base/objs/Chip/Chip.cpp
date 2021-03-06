/**                                                                       
 * @file                                                                  
 * $Revision: 1.13 $                                                             
 * $Date: 2010/02/02 15:04:39 $                                                                 
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
#include <algorithm>
#include "Chip.h"
#include "Cube.h"
#include "iException.h"
#include "Projection.h"
#include "Camera.h"
#include "Portal.h"
#include "Interpolator.h"
#include "LineManager.h"
#include "Statistics.h"
#include "PolygonTools.h"
#include "geos/geom/Point.h"
#include "tnt/tnt_array2d_utils.h"

namespace Isis {

  /**
   * Constructs a Chip. The default size is 3x3
   */
  Chip::Chip () {
    Init(3,3);
  }


  /**
   * @brief Single value assignment operator
   *  
   * Sets the entire chip to a constant 
   * 
   * @param d Value to set the chip to
   * 
   * @return Chip& Reference to this chip
   */
  void Chip::SetAllValues(const double &d) {
    for (unsigned int i = 0 ; i < p_buf.size() ; i++) {
       fill(p_buf[i].begin(), p_buf[i].end(), d);
    }
  }

  /**
   * Construct a Chip with specified dimensions
   * 
   * @param samples   number of samples in the chip
   * @param lines     number of lines in the chip
   */
  Chip::Chip (const int samples, const int lines) {
    Init(samples,lines);
  }


  //! Destroys the Chip object
  Chip::~Chip() {
    if (p_clipPolygon != NULL) delete p_clipPolygon;
  }


  //! Common initialization used by constructors
  void Chip::Init (const int samples, const int lines) {
    SetSize(samples,lines);
    SetValidRange();
    p_clipPolygon = NULL;
  }


  /**
   * Change the size of the Chip
   * 
   * @param samples   number of samples in the chip
   * @param lines     number of lines in the chip
   */
  void Chip::SetSize(const int samples, const int lines) {
    if(samples <= 0.0 || lines <= 0.0) {
      std::string msg = "samples and lines must be greater than zero.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    p_chipSamples = samples;
    p_chipLines = lines;
    p_buf.clear();
    p_buf.resize(lines);
    for (int i=0; i<lines; i++) {
      p_buf[i].resize(samples);
    }
    p_affine.Identity();
    p_tackSample = ((samples - 1) / 2) + 1;
    p_tackLine = ((lines - 1) / 2) + 1;
  }

  bool Chip::IsInsideChip(double sample, double line) {    
    double minSamp = p_cubeTackSample - ((p_chipSamples - 1) / 2); 
    double maxSamp = p_cubeTackSample + ((p_chipSamples - 1) / 2);  
    
    double minLine = p_cubeTackLine - ((p_chipLines - 1) / 2); 
    double maxLine = p_cubeTackLine + ((p_chipLines - 1) / 2); 

    if (sample < minSamp || sample > maxSamp) return false;
    if (line < minLine || line > maxLine) return false;
    return true;
  }


  /**
   * This sets which cube position will be located at the chip
   * tack position
   * 
   * @param cubeSample    the cube sample value to tack
   * @param cubeLine      the cube line value to tack
   */
  void Chip::TackCube (const double cubeSample, const double cubeLine) {
    p_cubeTackSample = cubeSample;
    p_cubeTackLine = cubeLine;
    p_affine.Identity();
    p_affine.Translate(p_cubeTackSample,p_cubeTackLine);
  }


  /**
   * Load cube data into the Chip. The data will be loaded such that the
   * position set using TackCube method will be put at the center of the
   * chip.  The data will be loaded to sub-pixel accuracy using a
   * cubic convolution interpolator.
   * 
   * @param cube      The cube used to put data into the chip
   * @param rotation  rotation in degrees of data about the 
   *                  cube tack point (default of 0)
   * @param scale     scale factor
   * @param band      Band number to use when loading
   */
  void Chip::Load(Cube &cube, const double rotation, const double scale,
                  const int band) {
    // Initialize our affine transform
    p_affine.Identity();

    // We want an affine which translates from chip to cube.  Note
    // that we want to give adjusted chip coordinates such that
    // (0,0) is at the chip tack point and maps to the cube tack point.
    p_affine.Scale(scale);
    p_affine.Rotate(rotation);
    p_affine.Translate(p_cubeTackSample,p_cubeTackLine);

    // Now go read the data from the cube into the chip
    Read(cube,band);

    // Store off the cube address in case someone wants to match
    // this chip
    p_filename = cube.Filename();
  }

  /**
   * @brief Load a chip using an Affine transform as provided by caller 
   *  
   * This method will load data from a cube using an established Affine 
   * transform as provided by the caller.  It is up to the caller to set up the 
   * affine appropriately. 
   *  
   * For example, the first thing this method will do is set the chip tack point 
   * to the transformed cube location by replacing the existing affine transform 
   * with the one passed in and then calling SetChipPosition providing the chip 
   * tack point as the argument.  This establishes which cube pixel is located 
   * at the chip tack point. 
   * 
   * @param cube Cube to load the data from 
   * @param affine Affine transform to set for chip load/operations
   * @param band Band number to read data from
   */
  void Chip::Load(Cube &cube, const Affine &affine, const bool &keepPoly,
                  const int band) {

    //  Set the tackpoint center to the cube location
    setTransform(affine);
    SetChipPosition(TackSample(), TackLine());

    //  Remove the clipping polygon if requested
    if (!keepPoly) {
      delete p_clipPolygon;
      p_clipPolygon = 0;
    }

    // Now go read the data from the cube into the chip
    Read(cube,band);

    // Store off the cube address in case someone wants to match
    // this chip
    p_filename = cube.Filename();
  }


  /**
   * Loads cube data into the Chip. The data will be loaded such that the
   * position set using TackCube method will be put at the center of the
   * chip.  The data will be loaded to sub-pixel accuracy using a
   * cubic convolution interpolator.  Additionally, the data will be loaded
   * such that it matches the camera and/or projective geometry of a
   * given Chip.
   * 
   * @param cube      The cube used to put data into the chip
   * @param match     Match the geometry of this chip
   * @param scale     scale factor
   * @param band      Band number to use when loading
   * 
   * @throws Isis::iException::Programmer - Match chip does not have an
   *                                        associated cube
   * @throws Isis::iException::Programmer - Match chip cube is not a camera or
   *                                        map projection
   */
  void Chip::Load (Cube &cube, Chip &match, Cube &matchChipCube, const double scale, const int band) {
    // See if the match cube has a camera or projection
    Camera *matchCam = NULL;
    Projection *matchProj = NULL;
    try {
      matchCam = matchChipCube.Camera();
    }
    catch (iException &error) {
      try {
        matchProj = matchChipCube.Projection();
        error.Clear();
      }
      catch (iException &error) {
        std::string msg = "Can not geom chip, ";
        msg += "Match chip cube [" + matchChipCube.Filename();
        msg += "] is not a camera or map projection";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }
    }

    // See if the cube we are loading has a camera/projection
    Camera *cam = NULL;
    Projection *proj = NULL;
    try {
      cam = cube.Camera();
    }
    catch (iException &error) {
      try {
        proj = cube.Projection();
        error.Clear();
      }
      catch (iException &error) {
        std:: string msg = "Can not geom chip, ";
        msg += "chip cube [" + matchChipCube.Filename();
        msg += "] is not a camera or map projection";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }
    }

    // Ok we can attempt to create an affine transformation that
    // maps our chip to the match chip.  We will need a set of control
    // points so we can fit the affine transform.
    std::vector<double> x,y,xp,yp;

    // Start by looping around the corners of our chip to get these
    // control points
    int startSamp = 1;
    int startLine = 1;    
    int endSamp = Samples()-1;
    int endLine = Lines()-1;
    while (x.size() < 3) {
      for (int chipSamp=startSamp; chipSamp<=endSamp+1; chipSamp+=endSamp) {
        for (int chipLine=startLine; chipLine<=endLine+1; chipLine+=endLine) {
          // Determine the offset from the tack point in our chip
          // to one of the four corners
          int sampOffset = chipSamp - TackSample();
          int lineOffset = chipLine - TackLine();

          // Use this offset to compute a chip position in the match
          // chip
          double matchChipSamp = match.TackSample() + sampOffset;
          double matchChipLine = match.TackLine() + lineOffset;

          // Now get the lat/lon at that chip position
          match.SetChipPosition(matchChipSamp,matchChipLine);
          double lat,lon;
          if (matchCam != NULL) {
            matchCam->SetImage(match.CubeSample(),match.CubeLine());
            if (!matchCam->HasSurfaceIntersection()) continue;
            lat = matchCam->UniversalLatitude();
            lon = matchCam->UniversalLongitude();
          }
          else {
            matchProj->SetWorld(match.CubeSample(),match.CubeLine());
            if (!matchProj->IsGood()) continue;
            lat = matchProj->UniversalLatitude();
            lon = matchProj->UniversalLongitude();
          }

          // Now use that lat/lon to find a line/sample in our chip
          double line,samp;
          if (cam != NULL) {
            cam->SetUniversalGround(lat,lon);
            if (!cam->HasSurfaceIntersection()) continue;
            samp = cam->Sample();
            line = cam->Line();
          }
          else {
            proj->SetUniversalGround(lat,lon);
            if (!proj->IsGood()) continue;
            samp = proj->WorldX();
            line = proj->WorldY();
          }

     //     if (line < 1 || line > cube.Lines()) continue;
     //     if (samp < 1 || samp > cube.Samples()) continue;

          // Ok save this control point
          x.push_back(sampOffset);
          y.push_back(lineOffset);
          xp.push_back(samp);
          yp.push_back(line);

        }
      }
      int sinc = (endSamp - startSamp) / 4;
      // Ensures that the inc can cause start and end to cross
      if (sinc < 1) {
        sinc = 1;
      }
      int linc = (endLine - startLine) / 3;
      // Ensures that the inc can cause start and end to cross
      if (linc < 1) {
        linc = 1;
      }
      startSamp += sinc;
      startLine += linc;
      endLine -= linc;
      endSamp -= sinc;
      if (startSamp >= endSamp || startLine >= endLine) {
        std:: string msg = "Cannot find enough points to geom chip";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }
    }

    // Now take our control points and create the affine map
    p_affine.Solve(&x[0],&y[0],&xp[0],&yp[0],(int)x.size());

    //  TLS  8/3/06  Apply scale
    p_affine.Scale(scale);

    // Finally we need to make the affine map the tack point
    // to the requested cube sample/line
    p_affine.Compute(0.0,0.0);
    double cubeSampleOffset = p_cubeTackSample - p_affine.xp();
    double cubeLineOffset = p_cubeTackLine - p_affine.yp();
    p_affine.Translate(cubeSampleOffset,cubeLineOffset);

    // Now go read the data from the cube into the chip
    Read(cube,band);

    // Store off the cube address in case someone wants to match
    // this chip
    p_filename = cube.Filename();
  }


  /**
   * Compute the position of the cube given a chip coordinate.  Any
   * rotation or geometric matching done during the Load process will
   * be taken into account. Use the CubeSample and CubeLine methods 
   * to obtain results.  Note the results could be outside of the cube
   * 
   * @param sample    chip sample coordinate
   * @param line      chip line coordinate
   */
  void Chip::SetChipPosition (const double sample, const double line) {
    p_chipSample = sample;
    p_chipLine = line;
    p_affine.Compute(sample-TackSample(),line-TackLine());
    p_cubeSample = p_affine.xp();
    p_cubeLine = p_affine.yp();
  }


  /**
   * Compute the position of the chip given a cube coordinate.  Any
   * rotation or geometric matching done during the Load process will
   * be taken into account. Use the ChipSample and ChipLine methods 
   * to obtain results.  Note that the results could be outside of the
   * chip.
   * 
   * @param sample    chip sample coordinate
   * @param line      chip line coordinate
   */
  void Chip::SetCubePosition (const double sample, const double line) {
    p_cubeSample = sample;
    p_cubeLine = line;
    p_affine.ComputeInverse(sample,line);
    p_chipSample = p_affine.x() + TackSample();
    p_chipLine = p_affine.y() + TackLine();
  }


  /**
   * Set the valid range of data in the chip.  If never called all
   * data in the chip is consider valid (other than special pixels).
   * 
   * @param minimum   minimum valid pixel value
   * @param maximum   maximum valid pixel value
   * 
   * @throws Isis::iException::Programmer - Invalid order of parameters, min>max
   */
  void Chip::SetValidRange (const double minimum, const double maximum) {
    if (minimum >= maximum) {
      std::string msg = "Invalid order for [SetValidRange] method in class [Chip]";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    p_validMinimum = minimum;
    p_validMaximum = maximum;
  }


  /**
   * Return if the pixel is valid at a particular position
   * 
   * @param sample    sample position to test
   * @param line      line position to test
   * 
   * @return bool - Returns true if the pixel is valid, and false if it is not
   */
 /* bool Chip::IsValid(int sample, int line) {
    double value = (*this)(sample,line);
    if (value < p_validMinimum) return false;
    if (value > p_validMaximum) return false;
    return true;
  }*/


  /**
   * Return if the total number of valid pixels in the chip meets a specified
   * percentage of the entire chip.
   * 
   * @param percentage The percentage that the valid pixels percentage must 
   *                   exceed 
   * 
   * @return bool Returns true if the percentage of valid pixels is greater  
   *              than the specified percentage, and false if it is not
   */
  bool Chip::IsValid(double percentage) {
    int validCount = 0;
    for (int samp=1; samp<=Samples(); samp++) {
      for (int line=1; line<=Lines(); line++) {
        if (IsValid(samp,line)) validCount++;
      }
    }
    double validPercentage = 100.0 * (double) validCount / 
                             (double) (Samples() * Lines());
    if (validPercentage < percentage) return false;
    return true;
  }


  /**
   * Extract a sub-chip from a chip.  
   *  
   * @param samples   Number of samples in the extracted chip (must
   *                  be less than or equal to "this" chip)
   * @param lines     Number of lines in the extracted chip (must
   *                  be less than or equal to "this" chip)
   * @param samp      Input chip sample to be placed at output chip tack
   * @param line      Input chip line to be placed at output chip tack
   * 
   * @throws Isis::iException::Programmer - Chip extraction invalid
   */
  Chip Chip::Extract (int samples, int lines, int samp, int line) {
    if (samples > Samples() || lines > Lines()) {
      std::string msg = "Chip extraction invalid";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    Chip chipped(samples,lines);
    for (int oline=1; oline<=lines; oline++) {
      for (int osamp=1; osamp<=samples; osamp++) {
        int thisSamp = samp + (osamp - chipped.TackSample());
        int thisLine = line + (oline - chipped.TackLine());
        if ((thisSamp < 1) || (thisLine < 1) || 
            (thisSamp > Samples()) || thisLine > Lines()) {
          chipped.SetValue(osamp,oline, Isis::Null);
        }
        else {
          chipped.SetValue(osamp,oline, GetValue(thisSamp,thisLine));
        }
      }
    }

    chipped.p_affine = p_affine;
    chipped.p_validMinimum = p_validMinimum;
    chipped.p_validMaximum = p_validMaximum;
    chipped.p_tackSample = chipped.TackSample() + TackSample() - samp;
    chipped.p_tackLine = chipped.TackLine() + TackLine() - line;

    return chipped;
  }

  /**
   *  @brief Extract a subchip centered at the designated coordinate
   *  
   *  This method extracts a subchip that is centered at the given sample and
   *  line coordinate.  All appropriate variables in the given chipped parameter
   *  are set appropriately prior to return.
   * 
   * @param samp Center (tack) sample chip coordinate to extract subchip
   * @param line Center (tack) line chip coordinate to extract subchip
   * @param chipped Chip to load the subchip in and return to caller
   */
  void Chip::Extract (int samp, int line, Chip &chipped) {
    int samples = chipped.Samples(); 
    int lines = chipped.Lines();
    //chipped.Init(samples, lines);
    chipped.p_tackSample = ((samples - 1) / 2) + 1;
    chipped.p_tackLine = ((lines - 1) / 2) + 1;

    for (int oline=1; oline<=lines; oline++) {
      for (int osamp=1; osamp<=samples; osamp++) {
        int thisSamp = samp + (osamp - chipped.TackSample());
        int thisLine = line + (oline - chipped.TackLine());
        if ((thisSamp < 1) || (thisLine < 1) || 
            (thisSamp > Samples()) || thisLine > Lines()) {
          chipped.SetValue(osamp,oline, Isis::Null);
        }
        else {
          chipped.SetValue(osamp,oline,GetValue(thisSamp,thisLine));
        }
      }
    }

    chipped.p_affine = p_affine;
    chipped.p_validMinimum = p_validMinimum;
    chipped.p_validMaximum = p_validMaximum;
    chipped.p_tackSample = chipped.TackSample() + TackSample() - samp;
    chipped.p_tackLine = chipped.TackLine() + TackLine() - line;

    return;
  }


  /**
   * @brief Extract a subchip of this chip using an Affine transform 
   *  
   * This method will translate the data in this chip using an Affine transform 
   * to the output chip as provided. Note that the Affine transformation is only
   * applied within the confines of this chip.  No file I/O is performed. 
   *  
   * A proper Affine transform should not deviate too much from the identity as 
   * the mapping operation may result in a NULL filled chip.  The operation of 
   * this affine is added to the existing affine so that proper relationship to 
   * the input cube (and any affine operations applied at load time) is 
   * preserved.  This implies that the resulting affine should yield nearly 
   * identical results when read directly from the cube. 
   *  
   * Bilinear interpolation is applied to surrounding transformed pixels to 
   * provide each new output pixel. 
   *  
   * The chipped parameter will be updated to fully reflect the state of this 
   * original chip.  The state of the chipped parameter dictates the size and 
   * the tack sample and line coordinates.  Upon return, the corresponding cube 
   * sample and line coordinate is updated to the tack sample and line chip 
   * coordinate. 
   *  
   * As such, note that an identity affine transform will yield identical 
   * results to the Chip::Extract method specifying the tack sample and line as 
   * the location to extract. 
   *  
   * The following example demonstrates how to linearly shift a chip one pixel 
   * right and one down. 
   * @code 
   *   Chip mychip(35,35);
   *   Cube cube("mycube.cub");
   *   mychip.TackCube(200.0,200.0);
   *   mychip.Load(cube);
   *  
   *   Affine shift;
   *   shift.Translate(-1.0,-1.0);
   *  
   *   Chip ochip(15,15);
   *   mychip.Extract(ochip, shift);
   * @endcode 
   *  
   * @param chipped  Input/output chip containing the transformed subchip
   * @param affine   Affine transform to apply to extract subchip
   */ 
  void Chip::Extract (Chip &chipped, Affine &affine) {
   // Create an interpolator and portal for interpolation
    Interpolator interp(Interpolator::BiLinearType);
    Portal port(interp.Samples(),interp.Lines(),Isis::Double,
                interp.HotSample(),interp.HotLine());

    int samples = chipped.Samples(); 
    int lines = chipped.Lines();

    for (int oline=1; oline<=lines; oline++) {
      int thisLine = TackLine() + (oline - chipped.TackLine());
      for (int osamp=1; osamp<=samples; osamp++) {
        int thisSamp = TackSample() + (osamp - chipped.TackSample());
        affine.Compute(thisSamp,thisLine);
        double xp = affine.xp();
        double yp = affine.yp();
        port.SetPosition(xp, yp, 1);
        for (int i = 0 ; i < port.size() ; i++) {
          int csamp = port.Sample(i);
          int cline = port.Line(i);
          if ((csamp < 1) || (cline < 1) || 
              (csamp > Samples()) || cline > Lines()) {
            port[i] = Isis::Null;
          }
          else {
            port[i] = GetValue(csamp,cline);
          }
        }
        chipped.SetValue(osamp,oline,interp.Interpolate (xp, yp, port.DoubleBuffer()));
      }
    }

    chipped.p_validMinimum = p_validMinimum;
    chipped.p_validMaximum = p_validMaximum;
    chipped.p_filename = p_filename;

    // Update the affine
    Affine::AMatrix taffine = p_affine.Forward() + 
                              (affine.Forward() - Affine::getIdentity());

    chipped.p_affine = Affine(taffine);
    chipped.SetChipPosition(chipped.TackSample(), chipped.TackLine());
    return;
  }


  /**
   * Returns a statistics object of the current data in the chip. 
   *  
   * The caller takes ownership of the returned instance. 
   * 
   * @return Isis::Statistics* Statistics of the data in the chip
   */
  Isis::Statistics *Chip::Statistics() {
    Isis::Statistics *stats = new Isis::Statistics();

    stats->SetValidRange(p_validMinimum, p_validMaximum);
    
    for (int i = 0; i < p_chipSamples; i++) {
      stats->AddData(&p_buf[i][0], p_chipLines);
    }

    return stats;
  }


  /**
   * This method will read data from a cube and put it into the chip.
   * The affine transform is used in the SetChipPosition routine and
   * therefore the geom of the chip is automatic
   * 
   * @param cube    Cube to read data from
   * @param band    Band number to read data from
   * 
   * @todo We could modify the affine class to return the coefficients
   * and then compute the derivative of the change in cube sample and line 
   * with respect to chip sample or line.  The change might make the geom
   * run a bit faster.
   */
  void Chip::Read(Cube &cube, const int band) {
    // Create an interpolator and portal for geoming
    Interpolator interp(Interpolator::CubicConvolutionType);
    Portal port(interp.Samples(),interp.Lines(),cube.PixelType(),
                interp.HotSample(),interp.HotLine());
    // Loop through the pixels in the chip and geom them
    for (int line=1; line<=Lines(); line++) {
      for (int samp=1; samp<=Samples(); samp++) {
        SetChipPosition((double)samp,(double)line);
        if ((CubeSample() < 0.5) || (CubeLine() < 0.5) ||
            (CubeSample() > cube.Samples()+0.5) ||
            (CubeLine() > cube.Lines()+0.5)) {
          p_buf[line-1][samp-1] = Isis::NULL8;
        }
        else if (p_clipPolygon == NULL) {
          port.SetPosition (CubeSample(),CubeLine(), band);
          cube.Read(port);
          p_buf[line-1][samp-1] = 
          interp.Interpolate (CubeSample(), CubeLine(), port.DoubleBuffer());
        }
        else {
          geos::geom::Point *pnt = globalFactory.createPoint(
                                   geos::geom::Coordinate(CubeSample(), CubeLine()));
          if (pnt->within(p_clipPolygon)) {
            port.SetPosition (CubeSample(),CubeLine(), band);
            cube.Read(port);
            p_buf[line-1][samp-1] = 
            interp.Interpolate (CubeSample(), CubeLine(), port.DoubleBuffer());
          }
          else {
            p_buf[line-1][samp-1] = Isis::NULL8;
          }
          delete pnt;
        }
      }
    }
  }


  /**
   * Writes the contents of the Chip to a cube.
   *  
   * @param filename  Name of the cube to create
   */
  void Chip::Write (const std::string &filename) {
    Cube c;
    c.SetDimensions(Samples(),Lines(),1);
    c.Create(filename);
    LineManager line(c);
    for (int i=1; i<=Lines(); i++) {
      line.SetLine(i);
      for (int j=1; j<=Samples(); j++) {
        line[j-1] = GetValue(j,i);
      }
      c.Write(line);
    }
    c.Close();
  }


  /**
   * Sets the clipping polygon for this chip. The coordinates must be in
   * (sample,line) order. All Pixel values outside this polygon will be set to
   * Null8. The cubic convolution interpolation is allowed to uses valid pixels
   * outside the clipping area.
   *  
   * @param clipPolygon  The polygons used to clip the chip
   */
  void Chip::SetClipPolygon (const geos::geom::MultiPolygon &clipPolygon) {
    if (p_clipPolygon != NULL) delete p_clipPolygon;
    p_clipPolygon = PolygonTools::CopyMultiPolygon(clipPolygon);
  }

} // end namespace isis
