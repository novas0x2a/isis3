 /* @file                                                                  
 * $Revision: 1.7 $                                                             
 * $Date: 2008/05/14 21:07:23 $                                                                 
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
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <sstream>
#include "HiImageClean.h"
#include "Cube.h"
#include "Table.h"
#include "Progress.h"
#include "iString.h"
#include "PvlGroup.h"
#include "Pixel.h"
#include "QuickFilter.h"
#include "iException.h"

namespace Isis {

  using std::string;
  using std::vector;


//  Declare constants for internal tracking
  const std::string HiImageClean::_version = "1.4.1";
  const std::string HiImageClean::_revision = "$Revision: 1.7 $";
  const std::string HiImageClean::_compatability = "Alan D. clean.pro, "
                                                   " version 2/29 sep 2006";
  const int HiImageClean::_filterWidth = 11;


/**
 *  @brief Cube based constructor for HiRISE image cleaner
 */
  HiImageClean::HiImageClean(Cube &cube) {
    _filename = cube.Filename();
    _lines =  _samples = 0;
    _totalMaskNulled = _totalDarkNulled = 0;
    init(cube);
  }

/**
 *  @brief File name based constructor for HiRISE image cleaner
 */
  HiImageClean::HiImageClean(const std::string &fname) {
    Cube cube;
    cube.Open(fname);
    _filename = fname;
    _lines = _samples = 0;
    _totalMaskNulled = _totalDarkNulled = 0;
    init(cube);
  }

/**
 * @brief Generate a Pvl group containing statistics data
 * 
 * This method returns a group called RadiometricCalibration and adds
 * a Name keyword with the name parameter as the value.  It follows the
 * HiRISE convention of tracking by adding a version number.  It includes
 * the version of \b iclean_v2.1.pro that its based upon.
 * 
 * Note that this method should only be called after processing an image
 * using the cleanLine method.  This method reports statistics on image
 * data that has been processed. 
 * 
 * @param [in] (string) name Value to be inserted in the group
 * @return (PvlGroup) An Isis group named RadiometricCalibration with many
 *                    keywords that indicate some information about the input
 *                    file and computed statistics.
 */
  PvlGroup HiImageClean::getPvlForm(const std::string &name) const {
    PvlGroup mygroup("RadiometricCalibration");

    mygroup += PvlKeyword("Name", name);
    mygroup += PvlKeyword("Version", _version);
    mygroup += PvlKeyword("Compatability", _compatability);
    mygroup += PvlKeyword("Revision", _revision);

//  mygroup.AddComment("Starting indexes");
    mygroup += PvlKeyword("MaskStartingRow", _firstMaskLine);
    mygroup += PvlKeyword("MaskEndingRow", _lastMaskLine);
    BigInt nBad = _maskStats.TotalPixels() - _maskStats.ValidPixels();
    mygroup += PvlKeyword("BadMaskPixels", nBad);
    mygroup += PvlKeyword("MaskInducedNulls", _totalMaskNulled);

    mygroup += PvlKeyword("DarkStartingColumn", 4);
    mygroup += PvlKeyword("DarkEndingColumn", 11);
    nBad = _darkStats.TotalPixels() - _darkStats.ValidPixels();
    mygroup += PvlKeyword("BadDarkPixels", nBad);
    mygroup += PvlKeyword("DarkInducedNulls", _totalDarkNulled);
    mygroup += PvlKeyword("LastGoodLine",_lastGoodLine+1);

    return(mygroup);
  }

  /**
   * Propagates the modified blobs to the output cube
   * @param info The CubeInfo for the output cube
   */
  void HiImageClean::propagateBlobs(Cube *cube) {

    Table calAnc("HiRISE Calibration Ancillary");
    cube->Read(calAnc);
    Table calImg("HiRISE Calibration Image");
    cube->Read(calImg);

    for (int r=0; r<calAnc.Records(); r++) {
      vector<int> tempBuf, tempDark, tempCal;
      TableRecord rec1 = calAnc[r];
      TableRecord rec2 = calImg[r];
      for (int i=0; i < _calbuf.dim2(); i++) {
        tempBuf.push_back(Pixel::To16Bit(_calbuf[r][i]));
      }
      for (int i=0; i < _caldark.dim2(); i++) {
        tempDark.push_back(Pixel::To16Bit(_caldark[r][i]));
      }
      for (int i=0; i < _calimg.dim2() ; i++) {
        tempCal.push_back(Pixel::To16Bit(_calimg[r][i]));
      }
      rec1["BufferPixels"] = tempBuf;
      rec1["DarkPixels"] = tempDark;
      rec2["Calibration"] = tempCal;
      calAnc.Update(rec1,r);
      calImg.Update(rec2,r);
    }

    Table anc("HiRISE Ancillary");
    cube->Read(anc);
    for (int r=0; r<anc.Records(); r++) {
      vector<int> tempABuf, tempADark;
      TableRecord rec = anc[r];
      for (int i=0; i < _ancbuf.dim2(); i++) {
        tempABuf.push_back(Pixel::To16Bit(_ancbuf[r][i]));
      }
      for (int i=0; i<_ancdark.dim2(); i++) {
        tempADark.push_back(Pixel::To16Bit(_ancdark[r][i]));   
      }
      rec["BufferPixels"] = tempABuf;
      rec["DarkPixels"] = tempADark;
      anc.Update(rec,r);
    }

    // Write the tables back out to the cube
    cube->Write(calAnc);
    cube->Write(calImg);
    cube->Write(anc);

    // Write out statistical information

    PvlGroup calib;
    calib = getPvlForm("hiclean");
    cube->PutGroup(calib);
  }

