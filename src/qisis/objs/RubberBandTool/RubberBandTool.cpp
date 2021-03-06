#include "RubberBandTool.h"
#include "SerialNumberList.h"
#include "PolygonTools.h"
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/CoordinateSequence.h"
#include "geos/geom/Coordinate.h"
#include "geos/geom/LineString.h"
#include "geos/geom/MultiLineString.h"
#include "geos/geom/Polygon.h"

namespace Qisis {
  RubberBandTool *RubberBandTool::p_instance = NULL;

  /**
   * This method returns the singleton instance of the rubber band tool.
   *   The first call to this method must have the optional argument *parent so
   *   the tool can properly initialize. After this tool is created, *parent is ignored.
   * 
   * @param parent A pointer to the main window for the tool to properly initialize
   * 
   * @return RubberBandTool* An instance of the rubber band tool
   */
  RubberBandTool *RubberBandTool::getInstance(QWidget *parent) {
    if (p_instance == NULL && parent != NULL) {
      p_instance = new RubberBandTool(parent);
    }

    return p_instance;
  }

  /**
   * This is the constructor. It's private because this class is a singleton.
   * 
   * @param parent
   */
  RubberBandTool::RubberBandTool (QWidget *parent) : Qisis::Tool(parent) {
    activate(false);
    repaint();
  }

  /**
   * This is the main paint method for the rubber bands.
   *
   * For angles and lines, simply connect the known vertices.vertices[0].x()
   * For polygons, paint the vertices & close if completed the shape.
   * For circles, figure out the circle's square and draw the circle inside of it.
   * For Ellipses, figure out the Ellipse's rectangle and draw the circle inside of it.
   * For rectangles, paint the rectangle either to the mouse or back to the start depending on if the shape is complete.
   * For rotated rectangles, if we can interpolate extra sides draw them and draw all known sides.
   *
   * @param vp
   * @param painter
   */
  void RubberBandTool::paintViewport(CubeViewport *vp, QPainter *painter) {
    QPen pen(QColor(255,0,0));
    QPen greenPen(QColor(0, 255, 0));
    pen.setStyle(Qt::SolidLine);
    greenPen.setStyle(Qt::SolidLine);
    painter->setPen(pen);

    if(vp != cubeViewport()) {
      return;
    }

    switch (p_bandingMode) {
    case Angle:
      paintVerticesConnected(painter);
      break;

    case Line:
      // if point needed, draw an X
      if (figureIsPoint() && !p_tracking) {
        painter->drawLine(p_vertices[0].x() - 10, p_vertices[0].y() - 10, 
                          p_vertices[0].x() + 10, p_vertices[0].y() + 10);
        painter->drawLine(p_vertices[0].x() - 10, p_vertices[0].y() + 10, 
                          p_vertices[0].x() + 10, p_vertices[0].y() - 10); 
      } else {
        paintVerticesConnected(painter);
      }

      break;

    case Polygon:
      paintVerticesConnected(painter);

      if (!p_tracking && p_vertices.size() > 0) {
        painter->drawLine(p_vertices[0], p_vertices[ p_vertices.size() - 1 ]);
      }
      break;

    case Circle:
    case Ellipse: {
        if (p_vertices.size() != 0) {
          QList<QPoint> vertices = getFoundVertices();
          int width = 2 * (vertices[1].x()-vertices[0].x());
          int height = 2 * (vertices[1].y()-vertices[0].y());

          // upper left x,y,width,height
          painter->drawEllipse(vertices[0].x()-width/2, vertices[0].y()-height/2, 
                               width, 
                               height
                              );
        }
      }
      break;

    case Rectangle: {
        if (figureIsPoint() && !p_tracking) {
          painter->drawLine(p_vertices[0].x() - 10, p_vertices[0].y() - 10, 
                            p_vertices[0].x() + 10, p_vertices[0].y() + 10);
          painter->drawLine(p_vertices[0].x() - 10, p_vertices[0].y() + 10, 
                            p_vertices[0].x() + 10, p_vertices[0].y() - 10); 
        } else {
          if (p_tracking && p_vertices.size() > 0) {
            paintRectangle(p_vertices[0], p_mouseLoc, painter);
          } else if (p_vertices.size() > 0) {
            paintVerticesConnected(painter);
            painter->drawLine(p_vertices[0], p_vertices[ p_vertices.size() - 1 ]);
          }
        }
      }
      break;

    case RotatedRectangle: {
        if (p_vertices.size() == 2) {
          QPoint missingVertex;
          calcRectCorners(p_vertices[0], p_vertices[1], p_mouseLoc, missingVertex);
          painter->drawLine(p_mouseLoc, missingVertex);
          painter->drawLine(missingVertex, p_vertices[0]);
        } else if (p_vertices.size() == 4) {
          painter->drawLine(p_vertices[0], p_vertices[ 3 ]);
        }

        paintVerticesConnected(painter);

        // Draw indicator on top of original lines if applicable
        if (p_indicatorColors) {
          painter->setPen(greenPen);
          if (p_vertices.size() > 1) {
            painter->drawLine(p_vertices[0], p_vertices[1]);
          } else if (p_vertices.size() == 1) {
            painter->drawLine(p_vertices[0], p_mouseLoc);
          }

          painter->setPen(pen);
        }
      }
      break;

    case SegmentedLine:
      paintVerticesConnected(painter);
      break;
    }
  }

