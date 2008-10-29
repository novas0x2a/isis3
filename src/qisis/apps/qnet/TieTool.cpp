#include <QApplication>
#include <QDialog>
#include <QMenuBar>
#include <QMenu>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QPoint>
#include <QStringList>
#include <QSizePolicy>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>

#include <vector>

#include "TieTool.h"
#include "MainWindow.h"
#include "CubeViewport.h"
#include "Filename.h"
#include "SerialNumber.h"
#include "QnetNewPointDialog.h"
#include "QnetDeletePointDialog.h"
#include "AutoReg.h"
#include "AutoRegFactory.h"
#include "ControlMeasure.h"

#include "qnet.h"
using namespace Qisis::Qnet;
using namespace std;


namespace Qisis {

  const int VIEWSIZE = 301;

  TieTool::TieTool (QWidget *parent) : Qisis::Tool(parent) {
#if 0
    p_createPoint = new QAction (parent);
    p_createPoint->setShortcut(Qt::Key_C);
    connect(p_createPoint,SIGNAL(triggered()),this,SLOT(createPoint()));

    p_modifyPoint = new QAction (parent);
    p_modifyPoint->setShortcut(Qt::Key_M);
//    connect(p_modifyPoint,SIGNAL(triggered()),this,SLOT(modifyPoint()));

    p_deletePoint = new QAction (parent);
    p_deletePoint->setShortcut(Qt::Key_D);
    connect(p_deletePoint,SIGNAL(triggered()),this,SLOT(deletePoint()));
#endif
    p_rotation = 0;
    p_timerOn = false;
    p_leftCube = NULL;
    p_rightCube = NULL;
    p_autoRegFact = NULL;

    try {
      Isis::Pvl pvl("$base/templates/autoreg/qnetReg.def");
      p_autoRegFact = Isis::AutoRegFactory::Create(pvl);
    } catch ( Isis::iException &e ) {
      p_autoRegFact = NULL;
      QString message = "Cannot create AutoRegFactory.  As a result,\n";
      message += "sub-pixel registration will not work\n";
      string errors = e.Errors();
      message += errors.c_str();
      e.Clear ();
      QMessageBox::information((QWidget *)parent,"Error",message);
    }

    createTieDialog(parent);
  }

  QAction *TieTool::toolPadAction(ToolPad *pad) {
    QAction *action = new QAction(pad);
    action->setIcon(QPixmap(toolIconDir()+"/stock_draw-connector-with-arrows.png"));
    action->setToolTip("Tie (T)");
    action->setShortcut(Qt::Key_T);
    return action;
  }

  QWidget *TieTool::createToolBarWidget (QStackedWidget *parent) {
    QWidget *hbox = new QWidget(parent);
#if 0
    QToolButton *pointButton = new QToolButton(p_hbox);
    pointButton->setToggleButton(true);
    pointButton->setOn(true);
    pointButton->setIcon(QPixmap(toolIconDir()+"/forward.png"));
    QString statusText = "Press C/M/D to create, modify, or delete a point";
    QToolTip::add(pointButton,"Show Points",toolTipGroup(),statusText);
    connect(pointButton,SIGNAL(toggled(bool)),p_showPoints,SLOT(setOn(bool)));

    QToolButton *polyButton = new QToolButton(p_hbox);
    polyButton->setToggleButton(true);
    polyButton->setOn(true);
    polyButton->setIconSet(QPixmap(toolIconDir()+"/forward.png"));
    QToolTip::add(polyButton,"Show Polygons",toolTipGroup(),statusText);
    connect(polyButton,SIGNAL(toggled(bool)),p_showPolygons,SLOT(setOn(bool)));
#endif
    return hbox;
  }

  /**
   * Handle mouse events
   * 
   * @param p[in]   (QPoint)            Point under cursor in cubeviewport
   * @param s[in]   (Qt::MouseButton)   Which mouse button was pressed
   * 
   * @internal
   * @history  2007-06-12 Tracie Sucharski - Swapped left and right mouse
   *                           button actions.
   */
  void TieTool::mouseButtonRelease(QPoint p, Qt::MouseButton s) {
    CubeViewport *cvp = cubeViewport();
    if (cvp  == NULL) return;
    if (cvp->cursorInside()) QPoint p = cvp->cursorPosition();

    p_leftFile = cvp->cube()->Filename();
    std::string leftSerialNumber = 
      g_serialNumberList->SerialNumber(p_leftFile);
    double samp,line;
    cvp->viewportToCube(p.x(),p.y(),samp,line);


    if (s == Qt::LeftButton) {
      //  Find closest control point in network
      Isis::ControlPoint *point =
                 g_controlNetwork->FindClosest(leftSerialNumber,samp,line);
      //  TODO:  test for errors and reality 
      modifyPoint(point);
    }
    else if (s == Qt::MidButton) {
      //  Find closest control point in network
      Isis::ControlPoint *point =
                 g_controlNetwork->FindClosest(leftSerialNumber,samp,line);
      //  TODO:  test for errors and reality 
      deletePoint(point);
    }
    else if (s == Qt::RightButton) {
      createPoint(samp,line);
    }

  }

  //! Find point from left ChipViewport in the right ChipViewport
  void TieTool::findPoint() {

    //  Get lat/lon from point in left
    int camIndex = 
      g_serialNumberList->SerialNumberIndex(p_leftMeasure->CubeSerialNumber());
    Isis::Camera *leftCam = g_controlNetwork->Camera(camIndex);
    leftCam->SetImage(p_leftView->tackSample(),p_leftView->tackLine());
    double lat = leftCam->UniversalLatitude();
    double lon = leftCam->UniversalLongitude();

    camIndex = 
      g_serialNumberList->SerialNumberIndex(p_rightMeasure->CubeSerialNumber());
    Isis::Camera *rightCam = g_controlNetwork->Camera(camIndex);
    //  Reload right chipViewport with this new tack point.
    if (rightCam->SetUniversalGround(lat,lon))
        emit updateRightView(rightCam->Sample(),rightCam->Line());

  }


