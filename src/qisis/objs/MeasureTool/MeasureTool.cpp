#include <QMenuBar>
#include <QMenu>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QLineEdit>
#include "MeasureTool.h"
#include "Camera.h"
#include "CubeViewport.h"
#include "Filename.h"

namespace Qisis {
  /**
   * MeasureTool constructor
   * 
   * 
   * @param parent 
   */
  MeasureTool::MeasureTool (QWidget *parent) : Qisis::Tool(parent) {
    p_rubberBand = NULL;
    p_tableWin = new TableMainWindow ("Measurements", parent);
    p_tableWin->setTrackListItems(true);
    
    // Create the action for showing the table
    p_action = new QAction(parent);
    p_action->setText("Measuring ...");

    connect(p_action,SIGNAL(triggered()),p_tableWin,SLOT(showTable()));
    connect(p_action,SIGNAL(triggered()),p_tableWin,SLOT(raise()));
    connect(p_action,SIGNAL(triggered()),p_tableWin,SLOT(syncColumns()));
    p_tableWin->installEventFilter(this);

    p_tableWin->addToTable (false,"Feature\nName","Feature Name");
    p_tableWin->addToTable (false,"Feature\nType","Feature Type");
    p_tableWin->addToTable (true,
                "Start\nLatitude:Start\nLongitude:End\nLatitude:End\nLongitude",
                "Ground Range");
    p_tableWin->addToTable (false,"Start\nSample:Start\nLine:End\nSample:End\nLine",
                "Pixel Range");
    p_tableWin->addToTable(true,"Kilometer\nDistance","Kilometer Distance");
    p_tableWin->addToTable (false,"Meter\nDistance","Meter Distance");
    p_tableWin->addToTable (false,"Pixel\nDistance","Pixel Distance");
    p_tableWin->addToTable (false,"Degree\nAngle","Degree Angle");
    p_tableWin->addToTable (false,"Radian\nAngle","Radian Angle");
    p_tableWin->addToTable(false,"Kilometer\nArea","Kilometer Area");
    p_tableWin->addToTable (false,"Meter\nArea","Meter Area");
    p_tableWin->addToTable (false,"Pixel\nArea","Pixel Area");
    p_tableWin->addToTable (false,"Path","Path");
    p_tableWin->addToTable (false,"Filename","Filename");
    p_tableWin->addToTable (false,"Notes","Notes");

    // Setup 10 blank rows in the table
    for (int r=0; r<4; r++) {
      p_tableWin->table()->insertRow(r);
      for (int c=0; c<p_tableWin->table()->columnCount(); c++) {
        QTableWidgetItem *item = new QTableWidgetItem("");
        p_tableWin->table()->setItem(r,c,item);
      }
    }

    p_tableWin->setStatusMessage("Click, Drag, and Release to Measure a Line");
  }


  /**
   * Add the measure tool action to the toolpad.
   * 
   * 
   * @param toolpad 
   * 
   * @return QAction* 
   */
  QAction *MeasureTool::toolPadAction(ToolPad *toolpad) {
    QAction *action = new QAction(toolpad);
    action->setIcon(QPixmap(toolIconDir()+"/measure.png"));
    action->setToolTip("Measure (M)");
    action->setShortcut(Qt::Key_M);

    QString text  =
      "<b>Function:</b>  Measure features in active viewport \
      <p><b>Shortcut:</b> M</p> ";
    action->setWhatsThis(text);

    return action;
  }