  /**
   * Given two set corners, and the mouse location, this method will interpolate the last two corners.  
   * 
   * @param corner1 Known point
   * @param corner2 Known point
   * @param corner3 Guessed corner (point to interpolate to).
   * @param corner4 Unknown corner.
   */
  void RubberBandTool::calcRectCorners(QPoint corner1, QPoint corner2, QPoint &corner3, QPoint &corner4) {
    double slope = ((double)corner2.y() - (double)corner1.y()) / ((double)corner2.x() - (double)corner1.x());
    int delta_x;
    int delta_y;

    if ((fabs(slope) > DBL_EPSILON) && (slope < DBL_MAX) && (slope > -DBL_MAX)) {
      // corner1,corner2 make up y=m(x-x1)+y1
      // corner3,corner4 must make up || line crossing corner3.
      // b (y-intercept) is what differs from the original line and our parallel line.
      // Go ahead and figure out our new b by using b = -mx1+y1 from the point-slope formula.
      double parallelB = -1 * slope * corner3.x() + corner3.y();

      // Now we have our equation for a parallel line, which our new points lie on. Let's find the perpendicular lines
      // which cross corner1,corner2 in order to figure out where they cross it. Use -1/slope = perpendicular slope and
      // now we have y=m(x-x1)+y1. What we care about is b in y=mx+b, so figure it out using b = m*(-x1)+y1 
      double perpSlope = -1.0 / slope;
      double perpLine1b = perpSlope * (-1 * corner1.x()) + corner1.y();
      double perpLine2b = perpSlope * (-1 * corner2.x()) + corner2.y();

      // Now let's find the perpendicular lines' intercepts on the parallel line.
      // y = mx+b = y = mx+b => mx+b(perpendicular) = mx+b(parallel) for the perp. lines and the parallel line.
      // Combine the b's on the left to make y= m(perp)x+k = m(par)x.
      double perpLine1k = perpLine1b - parallelB;
      double perpLine2k = perpLine2b - parallelB;

      // Now we have mx + k = mx (parallel). Divive the parallel slope out to get
      //    (m(perp)x+k)/m(parallel) = x. Move the x over from the left side of the equation by subtracting...
      //    k/m(parallel) = x - m(perp)x/m(parallel). Factor out the x's... k/m(par) = x(1-m(per)/m(par)) and divive
      //    both sides by "(1-m(per)/m(par))". So we end up with: (k/m(par)) / (1 - m(per) / m(par) ) =
      //    k/m(par) / ( (m(par)-m(per)) / m(par) ) = k / m(par) * m(par) / (m(par) - m(per)) = k / (m(par) - m(per))
      double perpLine1IntersectX = perpLine1k / (slope - perpSlope);
      double perpLine2IntersectX = perpLine2k / (slope - perpSlope);

      // The intersecting X values are now known, and the equation of the parallel line, so let's roll with it and
      // get our two corners set. perpLine1 => corner1 => corner4, perpLine2 => corner2 => corner3
      corner3.setX((int)perpLine2IntersectX);
      corner3.setY((int)(perpLine2IntersectX * slope + parallelB)); //mx+b
      corner4.setX((int)perpLine1IntersectX);
      corner4.setY((int)(perpLine1IntersectX * slope + parallelB)); //mx+b
    } else if (fabs(slope) < DBL_EPSILON) {
      delta_x = 0;
      delta_y = corner3.y() - corner2.y();
      corner3.setX(corner2.x());
      corner3.setY(corner3.y());
      corner4.setX(corner1.x());
      corner4.setY(corner3.y());
    } else {
      delta_x = corner3.x() - corner2.x();
      corner3.setX(corner3.x());
      corner3.setY(corner2.y());
      corner4.setX(corner3.x());
      corner4.setY(corner1.y());
    }
  }