  //!  Sub-pixel register point in right chipViewport with point in left
  void TieTool::registerPoint() {

    if (p_autoRegShown) {
      //  Undo Registration
      p_autoRegShown = false;
      p_autoRegExtension->hide();
      p_autoReg->setText("Register");

      //  Reload chip with original measure
      emit updateRightView(p_rightMeasure->Sample(),p_rightMeasure->Line());
      return;
      
    }

    p_autoRegFact->PatternChip()->TackCube(
                 p_leftMeasure->Sample(),p_leftMeasure->Line());
    p_autoRegFact->PatternChip()->Load(*p_leftCube);

    p_autoRegFact->SearchChip()->TackCube(
                 p_rightMeasure->Sample(),p_rightMeasure->Line());
    p_autoRegFact->SearchChip()->Load(*p_rightCube,*(p_autoRegFact->PatternChip()));

    if (p_autoRegFact->Register() != Isis::AutoReg::Success) {
      QString msg = "Cannot autoregister this point";
      QMessageBox::information((QWidget *)parent(),"Error",msg);
      return;
    }

    //  Load chip with new registered point
    emit updateRightView(p_autoRegFact->CubeSample(),p_autoRegFact->CubeLine());


    QString oldPos = "Original Sample: " + 
       QString::number(p_rightMeasure->Sample()) + "   Original Line:  " +
       QString::number(p_rightMeasure->Line());
    p_oldPosition->setText(oldPos);

    QString goodFit = "Goodness of Fit:  " +
       QString::number(p_autoRegFact->GoodnessOfFit());
    p_goodFit->setText(goodFit);

    p_autoRegExtension->show();
    p_autoRegShown = true;
    p_autoReg->setText("Undo Registration");
  }


  void TieTool::savePoint() {
    //  Get cube position at right chipViewport crosshair
    if (p_rightMeasure != NULL)
      p_rightMeasure->SetCoordinate(p_rightView->tackSample(),
                                    p_rightView->tackLine());

    //  If the right chip is the same as the left chip , update the left
    //  chipViewport.
    if (p_rightCombo->currentText() == p_leftCombo->currentText()) {
      p_leftMeasure->SetCoordinate(p_rightView->tackSample(),
                                   p_rightView->tackLine());
      emit updateLeftView(p_rightView->tackSample(),p_rightView->tackLine());
    }

    //  If autoReg is shown
    if (p_autoRegShown) {
      p_autoRegShown = false;
      p_autoRegExtension->hide();
      p_autoReg->setText("Register");
    }


    // Check if ControlPoint has reference measure, if reference Measure is
    // not the same measure that is on the left chip viewport, set left
    // measure as reference.
    if (p_controlPoint->HasReference()) {
      Isis::ControlMeasure *refMeasure = 
                      &(*p_controlPoint)[p_controlPoint->ReferenceIndex()];
      if (refMeasure != p_leftMeasure) {
        refMeasure->SetReference(false);
        p_leftMeasure->SetReference(true);
      }
    }
    else {
      p_leftMeasure->SetReference(true);
    }

    //  Redraw measures and overlaps on viewports
    emit editPointChanged(p_controlPoint->Id());
    emit netChanged();
  }

  //!  Create new control point
  void TieTool::createPoint(double samp,double line) {

    //  TODO:   ADD AUTOSEED OPTION (CHECKBOX?)

    //  Find lat/lon to find all other files containing 
    //  this point
    CubeViewport *cvp = cubeViewport();
    if (cvp  == NULL) return;
    Isis::Camera *cam = cvp->camera();
    cam->SetImage(samp,line);
    double lat = cam->UniversalLatitude();
    double lon = cam->UniversalLongitude();

    //  Create list of list box of all files highlighting those that
    //  contain the point.
    std::vector<std::string> pointFiles;

    //  Initialize camera for all images in control network,  
    //  TODO::   Needs to be moved to QnetFileTool.cpp
    //g_controlNetwork->SetImages(*g_serialNumberList);

    for (int i=0; i<g_serialNumberList->Size(); i++) {
      cam = g_controlNetwork->Camera(i);
      if (cam->SetUniversalGround(lat,lon)) {
        //  Make sure point is within image boundary
        double samp = cam->Sample();
        double line = cam->Line();
        if (samp >= 1 || samp <= cam->Samples() ||
            line >= 1 || line <= cam->Lines()) {
          pointFiles.push_back(g_serialNumberList->Filename(i));
        }
      }
    }

    QnetNewPointDialog *newPointDialog = new QnetNewPointDialog();
    newPointDialog->SetFiles(pointFiles);
    if (newPointDialog->exec()) {
      Isis::ControlPoint *newPoint = 
         new Isis::ControlPoint(newPointDialog->ptIdValue->text().toStdString());
      for (int i=0; i<newPointDialog->fileList->count(); i++) {
        QListWidgetItem *item = newPointDialog->fileList->item(i);
        if (!newPointDialog->fileList->isItemSelected(item)) continue;
        //  Create measure for any file selected
        Isis::ControlMeasure *m = new Isis::ControlMeasure;
        //  Find serial number for this file
        std::string sn =
                  g_serialNumberList->SerialNumber(item->text().toStdString());
        m->SetCubeSerialNumber(sn);
        int camIndex =
              g_serialNumberList->FilenameIndex(item->text().toStdString());
        cam = g_controlNetwork->Camera(camIndex);
        cam->SetUniversalGround(lat,lon);
        m->SetCoordinate(cam->Sample(),cam->Line());
        m->SetType(Isis::ControlMeasure::Estimated);
        m->SetDateTime();
        m->SetChooserName();
        newPoint->Add(*m);
      }
      //  Add new control point to control network
      g_controlNetwork->Add(*newPoint);
      //  Read newly added point
      p_controlPoint = 
        g_controlNetwork->Find(newPointDialog->ptIdValue->text().toStdString());
      //  Load new point in TieTool
      //p_controlPoint = newPoint;
      loadPoint();
      p_tieDialog->setShown(true);
      p_tieDialog->raise();

      emit editPointChanged(p_controlPoint->Id());
      emit netChanged();
    }
    
  }