  /**
   * Creates the widget (button) that goes on the tool bar
   * 
   * 
   * @param parent 
   * 
   * @return QWidget* 
   */
  QWidget *MeasureTool::createToolBarWidget (QStackedWidget *parent) {
    QWidget *hbox = new QWidget(parent);
    QToolButton *measureButton = new QToolButton(hbox);
    measureButton->setText("Table");
    measureButton->setToolTip("Record Measurement Data in Table");
    QString text =
    "<b>Function:</b> This button will bring up a table that will record the \
     starting and ending points of the line, along with the distance between \
     the two points on the image.  To measure the distance between two points, \
      click on the first point and releasing the mouse at the second point. \
      <p><b>Shortcut:</b>  CTRL+M</p>";
    measureButton->setWhatsThis(text);
    measureButton->setShortcut(Qt::CTRL+Qt::Key_M);
    connect(measureButton,SIGNAL(clicked()),p_tableWin,SLOT(showTable()));
    connect(measureButton,SIGNAL(clicked()),p_tableWin,SLOT(syncColumns()));
    connect(measureButton,SIGNAL(clicked()),p_tableWin,SLOT(raise()));
    measureButton->setEnabled(true);

    p_rubberBand = new RubberBandComboBox(
      RubberBandComboBox::Angle | 
      RubberBandComboBox::Circle |
      RubberBandComboBox::Ellipse |
      RubberBandComboBox::Line |
      RubberBandComboBox::Rectangle |
      RubberBandComboBox::RotatedRectangle |
      RubberBandComboBox::Polygon, // options
      RubberBandComboBox::Line // default
      );

    p_distLineEdit = new QLineEdit(hbox);
    p_distLineEdit->setText("");
    p_distLineEdit->setMaxLength(12);
    p_distLineEdit->setToolTip("Line Length");
    QString text2 = "<b>Function: </b> Shows the length of the line drawn on \
                     the image.";
    p_distLineEdit->setWhatsThis(text2);
    p_distLineEdit->setReadOnly(true);

    p_unitsComboBox = new QComboBox(hbox);
    p_unitsComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    updateUnitsCombo();
    connect(p_unitsComboBox,SIGNAL(activated(int)),this,SLOT(updateDistEdit()));
    connect(RubberBandTool::getInstance(),SIGNAL(modeChanged()),this,SLOT(updateUnitsCombo()));

    QHBoxLayout *layout = new QHBoxLayout(hbox);
    layout->setMargin(0);
    layout->addWidget(p_rubberBand);
    layout->addWidget(p_distLineEdit);
    layout->addWidget(p_unitsComboBox);
    layout->addWidget(measureButton);
    layout->addStretch(1);
    hbox->setLayout(layout);
    return hbox;
  }


  /**
   * Updates the units combo box.
   * 
   */
  void MeasureTool::updateUnitsCombo() {
    p_unitsComboBox->clear();
    
    if(RubberBandTool::getMode() == RubberBandTool::Line) {
      p_unitsComboBox->addItem("km");
      p_unitsComboBox->addItem("m");
      p_unitsComboBox->addItem("pixels");
      p_unitsComboBox->setCurrentIndex(2);
    }
    else if(RubberBandTool::getMode() == RubberBandTool::Angle) {
      p_unitsComboBox->addItem("degrees");
      p_unitsComboBox->addItem("radians");
      p_unitsComboBox->setCurrentIndex(0);
    }
    else {
      p_unitsComboBox->addItem("km^2");
      p_unitsComboBox->addItem("m^2");
      p_unitsComboBox->addItem("pix^2");
      p_unitsComboBox->setCurrentIndex(2);
    }
  }


  /**
   * Adds the measure action to the given menu.
   * 
   * 
   * @param menu 
   */
  void MeasureTool::addTo(QMenu *menu) {
    menu->addAction(p_action);
  }


  /**
   * Updates the Measure specifications.
   * 
   */
  void MeasureTool::updateMeasure() {
    CubeViewport *cvp = cubeViewport();
    CubeViewport *d;
    p_numLinked = 0;

    if (cvp == NULL) {
      p_tableWin->clearRow(p_tableWin->currentRow());
      return;
    }

    updateDist(cvp, p_tableWin->currentRow());

    p_tableWin->table()->selectRow(p_tableWin->currentRow());

    if(cvp->isLinked()) {
      for (int i=0; i<(int)cubeViewportList()->size(); i++) {
        d = (*(cubeViewportList()))[i];
  
        if (d->isLinked() && d != cvp) {
          p_numLinked++;
          updateDist(d, p_tableWin->currentRow()+p_numLinked);       
        }
      }
    }
  }


