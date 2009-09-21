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
#include <QCheckBox>
#include <QSpinBox>
#include <QPoint>
#include <QStringList>
#include <QSizePolicy>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>

#include <vector>

#include "QtieTool.h"
#include "MainWindow.h"
#include "CubeViewport.h"
#include "Filename.h"
#include "SerialNumber.h"
#include "AutoReg.h"
#include "AutoRegFactory.h"
#include "ControlMeasure.h"
#include "BundleAdjust.h"
#include "History.h"
#include "PvlObject.h"
#include "PvlEditDialog.h"
#include "iTime.h"
#include "Application.h"
#include "BundleAdjust.h"

using namespace Qisis;
using namespace std;


namespace Qisis {

  /**
   * Construct the QtieTool
   * 
   * @author 2008-11-21 Tracie Sucharski
   * 
   */
  QtieTool::QtieTool (QWidget *parent) : Qisis::Tool(parent) {

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
    p_serialNumberList = new Isis::SerialNumberList(false);
    p_controlNet = new Isis::ControlNet();
    p_controlNet->SetType(Isis::ControlNet::ImageToGround);
    p_controlNet->SetNetworkId("Qtie");

    p_twist = true;
    p_tolerance = 5;
    p_maxIterations = 10;
    p_ptIdIndex = 0;
    createQtieTool(parent);

  }



  /**
   * Design the QtieTool widget
   * 
   */
  void QtieTool::createQtieTool(QWidget *parent) {

    // Create dialog with a main window
    p_tieTool = new QMainWindow(parent);
    p_tieTool->setWindowTitle("Tie Point Tool");
    p_tieTool->layout()->setSizeConstraint(QLayout::SetFixedSize);

    createMenus();

    // Place everything in a grid
    QGridLayout *gridLayout = new QGridLayout();
    //gridLayout->setSizeConstraint(QLayout::SetFixedSize);
    //  ???  Very tacky-hardcoded to ChipViewport size of ControlPointEdit + xtra.
    //       Is there a better way to do this?
    gridLayout->setColumnMinimumWidth(0,310);
    gridLayout->setColumnMinimumWidth(1,310);
    //  grid row
    int row = 0;

    QCheckBox *twist = new QCheckBox("Twist");
    twist->setChecked(p_twist);
    connect(twist,SIGNAL(toggled(bool)),this,SLOT(setTwist(bool)));
    QLabel *iterationLabel = new QLabel("Maximum Iterations");
    QSpinBox *iteration = new QSpinBox();
    iteration->setRange(1,100);
    iteration->setValue(p_maxIterations);
    iterationLabel->setBuddy(iteration);
    connect(iteration,SIGNAL(valueChanged(int)),this,SLOT(setIterations(int)));
    QHBoxLayout *itLayout = new QHBoxLayout();
    itLayout->addWidget(iterationLabel);
    itLayout->addWidget(iteration);


    QLabel *tolLabel = new QLabel("Tolerance");
    p_tolValue = new QLineEdit();
    tolLabel->setBuddy(p_tolValue);
    QHBoxLayout *tolLayout = new QHBoxLayout();
    tolLayout->addWidget(tolLabel);
    tolLayout->addWidget(p_tolValue);
//    QDoubleValidator *dval = new QDoubleValidator
    QString tolStr;
    tolStr.setNum(p_tolerance);
    p_tolValue->setText(tolStr);
//    connect(p_tolValue,SIGNAL(valueChanged()),this,SLOT(setTolerance()));

    QHBoxLayout *options = new QHBoxLayout();
    options->addWidget(twist);
    options->addLayout(itLayout);
    options->addLayout(tolLayout);

    gridLayout->addLayout(options,row++,0);

    p_pointEditor = new Qisis::ControlPointEdit(parent,true);
    gridLayout->addWidget(p_pointEditor,row++,0,1,3);
    connect(p_pointEditor,SIGNAL(pointSaved()),this,SLOT(pointSaved()));
    p_pointEditor->show();

    QPushButton *solve = new QPushButton("Solve");
    connect(solve,SIGNAL(clicked()),this,SLOT(solve()));
    gridLayout->addWidget(solve,row++,0);

    QWidget *cw = new QWidget();
    cw->setLayout(gridLayout);
    p_tieTool->setCentralWidget(cw);

    connect(this,SIGNAL(editPointChanged()),this,SLOT(drawMeasuresOnViewports()));
    connect(this,SIGNAL(newSolution(Isis::Table *)),
            this,SLOT(writeNewCmatrix(Isis::Table *)));

  }


