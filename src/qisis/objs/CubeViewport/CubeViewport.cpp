/**
 * @file
 * $Date: 2008/08/11 20:40:43 $
 * $Revision: 1.22 $
 *
 *  Unless noted otherwise, the portions of Isis written by the USGS are public domain. See
 *  individual third-party library and package descriptions for intellectual property information,
 *  user agreements, and related information.
 *
 *  Although Isis has been used by the USGS, no warranty, expressed or implied, is made by the
 *  USGS as to the accuracy and functioning of such software and related material nor shall the
 *  fact of distribution constitute any such warranty, and no responsibility is assumed by the
 *  USGS in connection therewith.
 *
 *  For additional information, launch $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *  in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *  http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *  http://www.usgs.gov/privacy.html.
 */

#include <iostream>
#include <QApplication>
#include <QScrollBar>
#include <QCursor>
#include <QIcon>
#include <QPainter>
#include <QFileInfo>
#include <QMessageBox>

#include "CubeViewport.h"
#include "iException.h"
#include "Filename.h"
#include "Histogram.h"
#include "CameraFactory.h"
#include "ProjectionFactory.h"
#include "Tool.h"
#include "UniversalGroundMap.h"

namespace Qisis {
  /**
   * Construct a cube viewport
   * 
   * 
   * @param cube 
   * @param parent 
   */
  CubeViewport::CubeViewport (Isis::Cube *cube, QWidget *parent) :
  QAbstractScrollArea(parent) {
    // Is the cube usable?
    if (cube == NULL) {
      throw Isis::iException::Message(Isis::iException::Programmer,
                                      "Can not view NULL cube pointer",
                                      _FILEINFO_);
    }
    else if (!cube->IsOpen()) {
      throw Isis::iException::Message(Isis::iException::Programmer,
                                      "Can not view unopened cube",
                                      _FILEINFO_);
    }

    p_cube = cube;

    // Set up the scroll area
    setAttribute(Qt::WA_DeleteOnClose);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    viewport()->setCursor(QCursor(Qt::CrossCursor));
    viewport()->installEventFilter(this);
//    viewport()->setAttribute(Qt::WA_NoSystemBackground);
//    viewport()->setAttribute(Qt::WA_PaintOnScreen,false);

    p_saveEnabled = false;
    // Save off info about the cube
    p_scale = 1.0;
    p_color = false;
    setLinked(false);
    setCaption();

    p_redBrick = new Isis::Brick(4,1,1,cube->PixelType());
    p_grnBrick = new Isis::Brick(4,1,1,cube->PixelType());
    p_bluBrick = new Isis::Brick(4,1,1,cube->PixelType());
    p_gryBrick = new Isis::Brick(4,1,1,cube->PixelType());
    p_pntBrick = new Isis::Brick(4,1,1,cube->PixelType());

    p_paintPixmap = false;
    p_image = NULL;
    p_cubeShown = false;
    p_rubberBandEnabled = false;
    p_rubberBandDrawing = false;

    updateScrollBars(1,1);

    p_groundMap = NULL;
    p_camera = NULL;
    p_projection = NULL;

    // Setup a universal ground map
    try {
      p_groundMap = new Isis::UniversalGroundMap(*p_cube);
    }
    catch (Isis::iException &e) {
      e.Clear();
    }

    if (p_groundMap != NULL) {
      // Setup the camera or the projection
      p_camera = p_groundMap->Camera();
      p_projection = p_groundMap->Projection();
    }


    // Setup context sensitive help
    std::string cubeFileName = p_cube->Filename();
    p_whatsThisText = QString ("<b>Function: </b>Viewport to ") + cubeFileName.c_str();

    p_cubeWhatsThisText =
      "<p><b>Cube Dimensions:</b> \
      <blockQuote>Samples = " +
      QString::number(cube->Samples()) + "<br>" +
      "Lines = " +
      QString::number(cube->Lines()) + "<br>" +
      "Bands = " +
      QString::number(cube->Bands()) + "</blockquote></p>";

    /*setting up the qlist of CubeBandsStretch objs.*/
    for( int b = 0; b < p_cube->Bands(); b++) {
      CubeBandsStretch *stretch = new CubeBandsStretch();
      p_bandsStretchList.push_back(stretch); 
    }
 }

  /**
   * Destructor
   * 
   */
  CubeViewport::~CubeViewport() {
    delete p_cube;
//     Isis::Camera *p_camera;
//     Isis::Projection *p_projection;
//     Isis::UniversalGroundMap *p_groundMap;
//     Isis::Brick *p_redBrick, *p_grnBrick, *p_bluBrick,
//                 *p_gryBrick, *p_pntBrick;
//
//     QImage *p_image;

  }

  /**
   * This method sets the viewports cube.
   * 
   * @param cube 
   */
  void CubeViewport::setCube(Isis::Cube *cube) {
    p_cube = cube;
    setCaption();
  }