  void TieTool::deletePoint(Isis::ControlPoint *point) {

    //  Change point in viewport to red so user can see what point they are 
    //  about to delete.
    emit editPointChanged(point->Id());

    p_controlPoint = point;

    QnetDeletePointDialog *deletePointDialog = new QnetDeletePointDialog;
    string CPId = p_controlPoint->Id();
    deletePointDialog->pointIdValue->setText(CPId.c_str());
    //  Need all files for this point
    for (int i=0; i<p_controlPoint->Size(); i++) {
      Isis::ControlMeasure &m = (*p_controlPoint)[i];
      std::string file = g_serialNumberList->Filename(m.CubeSerialNumber());
      deletePointDialog->fileList->addItem(file.c_str());
    }

    if (deletePointDialog->exec()) {
      //  First see if entire point needs to be deleted
      if (deletePointDialog->deleteAllCheckBox->isChecked()) {
        g_controlNetwork->Delete(p_controlPoint->Id());
      }
      //  Otherwise, delete measures located on images chosen
      else {
        for (int i=0; i<deletePointDialog->fileList->count(); i++) {
          QListWidgetItem *item = deletePointDialog->fileList->item(i);
          if (! deletePointDialog->fileList->isItemSelected(item)) continue;
          //  Delete measure from ControlPoint
          p_controlPoint->Delete(i);
        }
      }
    }

    emit editPointChanged(p_controlPoint->Id());
    emit netChanged();
  }



  void TieTool::deletePoint () {
    CubeViewport *cvp = cubeViewport();
    if (cvp  == NULL) return;
    if (cvp->cursorInside()) {
      QPoint p = cvp->cursorPosition();
      //deletePoint(p);
    }
    emit netChanged();
  }



  void TieTool::modifyPoint(Isis::ControlPoint *point) {

    p_controlPoint = point;
    loadPoint();
    p_tieDialog->setShown(true);
    p_tieDialog->raise();
    emit editPointChanged(p_controlPoint->Id());
  }

  void TieTool::loadPoint () {

    //  Write pointId to Point Editor
    string CPId = p_controlPoint->Id();
    QString ptId = "Point ID:  " +
                   QString::fromStdString(CPId.c_str());
    p_ptIdValue->setText(ptId);
  
    //  Set ignore box correctly
    p_ignorePoint->setChecked(p_controlPoint->Ignore());
  
    //  Set held box correctly
    p_holdPoint->setChecked(p_controlPoint->Held());

    //  Set ground box correctly
    p_groundPoint->setChecked(
         p_controlPoint->Type() == Isis::ControlPoint::Ground);

    //  Make sure registration is turned off
    if (p_autoRegShown) {
      //  Undo Registration
      p_autoRegShown = false;
      p_autoRegExtension->hide();
      p_autoReg->setText("Register");
    }

    // Clear combo boxes
    p_leftCombo->clear();
    p_rightCombo->clear();
    p_pointFiles.clear();
    //  Need all files for this point
    for (int i=0; i<p_controlPoint->Size(); i++) {
      Isis::ControlMeasure &m = (*p_controlPoint)[i];
      std::string file = g_serialNumberList->Filename(m.CubeSerialNumber());
      p_pointFiles.push_back(file);
      string tempFilename = Isis::Filename(file).Name();
      p_leftCombo->addItem(tempFilename.c_str());
      p_rightCombo->addItem(tempFilename.c_str());
    }


    //  TODO:  WHAT HAPPENS IF THERE IS ONLY ONE MEASURE IN THIS CONTROLPOINT??
    // 
    // 
    //  Find the file from the cubeViewport that was originally used to select 
    //  the point, this will be displayed on the left ChipViewport.
    int leftIndex = 0;
    if (p_leftFile.length() != 0) {
      string tempFilename = Isis::Filename(p_leftFile).Name();
      leftIndex = p_leftCombo->findText(QString(tempFilename.c_str()));
    }
    int rightIndex = 0;
  
    if (leftIndex == 0) {
      rightIndex = 1;
    }
    else {
      rightIndex = 0;
    }
    p_leftCombo->setCurrentIndex(leftIndex);
    p_rightCombo->setCurrentIndex(rightIndex);
  
    //  Initialize chipViewports by calling slot connected to comboBoxes
    selectLeftChip(leftIndex);
    selectRightChip(rightIndex);

    updateLeftMeasureInfo(leftIndex);
    updateRightMeasureInfo(rightIndex);
  }

  void TieTool::selectLeftChip(int index) {

    std::string file = p_pointFiles[index];

    std::string serial = g_serialNumberList->SerialNumber(file);
    //  Find measure for each file
    p_leftMeasure = &(*p_controlPoint)[serial];

    //  If p_leftCube is not null, delete before creating new one
    if (p_leftCube != NULL) delete p_leftCube;
    p_leftCube = new Isis::Cube();
    p_leftCube->Open(file);
    p_leftChip->TackCube(p_leftMeasure->Sample(),p_leftMeasure->Line());
    p_leftChip->Load(*p_leftCube);

    // Dump into left chipViewport
    p_leftView->setChip(p_leftChip);

    //  Set ignore measure box correctly
    p_ignoreLeftMeasure->setChecked(p_leftMeasure->Ignore());

    //  Draw "X" on measure
    //p_leftView->markPoint (measure.Sample(),measure.Line());

  }


  void TieTool::selectRightChip(int index) {

    std::string file = p_pointFiles[index];

    std::string serial = g_serialNumberList->SerialNumber(file);
    //  Find measure for each file
    p_rightMeasure = &(*p_controlPoint)[serial];

    //  If p_rightCube is not null, delete before creating new one
    if (p_rightCube != NULL) delete p_rightCube;
    p_rightCube = new Isis::Cube();
    p_rightCube->Open(file);
    p_rightChip->TackCube(p_rightMeasure->Sample(),p_rightMeasure->Line());
    if (p_geomIt == false) {
      p_rightChip->Load(*p_rightCube);
    }
    else {
      try {
        p_rightChip->Load(*p_rightCube,*p_leftChip);

      } catch (Isis::iException &e) {
        QString message = "Geom failed  \n";
        string errors = e.Errors();
        message += errors.c_str();
        e.Clear ();
        QMessageBox::information((QWidget *)parent(),"Error",message);
        p_rightChip->Load(*p_rightCube);
        p_geomIt = false;
        p_nogeom->setChecked(true);
        p_geom->setChecked(false);
      }
    }

    // Dump into left chipViewport
    p_rightView->setChip(p_rightChip);

    //  Set ignore measure box correctly
    p_ignoreRightMeasure->setChecked(p_rightMeasure->Ignore());

    //  Draw "X" on measure
    //p_rightView->markPoint (measure.Sample(),measure.Line());
  }