/**
 * @brief Method to apply calibration to cubes line-by-line
 * 
 * cleanLine is passed an input image line in the \b in Buffer class and
 * is expected to have an exactly dimensioned Buffer in \b out where the
 * results are transferred back to the caller.
 * 
 * Once all lines are processed, the caller can get statistics in the
 * form of a Pvl Group structure.
 * 
 * @param [in]  (Buffer) in Input line buffer containing line to process
 * @param [out] (Buffer) out Buffer returning calibrated line
 */
  void HiImageClean::cleanLine(Buffer &in, Buffer &out) {

// Ensure buffers match in size
    if (in.size() != out.size()) {
      std::ostringstream mess;
      mess << "Size of input (" << in.size() << ") and output buffers (" 
      << out.size() << ") do not match!" << std::ends;
      throw(iException::Message(iException::Programmer, mess.str(), 
                                _FILEINFO_));  
    }

// Ensure the specified line is valid
    long line(in.Line()-1);
    long lastLine(_dark.dim()-_firstImageLine);
    if ((line < 0) || (line >= lastLine)) {
      std::ostringstream mess;
      mess << "Specified input line number (" << (line+1)
      << ") is not in valid range (Max: " << (lastLine+1) << ")"
      << std::ends;
      throw(iException::Message(iException::Programmer, mess.str(), 
                                _FILEINFO_));  
    }

//  Check for current line exceeding the last good line
    line = (line > _lastGoodLine) ? _lastGoodLine : line;

//  Use properties of H2DBuf to efficiently process the line.
    H2DBuf ibuf(1, in.size(), in.DoubleBuffer());
    H2DBuf lbuf = column_apply(ibuf, _mask, _firstImageSample, _totalMaskNulled,
                               1.0);
    lbuf = row_apply(lbuf, _dark, _firstImageLine+line, _totalDarkNulled,
                     (_binning*1.0));

    H2DBuf obuf(1, out.size(), out.DoubleBuffer());
    obuf.inject(lbuf);
    return;
  }

/**
 * @brief Convert blob to HiImageClean's internal buffer format
 * 
 * This conversion utility will convert a Blobber buffer to an internal format
 * compatable with the HiImageClean class.  This simplifies the intialization
 * and subsequent use of the blob regions of the cube data.
 * 
 * @param blob Blob to convert
 * 
 * @return HiImageClean::H2DBuf Return compatable 2-D buffer array
 */
HiImageClean::H2DBuf HiImageClean::blobvert(const Blobber &blob) const {
  H2DBuf transformer(blob.Lines(), blob.Samples(), 
                     const_cast<double *> (blob[0]));
  return (transformer.copy());
}