  /**
   * Called when the rubberBanding by the user is finished.
   * 
   */
  void MeasureTool::rubberBandComplete() {
    updateMeasure();

    if (RubberBandTool::getMode() != RubberBandTool::Angle && p_unitsComboBox->currentIndex() != 2) {
      if (p_cvp->camera() == NULL && p_cvp->projection() == NULL) {
        QMessageBox::information((QWidget *)parent(), "Error",
          "File must have a Camera Model or Projection to measure in km or m");
        return;
      }
    }
    
    if (!p_tableWin->table()->isVisible()) return;
    if (p_tableWin->table()->item(p_tableWin->currentRow(),StartLineIndex)->text() == "N/A" &&
        p_tableWin->table()->item(p_tableWin->currentRow(),AngleDegIndex)->text() == "N/A" &&
        p_tableWin->table()->item(p_tableWin->currentRow(),AreaPixIndex)->text() == "N/A") return;

    if(!p_cvp->isLinked())p_tableWin->setCurrentRow(p_tableWin->currentRow() + 1);
    if(p_cvp->isLinked()){
       p_tableWin->setCurrentRow(p_tableWin->currentRow() + p_numLinked + 1);
    }
    p_tableWin->setCurrentIndex(p_tableWin->currentIndex() + 1);
    while (p_tableWin->currentRow() >= p_tableWin->table()->rowCount()) {
      int row = p_tableWin->table()->rowCount();
      p_tableWin->table()->insertRow(row);
      for (int c=0; c<p_tableWin->table()->columnCount(); c++) {
        QTableWidgetItem *item = new QTableWidgetItem("");
        p_tableWin->table()->setItem(row,c,item);
      }
    }

    QApplication::sendPostedEvents(p_tableWin->table(),0);
    p_tableWin->table()->scrollToItem(p_tableWin->table()->item(p_tableWin->currentRow(),0),QAbstractItemView::PositionAtBottom);
  }


  /**
   * Mouse leave event.
   * 
   */
  void MeasureTool::mouseLeave() {
    //p_tableWin->clearRow(p_tableWin->currentRow());
  }


  /**
   * Enables/resets the rubberband tool.
   * 
   */
  void MeasureTool::enableRubberBandTool() {
    if(p_rubberBand) {
      p_rubberBand->reset();
    }
  }