  /**
   * This paints connecting lines to p_vertices. If tracking, a line is also drawn to
   *   the mouse location.
   *
   * @param painter
   */
  void RubberBandTool::paintVerticesConnected(QPainter *painter) {
    for (int vertex = 1; vertex < p_vertices.size(); vertex++) {
      QPoint start = p_vertices[vertex - 1];
      QPoint end = p_vertices[vertex];

      painter->drawLine(start,end);
    }

    if (p_tracking && (p_vertices.size() > 0)) {
      QPoint start = p_vertices[p_vertices.size() - 1];
      QPoint end = p_mouseLoc;

      painter->drawLine(start,end);
    }
  }

  /**
   * Given opposite corners, the other two are interpolated and the rectangle is drawn.
   * 
   * @param upperLeft Corner opposite of lowerRight
   * @param lowerRight Corner opposite of upperLeft
   * @param painter
   */
  void RubberBandTool::paintRectangle(QPoint upperLeft, QPoint lowerRight, QPainter *painter) {
    QPoint upperRight = QPoint(lowerRight.x(), upperLeft.y());
    QPoint lowerLeft = QPoint(upperLeft.x(), lowerRight.y());

    paintRectangle(upperLeft, upperRight, lowerLeft, lowerRight, painter);
  }

  /**
   * This draws a box around the 4 points using the painter.
   *
   * @param upperLeft Initial corner
   * @param upperRight Corner connected to upperLeft, lowerRight
   * @param lowerLeft Corner connected to lowerRight, upperLeft
   * @param lowerRight Corner connected to lowerLeft, upperRight
   * @param painter
   */
  void RubberBandTool::paintRectangle(QPoint upperLeft, QPoint upperRight, 
                                      QPoint lowerLeft, QPoint lowerRight, QPainter *painter) {
    painter->drawLine(upperLeft,upperRight);
    painter->drawLine(upperRight,lowerRight);
    painter->drawLine(lowerRight,lowerLeft);
    painter->drawLine(lowerLeft,upperLeft);
  }

  /**
   * This is called when changing modes or turning on. So, set the mode, reset, and activate
   *   our event handlers.
   * 
   * @param mode
   * @param showIndicatorColors Color the first side of figures differently
   */
  void RubberBandTool::enableBanding(RubberBandMode mode, bool showIndicatorColors) {
    RubberBandMode oldMode = p_bandingMode;
    p_bandingMode = mode;
    p_indicatorColors = showIndicatorColors;
    //Took this out because it was reseting and not letting us plot single points.
    //p_pointTolerance = 0;
    p_allClicks = false;
    reset();
    activate(true);

    if (oldMode != mode) {
      emit modeChanged();
    }
  }

  /**
   * This is called when something is not using me, so 
   *   turn off events, reset & repaint to clear the clear.
   */
  void RubberBandTool::disableBanding() {

    activate(false);
    reset();
    repaint();
  }

  /**
   * This triggers on a second mouse press. Only polygons care about this, and it signifies an end of
   *   shape. So, if we're in polygon mode, stop tracking the mouse and emit a complete.
   * @param p
   */
  void RubberBandTool::mouseDoubleClick(QPoint p) {
    p_doubleClicking = true;
    p_mouseLoc = p;

    switch (p_bandingMode) {
    case Angle:
    case Circle:
    case Ellipse:
    case Line:
    case Rectangle:
    case RotatedRectangle:
      break;

    case SegmentedLine:
    case Polygon:
      p_tracking = false;
      repaint();
      emit bandingComplete();
      break;        
    }
  }