/**
 * @brief Initializes the object by computing all calibration statistics
 * 
 * This method validates the input file, reads labels for needed values
 * and computes calibration statistics for data reduction.
 * 
 * @param [in] (CubeInfo &) cube Opened cube where label and ancillary data
 *                               is read from
 */
  void HiImageClean::init(Cube &cube) {

    _lines = cube.Lines();
    _samples = cube.Samples();
    _lastGoodLine = _lines - 1;
    _totalMaskNulled = _totalDarkNulled = 0;

    PvlGroup &instrument = cube.GetGroup("Instrument");

// It may be too late and a non-issue by this time, but should check to ensure
// this is a valid HiRISE image
    iString instId = (std::string) instrument["InstrumentId"];
    if (instId.UpCase() != "HIRISE") {
      string message = "Image must be a HiRISE image (InstrumentId != HIRISE)";
      iException::Message(iException::User, message, _FILEINFO_);
    }

// Extract what is needed
    _binning = instrument["Summing"];
    _tdi = instrument["Tdi"];
    _cpmm = instrument["CpmmNumber"];
    _channelNo = instrument["ChannelNumber"];

//  Initialize all HiRISE calibration blobs
    _calimg  = blobvert(HiCalibrationImage(cube));
    _calbuf  = blobvert(HiCalibrationBuffer(cube));
    _caldark = blobvert(HiCalibrationDark(cube));
    _ancbuf  = blobvert(HiAncillaryBuffer(cube));
    _ancdark = blobvert(HiAncillaryDark(cube));

//  Compute statistics from blobs
    computeStats();
    return;
  }

/**
 * @brief Resets all internal image cleaning statistics
 * 
 * This method resets the statistics to the initial collection state
 * prior to cleaning a HiRISE image.  This is provided to allow users to
 * apply the cleaning to several images if desired and ensure valid statistics.
 */
  void HiImageClean::resetStats() {
    _totalMaskNulled = _totalDarkNulled = 0;
    return;
  }

/**
 * @brief Append buffers in the line (vertical) dimension
 * 
 * Turns out this algorithm is more easily applied to the full image that
 * has all ancillary data contiguously stored (the process that \b higlob
 * performs on Isis HiRISE images).
 * 
 * This method appends blob lines of all passed blobs into one large
 * 2-dimensional buffer.  This makes the computations of necessary data
 * much easier.
 * 
 * @param [in] (vector<H2DBuf *>) buffers Contains pointers to buffers
 *                                         objects that are to be spatially
 *                                         stacked into one buffer
 * @return (H2DBuf) Two dimensional buffer containing stacked buffer data
 * 
 */
  HiImageClean::H2DBuf HiImageClean::appendLines(
                                     const std::vector<H2DBuf> &buffers) 
                                     const {

//  Funky kludge to overcome inability to directly instantiate iException object
    iException ie = iException::Message(iException::Programmer, "", _FILEINFO_);
    ie.Clear();

    long samples(0);
    long lines(0);
    long nerrors(0);
    std::vector<H2DBuf>::const_iterator b;
    for (b = buffers.begin() ; b != buffers.end() ; ++b) {
      const H2DBuf &buf= (*b);

//  Check for consistancy in samples
      if (b == buffers.begin()) {
        samples = buf.dim2();
      }
      else {
        if (buf.dim2() != samples) {
          std::ostringstream mess;
          mess << "Buffers must have same number of samples (" 
          << buf.dim2() << ") as all other buffers (" << samples << ")"
          << std::ends;
          iException::Message(iException::Programmer, mess.str(), _FILEINFO_);
          nerrors++;
        }
      }

//  Sum up all lines
      lines += buf.dim1();
    }

//  If errors, throw an error
    if (nerrors > 0) {
      std::string message = "Inconsistancies exist in buffers";
      throw(iException::Message(iException::Programmer, message, _FILEINFO_));
    }

    H2DBuf mbuf(lines, samples);
    int iLine(0);
    for (b = buffers.begin() ; b != buffers.end() ; ++b) {
      const H2DBuf &buf= (*b);
      for (int row = 0 ; row < buf.dim1() ; row++, iLine++) {
        for (int col = 0 ; col < buf.dim2() ; col++) {
          mbuf[iLine][col] = buf[row][col];
        }
      }
    }

    return(mbuf);
  }


