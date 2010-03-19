/**
 * @file
 * $Revision: 1.6 $
 * $Date: 2009/10/30 17:53:07 $
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

#include "ControlMeasure.h"
#include "SpecialPixel.h"
#include "Camera.h"
#include "Application.h"
#include "iTime.h"


namespace Isis {
  //! Create a control point measurement
  ControlMeasure::ControlMeasure() {
    SetType(Unmeasured);
    SetCoordinate(0.0,0.0);
    SetDiameter(Isis::Null);
    SetCubeSerialNumber("");
    SetDateTime("");
    SetChooserName("");
    SetIgnore(false);
    SetError(0.0,0.0);
    SetGoodnessOfFit(Isis::Null);
    SetZScores(Isis::Null, Isis::Null);
    SetReference(false);
    SetCamera(0);
  }

 /**
  * Loads a PvlGroup into the ControlMeasure
  *
  * @param p PvlGroup containing ControlMeasure information
  *
  * @throws Isis::iException::User - Invalid Measure Type
  */
  void ControlMeasure::Load(PvlGroup &p) {
    SetCubeSerialNumber((std::string)p["SerialNumber"]);
    std::string type = p["MeasureType"];
    MeasureType mType;
    if (type == "Unmeasured") mType = Unmeasured;
    else if (type == "Manual") mType = Manual;
    else if (type == "Estimated") mType = Estimated;
    else if (type == "Automatic") mType = Automatic;
    else if (type == "ValidatedManual") mType = ValidatedManual;
    else if (type == "ValidatedAutomatic") mType = ValidatedAutomatic;
    else {
      std::string msg = "Invalid Measure Type, [" + type + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    SetType(mType);
    if (mType != Unmeasured) {
     SetCoordinate(p["Sample"],p["Line"]);
     SetError(p["ErrorSample"],p["ErrorLine"]);
     ErrorMagnitude();
    }
    if (p.HasKeyword("Diameter")) p_diameter = p["Diameter"];
    if (p.HasKeyword("DateTime")) p_dateTime = (std::string)p["DateTime"];
    if (p.HasKeyword("ChooserName")) p_chooserName = (std::string)p["ChooserName"];
    if (p.HasKeyword("Ignore")) p_ignore = true;
    if (p.HasKeyword("GoodnessOfFit")) p_goodnessOfFit = p["GoodnessOfFit"];
    if (p.HasKeyword("Reference")) p_isReference = ((std::string)p["Reference"] == "True");

    if (p.HasKeyword("ZScore")) SetZScores(p["ZScore"][0], p["ZScore"][1]);
  }

 /**
  * Sets up and returns a PvlGroup for the ControlMeasure
  *
  * @return The PvlGroup for the ControlMeasure
  *
  * @throws Isis::iException::Programmer - Invalid Measure Enumeration
  */
  PvlGroup ControlMeasure::CreatePvlGroup() {
    PvlGroup p("ControlMeasure");
    p += PvlKeyword("SerialNumber", p_serialNumber);

    if (p_measureType == Unmeasured) {
      p += PvlKeyword("MeasureType", "Unmeasured");
    }
    else if (p_measureType == Manual) {
      p += PvlKeyword("MeasureType", "Manual");
    }
    else if (p_measureType == Estimated) {
      p += PvlKeyword("MeasureType", "Estimated");
    }
    else if (p_measureType == Automatic) {
      p += PvlKeyword("MeasureType", "Automatic");
    }
    else if (p_measureType == ValidatedManual) {
      p += PvlKeyword("MeasureType", "ValidatedManual");
    }
    else if (p_measureType == ValidatedAutomatic) {
      p += PvlKeyword("MeasureType", "ValidatedAutomatic");
    }
    else {
      std::string msg = "Invalid Measure Enumeration, [" + iString(p_measureType) + "]";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    if (p_measureType == Unmeasured) {
      p += PvlKeyword("Sample", "Null");
      p += PvlKeyword("Line", "Null");
    }
    else {
      p += PvlKeyword("Sample", p_sample);
      p += PvlKeyword("Line", p_line);
      p += PvlKeyword("ErrorLine", p_lineError);
      p += PvlKeyword("ErrorSample", p_sampleError);
      p += PvlKeyword("ErrorMagnitude", ErrorMagnitude());
    }

    if(p_zScoreMin != Isis::Null && p_zScoreMax != Isis::Null) {
      PvlKeyword zscores("ZScore");
      zscores += p_zScoreMin;
      zscores += p_zScoreMax;
      p += zscores;
    }

    if (p_diameter != Isis::Null) p += PvlKeyword("Diameter", p_diameter);
    if (p_dateTime != "") p += PvlKeyword("DateTime", p_dateTime);
    if (p_chooserName != "") p += PvlKeyword("ChooserName", p_chooserName);
    if (p_ignore == true) p += PvlKeyword("Ignore", "True");
    if (p_goodnessOfFit != Isis::Null) {
      p += PvlKeyword("GoodnessOfFit", p_goodnessOfFit);
    }
    if (IsReference()) p += PvlKeyword("Reference", "True");
    else p += PvlKeyword("Reference", "False");

    return p;
  }


  //! Return error magnitude
  double ControlMeasure::ErrorMagnitude() const {
    double dist = (p_lineError * p_lineError) + (p_sampleError * p_sampleError);
    return sqrt(dist);
  }
  
  
  //! Set date/time the coordinate was last changed to the current date/time
  void ControlMeasure::SetDateTime() {
    p_dateTime = iTime::CurrentLocalTime();
  };
 
  
  //! Set chooser name to a user who last changed the coordinate
  void ControlMeasure::SetChooserName() {
    p_chooserName = Application::UserName();
  };

  
  //! Set the focal plane x/y for the measured line/sample
  void ControlMeasure::SetFocalPlaneMeasured(double x, double y) {
    p_focalPlaneMeasuredX = x;
    p_focalPlaneMeasuredY = y;
  }


  //! Set the focal plane x/y for the computed (apriori) lat/lon
  void ControlMeasure::SetFocalPlaneComputed(double x, double y) {
    p_focalPlaneComputedX = x;
    p_focalPlaneComputedY = y;
  }
  

  //! One Getter to rule them all
  const double ControlMeasure::GetMeasureData(QString data) const {
    if (data == "ZScoreMin")
      return p_zScoreMin;
    else if (data == "ZScoreMax")
      return p_zScoreMax;
    else if (data == "SampleError")
      return p_sampleError;
    else if (data == "LineError")
      return p_lineError;
    else if (data == "ErrorMagnitude")
      return ErrorMagnitude();
    else if (data == "Type")
      return p_measureType;
    else if (data == "IsMeasured")
      return IsMeasured();
    else if (data == "IsValidated")
      return IsValidated();
    else if (data == "Ignore")
      return p_ignore;
    else if (data == "GoodnessOfFit")
      return p_goodnessOfFit;
    else
    {
      std::string msg = data.toStdString();
      msg += " passed to GetMeasureData but is invalid";
      throw Isis::iException::Message(Isis::iException::Programmer, msg,
                                      _FILEINFO_);
    }
  }

  
  //! Returns a list of all valid options to pass to GetMeasureData
  const QVector< QString > ControlMeasure::GetMeasureDataNames() {
    QVector< QString > names;
    
    names.push_back("ZScoreMin");
    names.push_back("ZScoreMax");
    names.push_back("SampleError");
    names.push_back("LineError");
    names.push_back("ErrorMagnitude");
    names.push_back("Type");
    names.push_back("IsMeasured");
    names.push_back("IsValidated");
    names.push_back("Ignore");
    names.push_back("GoodnessOfFit");
    
    return names;
  }
  
}