  void TieTool::updateLeftMeasureInfo (int index) {

    if (index < 0) return;
    
    QString s;

    std::string file = p_pointFiles[index];

    std::string serial = g_serialNumberList->SerialNumber(file);
    //  Find measure info
    Isis::ControlMeasure *m = &(*p_controlPoint)[serial];

    s = "Measure Type: ";
    if (m->Type() == Isis::ControlMeasure::Unmeasured) s+= "Unmeasured";
    if (m->Type() == Isis::ControlMeasure::Manual) s+= "Manual";
    if (m->Type() == Isis::ControlMeasure::Estimated) s+= "Estimated";
    if (m->Type() == Isis::ControlMeasure::Automatic) s+= "Automatic";
    if (m->Type() == Isis::ControlMeasure::ValidatedManual) s+= "ValidatedManual";
    if (m->Type() == Isis::ControlMeasure::ValidatedAutomatic) s+= "ValidatedAutomatic";
    p_leftMeasureType->setText(s);
    s = "Sample Error: " + QString::number(m->SampleError());
    p_leftSampError->setText(s);
    s = "Line Error: " + QString::number(m->LineError());
    p_leftLineError->setText(s);

  }

  void TieTool::updateRightMeasureInfo (int index) {

    QString s;

    std::string serial = g_serialNumberList->SerialNumber(p_pointFiles[index]);
    //  Find measure info
    Isis::ControlMeasure *m = &(*p_controlPoint)[serial];

    s = "Measure Type: ";
    if (m->Type() == Isis::ControlMeasure::Unmeasured) s+= "Unmeasured";
    if (m->Type() == Isis::ControlMeasure::Manual) s+= "Manual";
    if (m->Type() == Isis::ControlMeasure::Estimated) s+= "Estimated";
    if (m->Type() == Isis::ControlMeasure::Automatic) s+= "Automatic";
    if (m->Type() == Isis::ControlMeasure::ValidatedManual) s+= "ValidatedManual";
    if (m->Type() == Isis::ControlMeasure::ValidatedAutomatic) s+= "ValidatedAutomatic";
    p_rightMeasureType->setText(s);
    s = "Sample Error: " + QString::number(m->SampleError());
    p_rightSampError->setText(s);
    s = "Line Error: " + QString::number(m->LineError());
    p_rightLineError->setText(s);

  }


  bool TieTool::eventFilter (QObject *o, QEvent *e) {
    if (e->type() != QEvent::Leave) return false;
    if (o == p_leftCombo->view()) {
      updateLeftMeasureInfo (p_leftCombo->currentIndex());
      p_leftCombo->hidePopup();
    }
    if (o == p_rightCombo->view()) {
      updateRightMeasureInfo (p_rightCombo->currentIndex());
      p_rightCombo->hidePopup();
    }
    return true;
  }

  void TieTool::updateLeftPositionLabel(double zoomFactor) {
    QString pos = "Sample: " + QString::number(p_leftView->tackSample()) +
                  "    Line:  " + QString::number(p_leftView->tackLine());
    p_leftSampLinePosition->setText(pos);

    //  Get lat/lon from point in left
    int camIndex = 
      g_serialNumberList->SerialNumberIndex(p_leftMeasure->CubeSerialNumber());
    Isis::Camera *cam = g_controlNetwork->Camera(camIndex);
    cam->SetImage(p_leftView->tackSample(),p_leftView->tackLine());
    double lat = cam->UniversalLatitude();
    double lon = cam->UniversalLongitude();

    pos = "Latitude: " + QString::number(lat) +
                  "    Longitude:  " + QString::number(lon);
    p_leftLatLonPosition->setText(pos);

    //  Print zoom scale factor
    pos = "Zoom Factor: " + QString::number(zoomFactor);
    p_leftZoomFactor->setText(pos);

  }


  void TieTool::updateRightPositionLabel(double zoomFactor) {
    QString pos = "Sample: " + QString::number(p_rightView->tackSample()) +
                  "    Line:  " + QString::number(p_rightView->tackLine());
    p_rightSampLinePosition->setText(pos);

    //  Get lat/lon from point in right
    int camIndex = 
      g_serialNumberList->SerialNumberIndex(p_rightMeasure->CubeSerialNumber());
    Isis::Camera *cam = g_controlNetwork->Camera(camIndex);
    cam->SetImage(p_rightView->tackSample(),p_rightView->tackLine());
    double lat = cam->UniversalLatitude();
    double lon = cam->UniversalLongitude();

    pos = "Latitude: " + QString::number(lat) +
                  "    Longitude:  " + QString::number(lon);
    p_rightLatLonPosition->setText(pos);

        //  Print zoom scale factor
    pos = "Zoom Factor: " + QString::number(zoomFactor);
    p_rightZoomFactor->setText(pos);

  }


  void TieTool::updateRightGeom() {

    if (p_geomIt) {
      try {
        p_rightView->geomChip(p_leftChip);

      } catch (Isis::iException &e) {
        QString message = "Geom failed  \n";
        string errors = e.Errors();
        message += errors.c_str();
        e.Clear ();
        QMessageBox::information((QWidget *)parent(),"Error",message);
        p_geomIt = false;
        p_nogeom->setChecked(true);
        p_geom->setChecked(false);
      }
    }

  }


  void TieTool::setIgnorePoint (bool ignore) {
    if (p_controlPoint != NULL) p_controlPoint->SetIgnore(ignore);
  }


  void TieTool::setHoldPoint (bool hold) {
    if (p_controlPoint != NULL) p_controlPoint->SetHeld(hold);
  }


  void TieTool::setGroundPoint (bool ground) {

    //  if false, turn back Tie
    if (!ground) {
      p_controlPoint->SetType(Isis::ControlPoint::Tie);
    }
    else {
      p_groundPointDialog = new QnetGroundPointDialog;
      p_groundPointDialog->setModal(true);
      p_groundPointDialog->setPoint(*p_controlPoint);
      connect (p_groundPointDialog,SIGNAL(groundPoint(Isis::ControlPoint &)),
               this,SLOT(newGroundPoint(Isis::ControlPoint &)));
      p_groundPointDialog->exec();

    }

  }



  void TieTool::newGroundPoint(Isis::ControlPoint &point) {

    p_controlPoint = &point;
    p_controlPoint->SetType(Isis::ControlPoint::Ground);

    p_groundPointDialog->close();

  }