/**
 * @brief Append buffers in the sample (horizontal) dimension
 * 
 * Turns out this algorithm is more easily applied to the full image that
 * has all ancillary data contiguously stored (the process that \b higlob
 * performs on Isis HiRISE images).
 * 
 * This method appends blob samples of all passed blobs into one large
 * 2-dimensional buffer.  This makes the computations of necessary data
 * much easier.
 * 
 * @param [in] (vector<H2DBuf *>) buffers Contains pointers to buffers
 *                                         objects that are to be spatially
 *                                         layed next to one another in one 
 *                                         buffer
 * @return (H2DBuf) Two dimensional buffer containing sampled ordered buffer
 *                 data
 * 
 */
  HiImageClean::H2DBuf HiImageClean::appendSamples(
                                     const std::vector<H2DBuf> &buffers) 
                                                   const {

    long samples(0);
    long lines(0);
    long nerrors(0);
    std::vector<H2DBuf>::const_iterator b;
    for (b = buffers.begin() ; b != buffers.end() ; ++b) {
      const H2DBuf &buf = (*b);

//  Check for consistancy in samples
      if (b == buffers.begin()) {
        lines = buf.dim1();
      }
      else {
        if (buf.dim1() != lines) {
          std::ostringstream mess;
          mess << "Buffer must have same number of lines (" 
          << buf.dim1() << ") as all other buffers (" << lines << ")"
          << std::ends;
          iException::Message(iException::Programmer, mess.str(), 
                              _FILEINFO_);
          nerrors++;
        }
      }

//  Sum up all samples
      samples += buf.dim2();
    }

//  If errors, throw an error
    if (nerrors > 0) {
      std::string message = "Inconsistancies exist in buffers";
      throw(iException::Message(iException::Programmer, message, _FILEINFO_));
    }

    H2DBuf mbuf(lines, samples);
    int iCol(0);
    for (b = buffers.begin() ; b != buffers.end() ; ++b) {
      const H2DBuf &buf= (*b);
      for (int col = 0 ; col < buf.dim2() ; col++, iCol++) {
        for (int row = 0 ; row < buf.dim1() ; row++) {
          mbuf[row][iCol] = buf[row][col];
        }
      }
    }

    return(mbuf);
  }


/**
 * @brief Extracts a vertical slice (sample) from a 2D buffer
 * 
 * This method will return a 1 dimensional buffer taken from a 
 * sample location for each line.  It extracts a vertical slice
 * through the 2D buffer at a specified sample.  This method makes
 * computations of vertical statistics much simpler.
 * 
 * @param [in] (H2DBuf) buffer Two dimensional data buffer to extract
 *                             a vertical slice from at the specified
 *                             sample
 * @param [in] (long) sample Sample in buffer where the slice is to
 *                             be extracted from.  Indexes start at 0.
 * @return (H1DBuf) Returns a 1 dimensional buffer of the requested data
 */
  HiImageClean::H1DBuf HiImageClean::slice(const H2DBuf &buffer, long sample) 
  const {

//  Validate sample request
    if ((sample < 0) || (sample >= buffer.dim2())) {
      std::ostringstream mess;
      mess << "Sample index (" << sample << ") out of range (Max:" 
      << buffer.dim2() << ")" << std::ends;
      throw(iException::Message(iException::Programmer, mess.str(),_FILEINFO_));
    }

//  Allocate output buffer and extract it
    H1DBuf buf(buffer.dim1());
    for (int i = 0 ; i < buffer.dim1() ; i++) {
      buf[i] = buffer[i][sample];
    }

    return(buf);
  }

