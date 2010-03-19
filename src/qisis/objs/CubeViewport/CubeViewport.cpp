/**
 * @file
 * $Date: 2009/10/23 19:16:12 $
 * $Revision: 1.34 $
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

#include <iomanip>
#include <iostream>
#include <QApplication>
#include <QCursor>
#include <QIcon>
#include <QPen>
#include <QPainter>
#include <QFileInfo>
#include <QMessageBox>

#include "CubeViewport.h"
#include "iException.h"
#include "Filename.h"
#include "Histogram.h"
#include "Tool.h"

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
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
//    viewport()->setAttribute(Qt::WA_NoSystemBackground);
//    viewport()->setAttribute(Qt::WA_PaintOnScreen,false);

    p_saveEnabled = false;
    // Save off info about the cube
    //p_scale = 1.0;
    p_color = false;
    p_linked = false;

    std::string unlinkedIcon = Isis::Filename("$base/icons/unlinked.png").Expanded();
    static QIcon unlinked(unlinkedIcon.c_str());
    setWindowIcon(unlinked);


    setCaption();

    p_redBrick = new Isis::Brick(4,1,1,cube->PixelType());
    p_grnBrick = new Isis::Brick(4,1,1,cube->PixelType());
    p_bluBrick = new Isis::Brick(4,1,1,cube->PixelType());
    p_gryBrick = new Isis::Brick(4,1,1,cube->PixelType());
    p_pntBrick = new Isis::Brick(4,1,1,cube->PixelType());

    p_paintPixmap = false;
    p_image = NULL;
    p_cubeShown = true;

    //updateScrollBars(1,1);

    p_groundMap = NULL;
    p_projection = NULL;
    p_camera = NULL;
    
    // Setup a universal ground map
    try {
      p_groundMap = new Isis::UniversalGroundMap(*p_cube);
    }
    catch (Isis::iException &e) {
      e.Clear();
    }

    if (p_groundMap != NULL) {
      // Setup the camera or the projection
      if(p_groundMap->Camera() != NULL) {
        p_camera = p_groundMap->Camera();
        if(p_camera->HasProjection()) {
          try{
            p_projection = cube->Projection();
          } catch (Isis::iException &e) {
            e.Clear();
          }
        }
      }
      else {
        p_projection = p_groundMap->Projection();
      }
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

    /*setting up the qlist of CubeBandsStretch objs.
    for( int b = 0; b < p_cube->Bands(); b++) {
      CubeBandsStretch *stretch = new CubeBandsStretch();
      p_bandsStretchList.push_back(stretch); 
    }*/

    p_grayBuffer = new ViewportBuffer(this, p_cube);
    p_grayBuffer->enable(false);
    p_grayBuffer->setBand(1);

    p_redBuffer = NULL;
    p_greenBuffer = NULL;
    p_blueBuffer = NULL;

    p_bgColor = Qt::black;
 }

  /**
   * This method is called to initially show the viewport. It will set the
   * scale to show the entire cube and enable the gray buffer.
   * 
   */
  void CubeViewport::show() {
    double sampScale = (double) sizeHint().width() / (double) cubeSamples();
    double lineScale = (double) sizeHint().height() / (double) cubeLines();
    double scale = sampScale < lineScale ? sampScale:lineScale;

    setScale(scale, cubeSamples()/2.0, cubeLines()/2.0);
    p_grayBuffer->enable(true);

    QAbstractScrollArea::show();

    p_paintPixmap = true;
    paintPixmap();
  }

  /**
   * Destructor
   * 
   */
  CubeViewport::~CubeViewport() {
    delete p_cube;
    delete p_redBrick;
    delete p_grnBrick;
    delete p_bluBrick;
    delete p_gryBrick;
    delete p_pntBrick;
    delete p_groundMap;
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
    //if (scale < 1.0/16.0) scale = 1.0/16.0;

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

    if(p_grayBuffer) p_grayBuffer->scaleChanged();
    if(p_redBuffer) p_redBuffer->scaleChanged();
    if(p_greenBuffer) p_greenBuffer->scaleChanged();
    if(p_blueBuffer) p_blueBuffer->scaleChanged();

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

    bool wasEnabled = (p_grayBuffer && p_grayBuffer->enabled()) || (p_redBuffer && p_redBuffer->enabled());
    if(p_grayBuffer) p_grayBuffer->enable(false);
    if(p_redBuffer) p_redBuffer->enable(false); 
    if(p_greenBuffer) p_greenBuffer->enable(false);
    if(p_blueBuffer) p_blueBuffer->enable(false);

    if(p_paintPixmap) {
      p_paintPixmap = false;
      setScale(scale);
      p_paintPixmap = true;
    }
    else {
      setScale(scale);
    }

    center(sample,line);

    if(p_grayBuffer) p_grayBuffer->enable(wasEnabled);
    if(p_redBuffer) p_redBuffer->enable(wasEnabled); 
    if(p_greenBuffer) p_greenBuffer->enable(wasEnabled);
    if(p_blueBuffer) p_blueBuffer->enable(wasEnabled);

    paintPixmap();
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

    int panX = horizontalScrollBar()->value() - x;
    int panY = verticalScrollBar()->value() - y;

    updateScrollBars(x,y);

    if(p_grayBuffer) p_grayBuffer->pan(panX, panY);
    if(p_redBuffer) p_redBuffer->pan(panX, panY);
    if(p_greenBuffer) p_greenBuffer->pan(panX, panY);
    if(p_blueBuffer) p_blueBuffer->pan(panX, panY);

    //paintPixmap();
    //viewport()->update();
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
    if (viewport()->signalsBlocked()) {
      return;
    }

    if(p_grayBuffer) p_grayBuffer->pan(dx, dy);
    if(p_redBuffer) p_redBuffer->pan(dx, dy);
    if(p_greenBuffer) p_greenBuffer->pan(dx, dy);
    if(p_blueBuffer) p_blueBuffer->pan(dx, dy);

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

    emit scaleChanged();
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
      parentWidget()->setWindowIcon(linked);
    }
    else {
      parentWidget()->setWindowIcon(unlinked);
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
    if(p_grayBuffer) p_grayBuffer->resizedViewport();
    if(p_redBuffer) p_redBuffer->resizedViewport();
    if(p_greenBuffer) p_greenBuffer->resizedViewport();
    if(p_blueBuffer) p_blueBuffer->resizedViewport();

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

    QPainter p(&p_pixmap);
    p.fillRect(rect, QBrush(p_bgColor));

    QRect dataArea;

    if(p_grayBuffer && p_grayBuffer->enabled()) {
      dataArea = QRect( p_grayBuffer->bufferXYRect().intersected( rect ) );

      for(int y = dataArea.top(); !dataArea.isNull() && y <= dataArea.bottom(); y++) {
        const std::vector<double> &line = p_grayBuffer->getLine(y - p_grayBuffer->bufferXYRect().top());
        QRgb *rgb = (QRgb *) p_image->scanLine(y);
  
        for(int x = dataArea.left(); x <= dataArea.right(); x++) {
          //int grayscalePix = (int)(p_gray.stretch.Map( line[ x - p_grayBuffer->bufferXYRect().left() ] ) + 0.5);

          int redPix = (int)(p_red.stretch.Map( line[ x - p_grayBuffer->bufferXYRect().left() ] ) + 0.5);
          int greenPix = (int)(p_green.stretch.Map( line[ x - p_grayBuffer->bufferXYRect().left() ] ) + 0.5);
          int bluePix = (int)(p_blue.stretch.Map( line[ x - p_grayBuffer->bufferXYRect().left() ] ) + 0.5);
          rgb[x] =  qRgb(redPix, greenPix, bluePix);
        }
      }
    }
    else if(p_redBuffer && p_redBuffer->enabled()) {
      dataArea = QRect( p_redBuffer->bufferXYRect().intersected( rect ) );

      for(int y = dataArea.top(); !dataArea.isNull() && y <= dataArea.bottom(); y++) {
        const std::vector<double> &redLine = p_redBuffer->getLine(y - p_redBuffer->bufferXYRect().top());
        const std::vector<double> &greenLine = p_greenBuffer->getLine(y - p_greenBuffer->bufferXYRect().top());
        const std::vector<double> &blueLine = p_blueBuffer->getLine(y - p_blueBuffer->bufferXYRect().top());

        QRgb *rgb = (QRgb *) p_image->scanLine(y);
  
        for(int x = dataArea.left(); x <= dataArea.right(); x++) {
          int redPix = (int)(p_red.stretch.Map( redLine[ x - p_redBuffer->bufferXYRect().left() ] ) + 0.5);
          int greenPix = (int)(p_green.stretch.Map( greenLine[ x - p_greenBuffer->bufferXYRect().left() ] ) + 0.5);
          int bluePix = (int)(p_blue.stretch.Map( blueLine[ x - p_blueBuffer->bufferXYRect().left() ] ) + 0.5);
          rgb[x] =  qRgb(redPix, greenPix, bluePix);
        }
      }
    }

    if(!dataArea.isNull()) {
      p.drawImage(dataArea.topLeft(), *p_image, dataArea);
    }
    
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
          return true;
        }

        case QEvent::MouseMove:
        {
          QMouseEvent *m = (QMouseEvent *) e;
          emit mouseMove(m->pos());
          return true;
        }

        case QEvent::Leave: {
          viewport()->setMouseTracking(false);
          emit mouseLeave();
          return true;
        }

        case QEvent::MouseButtonPress: {
          QMouseEvent *m = (QMouseEvent *) e;
          emit mouseButtonPress(m->pos(),
                               (Qt::MouseButton)(m->button()+m->modifiers()));
          return true;
        }

        case QEvent::MouseButtonRelease: {
          QMouseEvent *m = (QMouseEvent *) e;
          emit mouseButtonRelease(m->pos(),
                                  (Qt::MouseButton)(m->button()+m->modifiers()));
          return true;
        }

        case QEvent::MouseButtonDblClick: {
          QMouseEvent *m = (QMouseEvent *) e;
          emit mouseDoubleClick(m->pos());
          return true;
        }

        default: {
          return false;
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

    if(!p_grayBuffer) p_grayBuffer = new ViewportBuffer(this, p_cube);
    
    p_grayBuffer->setBand(band);

    if(p_redBuffer) delete p_redBuffer;
    p_redBuffer = NULL;

    if(p_greenBuffer) delete p_greenBuffer;
    p_greenBuffer = NULL;

    if(p_blueBuffer) delete p_blueBuffer;
    p_blueBuffer = NULL;

    for (int i=0; i < p_toolList.size(); i++) {
      p_toolList[i]->updateTool();
    }

    if(p_camera) {
      p_camera->SetBand(band);
    }
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

    if(!p_redBuffer) p_redBuffer = new ViewportBuffer(this, p_cube);
    p_redBuffer->setBand(rband);

    if(!p_greenBuffer) p_greenBuffer = new ViewportBuffer(this, p_cube);
    p_greenBuffer->setBand(gband);

    if(!p_blueBuffer) p_blueBuffer = new ViewportBuffer(this, p_cube);
    p_blueBuffer->setBand(bband);

    if(p_grayBuffer) delete p_grayBuffer;
    p_grayBuffer = NULL;

    for (int i=0; i < p_toolList.size(); i++) {
      p_toolList[i]->updateTool();
    }

    if(p_camera) {
      p_camera->SetBand(rband);
    }
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
    p_gray.stretch.Parse(gstr.toStdString());

    p_red.stretch.Parse(gstr.toStdString());
    p_green.stretch.Parse(gstr.toStdString());
    p_blue.stretch.Parse(gstr.toStdString());
    paintPixmap();
    viewport()->update();
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


  void CubeViewport::stretchGray (const Isis::Stretch &stretch) {
    p_gray.stretch = stretch;

    p_red.stretch.CopyPairs(stretch);
    p_green.stretch.CopyPairs(stretch);
    p_blue.stretch.CopyPairs(stretch);
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
    if(p_grayBuffer) p_grayBuffer->fillBuffer(vpRect);
    if(p_redBuffer) p_redBuffer->fillBuffer(vpRect);
    if(p_greenBuffer) p_greenBuffer->fillBuffer(vpRect);
    if(p_blueBuffer) p_blueBuffer->fillBuffer(vpRect);
    paintPixmap(vpRect);
    viewport()->repaint();
  }


  void CubeViewport::initialStretch() {
    std::vector< std::pair <ViewportBuffer *, Isis::Stretch *> > buffers;
    if(p_grayBuffer) {
      buffers.push_back(std::pair <ViewportBuffer *, Isis::Stretch *>(p_grayBuffer, &p_gray.stretch));
    }
    if(p_redBuffer) {
      buffers.push_back(std::pair <ViewportBuffer *, Isis::Stretch *>(p_redBuffer, &p_red.stretch));
    }
    if(p_greenBuffer) {
      buffers.push_back(std::pair <ViewportBuffer *, Isis::Stretch *>(p_greenBuffer, &p_green.stretch));
    }
    if(p_blueBuffer) {
      buffers.push_back(std::pair <ViewportBuffer *, Isis::Stretch *>(p_blueBuffer, &p_blue.stretch));
    }

    for(unsigned int i = 0; i < buffers.size(); i++) {
      ViewportBuffer *currBuffer = buffers[i].first;
      Isis::Stretch *currStretch = buffers[i].second;

      Isis::Statistics stats;
      for(int line = 0; line < currBuffer->bufferXYRect().height(); line++) {
        stats.AddData(&currBuffer->getLine(line).front(), currBuffer->getLine(line).size()); 
      }

    	if(fabs(stats.BestMinimum()) < DBL_MAX && fabs(stats.BestMaximum()) < DBL_MAX) {
        Isis::Histogram hist(stats.BestMinimum(),stats.BestMaximum());
        for(int line = 0; line < currBuffer->bufferXYRect().height(); line++) {
          hist.AddData(&currBuffer->getLine(line).front(), currBuffer->getLine(line).size()); 
        }
    
        currStretch->ClearPairs();
        double percentile1 = hist.Percent(0.5);
        double percentile2 = hist.Percent(99.5);
        if (fabs(percentile1 - percentile2) > DBL_EPSILON) {
          currStretch->AddPair(percentile1, 0.0);
          currStretch->AddPair(percentile2, 255.0);
        }
        else {
          currStretch->AddPair(-DBL_MAX, 0.0);
          currStretch->AddPair(DBL_MAX, 255.0);
        }
    	}
    	else {
    	  currStretch->ClearPairs();
        currStretch->AddPair(-DBL_MAX, 0.0);
        currStretch->AddPair(DBL_MAX, 255.0);
      }
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
  void CubeViewport::setStretchInfo(int band, bool stretchFlag, double min, double max){
      p_bandsStretchList[band-1]->p_stretchMin = min;
      p_bandsStretchList[band-1]->p_stretchMax = max;
      p_bandsStretchList[band-1]->p_stretched = stretchFlag;

      p_gray.stretch.ClearPairs();
      p_gray.stretch.AddPair(min,0.0);
      p_gray.stretch.AddPair(max,255.0);
   
  }*/

  /** 
   * 
   * 
   * @param band
   * 
   * @return double
   *
  double CubeViewport::getStretchMin(int band){
    return p_bandsStretchList[band-1]->p_stretchMin;
  }*/

  /** 
   * 
   * 
   * @param band
   * 
   * @return double
   *
  double CubeViewport::getStretchMax(int band){
    return p_bandsStretchList[band-1]->p_stretchMax;
  }*/

  /** 
   * 
   * 
   * @param band
   * 
   * @return bool
   *
  bool CubeViewport::getStretchFlag(int band){
    return p_bandsStretchList[band-1]->p_stretched;
  }*/


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
