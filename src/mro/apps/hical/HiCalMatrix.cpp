/**                                                                       
 * @file                                                                  
 * $Revision: 1.4 $
 * $Date: 2008/07/15 15:33:25 $
 * $Id: HiCalMatrix.cpp,v 1.4 2008/07/15 15:33:25 skoechle Exp $
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
#include <numeric>
#include <iostream>
#include <sstream>

#include "HiCalMatrix.h"
#include "Pvl.h"
#include "Filename.h"
#include "Cube.h"
#include "Brick.h"
#include "SpecialPixel.h"
#include "iString.h"
#include "iException.h"

#include "SpiceUsr.h"

using namespace std;

namespace Isis {

bool HiCalMatrix::_naifLoaded = false;

/**
 * @brief Default constructor for HiCalMatrix
 */
  HiCalMatrix::HiCalMatrix() : DbAccess() { 
    _profName.clear();
    init(); 
  } 


  /**
   * @brief Construct from a HiRISE label
   * @param label Label from HiRISE cube file
   */
  HiCalMatrix::HiCalMatrix(Pvl &label) : DbAccess() { 
    _profName.clear();
    init(label);
  } 

  /**
   * @brief Construct from HiRISE label and configuration file
   * 
   * @param label Label from HiRISE cube file
   * @param conf Name of configuration file to use
   */
  HiCalMatrix::HiCalMatrix(Pvl &label, const std::string &conf) : 
       DbAccess(Pvl(filepath(conf)).FindObject("Hical", PvlObject::Traverse)) {
    _profName.clear();
    init(label);
  } 

  /**
   * @brief Define label to initialize parameters from
   * 
   * @param label Label from HiRISE cube file
   */
  void HiCalMatrix::setLabel(Pvl &label) {
    init(label);
    return;
  }

  /**
   * @brief Resolve a file path validating existance
   * 
   * This method determines if a filepath is versioned (as indicated with one or
   * more '?') and returns the expanded file name \b only.  It does not expand
   * the path portion of the filespec.  This is so to make use of the results of
   * this method in reporting files in labels...to make it tidy.
   * 
   * @param fname  File specification with or without versioning patterns
   * 
   * @return string Expanded filename but not the filepath
   */
  string HiCalMatrix::filepath(const std::string &fname) const {
    string::size_type start = fname.find_first_of("?");
    if (start != string::npos) {
      Filename efile(fname);

      string path(efile.OriginalPath());
      if (!path.empty()) path += "/";

      efile.HighestVersion();

      return (path + efile.Name());
    }
    return (fname);
  }


  /**
   * @brief Establish the configuration file used for calibration parameters
   * 
   * This file can be established at any point in the processing as parameters
   * are resolved as needed (lazy instantiation).  One must be established
   * before any calibration can take place.
   * 
   * @param conf Name of configuration file to use
   */
  void HiCalMatrix::setConf(const std::string &conf) {
    load(Pvl(filepath(conf)).FindObject("Hical", PvlObject::Traverse));
  }

  /**
   * @brief Selects a profile other than the default
   * 
   * Use of this method is to explicitly select a named profile in the
   * configuration file.  If this is used, additional profile options are not
   * loaded.
   * 
   * @param profile Name of existing profile in the configuration file
   */
  void HiCalMatrix::selectProfile(const std::string &profile) {
    _profName = profile;
    return;
  }

  /**
   * @brief Returns the fully optioned profile name used
   * 
   * This method returns the fully optioned profile used to define calibration
   * processing parameters.  It has the form:
   * @code
   * default+[option1]+[option2]+...+[optionN]
   * @endcode
   * This specifies the name of the default profile and any additional option
   * profile loaded in the indicated order.
   * 
   * @return string Combined profile name
   */
  string HiCalMatrix::getProfileName() const {
    return (getMatrixProfile().Name());
  }

  /**
   * @brief Returns the list of matrices define in the configuration file
   * 
   * This method simply determines the variable names of the calibration
   * matrices contained in the \b Matrices keyword in the \b Hical object in the
   * configuration file.  It does not provide the fully expanded file reference
   * of them, however.  See getMatrixSource for this provision.
   * 
   * @return std::vector<std::string> List of define matrices in the config file
   */
  std::vector<std::string> HiCalMatrix::getMatrixList() const {
   return (getList(getMatrixProfile(), "Matrices"));
  }

  /**
   * @brief Returns the list of scalars define in the configuration file
   * 
   * This method simply determines the variable names of the calibration scalar
   * constants contained in the \b Scalars keyword in the \b Hical object in the
   * configuration file.  It does not provide the values for them, however. See
   * getScalar for this provision.
   * 
   * @return std::vector<std::string> List of define matrices in the config file
   */
  std::vector<std::string> HiCalMatrix::getScalarList() const {
    return (getList(getMatrixProfile(), "Scalars"));
  }

  /**
   * @brief Returns the list of keywords define in the configuration file
   * 
   * This method simply determines the variable names of the calibration label
   * keywords contained in the \b Keywords keyword in the \b Hical object in the
   * configuration file.  It does not provide the values for them, however. See
   * getKeyword for this provision.
   * 
   * @return std::vector<std::string> List of define matrices in the config file
   */
  std::vector<std::string> HiCalMatrix::getKeywordList() const {
    return (getList(getMatrixProfile(), "Keywords"));
  }

  /**
   * @brief Returns the named expanded matrix file reference
   * 
   * This method returns the name of a matrix file reference variable from the
   * \b Matrices keyword as determined from the fully option profile. It then
   * expands the filename portion (not the filepath!) and returns the result as
   * a string for subsequent use.  See also getMatrixList and getMatrix.
   * 
   * @param name  Name of Matrix to resolve.  It must be one in the \b Matrices
   *              keyword from the configuration file
   * 
   * @return std::string The expanded file name reference for the matrix
   */
  std::string HiCalMatrix::getMatrixSource(const std::string &name) const {
    DbProfile matconf = getMatrixProfile();
    std::string mfile = parser(matconf.value(name));
 
//  Translate and return
    return (filepath(mfile));
  }

  /**
   * @brief Returns the named matrix from the specified file reference
   * 
   * The matrix specified in the name parameter must exist in the \b Matrices
   * keyword in the \b Hical object of the configuration file.  A fully option
   * profile is created within this method and the keyword of the named variable
   * is retreived from it.  The fully qualified filepath is generated to
   * finally resolve to a multi-band, single line, Isis cube file.  A specific
   * band is extracted from the cube file as determined by the CCD channel
   * (provide by the getMatrixBand method) and returned in a HiMatrix data
   * array.
   * 
   * Note that the caller can specify the number of samples expected from the
   * matrix cube in the expected_size parameter if it is non-zero.  Once the
   * matrix cube is opened, the number of samples is checked against this
   * parameter and if they do not match, an exception is thrown.
   * 
   * @param name Name of matrix to retreive
   * @param expected_size Expected number of samples in the matrix cube
   * 
   * @return HiCalMatrix::HiMatrix Returns the extracted band from the cube
   */
  HiCalMatrix::HiMatrix HiCalMatrix::getMatrix(const std::string &name,
                                               int expected_size) const {
    std::string mfile = getMatrixSource(name);

// Crack the file and read the appropriate band
    Cube cube;
    cube.Open(mfile);

//  Check for proper size if specifeid
    if (expected_size != 0) {
      if (cube.Samples() != expected_size) {
        ostringstream mess;
        mess << "Specifed matrix  (" << name 
             << ") from file \"" << mfile 
             << "\" does not have expected samples (" << expected_size
             << ") but has " << cube.Samples();
        cube.Close();
        throw iException::Message(iException::User, mess.str(), _FILEINFO_);
      }
    }

//  Read the specifed region
    Brick bandio(cube.Samples(), 1, 1, Real);
    bandio.SetBasePosition(1,1,getMatrixBand());
    cube.Read(bandio);

//  Now create the output buffer with some TNT funny business
    HiMatrix temp(cube.Samples(), bandio.DoubleBuffer());
    HiMatrix out(cube.Samples());
    out.inject(temp);
    cube.Close();
    return (out);
  }

  /**
   * @brief Returns a named scalar constant
   * 
   * This method returns a named scalar constant parameter retrieved from the
   * configuration file through a fully optioned profile.  This keyword does not
   * necessarily have to exist in the \b Scalars keyword in the \b Hical object,
   * but is required to be a floating point value (or values).  The result is
   * returned as a HiMatrix data array.
   * 
   * @param name Name of scalar constant to return
   * @param expected_size Expected size of constant if non-zero
   * 
   * @return HiCalMatrix::HiMatrix Values of scalar constants
   */
  HiCalMatrix::HiMatrix HiCalMatrix::getScalar(const std::string &name,
                                               int expected_size) const {
    DbProfile matconf = getMatrixProfile();
    int nvals = matconf.count(name);

    //  Check for proper size if specifeid
    if (expected_size != 0) {
      if (nvals != expected_size) {
        ostringstream mess;
        mess << "Specifed scalar (" << name 
             << ") does not have expected size (" << expected_size
             << ") but has " << nvals;
        throw iException::Message(iException::User, mess.str(), _FILEINFO_);
      }
    }
//  All is OK
    HiMatrix mtx(nvals, Null);
    for (int i = 0  ; i < nvals ; i++) {
      iString value = matconf(name, i);
      mtx[i] = value.ToDouble();
    }
    return (mtx);
  }


  /**
   * @brief Return the value of a keyword label as a calibration matrix
   * 
   * This method will retreive the values from a HiRISE label keyword and return
   * it as a HiMatrix data array.  Note that the named keyword path must be
   * specified in the Keywords profile as a label keyword name and option group
   * designation if contained in a group.  This is the only way an keyword can
   * be extracted from the label.
   * 
   * Once retreived from the HiRISE label, all its values are converted to
   * double floating point values and returned in a HiMatrix data array.
   * 
   * If expected_size is non-zero, the number of values in the extracted
   * keyword is compared to it.  If the expected_size and number of values
   * differ, an exception is thrown.
   * 
   * @param name  Name of label keyword (reference) to retrieve
   * @param expected_size If non-zero, ensure the number of values are this size
   * 
   * @return HiCalMatrix::HiMatrix Returns the values as a HiMatrix data element
   */
  HiCalMatrix::HiMatrix HiCalMatrix::getKeyword(const std::string &name,
                                                 int expected_size)  {
    DbProfile keyProf = getProfile("Keywords");
    vector<string> keypath = getList(keyProf, name);
    string group = (keypath.size() == 2) ? keypath[1] : "";
    PvlKeyword &key = getKey(name,group);

      //  Check for proper size if specified
    int nvals = key.Size();
    if (expected_size != 0) {
      if (nvals != expected_size) {
        ostringstream mess;
        mess << "Specifed keyword (" << name 
             << ") does not have expected size (" << expected_size
             << ") but has " << nvals;
        throw iException::Message(iException::User, mess.str(), _FILEINFO_);
      }
    }

//  All is OK
    HiMatrix mtx(nvals, Null);
    for (int i = 0  ; i < nvals ; i++) {
      mtx[i] = (double) key[i];
    }
    return (mtx);
  }

  /**
   * @brief Computes the distance from the Sun to the observed body
   * 
   * This method requires the appropriate NAIK kernels to be loaded that
   * provides instrument time support, leap seconds and planet body ephemeris.
   * 
   * @return double Distance in AU between Sun and observed body 
   */
  double HiCalMatrix::sunDistanceAU() {
    loadNaifTiming();

    string scStartTime = getKey("SpacecraftClockStartCount", "Instrument");
    double obsStartTime;
    scs2e_c (-74999,scStartTime.c_str(),&obsStartTime);

    string targetName = getKey("TargetName", "Instrument");
    if ((iString::Equal(targetName, "Sky") ) || 
         (iString::Equal(targetName, "Cal")) ) {
      targetName = "Mars";
    }
    double sunv[3];
    double lt;
    (void) spkpos_c(targetName.c_str(), obsStartTime, "J2000", "LT+S", "sun",
                    sunv, &lt);
    double sunkm = vnorm_c(sunv);
    //  Return in AU units
    return (sunkm/1.49597870691E8);
  }


  /**
   * @brief Returns the band number of a Matrix file given CCD, Channel number
   * 
   * The ordering of HiRISE Isis matrix cubes are required to be ordered by CCD
   * and channel number.  The band number is computed as:
   * @code
   * band =  1 + ccd * 2 + channel
   * @endcode
   * 
   * @return int Band number of matrix cube
   */
  int HiCalMatrix::getMatrixBand() const {
    return (1 + _ccd*2 + _channel); 
  }

  /**
   * @brief Generic profile keyword value extractor
   * 
   * This method retrieves a profile keyword from the given profile and returns
   * all its values a a list of strings.  An exception will be thrown
   * incidentally if the keyword does not exist.
   * 
   * @param profile Profile containing the keyword to extract
   * @param key  Name of keyword to retrieve
   * 
   * @return std::vector<std::string> List of values from profile keyword
   */
  std::vector<std::string> HiCalMatrix::getList(const DbProfile &profile, 
                                                const std::string &key) const {
    std::vector<std::string> slist;

//  Get keyword parameters
    int nvals = profile.count(key);
    for (int i = 0 ; i < nvals ; i++) {
      slist.push_back(profile.value(key, i));
    }
    return (slist);
  }