/**
 * @brief Applies calibration correction to each sample
 * 
 * This method is generalized to apply individually computed sample
 * corrections for HiRISE images.  It provides the ability to define
 * a specific sample to map the correction to.  This is needed since
 * the image section is in the larger middle portion of the complete
 * representation of an HiRISE observation.
 * 
 * @param [in] (H2DBuf) data Data to be cleaned.  Any number of lines can
 *                           be processed but the number of samples must 
 *                           satisfy noise.Samples() <= (startStatNdx +
 *                           data.Samples() - 1)
 * @param [in] (StatBuf) noise Buffer containing noise data to apply to
 *                             each sample in data.
 * @param [in] (long) startStatNdx Starting index into the noise buffer that
 *                                 map to the first sample in data.
 * @param [in] (double) calmult Optional calibration multiplier prior to
 *        subtraction from data
 * 
 * @return (H2DBuf) A new array derived from data but with noise correction
 *                  applied
 *                          
 */
  HiImageClean::H2DBuf HiImageClean::column_apply(const H2DBuf &data, 
                                                  const H1DBuf &cal, 
                                                  const int startStatNdx,
                                                  BigInt &nNulled,
                                                  double calmult) const {

//  Sanity check on data versus stats
    long maxsamples(cal.dim()-startStatNdx);
    if (data.dim2() > maxsamples) {
      std::ostringstream mess;
      mess << "Number of samples in input buffer (" << data.dim2()
      << ") exceeds samples in statistics buffer (StartNdx="
      << startStatNdx
      << ", TotalSamples=" << cal.dim()
      << ", Samples=" << maxsamples << ")"
      << std::ends;
      throw(iException::Message(iException::Programmer,mess.str(),_FILEINFO_));    
    }

    H2DBuf lbuf(data.dim1(), data.dim2());
//  Use average (for lutted images) or medians
    for (int line=0 ; line < data.dim1() ; line++) {
      for (int sample=0, sndx=startStatNdx ; sample < data.dim2() ; 
           sample++, sndx++) {
        if (IsSpecial(data[line][sample])) {
          lbuf[line][sample] = data[line][sample];
        }
        else if (IsSpecial(cal[sndx])) {
          lbuf[line][sample] = Null;
          nNulled++;
        }
        else {
          lbuf[line][sample] = data[line][sample] - (cal[sndx] * calmult); 
        }
      }
    }

  return(lbuf);
}

/**
 * @brief Applies calibration correction to each line
 * 
 * This method is generalized to apply individually computed line corrections
 * for HiRISE images.  It provides the ability to define a specific line to map
 * the correction to.  This is needed since the image section is in the larger
 * middle portion of the complete representation of an HiRISE observation.
 * 
 * @param [in] (H2DBuf) data Data to be cleaned.  Any number of lines can
 *                           be processed but the number of samples must 
 *                           satisfy noise.Samples() <= (startStatNdx +
 *                           data.Samples() - 1)
 * @param [in] (StatBuf) noise Buffer containing noise data to apply to
 *                             each sample in data.
 * @param [in] (long) startStatNdx Starting index into the cal buffer that
 *                                 map to the first line in data.
 * @param [in] (double) calmult Optional calibration multiplier prior to
 *        subtraction from data
 * 
 * @return (H2DBuf) A new array derived from data but with noise correction
 *                  applied
 *                          
 */
  HiImageClean::H2DBuf HiImageClean::row_apply(const H2DBuf &data, 
                                               const H1DBuf &cal, 
                                               const int startStatNdx,
                                               BigInt &nNulled,
                                               double calmult) const {

//  Sanity check on data versus stats
    long maxlines(cal.dim()-startStatNdx);
    if (data.dim1() > maxlines) {
      std::ostringstream mess;
      mess << "Number of lines in input buffer (" << data.dim1()
      << ") exceeds lines in statistics buffer (StartNdx="
      << startStatNdx
      << ", TotalLines=" << cal.dim()
      << ", Lines=" << maxlines << ")"
      << std::ends;
      throw(iException::Message(iException::Programmer,mess.str(),_FILEINFO_));    
    }

    H2DBuf lbuf(data.dim1(), data.dim2());
    for (int line=0, sndx=startStatNdx ; line < data.dim1() ; line++, sndx++) {
      for (int sample=0 ; sample < data.dim2() ; sample++) {
        if (IsSpecial(data[line][sample])) {
          lbuf[line][sample] = data[line][sample];
        }
        else if (IsSpecial(cal[sndx])) {
          lbuf[line][sample] = Null;
          nNulled++;
        }
        else {
          lbuf[line][sample] = data[line][sample] - (cal[sndx] * calmult); 
        }
      }
    }
  return(lbuf);
}