  void TieTool::setIgnoreLeftMeasure (bool ignore) {
    if (p_leftMeasure != NULL) p_leftMeasure->SetIgnore(ignore);

    //  If the right chip is the same as the left chip , update the right
    //  ignore blox.
    if (p_rightCombo->currentText() == p_leftCombo->currentText()) {
      if (p_rightMeasure != NULL) {
        p_rightMeasure->SetIgnore(ignore);
        p_ignoreRightMeasure->setChecked(ignore);
      }
    }
      

  }


  void TieTool::setIgnoreRightMeasure (bool ignore) {
    if (p_rightMeasure != NULL) p_rightMeasure->SetIgnore(ignore);

    //  If the right chip is the same as the left chip , update the right
    //  ignore blox.
    if (p_rightCombo->currentText() == p_leftCombo->currentText()) {
      if (p_leftMeasure != NULL) {
        p_leftMeasure->SetIgnore(ignore);
        p_ignoreLeftMeasure->setChecked(ignore);
      }
    }

  }


  void TieTool::setRotate() {
    p_dial->setEnabled(true);
    p_dialNumber->setEnabled(true);
    p_dial->setNotchesVisible(true);

  }

  void TieTool::setRotation(int rotation) {

    //  TODO:   Do we need this method  ??? Don't think we need p_rotation
    p_rotation = rotation;

  }

  
  /**
   * Turn geom on
   * 
   * @internal
   * @history  2007-06-15 Tracie Sucharski - Grey out zoom buttons
   * 
   */
  void TieTool::setGeom() {

    if (p_geomIt == true) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    //  Grey right view zoom buttons
    QString text = "Zoom functions disabled when Geom is set";
    p_rightZoomIn->setEnabled(false);
    p_rightZoomIn->setWhatsThis(text);
    p_rightZoomIn->setToolTip(text);
    p_rightZoomOut->setEnabled(false);
    p_rightZoomOut->setWhatsThis(text);
    p_rightZoomOut->setToolTip(text);
    p_rightZoom1->setEnabled(false);
    p_rightZoom1->setWhatsThis(text);
    p_rightZoom1->setToolTip(text);


    //  Reset dial to 0 before disabling
    p_dial->setValue(0);
    p_dial->setEnabled(false);
    p_dialNumber->setEnabled(false);

    p_geomIt = true;

    try {
      p_rightView->geomChip(p_leftChip);

    } catch (Isis::iException &e) {
      QString message = "Geom failed  \n";
      string errors = e.Errors();
      message += errors.c_str();
      e.Clear ();
      QMessageBox::information((QWidget *)parent(),"Error",message);
      p_geomIt = false;
      p_nogeom->setChecked(true);
      p_geom->setChecked(false);
    }

    QApplication::restoreOverrideCursor();
  }


  void TieTool::setNoGeom() {

    if (p_geomIt == false) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString text = "Zoom in 2X";
    p_rightZoomIn->setEnabled(true);
    p_rightZoomIn->setWhatsThis(text);
    p_rightZoomIn->setToolTip("Zoom In");
    text = "Zoom out 2X";
    p_rightZoomOut->setEnabled(true);
    p_rightZoomOut->setWhatsThis(text);
    p_rightZoomOut->setToolTip("Zoom Out");
    text = "Zoom 1:1";
    p_rightZoom1->setEnabled(true);
    p_rightZoom1->setWhatsThis(text);
    p_rightZoom1->setToolTip("Zoom 1:1");

    //  Reset dial to 0 before disabling
    p_dial->setValue(0);
    p_dial->setEnabled(false);
    p_dialNumber->setEnabled(false);

    p_geomIt = false;
    p_rightView->nogeomChip();

    QApplication::restoreOverrideCursor();
  }


  void TieTool::blinkStart() {
    if (p_timerOn) return;

    //  Set up blink list
    p_blinkList.push_back(p_leftView);
    p_blinkList.push_back(p_rightView);
    p_blinkIndex = 0;

    p_timerOn = true;
    int msec = (int) (p_blinkTimeBox->value() * 1000.0);
    p_timer = new QTimer(this);
    connect(p_timer, SIGNAL(timeout()), this, SLOT(updateBlink()));
    p_timer->start(msec);
  }


  void TieTool::blinkStop() {
    p_timer->stop();
    p_timerOn = false;
    p_blinkList.clear();

    //  Reload left chipViewport with original chip
    p_leftView->repaint();

  }


  void TieTool::changeBlinkTime(double interval) {
    if (p_timerOn) p_timer->setInterval((int)(interval*1000.));
  }


  void TieTool::updateBlink() {

    p_blinkIndex = !p_blinkIndex;
    p_leftView->loadView(*(p_blinkList)[p_blinkIndex]);
  }

