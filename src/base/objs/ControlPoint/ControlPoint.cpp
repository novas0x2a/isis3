#include "ControlPoint.h"
#include "SpecialPixel.h"
#include "CameraDistortionMap.h"
#include "iString.h"

namespace Isis {
  //! Construct a control point
  ControlPoint::ControlPoint () {
    SetId("");
    SetType(Tie);
    SetUniversalGround(Isis::Null,Isis::Null,Isis::Null);
    SetIgnore(false);
    SetHeld(false);
  }

  //! Construct a control point with given Id
  ControlPoint::ControlPoint (const std::string &id) {
    SetId(id);
    SetType(Tie);
    SetUniversalGround(Isis::Null,Isis::Null,Isis::Null);
    SetIgnore(false);
    SetHeld(false);
  }

 /**
  * Loads the PvlObject into a ControlPoint
  *
  * @param p PvlObject containing ControlPoint information
  *
  * @throws Isis::iException::User - Invalid Point Type
  * @throws Isis::iException::User - Unable to add ControlMeasure to ControlPoint
  *  
  * @history 2008-06-18  Tracie Sucharski/Jeannie Walldren, Fixed bug with 
  *                              checking for "True" vs "true", change to
  *                              lower case for comparison.
  */
  void ControlPoint::Load(PvlObject &p) {
    SetId(p["PointId"]);
    if (p.HasKeyword("Latitude")) {
      SetUniversalGround(p["Latitude"], p["Longitude"], p["Radius"]);
    }
    if ((std::string)p["PointType"] == "Ground") SetType(Ground);
    else if ((std::string)p["PointType"] == "Tie") SetType(Tie);
    else {
      std::string msg = "Invalid Point Type, [" + (std::string)p["PointType"] + "]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    if (p.HasKeyword("Held")) {
      iString held = (std::string)p["Held"];
      if (held.DownCase() == "true") SetHeld(true);
    }
    if (p.HasKeyword("Ignore")) {
      iString ignore = (std::string)p["Ignore"];
      if (ignore.DownCase() == "true") SetIgnore(true);
    }
    for (int g=0; g<p.Groups(); g++) {
      try {
        if (p.Group(g).IsNamed("ControlMeasure")) {
          ControlMeasure cm;
          cm.Load(p.Group(g));
          Add(cm);
        }
      }
      catch (iException &e) {
        std::string msg = "Unable to add Control Measure to ControlPoint [" +
          Id() + "]";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }
    }
  }