  /**
   * This method updates the row in the table window with the 
   * current measure information. 
   * 
   * 
   * @param row 
   */
  void MeasureTool::updateRow(int row) {

    if (row+1 > p_tableWin->table()->rowCount()) {
      p_tableWin->table()->insertRow(row);
      for (int c=0; c<p_tableWin->table()->columnCount(); c++) {
        QTableWidgetItem *item = new QTableWidgetItem("");
        p_tableWin->table()->setItem(row,c,item);
        if (c == 0) p_tableWin->table()->scrollToItem(item);
      }
        
    }
      
    // Blank out the row to remove stuff left over from previous cvps
    for (int c=0; c<p_tableWin->table()->columnCount(); c++) {
      p_tableWin->table()->item(row,c)->setText("");
    }

    // Write all the new info to the current row
    if (p_startLat != Isis::Null && p_startLon != Isis::Null) {
      p_tableWin->table()->item(row,StartLatIndex)->setText(QString::number(p_startLat));
      p_tableWin->table()->item(row,StartLonIndex)->setText(QString::number(p_startLon));
    }
    else {
      p_tableWin->table()->item(row,StartLatIndex)->setText("N/A");
      p_tableWin->table()->item(row,StartLonIndex)->setText("N/A");
    }

    if (p_endLat != Isis::Null && p_endLon != Isis::Null) {
      p_tableWin->table()->item(row,EndLatIndex)->setText(QString::number(p_endLat));
      p_tableWin->table()->item(row,EndLonIndex)->setText(QString::number(p_endLon));
      p_tableWin->table()->item(row,DistanceKmIndex)->setText(QString::number(p_kmDist));
      p_tableWin->table()->item(row,DistanceMIndex)->setText(QString::number(p_mDist));
    }
    else {
      p_tableWin->table()->item(row,EndLatIndex)->setText("N/A");
      p_tableWin->table()->item(row,EndLonIndex)->setText("N/A");
      p_tableWin->table()->item(row,DistanceKmIndex)->setText("N/A");
      p_tableWin->table()->item(row,DistanceMIndex)->setText("N/A");
    }

    if (p_degAngle != Isis::Null && p_radAngle != Isis::Null) {
      p_tableWin->table()->item(row,AngleDegIndex)->setText(QString::number(p_degAngle));
      p_tableWin->table()->item(row,AngleRadIndex)->setText(QString::number(p_radAngle));
    }
    else {
      p_tableWin->table()->item(row,AngleDegIndex)->setText("N/A");
      p_tableWin->table()->item(row,AngleRadIndex)->setText("N/A");
    }

    if (p_startSamp != Isis::Null && p_startLine != Isis::Null) {
      p_tableWin->table()->item(row,StartSampIndex)->setText(QString::number(p_startSamp));
      p_tableWin->table()->item(row,StartLineIndex)->setText(QString::number(p_startLine)); 
    }
    else {
      p_tableWin->table()->item(row,StartSampIndex)->setText("N/A");
      p_tableWin->table()->item(row,StartLineIndex)->setText("N/A"); 
    }

    if (p_endSamp != Isis::Null && p_endLine != Isis::Null) {
      p_tableWin->table()->item(row,EndSampIndex)->setText(QString::number(p_endSamp));
      p_tableWin->table()->item(row,EndLineIndex)->setText(QString::number(p_endLine)); 
      p_tableWin->table()->item(row,DistancePixIndex)->setText(QString::number(p_pixDist));
    }
    else {
      p_tableWin->table()->item(row,EndSampIndex)->setText("N/A");
      p_tableWin->table()->item(row,EndLineIndex)->setText("N/A"); 
      p_tableWin->table()->item(row,DistancePixIndex)->setText("N/A");
    }

    if (p_pixArea != Isis::Null) {
      p_tableWin->table()->item(row,AreaPixIndex)->setText(QString::number(p_pixArea));
    }
    else {
      p_tableWin->table()->item(row,AreaPixIndex)->setText("N/A");
    }

    if (p_mArea != Isis::Null) {
      p_tableWin->table()->item(row,AreaKmIndex)->setText(QString::number(p_kmArea));
      p_tableWin->table()->item(row,AreaMIndex)->setText(QString::number(p_mArea));
    }
    else {
      p_tableWin->table()->item(row,AreaKmIndex)->setText("N/A");
      p_tableWin->table()->item(row,AreaMIndex)->setText("N/A");
    }

    p_tableWin->table()->item(row,PathIndex)->setText(p_path.c_str());
    p_tableWin->table()->item(row,FilenameIndex)->setText(p_fname.c_str());

  }


