#ifndef RubberBandTool_h
#define RubberBandTool_h

#include "Tool.h"
#include "geos/geom/Geometry.h"

namespace Qisis {
  /**
  * @brief Rubber banding tool
  *
  * @ingroup Visualization Tools
  *
  * @author 2007-09-11 Steven Lambright 
  *  
  * @internal 
  *   @history 2006-01-01 Jeff Anderson Original version 
  *   @history 2008-01-03 Steven Lambright bug fix on the polygons
  *   @history 2008-05-23 Noah Hilt added getRectangle method
  *   @history 2008-08-18 Steven Koechle updated to work with Geos 3.0.0
  */

  class RubberBandTool : public Qisis::Tool {
    Q_OBJECT

  public:
    static RubberBandTool *getInstance(QWidget *parent = NULL);

    enum RubberBandMode {
      Angle,            //<! Measure an angle
      Circle,           //<! Draw a perfect circle
      Ellipse,          //<! Draw an ellipse (oval)
      Line,             //<! Draw a simple line
      Rectangle,        //<! Draw a rectangle without any rotation (perfectly horizonal/verticle)
      RotatedRectangle, //<! Draw a rotatable rectangle 
      Polygon           //<! Draw any closed shape
    };

    static void enable(RubberBandMode mode, bool showIndicatorColors = false) { getInstance()->enableBanding(mode, showIndicatorColors);}
    void enableBanding(RubberBandMode mode, bool showIndicatorColors = false);

    static void disable() { getInstance()->disableBanding();}
    void disableBanding();

    static QList<QPoint> getVertices() { return getInstance()->getFoundVertices();}
    QList<QPoint> getFoundVertices();

    static RubberBandMode getMode() { return getInstance()->getCurrentMode();};
    RubberBandMode getCurrentMode() { return p_bandingMode;};

    static double getArea() { return getInstance()->getAreaMeasure();}
    double getAreaMeasure() { return 0.0;} //<! Returns the area of the figure

    static double getAngle() { return getInstance()->getAngleMeasure();}
    double getAngleMeasure(); //<! Returns the angle measurement (in radians)

    static geos::geom::Geometry *geometry() { return getInstance()->getGeometry();}
    geos::geom::Geometry *getGeometry();

    static QRect rectangle() { return getInstance()->getRectangle();}
    QRect getRectangle();

    static Qt::MouseButton mouseButton() { return getInstance()->getMouseButton();}
    Qt::MouseButton getMouseButton();

    void paintViewport(CubeViewport *vp,QPainter *painter);

    static QList< QList<double> > getDataByLine(long index) {
      return getInstance()->getRotatedRectDataByLine(index);
    }

    //<! This returns true if we can return complete & valid data.
    static bool isValid() { return getInstance()->figureValid();} 
    bool figureComplete();
    bool figureValid();

    static bool isPoint() { return getInstance()->figureIsPoint();}
    bool figureIsPoint();

    static void allowPoints(unsigned int pixTolerance = 2) { getInstance()->enablePoints(pixTolerance);}
    void enablePoints(unsigned int pixTolerance = 2);

    static void allowAllClicks() { getInstance()->enableAllClicks();}
    void enableAllClicks() {p_allClicks = true;};

    QList< QList<double> > getRotatedRectDataByLine(long index);

  public slots:
    void escapeKeyPress() { reset();}

    signals:
    void modeChanged();
    void bandingComplete();
    void measureChange();

  protected:
    void enableRubberBandTool() {}
    void scaleChanged() { reset();}

  protected slots:
    void mouseMove(QPoint p);
    void mouseDoubleClick(QPoint p);
    void mouseButtonPress(QPoint p, Qt::MouseButton s);
    void mouseButtonRelease(QPoint p, Qt::MouseButton s);

  private:
    RubberBandTool (QWidget *parent);

    void repaint();
    void paintVerticesConnected(QPainter *painter);
    void paintRectangle(QPoint upperLeft, QPoint lowerRight, QPainter *painter);
    void paintRectangle(QPoint upperLeft, QPoint upperRight, 
                        QPoint lowerLeft, QPoint lowerRight, QPainter *painter);

    // This is used for rotated rectangle
    void calcRectCorners(QPoint corner1, QPoint corner2, QPoint &corner3, QPoint &corner4);

    static RubberBandTool *p_instance;

    void reset();                  //<! This resets the member variables

    bool p_mouseDown;              //<! True if the mouse is pressed
    bool p_doubleClicking;         //<! True if on second click of a double click
    bool p_tracking;               //<! True if painting on mouse move
    bool p_allClicks;              //<! Enables all mouse buttons for banding 
    RubberBandMode p_bandingMode;  //<! Current type of rubber band
    QList<QPoint> p_vertices;      //<! Known vertices pertaining to the current rubber band
    QPoint p_mouseLoc;             //<! Current mouse location, only valid of p_tracking
    Qt::MouseButton p_mouseButton; //<! Last mouse button status
    bool p_indicatorColors;        //<! Color the first side of objects a different color, if it's significant
    unsigned int p_pointTolerance; //<! Tolerance for points (zero for no points)
  };
};

#endif