  /**
   * Create the menus for QtieTool
   * 
   * 
   */
  void QtieTool::createMenus () {


    QAction *saveNet = new QAction(p_tieTool);
    saveNet->setText("Save Control Network &As...");
    QString whatsThis =
      "<b>Function:</b> Saves the current <i>control network</i> under chosen filename";
    saveNet->setWhatsThis(whatsThis);
    connect (saveNet,SIGNAL(activated()),this,SLOT(saveNet()));

    QAction *closeQtieTool = new QAction(p_tieTool);
    closeQtieTool->setText("&Close");
    closeQtieTool->setShortcut(Qt::ALT+Qt::Key_F4);
    whatsThis =
      "<b>Function:</b> Closes the Qtie Tool window for this point \
       <p><b>Shortcut:</b> Alt+F4 </p>";
    closeQtieTool->setWhatsThis(whatsThis);
    connect (closeQtieTool,SIGNAL(activated()),p_tieTool,SLOT(close()));

    QMenu *fileMenu = p_tieTool->menuBar()->addMenu("&File");
    fileMenu->addAction(saveNet);
    fileMenu->addAction(closeQtieTool);

    QAction *templateFile = new QAction(p_tieTool);
    templateFile->setText("&Set registration template");
    whatsThis = 
      "<b>Function:</b> Allows user to select a new file to set as the registration template";
    templateFile->setWhatsThis(whatsThis);
    connect (templateFile,SIGNAL(activated()),this,SLOT(setTemplateFile()));

    QAction *viewTemplate = new QAction(p_tieTool);
    viewTemplate->setText("&View/edit registration template");
    whatsThis = 
      "<b>Function:</b> Displays the curent registration template.  \
       The user may edit and save changes under a chosen filename.";
    viewTemplate->setWhatsThis(whatsThis);
    connect (viewTemplate,SIGNAL(activated()),this,SLOT(viewTemplateFile()));


    QMenu *optionMenu = p_tieTool->menuBar()->addMenu("&Options");
    QMenu *regMenu = optionMenu->addMenu("&Registration");

    regMenu->addAction(templateFile);
    regMenu->addAction(viewTemplate);
    //    registrationMenu->addAction(interestOp);

  }



  /**
   * Put the QtieTool icon on the main window Toolpad
   * 
   * @param pad   Input  Toolpad for the main window
   * 
   */

  QAction *QtieTool::toolPadAction(ToolPad *pad) {
    QAction *action = new QAction(pad);
    action->setIcon(QPixmap(toolIconDir()+"/stock_draw-connector-with-arrows.png"));
    action->setToolTip("Tie (T)");
    action->setShortcut(Qt::Key_T);
    return action;
  }




  /**
   * Setup the base map and match cube
   * 
   * @param baseCube   Input  Base map cube
   * @param matchCube  Input  Match cube
   * 
   */
  void QtieTool::setFiles(Isis::Cube &baseCube,Isis::Cube &matchCube) {
    //  Save off base map cube, but add matchCube to serial number list
    p_baseCube = &baseCube;
    p_matchCube = &matchCube;
    p_baseSN = Isis::SerialNumber::Compose(*p_baseCube,true);
    p_matchSN = Isis::SerialNumber::Compose(*p_matchCube);

    p_serialNumberList->Add(matchCube.Filename());

    //  Save off universal ground maps
    try {
      p_baseGM = new Isis::UniversalGroundMap(*p_baseCube);
    } catch (Isis::iException &e) {
      QString message = "Cannot initialize universal ground map for basemap.\n";
      string errors = e.Errors();
      message += errors.c_str();
      e.Clear();
      QMessageBox::critical((QWidget *)parent(),"Error",message);
      return;
    }
    try {
      p_matchGM = new Isis::UniversalGroundMap(matchCube);
    } catch (Isis::iException &e) {
      QString message = "Cannot initialize universal ground map for match cube.\n";
      string errors = e.Errors();
      message += errors.c_str();
      e.Clear();
      QMessageBox::critical((QWidget *)parent(),"Error",message);
      return;
    }

  }