/**
 * @brief Load required NAIF kernels required for timing needs
 * 
 * This method maintains the loading of kernels for HiRISE timing and planetary
 * body ephemerides to support time and relative positions of planet bodies.
 */
void HiCalMatrix::loadNaifTiming( ) {
  if (!_naifLoaded) {
//  Load the NAIF kernels to determine timing data
    Isis::Filename leapseconds("$base/kernels/lsk/naif????.tls");
    leapseconds.HighestVersion();

    Isis::Filename sclk("$mro/kernels/sclk/MRO_SCLKSCET.?????.65536.tsc");
    sclk.HighestVersion();

    Isis::Filename pck("$base/kernels/spk/de???.bsp");
    pck.HighestVersion();

//  Load the kernels
    string lsk = leapseconds.Expanded();
    string sClock = sclk.Expanded();
    string pConstants = pck.Expanded();
    furnsh_c(lsk.c_str());
    furnsh_c(sClock.c_str());
    furnsh_c(pConstants.c_str());

//  Ensure it is loaded only once
    _naifLoaded = true;
  }
  return;
}

/**
 * @brief Intialization of object variables
 */
void HiCalMatrix::init() {
  _ccd = -1;
  _channel = -1;
  _tdi     = 0;
  _binMode = 0;
  _filter = "_undefined_";
  return;
}

