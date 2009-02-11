#include <QStatusBar>
#include <QLabel>
#include <QCursor>

#include "SpecialPixel.h"
#include "TrackTool.h"

namespace Qisis {
  /**
   * TrackTool constructor
   * 
   * 
   * @param parent 
   */
  TrackTool::TrackTool (QStatusBar *parent) : Tool(parent) {
    p_sbar = parent;

    p_sampLabel = new QLabel(p_sbar);
    p_sampLabel->setText("W 999999");
    p_sampLabel->setMinimumSize(p_sampLabel->sizeHint());
    p_sampLabel->setToolTip("Sample Position");
    p_sbar->addPermanentWidget(p_sampLabel);

    p_lineLabel = new QLabel(p_sbar);
    p_lineLabel->setText("W 999999");
    p_lineLabel->setMinimumSize(p_lineLabel->sizeHint());
    p_lineLabel->setToolTip("Line Position");
    p_sbar->addPermanentWidget(p_lineLabel);

    p_latLabel = new QLabel(p_sbar);
    p_latLabel->setText("9.999999E-99");
    p_latLabel->setMinimumSize(p_latLabel->sizeHint());
    p_latLabel->hide();
    p_latLabel->setToolTip("Latitude Position");
    p_sbar->addPermanentWidget(p_latLabel);

    p_lonLabel = new QLabel(p_sbar);
    p_lonLabel->setText("9.999999E-99");
    p_lonLabel->setMinimumSize(p_lonLabel->sizeHint());
    p_lonLabel->hide();
    p_lonLabel->setToolTip("Longitude Position");
    p_sbar->addPermanentWidget(p_lonLabel);

    p_grayLabel = new QLabel(p_sbar);
    p_grayLabel->setText("9.999999E-99");
    p_grayLabel->setMinimumSize(p_grayLabel->sizeHint());
    p_grayLabel->setToolTip("Gray Pixel Value");
    p_sbar->addPermanentWidget(p_grayLabel);

    p_redLabel = new QLabel(p_sbar);
    p_redLabel->setText("W 9.999999E-99");
    p_redLabel->setMinimumSize(p_redLabel->sizeHint());
    p_redLabel->hide();
    p_redLabel->setToolTip("Red Pixel Value");
    p_sbar->addPermanentWidget(p_redLabel);

    p_grnLabel = new QLabel(p_sbar);
    p_grnLabel->setText("W 9.999999E-99");
    p_grnLabel->setMinimumSize(p_grnLabel->sizeHint());
    p_grnLabel->hide();
    p_grnLabel->setToolTip("Green Pixel Value");
    p_sbar->addPermanentWidget(p_grnLabel);

    p_bluLabel = new QLabel(p_sbar);
    p_bluLabel->setText("W 9.999999E-99");
    p_bluLabel->setMinimumSize(p_bluLabel->sizeHint());
    p_bluLabel->hide();
    p_bluLabel->setToolTip("Blue Pixel Value");
    p_sbar->addPermanentWidget(p_bluLabel);

    clearLabels();

    activate(true);
  }


  /**
   * Updates the labels anytime the mouse moves.
   * 
   * 
   * @param p 
   */
  void TrackTool::mouseMove(QPoint p) {
    updateLabels(p);
  }


  /**
   * Clears the labels if the mouse leaves the application.
   * 
   */
  void TrackTool::mouseLeave() {
    clearLabels();
  }


