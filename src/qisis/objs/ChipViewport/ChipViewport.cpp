#include "ChipViewport.h"

#include <QPainter>

namespace Qisis {

  //!  Construct an empty viewport
  ChipViewport::ChipViewport (int width, int height, QWidget *parent) : 
                QWidget(parent) {

    setFixedSize(width,height);
    setBackgroundRole(QPalette::Dark);
//    setFocusPolicy(Qt::StrongFocus);

    p_width = width;
    p_height = height;
    p_zoomFactor = 1.0;
    p_rotation = 0;
    p_geomIt = false;
    p_tempView = NULL;
    p_cross = true;
    p_circle = false;

  }


  /**
   * Destructor
   * 
   */
  ChipViewport::~ChipViewport() {

  }


  void ChipViewport::setChip (Isis::Chip *chip) {
    // Is the chip usable?
    if (chip == NULL) {
      throw Isis::iException::Message(Isis::iException::Programmer,
                                      "Can not view NULL chip pointer",
                                      _FILEINFO_);
    }

    p_zoomFactor = 1.0;
    p_rotation = 0;

    p_chip = chip;
    p_image = new QImage(chip->Samples(),chip->Lines(),QImage::Format_RGB32);

    autoStretch();

    emit tackPointChanged(p_zoomFactor);
  }


  //! Apply automatic stretch using data from entire chip
  void ChipViewport::autoStretch () {
    computeStretch(p_gray.stretch);
    paintImage();
    update();
  }


