#include <string>
#include "CissLabels.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "SpecialPixel.h"       
#include "iException.h"
#include "iString.h"

using namespace std;
namespace Isis {
 /**
  * Constructs a CissLabels object from an @b Isis::Pvl object.
  *  
  * @param lab @b Pvl  Labels of Cassini ISS file 
  */
  CissLabels::CissLabels(Pvl &lab) {
    Init(lab);
  }
  
 /**
  * Constructs a CissLabels object from a file name. 
  *  
  * @param file Name of Cassini ISS file 
  */
  CissLabels::CissLabels(const string &file) {
    Pvl lab(file);
    Init(lab);
  }
  
 /**
  * General initializer.  Reads labels of the file and computes 
  * values of image properties not already in the labels. 
  *  
  * @param lab @b Pvl Labels of Cassini ISS file 
  * @throws iException::Pvl Not valid Cassini ISS instrument 
  */ 
  void CissLabels::Init(Pvl &lab) {
    try {
      ReadLabels(lab);
      ComputeImgProperties();
    }
    catch (iException &e) {
      e.Report();
      string msg = "Labels do not appear contain a valid Cassini ISS instrument";
      throw iException::Message(iException::Pvl,msg,_FILEINFO_);
    }
  }
  
 /**
  * @brief Reads the @b Pvl Labels 
  *  
  * This method reads the required keywords from the labels and 
  * sets the values of the private variables. 
  *  
  * @param lab @b Pvl Labels of Cassini ISS file 
  */   
  void CissLabels::ReadLabels(Pvl &lab) {
    // Get values out of the instrument group
    PvlGroup &inst    = lab.FindGroup("Instrument",Pvl::Traverse);
    p_ABflag                = (string) inst["AntiBloomingStateFlag"];   //valid values: On, Off, Unknown
    p_biasStripMean         = (double) inst["BiasStripMean"];           //valid values: real numbers
    p_compressionRatio      = (string) inst["CompressionRatio"];        //valid values: NotCompressed or real number
    p_compressionType       = (string) inst["CompressionType"];         //valid values: Lossy, Lossless, NotCompressed
    p_dataConversionType    = (string) inst["DataConversionType"];      //valid values: 12Bit, 8LSB, Table
    p_delayedReadoutFlag    = (string) inst["DelayedReadoutFlag"];      //valid values: Yes, No, Unknown
    p_exposureDuration      = (double) inst["ExposureDuration"];        //valid values: real numbers
    p_flightSoftwareVersion = (string) inst["FlightSoftwareVersionId"]; //valid values: Unknown, 1.2, 1.3, 1.4
    p_gainModeId            = (int)    inst["GainModeId"];              //valid values: 12, 29, 95, 215
    p_gainState             = (int)    inst["GainState"];               //valid values: 0, 1, 2, 3
    p_instrumentDataRate    = (double) inst["InstrumentDataRate"];      //valid values: 60.9, 121.9, 182.8, 243.7, 304.6, 365.6, -999.0
    p_instrumentModeId      = (string) inst["InstrumentModeId"];        //valid values: Full, Sum2, Sum4
    p_instrumentId          = (string) inst["InstrumentId"];            //valid values: ISSNA, ISSWA
    p_readoutCycleIndex     = (string) inst["ReadoutCycleIndex"];       //valid values: Unknown or integers 0-15
    p_readoutOrder          = (int)    inst["ReadoutOrder"];            //valid values: 0 or 1
    p_shutterModeId         = (string) inst["ShutterModeId"];           //valid values: BothSim, NacOnly, WacOnly
    p_summingMode           = (int)    inst["SummingMode"];             //valid values: 1, 2, 4
    p_frontOpticsTemp       = inst["OpticsTemperature"][0].ToDouble();  //valid values: real numbers
    // Get values out of the archive group
    PvlGroup &arch    = lab.FindGroup("Archive",Pvl::Traverse);
    p_imageNumber           = (double) arch["ImageNumber"];
    // Get values out of the bandbin group
    PvlGroup &bandbin = lab.FindGroup("BandBin",Pvl::Traverse);
    iString filter = (string) bandbin["FilterName"];
    p_filter.push_back(filter.Token("/"));
    p_filter.push_back(filter);
  }