  /**
   * New files selected, clean up old file info
   * 
   * @internal
   * @history  2007-06-12 Tracie Sucharski - Added method to allow user to  
   *                           run on new files. 
   */
  void QtieTool::clearFiles() {
    p_tieTool->setShown(false);
  //  delete p_baseCube;
  //  delete p_matchCube;

    delete p_serialNumberList;
    p_serialNumberList = new Isis::SerialNumberList(false);

    delete p_controlNet;
    p_controlNet = new Isis::ControlNet();
    p_controlNet->SetType(Isis::ControlNet::ImageToGround);
    p_controlNet->SetNetworkId("Qtie");

    delete p_baseGM;
    delete p_matchGM;

  }



  /**
   * Save control point under crosshairs of ChipViewports
   * 
   */
  void QtieTool::pointSaved() {
    //  Get sample/line from base map and find lat/lon
    double samp = (*p_controlPoint)[Base].Sample();
    double line = (*p_controlPoint)[Base].Line();

    p_baseGM->SetImage(samp,line);
    double lat = p_baseGM->UniversalLatitude();
    double lon = p_baseGM->UniversalLongitude();
    double radius = p_baseGM->Projection()->LocalRadius();

    p_controlPoint->SetUniversalGround(lat,lon,radius);

    emit editPointChanged();

  }



  /**
   * Handle mouse events on match CubeViewport
   * 
   * @param p[in]   (QPoint)            Point under cursor in cubeviewport
   * @param s[in]   (Qt::MouseButton)   Which mouse button was pressed
   * 
   * @internal
   * @history  2007-06-12 Tracie Sucharski - Swapped left and right mouse
   *                           button actions.
   * @history  2008-11-19 Tracie Sucharski - Only allow mouse events on 
   *                           match cube. 
   *  
   */
  void QtieTool::mouseButtonRelease(QPoint p, Qt::MouseButton s) {
    CubeViewport *cvp = cubeViewport();
    if (cvp  == NULL) return;
    if (cubeViewportList()->size() != 2) {
      QString message = "You must have a basemap and a match cube open.";
      QMessageBox::critical((QWidget *)parent(),"Error",message);
      return;
    }
    if (cvp->cube() == p_baseCube) {
      QString message = "Select points on match Cube only.";
      QMessageBox::information((QWidget *)parent(),"Warning",message);
      return;
    }
    if (cvp->cursorInside()) QPoint p = cvp->cursorPosition();

    // ???  do we only allow mouse clicks on level1???
    //    If we allow on both, need to find samp,line on level1 if
    //    they clicked on basemap.
    std::string file = cvp->cube()->Filename();
    std::string sn = p_serialNumberList->SerialNumber(file);

    double samp,line;
    cvp->viewportToCube(p.x(),p.y(),samp,line);

    if (s == Qt::LeftButton) {
      //  Find closest control point in network
      Isis::ControlPoint *point =
      p_controlNet->FindClosest(sn,samp,line);
      //  TODO:  test for errors and reality 
      if (point == NULL) {
        QString message = "No points exist for editing.  Create points ";
        message += "using the right mouse button.";
        QMessageBox::information((QWidget *)parent(),"Warning",message);
        return;
      }
      modifyPoint(point);
    }
    else if (s == Qt::MidButton) {
      //  Find closest control point in network
      Isis::ControlPoint *point =
      p_controlNet->FindClosest(sn,samp,line);
      //  TODO:  test for errors and reality 
      if (point == NULL) {
        QString message = "No points exist for deleting.  Create points ";
        message += "using the right mouse button.";
        QMessageBox::information((QWidget *)parent(),"Warning",message);
        return;
      }
      deletePoint(point);
    }
    else if (s == Qt::RightButton) {
      p_matchGM->SetImage(samp,line);
      double lat = p_matchGM->UniversalLatitude();
      double lon = p_matchGM->UniversalLongitude();

      createPoint(lat,lon);
    }
  }