  /**
   * This method is called when the viewport recieves a close 
   * event. If changes have been made to this viewport it opens an
   * information dialog that asks the user if they want to save, 
   * discard changes, or cancel.
   * 
   * @param event 
   */
  void CubeViewport::closeEvent(QCloseEvent *event) {
    if(p_saveEnabled) {
      switch(QMessageBox::information(this, "Qview",
                                      "The document contains unsaved changes\n"
                                      "Do you want to save the changes before exiting?",
                                      "&Save", "&Discard", "Cancel",
                                      0,    // Enter == button 0
                                      2)) { // Escape == button 2
      //Save changes and close viewport                                     
      case 0:
        emit saveChanges();
        event->accept();
        break;
      //Discard changes and close viewport
      case 1: 
        emit discardChanges();
        event->accept();
        break;
      //Cancel, don't close viewport
      case 2: 
        event->ignore();
        break;
      }
    }
  }

  /**
   * This method is called when the cube has changed or changes 
   * have been finalized. 
   * 
   * @param changed 
   */
  void CubeViewport::cubeChanged(bool changed) {
    p_saveEnabled = changed;
  }

  /**
   *  Make viewports show up as 512 by 512
   * 
   * 
   * @return QSize 
   */
  QSize CubeViewport::sizeHint() const {
    QSize s(512,512);
    return s;
  }


  /** 
   *  Change the scale of the cube
   *  
   * @param scale 
   */
  void CubeViewport::setScale (double scale) {
    // Sanitize the scale value
    if (scale == p_scale) return;
    if (scale > 16.0) scale = 16.0;
//    if (scale < 1.0/16.0) scale = 1.0/16.0;

    // Resize the scrollbars to reflect the new scale
    double samp,line;
    contentsToCube(horizontalScrollBar()->value(),verticalScrollBar()->value(),
                   samp,line);
    p_scale = scale;
    updateScrollBars(1,1);  // Setting to 1,1 make sure we don't have bad values

    // Now update the scroll bar value to the old line/sample
    int x,y;
    cubeToContents(samp,line,x,y);
    updateScrollBars(x,y);
    
    // Notify other tools about the scale change
    emit scaleChanged();
    
    // Update the display
    setCaption();
    paintPixmap();
    viewport()->update();
  }

  /**
   *  Change the scale of the cube after moving x,y to the center
   * 
   * 
   * @param scale 
   * @param x 
   * @param y 
   */
  void CubeViewport::setScale (double scale, int x, int y) {
    double samp,line;
    viewportToCube(x,y,samp,line);
    setScale(scale,samp,line);
  }


  /**
   * Change the scale of the cube after moving samp/line to the 
   * center 
   * 
   * 
   * @param scale 
   * @param sample 
   * @param line 
   */
  void CubeViewport::setScale (double scale, double sample, double line) {
    viewport()->setUpdatesEnabled(false);
    p_paintPixmap = false;
    setScale(scale);
    p_paintPixmap = true;
    center(sample,line);
    viewport()->setUpdatesEnabled(true);
    viewport()->update();
  }


  /**
   *  Bring the cube pixel under viewport x/y to the center
   * 
   * 
   * @param x 
   * @param y 
   */
  void CubeViewport::center (int x, int y) {
    double sample,line;
    viewportToCube(x,y,sample,line);
    center(sample,line);
  }


  /**
   * Bring the cube sample/line the center
   * 
   * 
   * @param sample 
   * @param line 
   */
  void CubeViewport::center (double sample, double line) {
    // TODO:  If the x/y is close to the scrollbar values then
    // we should use the scrollBy routine to make the display faster

    int x,y;
    cubeToContents(sample,line,x,y);
    updateScrollBars(x,y);

    paintPixmap();
    viewport()->update();
  }


  /**
   *  Convert a contents x/y to a cube sample/line (may be outside
   *  the cube)
   * 
   * 
   * @param x 
   * @param y 
   * @param sample 
   * @param line 
   */
  void CubeViewport::contentsToCube(int x, int y,
                                    double &sample, double &line) const {
    sample = x / p_scale;
    line = y / p_scale;
  }


  /**
   * Convert a viewport x/y to a cube sample/line (may be outside 
   * the cube) 
   * 
   * 
   * @param x 
   * @param y 
   * @param sample 
   * @param line 
   */
  void CubeViewport::viewportToCube(int x, int y,
                                    double &sample, double &line) const {
    x += horizontalScrollBar()->value();
    x -= viewport()->width() / 2;
    y += verticalScrollBar()->value();
    y -= viewport()->height() / 2;
    contentsToCube(x,y,sample,line);
  }


  /**
   * Convert a cube sample/line to a contents x/y (should not be 
   * outside) 
   * 
   * 
   * @param sample 
   * @param line 
   * @param x 
   * @param y 
   */
  void CubeViewport::cubeToContents(double sample, double line,
                                    int &x, int &y) const {
    x = (int) (sample * p_scale + 0.5);
    y = (int) (line * p_scale + 0.5);
  }


  /**
   * Convert a cube sample/line to a viewport x/y (may be outside 
   * the viewport) 
   * 
   * 
   * @param sample 
   * @param line 
   * @param x 
   * @param y 
   */
  void CubeViewport::cubeToViewport(double sample, double line,
                                    int &x, int &y) const {
    cubeToContents(sample,line,x,y);
    x -= horizontalScrollBar()->value();
    x += viewport()->width() / 2;
    y -= verticalScrollBar()->value();
    y += viewport()->height() / 2;
  }