  void TieTool::createTieDialog(QWidget *parent) {
    // Create dialog with a main window
    p_tieDialog = new QMainWindow(parent);
    p_tieDialog->setWindowTitle("Point Editor");
    p_tieDialog->layout()->setSizeConstraint(QLayout::SetFixedSize);
    // Add menus
    createMenus ();

    // Place everything in a grid
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSizeConstraint(QLayout::SetFixedSize);
    //  grid row
    int row = 0;

    //  Point Id and Hold / Ignore flags
    p_ptIdValue = new QLabel();
    gridLayout->addWidget(p_ptIdValue,row++,0);
    p_ignorePoint = new QCheckBox("Ignore Point");
    gridLayout->addWidget(p_ignorePoint,row++,0);
    connect(p_ignorePoint,SIGNAL(toggled(bool)),this,SLOT(setIgnorePoint(bool)));

    p_holdPoint = new QCheckBox("Hold Point");
    gridLayout->addWidget(p_holdPoint,row++,0);
    connect(p_holdPoint,SIGNAL(toggled(bool)),this,SLOT(setHoldPoint(bool)));

    p_groundPoint = new QCheckBox("Ground Point");
    gridLayout->addWidget(p_groundPoint,row++,0);
    connect(p_groundPoint,SIGNAL(toggled(bool)),this,SLOT(setGroundPoint(bool)));

    p_leftMeasureType = new QLabel();
    gridLayout->addWidget(p_leftMeasureType,row,0);
    p_rightMeasureType = new QLabel();
    gridLayout->addWidget(p_rightMeasureType,row++,1);

    p_leftSampError = new QLabel();
    gridLayout->addWidget(p_leftSampError,row,0);
    p_rightSampError = new QLabel();
    gridLayout->addWidget(p_rightSampError,row++,1);

    p_leftLineError = new QLabel();
    gridLayout->addWidget(p_leftLineError,row,0);
    p_rightLineError = new QLabel();
    gridLayout->addWidget(p_rightLineError,row++,1);

    // Set up chipViewport and comboBox to select chips
    p_leftCombo = new QComboBox ();
    p_leftCombo->setFixedWidth(256);
    gridLayout->addWidget(p_leftCombo,row,0);

    //  Install event filter so that when user moves away
    //  from combo box the correct measurement info is written back
    //  out.
    p_leftCombo->view()->installEventFilter(this);
    connect(p_leftCombo,SIGNAL(activated(int)),
            this,SLOT(selectLeftChip(int)));
    connect(p_leftCombo,SIGNAL(activated(int)),
            this,SLOT(updateLeftMeasureInfo(int)));
    connect(p_leftCombo,SIGNAL(highlighted(int)),
            this,SLOT(updateLeftMeasureInfo(int)));

    p_rightCombo = new QComboBox ();
    p_rightCombo->setFixedWidth(256);
    gridLayout->addWidget(p_rightCombo,row++,1);
    p_rightCombo->view()->installEventFilter(this);
    connect(p_rightCombo,SIGNAL(activated(int)),
            this,SLOT(selectRightChip(int)));
    connect(p_rightCombo,SIGNAL(activated(int)),
            this,SLOT(updateRightMeasureInfo(int)));
    connect(p_rightCombo,SIGNAL(highlighted(int)),
            this,SLOT(updateRightMeasureInfo(int)));

    QSize isize(27,27);
    //  Add zoom buttons
    QToolButton *leftZoomIn = new QToolButton();
    leftZoomIn->setIcon(QPixmap(toolIconDir()+"/viewmag+.png"));
    leftZoomIn->setIconSize(isize);
    leftZoomIn->setToolTip("Zoom In");

    QToolButton *leftZoomOut = new QToolButton();
    leftZoomOut->setIcon(QPixmap(toolIconDir()+"/viewmag-.png"));
    leftZoomOut->setIconSize(isize);
    leftZoomOut->setToolTip("Zoom Out");

    QToolButton *leftZoom1 = new QToolButton();
    leftZoom1->setIcon(QPixmap(toolIconDir()+"/viewmag1.png"));
    leftZoom1->setIconSize(isize);
    leftZoom1->setToolTip("Zoom 1:1");

    QHBoxLayout *leftZoom = new QHBoxLayout;
    leftZoom->addWidget(leftZoomIn);
    leftZoom->addWidget(leftZoomOut);
    leftZoom->addWidget(leftZoom1);
    leftZoom->addStretch();

    gridLayout->addLayout(leftZoom,row,0);


    p_rightZoomIn = new QToolButton();
    p_rightZoomIn->setIcon(QPixmap(toolIconDir()+"/viewmag+.png"));
    p_rightZoomIn->setIconSize(isize);
    p_rightZoomIn->setToolTip("Zoom In");
    QString text = "Zoom in 2X";
    p_rightZoomIn->setWhatsThis(text);

    p_rightZoomOut = new QToolButton();
    string rightZoomIconFile =Isis::Filename("$base/icons/viewmag-.png").Expanded();
    p_rightZoomOut->setIcon(QIcon(rightZoomIconFile.c_str()));
    p_rightZoomOut->setIconSize(isize);
    p_rightZoomOut->setToolTip("Zoom Out");
    text = "Zoom out 2X";
    p_rightZoomOut->setWhatsThis(text);

    p_rightZoom1 = new QToolButton();
    p_rightZoom1->setIcon(QPixmap(toolIconDir()+"/viewmag1.png"));
    p_rightZoom1->setIconSize(isize);
    p_rightZoom1->setToolTip("Zoom 1:1");
    text = "Zoom 1:1";
    p_rightZoom1->setWhatsThis(text);

    QHBoxLayout *rightZoomPan = new QHBoxLayout;
    rightZoomPan->addWidget(p_rightZoomIn);
    rightZoomPan->addWidget(p_rightZoomOut);
    rightZoomPan->addWidget(p_rightZoom1);

    //  Add arrows for panning
    QToolButton *upButton = new QToolButton(parent);
    string upButtonFile = Isis::Filename("$base/icons/up.png").Expanded();
    upButton->setIcon(QIcon(upButtonFile.c_str()));
    upButton->setIconSize(isize);
    upButton->setToolTip("Move up 1 screen pixel");
    QToolButton *downButton = new QToolButton(parent);
    string downButtonFile = Isis::Filename("$base/icons/down.png").Expanded();
    downButton->setIcon(QIcon(downButtonFile.c_str()));
    downButton->setIconSize(isize);
    downButton->setToolTip("Move down 1 screen pixel");
    QToolButton *leftButton = new QToolButton(parent);
    string leftButtonFile = Isis::Filename("$base/icons/back.png").Expanded();
    leftButton->setIcon(QIcon(leftButtonFile.c_str()));
    leftButton->setIconSize(isize);
    leftButton->setToolTip("Move left 1 screen pixel");
    QToolButton *rightButton = new QToolButton(parent);
    string rightButtonFile = Isis::Filename("$base/icons/forward.png").Expanded();
    rightButton->setIcon(QIcon(rightButtonFile.c_str()));
    rightButton->setIconSize(isize);
    rightButton->setToolTip("Move right 1 screen pixel");


    rightZoomPan->addWidget(upButton);
    rightZoomPan->addWidget(downButton);
    rightZoomPan->addWidget(leftButton);
    rightZoomPan->addWidget(rightButton);
    rightZoomPan->addStretch();

    gridLayout->addLayout(rightZoomPan,row++,1);

    //  Add zoom factor label
    p_leftZoomFactor = new QLabel();
    gridLayout->addWidget(p_leftZoomFactor,row,0);
    p_rightZoomFactor = new QLabel();
    gridLayout->addWidget(p_rightZoomFactor,row++,1);

    p_leftView = new ChipViewport(VIEWSIZE,VIEWSIZE,p_tieDialog);
    p_leftView->setFocusPolicy(Qt::NoFocus);
    //  Do not want to accept mouse/keyboard events
    p_leftView->setDisabled(true);
    gridLayout->addWidget(p_leftView,row,0);
    //  Connect the ChipViewport tackPointChanged signal to
    //  the update sample/line label
    connect(p_leftView,SIGNAL(tackPointChanged(double)),
            this,SLOT(updateLeftPositionLabel(double)));
    connect(this,SIGNAL(updateLeftView(double,double)),
            p_leftView,SLOT(refreshView(double,double)));

    p_rightView = new ChipViewport(VIEWSIZE,VIEWSIZE,p_tieDialog);
    gridLayout->addWidget(p_rightView,row,1);
    //  Connect the ChipViewport tackPointChanged signal to
    //  the update sample/line label
    connect(p_rightView,SIGNAL(tackPointChanged(double)),
            this,SLOT(updateRightPositionLabel(double)));
    connect(this,SIGNAL(updateRightView(double,double)),
            p_rightView,SLOT(refreshView(double,double)));

    //  Connect pan buttons to ChipViewport
    connect(upButton,SIGNAL(clicked()),p_rightView,SLOT(panUp()));
    connect(downButton,SIGNAL(clicked()),p_rightView,SLOT(panDown()));
    connect(leftButton,SIGNAL(clicked()),p_rightView,SLOT(panLeft()));
    connect(rightButton,SIGNAL(clicked()),p_rightView,SLOT(panRight()));

    //  Connect left zoom buttons to ChipViewport's zoom slots
    connect(leftZoomIn,SIGNAL(clicked()),p_leftView,SLOT(zoomIn()));
    connect(leftZoomOut,SIGNAL(clicked()),p_leftView,SLOT(zoomOut()));
    connect(leftZoom1,SIGNAL(clicked()),p_leftView,SLOT(zoom1()));

    //  If zoom on left, need to re-geom right
    connect(leftZoomIn,SIGNAL(clicked()),this,SLOT(updateRightGeom()));
    connect(leftZoomOut,SIGNAL(clicked()),this,SLOT(updateRightGeom()));
    connect(leftZoom1,SIGNAL(clicked()),this,SLOT(updateRightGeom()));

    connect(p_rightZoomIn,SIGNAL(clicked()),p_rightView,SLOT(zoomIn()));
    connect(p_rightZoomOut,SIGNAL(clicked()),p_rightView,SLOT(zoomOut()));
    connect(p_rightZoom1,SIGNAL(clicked()),p_rightView,SLOT(zoom1()));


    //  Create chips for left and right
    p_leftChip = new Isis::Chip(VIEWSIZE,VIEWSIZE);
    p_rightChip = new Isis::Chip(VIEWSIZE,VIEWSIZE);

    p_nogeom = new QRadioButton ("No geom");
    p_nogeom->setChecked(true);
    p_geom   = new QRadioButton ("Geom");
    QRadioButton *rotate = new QRadioButton ("Rotate");
    QButtonGroup *bgroup = new QButtonGroup ();
    bgroup->addButton(p_nogeom);
    bgroup->addButton(p_geom);
    bgroup->addButton(rotate);

    connect(p_nogeom,SIGNAL(clicked()),this,SLOT(setNoGeom()));
    connect(p_geom,SIGNAL(clicked()),this,SLOT(setGeom()));

    //  TODO:  ?? Don't think we need this connection
    connect(rotate,SIGNAL(clicked()),this,SLOT(setRotate()));

    //  Set some defaults
    p_geomIt = false;
    p_rotation = 0;

    p_dial = new QDial();
    p_dial->setRange(0,360);
    p_dial->setWrapping(false);
    p_dial->setNotchesVisible(true);
    p_dial->setNotchTarget(5.);
    //std::cout<<"notch vis: "<<p_dial->notchesVisible()<<std::endl;
    //std::cout<<"notch size: "<<p_dial->notchSize()<<std::endl;
    //std::cout<<"notch targ: "<<p_dial->notchTarget()<<std::endl;
    p_dial->setEnabled(false);

    p_dialNumber = new QLCDNumber();
    p_dialNumber->setEnabled(false);
    connect(p_dial,SIGNAL(valueChanged(int)),p_dialNumber,SLOT(display(int)));
    connect(p_dial,SIGNAL(valueChanged(int)),p_rightView,SLOT(rotateChip(int)));

    QCheckBox *cross = new QCheckBox("Show crosshair");
    connect(cross,SIGNAL(toggled(bool)),p_leftView,SLOT(setCross(bool)));
    connect(cross,SIGNAL(toggled(bool)),p_rightView,SLOT(setCross(bool)));
    cross->setChecked(true);

    QCheckBox *circle = new QCheckBox("Circle");
    circle->setDisabled(true);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addWidget(p_nogeom);
    vlayout->addWidget(p_geom);
    vlayout->addWidget(rotate);
    vlayout->addWidget(p_dial);
    vlayout->addWidget(p_dialNumber);
    vlayout->addWidget(cross);
    vlayout->addWidget(circle);
    gridLayout->addLayout(vlayout,row++,2);

    //  Show sample / line for measure of chips shown
    p_leftSampLinePosition = new QLabel();
    gridLayout->addWidget(p_leftSampLinePosition,row,0);
    p_rightSampLinePosition = new QLabel();
    gridLayout->addWidget(p_rightSampLinePosition,row++,1);

    //  Show lat / lon for measure of chips shown
    p_leftLatLonPosition = new QLabel();
    gridLayout->addWidget(p_leftLatLonPosition,row,0);
    p_rightLatLonPosition = new QLabel();
    gridLayout->addWidget(p_rightLatLonPosition,row++,1);


    //  Show if measure ignore flag is set
    p_ignoreLeftMeasure = new QCheckBox("Ignore Measure");
    gridLayout->addWidget(p_ignoreLeftMeasure,row,0);
    connect(p_ignoreLeftMeasure,SIGNAL(toggled(bool)),
            this,SLOT(setIgnoreLeftMeasure(bool)));

    p_ignoreRightMeasure = new QCheckBox("Ignore Measure");
    gridLayout->addWidget(p_ignoreRightMeasure,row++,1);
    connect(p_ignoreRightMeasure,SIGNAL(toggled(bool)),
            this,SLOT(setIgnoreRightMeasure(bool)));

        //  Add auto registration extension
    p_autoRegExtension = new QWidget;
    p_oldPosition = new QLabel;
    p_goodFit = new QLabel;
    QVBoxLayout *autoRegLayout = new QVBoxLayout;
    autoRegLayout->setMargin(0);
    autoRegLayout->addWidget(p_oldPosition);
    autoRegLayout->addWidget(p_goodFit);
    p_autoRegExtension->setLayout(autoRegLayout);
    p_autoRegShown = false;
    gridLayout->addWidget(p_autoRegExtension,row++,1);


    QHBoxLayout *leftLayout = new QHBoxLayout();
    QToolButton *stop = new QToolButton();
    stop->setIcon(QPixmap(toolIconDir()+"/blinkStop.png"));
    stop->setIconSize(QSize(22,22));
    stop->setToolTip("Blink Stop");
    text = "<b>Function:</b> Stop automatic timed blinking";
    stop->setWhatsThis(text);
    connect(stop,SIGNAL(released()),this,SLOT(blinkStop()));

    QToolButton *start = new QToolButton();
    start->setIcon(QPixmap(toolIconDir()+"/blinkStart.png"));
    start->setIconSize(QSize(22,22));
    start->setToolTip("Blink Start");
    text = "<b>Function:</b> Start automatic timed blinking.  Cycles \
           through linked viewports at variable rate";
    start->setWhatsThis(text);
    connect(start,SIGNAL(released()),this,SLOT(blinkStart()));

    p_blinkTimeBox = new QDoubleSpinBox();
    p_blinkTimeBox->setMinimum(0.1);
    p_blinkTimeBox->setMaximum(5.0);
    p_blinkTimeBox->setDecimals(1);
    p_blinkTimeBox->setSingleStep(0.1);
    p_blinkTimeBox->setValue(0.5);
    p_blinkTimeBox->setToolTip("Blink Time Delay");
    text = "<b>Function:</b> Change automatic blink rate between " +
      QString::number(p_blinkTimeBox->minimum()) + " and " +
      QString::number(p_blinkTimeBox->maximum()) + " seconds";
    p_blinkTimeBox->setWhatsThis(text);
    connect(p_blinkTimeBox,SIGNAL(valueChanged(double)),
            this,SLOT(changeBlinkTime(double)));

    QPushButton *find = new QPushButton ("Find");

    leftLayout->addWidget(stop);
    leftLayout->addWidget(start);
    leftLayout->addWidget(p_blinkTimeBox);
    leftLayout->addWidget(find);
    leftLayout->addStretch();
    //  TODO: Move AutoPick to QnetNewPointDialog
//    QPushButton *autopick = new QPushButton ("AutoPick");
//    autopick->setFocusPolicy(Qt::NoFocus);
//    QPushButton *blink = new QPushButton ("Blink");
//    blink->setFocusPolicy(Qt::NoFocus);
//    leftLayout->addWidget(autopick);
//    leftLayout->addWidget(blink);
    gridLayout->addLayout(leftLayout,row,0);


    QHBoxLayout *rightLayout = new QHBoxLayout();
    p_autoReg = new QPushButton ("Register");
    QPushButton *save = new QPushButton ("Save Point");

    rightLayout->addWidget(p_autoReg);
    rightLayout->addWidget(save);
    rightLayout->addStretch();
    gridLayout->addLayout(rightLayout,row,1);

    connect(find,SIGNAL(clicked()),this,SLOT(findPoint()));
    connect(p_autoReg,SIGNAL(clicked()),this,SLOT(registerPoint()));
    connect(save,SIGNAL(clicked()),this,SLOT(savePoint()));

    QPushButton *addMeasure = new QPushButton ("Add Measure(s) to Point");
    connect(addMeasure,SIGNAL(clicked()),this,SLOT(addMeasure()));
    gridLayout->addWidget(addMeasure,row++,2);

    QWidget *cw = new QWidget();
    cw->setLayout(gridLayout);
    
    p_tieDialog->setCentralWidget(cw);
    p_autoRegExtension->hide();

  }