void HiImageClean::cimage_mask() {

// Combine calibration region
  std::vector<H2DBuf> blobs;
  blobs.push_back(_calbuf);
  blobs.push_back(_calimg);
  blobs.push_back(_caldark);
  H2DBuf calibration = appendSamples(blobs);

// Set the mask depending on the binning mode
  _firstMaskLine = 20;
  _lastMaskLine = 39;
  switch (_binning) {
    case 1:
      _firstMaskLine = 21;
      _lastMaskLine = 38;
      break;
    case 2:
      _firstMaskLine = 21;
      _lastMaskLine = 29;
      break;
    case 3:
      _firstMaskLine = 21;
      _lastMaskLine = 26;
      break;
    case 4:
      _firstMaskLine = 21;
      _lastMaskLine = 24;
      break;
    case 8:
      _firstMaskLine = 21;
      _lastMaskLine = 22;
      break;
    case 16:
      _firstMaskLine = 21;
      _lastMaskLine = 21;
      break;
    default:
      std::ostringstream msg;
      msg << "Invalid binning mode (" << _binning 
          << ") - valid are 1-4, 8 and 16" << std::ends;
      throw(iException::Message(iException::Programmer,msg.str(),_FILEINFO_));
  }

  //  Initialize lines and samples of mask area of interest
  int nsamples(calibration.dim2());
  int nlines(_lastMaskLine - _firstMaskLine + 1);

  // Compute averages for the mask area
  _premask = H1DBuf(nsamples);
  for (int samp = 0 ; samp < nsamples; samp++) {
    H1DBuf maskcol = slice(calibration, samp);
    Statistics maskave;
    maskave.AddData(&maskcol[_firstMaskLine], nlines);
    _premask[samp] = maskave.Average();
  }
  _mask = _premask.copy();

  // Get statistics to determine state of mask and next course of action
  _maskStats.Reset();
  _maskStats.AddData(&_premask[0], nsamples);
  if (_maskStats.ValidPixels() <= 0) {
    std::ostringstream mess;
    mess << "No valid pixels in calibration mask region in lines " 
         << (_firstMaskLine+1) << " to " << (_lastMaskLine+1) << ", binning = "
         << _binning << std::ends;
    throw(iException::Message(iException::Programmer,mess.str(),_FILEINFO_));
  }

  //  If there are any missing values, replace with mins/maxs of region
  if (_maskStats.TotalPixels() != _maskStats.ValidPixels()) {
    for (int samp = 0 ; samp < nsamples ; samp++) {
      if (Pixel::IsLow(_premask[samp]) || Pixel::IsNull(_premask[samp])) {
        _mask[samp] = _maskStats.Minimum();
      }
      else if (Pixel::IsHigh(_premask[samp])) {
        _mask[samp] = _maskStats.Maximum();
      }
    }
  }

  // Now apply to all calibration data
  BigInt nbad(0);
  _calimg  = column_apply(_calimg,  _mask, _firstImageSample,  nbad, 1.0);
  _calbuf  = column_apply(_calbuf,  _mask, _firstBufferSample, nbad, 1.0);
  _caldark = column_apply(_caldark, _mask, _firstDarkSample,   nbad, 1.0);
  _ancbuf  = column_apply(_ancbuf,  _mask, _firstBufferSample, nbad, 1.0);
  _ancdark = column_apply(_ancdark, _mask, _firstDarkSample,   nbad, 1.0);
  return;
}

void HiImageClean::cimage_dark() {

// Combine calibration region
  std::vector<H2DBuf> blobs;
  blobs.push_back(_caldark);
  blobs.push_back(_ancdark);
  H2DBuf dark = appendLines(blobs);


  int nsamples(dark.dim2());
  int nlines(dark.dim1());

  // Compute averages for the mask area
  int firstDark(4);
  int ndarks(dark.dim2()-firstDark);
  _predark = H1DBuf(nlines);
  for (int line = 0 ; line < nlines ; line++) {
    Statistics darkave;
    darkave.AddData(&dark[line][firstDark], ndarks);
    _predark[line] = darkave.Average();
  }

  // Get statistics to determine state of mask and next course of action
  _darkStats.Reset();
  _darkStats.AddData(&_predark[0], _predark.dim1());
  if (_darkStats.ValidPixels() <= 0) {
    std::ostringstream mess;
    mess << "No valid pixels in calibration/ancillary dark regions, " 
         << "binning = " << _binning << std::ends;
    throw(iException::Message(iException::Programmer,mess.str(),_FILEINFO_));
  }

  //  Now apply a smoothing filter
  QuickFilter smooth(_predark.dim1(), _filterWidth, 1);
  smooth.AddLine(&_predark[0]);
  nsamples = smooth.Samples();
  _dark = H1DBuf(nsamples);
  for (int s = 0 ; s < nsamples ; s++) {
    _dark[s] = smooth.Average(s);
  }

  // Now apply to all calibration data
  BigInt nbad(0);
  _calimg  = row_apply(_calimg,  _dark,               0, nbad, 1.0);
  _calbuf  = row_apply(_calbuf,  _dark,               0, nbad, 1.0);
  _caldark = row_apply(_caldark, _dark,               0, nbad, 1.0);
  _ancbuf  = row_apply(_ancbuf,  _dark, _firstImageLine, nbad, 1.0);
  _ancdark = row_apply(_ancdark, _dark, _firstImageLine, nbad, 1.0);
  return;
}