  /**
   * Move the scrollbars by dx/dy screen pixels
   * 
   * 
   * @param dx 
   * @param dy 
   */
  void CubeViewport::scrollBy(int dx, int dy) {
    // Make sure we don't generate bad values outside of the scroll range
    int x = horizontalScrollBar()->value() + dx;
    if (x <= 1) {
      dx = 1 - horizontalScrollBar()->value();
    }
    else if (x >= horizontalScrollBar()->maximum()) {
      dx = horizontalScrollBar()->maximum() - horizontalScrollBar()->value();
    }

    // Make sure we don't generate bad values outside of the scroll range
    int y = verticalScrollBar()->value() + dy;
    if (y <= 1) {
      dy = 1 - verticalScrollBar()->value();
    }
    else if (y >= verticalScrollBar()->maximum()) {
      dy = verticalScrollBar()->maximum() - verticalScrollBar()->value();
    }

    // Do we do anything?
    if ((dx == 0) && (dy == 0)) return;

    // We do so update the scroll bars
    updateScrollBars(horizontalScrollBar()->value()+dx,
                     verticalScrollBar()->value()+dy);

    // Scroll the the pixmap
    scrollContentsBy(-dx,-dy);
  }


  /**
   * Scroll the viewport contents by dx/dy screen pixels
   * 
   * 
   * @param dx 
   * @param dy 
   */
  void CubeViewport::scrollContentsBy(int dx, int dy) {
    // We shouldn't do anything if scrollbars are being updated
    if (viewport()->signalsBlocked()) return;

    // Prep to scroll the pixmap
    int x = dx;
    int sx = 0;
    if (dx < 0) {
      x = 0;
      sx = -dx;
    }

    int y = dy;
    int sy = 0;
    if (dy < 0) {
      y = 0;
      sy = -dy;
    }

    // See if we need to repaint the whole viewport
    if ((abs(dx) >= viewport()->width()*0.9) ||
        (abs(dy) >= viewport()->height()*0.9)) {
      paintPixmap();
      viewport()->repaint();
      return;
    }

    // Ok we can shift the pixmap and filling
    QPainter p(&p_pixmap);

//  Apple's Intel and PowerPC 10.4 (Tiger) OS do not handle shifting a pixmap
// in place.  This makes a copy of the whole pixmap and shifts it using the
// copy.
#if defined(__APPLE__)
    QPixmap macCopy = p_pixmap.copy();
    p.drawPixmap(x,y,macCopy,sx,sy,p_pixmap.width()-sx+1,p_pixmap.height()-sy+1);
#else
    p.drawPixmap(x,y,p_pixmap,sx,sy,p_pixmap.width()-sx+1,p_pixmap.height()-sy+1);
#endif
    p.end();

    // Now fillin the left or right side
    if (dx > 0) {
      QRect rect(0,0,dx,p_pixmap.height());
      paintPixmap(rect);
    }
    else if (dx < 0) {
      QRect rect(p_pixmap.width()+dx,0,-dx,p_pixmap.height());
      paintPixmap(rect);
    }

    // Now fillin the top or bottom side
    if (dy > 0) {
      QRect rect(0,0,p_pixmap.width(),dy);
      paintPixmap(rect);
    }
    else if (dy < 0) {
      QRect rect(0,p_pixmap.height()+dy,p_pixmap.width(),-dy);
      paintPixmap(rect);
    }

    viewport()->repaint();
  }


  /**
   * Change the caption on the viewport title bar
   * 
   */
  void CubeViewport::setCaption () {
    std::string cubeFilename= p_cube->Filename();
    QString str = QFileInfo(cubeFilename.c_str()).fileName();
    str += QString(" @ ");
    str += QString::number(p_scale*100.0);
    str += QString("% ");

    if (p_color) {
      str += QString("(RGB = ");
      str += QString::number(p_red.band);
      str += QString(",");
      str += QString::number(p_green.band);
      str += QString(",");
      str += QString::number(p_blue.band);
      str += QString (")");
    }
    else {
      str += QString("(Gray = ");
      str += QString::number(p_gray.band);
      str += QString (")");
    }

    //If changes have been made make sure to add '*' to the end
    if(p_saveEnabled) {
      str += "*";
    }

    QWidget::setWindowTitle ( str );
    emit windowTitleChanged();
  }


  /**
   * Change the linked state of the viewport
   * 
   * 
   * @param b 
   */
  void CubeViewport::setLinked(bool b) {
    std::string unlinkedIcon = Isis::Filename("$base/icons/unlinked.png").Expanded();
    static QIcon unlinked(unlinkedIcon.c_str());
    std::string linkedIcon = Isis::Filename("$base/icons/linked.png").Expanded();
    static QIcon linked(linkedIcon.c_str());

    bool notify = false;
    if (b != p_linked) notify = true;

    p_linked = b;
    if (p_linked) {
      setWindowIcon(linked);
    }
    else {
      setWindowIcon(unlinked);
    }
    if (notify) emit linkChanging(b);
  }


  /**
   * The viewport is being resized
   * 
   * 
   * @param e 
   */
  void CubeViewport::resizeEvent (QResizeEvent *e) {
    // Change the size of the image and pixmap
    if (p_image != NULL) delete p_image;
    p_image = new QImage (viewport()->size(),QImage::Format_RGB32);
    p_pixmap = QPixmap(viewport()->size());

    // Fixup the scroll bars
    updateScrollBars(horizontalScrollBar()->value(),
                     verticalScrollBar()->value());

    p_viewportWhatsThisText =
      "<p><b>Viewport Dimensions:</b> \
      <blockQuote>Samples = " +
      QString::number(viewport()->width()) + "<br>" +
      "Lines = " +
      QString::number(viewport()->height()) + "</blockquote></p>";

    // Repaint the pixmap
    paintPixmap();
    viewport()->repaint();
  }