  /**
   * This method updates the distance variables.
   * 
   * 
   * @param cvp 
   * @param row 
   */
  void MeasureTool::updateDist (CubeViewport *cvp, int row) {
   
    p_startSamp = Isis::Null;
    p_endSamp = Isis::Null;
    p_startLine = Isis::Null;
    p_endLine = Isis::Null;
    p_kmDist = Isis::Null;
    p_mDist =  Isis::Null;
    p_pixDist = Isis::Null;
    p_startLon = Isis::Null;
    p_startLat = Isis::Null;
    p_endLon = Isis::Null;
    p_endLat = Isis::Null;
    p_radAngle = Isis::Null;
    p_degAngle = Isis::Null;
    p_pixArea = Isis::Null;
    p_kmArea = Isis::Null;
    p_mArea = Isis::Null;

    // Write out col 8 (the file name)
    Isis::Filename fname = Isis::Filename(cvp->cube()->Filename()).Expanded();
    p_path = fname.Path();
    p_fname = fname.Name();

    if(RubberBandTool::getMode() == RubberBandTool::Line) {
      QPoint start = RubberBandTool::getVertices()[0];
      QPoint end   = RubberBandTool::getVertices()[1];
  
      cvp->viewportToCube(start.x(),start.y(),p_startSamp,p_startLine);
      if (cvp->camera() != NULL) {
        if (cvp->camera()->SetImage(p_startSamp,p_startLine)) {
          // Write columns 2-3 (Start lat/lon)
          p_startLat = cvp->camera()->UniversalLatitude();
          p_startLon = cvp->camera()->UniversalLongitude();
        }
      }
  
      if (cvp->projection() != NULL) {
        if (cvp->projection()->SetWorld(p_startSamp,p_startLine)) {
          // If our projection is sky, the lat & lons are switched
          if (cvp->projection()->IsSky()) {
            p_startLat = cvp->projection()->UniversalLongitude();
            p_startLon = cvp->projection()->UniversalLatitude();
          }
          else {
            p_startLat = cvp->projection()->UniversalLatitude();
            p_startLon = cvp->projection()->UniversalLongitude();
          }
        }
      }
  
      //  Convert rubber band line to cube coordinates
      cvp->viewportToCube(start.x(),start.y(),
                          p_startSamp,p_startLine);
      cvp->viewportToCube(end.x(), end.y(),
                          p_endSamp,p_endLine);
  
      // Don't write anything if we are outside the cube
      if ((p_startSamp < 0.5) || (p_endSamp < 0.5)) return;
      if ((p_startLine < 0.5) || (p_endLine < 0.5)) return;
      if ((p_startSamp > cvp->cubeSamples() + 0.5) || 
          (p_endSamp > cvp->cubeSamples() + 0.5)) return;
      if ((p_startLine > cvp->cubeLines() + 0.5) || 
          (p_endLine > cvp->cubeLines() + 0.5))return;
  
      // Calculate the pixel difference
      double lineDif = p_startLine - p_endLine;
      double sampDif = p_startSamp - p_endSamp;
      p_pixDist =  sqrt(lineDif * lineDif + sampDif * sampDif);
  
      // Do we have a camera model?
      if (cvp->camera() != NULL) {
        if (cvp->camera()->SetImage(p_startSamp,p_startLine)) {
          // Write columns 2-3 (Start lat/lon)
          p_startLat = cvp->camera()->UniversalLatitude();
          p_startLon = cvp->camera()->UniversalLongitude();
          if (cvp->camera()->SetImage(p_endSamp,p_endLine)) {
            // Write columns 4-5 (End lat/lon)
            p_endLat = cvp->camera()->UniversalLatitude();
            p_endLon = cvp->camera()->UniversalLongitude();
  
            // Calculate and write out the distance between the two points
            double radius = cvp->camera()->LocalRadius();
            p_mDist = Isis::Camera::Distance(p_startLat,p_startLon,p_endLat,
                                             p_endLon,radius);
            p_kmDist = p_mDist / 1000.0;
          }
        }
      }
  
      if (cvp->projection() != NULL) {
        if (cvp->projection()->SetWorld(p_startSamp,p_startLine)) {
          // If our projection is sky, the lat & lons are switched
          if (cvp->projection()->IsSky()) {
            p_startLat = cvp->projection()->UniversalLongitude();
            p_startLon = cvp->projection()->UniversalLatitude();
          }
          else {
            p_startLat = cvp->projection()->UniversalLatitude();
            p_startLon = cvp->projection()->UniversalLongitude();
          }
  
          if (cvp->projection()->SetWorld(p_endSamp,p_endLine)) {
            // If our projection is sky, the lat & lons are switched
            if (cvp->projection()->IsSky()) {
              p_endLat = cvp->projection()->UniversalLongitude();
              p_endLon = cvp->projection()->UniversalLatitude();
            }
            else {
              p_endLat = cvp->projection()->UniversalLatitude();
              p_endLon = cvp->projection()->UniversalLongitude();
            }
  
            // Calculate and write out the distance
            double radius = cvp->projection()->LocalRadius();
            p_mDist = Isis::Camera::Distance(p_startLat,p_startLon,p_endLat,
                                             p_endLon,radius);
            p_kmDist = p_mDist / 1000.0;  
          }
        }
      }
    }
    else if(RubberBandTool::getMode() == RubberBandTool::Angle) {
      p_radAngle = RubberBandTool::getAngle();
      p_degAngle = p_radAngle * 180.0 / Isis::PI;
    }
    else {
      geos::geom::Geometry *polygon = RubberBandTool::geometry();
      if(polygon != NULL) {
        // pix area = screenpix^2 / scale^2
        p_pixArea = polygon->getArea() / pow(cvp->scale(), 2);
        geos::geom::Point *center = polygon->getCentroid();
        double line, sample;
        cvp->viewportToCube((int)center->getX(), (int)center->getY(), sample,line);

        if(cvp->camera() != NULL) {
          cvp->camera()->SetImage(sample,line);
          // pix^2 * (m/pix)^2 = m^2
          p_mArea = p_pixArea * pow(cvp->camera()->PixelResolution(), 2);
          // m^2 * (km/m)^2 = km^2
          p_kmArea = p_mArea * pow(1 / 1000.0, 2);
        }

        if(cvp->projection() != NULL) {
          cvp->projection()->SetWorld(sample,line);
          // pix^2 * (m/pix)^2 = m^2
          p_mArea = p_pixArea * pow(cvp->projection()->Resolution(), 2);
          // m^2 * (km/m)^2 = km^2
          p_kmArea = p_mArea * pow(1 / 1000.0, 2);
        }
      }
    }

    updateDistEdit();
    updateRow(row);
  }