/**
 * @brief Compute statistics from HiRISE ancillary data BLOBs
 * 
 * From the HiRISE BLOBS (objects), this method computes statistics from
 * the Reverse Clock, Mask, Buffer and Dark current ancillary data.  This
 * is used to radiometrically calibrate each HiRISE pixel.  Methods are
 * applied to individual samples (Reverse Clock, Mask) and complete lines
 * (Buffer and Dark).
 * 
 * It store the results internally for future computations of a line of
 * HiRISE image data.  See the cleanLine() method.
 */
  void HiImageClean::computeStats() {

//  Set up indexes
    _firstImageSample = _calbuf.dim2();
    _firstImageLine   = _calimg.dim1();

    _firstBufferSample = 0;
    _firstDarkSample = _firstImageSample + _calimg.dim2();

    cimage_mask();
    cimage_dark();

// Finally, reset the statistics
    resetStats();
    return;
  }

/**
 * @brief Helper method that creates an array of indexes
 * 
 * This method is designed to compute indexes start at firstIndex and
 * incrementing by one each subsequent array element.  It returns arrays
 * of size count.
 * 
 * @param (string) name Name the buffer
 * @param (BigInt) count Number of elements to allocate to arrays
 * @parm (BigInt) firstIndex Starting value of the first array element
 * @return (StatBuf) Intended to be used in identifying each element of
 *                   values listed in ouput data streams.
 */
  HiImageClean::DataBuffer HiImageClean::createIndex(const std::string &name,
                                                     BigInt count, 
                                                     BigInt firstIndex) const {
    H1DBuf index(count, 0.0);
    for (int i = 0 ; i < count ; i++) {
      index[i] = firstIndex++;
    }
    return(DataBuffer(name,index));
  }

/**
 * @brief Writes to contents of this class to an output stream
 * 
 * This output stream operator dumps the complete contents of this 
 * object to the designated stream.
 * 
 * @param (ostream) o Stream to write data
 * @param (HiImageClean) An object of this class whose contents are to be
 *                       written
 */
  std::ostream& operator<<(std::ostream& o, const HiImageClean &cs) {
    Pvl p;
    PvlGroup imgStats, calStats;
    cs.PvlImageStats(imgStats);
    cs.PvlCalStats(calStats);
    p.AddGroup(imgStats);
    p.AddGroup(calStats);
    o << p << "\n";

    o << "\n*** Dark & Mask Correction Buffers ***\n";
    std::vector<HiImageClean::DataBuffer> stats;
    HiImageClean::DataBuffer rows = cs.createIndex("Row", cs._dark.dim(),
                                                   -cs._firstImageLine);
    stats.push_back(rows);
    stats.push_back(HiImageClean::DataBuffer("Dark", cs._dark));

    HiImageClean::DataBuffer columns = cs.createIndex("Column", cs._mask.dim(), 
                                                      -cs._firstImageSample);
    stats.push_back(columns);
    stats.push_back(HiImageClean::DataBuffer("Mask",cs._mask));
    cs.dumpStatBufs(o, stats);
    return(o);
  }