  /**
   * Repaint the viewport
   * 
   * @param e [in]  (QPaintEvent *)  event
   * 
   * @internal
   *
   * @history  2007-04-30  Tracie Sucharski - Add the QPainter to the call to
   *                           Tool::paintViewport.
   */
  void CubeViewport::paintEvent (QPaintEvent *e) {
    if (!p_cubeShown) return;
    QPainter painter(viewport());
    painter.drawPixmap(0,0,p_pixmap);
    emit viewportUpdated();

    // Write the rubberband if necessary
    if (p_rubberBandDrawing) {
      QPen pen(Qt::DashDotLine);
      pen.setColor(QColor("Red"));
      painter.setPen(pen);

      if (p_rubberBandShape == Rectangle) {
        painter.drawLine(p_rubberBandRect.topLeft(),p_rubberBandRect.topRight());
        painter.drawLine(p_rubberBandRect.topRight(),p_rubberBandRect.bottomRight());
        painter.drawLine(p_rubberBandRect.bottomRight(),p_rubberBandRect.bottomLeft());
        painter.drawLine(p_rubberBandRect.bottomLeft(),p_rubberBandRect.topLeft());
      }
      else {
        painter.drawLine(p_rubberBandLine.p1(),p_rubberBandLine.p2());
      }
    }

    // Draw anything the tools might need
    for (int i=0; i < p_toolList.size(); i++) {
      p_toolList[i]->paintViewport(this,&painter);
    }

    painter.end(); 
  }

  //! Paint the whole pixmap
  void CubeViewport::paintPixmap() {
    QRect rect(0,0,viewport()->width(),viewport()->height());
    paintPixmap(rect);
  }

 
  /**
   * Paint a region of the pixmap
   * 
   * 
   * @param rect 
   */
  void CubeViewport::paintPixmap (QRect rect) {
    if (!p_paintPixmap) return;
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Get the chunk to paint
    int clipx = rect.x();
    int clipy = rect.y();
    int clipw = rect.width();
    int cliph = rect.height();

    // Prep to create minimal buffer(s) to read the cube
    double ssamp,esamp,temp;
    viewportToCube(clipx,clipy,ssamp,temp);
    viewportToCube(clipx+clipw-1,clipy,esamp,temp);
    int bufns = (int) (ceil(esamp) - floor(ssamp)) + 1;

    // Write to the qimage in color if selected
    if (p_color) {
      p_redBrick->Resize(bufns,1,1);
      p_grnBrick->Resize(bufns,1,1);
      p_bluBrick->Resize(bufns,1,1);

      double samp,line;
      for (int y=clipy; y <= (const int)(clipy+cliph-1); y++) {
        viewportToCube(clipx,y,samp,line);
        p_redBrick->SetBasePosition((int)(samp+0.5),(int)(line+0.5),p_red.band);
        p_grnBrick->SetBasePosition((int)(samp+0.5),(int)(line+0.5),p_green.band);
        p_bluBrick->SetBasePosition((int)(samp+0.5),(int)(line+0.5),p_blue.band);
        p_cube->Read(*p_redBrick);
        p_cube->Read(*p_grnBrick);
        p_cube->Read(*p_bluBrick);
        QRgb *rgb = (QRgb *) p_image->scanLine(y-clipy);
        int r,g,b,index;
        for (int x=clipx; x <= (const int)(clipx+clipw-1); x++) {
          viewportToCube(x,y,samp,line);
          index = (int)(samp + 0.5) - p_redBrick->Sample();
          if ((index < 0) || (index >= p_redBrick->size())) {
            rgb[x-clipx] = qRgb(0,0,0);
          }
          else {
            r = (int) p_red.stretch.Map((*p_redBrick)[index]);
            g = (int) p_green.stretch.Map((*p_grnBrick)[index]);
            b = (int) p_blue.stretch.Map((*p_bluBrick)[index]);
            rgb[x-clipx] = qRgb(r,g,b);
          }
        }
      }
    }

    // Otherwise write to the qimage in b/w
    else {
      p_gryBrick->Resize(bufns,1,1);

      double samp,line;
      for (int y=clipy; y <= (const int)(clipy+cliph-1); y++) {
        viewportToCube(clipx,y,samp,line);
        p_gryBrick->SetBasePosition((int)(samp+0.5),(int)(line+0.5),p_gray.band);
        p_cube->Read(*p_gryBrick);
        QRgb *rgb = (QRgb *) p_image->scanLine(y-clipy);
        int r,g,b,index;
        for (int x=clipx; x <= (const int)(clipx+clipw-1); x++) {
          viewportToCube(x,y,samp,line);
          index = (int)(samp + 0.5) - p_gryBrick->Sample();
          if ((index < 0) || (index >= p_gryBrick->size())) {
            rgb[x-clipx] = qRgb(0,0,0);
          }
          else {
            r = (int) p_red.stretch.Map((*p_gryBrick)[index]);
            g = (int) p_green.stretch.Map((*p_gryBrick)[index]);
            b = (int) p_blue.stretch.Map((*p_gryBrick)[index]);
            rgb[x-clipx] =  qRgb(r,g,b);
          }
        }
      }
    }

    // Copy image to the pixmap
    QPainter p(&p_pixmap);
    p.drawImage(QPoint(clipx,clipy),*p_image,QRect(0,0,clipw,cliph));
    QApplication::restoreOverrideCursor();

    // Change whats this info
    updateWhatsThis();
  }