  //! Change the value in the distance edit to match the units
  void MeasureTool::updateDistEdit() {
    if(RubberBandTool::getMode() == RubberBandTool::Line) {
      if (p_unitsComboBox->currentIndex() == 0) {
        if (p_kmDist == Isis::Null) {
          p_distLineEdit->setText("N/A");
        }
        else {
          p_distLineEdit->setText(QString::number(p_kmDist));
        }
      }
      else if (p_unitsComboBox->currentIndex() == 1) {
        if (p_mDist == Isis::Null) {
          p_distLineEdit->setText("N/A");
        }
        else {
          p_distLineEdit->setText(QString::number(p_mDist));
        }
      }
      else {
        p_distLineEdit->setText(QString::number(p_pixDist));
      }
    }
    else if(RubberBandTool::getMode() == RubberBandTool::Angle) {
      if (p_unitsComboBox->currentIndex() == 0) {
        p_distLineEdit->setText(QString::number(p_degAngle));
      }
      else {
        p_distLineEdit->setText(QString::number(p_radAngle));
      }
    }
    else {
      if (p_unitsComboBox->currentIndex() == 0) {
        if (p_kmArea == Isis::Null) {
          p_distLineEdit->setText("N/A");
        }
        else {
          p_distLineEdit->setText(QString::number(p_kmArea));
        }
      }
      else if (p_unitsComboBox->currentIndex() == 1) {
        if (p_mArea == Isis::Null) {
          p_distLineEdit->setText("N/A");
        }
        else {
          p_distLineEdit->setText(QString::number(p_mArea));
        }
      }
      else {
        if(p_pixArea != Isis::Null) {
          p_distLineEdit->setText(QString::number(p_pixArea));
        }
        else {
          p_distLineEdit->setText("N/A");
        }
      }
    }
  }


  /**
   * Removes the connection on the given cube viewport.
   * 
   * 
   * @param cvp 
   */
  void MeasureTool::removeConnections(CubeViewport *cvp) {
    //  cvp->repaint();
    cvp->update();
  }


  /**
   * Updates the measure tool.
   * 
   */
  void MeasureTool::updateTool() {
    p_distLineEdit->clear();   
  }


}

