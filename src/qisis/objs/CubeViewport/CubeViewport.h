#ifndef CubeViewport_h
#define CubeViewport_h

/**
 * @file
 * $Date: 2008/08/11 20:40:43 $
 * $Revision: 1.13 $
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


#include <QAbstractScrollArea>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QPen>
#include <QPoint>
#include <QLine>

#include "Cube.h"
#include "Stretch.h"
#include "Brick.h"
#include "Camera.h"
#include "Projection.h"
#include "UniversalGroundMap.h"

namespace Qisis {
 /**
 * @brief Widget to display Isis cubes for qt apps
 *
 * @ingroup Visualization Tools
 *
 * @author ??? Jeff Anderson
 *
 * @internal
 *  @history 2007-01-30  Tracie Sucharski,  Add information
 *                           message if spice not found.
 *  @history 2007-02-07  Tracie Sucharski,  Remove error message, decided it
 *                           was more of a hassle to click ok when displaying
 *                           many images without spice.
 *  @history 2007-03-20  Tracie Sucharski,  Add fitScaleMinDimension,
 *                           fitScaleWidth and fitScaleHeight methods.  Change
 *                           cursor to wait cursor in paintPixmap.
 *  @history 2007-04-13  Tracie Sucharski, Remove fitScaleMinDimension, turns
 *                           out it was not needed.
 *  @history 2007-09-11  Steven Lambright, Added double click signal
 *  @history 2007-12-11  Steven Lambright, Added 1x1xn cube auto stretch support
 *  @history 2008-06-19  Noah Hilt, Added a close event for saving and discarding
 *                           changes and a method to set the cube.
 */

  class Tool;

  class CubeViewport : public QAbstractScrollArea {
    Q_OBJECT


    public:
      CubeViewport (Isis::Cube *cube, QWidget *parent=0);
      ~CubeViewport ();

      void setCube(Isis::Cube *cube);
 
      //! Return the number of samples in the cube
      int cubeSamples() const { return p_cube->Samples(); };

      //! Return the number of lines in the cube
      int cubeLines() const { return p_cube->Lines(); };

      //! Return the number of bands in the cube
      int cubeBands() const { return p_cube->Bands(); };

      //! Is the viewport linked with other viewports
      bool isLinked() const { return p_linked; };

      //! Is the rubberband being drawn
      bool isRubberBandDrawing() const { return p_rubberBandDrawing; };

      //! Is the viewport shown in 3-band color
      bool isColor() const { return p_color; };

      //! Is the viewport shown in gray / b&w
      bool isGray() const { return !p_color; };

      //! Return the gray band currently viewed
      int grayBand() const { return p_gray.band; };

      //! Return the red band currently viewed
      int redBand() const { return p_red.band; };

      //! Return the green band currently viewed
      int greenBand() const { return p_green.band; };

      //! Return the blue band currently viewed
      int blueBand() const { return p_blue.band; };

      //! Return the scale
      double scale() const { return p_scale; };

      //! Return if the cube is visible
      bool cubeShown() const { return p_cubeShown; };

      void cubeContentsChanged (QRect rect);

      double fitScale() const;
      double fitScaleWidth() const;
      double fitScaleHeight() const;

      void viewportToCube(int x, int y,
                          double &sample, double &line) const;
      void cubeToViewport(double sample, double line,
                          int &x, int &y) const;
      void contentsToCube(int x, int y,
                          double &sample, double &line) const;
      void cubeToContents(double sample, double line,
                          int &x, int &y) const;

      double redPixel (int sample, int line);
      double greenPixel (int sample, int line);
      double bluePixel (int sample, int line);
      double grayPixel (int sample, int line);

      //! Return the gray band stretch
      Isis::Stretch grayStretch () const { return p_gray.stretch; };

      //! Return the red band stretch
      Isis::Stretch redStretch () const { return p_red.stretch; };

      //! Return the green band stretch
      Isis::Stretch greenStretch () const { return p_green.stretch; };

      //! Return the blue band stretch
      Isis::Stretch blueStretch () const { return p_blue.stretch; };

      //! Return the cube associated with viewport
      Isis::Cube *cube() const { return p_cube; };

      //! Return the projection associated with cube (NULL implies none)
      Isis::Projection *projection () const { return p_projection; };

      //! Return the camera associated with the cube (NULL implies none)
      Isis::Camera *camera () const { return p_camera; };

      //! Return the universal ground map associated with the cube (NULL implies none)
      Isis::UniversalGroundMap *universalGroundMap () const { return p_groundMap; };

      void moveCursor(int x, int y);
      bool cursorInside() const;
      QPoint cursorPosition() const;
      void setCursorPosition(int x, int y);
      void setCaption();


      /**
       * RubberBandShape
       */
      enum RubberBandShape {
        Rectangle, //!< 0
        Line//!< 1
      };

      void enableRubberBand(bool enable, RubberBandShape shape=Rectangle);
      QRect rubberBandRect();
      QLine rubberBandLine();

      /**
       * Returns the pixmap
       * 
       * 
       * @return QPixmap 
       */
      QPixmap pixmap() { return p_pixmap; };

      void registerTool (Tool *tool);
      

   signals:
      void viewportUpdated();//!< Emitted when viewport updated.
      void mouseEnter();//!< Emitted when the mouse enters the viewport
      void mouseMove(QPoint);//!< Emitted when the mouse moves.
      void mouseLeave();//!< Emitted when the mouse leaves the viewport.
      void mouseButtonPress(QPoint,Qt::MouseButton);//!< Emitted when mouse button pressed
      void mouseButtonRelease(QPoint,Qt::MouseButton);//!< Emitted when mouse button released
      void mouseDoubleClick(QPoint);//!< Emitted when double click happens
      void linkChanging(bool);//!< Emitted when linking changes
      void windowTitleChanged();//!< Emitted when window title changes
      void scaleChanged(); //!< Emitted when zoom factor changed just before the repaint event
      void saveChanges(); //!< Emitted when changes should be saved
      void discardChanges(); //!< Emitted when changes should be discarded


    public slots:
      void setStretchInfo(int band, bool stretchFlag, double min, double max);
      double getStretchMin(int band);
      double getStretchMax(int band);
      bool getStretchFlag(int band);
      QSize sizeHint() const;
      void setLinked(bool b);
      void setScale (double scale);
      void setScale (double scale, double sample, double line);
      void setScale (double scale, int x, int y);
      void center (int x, int y);
      void center (double sample, double line);

      void viewRGB (int redBand, int greenBand, int blueBand);
      void viewGray (int band);
      void autoStretch (int lineRate=0);
      void autoStretch (int ssamp, int esamp, int sline, int eline,
                        int lineRate=0);
      void stretchRGB (const QString &rstr,
                       const QString &gstr,
                       const QString &bstr);
      void stretchGray (const QString &string);
      void stretchRGB (const Isis::Stretch &rstr,
                       const Isis::Stretch &gstr,
                       const Isis::Stretch &bstr);
      void cubeChanged(bool changed);
      void showCube ();

      void scrollBy(int dx, int dy);

      void changeCursor(QCursor cursor);

    protected:
      virtual void closeEvent(QCloseEvent *event);
      void scrollContentsBy(int dx, int dy);
      void paintEvent (QPaintEvent *e);
      void resizeEvent (QResizeEvent *e);
      bool eventFilter (QObject *o, QEvent *e);
      void keyPressEvent(QKeyEvent *e);

    private:
      void updateScrollBars(int x, int y);
      void computeStretch(Isis::Brick *brick, int band,
                          int ssamp, int esamp,
                          int sline, int eline, int linerate,
                          Isis::Stretch &stretch);


      Isis::Cube *p_cube;//!< The cube associated with the viewport.
      Isis::Camera *p_camera;//!< The camera from the cube.
      Isis::Projection *p_projection;//!< The projection from the cube.
      Isis::UniversalGroundMap *p_groundMap;//!< The universal ground map from the cube.

      double p_scale;//!< The scale number.
      bool p_linked;//!< Is the viewport linked?

      class BandInfo {
        public:
          int band; //!< Band number
          Isis::Stretch stretch;//!< Stretch
          /**
           * BandInfo constructor
           * 
           */
          BandInfo() {
            band = 1;
            stretch.SetNull(0.0);
            stretch.SetLis(0.0);
            stretch.SetLrs(0.0);
            stretch.SetHis(255.0);
            stretch.SetHrs(255.0);
            stretch.SetMinimum(0.0);
            stretch.SetMaximum(255.0);
          };
      };

      
      bool p_color;//!< Is the viewport in color?
      BandInfo p_gray;//!< Gray band info
      BandInfo p_red;//!< Red band info
      BandInfo p_green;//!< Green band info
      BandInfo p_blue;//!< Blue band info

      Isis::Brick *p_redBrick;//!< Bricks for every color.
      Isis::Brick *p_grnBrick;//!< Bricks for every color.
      Isis::Brick *p_bluBrick;//!< Bricks for every color.
      Isis::Brick *p_gryBrick;//!< Bricks for every color.
      Isis::Brick *p_pntBrick;//!< Bricks for every color.
      bool p_saveEnabled; //!< Has the cube changed?
      bool p_cubeShown;//!< Is the cube visible?
      QImage *p_image;//!< The qimage. 
      QPixmap p_pixmap;//!< The qpixmap.
      bool p_paintPixmap;//!< Paint the pixmap?

      void paintPixmap();
      void paintPixmap(QRect rect);

      bool p_rubberBandDrawing;  //!< Is rubber band being drawn?
      bool p_rubberBandEnabled;  //!< Can rubber band be drawn?
      RubberBandShape p_rubberBandShape;//!< The shape of the rubberband.
      QRect p_rubberBandRect;//!< The rectangle of the rubberband.
      QLine p_rubberBandLine;//!< The line of the rubberband.

      QString p_whatsThisText;//!< The text for What's this.
      QString p_cubeWhatsThisText;//!< The text for the cube's What's this.
      QString p_viewportWhatsThisText;//!< The text for the viewport's what's this.
      void updateWhatsThis();

      QList<Tool *> p_toolList;//!< A list of the tools 

      class CubeBandsStretch {
        public:
          double p_stretchMin; //!< Min stretch 
          double p_stretchMax; //!< Max stretch 
          bool p_stretched; //!< Stretched?

          /**
           * CubeBandsStretch constructor
           * 
           */
          CubeBandsStretch() {
            p_stretchMin = 0;
            p_stretchMax = 0;
            p_stretched = false; 
  
          };
      };
  
      QList<CubeBandsStretch *> p_bandsStretchList; //!< A list of the stretches


  };
};

#endif