/**
 * @brief Writes data specific to image statistics out in Pvl format
 * 
 * This method creates a PvlGroup containing the image statistics
 * acculated during line-by-line processing of raw image data.
 */
  void HiImageClean::PvlImageStats(PvlGroup &grp) const {
    grp.SetName("ImageStatistics");
    grp += PvlKeyword("File",_filename.Name());
    grp += PvlKeyword("Lines",_lines);
    grp += PvlKeyword("Samples",_samples);

    BigInt nBad = _maskStats.TotalPixels() - _maskStats.ValidPixels();
    grp += PvlKeyword("MaskAverage", _maskStats.Average());
    grp += PvlKeyword("MaskStdDev", _maskStats.StandardDeviation());
    grp += PvlKeyword("BadMaskPixels", nBad);
    grp += PvlKeyword("MaskInducedNulls", _totalMaskNulled);

    nBad = _darkStats.TotalPixels() - _darkStats.ValidPixels();
    grp += PvlKeyword("DarkAverage", _darkStats.Average());
    grp += PvlKeyword("DarkStdDev", _darkStats.StandardDeviation());
    grp += PvlKeyword("BadDarkPixels", nBad);
    grp += PvlKeyword("DarkInducedNulls", _totalDarkNulled);

  }

/**
 * @brief Writes data specific to calibration statistics in Pvl Format
 * 
 * This method is a compartmentalization of the writing of the contents of
 * a HiImageClean object.  
 * 
 * This method dumps the calibration statistics computed from the HiRISE
 * cube specifed on construction or through the load methods.
 */
  void HiImageClean::PvlCalStats(PvlGroup &grp) const {
    grp.SetName("CalibrationStatistics");
    grp += PvlKeyword("Binning",_binning);
    grp += PvlKeyword("TDI",_tdi);
    grp += PvlKeyword("CPMM",_cpmm);
    grp += PvlKeyword("Channel",_channelNo);
    grp += PvlKeyword("FirstImageSample",_firstImageSample);         
    grp += PvlKeyword("FirstImageLine",_firstImageLine);          
    grp += PvlKeyword("FirstBufferSample",_firstBufferSample); 
    grp += PvlKeyword("FirstDarkSample",_firstDarkSample);
    grp += PvlKeyword("MaskInducedNulls", _totalMaskNulled);
    grp += PvlKeyword("DarkInducedNulls", _totalDarkNulled);
    grp += PvlKeyword("LastGoodLine",_lastGoodLine+1);
  }

/**
 * @brief Writes calibration data arrays to output stream
 * 
 * This method is a compartmentalization of the writing of the contents of
 * a HiImageClean object.  
 * 
 * This method dumps the calibration arrays computed from the HiRISE
 * cube specifed on construction or through the load methods.
 * 
 * This method prints the contents of each StatBuf contained in the
 * input vector in a single column in the output stream.  The name of 
 * the column is used as the header.  
 * 
 * Note that the size of the Statbufs can vary.  This method is designed
 * to correctly handle any size given for each array.  Output stops when
 * the last element has been reached for a given buffer.  Spaces of the
 * width argument are subsequently written to preserve spacing.
 * 
 * @param [in] o Output stream to dump data
 * @param [in] (vector<StatBuf *>) This vector contains const pointers to 
 *                                 a series of StatBuf buffers
 * @param [in] (int) width Specifies the width of each field to write
 * @param [in] (int) prec  Specifies the precision to be applied to floating
 *                         point data when writing
 * @return o, the stream provided by the caller when completed
 */
  std::ostream &HiImageClean::dumpStatBufs(std::ostream &o, 
                                           const std::vector<DataBuffer> &stat,
                                           int width, int prec) const {
    std::vector<DataBuffer>::const_iterator s;

    //  Determine maximum number of lines in data and print header
    long maxsize(0);
    for (s = stat.begin() ; s != stat.end() ; ++s) {
      if (s->data.dim() > maxsize) maxsize = s->data.dim();
      o << std::setw(width) << s->name;
    }
    o << std::endl;

    //  Print the data
    for (int i = 0 ; i < maxsize ; i++) {
      for (s = stat.begin() ; s != stat.end() ; ++s) {
        if (i < s->data.dim()) {
          o << std::setw(width) << formValue(s->data[i], width, prec);
        }
        else {
          o << std::setw(width) << " ";
        }
      }
      o << std::endl;
    }

    return(o);
  }

}  //  end namespace Isis