  /**
   * If the click is not the left mouse button, this does nothing.
   * 
   * This will set mouseDown as true. When the calculations are complete,
   *   p_mouseDown is set to true.
   * 
   * For drag-only,
   *   A press means starting a new rubber band so reset & record the point. This applies to
   *     Circles, Eliipsoids, Lines and Rectangles.
   * 
   * For Rotated Rectangles,
   *   A mount press means we're starting over, setting the first point, or completing.
   *     For the first two, simply reset and record the point. For the latter, figure out the
   *     corners and store those points.
   * 
   * For polygons,
   *   A press means record the current point. Reset first if we're not currently drawing. 
   * 
   * @param p
   * @param s
   */
  void RubberBandTool::mouseButtonPress(QPoint p, Qt::MouseButton s) {
    p_mouseLoc = p;

    if ((s & Qt::LeftButton) != Qt::LeftButton && !p_allClicks) {
      return;
    }

    switch (p_bandingMode) {
    case Angle:
      break;

    case Circle:
    case Ellipse:
    case Line:
    case Rectangle:
      reset();
      p_tracking = true;
      p_vertices.push_back(p);
      break;

    case RotatedRectangle:
      if (p_vertices.size() == 4) {
        reset();
      }

      if (p_vertices.size() == 0) {
        p_vertices.push_back(p);
        p_tracking = true;
      }
      break;

    case SegmentedLine:
    case Polygon:
      if (!p_tracking) {
        reset();
        p_tracking = true;
      }

      if (p_vertices.size() == 0 || p_vertices[ p_vertices.size() - 1 ] != p) {
        p_vertices.push_back(p);
      }

      break;        
    }

    p_mouseDown = true;
  }

  /**
   * If this is not the left mouse button, this does nothing.
   * 
   * This will set mouseDown as false. When the calculations are complete, 
   * p_doubleClicking is
   *   set to false. The double click event occurs with 
   * `the press event so it's safe to set that flag here.
   * 
   * The implementation differs, based on the mode, as follows:
   * 
   * For angles,
   *   This signifies a click. We're always setting one of the 
   *  three vertexes, but if there is an already
   *     complete vertex go ahead and reset first to start a new angle.
   * 
   * For circles,
   *   Since this is a drag-only rubber band, a release signifies a complete. Figure out the corner, based
   *     on the mouse location, and push it onto the back of the vertex list and emit a complete.
   * 
   * For Ellipses,
   *   Since this is a drag-only rubber band, a release signifies a complete. We know the corner, it's the mouse loc,
   *     push it onto the back of the vertex list and emit a complete.
   * 
   * For lines,
   *   Since this is a drag-only rubber band, a release signifies a complete. We know the end point,
   *     push it onto the back of the vertex list and emit a complete.
   * 
   * For rectangles,
   *   Since this is a drag-only rubber band, a release signifies a complete. We know the opposite corner,
   *     figure out the others and push them onto the back of the vertex list and emit a complete.
   * 
   * For rotated rectangles,
   *   If we're finishing dragging the first side, store the end point.
   * 
   * For polygons,
   *   Do nothing, this is taken care of on press.
   * 
   * @param p Current mouse Location
   * @param s Which mouse button was released
   */
  void RubberBandTool::mouseButtonRelease(QPoint p, Qt::MouseButton s) {
    p_mouseLoc = p;
    p_mouseButton = s;

    if ((s & Qt::LeftButton) == Qt::LeftButton || p_allClicks) {
      p_mouseDown = false;
    } else {
      return;
    }

    switch (p_bandingMode) {
    case Angle: {
        if (p_vertices.size() == 3) {
          reset();
        }

        p_vertices.push_back(p);
        p_tracking = true;

        if (p_vertices.size() == 3) {
          p_tracking = false;
          emit bandingComplete();
        }
      }
      break;

    case Line:
    case Circle:
    case Ellipse: 
    case Rectangle: {
        p_vertices = getFoundVertices();
        p_tracking = false;
        repaint();
        emit bandingComplete();
      }
      break;

    case RotatedRectangle: {
        if (p_vertices.size() == 1) {
          p_vertices.push_back(p);
        } else if (p_vertices.size() == 2) {
          p_vertices = getFoundVertices();
          p_tracking = false;
          emit bandingComplete();
        }
      }
      break;

    case SegmentedLine:
    case Polygon:
      break;
    }

    p_doubleClicking = false; // If we were in a double click, it's over now.
  }