/**
 * @brief Initialization of object using HiRISE label
 * 
 * This method initializes the object from a HiRISE label.  Note that a copy of
 * the label is created so that subsequent operations can be supported and it is
 * not dependant upon callers behaviour.
 * 
 * @param label Pvl label of HiRISE cube
 */
void HiCalMatrix::init(Pvl &label) {
  init();

  _label = label;
  PvlGroup &inst = _label.FindGroup("Instrument", Pvl::Traverse);
  const int cpmm2ccd[] = {0,1,2,3,12,4,10,11,5,13,6,7,8,9};
  _ccd = cpmm2ccd[(int) inst["CpmmNumber"]];
  _channel = inst["ChannelNumber"];
  _tdi     = inst["Tdi"];
  _binMode = inst["Summing"];
  if (_ccd <= 9)       { _filter = "RED"; }
  else if (_ccd <= 11) { _filter = "IR"; }
  else                 { _filter = "BG"; }
  return;
}


/**
 * @brief Get a label keyword
 * 
 * Retreives a keyword from a HiRISE label and returns a reference to it.  If it
 * does not exist, an exception is thrown incidentally.
 * 
 * @param key Name of keyword in label to retrieve
 * @param group Optional group name to use if non-empty.
 * 
 * @return PvlKeyword& Reference to retrieved label keyword
 */