  //! Compute automatic stretch for a portion of the cube
  void ChipViewport::computeStretch(Isis::Stretch &stretch) {
    Isis::Statistics stats;
    for (int line=1; line<p_chip->Lines(); line++) {
      for (int samp=1; samp<p_chip->Samples(); samp++) {
        double value = (*p_chip)(samp,line);
        stats.AddData(&value,1);
      }
    }

    Isis::Histogram hist(stats.BestMinimum(),stats.BestMaximum());
    for (int line=1; line<=p_chip->Lines(); line++) {
      for (int samp=1; samp<=p_chip->Samples(); samp++) {
        double value = (*p_chip)(samp,line);
        hist.AddData(&value,1);
      }
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


  //! Paint QImage
  void ChipViewport::paintImage () {
    //  TODO: ??? Need something similar to CubeViewport clipping, so that
    //         at edge of image, fill viewport w/ black
    for (int y=0; y<p_chip->Lines(); y++) {
      QRgb *rgb = (QRgb *) p_image->scanLine(y);
      int r,g,b;
      for (int x=0; x<p_chip->Samples(); x++) {
        r = g = b = (int) p_gray.stretch.Map((*p_chip)(x+1,y+1));
        rgb[x] =  qRgb(r,g,b);
      }
    }
    repaint();

  }


  //! Repaint the viewport
  void ChipViewport::paintEvent (QPaintEvent *e) {
    QPainter painter(this);

    if (p_tempView != NULL) {
      painter.drawImage(0,0,*(p_tempView->p_image));
    }
    else {
      painter.drawImage(0,0,*p_image);
    }

    if (p_cross == true) {
      painter.setPen(Qt::red);
      painter.drawLine(0,(p_height-1)/2,p_width-1,(p_height-1)/2);
      painter.drawLine((p_width-1)/2,0,(p_width-1)/2,p_height-1);
    }

    if (p_circle == true) {
      painter.setPen(Qt::red);
      painter.drawEllipse((p_height-1)/2-p_circleSize/2,(p_width-1)/2-p_circleSize/2,
                           p_circleSize,p_circleSize);
    }
    p_tempView = NULL;
    //painter.end();

  }


  //!  Load with info from given ChipViewport
  void ChipViewport::loadView(ChipViewport &newView) {
    p_tempView = &newView;
    update ();
  }


  /**
   * Returns tack sample
   * 
   * 
   * @return double 
   */
  double ChipViewport::tackSample () {
    p_chip->SetChipPosition(p_chip->TackSample(),p_chip->TackLine());
    return p_chip->CubeSample();
  }


  /**
   * returns tack Line
   * 
   * 
   * @return double 
   */
  double ChipViewport::tackLine () {
    p_chip->SetChipPosition(p_chip->TackSample(),p_chip->TackLine());
    return p_chip->CubeLine();
  }


  //!<  Pan up
  void ChipViewport::panUp() {
    int x,y;
    x = p_chip->TackSample();
    y = p_chip->TackLine() - 1;
    //  Reload with new cube position
    p_chip->SetChipPosition((double)x,(double)y);
    reloadChip(p_chip->CubeSample(),p_chip->CubeLine());
  }


  //!<  Pan down
  void ChipViewport::panDown() {
    int x,y;
    x = p_chip->TackSample();
    y = p_chip->TackLine() + 1;
    //  Reload with new cube position
    p_chip->SetChipPosition((double)x,(double)y);
    reloadChip(p_chip->CubeSample(),p_chip->CubeLine());
  }


  //!<  Pan left
  void ChipViewport::panLeft() {
    int x,y;
    x = p_chip->TackSample() - 1;
    y = p_chip->TackLine();
    //  Reload with new cube position
    p_chip->SetChipPosition((double)x,(double)y);
    reloadChip(p_chip->CubeSample(),p_chip->CubeLine());
  }


  //!<  Pan right
  void ChipViewport::panRight() {
    int x,y;
    x = p_chip->TackSample() + 1;
    y = p_chip->TackLine();
    //  Reload with new cube position
    p_chip->SetChipPosition((double)x,(double)y);
    reloadChip(p_chip->CubeSample(),p_chip->CubeLine());
  }


  /**
   * Zoom in
   * 
   */
  void ChipViewport::zoomIn() {

    p_zoomFactor /= 2.0;
    reloadChip();
  }


  /**
   * Zoom out
   * 
   */
  void ChipViewport::zoomOut() {

    p_zoomFactor *= 2.0;
    reloadChip();
  }


  /**
   * Zoom by a factor of one.
   * 
   */
  void ChipViewport::zoom1() {

    p_zoomFactor = 1.0;
    reloadChip();
  }


  //!<  Slot to refresh viewport , point has changed
  void ChipViewport::refreshView(double tackSample,double tackLine) {

    reloadChip(tackSample,tackLine);

  }


  //!<  If mouse enters, make sure key events are processed w/o clicking
  void ChipViewport::enterEvent(QEvent *e) {
    setFocus();
  }


  //!< Process arrow keystrokes on cube
  void ChipViewport::keyPressEvent(QKeyEvent *e) {

    int x,y;
    x = p_chip->TackSample();
    y = p_chip->TackLine();
    if (e->key() == Qt::Key_Up) {
      y--;
    }
    else if (e->key() == Qt::Key_Down) {
      y++;
    }
    else if (e->key() == Qt::Key_Left) {
      x--;
    }
    else if (e->key() == Qt::Key_Right) {
      x++;
    }
    else {
      QWidget::keyPressEvent(e);
      return;
    }

    //  Reload with new cube position
    p_chip->SetChipPosition((double)x,(double)y);
    reloadChip(p_chip->CubeSample(),p_chip->CubeLine());

  }



  //!<  Process mouse events
  void ChipViewport::mousePressEvent(QMouseEvent *event) {
    
    QPoint p = event->pos();
    if (event->button() == Qt::LeftButton) {
      //  Reload with new cube position
      p_chip->SetChipPosition((double)p.x(),(double)p.y());
      reloadChip(p_chip->CubeSample(),p_chip->CubeLine());
    }
  }


  //!<  Slot to change state of crosshair
  void ChipViewport::setCross(bool checked) {

    if (checked == p_cross) return;

    p_cross = checked;
    repaint();
      
  }


  void ChipViewport::setCircle(bool checked) {

    if (checked == p_circle) return;

    p_circle = checked;
    repaint();
  }


  void ChipViewport::setCircleSize(int size) {

    p_circleSize = size;
    repaint();

  }



  //!<  Slot to geom chip
  void ChipViewport::geomChip (Isis::Chip *matchChip) {

    p_geomIt = true;
    p_matchChip = matchChip;
    p_chip->ReLoad(*matchChip);
//    p_chip->ReLoad(*matchChip,p_zoomFactor);

     //  TODO:   ??? Can these be added to paintEvent method ???
    autoStretch();
  }


  //!<  Slot to geom chip
  void ChipViewport::nogeomChip () {

    p_geomIt = false;
    p_chip->ReLoad(p_rotation,p_zoomFactor);

    //  TODO:   ??? Can these be added to paintEvent method ???
    autoStretch();
  }


  /**
   * Slot to rotate chip
   * 
   * 
   * 
   * @param rotation 
   */
  void ChipViewport::rotateChip(int rotation) {

    p_rotation = -rotation;
    p_chip->ReLoad(-rotation,p_zoomFactor);

    //  TODO:   ??? Can these be added to paintEvent method ???
    autoStretch();
  }


  /**
   * Reloads the chip.
   * 
   * 
   * @param tackSample 
   * @param tackLine 
   */
  void ChipViewport::reloadChip(double tackSample,double tackLine) {

    // Is the chip usable?
    if (p_chip == NULL) {
      throw Isis::iException::Message(Isis::iException::Programmer,
                                      "Can not view NULL chip pointer",
                                      _FILEINFO_);
    }

    if (tackSample != 0. && tackLine != 0.) 
      p_chip->TackCube(tackSample,tackLine);
    if (p_geomIt) {
      if (p_matchChip == NULL) {
        throw Isis::iException::Message(Isis::iException::User,
                                        "Invalid match chip",_FILEINFO_);
      }
      p_chip->ReLoad(*p_matchChip);
//      p_chip->ReLoad(*p_matchChip,p_zoomFactor);
    }
    else {
      p_chip->ReLoad(p_rotation,p_zoomFactor);
    }


    //  TODO:   ??? Can these be added to paintEvent method ???
    autoStretch();

    //  emit signal indicating tack point changed so that the TieTool
    //  can update the sample/line label.
    emit tackPointChanged(p_zoomFactor);
  }

}