  /**
   * Create control point at given lat,lon
   * 
   * @param lat   Input  Latitude of new point
   * @param lon   Input  Longitude of new point
   *  
   * @internal 
   * @history  2008-12-06 Tracie Sucharski - Set point type to Ground
   */
  void QtieTool::createPoint(double lat,double lon) {

    //  TODO:   ADD AUTOSEED OPTION (CHECKBOX?)

    double baseSamp=0,baseLine=0;
    double matchSamp,matchLine;

    //  if clicked in match, get samp,line
    p_matchGM->SetUniversalGround(lat,lon);
    matchSamp = p_matchGM->Sample();
    matchLine = p_matchGM->Line();

    //  Make sure point is on base 
    if (p_baseGM->SetUniversalGround(lat,lon)) {
      //  Make sure point on base cube
      baseSamp = p_baseGM->Sample();
      baseLine = p_baseGM->Line();
      if (baseSamp < 1 || baseSamp > p_baseCube->Samples() ||
          baseLine < 1 || baseLine > p_baseCube->Lines()) {
        // throw error? point not on base
        QString message = "Point does not exist on base map.";
        QMessageBox::warning((QWidget *)parent(),"Warning",message);
      }
    }
    else {
      //  throw error?  point not on base cube
      // throw error? point not on base
      QString message = "Point does not exist on base map.";
      QMessageBox::warning((QWidget *)parent(),"Warning",message);
    }

    //  Get radius
    double radius = p_baseGM->Projection()->LocalRadius();

    //  Point is on both base and match, create new control point
    std::string ptId = "Point" + Isis::iString(++p_ptIdIndex);
    Isis::ControlPoint *newPoint = new Isis::ControlPoint(ptId);
    newPoint->SetUniversalGround(lat,lon,radius);
    newPoint->SetType(Isis::ControlPoint::Ground);

    // Set first measure to base measure and set to Ignore=yes
    Isis::ControlMeasure *mB = new Isis::ControlMeasure;
    mB->SetCubeSerialNumber(p_baseSN);
    mB->SetCoordinate(baseSamp,baseLine);
    mB->SetType(Isis::ControlMeasure::Estimated);
    mB->SetDateTime();
    mB->SetChooserName();
    mB->SetIgnore(true);
    newPoint->Add(*mB);

    Isis::ControlMeasure *mM = new Isis::ControlMeasure;
    mM->SetCubeSerialNumber(p_matchSN);
    mM->SetCoordinate(matchSamp,matchLine);
    mM->SetType(Isis::ControlMeasure::Estimated);
    mM->SetDateTime();
    mM->SetChooserName();
    newPoint->Add(*mM);

    //  Add new control point to control network
    p_controlNet->Add(*newPoint);
    //  Read newly added point
    p_controlPoint = p_controlNet->Find(ptId);
    //  Load new point in QtieTool
    loadPoint();
    p_tieTool->setShown(true);
    p_tieTool->raise();

    emit editPointChanged();

  }



  /**
   * Delete given control point
   * 
   * @param point Input  Control Point to delete
   * 
   */
  void QtieTool::deletePoint(Isis::ControlPoint *point) {

    p_controlPoint = point;
    //  Change point in viewport to red so user can see what point they are 
    //  about to delete.
    emit editPointChanged();

    loadPoint();

    p_controlNet->Delete(p_controlPoint->Id());
    p_tieTool->setShown(false);

    emit editPointChanged();
  }



  /**
   * Modify given control point
   * 
   * @param point Input  Control Point to modify
   * 
   */
  void QtieTool::modifyPoint(Isis::ControlPoint *point) {

    p_controlPoint = point;
    loadPoint();
    p_tieTool->setShown(true);
    p_tieTool->raise();
    emit editPointChanged();
  }


  /**
   * Load control point into the ControlPointEdit widget
   * 
   */
  void QtieTool::loadPoint () {

    //  Initialize pointEditor with measures
    p_pointEditor->setLeftMeasure (&(*p_controlPoint)[Base],p_baseCube,
                                   p_controlPoint->Id());
    p_pointEditor->setRightMeasure (&(*p_controlPoint)[Match],p_matchCube,
                                   p_controlPoint->Id());
  }



  /**
   * Draw all Control Measures on each viewport
   * 
   */
  void QtieTool::drawMeasuresOnViewports () {

    CubeViewport *vp;
    for (int i=0; i<(int)cubeViewportList()->size(); i++) {
      vp = (*(cubeViewportList()))[i];
      vp->viewport()->update();
    }
  }



