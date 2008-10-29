#ifndef Qisis_AdvancedTrackTool_h
#define Qisis_AdvancedTrackTool_h
/**
 * @file
 * $Revision: 1.6 $
 * $Date: 2008/06/25 23:52:59 $
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
#include "Tool.h"
#include "TableMainWindow.h"
#include <QAction>
class QDialog;
class QListWidget;
class QMenu;
class QLabel;
class QStatusBar;
class QListWidgetItem;


namespace Qisis {
  /**
   * @brief Tool to display info for a point on a cube
   *
   * This tool is part of the Qisis namespace and allows the user to view and 
   * record information from a point on a cube such as line, sample, band, lats, 
   * longs, etc. 
   * 
   * @ingroup Visualization Tools
   * 
   * @author  ?? Unknown
   * 
   * @internal
   *  @history 2008-06-25 Noah Hilt - Added enumeration for different column
   *           values.
   *
   */
  class AdvancedTrackTool : public Tool {
    Q_OBJECT

    public:
      AdvancedTrackTool (QWidget *parent);
      void addTo(QMenu *menu);
      void addToPermanent (QToolBar *perm);
      bool eventFilter(QObject *o,QEvent *e);

    public slots:
      virtual void mouseMove(QPoint p);
      virtual void mouseLeave();
     
    protected:
      /**
       * This method returns the menu name associated with this tool.
       * 
       * 
       * @return QString 
       */
      QString menuName() const { return "&Options"; };

    private slots:
      void updateRow(QPoint p);
      void updateRow(CubeViewport *cvp, QPoint p, int row);
      void record();
      void updateID();

    private:
      /**
       * Enum for column values
       */
      enum {
        ID, //!< The record ID
        SAMPLE, //!< The current sample
        LINE, //!< The current line
        BAND, //!< The current band
        PIXEL, //!< The current pixel
        PLANETOCENTRIC_LAT, //!< The planetocentric latitude for this point
        PLANETOGRAPHIC_LAT, //!< The planetographic latitude for this point
        EAST_LON_360, //!< The 360 east longitude for this point
        WEST_LON_360, //!< The 360 west longitude for this point
        EAST_LON_180, //!< The 180 east longitude for this point
        WEST_LON_180, //!< The 180 west longitude for this point
        RADIUS, //!< The radius for this point
        POINT_X, //!< The x value for this point
        POINT_Y, //!< The y value for this point
        POINT_Z, //!< The z value for this point
        RIGHT_ASCENSION, //!< The right ascension for this point
        DECLINATION, //!< The declination for this point
        RESOLUTION,//!< The resoultion for this point
        PHASE,//!< The phase for this point 
        INCIDENCE, //!< The incidence for this point
        EMISSION, //!< The emission for this point
        NORTH_AZIMUTH, //!< The north azimuth for this cube
        SUN_AZIMUTH, //!< The sun azimuth for this cube
        SOLAR_LON, //!< The solar longitude for this point
        SPACECRAFT_X, //!< The spacecraft x position for this cube
        SPACECRAFT_Y, //!< The spacecraft y position for this cube
        SPACECRAFT_Z, //!< The spacecraft z position for this cube
        SPACECRAFT_AZIMUTH, //!< The spacecraft azimuth for this cube
        SLANT, //!< The slant for this cube
        EPHEMERIS_TIME, //!< The ephemeris time for this cube
        SOLAR_TIME,//!< The local solar time for this cube
        UTC, //!< The UTC for this cube
        PATH, //!< The path for this cube
        FILENAME,//!< The filename for this cube
        SERIAL_NUMBER, //!< The serial number for this cube
        NOTES //!< Any notes for this record
      };
      QAction *p_action; //!< Action to bring up the track tool
      int p_numRows; //!< The number of rows in the table
      int p_id; //!< The record id 
      Qisis::TableMainWindow *p_tableWin; //!< The table window
 
  };

};

#endif