 /**
  * @brief Computes values of non-keyword image properties.
  *  
  * This method computes and sets the values of the image 
  * properties that are not keywords in the labels. 
  *  
  */  
  void CissLabels::ComputeImgProperties() {
    //set boolean p_antiblooming if antiblooming flag is on
    if (p_ABflag == "On") p_antiblooming = true;
    else p_antiblooming = false;

    //set boolean p_cissNA if camera type is narrow
    if (p_instrumentId == "ISSNA") p_cissNA = true;
    else p_cissNA = false;

    //set filter 1 and filter 2 indices
    if (p_filter[0] == "UV1")       p_filterIndex.push_back(0);       
    else if (p_filter[0] == "UV2")  p_filterIndex.push_back(1); 
    else if (p_filter[0] == "BL1")  p_filterIndex.push_back(3); 
    else if (p_filter[0] == "RED")  p_filterIndex.push_back(6); 
    else if (p_filter[0] == "IR2")  p_filterIndex.push_back(8); 
    else if (p_filter[0] == "IR4")  p_filterIndex.push_back(10);  
    else if (p_filter[0] == "CL1")  p_filterIndex.push_back(17);  
    else if (p_filter[0] == "HAL")  p_filterIndex.push_back(19);  
    else if (p_filter[0] == "IRP0") p_filterIndex.push_back(20);  
    else if (p_filter[0] == "P0")   p_filterIndex.push_back(21);  
    else if (p_filter[0] == "P60")  p_filterIndex.push_back(22);  
    else if (p_filter[0] == "P120") p_filterIndex.push_back(23);  
    else if (p_filter[0] == "IR3")  p_filterIndex.push_back(24);  
    else if (p_filter[0] == "IR5")  p_filterIndex.push_back(25);  
    else if (p_filter[0] == "CB3")  p_filterIndex.push_back(26);  
    else if (p_filter[0] == "MT3")  p_filterIndex.push_back(27);  
    else if (p_filter[0] == "CB2")  p_filterIndex.push_back(28);  
    else if (p_filter[0] == "MT2")  p_filterIndex.push_back(29);  
    else throw iException::Message(iException::Pvl, "Labels have invalid filter 1 name.  Cannot get filter 1 index.", _FILEINFO_);

    if (p_filter[1] == "UV3")        p_filterIndex.push_back(2); 
    else if (p_filter[1] == "BL2")   p_filterIndex.push_back(4); 
    else if (p_filter[1] == "GRN")   p_filterIndex.push_back(5); 
    else if (p_filter[1] == "IR1")   p_filterIndex.push_back(7); 
    else if (p_filter[1] == "IR3")   p_filterIndex.push_back(9);  
    else if (p_filter[1] == "CB1")   p_filterIndex.push_back(11);  
    else if (p_filter[1] == "CB2")   p_filterIndex.push_back(12);  
    else if (p_filter[1] == "CB3")   p_filterIndex.push_back(13);  
    else if (p_filter[1] == "MT1")   p_filterIndex.push_back(14);  
    else if (p_filter[1] == "MT2")   p_filterIndex.push_back(15);  
    else if (p_filter[1] == "MT3")   p_filterIndex.push_back(16);  
    else if (p_filter[1] == "CL2")   p_filterIndex.push_back(18);  
    else if (p_filter[1] == "RED")   p_filterIndex.push_back(30);  
    else if (p_filter[1] == "BL1")   p_filterIndex.push_back(31);  
    else if (p_filter[1] == "VIO")   p_filterIndex.push_back(32);  
    else if (p_filter[1] == "HAL")   p_filterIndex.push_back(33);  
    else if (p_filter[1] == "IRP90") p_filterIndex.push_back(34);  
    else if (p_filter[1] == "IRP0")  p_filterIndex.push_back(35);  
    else throw iException::Message(iException::Pvl, "Labels have invalid filter 2 name.  Cannot get filter 2 index.", _FILEINFO_);
    return;
  }
}