  /**
   * If tracking is not enabled, this does nothing.
   * 
   * This will first update the mouse location for painting purposes.
   * 
   * Most of the implementation is a matter of emitting measureChanges:
   * For angles, if the first two vertices are specified a measureChange will be emitted.
   * For circles, if the center of the circle is known a measureChange will be emitted.
   * For Ellipses, if the center of the Ellipse is known a measureChange will be emitted.
   * For lines, if the first point of the line is known a measureChange will be emitted.
   * For rectangles, if the starting point is known a measureChange will be emitted.
   * For rotated rectangles, if the first side is specified a measureChange will be emitted.
   * 
   * However, there is one exception:
   * For polygons, if the mouse button is pressed the mouse location is recorded as a valid vertex.
   * 
   * @param p Current mouse Location
   */
  void RubberBandTool::mouseMove(QPoint p) {
    if (!p_tracking) {
      return;
    }

    // Store the mouse location for painting the polygons
    QPoint oldMouseLoc = p_mouseLoc;
    p_mouseLoc = p;

    switch (p_bandingMode) {
    case Angle:
    case RotatedRectangle:
      if (p_vertices.size() == 2) {
        emit measureChange();
      }
      break;

    case Circle:
    case Ellipse:
    case Rectangle: 
      if (p_vertices.size() == 1) {
        emit measureChange();
      }
      break;

    case Line:
      emit measureChange();
      break;

    case SegmentedLine:
    case Polygon:
      {
        if (p_mouseDown && p != p_vertices[ p_vertices.size() - 1 ]) {
          p_vertices.push_back(p);
        }
      }
      break;
    }

    // Repaint because tracking is enabled, meaning we're drawing to the mouse loc
    repaint();
  }

  /**
   * This method returns the vertices. The return value is mode-specific, and the return will be
   *   consistent whether in a measureChange or bandingComplete slot.
   * 
   * The return values are always in pixels.
   * 
   * The return values are as follows:
   * For angles, the return will always be of size 3. The elements at 0 and 2 are the edges of the angle,
   *   while the element at 1 is the vertex of the angle.
   * 
   * For circles, the return will always be of size 2. The element at 0 is the center of the circle, and the
   *   element at 1 is offset by the radius in both directions.
   * 
   * For Ellipses, the return will always be of size 2. The element at 0 is the center of the circle, and the
   *   element at 1 is offset by the radius in both directions.
   * 
   * For lines, the return will always be of size 2. The elements are the start and end points.
   * 
   * For rectangles, the return will always be of size 4. The elements will be the corners,
   *   in either a clockwise or counter-clockwise direction.
   * 
   * For rotated rectangles, the same applies.
   * 
   * For polygons, the return will be a list of vertices in the order that the user drew them.
   *  
   * **It is NOT valid to call this unless you're in a measureChange or bandingComplete slot.
   * 
   * @return QList<QPoint>
   */
  QList<QPoint> RubberBandTool::getFoundVertices() {
    QList<QPoint> vertices = p_vertices;

    if (!figureComplete()) return vertices;

    if (p_tracking) {
      switch (p_bandingMode) {
      case Angle:
      case Line:
        vertices.push_back(p_mouseLoc);
        break;

      case Rectangle: {
          QPoint corner1 = QPoint(p_mouseLoc.x(), vertices[0].y());
          QPoint corner2 = QPoint(p_mouseLoc.x(), p_mouseLoc.y());
          QPoint corner3 = QPoint(vertices[0].x(), p_mouseLoc.y());
          vertices.push_back(corner1);
          vertices.push_back(corner2);
          vertices.push_back(corner3);
        }
        break;

      case RotatedRectangle: {
          QPoint missingVertex;
          calcRectCorners(p_vertices[0], p_vertices[1], p_mouseLoc, missingVertex);
          vertices.push_back(p_mouseLoc);
          vertices.push_back(missingVertex);
        }
        break;


      case Circle: {
          int xradius = abs(p_mouseLoc.x() - vertices[0].x()) / 2;
          int yradius = xradius;

          if (p_mouseLoc.x() - vertices[0].x() < 0) {
            xradius *= -1;
          }

          if (p_mouseLoc.y() - vertices[0].y() < 0) {
            yradius *= -1;
          }

          // Adjust p_vertices[0] from upper left to center
          vertices[0].setX(vertices[0].x() + xradius);
          vertices[0].setY(vertices[0].y() + yradius);

          vertices.push_back(p_mouseLoc);

          vertices[1].setX(vertices[0].x() + xradius);
          vertices[1].setY(vertices[0].y() + yradius);
        }
        break;

      case Ellipse: {
          // Adjust p_vertices[0] from upper left to center
          double xradius = (p_mouseLoc.x() - vertices[0].x()) / 2.0;
          double yradius = (p_mouseLoc.y() - vertices[0].y()) / 2.0;
          vertices[0].setX((int)(vertices[0].x() + xradius));
          vertices[0].setY((int)(vertices[0].y() + yradius));

          vertices.push_back(p_mouseLoc);
        }
        break;

      case SegmentedLine:
      case Polygon:
        break;
      }
    }

    return vertices;
  }