  /**
   * Update the What's This text.
   * 
   */
  void CubeViewport::updateWhatsThis() {
    double sl,ss;
    viewportToCube(0,0,ss,sl);
    if (ss < 1.0) ss = 1.0;
    if (sl < 1.0) sl = 1.0;

    double el,es;
    viewportToCube(viewport()->width()-1,viewport()->height()-1,es,el);
    if (es > cubeSamples()) es = cubeSamples();
    if (el > cubeLines()) el = cubeLines();

    QString area =
      "<p><b>Visible Cube Area:</b><blockQuote> \
      Samples = " + QString::number(int(ss+0.5)) + "-" +
      QString::number(int(es+0.5)) + "<br> \
      Lines = " + QString::number(int(sl+0.5)) + "-" +
      QString::number(int(el+0.5)) + "<br></blockQuote></p>";
    viewport()->setWhatsThis(p_whatsThisText+
                             area+
                             p_cubeWhatsThisText+
                             p_viewportWhatsThisText);
  }


  /**
   * Turn the rubberband on/off
   * 
   * 
   * @param enable 
   * @param shape 
   */
  void CubeViewport::enableRubberBand(bool enable,RubberBandShape shape) {
    p_rubberBandEnabled = enable;
    p_rubberBandShape = shape;
  }


  /**
   * Return the rubber band rectangle
   * 
   * 
   * @return QRect 
   */
  QRect CubeViewport::rubberBandRect () {
    QRect r = p_rubberBandRect.normalized();
    if (r.top() < 0) r.setTop(0);
    if (r.left() < 0) r.setLeft(0);
    if (r.bottom() >= viewport()->height()) r.setBottom(viewport()->height()-1);
    if (r.right() >= viewport()->width()) r.setRight(viewport()->width()-1);
    return r;
  }


  /**
   * Return the points making up a rubberBand line
   * 
   * 
   * @return QLine 
   */
  QLine CubeViewport::rubberBandLine () {

    QPoint p1 = p_rubberBandLine.p1();
    if (p1.x() < 0) p1.setX(0);
    if (p1.y() < 0) p1.setY(0);
    QPoint p2 = p_rubberBandLine.p2();
    if (p2.x() > viewport()->width()) p2.setX(viewport()->width() - 1);
    if (p2.y() > viewport()->height()) p2.setX(viewport()->height() - 1);

    QLine l(p1,p2);
    
    return l;
  }


  /**
   * Return the red pixel value at a sample/line
   * 
   * 
   * @param sample 
   * @param line 
   * 
   * @return double 
   */
  double CubeViewport::redPixel (int sample, int line) {
    p_pntBrick->SetBasePosition(sample,line,p_red.band);
    p_cube->Read(*p_pntBrick);
    return (*p_pntBrick)[0];
  }


  /**
   * Return the green pixel value at a sample/line
   * 
   * 
   * @param sample 
   * @param line 
   * 
   * @return double 
   */
  double CubeViewport::greenPixel (int sample, int line) {
    p_pntBrick->SetBasePosition(sample,line,p_green.band);
    p_cube->Read(*p_pntBrick);
    return (*p_pntBrick)[0];
  }


  /**
   * Return the blue pixel value at a sample/line
   * 
   * 
   * @param sample 
   * @param line 
   * 
   * @return double 
   */
  double CubeViewport::bluePixel (int sample, int line) {
    p_pntBrick->SetBasePosition(sample,line,p_blue.band);
    p_cube->Read(*p_pntBrick);
    return (*p_pntBrick)[0];
  }


  /**
   * Return the gray pixel value at a sample/line
   * 
   * 
   * @param sample 
   * @param line 
   * 
   * @return double 
   */
  double CubeViewport::grayPixel (int sample, int line) {
    p_pntBrick->SetBasePosition(sample,line,p_gray.band);
    p_cube->Read(*p_pntBrick);
    return (*p_pntBrick)[0];
  }


