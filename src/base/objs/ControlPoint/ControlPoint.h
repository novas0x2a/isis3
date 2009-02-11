/**
 * @file
 * $Revision: 1.6 $
 * $Date: 2008/09/30 22:28:38 $
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

#if !defined(ControlPoint_h)
#define ControlPoint_h

#include <vector>
#include "ControlMeasure.h"

namespace Isis {
  /**
   * @brief A single control point
   *
   * A control point is one or more measurements that identify the same feature
   * or location in different images.
   *
   * @ingroup ControlNetwork
   *
   * @author 2005-07-29 Jeff Anderson
   *
   * @see ControlMeasure ControlNet
   *
   * @internal
   *   @history 2005-07-29 Jeff Anderson Original version
   *   @history 2006-01-11 Jacob Danton Added ReferenceIndex method and updated unitTest
   *   @history 2006-06-28 Tracie Sucharski, Added method to return measure
   *            for given serial number.
   *   @history 2006-10-31 Tracie Sucharski, Added HasReference method,
   *            changed ReferenceIndex method to throw error if there is no
   *            Reference ControlMeasure.
   *   @history 2007-01-25 Debbie A. Cook, Removed return statement in SetApriori method
   *            for GroundPoint case so that FocalPlaneMeasures will get set.  The method
   *            already has a later return statement to avoid changing the lat/lon values.
   *   @history 2007-10-19 Debbie A. Cook, Wrapped longitudes when calculating apriori longitude
   *            for points with a difference of more than 180 degrees of longitude between measures.
   *   @history 2008-01-14 Debbie A. Cook, Changed call to Camera->SetUniversalGround in ComputeErrors
   *            to include the radius as an argument since the method has been overloaded to include
   *            radius.
   *   @history 2008-09-12 Tracie Sucharski, Add method to return true/false
   *                   for existence of Serial Number.
   *
   */
  class ControlPoint {
    public:
      ControlPoint();
      ControlPoint (const std::string &id);

      //! Destroy a control point
      ~ControlPoint () {};

      void Load(PvlObject &p);

      PvlObject CreatePvlObject();

      //! Sets the Id of the control point
      void SetId(const std::string &id) { p_id = id; };

      //! Return the Id of the control point
      std::string Id() const { return p_id; };

      void Add(const Isis::ControlMeasure &measure);
      void Delete(int index);

      //! Return the ith measurement of the control point
      ControlMeasure &operator[](int index) { return p_measure[index]; };

      //! Return the ith measurement of the control point
      const ControlMeasure &operator[](int index) const { return p_measure[index]; };

      //! Return the measurement for the given serial number
      ControlMeasure &operator[](const std::string &serialNumber);

      //! Return the measurement for the given serial number
      const ControlMeasure &operator[](const std::string &serialNumber) const;

      //! Does Serial Number exist in point
      bool HasSerialNumber (std::string &serialNumber);

      //! Return the number of measurements in the control point
      int Size () const { return p_measure.size(); };

      //! Set whether to ignore or use control point
      void SetIgnore(bool ignore) { p_ignore = ignore; };

      //! Return if the control point should be ignored
      bool Ignore() const { return p_ignore; };

      //! Set the control point as held to its lat/lon
      void SetHeld(bool held) { p_held = held; };

      //! Is the control point lat/lon held?
      bool Held() const { return p_held; };

      /**
       * A control point can have one of two types, either Ground or Tie.
       *
       * A Ground point is a Control Point whose lat/lon is well established
       * and should not be changed. Some people will refer to this as a
       * truth (i.e., ground truth).  Holding a point is equivalent to making
       * it a ground point.  A ground point can be identifed in one or more
       * cubes.
       *
       * A Tie point is a Control Point that identifies common measurements
       * between two or more cubes. While it could have a lat/lon, it is not
       * necessarily correct and is subject to change.  This is the most
       * common type of control point.
       */
      enum PointType { Ground, Tie };

      //! Change the type of the control point
      void SetType (PointType type) { p_type = type; };

      //! Return the type of the point
      PointType Type () const { return p_type; };

      void SetUniversalGround (double lat, double lon, double radius);

      //! Return the planetocentric latitude of the point
      double UniversalLatitude () const { return p_latitude; };

      //! Return the planetocentric longitude of the point
      double UniversalLongitude () const { return p_longitude; };

      //! Return the radius of the point in meters
      double Radius () const { return p_radius; };

      double AverageError() const;

      // std::string Thumbnail() const;
      // std::string FeatureName() const;

      bool HasReference();
      int ReferenceIndex();

      void ComputeApriori();

      void ComputeErrors();

      double MaximumError() const;

      double WrapLongitude ( double lon, double baselon);

    private:
      std::string p_id;
      std::vector<Isis::ControlMeasure> p_measure;
      PointType p_type;
      bool p_ignore;
      bool p_held;
      double p_latitude;
      double p_longitude;
      double p_radius;
  };
};

#endif