  /**
   * This initializes the class except for the current mode, which is
   *   set on enable.
   * 
   */
  void RubberBandTool::reset() {
    p_vertices.clear();
    p_tracking = false;
    p_mouseDown = false;
    p_doubleClicking = false;
    repaint();
  }

  double RubberBandTool::getAngleMeasure() {
    double angle = 0;

    if (getCurrentMode() == Angle) {
      // We cancluate the angle by considering each line an angle itself, with respect to the
      //   X-Axis, and then differencing them.
      // 
      //  theta1 = arctan((point0Y - point1Y) / (point0X - point1X))
      //  theta2 = arctan((point2Y - point1Y) / (point2X - point1X))
      //                   |            
      //                   |        / <-- point0
      //                   |      / |
      //                   |    /   |
      //           theta1  |  /     |
      //            -->    |/       | <-- 90 degrees
      //  point1 --> ------|---------------------------
      //(vertex)     -->   |\     | <-- 90 degrees
      //           theta2  | \    |
      //                   |  \   |
      //                   |   \  |
      //                   |    \ |
      //                   |     \|
      //                   |      | <-- point 2
      //   
      // angle = theta1 - theta2; **
      QList<QPoint> vertices = getFoundVertices();
      double theta1 = atan2((double)(vertices[0].y() - vertices[1].y()), (double)(vertices[0].x() - vertices[1].x()) );
      double theta2 = atan2((double)(vertices[2].y() - vertices[1].y()), (double)(vertices[2].x() - vertices[1].x()) );
      angle = (theta1 - theta2);

      // Force the angle into [0,2PI]
      while (angle < 0.0) {
        angle += Isis::PI * 2;
      }
      while (angle > Isis::PI * 2) {
        angle -= Isis::PI * 2;
      }

      // With our [0,2PI] angle, let's make it [0,PI] to get the interior angle.
      if (angle > Isis::PI) {
        angle = (Isis::PI * 2.0) - angle;
      }
    }

    return angle;
  }

  /**
   * This method will call the viewport's repaint if there is a current cube viewport.
   */
  void RubberBandTool::repaint() {
    if (p_cvp != NULL) {
      p_cvp->viewport()->repaint();
    }
  }