  /**
   * Event filter to watch for mouse events on viewport
   * 
   * 
   * @param o 
   * @param e 
   * 
   * @return bool 
   */
  bool CubeViewport::eventFilter(QObject *o,QEvent *e) {
    // Handle standard mouse tracking on the viewport
    if (o == viewport()) {
      switch (e->type()) {
        case QEvent::Enter:
        {
          viewport()->setMouseTracking(true);
          emit mouseEnter();
          return TRUE;
        }

        case QEvent::MouseMove:
        {
          QMouseEvent *m = (QMouseEvent *) e;
          if (p_rubberBandDrawing) {
            if (p_rubberBandShape == Rectangle) {
              p_rubberBandRect = QRect(p_rubberBandRect.topLeft(),m->pos());
            }
            else {
              p_rubberBandLine = QLine(p_rubberBandLine.p1(),m->pos());
            }
            viewport()->repaint();
          }

          emit mouseMove(m->pos());
          return TRUE;
        }

        case QEvent::Leave: {
          viewport()->setMouseTracking(false);
          emit mouseLeave();
          return TRUE;
        }

        case QEvent::MouseButtonPress: {
          QMouseEvent *m = (QMouseEvent *) e;
          if ((m->button() == Qt::LeftButton) &&
              (m->modifiers() == 0) &&
              (p_rubberBandEnabled)) {
            p_rubberBandDrawing = true;
            if (p_rubberBandShape == Rectangle) {
              p_rubberBandRect = QRect(m->pos(),m->pos());
            }
            else {
              p_rubberBandLine = QLine(m->pos(),m->pos());
            }
          }

          emit mouseButtonPress(m->pos(),
                               (Qt::MouseButton)(m->button()+m->modifiers()));
          return TRUE;
        }

        case QEvent::MouseButtonRelease: {
          QMouseEvent *m = (QMouseEvent *) e;
          if ((m->button() == Qt::LeftButton) && (p_rubberBandDrawing)) {
            p_rubberBandDrawing = false;
            if (p_rubberBandShape == Rectangle) {
              p_rubberBandRect = QRect(p_rubberBandRect.topLeft(),m->pos());
            }
            else {
              p_rubberBandLine = QLine(p_rubberBandLine.p1(),m->pos());
            }
          }

          emit mouseButtonRelease(m->pos(),
                                  (Qt::MouseButton)(m->button()+m->modifiers()));
          return TRUE;
        }

        case QEvent::MouseButtonDblClick: {
          QMouseEvent *m = (QMouseEvent *) e;
          emit mouseDoubleClick(m->pos());
          return TRUE;
        }

        default: {
          return FALSE;
        }
      }
    }
    else {
      return QAbstractScrollArea::eventFilter(o,e);
    }
  }