 /**
  * Creates a PvlObject from the ControlPoint
  *
  * @return The PvlObject created
  *
  * @throws Isis::iException::Programmer - Invalid Point Enumeration
  */
  PvlObject ControlPoint::CreatePvlObject() {
    PvlObject p("ControlPoint");
    if (p_type == Ground) {
      p += PvlKeyword("PointType", "Ground");
    }
    else if (p_type == Tie) {
      p += PvlKeyword("PointType", "Tie");
    }
    else {
      std::string msg = "Invalid Point Enumeration, [" + iString(p_type) + "]";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    p += PvlKeyword("PointId", p_id);
    if (p_latitude != Isis::Null && p_longitude != Isis::Null && p_radius != Isis::Null) {
       p += PvlKeyword("Latitude", p_latitude);
       p += PvlKeyword("Longitude", p_longitude);
       p += PvlKeyword("Radius", p_radius);
    }
    if (p_held == true) {
      p += PvlKeyword("Held", "True");
    }
    if (p_ignore == true) {
      p += PvlKeyword("Ignore", "True");
    }

    for (int g=0; g<Size(); g++) {
      p.AddGroup(this->operator[](g).CreatePvlGroup());
    }

    return p;
  }

 /**
  * Add a measurement to the control point
  *
  * @param measure The ControlMeasure to add
  */
  void ControlPoint::Add(const ControlMeasure &measure) {
    for (int i=0; i<Size(); i++) {
      if (this->operator[](i).CubeSerialNumber() == measure.CubeSerialNumber()) {
        std::string msg = "The SerialNumber [";
        msg += measure.CubeSerialNumber() + "] is not unique, another ControlMeasure shares this SerialNumber"; 
        throw iException::Message(iException::Programmer,msg,_FILEINFO_);
      }
    }
    p_measure.push_back(measure);
  }

  /**
   * Remove a measurement from the control point
   *
   * @param index The index of the control point to delete
   */
  void ControlPoint::Delete(int index) {
    p_measure.erase(p_measure.begin()+index);
  }

  /**
   *  Return the measurement for the given serial number
   *
   *  @param serialNumber  The serial number
   *  @return The ControlMeasure corresponding to the give serial number
   */
  ControlMeasure &ControlPoint::operator[](const std::string &serialNumber) {
    for (int m=0; m<this->Size(); m++) {
      if (this->operator[](m).CubeSerialNumber() == serialNumber) {
        return this->operator [](m);
      }
    }
    std::string msg = "Requested serial number [" + serialNumber + "] ";
    msg += "does not exist in this ControlPoint";
    throw iException::Message(iException::User,msg,_FILEINFO_);

  }

  /**
   *  Return the measurement for the given serial number
   *
   *  @param serialNumber  The serial number
   *  @return The ControlMeasure corresponding to the give serial number
   */
  const ControlMeasure &ControlPoint::operator[](const std::string &serialNumber) const{
    for (int m=0; m<this->Size(); m++) {
      if (this->operator[](m).CubeSerialNumber() == serialNumber) {
        return this->operator [](m);
      }
    }
    std::string msg = "Requested serial number [" + serialNumber + "] ";
    msg += "does not exist in this ControlPoint";
    throw iException::Message(iException::User,msg,_FILEINFO_);

  }

  /**
   *  Return true if given serial number exists in point
   *
   *  @param serialNumber  The serial number
   *  @return True if point contains serial number, false if not
   */
  bool ControlPoint::HasSerialNumber(std::string &serialNumber) {
    for (int m=0; m<this->Size(); m++) {
      if (this->operator[](m).CubeSerialNumber() == serialNumber) {
        return true;
      }
    }
    return false;

  }

  /**
   * Set the ground coordinate of a control point
   *
   * @param lat     planetocentric latitude in degrees
   * @param lon     planetocentric longitude in degrees
   * @param radius  radius at coordinate in meters
   */
  void ControlPoint::SetUniversalGround (double lat, double lon, double radius) {
    p_latitude = lat;
    p_longitude = lon;
    p_radius = radius;
  }

  //! Return the average error of all measurements
  double ControlPoint::AverageError() const {
    double cerr = 0.0;
    int count = 0;
    for (int i=0; i<(int)p_measure.size(); i++) {
      if (p_measure[i].Ignore()) continue;
      if (p_measure[i].Type() == ControlMeasure::Unmeasured) continue;
      cerr += p_measure[i].ErrorMagnitude();
      count++;
    }

    if (count == 0) return 0.0;
    return cerr / (double) count;
  }


  /**
   * Return true if there is a Reference measure, otherwise return false
   *
   * @todo  ??? Check for more than one reference measure ???
   *          Should print error, this check should also go in
   *          ReferenceIndex.
   */
  bool ControlPoint::HasReference() {

    if (p_measure.size() == 0) {
      std::string msg = "There are no ControlMeasures in the ControlPoint [" + Id() + "]";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    // Return true if reference measure is found
    for (unsigned int i=0; i<p_measure.size(); i++) {
      if (p_measure[i].IsReference()) return true;
    }
    return false;
  }

  /**
  * Return the index of the reference measurement
  * if none is specified, return the first measured CM
  *
  * @return The PvlObject created
  */
  int ControlPoint::ReferenceIndex() {
    if (p_measure.size() == 0) {
      std::string msg = "There are no ControlMeasures in the ControlPoint [" + Id() + "]";
      throw iException::Message(iException::Programmer,msg,_FILEINFO_);
    }

    // Return the first ControlMeasure that is a reference
    for (unsigned int i=0; i<p_measure.size(); i++) {
      if (p_measure[i].IsReference()) return i;
    }

    // Or return the first measured ControlMeasure
//    for (unsigned int i=0; i<p_measure.size(); i++) {
//      if (p_measure[i].IsMeasured()) return i;
//    }

//    std::string msg = "There are no Measured ControlMeasures in the ControlPoint [" + Id() + "]";
//    throw iException::Message(iException::Programmer,msg,_FILEINFO_);

    std::string msg = "There is no Reference ControlMeasure in the ControlPoint [" + Id() + "]";
    throw iException::Message(iException::Programmer,msg,_FILEINFO_);
  }

  /**
   * This method computes the apriori lat/lon for a point.  It computes this
   * by determining the average lat/lon of all the measures.  Note that this
   * does not change held, ignored, or ground points.  Also, it does not
   * use unmeasured or ignored measures when computing the lat/lon. 
   *  
   * @history 2008-06-18  Tracie Sucharski/Jeannie Walldren, Changed error 
   *                               messages for Held/Ground points. 
   */
  void ControlPoint::ComputeApriori() {
    // Should we ignore the point altogether?
    if (Ignore()) return;

    // Don't goof with ground points.  The lat/lon is what it is ... if
    // it exists!
    if (Type() == Ground) {
      if (p_latitude == Isis::Null ||
          p_longitude == Isis::Null ||
          p_radius == Isis::Null) {
        std::string msg = "ControlPoint [" + Id() + "] is a ground point ";
        msg += "and requires lat/lon/radius";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }
      // Don't return until after the FocalPlaneMeasures have been set
      //      return;
    }

    // A held point is basically a ground point.  So don't mess with it either
    if (Held()) {
      if (p_latitude == Isis::Null &&
          p_longitude == Isis::Null &&
          p_radius == Isis::Null) {
        std::string msg = "ControlPoint [" + Id() + "] is held and ";
        msg += "requires lat/lon/radius";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }
    }

    double lat = 0.0;
    double lon = 0.0;
    double rad = 0.0;
    int goodMeasures = 0;
    double baselon = 180.;

    // Loop for each measure and compute the sum of the lat/lon/radii
    for (int j=0; j<(int)p_measure.size(); j++) {
      ControlMeasure &m = p_measure[j];
      if (m.Type() == ControlMeasure::Unmeasured ) {
        // TODO: How do we deal with unmeasured measures
      }
      else if (m.Ignore()) {
        // TODO: How do we deal with ignored measures
      }
      else {
        Camera *cam = m.Camera();
        if( cam == NULL ) {
          std::string msg = "The Camera must be set prior to calculating apriori";
          throw iException::Message(iException::Programmer,msg,_FILEINFO_);
        }
        if (cam->SetImage(m.Sample(),m.Line())) {
          goodMeasures++;
          lat += cam->UniversalLatitude();

          // Deal with longitude wrapping
          double wraplon = WrapLongitude(cam->UniversalLongitude(), baselon);
          lon += wraplon;
          baselon = wraplon;
          rad += cam->LocalRadius();
          double x = cam->DistortionMap()->UndistortedFocalPlaneX();
          double y = cam->DistortionMap()->UndistortedFocalPlaneY();
          m.SetFocalPlaneMeasured(x,y);
          m.SetMeasuredEphemerisTime(cam->EphemerisTime());
        }
        else {
          // TODO: What do we do
          std::string msg = "Cannot compute lat/lon for control point [" +
            Id() + "], measure [" + m.CubeSerialNumber() + "]";
          throw iException::Message(iException::User,msg,_FILEINFO_);

          // m.SetFocalPlaneMeasured(?,?);
        }
      }
    }

    // Did we have any measures?
    if (goodMeasures == 0) {
      std::string msg = "ControlPoint [" + Id() + "] has no measures which ";
      msg += "project to latitude/longitude";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    // Don't update the lat/lon for held or ground points
    if (Held()) return;
    if (Type() == ControlPoint::Ground) return;

    // Compute the averages
    lat = lat / goodMeasures;
    lon = lon / goodMeasures;
    if (lon < 0)  lon += 360.;
    rad = rad / goodMeasures;

    SetUniversalGround(lat,lon,rad);
  }


  /**
   * This method computes the errors for a point. 
   *  
   * @history 2008-07-17  Tracie Sucharski,  Added ptid and measure serial 
   *                            number to the unable to map to surface error. 
   */

  void ControlPoint::ComputeErrors() {
    if (Ignore()) return;

    double lat = UniversalLatitude();
    double lon = UniversalLongitude();
    double rad = Radius();

    // Loop for each measure to compute the error
    for (int j=0; j<(int)p_measure.size(); j++) {
      ControlMeasure &m = p_measure[j];
      if (m.Ignore()) continue;
      if (m.Type() == ControlMeasure::Unmeasured) continue;

      // TODO:  Should we use crater diameter?
      Camera *cam = m.Camera();
      if (cam->SetUniversalGround(lat,lon,rad)) {
        double sampError = m.Sample() - cam->Sample();
        double lineError = m.Line() - cam->Line();
        m.SetError(sampError,lineError);

        double x = cam->DistortionMap()->UndistortedFocalPlaneX();
        double y = cam->DistortionMap()->UndistortedFocalPlaneY();
        m.SetFocalPlaneComputed(x,y);
        m.SetComputedEphemerisTime(cam->EphemerisTime());
      }
      else {
        std::string msg = "Unable to map point to surface, ControlPoint [" +
                          Id() + "], ControlMeasure [" + m.CubeSerialNumber() +
                          "]";
        throw iException::Message(iException::User,msg,_FILEINFO_);
        // TODO: What should we do?
        // m.SetError(999.0,999.0);
        // m.SetFocalPlaneComputed(?,?);
      }
    }
    return;
  }

  /**
   * Return the maximum error magnitude of the measures in the point.
   * Ignored and unmeasured measures will not be included.
   */
  double ControlPoint::MaximumError () const {
    double maxError = 0.0;
    if (Ignore()) return maxError;

    for (int j=0; j<(int) p_measure.size(); j++) {
      if (p_measure[j].Ignore()) continue;
      if (p_measure[j].Type() == ControlMeasure::Unmeasured) continue;
      if (p_measure[j].ErrorMagnitude() > maxError) {
        maxError = p_measure[j].ErrorMagnitude();
      }
    }

    return maxError;
  }


 /**
  * Wraps the input longitude toward a base longitude
  *
  * @param  lon     Input longitude to be wrapped
  * @param  baselon Longitude to compare
  * @return The wrapped longitude
  */
  double ControlPoint::WrapLongitude( double lon, double baselon) {
    double diff = baselon - lon;

    if (diff <= 180.  &&  diff >= -180.) { // No wrap needed
      return lon;
    }
    else if (diff > 180.) {
      return (lon + 360.);
    }
    else {  // (diff < -180.)
      return (lon - 360.);
    }
  }
}