  /**
   * 
   * 
   * @return geos::Geometry*
   */
  geos::geom::Geometry *RubberBandTool::getGeometry() {
    geos::geom::Geometry *geometry = NULL;
    QList<QPoint> vertices = getFoundVertices();

    switch (p_bandingMode) {
      case Angle: {
        if (vertices.size() != 3) break;

        geos::geom::CoordinateSequence *points1 = new geos::geom::CoordinateArraySequence();
        geos::geom::CoordinateSequence *points2 = new geos::geom::CoordinateArraySequence();

        points1->add(geos::geom::Coordinate(vertices[0].x(), vertices[0].y()));
        points1->add(geos::geom::Coordinate(vertices[1].x(), vertices[1].y()));
        points2->add(geos::geom::Coordinate(vertices[1].x(), vertices[1].y()));
        points2->add(geos::geom::Coordinate(vertices[2].x(), vertices[2].y()));

        geos::geom::LineString *line1 = Isis::globalFactory.createLineString(points1);
        geos::geom::LineString *line2 = Isis::globalFactory.createLineString(points2);
        std::vector<geos::geom::Geometry*> *lines = new std::vector<geos::geom::Geometry*>;
        lines->push_back(line1);
        lines->push_back(line2);

        geos::geom::MultiLineString *angle = Isis::globalFactory.createMultiLineString(lines);
        geometry = angle;
      }
      break;

      case Circle:
      case Ellipse: {
        if (vertices.size() != 2) break;
        // A circle is an ellipse, so it gets no special case
        // Equation of an ellipse: (x-h)^2/a^2 + (y-k)^2/b^2 = 1 where
        // h is the X-location of the center, k is the Y-location of the center
        // a is the x-radius and b is the y-radius.
        // Solving for y, we get 
        //   y = +-sqrt(b^2(1-(x-h)^2/a^2)) + k
        //   and our domain is [h-a,h+a]
        // We need the equation of this ellipse!
        double h = vertices[0].x();
        double k = vertices[0].y();
        double a = abs(vertices[0].x() - vertices[1].x());
        double b = abs(vertices[0].y() - vertices[1].y());
        if (a == 0) break; // Return null, we can't make an ellipse from this.
        if (b == 0) break; // Return null, we can't make an ellipse from this.

        // We're ready to try to solve
        double originalX = 0.0, originalY = 0.0;
        geos::geom::CoordinateSequence *points = new geos::geom::CoordinateArraySequence();

        // Now iterate through our domain, solving for y positive, using 1/5th of a pixel increments
        for (double x = h - a; x <= h + a; x += 0.2) {
          double y = sqrt(pow(b,2) * (1.0 - pow((x-h),2)/pow(a,2))) + k;
          points->add(geos::geom::Coordinate(x, y));

          if (x == h - a) {
            originalX = x;
            originalY = y;
          }
        }

        // Iterate through our domain backwards, solving for y negative, using 1/5th of a pixel decrements
        for (double x = h + a; x >= h - a; x -= 0.2) {
          double y = -1.0 * sqrt(pow(b,2) * (1.0 - pow((x-h),2)/pow(a,2))) + k;
          points->add(geos::geom::Coordinate(x, y));
        }

        points->add(geos::geom::Coordinate(originalX, originalY));

        geometry = Isis::globalFactory.createPolygon(
                                                    Isis::globalFactory.createLinearRing(points), NULL
                                                    );
      }
      break;

      case Rectangle:
      case RotatedRectangle:
      case Polygon: {
        if (vertices.size() < 3) break;

        geos::geom::CoordinateSequence *points = new geos::geom::CoordinateArraySequence();

        for (int vertex = 0; vertex < vertices.size(); vertex++) {
          points->add(geos::geom::Coordinate(vertices[vertex].x(), vertices[vertex].y()));
        }

        points->add(geos::geom::Coordinate(vertices[0].x(), vertices[0].y()));

        geometry = Isis::globalFactory.createPolygon(Isis::globalFactory.createLinearRing(points), NULL);

      }
      break;

      case Line: {
        if (vertices.size() != 2) break;
        geos::geom::CoordinateSequence *points = new geos::geom::CoordinateArraySequence();
        points->add(geos::geom::Coordinate(vertices[0].x(), vertices[0].y()));
        points->add(geos::geom::Coordinate(vertices[1].x(), vertices[1].y()));
        geos::geom::LineString *line = Isis::globalFactory.createLineString(points);
        geometry = line;
      }
      break;

      case SegmentedLine: {
        if (vertices.size() < 2) break;
        geos::geom::CoordinateSequence *points = new geos::geom::CoordinateArraySequence();

        for (int vertex = 0; vertex < vertices.size(); vertex++) {
          points->add(geos::geom::Coordinate(vertices[vertex].x(), vertices[vertex].y()));
        }

        geos::geom::LineString *line = Isis::globalFactory.createLineString(points);
        geometry = line;
      }
      break;
    }

    if (geometry != NULL && !geometry->isValid()) {
      geometry = NULL;
    }

    return geometry;
  }

  /** 
   * This method returns a rectangle from the vertices set by the 
   * RubberBandTool. It calculates the upper left and bottom right
   * points for the rectangle and properly initializes a QRect 
   * object with these vertices. If the RubberBandTool is in the 
   * incorrect mode, or the rectangle is not valid it will return 
   * an error message. 
   * 
   * 
   * @return QRect The QRect the user selected on the viewport in 
   *         pixels
   */
  QRect RubberBandTool::getRectangle(){
    QRect rect;
    
    if(getCurrentMode() == Rectangle && figureValid()) {
      QList<QPoint> vertices = getFoundVertices();

      //Check the corners for upper left and lower right.
      int x1,x2,y1,y2;
      //Point 1 is in the left, make point1.x upperleft.x
      if(vertices[0].x() < vertices[2].x()) {
        x1 = vertices[0].x(); 
        x2 = vertices[2].x();
      } 
      //Otherwise Point1 is in the right, make point1.x lowerright.x
      else {
        x1 = vertices[2].x(); 
        x2 = vertices[0].x();
      }

      //Point 1 is in the top, make point1.y upperleft.y
      if(vertices[0].y() < vertices[2].y()) {
        y1 = vertices[0].y(); 
        y2 = vertices[2].y();
      } 
      //Otherwise Point1 is in the bottom, make point1.y lowerright.y
      else {
        y1 = vertices[2].y(); 
        y2 = vertices[0].y();
      }

      rect = QRect(x1,y1,x2-x1,y2-y1);
    } 
    //Rectangle is not valid, or RubberBandTool is in the wrong mode, return error
    else {
      QMessageBox::information((QWidget *)parent(),
                               "Error","**PROGRAMMER ERROR** Invalid Rectangle");
    }

    return rect;
  }