  /**
   * Updates the tracking labels.
   * 
   * 
   * @param p 
   */
  void TrackTool::updateLabels (QPoint p) {
    CubeViewport *cvp = cubeViewport();
    if (cvp == NULL) {
      clearLabels();
      return;
    }

    double sample,line;
    cvp->viewportToCube(p.x(),p.y(),sample,line);
    if ((sample < 0.5) || (line < 0.5) ||
        (sample > cvp->cubeSamples()+0.5) ||
        (line > cvp->cubeLines()+0.5)) {
      clearLabels();
      return;
    }

    int isamp = (int) (sample + 0.5);
    QString text;
    text.setNum(isamp);
    text = "S " + text;
    p_sampLabel->setText(text);

    int iline = (int) (line + 0.5);
    text.setNum(iline);
    text = "L " + text;
    p_lineLabel->setText(text);


    // Do we have a projection?
    if (cvp->projection() != NULL) {
      p_latLabel->show();
      p_lonLabel->show();

      if (cvp->projection()->SetWorld(sample,line)) {
        double lat = cvp->projection()->Latitude();
        double lon = cvp->projection()->Longitude();
        p_latLabel->setText(QString("Lat %1").arg(lat));
        p_lonLabel->setText(QString("Lon %1").arg(lon));
      }
      else {
        p_latLabel->setText("Lat n/a");
        p_lonLabel->setText("Lon n/a");
      }
    }
    // Do we have a camera model?
    else if (cvp->camera() != NULL) {
      p_latLabel->show();
      p_lonLabel->show();

      if (cvp->camera()->SetImage(sample,line)) {
        double lat = cvp->camera()->UniversalLatitude();
        double lon = cvp->camera()->UniversalLongitude();
        p_latLabel->setText(QString("Lat %1").arg(lat));
        p_lonLabel->setText(QString("Lon %1").arg(lon));
      }
      else {
        p_latLabel->setText("Lat n/a");
        p_lonLabel->setText("Lon n/a");
      }
    }

    else {
      p_latLabel->hide();
      p_lonLabel->hide();
    }

    if (cvp->isGray()) {
      p_grayLabel->show();
      p_redLabel->hide();
      p_grnLabel->hide();
      p_bluLabel->hide();

      std::string pixelString = Isis::PixelToString(cvp->grayPixel(isamp,iline));
      p_grayLabel->setText(pixelString.c_str());
    }
    else {
      p_grayLabel->hide();
      p_redLabel->show();
      p_grnLabel->show();
      p_bluLabel->show();

      QString rlab = "R ";
      QString glab = "G ";
      QString blab = "B ";
      std::string redLabel = Isis::PixelToString(cvp->redPixel(isamp,iline));
      std::string greenLabel = Isis::PixelToString(cvp->greenPixel(isamp,iline));
      std::string blueLabel = Isis::PixelToString(cvp->bluePixel(isamp,iline));
      p_redLabel->setText(rlab+redLabel.c_str());
      p_grnLabel->setText(glab+greenLabel.c_str());
      p_bluLabel->setText(blab+blueLabel.c_str());
    }
  }


  /**
   * Clears the labels.
   * 
   */
  void TrackTool::clearLabels () {
    p_sampLabel->setText("S n/a");
    p_lineLabel->setText("L n/a");
    p_latLabel->setText("Lat n/a");
    p_lonLabel->setText("Lon n/a");
    p_grayLabel->setText("n/a");
    p_redLabel->setText("R n/a");
    p_grnLabel->setText("G n/a");
    p_bluLabel->setText("B n/a");
  }


  /**
   * Finds the cursor position.
   * 
   */
  void TrackTool::locateCursor () {
    if (cubeViewport() == NULL) return;
    QPoint p = cubeViewport()->viewport()->mapFromGlobal(QCursor::pos());
    if (p.x() < 0) return;
    if (p.y() < 0) return;
    if (p.x() >= cubeViewport()->viewport()->width()) return;
    if (p.y() >= cubeViewport()->viewport()->height()) return;
    updateLabels(p);
  }


  /**
   * Adds the connections to the given viewport.
   * 
   * 
   * @param cvp 
   */
  void TrackTool::addConnections (CubeViewport *cvp) {
    connect(cubeViewport(),SIGNAL(viewportUpdated()),
            this,SLOT(locateCursor()));
  }


  /**
   * Removes the connections from the given viewport.
   * 
   * 
   * @param cvp 
   */
  void TrackTool::removeConnections (CubeViewport *cvp) {
    disconnect(cubeViewport(),SIGNAL(viewportUpdated()),
               this,SLOT(locateCursor()));
  }
}