  void TieTool::createMenus () {

    QAction *save = new QAction(p_tieDialog);
    save->setText("Save Control Network");
    connect (save,SIGNAL(activated()),this,SLOT(save()));

    QMenu *fileMenu = p_tieDialog->menuBar()->addMenu("&File");
    fileMenu->addAction(save);

    QAction *templateFile = new QAction(p_tieDialog);
    templateFile->setText("Select registration template");
    connect (templateFile,SIGNAL(activated()),this,SLOT(openTemplateFile()));

    QAction *viewTemplate = new QAction(p_tieDialog);
    viewTemplate->setText("View current registration template");
 //   connect (viewTemplate,SIGNAL(activated()),this,SLOT(viewTemplateFile()));

//     QAction *interestOp = new QAction(p_tieDialog);
//     interestOp->setText("Choose interest operator");
//     connect (interestOp,SIGNAL(activated()),this,SLOT(setInterestOp()));

    QMenu *optionMenu = p_tieDialog->menuBar()->addMenu("&Options");
    QMenu *regMenu = optionMenu->addMenu("&Registration");

    regMenu->addAction(templateFile);
    regMenu->addAction(viewTemplate);
    //    registrationMenu->addAction(interestOp);

  }


  void TieTool::save () {
    emit tieToolSave();
  }


  void TieTool::openTemplateFile() {

    QString filter = "Select registration template (*.def *.pvl);;";
    filter += "All (*)";
    QString regDef = QFileDialog::getOpenFileName((QWidget*)parent(),
                                                "Select a registration template",
                                                ".",
                                                filter);
    if (regDef.isEmpty()) return;
    Isis::AutoReg *reg=NULL;
    try {
      Isis::Pvl pvl(regDef.toStdString());
      reg = Isis::AutoRegFactory::Create(pvl);
    } catch ( Isis::iException &e ) {
      p_autoRegFact = NULL;
      QString message = "Cannot create AutoRegFactory.  As a result,\n";
      message += "sub-pixel registration will not work\n";
      string errors = e.Errors();
      message += errors.c_str();
      e.Clear ();
      QMessageBox::information((QWidget *)parent(),"Error",message);
    }

    if (p_autoRegFact != NULL) delete p_autoRegFact;
    p_autoRegFact = reg;

  }


  void TieTool::addMeasure() {

    

  }


#if 0
  void TieTool::setInterestOp() {



  }
#endif

}