  /**
   * Repaint the given CubeViewport
   * 
   * @param vp       Input  CubeViewport to repain 
   * @param painter  Input  Qt Painter
   * 
   */
  void QtieTool::paintViewport (CubeViewport *vp,QPainter *painter) {

    //  Make sure we have points to draw
    if (p_controlNet->Size() == 0) return;
    
    //  Draw all measures
    std::string serialNumber = Isis::SerialNumber::Compose(*vp->cube(),true);
    for (int i=0; i<p_controlNet->Size(); i++) {
      Isis::ControlPoint &p = (*p_controlNet)[i];
      if (p.Id() == p_controlPoint->Id()) {
        painter->setPen(QColor(200,0,0));
      }
      else {
        painter->setPen(QColor(0,200,0));
      }

      double samp,line;
      if (vp->cube()->Filename() == p_baseCube->Filename()) {
        // Draw on left viewport (base)
        samp = p[Base].Sample();
        line = p[Base].Line();
      }
      else {
        // Draw on right viewport (match)
        samp = p[Match].Sample();
        line = p[Match].Line();
      }
      int x,y;
      vp->cubeToViewport(samp,line,x,y);
          painter->drawLine(x-5,y,x+5,y);
          painter->drawLine(x,y-5,x,y+5);
    }

  }




  /**
   * Perform the BundleAdjust Solve 
   * 
   * 
   */
  void QtieTool::solve () {

    //  First off , get tolerance,  NEED to VALIDATE
    p_tolerance = p_tolValue->text().toDouble();

    //  Need at least 2 points to solve for twist
    if (p_twist) {
      if (p_controlNet->Size() < 2) {
        QString message = "Need at least 2 points to solve for twist. \n";
        QMessageBox::critical((QWidget *)parent(),"Error",message);
        return;
      }
    }
    Isis::ControlNet net;

    // Bundle adjust to solve for new pointing
    try {

      //  Create new control net for bundle adjust , deleting ignored measures
      for (int p=0; p<p_controlNet->Size(); p++) {
        Isis::ControlPoint pt = (*p_controlNet)[p];
        for (int m=0; m<pt.Size(); m++) {
          if (pt[m].Ignore()) pt.Delete(m);
        }
        net.Add(pt);
      }

      Isis::BundleAdjust b(net,*p_serialNumberList,false);
      b.SetSolveTwist(p_twist);
      b.Solve(p_tolerance,p_maxIterations);

      // Print results and give user option of updating cube pointin
      double maxError = net.MaximumError();
      double avgError = net.AverageError();

      QString message = "Maximum Error = " + QString::number(maxError);
      message += "\nAverage Error = " + QString::number(avgError);
      QString msgTitle = "Update camera pointing?";
      QMessageBox msgBox(QMessageBox::Question,msgTitle,message,0,p_tieTool);
      QPushButton *update = msgBox.addButton("Update",QMessageBox::AcceptRole);
      QPushButton *close = msgBox.addButton("Close",QMessageBox::RejectRole);
      msgBox.setDefaultButton(close);
      msgBox.exec();
      if (msgBox.clickedButton() == update) {
        p_matchCube->ReOpen("rw");
        Isis::Table cmatrix = b.Cmatrix(0);
        //cmatrix = b.Cmatrix(0);
        emit newSolution(&cmatrix);
      }
      else {
        return;
      }

    }
    catch (Isis::iException &e) {
      QString message = "Bundle Solution failed.\n";
      string errors = e.Errors();
      message += errors.c_str();
      e.Clear();
      message += "\n\nMaximum Error = " + QString::number(net.MaximumError());
      message += "\nAverage Error = " + QString::number(net.AverageError());
      QMessageBox::warning((QWidget *)parent(),"Error",message);
      return;
    }
  }