  /** 
   * This method returns the mouse button  modifier 
   *  
   * @return MouseButton Mouse button modifier on last release
   */
  Qt::MouseButton RubberBandTool::getMouseButton() {
    return p_mouseButton;
  }

  bool RubberBandTool::figureComplete() {
    bool complete = false;

    switch (p_bandingMode) {
    case Angle:
      complete = (p_vertices.size() == 2 && p_tracking) || (p_vertices.size() == 3);
      break;
    case Line:
      complete = (p_vertices.size() == 1 && p_tracking) || (p_vertices.size() == 2);
      break;

    case Rectangle:
      complete = (p_vertices.size() == 1 && p_tracking) || (p_vertices.size() == 4);       
      break;

    case RotatedRectangle:
      complete = (p_vertices.size() == 2 && p_tracking) || (p_vertices.size() == 4);
      break;

    case Circle: 
    case Ellipse:
      complete = (p_vertices.size() == 1 && p_tracking) || (p_vertices.size() == 2);
      break;

    case SegmentedLine:
      complete = (p_vertices.size() > 1 && !p_tracking);
      break;

    case Polygon:
      complete = (p_vertices.size() > 2 && !p_tracking);
      break;
    }
    return complete;
  }

  bool RubberBandTool::figureValid() {
    if (!figureComplete()) return false;
    bool valid = true;
    QList<QPoint> vertices = getFoundVertices();

    switch (p_bandingMode) {
    case Angle: {
        // Just make sure the vertex and an angle side don't lie on the same point
        // No point tolerance allowed
        valid = vertices[0].x() != vertices[1].x() || vertices[0].y() != vertices[1].y();
        valid &= vertices[2].x() != vertices[1].x() || vertices[2].y() != vertices[1].y();
      }
      break;
    case Line: {
        // Just make sure the line doesnt start/end at same point if point not allowed
        if (p_pointTolerance == 0) {
          valid = vertices[0].x() != vertices[1].x() || vertices[0].y() != vertices[1].y();
        }
      }
      break;

    case Rectangle: {
        // Make sure there's a height and width if point not allowed
        if (p_pointTolerance == 0) {
          valid = vertices[0].x() != vertices[2].x() && vertices[0].y() != vertices[2].y();
        }
      }
      break;

    case RotatedRectangle: {
        // Make sure there's a height and width once again, point tolerance not allowed
        valid = vertices[0].x() != vertices[1].x() || vertices[0].y() != vertices[1].y();
        valid &= vertices[1].x() != vertices[2].x() || vertices[1].y() != vertices[2].y();
      }
      break;

    case Circle: 
    case Ellipse: {
        // Make sure there's a height and width, point tolerance not allowed
        valid = vertices[0].x() != vertices[1].x() && vertices[0].y() != vertices[1].y();
      }
      break;

    case SegmentedLine: {
        valid = vertices.size() > 1;
      }
      break;

    case Polygon: {
        // For polygons, we must revert back to using geos
        geos::geom::Geometry *poly = getGeometry();
        valid = poly && poly->isValid();
        delete poly;
      }
      break;
    }

    return valid;
  }

  void RubberBandTool::enablePoints(unsigned int pixTolerance) {
    p_pointTolerance = pixTolerance;
  }

  bool RubberBandTool::figureIsPoint() {
    if (!figureValid()) return false;
    bool isPoint = true;
    QList<QPoint> vertices = getFoundVertices();

    switch (p_bandingMode) {
    case Angle: 
    case RotatedRectangle: 
    case Circle: 
    case Ellipse: 
    case Polygon: 
    case SegmentedLine:
      isPoint = false;
      break;

    case Line: {
        isPoint = (abs(vertices[0].x() - vertices[1].x()) + 
                   abs(vertices[0].y() - vertices[1].y()) < (int)p_pointTolerance);
      }
      break;

    case Rectangle: {
        isPoint = (abs(vertices[0].x() - vertices[2].x()) + 
                   abs(vertices[0].y() - vertices[2].y()) < (int)p_pointTolerance);
      }
      break;
    }

    return isPoint;
  }
}