PvlKeyword &HiCalMatrix::getKey(const std::string &key, 
                                 const std::string &group) {
  if (!group.empty()) {
    PvlGroup &grp = _label.FindGroup(group, Pvl::Traverse);
    return (grp[key]);
  }
  else {
    return (_label.FindKeyword(key));
  }
}

/**
 * @brief Returns the Matrix profile
 * 
 * This method constructs a fully optioned matrix profile from the
 * configuration file.  If the caller has designated a specific named profile,
 * the options profiles are not loaded.
 * 
 * Options profiles are read from the \b ProfileOptions configuration keyword
 * and resolved through pattern replacement of FILTER, TDI, BIN, CCD and CHANNEL
 * values are determined from the HiRISE label.  If named profiles exist after
 * the textual substitutions of these values they are added to the base default
 * profile.  This process will replace existing default keywords of the same
 * name in subseqent optional keywords.
 * 
 * @return DbProfile  Returns a fully optioned profile
 */
DbProfile HiCalMatrix::getMatrixProfile() const {
  DbProfile matconf = getProfile(_profName);
  if (!matconf.isValid()) {
    ostringstream mess;
    mess << "Specifed matrix profile (" << matconf.Name() 
         << ") does not exist or is invalid!";
    throw iException::Message(iException::User, mess.str(), _FILEINFO_);
  }  

// If the user has set a specific profile, do not load any additional ones
  if (!_profName.empty()) return (matconf);

//  For the default profile, load any optional ones
  vector<string> proforder = getList(matconf,"ProfileOptions");
  string pName(matconf.Name());
  for (unsigned int i = 0 ; i < proforder.size() ; i++) {
    std::string profile = parser(proforder[i]);
    if (profileExists(profile)) {
      pName += "+[" + profile + "]";
      matconf = DbProfile(matconf,getProfile(profile), pName);
    }
  }

  return (matconf);
}

/**
 * @brief Performs a search and replace operation for the given string
 * 
 * This method will search the input string s for predefined keywords (FILTER,
 * TDI, BIN, CCD and CHANNEL) delimited by {} and replace these occurances with
 * the textual equivalent.
 * 
 * @param s String to conduct search/replace of values
 * 
 * @return std::string  Results of search/replace
 */
std::string HiCalMatrix::parser(const std::string &s) const {
    string sout(s); 

    //  Replace FILTER, CCD, CHANNEL, TDI and BIN modes textually
    iString sRep(_filter);
    sout = iString::Replace(sout, "{FILTER}", sRep);

    sRep = _ccd;
    sout = iString::Replace(sout, "{CCD}", sRep);

    sRep = _channel;
    sout = iString::Replace(sout, "{CHANNEL}", sRep);

    sRep = _tdi;
    sout = iString::Replace(sout, "{TDI}", sRep);

    sRep = _binMode;
    sout = iString::Replace(sout, "{BIN}", sRep);

    return (sout);
}

}  // namespace Isis