  /**
   * Write the new cmatrix to the match cube
   * 
   * @param cmatrix    Input  New adjusted cmatrix
   * 
   */
  void QtieTool::writeNewCmatrix (Isis::Table *cmatrix) {

    //check for existing polygon, if exists delete it
    if (p_matchCube->Label()->HasObject("Polygon")) {
      p_matchCube->Label()->DeleteObject("Polygon");
    }

    // Update the cube history
    p_matchCube->Write(*cmatrix);
    Isis::History h("IsisCube");
    try {
      p_matchCube->Read(h);
    } catch (Isis::iException &e) {
      QString message = "Could not read cube history, will not update history.\n";
      string errors = e.Errors();
      message += errors.c_str();
      QMessageBox::warning((QWidget *)parent(),"Warning",message);
      e.Clear();
      return;
    }
    Isis::PvlObject history("qtie");
    history += Isis::PvlKeyword("IsisVersion",Isis::version);
    QString path = QCoreApplication::applicationDirPath();
    history += Isis::PvlKeyword("ProgramPath", path);
    history += Isis::PvlKeyword("ExecutionDateTime",Isis::Application::DateTime());
    history += Isis::PvlKeyword("HostName",Isis::Application::HostName());
    history += Isis::PvlKeyword("UserName",Isis::Application::UserName());
    Isis::PvlGroup results("Results");
    results += Isis::PvlKeyword("CameraAnglesUpdated","True");
    results += Isis::PvlKeyword("BaseMap",p_baseCube->Filename());
    history += results;

    h.AddEntry(history);
    p_matchCube->Write(h);
    p_matchCube->ReOpen("r");

  }




  /**
   * Allows user to set a new template file.
   * @author 2008-12-10 Jeannie Walldren 
   * @internal 
   *   @history 2008-12-10 Jeannie Walldren - Original Version
   */

  void QtieTool::setTemplateFile() {
    p_pointEditor->setTemplateFile();
  }




  /**
   * Allows the user to view the template file that is currently 
   * set. 
   * @author 2008-12-10 Jeannie Walldren 
   * @internal 
   *   @history 2008-12-10 Jeannie Walldren - Original Version
   *   @history 2008-12-10 Jeannie Walldren - Added "Isis::"
   *            namespace to PvlEditDialog reference and changed
   *            registrationDialog from pointer to object
   *   @history 2008-12-15 Jeannie Walldren - Added QMessageBox
   *            warning in case Template File cannot be read.
   */
  void QtieTool::viewTemplateFile() {
    try{
      // Get the template file from the ControlPointEditor object
      Isis::Pvl templatePvl(p_pointEditor->templateFilename());
      // Create registration dialog window using PvlEditDialog class
      // to view and/or edit the template
      Isis::PvlEditDialog registrationDialog(templatePvl);
      registrationDialog.setWindowTitle("View or Edit Template File: " 
                                         + QString::fromStdString(templatePvl.Filename()));
      registrationDialog.resize(550,360);
      registrationDialog.exec();
    }
    catch (Isis::iException &e){
      QString message = e.Errors().c_str();
      e.Clear ();
      QMessageBox::warning((QWidget *)parent(),"Error",message);
    }
  }



  /**
   * Save the ground points to a ControlNet. 
   *  
   * @author 2008-12-30 Tracie Sucharski
   */
  void QtieTool::saveNet () {

    QString filter = "Control net (*.net);;";
    filter += "Text file (*.txt);;";
    filter += "All (*)";
    QString fn=QFileDialog::getSaveFileName((QWidget*)parent(),
                                            "Choose filename to save under",
                                            ".", filter);
    if ( !fn.isEmpty() ) {
      try {
        //  Create new control net for bundle adjust , deleting ignored measures
        // which are the basemap measures.
        Isis::ControlNet net;
        for (int p=0; p<p_controlNet->Size(); p++) {
          Isis::ControlPoint pt = (*p_controlNet)[p];
          for (int m=0; m<pt.Size(); m++) {
            if (pt[m].Ignore()) pt.Delete(m);
          }
          net.SetType(Isis::ControlNet::ImageToGround);
          net.SetTarget(p_matchCube->Camera()->Target());
          net.SetNetworkId("Qtie");
          net.SetUserName(Isis::Application::UserName());
          net.SetCreatedDate( Isis::Application::DateTime() );
          net.SetModifiedDate( Isis::iTime::CurrentLocalTime() );
          net.SetDescription("Qtie Ground Points");
          net.Add(pt);
        }

        net.Write(fn.toStdString());
      } 
      catch (Isis::iException &e) {
        QString message = "Error saving control network.  \n";
        string errors = e.Errors();
        message += errors.c_str();
        e.Clear ();
        QMessageBox::information((QWidget *)parent(),"Error",message);
        return;
      }
    }
    else {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Saving Aborted");
    }

  }

}
