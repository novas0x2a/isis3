/**
 * @file
 * $Revision: 1.1 $
 * $Date: 2008/11/06 00:00:37 $
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

#include <cmath>  //use ceiling and floor functions
#include <vector>  
#include "Application.h"
#include "Brick.h"
#include "CisscalFile.h"
#include "CissLabels.h"
#include "Cube.h"
#include "DarkCurrent.h"
#include "NumericalApproximation.h"
#include "Message.h"
#include "Preference.h"
#include "Progress.h"
#include "Pvl.h"
#include "SpecialPixel.h"
#include "iException.h"
#include "iString.h"

using namespace std;
namespace Isis {
  /** 
  * Constructs a DarkCurrent object.  Sets class variables. 
  * 
  * @param cissLab <B>CissLabels</B> object from Cassini ISS cube 
  * @throws Isis::iException::Pvl If the input image has an 
  *                   invalid InstrumentDataRate or SummingMode.
  */
  DarkCurrent::DarkCurrent(Isis::CissLabels &cissLab){
    p_compType = cissLab.CompressionType();
    p_dataConvType = cissLab.DataConversionType();
    p_expDur = cissLab.ExposureDuration();
    p_gainMode = cissLab.GainModeId();
    p_narrow = cissLab.NarrowAngle();
    p_sum = cissLab.SummingMode();

    if (cissLab.ReadoutCycleIndex() == "Unknown") {
      p_readoutIndex = -999;
    }
    else{
      p_readoutIndex = cissLab.ReadoutCycleIndex().ToInteger();
    }

    if (p_compType == "NotCompressed") {
      p_compRatio = 1.0;
    }
    else {
      p_compRatio = cissLab.CompressionRatio().ToDouble();
    }

    if (cissLab.DelayedReadoutFlag() == "No"){
      p_btsm = 0;
    }
    else if (cissLab.DelayedReadoutFlag() == "Yes"){
      p_btsm = 1;
    }
    else{ 
      p_btsm = -1;
    }

    double instDataRate = cissLab.InstrumentDataRate();
    if (instDataRate >=  60.0 && instDataRate <=  61.0) p_telemetryRate = 8;
    else if (instDataRate >= 121.0 && instDataRate <= 122.0) p_telemetryRate = 16;
    else if (instDataRate >= 182.0 && instDataRate <= 183.0) p_telemetryRate = 24;
    else if (instDataRate >= 243.0 && instDataRate <= 244.0) p_telemetryRate = 32;
    else if (instDataRate >= 304.0 && instDataRate <= 305.0) p_telemetryRate = 40;
    else if (instDataRate >= 365.0 && instDataRate <= 366.0) p_telemetryRate = 48;
    else throw iException::Message(iException::Pvl, 
                                   "Input file contains invalid InstrumentDataRate. See Software Interface Specification (SIS), Version 1.1, page 31.", 
                                   _FILEINFO_);

    p_readoutOrder = cissLab.ReadoutOrder();

    switch(p_sum.ToInteger()) {
      case 1: p_lines   = 1024;break;
      case 2: p_lines   = 512; break;
      case 4: p_lines   = 256; break;
      default: throw iException::Message(iException::Pvl, 
                                         "Input file contains invalid SummingMode. See Software Interface Specification (SIS), Version 1.1, page 31.", 
                                         _FILEINFO_);
    }
    p_samples = p_lines;
    p_startTime.resize(p_samples);
    p_endTime.resize(p_samples);
    p_duration.resize(p_samples);
    for (int i = 0; i < p_samples; i++) {
      p_startTime[i].resize(p_lines);
      p_endTime[i].resize(p_lines);
      p_duration[i].resize(p_lines);
    }
  }//end constructor
  

  /** 
  * @brief Compute dark current values to subtract from image 
  *  
  * This method computes the dark current DN values to be 
  * subtracted from each pixel and returns those values in an 
  * array of the equal dimension to the image. 
  *  
  * @returns <b>vector<vector<double>></b> Final array of dark 
  *         current DNs to be subtracted
  * @throws Isis::iException::Pvl If the input image has an 
  *             unknown ReadoutCycleIndex or DelayedReadoutFlag.
  * @throws Isis::iException::Pvl If the input image has an 
  *             invalid GainModeId.
  * @throws Isis::iException::Math If MakeDarkArray() returns a
  *             vector of zeros.
  * @see MakeDarkArray()
  */
  vector <vector <double> > DarkCurrent::ComputeDarkDN(){//get dark_DN
    vector <vector <double> > dark_e(p_samples, p_lines), dark_DN(p_samples, p_lines);
    if(p_readoutIndex == -999){
      throw iException::Message(iException::Pvl, 
                                "Readout cycle index is unknown.", 
                                _FILEINFO_);
    }
    if(p_btsm == -1) {
      throw iException::Message(iException::Pvl, 
                                "Delayed readout flag is unknown.", 
                                _FILEINFO_);
    }
    int notzero = 0;
       // create new file
    dark_e = DarkCurrent::MakeDarkArray();
    for(unsigned int i = 0; i < dark_e.size(); i++){
      for(unsigned int j = 0; j < dark_e[0].size(); j++){
        if(dark_e[i][j] != 0.0){
          notzero++;
        }
      }
    }
    if( notzero != 0 ) {               
      // correct for gain:
      double gain2, gainRatio;
      if(p_narrow){
        gain2 = 30.27;
        switch(p_gainMode){ // GainState()){
          case 215: //gs = 0:
            gainRatio = 0.135386;
            break;
          case 95: //1:
            gainRatio = 0.309569;
            break;
          case 29: //2:
            gainRatio = 1.0;
            break;
          case 12: //3:
            gainRatio = 2.357285;
            break;
          default: throw iException::Message(iException::Pvl, 
                                             "Input file contains invalid GainModeId. See Software Interface Specification (SIS), Version 1.1, page 29.", 
                                             _FILEINFO_);
        }
      }
      else {
        gain2 = 27.68;
        switch(p_gainMode){ // GainState()){
          case 215://0:
            gainRatio = 0.125446;
            break;
          case 95://1:
            gainRatio = 0.290637;
            break;
          case 29://2:
            gainRatio = 1.0;
            break;
          case 12://3:
            gainRatio = 2.360374;
            break;
          default: throw iException::Message(iException::Pvl, 
                                             "Input file contains invalid GainModeId. See Software Interface Specification (SIS), Version 1.1, page 29.", 
                                             _FILEINFO_);
        }
      }
      for(unsigned int i = 0; i < dark_e.size(); i++){
        for(unsigned int j = 0; j < dark_e[0].size(); j++){
          dark_DN[i][j] = (dark_e[i][j] /(gain2/gainRatio));
        }
      }
      return dark_DN;
    }
    else {
      throw iException::Message(iException::Math, 
                                "Error in dark simulation; dark array conatains all zeros.", 
                                _FILEINFO_);
    }
  }//end ComputeDarkDN


  /** 
  * @brief Compute time spent on CCD for given line 
  *  
  * Compute the line-time from light-flood erase to read out for a 
  * given image line.  Returns the real seconds that the given 
  * line spends on the CCD. The parameter <B>lline</B> must be 
  * between 1 and 1024. 
  *  
  * @param lline Current line should range from 1 to 1024
  * @returns <b>double</b> Time in seconds spent on CCD for given 
  *        line
  * @throws Isis::iException::Programmer If the input parameter is
  *             out of valid range.
  * @throws Isis::iException::Pvl If the input image contains an 
  *             invalid number of lines.
  * @throws Isis::iException::Pvl If the input image has an 
  *             invalid ReadoutCycleIndex.
  */
  double DarkCurrent::ComputeLineTime(int lline){ //returns the time for this line, takes the line number
    double linetime;
    double tlm = p_telemetryRate/8;
    double t0 = p_expDur/1000 + 0.0004 + 0.0086;		//time from erase to first line
    t0 = t0 + 0.68*(p_lines - lline)/((double) p_lines);
              //time due to 680ms erase
    int line = lline-1; 			//lline is from 1 to 1024 => line is from 0 to 1023
    if (line < 0 || line > 1023){
      iString msg = "DarkCurrent: For ComputeLineTime(lline), lline must be between 1 and 1024."+iString(lline)+" out of range";
      throw iException::Message(iException::Programmer,msg.c_str() , _FILEINFO_);
    }
    if(p_compType == "Lossy" ) {
      double r1 = 0;
      switch(p_lines) {
        case 256:  r1 = 89.864;  break;
        case 512:  r1 = 110.248; break;
        case 1024: r1 = 203.984; break;
        default: throw iException::Message(iException::Pvl, 
                                           "Input file contains invalid number of lines. See Software Interface Specification (SIS), Version 1.1, page 50.", 
                                           _FILEINFO_);
      }
      double linetime = t0 + line/r1;
      return linetime;
    }
    int cindex;
    double data;
    if (p_dataConvType == "12Bit"){ 
      cindex = 0;
      data = 16.0;
    }
    else {
      cindex = 1;
      data = 8.0;
    }
    //!!!!!!  THIS PART SHOULD BE UPDATED WHEN BETTER INFO EXISTS !!!!!
    double blocksize = p_lines*data/16.0/p_compRatio;

    double ro_ratefit, r1, r0;
    switch(p_lines){
      case 1024: {
        double ro_rate0;
        if(p_compType == "NotCompressed" ) {
          if(cindex == 0) ro_rate0 = 67.443;
          else ro_rate0 = 71.946;
          ro_ratefit = ro_rate0;
        } 
        else {
          double slope;
          if(cindex == 0) {
              ro_rate0 = 67.440;
              slope = 1.77299;
          }
          else{
              ro_rate0 = 74.861;
              slope = 0.488212;
          }
          ro_ratefit = ro_rate0 + slope*p_compRatio;
        }
        break;
      }

      case 512: {
         ro_ratefit =  94.443 -0.0172194*blocksize;
         break;
      }

      case 256: {
         ro_ratefit = 156.430 -0.0485036*blocksize;
         break;
      }
      default: throw iException::Message(iException::Pvl, 
                                         "Input file contains invalid number of lines. See Software Interface Specification (SIS), Version 1.1, page 50.", 
                                         _FILEINFO_);
    }
    //!!!!!!  THIS PART SHOULD BE UPDATED WHEN BETTER INFO EXISTS !!!!!
    if (p_compType == "NotCompressed") {
       r1 = ro_ratefit * (1.0+ (.017)*((48.0-p_telemetryRate)/48.0));
       r0 = ro_ratefit * 1.017;
    }
    else {
      r1 = ro_ratefit * (1.0- (.0087)*((48.0-p_telemetryRate)/48.0));
      r0 = ro_ratefit * 0.9913;
    }
    double tratio = p_compRatio;
    if (p_compType == "Lossless" && tratio < 2.0){
      tratio = 2.0;
    }
    int biu_line = 440/(4+((int) (p_lines*data/16/tratio))) + 1;

    // if camera is opposite of read_out_order (second)
    // Calculate number of lines read in early pad of 0.262 seconds
    // BIU swap occurs after these number of lines or when
    // first science packet is complete (at biu_line) which
    // ever is greater
    bool second = false;

    if (p_narrow && p_readoutOrder == 1){
      second = true;
    }
    else if (!p_narrow && p_readoutOrder == 0){
      second = true;
    }
     // First line after biu wait is at 0.289 seconds
    double biutime = 0.289;
    if (second && p_btsm == 0) {
      int early_lines = ((int) (0.262*r0)) + 1;
      if (early_lines > biu_line){
        biu_line = early_lines;
      }
      biutime = 0.539;
    }
    double rate;
    if(p_lines < 1024 ) {
      rate = r1;
      if(p_btsm == 1) {
        rate = r0;
      }
      if(p_btsm == 0 && line >= biu_line) {
        linetime = t0 + biutime + (line-biu_line)/rate;
      }
      else{
        linetime = t0 + line/rate;
      }
      return linetime;
    }
    // Only FULL images can fill image buffer
    double r2 = 3.59733*tlm;             // ITL measured
    if (p_dataConvType != "12Bit"){
      r2 = r2 * 2.0;
    }
    // r2 depends on compression ratio: r2*tratio, but not faster than r1
    if (p_compType == "Lossless"){
      if (r1 < r2*tratio){
        r2 = r1;
      }
      else{
        r2 = r2*tratio;
      }
    }
    int buffer;
    if (p_dataConvType == "12Bit") {
      buffer = 335;
    }
    else {
      buffer = 671;
    }
    // Stores 2 compressed lines into one
    if (p_compType == "Lossless") {
      buffer = 2*buffer;
    }
    // Now treat more complicated 1x1 case.
    if(p_btsm == 0) {
      // Calculate line break
      int line_break;
      if (r2 >= r1) {
        line_break = 1024;  
      }
      else {
        line_break = biu_line + ((int) (buffer*r1/(r1-r2))) + 1;
      }
      linetime = t0 + line/r1;
      if(line >= biu_line && line < line_break ) {
          linetime = t0 + biutime + (line-biu_line)/r1;
      }
      if(line >= line_break ){
        linetime = t0 + biutime + (line_break-biu_line)/r1 + (line-line_break)/r2;
      }
    } 
    else {//p_btsm == 1
    // t1 is amount of time botsim image waits for first image readout window
    // t1 only depends on readout index
      int readout;
      switch((int) p_readoutIndex/4){
        case 0: readout = 50; break;
        case 1: readout = 25; break;
        case 2: readout = 14; break;
        case 3: readout = 6;  break;
        default: throw iException::Message(iException::Pvl,
                                           "Input file contains invalid ReadoutCycleIndex. See Software Interface Specification (SIS), Version 1.1, page 40.", 
                                           _FILEINFO_);
      }
      double t1;
      if (readout*(6.0/((double) (tlm))) - ((int) readout*(6.0/((double) (tlm)))) < .5) {
        t1 = floor(readout*(6.0/((double) (tlm)))) + 0.539;
      }
      else{
        t1 = ceil(readout*(6.0/((double) (tlm)))) + 0.539;
      }
      linetime = t0 + line/r0;
      int line_break = buffer + biu_line; // Full buffer
      // NotCompressed 12Bit always stops and waits when buffer filled
      if (p_dataConvType == "12Bit" && p_compType == "NotCompressed") {
        if (line > line_break){
          linetime = t0 + t1 + (line-line_break)/r2;
        }
        return linetime; 
      }
      // Line at which transmission starts
      int trans_line = ((int) ((t1-0.25)*r0)) + 1;
      if (p_dataConvType != "12Bit" && p_compType == "NotCompressed") {
        if (trans_line >= line_break) {
          if (line >= line_break){
            linetime = t0 + t1 + (line-line_break)/r2; // waits
          }
        } 
        else {
          if (r2 >= r1) {
            line_break = 1024;
          }
          else{
            line_break = trans_line + ((int) ((line_break-trans_line)*r1/(r1-r2))) + 1;
          }
          if (line >= trans_line){
            linetime = t0 + trans_line/r0 + (line-trans_line)/r1 + 0.25;
          }
          if (line >= line_break) {
            linetime = t0 + trans_line/r0 + (line_break-trans_line)/r1 + (line-line_break)/r2 + 0.25;
          }
        }
        return linetime;
      }
      if (p_dataConvType == "12Bit" && p_compType == "Lossless") {
        if (trans_line >= line_break) {
          if (line >= line_break){
            linetime = t0 + t1 + (line-line_break)/r2; // waits
          }
        }
        else {
          if (r2 >= r1) {
            line_break = 1024;
          }
          else{
            line_break = trans_line + ((int) ((line_break-trans_line)*r1/(r1-r2))) + 1;
          }
          if (line >= trans_line){
            linetime = t0 + trans_line/r0 + (line-trans_line)/r1 + 0.25;
          }
          if (line >= line_break){
            linetime = t0 + trans_line/r0 + (line_break-trans_line)/r1 + (line-line_break)/r2 + 0.25;
          }
        }
        return linetime;
      }
      // LOSSLESS with 8LSB or TABLE fits in image memory
      if (p_dataConvType != "12Bit" && p_compType == "LOSSLESS" && line > trans_line) {
        linetime = t0 + trans_line/r0 + (line-trans_line)/r1 + 0.25;
      }
    }
    return linetime;
  }// end ComputeLineTime


  /** 
  * @brief Find dark current files for this image. 
  *  
  * Determines which dark parameters file and bias distortion 
  * table, if any, should be used for this image and assigns these
  * filenames to p_dparamfile and p_bdpath, respectively. These 
  * are dependent on the Instrument ID (ISSNA or ISSWA) and 
  * Instrument Mode ID (Full, Sum2, or Sum4). 
  *  
  * @see DarkParameterFile() 
  * @see BiasDistortionTable() 
  *  
  */
  void DarkCurrent::FindDarkFiles(){
  // Get the directory where the CISS darkcurrent directory is
    PvlGroup &dataDir = Preference::Preferences().FindGroup("DataDirectory");
    iString missionDir = (string) dataDir["Cassini"];
    iString darkDir(missionDir+"/calibration/darkcurrent/");
  
    iString instrumentId ("");
  
    if(p_narrow){
      instrumentId += "nac";
      p_bdpath = darkDir + "nac_bias_distortion.tab";
    }
    else {
      instrumentId += "wac";
    }
    iString instModeId("");
    if(p_sum.ToInteger() > 1 ){
      instModeId = instModeId + "sum" + p_sum;
    }
    else{
      instModeId += "full";
    }
    p_dparamfile = darkDir + instrumentId + "_median_dark_parameters" + "?????" + "." + instModeId + ".cub";
    p_dparamfile.HighestVersion();
  }//end FindDarkFiles


  /** 
  * Computes begin time, end time, and duration for each pixel of 
  * the image. 
  *  
  * @see ComputeLineTime() 
  * 
  */
  void DarkCurrent::ComputeTimeArrays(){
    int numberNegTime = 0;
    vector <double> timeToRead(p_lines);
    for(int i = 0; i < p_lines; i++ ){
        timeToRead[i] = ComputeLineTime(i+1);
        if (timeToRead[i] < 0){
          numberNegTime++;
        }
    }
    if (numberNegTime > 0 ) return;
    for(int i = 0;i < p_lines; i++){
      for(int j = 0; j <= i; j++){
        p_endTime[i][j] = timeToRead[i-j];
      }
      for(int j = 0; j <= i; j++){
        if (j < i ){ 
          p_startTime[i][j] = p_endTime[i][j+1];
        } 
        else {
          p_startTime[i][j] = 0.0;
        }
        p_duration[i][j] = p_endTime[i][j]-p_startTime[i][j];
      }
    }
    for (int i = 0; i < p_lines; i++) {
      for(int j = 0; j < p_samples; j++) {
        if (p_duration[i][j] <= 0) {
          p_duration[i][j] = 0;
          //I belive this is equivalent to the IDL code :
          //     p_duration(*,*) = p_duration(*,*) > 0.0
        }
      }
    }
    for (int i = 0; i < p_lines; i++) {
      for(int j = 0; j < p_samples; j++) {
        p_endTime[i][j] = p_startTime[i][j] + p_duration[i][j];
      }
    }
  }//end ComputeTimeArrays


 /** 
  * @brief Creates dark array 
  * This method reads in the coefficients from the dark 
  * parameters file, calls MakeManyLineDark() to create dark_e,
  * removes artifacts of this array be taking the median of every 
  * 5 values, and corrects for the average bias distortion at the 
  * beginning of each line. 
  * 
  * @returns <b>vector<vector<double>></b> Secondary dark array 
  *         removed of artifacts and corrected for average bias
  *         distortion.
  *  @throws Isis::iException::Io If the dark parameter file or
  *              bias distortion table is not found.
  *  @throws Isis::iException::Io If p_startTime equals p_endTime
  *              for all pixels.
  *  @see FindDarkFiles()
  *  @see ComputeTimeArrays()
  *  @see MakeManyLineDark()
  */
  vector < vector <double> > DarkCurrent::MakeDarkArray(){//return dark_e
    vector < vector <double> > dark_e(p_samples, p_lines);
    FindDarkFiles();    
    if ( !p_dparamfile.Exists()) {
      throw iException::Message(iException::Io,
                                "DarkParameterFile ***"
                                + p_dparamfile.Expanded() + "*** not found.", _FILEINFO_);
    }
    if (p_narrow && (!p_bdpath.Exists())) {
      throw iException::Message(iException::Io,
                                "BiasDistortionFile ***"
                                + p_bdpath.Expanded() + "*** not found.", _FILEINFO_);
    }
    ComputeTimeArrays();//fill in values for p_startTime, p_endTime, p_duration
    int good = 0;
    for (int i = 0; i < p_lines; i++) {
      for (int j = 0; j < p_samples; j++) {
        if (p_startTime[i][j] != p_endTime[i][j]) {
          good++;
        }
      }
    }
    if( good != 0 ) {
      //read the coefficient cube into a Brick
      Brick *darkCoefficients;
      Cube dparamCube;
      dparamCube.Open(p_dparamfile.Expanded());
      darkCoefficients = new Brick(p_samples,p_lines,8,dparamCube.PixelType());
      darkCoefficients->SetBasePosition(1,1,1);
      dparamCube.Read(*darkCoefficients);
      dparamCube.Close();
    // Assume WAC dark current is 0 for 0.005 ms. This is not the case for
    // the NAC where there are negative values near the left edge of the frame:
      if( !p_narrow ) {
        for(int i = 0; i < p_lines; i++){
          for(int j = 0; j < p_samples; j++){
              (*darkCoefficients)[darkCoefficients->Index(i+1,j+1,1)] = 0.0;
          }
        }
      }
      // add functionality for summed images:
      dark_e = MakeManyLineDark(*darkCoefficients);
    // Median-ed dark images have some spikes below the fitted curve.
    // These are probably artifacts and the next section removes them.
      vector <vector <double> > di1(p_samples, p_lines);
      vector <double> neighborhood(5);
      //replace each value of di1 with the median of neighborhood of 5 values
      for(int i = 0; i < p_lines; i++ ){
        for(int j = 0; j < p_samples; j++) {
          if (j < 2 || j > (p_samples-3)) {
            di1[i][j] = dark_e[i][j];
          }
          else{
            for (int n = -2; n < 3; n++) {
              neighborhood[n+2] = dark_e[i][j+n];
            }
            //sort these five values
            sort(neighborhood.begin(),neighborhood.end());
              for(int f = 0; f < 5; f++) {
            }
            //set equal to median
            di1[i][j] = neighborhood[2];
          }
        }
      }
      for (int i = 0; i < p_lines; i++) {
        for (int j = 0; j < p_samples; j++) {
          if (di1[i][j] - dark_e[i][j] > 10) {
            dark_e[i][j] = di1[i][j];
          }
        }
      }
    // correct for the average bias distortion at the beginning of each line:
      if( p_narrow ) {   
        CisscalFile *biasDist = new CisscalFile(p_bdpath.Expanded());
        vector<double> samp, bias_distortion;
        for(int i = 0; i < biasDist->LineCount(); i++){
          iString line;
          biasDist->GetLine(line);  //assigns value to line
          line = line.ConvertWhiteSpace();
          line = line.Compress();
          line = line.TrimHead(" ");
          if (line == "") {
            break;
          }
          samp.push_back(line.Token(" ").ToDouble());
          bias_distortion.push_back(line.Trim(" ").ToDouble());
        }
        biasDist->Close();
        for(int i = 0; i < 21; i++ ){
          for(int j = 0; j < p_lines; j++ ){
            dark_e[i][j] = dark_e[i][j] - bias_distortion[i];
          }
        }
      }
      return dark_e;
    }
    throw iException::Message(iException::Io,
                              "StartTime == EndTime for all pixels.", 
                              _FILEINFO_);
  }//end MakeDarkArray

  /** 
  * @brief Creates preliminary dark array from dark parameters 
  * file and line-time information 
  *  
  * Compute one line of a synthetic dark frame from timing tables 
  * and dark current parameters for each pixel using a spline 
  * interpretation method and evaluating at the start and end 
  * times for that pixel. 
  * 
  * @param darkBrick Containing the coefficients found in the dark
  *                  parameters file for each pixel.
  *  
  * @returns <b>vector<vector<double>></b> Preliminary dark array 
  *         using the darkBrick values
  */
  vector <vector <double> > DarkCurrent::MakeManyLineDark(Brick &darkBrick){//returns dark_e
    int num_params = 8;
    vector <vector <double> > dark(p_samples,p_lines);
    vector <vector <double> > v1(num_params,num_params);
    vector <double> temp(p_samples),tgrid(num_params);
    vector <double> c(2), timespan(2);
    for (int i = 0; i < num_params; i++) {
      switch(i){
        case 0:tgrid[i] = 0.0;    break;
        case 1:tgrid[i] = 10.0;   break;
        case 2:tgrid[i] = 32.0;   break;
        case 3:tgrid[i] = 100.0;  break;
        case 4:tgrid[i] = 220.0;  break;
        case 5:tgrid[i] = 320.0;  break;
        case 6:tgrid[i] = 460.0;  break;
        case 7:tgrid[i] = 1200.0; break;
        default: tgrid[i] = Null;
      }
    }
    for(int j = 0; j < num_params; j++){
      v1[j][j] = 1.0;
    }   
    Progress progress;
    progress.SetText("Computing dark current array...");
    progress.SetMaximumSteps(p_lines);
    progress.CheckStatus();
    for (int jline = 0; jline < p_lines; jline++){
      for(int i = 0; i < p_samples; i++) {
        temp[i] = darkBrick[darkBrick.Index(i+1,jline+1,1)]; // constant term
      }
      // sum the contribution from every pixel downstream of jline, including jline
      for (int kline = 0; kline <=jline; kline++){  
        // derive coefficients so that parameters can be multiplied and added
        // rather than interpolated
        timespan[0] = p_startTime[jline][kline];
        timespan[1] = p_endTime[jline][kline];
          //	Interpolate by fitting a cubic spline to the
          //	  4 point neighborhood (x[i-1], x[i], x[i+1], x[i+2]) surrounding
          //	  the interval, x[i] <= u < x[i+1].
        NumericalApproximation spline(NumericalApproximation::CubicNeighborhood);
        for (int j = 0; j < num_params; j++){
          spline.AddData(tgrid,v1[j]);
          //spline.Compute();
          c = spline.Evaluate(timespan);
          spline.Reset();
          c[0] =  c[1] - c[0];  
          if (c[0] != 0.0){
            for(int i = 0; i < p_samples; i++) {
              temp[i] = temp[i] + c[0]*darkBrick[darkBrick.Index(i+1,kline+1,j+1)];
            }
          }
        }
      }
      for(int i = 0; i < p_samples; i++) {
        dark[i][jline] = temp[i];
      }

      progress.CheckStatus();
    }
    return dark;
  }//end MakeManyLineDark
}//end namespace Isis