  /**
   * Process arrow keystrokes on cube
   * 
   * 
   * @param e 
   */
  void CubeViewport::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Plus) {
      double scale = p_scale * 2.0;
      setScale(scale);
      e->accept();
    }
    else if (e->key() == Qt::Key_Minus) {
      double scale = p_scale / 2.0;
      setScale(scale);
      e->accept();
    }
    else if (e->key() == Qt::Key_Up) {
      moveCursor (0,-1);
      e->accept();
    }
    else if (e->key() == Qt::Key_Down) {
      moveCursor (0,1);
      e->accept();
    }
    else if (e->key() == Qt::Key_Left) {
      moveCursor (-1,0);
      e->accept();
    }
    else if (e->key() == Qt::Key_Right) {
      moveCursor (1,0);
      e->accept();
    }
    else {
      QAbstractScrollArea::keyPressEvent(e);
    }
  }


  /**
   * Is cursor inside viewport
   * 
   * 
   * @return bool 
   */
  bool CubeViewport::cursorInside() const {
    QPoint g = QCursor::pos();
    QPoint v = viewport()->mapFromGlobal(g);
    if (v.x() < 0) return false;
    if (v.y() < 0) return false;
    if (v.x() >= viewport()->width()) return false;
    if (v.y() >= viewport()->height()) return false;
    return true;
  }


  /**
   * Return the cursor position in the viewport
   * 
   * 
   * @return QPoint 
   */
  QPoint CubeViewport::cursorPosition() const {
    QPoint g = QCursor::pos();
    return viewport()->mapFromGlobal(g);
  }


  /**
   * Move the cursor by x,y if possible
   * 
   * 
   * @param x 
   * @param y 
   */
  void CubeViewport::moveCursor(int x, int y) {
    QPoint g = QCursor::pos();
    g += QPoint(x,y);
    QPoint v = viewport()->mapFromGlobal(g);
    if (v.x() < 0) return;
    if (v.y() < 0) return;
    if (v.x() >= viewport()->width()) return;
    if (v.y() >= viewport()->height()) return;
    QCursor::setPos(g);
  }


  /**
   * Set the cursor position to x/y in the viewport
   * 
   * 
   * @param x 
   * @param y 
   */
  void CubeViewport::setCursorPosition(int x, int y) {
    QPoint g(x,y);
    QPoint v = viewport()->mapToGlobal(g);
    QCursor::setPos(v);
  }


  /**
   * Update the scroll bar
   * 
   * 
   * @param x 
   * @param y 
   */
  void CubeViewport::updateScrollBars(int x, int y) {
    viewport()->blockSignals(true);

    verticalScrollBar()->setValue(1);
    verticalScrollBar()->setMinimum(1);
    verticalScrollBar()->setMaximum((int)(ceil(cubeLines() * p_scale) + 0.5));
    verticalScrollBar()->setPageStep(viewport()->height() / 2);

    horizontalScrollBar()->setValue(1);
    horizontalScrollBar()->setMinimum(1);
    horizontalScrollBar()->setMaximum((int)(ceil(cubeSamples() * p_scale) + 0.5));
    horizontalScrollBar()->setPageStep(viewport()->width() / 2);

    if(horizontalScrollBar()->value() != x || verticalScrollBar()->value() != y) {
        horizontalScrollBar()->setValue(x);
        verticalScrollBar()->setValue(y);
        emit scaleChanged();
    }

    QApplication::sendPostedEvents(viewport(),0);
    viewport()->blockSignals(false);
  }


  /**
   * View cube as gray
   * 
   * 
   * @param band 
   */
  void CubeViewport::viewGray(int band) {
    p_gray.band = band;
    p_color = false;
    setCaption();
    if (!p_bandsStretchList[band-1]->p_stretched){
      autoStretch ();
    }else {
    
      QString str = QString::fromStdString(p_gray.stretch.Text());
      p_gray.stretch.ClearPairs();
      p_gray.stretch.AddPair(p_bandsStretchList[band-1]->p_stretchMin,0.0);
      p_gray.stretch.AddPair(p_bandsStretchList[band-1]->p_stretchMax,255.0);
      stretchGray(QString::fromStdString(p_gray.stretch.Text()));
      
    }
    
    paintPixmap();
    viewport()->update();
  }


  /**
   * View cube as color
   * 
   * 
   * @param rband 
   * @param gband 
   * @param bband 
   */
  void CubeViewport::viewRGB(int rband, int gband, int bband) {
    p_red.band = rband;
    p_green.band = gband;
    p_blue.band = bband;
    p_color = true;
    setCaption();
    autoStretch ();
    viewport()->update();
  }


  /**
   * Apply stretch pairs to rgb bands
   * 
   * 
   * @param rstr 
   * @param gstr 
   * @param bstr 
   */
  void CubeViewport::stretchRGB (const QString &rstr,
                                 const QString &gstr,
                                 const QString &bstr) {
    p_red.stretch.Parse(rstr.toStdString());
    p_green.stretch.Parse(gstr.toStdString());
    p_blue.stretch.Parse(bstr.toStdString());
    paintPixmap();
    viewport()->update();
  }


  /**
   *  Apply stretch pairs to gray band
   * 
   * 
   * @param gstr 
   */
  void CubeViewport::stretchGray (const QString &gstr) {
    stretchRGB(gstr,gstr,gstr);
 //   p_gray.stretch.Parse(gstr.toStdString());
 //   paintPixmap();
 //   viewport()->update();
  }


  /**
   * Overwrite the current red, green, and bule stretches with the 
   * input 
   * 
   * 
   * @param rstr 
   * @param gstr 
   * @param bstr 
   */
  void CubeViewport::stretchRGB(const Isis::Stretch &rstr,
                                const Isis::Stretch &gstr,
                                const Isis::Stretch &bstr) {
    p_red.stretch = rstr;
    p_green.stretch = gstr;
    p_blue.stretch = bstr;
    paintPixmap();
    viewport()->update();
  }

  /**
   * Determine the scale that causes the full cube to fit in the viewport.
   * 
   * @return The scale
   * 
   * @internal
   */
  double CubeViewport::fitScale () const {
    double sampScale = (double) viewport()->width() / (double) cubeSamples();
    double lineScale = (double) viewport()->height() / (double) cubeLines();
    double scale = sampScale < lineScale ? sampScale:lineScale;
//    scale = floor(scale * 100.0) / 100.0;
    return scale;
  }


  /**
   * Determine the scale of cube in the width to fit in the viewport
   * 
   * @return The scale for width
   * 
   */
  double CubeViewport::fitScaleWidth () const {
    double scale = (double) viewport()->width() / (double) cubeSamples();

//    scale = floor(scale * 100.0) / 100.0;
    return scale;
  }

  /**
   * Determine the scale of cube in heighth to fit in the viewport
   * 
   * @return The scale for heighth
   * 
   */
  double CubeViewport::fitScaleHeight () const {
    double scale = (double) viewport()->height() / (double) cubeLines();

//    scale = floor(scale * 100.0) / 100.0;
    return scale;
  }

  /**
   *  Visualize the cube
   * 
   */
  void CubeViewport::showCube() {
    if (p_cubeShown) return;
    p_cubeShown = true;

    updateScrollBars(horizontalScrollBar()->value(),
                     verticalScrollBar()->value());

    p_paintPixmap = true;
    autoStretch();
    setCaption();

    viewport()->update();
    viewport()->setAttribute(Qt::WA_NoBackground);
    return;
  }

  /**
   *   Cube changed, repaint given area
   * 
   * @param[in] cubeRect (QRect rect)  Rectange containing portion of cube
   *                                  (sample/line) that changed.
   * 
   */
  void CubeViewport::cubeContentsChanged(QRect cubeRect) {

    double ss,sl,es,el;
    ss = (double)(cubeRect.left()) - 1.;
    sl = (double)(cubeRect.top()) - 1.;
    es = (double)(cubeRect.right()) + 1.;
    el = (double)(cubeRect.bottom()) + 1.;
    if (ss < 1) ss = 0.5;
    if (sl < 1) sl = 0.5;
    if (es > cube()->Samples()) es = cube()->Samples() + 0.5;
    if (el > cube()->Lines()) el = cube()->Lines() + 0.5;

    int sx,sy,ex,ey;

    cubeToViewport(ss,sl,sx,sy);
    cubeToViewport(es,el,ex,ey);
    if (sx < 0) sx = 0;
    if (sy < 0) sy = 0;
    if (ex > viewport()->width()) ex = viewport()->width();
    if (ey > viewport()->height()) ey = viewport()->height();
    QRect vpRect(sx,sy,ex-sx+1,ey-sy+1);
    paintPixmap(vpRect);
    viewport()->repaint();
  }


  /**
   * Apply automatic stretch using data from entire cube
   * 
   * 
   * @param lineRate 
   */
  void CubeViewport::autoStretch (int lineRate) {
    autoStretch(1,cubeSamples(),1,cubeLines(),lineRate);
  }


  /**
   * Apply automatic stretch using data from a portion of the cube
   * 
   * 
   * @param ssamp 
   * @param esamp 
   * @param sline 
   * @param eline 
   * @param lineRate 
   */
  void CubeViewport::autoStretch (int ssamp, int esamp,
                                  int sline, int eline, int lineRate) {
    if (lineRate <= 0) lineRate = (int) ((eline - sline + 1.0) * 0.1);
    if (lineRate <= 0) lineRate = 1;

    if (p_color) {
      computeStretch(p_redBrick,p_red.band,
                     ssamp,esamp,sline,eline,lineRate,
                     p_red.stretch);
      computeStretch(p_grnBrick,p_green.band,
                     ssamp,esamp,sline,eline,lineRate,
                     p_green.stretch);
      computeStretch(p_bluBrick,p_blue.band,
                     ssamp,esamp,sline,eline,lineRate,
                     p_blue.stretch);
    }
    else {
      computeStretch(p_gryBrick,p_gray.band,
                     ssamp,esamp,sline,eline,lineRate,
                     p_gray.stretch);

      p_red.stretch.ClearPairs();
      for (int i=0; i < p_gray.stretch.Pairs(); i++) {
        p_red.stretch.AddPair(p_gray.stretch.Input(i),p_gray.stretch.Output(i));
      }

      p_green.stretch.ClearPairs();
      for (int i=0; i < p_gray.stretch.Pairs(); i++) {
        p_green.stretch.AddPair(p_gray.stretch.Input(i),p_gray.stretch.Output(i));
      }

      p_blue.stretch.ClearPairs();
      for (int i=0; i < p_gray.stretch.Pairs(); i++) {
        p_blue.stretch.AddPair(p_gray.stretch.Input(i),p_gray.stretch.Output(i));
      }

    }
    paintPixmap();
    viewport()->update();
  }


  /**
   * Compute automatic stretch for a portion of the cube
   * 
   * 
   * @param brick 
   * @param band 
   * @param ssamp 
   * @param esamp 
   * @param sline 
   * @param eline 
   * @param lineRate 
   * @param stretch 
   */
  void CubeViewport::computeStretch(Isis::Brick *brick, int band,
                                    int ssamp, int esamp,
                                    int sline, int eline, int lineRate,
                                    Isis::Stretch &stretch) {
    Isis::Statistics stats;
    int bufns = esamp - ssamp + 1;
    brick->Resize(bufns,1,1);

    for (int line=sline; line <= eline; line+=lineRate) {
      brick->SetBasePosition(ssamp,line,band);
      p_cube->Read(*brick);
      stats.AddData(brick->DoubleBuffer(),bufns);
    }

	if(fabs(stats.BestMinimum()) < DBL_MAX && fabs(stats.BestMaximum()) < DBL_MAX) {
      Isis::Histogram hist(stats.BestMinimum(),stats.BestMaximum());
      for (int line=sline; line <= eline; line+=lineRate) {
        brick->SetBasePosition(ssamp,line,band);
        p_cube->Read(*brick);
        hist.AddData(brick->DoubleBuffer(),bufns);
      }

      stretch.ClearPairs();
      if (hist.Percent(0.5) != hist.Percent(99.5)) {
        stretch.AddPair(hist.Percent(0.5),0.0);
        stretch.AddPair(hist.Percent(99.5),255.0);
      }
      else {
        stretch.AddPair(-DBL_MAX,0.0);
        stretch.AddPair(DBL_MAX,255.0);
      }
	}
	else {
	  stretch.ClearPairs();
      stretch.AddPair(-DBL_MAX,0.0);
      stretch.AddPair(DBL_MAX,255.0);
	}
  }

  /**
   * Registers the tool given tool.
   * 
   * 
   * @param tool 
   */
  void CubeViewport::registerTool (Tool *tool) {
    p_toolList.push_back(tool);
  }

  /** 
   * 
   * 
   * @param band
   * @param stretchFlag
   * @param min
   * @param max
   */
  void CubeViewport::setStretchInfo(int band, bool stretchFlag, double min, double max){
      p_bandsStretchList[band-1]->p_stretchMin = min;
      p_bandsStretchList[band-1]->p_stretchMax = max;
      p_bandsStretchList[band-1]->p_stretched = stretchFlag;

      p_gray.stretch.ClearPairs();
      p_gray.stretch.AddPair(min,0.0);
      p_gray.stretch.AddPair(max,255.0);
   
  }

  /** 
   * 
   * 
   * @param band
   * 
   * @return double
   */
  double CubeViewport::getStretchMin(int band){
    return p_bandsStretchList[band-1]->p_stretchMin;
  }

  /** 
   * 
   * 
   * @param band
   * 
   * @return double
   */
  double CubeViewport::getStretchMax(int band){
    return p_bandsStretchList[band-1]->p_stretchMax;
  }

  /** 
   * 
   * 
   * @param band
   * 
   * @return bool
   */
  bool CubeViewport::getStretchFlag(int band){
    return p_bandsStretchList[band-1]->p_stretched;
  }


  /**
   * Allows users to change the cursor type on the viewport.
   * 
   * 
   * @param cursor 
   */
  void CubeViewport::changeCursor(QCursor cursor){
    viewport()->setCursor(cursor);
  }
}
