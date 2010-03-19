#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QLabel>
#include <QValidator>
#include <QHBoxLayout>
#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QMessageBox>

#include "FindTool.h"
#include "MainWindow.h"
#include "CubeViewport.h"
#include "Workspace.h"
#include "Projection.h"

namespace Qisis {
  /**
   * Constructs a FindTool object
   * 
   * @param parent 
   */
  FindTool::FindTool (QWidget *parent) : Qisis::Tool(parent) {
    //set these to null so that we can delete the right ones in the destructor
    p_dialog = NULL;
    p_findPoint = NULL;
    p_findPtButton = NULL;
    p_linkButton = NULL;
    p_syncScale = NULL;
    p_status = NULL;
    p_tabWidget = NULL;
    p_groundTab = NULL;
    p_imageTab = NULL;

    p_lat = "";
    p_lon = "";
    p_samp = "";
    p_line = "";
    p_pressed = false;
    p_released = true;
    p_paint = false;
    //Set up dialog box
    createDialog(parent);

    // Set up find point action
    p_findPoint = new QAction (parent);
    p_findPoint->setShortcut(Qt::CTRL+Qt::Key_F);
    p_findPoint->setText("&Find Point");
    p_findPoint->setIcon(QPixmap(toolIconDir()+"/find.png"));
    p_findPoint->setEnabled(false);

    p_pen.setColor(Qt::red);
    p_pen.setWidth(3);
    p_pen.setStyle(Qt::SolidLine);

  }

  FindTool::~FindTool() {
    delete p_groundTab->p_latLineEdit->validator();
    delete p_groundTab->p_lonLineEdit->validator();
    /*delete p_findPoint;
    if(p_dialog != NULL) delete p_dialog;
    if(p_findPoint != NULL) delete p_findPoint;
    if(p_findPtButton != NULL) delete p_findPtButton;
    if(p_linkButton != NULL) delete p_linkButton;
    if(p_syncScale != NULL) delete p_syncScale;
    if(p_status != NULL) delete p_status;
    if(p_tabWidget != NULL) delete p_tabWidget;
    if(p_groundTab != NULL) delete p_groundTab;
    if(p_imageTab != NULL) delete p_imageTab;*/

  }

  /**
   * Creates the dialog used by this tool
   * 
   * @param parent 
   */
  void FindTool::createDialog(QWidget *parent) {
    
    p_dialog = new QDialog(parent);
    p_tabWidget = new QTabWidget(p_dialog);
    p_dialog->setWindowTitle("Find Latitude/Longitude Coordinate");

    p_groundTab = new GroundTab();
    p_imageTab = new ImageTab();
    p_tabWidget->addTab(p_imageTab, "Image");
    p_tabWidget->addTab(p_groundTab, "Ground");

    // Create the action buttons
    QPushButton *okButton = new QPushButton ("Ok");
    connect(okButton, SIGNAL(clicked()), this, SLOT(okAction()));

    QPushButton *recordButton = new QPushButton ("Record Point");
    connect(recordButton, SIGNAL(clicked()), this, SLOT(recordAction()));//?????

    QPushButton *clearButton = new QPushButton("Clear Dot");
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearPoint()));

    QPushButton* cancelButton = new QPushButton ("Cancel");
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(clearPoint()));
    connect(cancelButton, SIGNAL(clicked()), p_dialog, SLOT(hide()));

    // Put the buttons in a horizontal orientation
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->addWidget(okButton);
    actionLayout->addWidget(recordButton);
    actionLayout->addWidget(clearButton);
    actionLayout->addWidget(cancelButton);

    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialogLayout->addWidget(p_tabWidget);
    dialogLayout->addLayout(actionLayout); 
    p_dialog->setLayout(dialogLayout);
       
  }

  /**
   * The ground tab used by the dialog in the FindTool
   * 
   * @param parent 
   */
  GroundTab::GroundTab(QWidget *parent) : QWidget(parent){

    p_latLineEdit = new QLineEdit();
    p_latLineEdit->setText("0");
    p_latLineEdit->setValidator(new QDoubleValidator(-90.0,90.0,99,parent));
    p_lonLineEdit = new QLineEdit();
    p_lonLineEdit->setText("0");
    p_lonLineEdit->setValidator(new QDoubleValidator(parent));
    QLabel *latLabel = new QLabel("Latitude");
    QLabel *lonLabel = new QLabel("Longitude");

    // Put the buttons and text field in a gridlayout
    QGridLayout *gridLayout = new QGridLayout ();
    gridLayout->addWidget(latLabel,0,0);
    gridLayout->addWidget(p_latLineEdit,0,1);
    gridLayout->addWidget(lonLabel,1,0);
    gridLayout->addWidget(p_lonLineEdit, 1, 1);
    setLayout(gridLayout);

  }

  /**
   * The image tab used by the dialog in the FindTool
   * 
   * @param parent 
   */
  ImageTab::ImageTab(QWidget *parent) : QWidget(parent){
    p_sampLineEdit = new QLineEdit();
    p_sampLineEdit->setText("1");
    p_lineLineEdit = new QLineEdit();
    p_lineLineEdit->setText("1");
    QLabel *sampleLabel = new QLabel("Sample");
    QLabel *lineLabel = new QLabel("Line");

    // Put the buttons and text field in a gridlayout
    QGridLayout *gridLayout = new QGridLayout ();
    gridLayout->addWidget(sampleLabel,0,0);
    gridLayout->addWidget(p_sampLineEdit,0,1);
    gridLayout->addWidget(lineLabel,1,0);
    gridLayout->addWidget(p_lineLineEdit, 1, 1);
    setLayout(gridLayout);

  }

  /**
   * Adds the find tool to the toolpad
   * 
   * @param toolpad 
   * 
   * @return QAction* 
   */
  QAction *FindTool::toolPadAction(ToolPad *toolpad) {
    QAction *action = new QAction(toolpad);
    action->setIcon(QPixmap(toolIconDir()+"/find.png"));
    action->setToolTip("Find (F)");
    action->setShortcut(Qt::Key_F);
    QString text  =
      "<b>Function:</b>  Find a lat/lon or line/sample coordinate in this cube. \
      <p><b>Shortcut:</b>F</p> ";
    action->setWhatsThis(text);
    return action;
  }

  /**
   * Adds the find tool to the menu
   * 
   * @param menu 
   */
  void FindTool::addTo(QMenu *menu) {
    menu->addAction(p_findPoint);
  }

  /**
   * Creates the tool bar for the find tool
   * 
   * @param parent 
   * 
   * @return QWidget* 
   */
  QWidget *FindTool::createToolBarWidget (QStackedWidget *parent) {
    QWidget *hbox = new QWidget(parent);

    p_findPtButton= new QToolButton(hbox);
    p_findPtButton->setIcon(QPixmap(toolIconDir()+"/find.png"));
    p_findPtButton->setToolTip("Find Point");
    QString text =
     "<b>Function:</b> Centers all linked viewports to the selected lat/lon. \
      The user can click anywhere on the image to have that point centered, or \
      they can use the shortcut or button to bring up a window that they can \
      enter a specific lat/lon position into. \
      <p><b>Shortcut: </b> Ctrl+F </p> \
      <p><b>Hint: </b> This option will only work if the image has a camera \
            model or is projected, and will only center the point on images  \
            where the selected lat/lon position exists.</p>";
    p_findPtButton->setWhatsThis(text);
    connect(p_findPtButton,SIGNAL(clicked()),p_dialog,SLOT(show()));
    p_findPtButton->setAutoRaise(true);
    p_findPtButton->setIconSize(QSize(22,22));

    p_syncScale = new QCheckBox("Sync Scale");
    p_syncScale->setToolTip("Synchronize Scale");
    text = "<b>Function:</b> Syncronizes the scale of all linked viewports.";
    p_syncScale->setWhatsThis(text);

    p_linkButton = new QToolButton(hbox);
    p_linkButton->setIcon(QPixmap(toolIconDir()+"/link_valid.png"));
    p_linkButton->setToolTip("Link Valid Viewports");
    p_linkButton->setWhatsThis("<b>Function: </b> Links all open images that have\
                              a camera model or are map projections");
    connect(p_linkButton,SIGNAL(clicked()),this,SLOT(linkValid()));
    p_linkButton->setAutoRaise(true);
    p_linkButton->setIconSize(QSize(22,22));

    p_status = new QLineEdit();
    p_status->setReadOnly(true); 
    p_status->setToolTip("Cube Type");
    p_status->setWhatsThis("<b>Function: </b> Displays whether the active cube \
                           is a camera model, projection, both, or none. <p> \
                           <b>Hint: </b> If the cube is 'None' the find tool \
                           will not be active</p>"); 

    QHBoxLayout *layout = new QHBoxLayout(hbox);
    layout->setMargin(0);
    layout->addWidget(p_status);
    layout->addWidget(p_findPtButton);
    layout->addWidget(p_linkButton);
    layout->addWidget(p_syncScale);
    layout->addStretch(1);
    hbox->setLayout(layout);

    return hbox;
  }

  /**
   * Overriden method to update this tool
   * 
   */
  void FindTool::updateTool() {
    if (cubeViewport() == NULL) {
      p_linkButton->setEnabled(false);
      p_findPoint->setEnabled(false);
      p_findPtButton->setEnabled(false);
      p_syncScale->setEnabled(false);
      p_status->setText("None");
      if(p_dialog->isVisible()) {
        p_dialog->close();
      }
    }
    else { 
      /*if (cubeViewportList()->size() > 1) {
        p_linkButton->setEnabled(true);
        p_syncScale->setEnabled(true);
      }
      else {
        p_linkButton->setEnabled(false);
        p_syncScale->setEnabled(false);
      }*/

      // Update Status Text Field
      if (cubeViewport()->universalGroundMap() == NULL) {
        p_status->setText("None");
        p_findPoint->setEnabled(false);
        p_findPtButton->setEnabled(false);
        p_syncScale->setEnabled(false);
        //if (cubeViewportList()->size() > 1) {
        //  p_linkButton->setEnabled(true);
        //}
        //else {
          p_linkButton->setEnabled(false);
        //}
        if(p_dialog->isVisible()) {
          p_dialog->close();
        }
      }
      else {
        if (cubeViewportList()->size() > 1) {
          p_linkButton->setEnabled(true);
          p_syncScale->setEnabled(true);
        }
        else {
          p_linkButton->setEnabled(false);
          p_syncScale->setEnabled(false);
        }
        if (cubeViewport()->camera() != NULL) {
          p_findPoint->setEnabled(true);
          p_findPtButton->setEnabled(true);

          if (cubeViewport()->camera()->HasProjection()) {
            p_status->setText("Both");
          }
          else {
            p_status->setText("Camera");
          }
        }
        else {
          p_status->setText("Projection");
          p_findPoint->setEnabled(true);
          p_findPtButton->setEnabled(true);
        }
      }
    }
  }

  /**
   * Slot called when the okay button is clicked
   * 
   */
  void FindTool::okAction() {

    if (p_tabWidget->tabText(p_tabWidget->currentIndex()) == "Ground") {
      getUserGroundPoint();
    } else if (p_tabWidget->tabText(p_tabWidget->currentIndex())== "Image") {
      getUserImagePoint();
    }
  }

  /** 
   * Slot called when the record button is clicked.  It creates a
   * QPoint from the current line/sample in the active cube 
   * viewport and emits the recordPoint() signal. 
   *  
   * @return void 
   * @author Jeannie Walldren 
   *  
   * @internal 
   *  @history 2010-03-08 - Jeannie Walldren - This slot was
   *           created to connect the recordPoint() signal to the
   *           AdvancedTrackTool record() slot in qview.
   */ 
  void FindTool::recordAction() { 
    if (p_lat != NULL && p_lon != NULL &&
        p_samp != NULL && p_line != NULL ){

      int x,y;
      CubeViewport *cvp = cubeViewport();
      cvp->cubeToViewport(p_samp.toDouble(),p_line.toDouble(), x, y);
      QPoint p;
      p.setX(x);
      p.setY(y);
      emit recordPoint(p); 
    }
  }


  /** 
   *  Gets the lat/lon position the user entered from the dialog box
   *  
   *  @history 2009-11-10 Tracie Sucharski - Changed emit findPoint to
   *                         simply a call to findPoint method.  There was
   *                         no findPoint signal defined.
   *  
   */
  void FindTool::getUserGroundPoint() {
    // Get the users input
    CubeViewport *d = cubeViewport();
    if (d == NULL) return;

    //Validate latitude value
    QString latitude = p_groundTab->p_latLineEdit->text();
    int cursorPos = 0;
    QValidator::State validLat = 
      p_groundTab->p_latLineEdit->validator()->validate(latitude,cursorPos);
    if (validLat != QValidator::Acceptable) {
      QMessageBox::warning(p_dialog, "Error",
                "Latitude value must be in the range -90 to 90",
                QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
      return;
    }

    //Validate longitude value
    QString longitude = p_groundTab->p_lonLineEdit->text();
    QValidator::State validLon = 
      p_groundTab->p_lonLineEdit->validator()->validate(longitude,cursorPos);
    if (validLon != QValidator::Acceptable) {
      QMessageBox::warning(p_dialog, "Error",
                "Longitude value must be a double",
                QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
      return;
    }

    double lat = Isis::iString(latitude.toStdString()).ToDouble();
    double lon = Isis::iString(longitude.toStdString()).ToDouble();

    if (d->universalGroundMap()->SetUniversalGround(lat,lon) &&
        d->universalGroundMap()->Sample() > 0 && d->universalGroundMap()->Sample() <= d->cubeSamples() &&
        d->universalGroundMap()->Line() > 0 && d->universalGroundMap()->Line() <= d->cubeLines()) {
      double sample = d->universalGroundMap()->Sample();
      double line = d->universalGroundMap()->Line();
      int x,y;
      d->cubeToViewport(sample,line,x,y);
      findPoint(QPoint(x,y));          

          //jw set private variables???????
    p_lat = QString::number(lat);
    p_lon = QString::number(lon);
    p_samp = QString::number(sample);
    p_line = QString::number(line);
    p_imageTab->p_sampLineEdit->setText(p_samp);
    p_imageTab->p_lineLineEdit->setText(p_line);
//end jw ???????
    }
    else {
      QMessageBox::warning(p_dialog, "Error",
                "Specified Lat/Lon position is not in active viewport",
                QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
    }
  }


  /** 
   *  Gets the line/sample position the user entered from the dialog box
   *  
   *  @history 2009-11-10 Tracie Sucharski - Changed emit findPoint to
   *                         simply a call to findPoint method.  There was
   *                         no findPoint signal defined.  Fixed error checking.
   *  
   */
  void FindTool::getUserImagePoint() {
    CubeViewport *d = cubeViewport();
    if (d == NULL) return;

    double line = p_imageTab->p_lineLineEdit->text().toDouble();
    double sample = p_imageTab->p_sampLineEdit->text().toDouble();
    if(sample > d->cubeSamples() || sample < 1) {
      QMessageBox::warning(p_dialog, "Error",
                "Specified Sample is not in active viewport",
                QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
      return;
    }

    if(line > d->cubeLines() || line < 1) {
      QMessageBox::warning(p_dialog, "Error",
                "Specified Line is not in active viewport",
                QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
      return;
    }

    int x,y;
    d->cubeToViewport(sample,line,x,y);

    findPoint(QPoint(x,y));

    //jw set private variables????????????
    //??? find lat long and add to "Ground" Tab????
    if (d->universalGroundMap()->SetImage(sample,line) ) {
      p_lat = QString::number(d->universalGroundMap()->UniversalLatitude());
      p_lon = QString::number(d->universalGroundMap()->UniversalLongitude());
      p_samp = QString::number(sample);
      p_line = QString::number(line);
      p_groundTab->p_latLineEdit->setText(p_lat);
      p_groundTab->p_lonLineEdit->setText(p_lon);
    }
    else{
      QMessageBox::warning(p_dialog, "Error",
                "Specified Sample/Line position is not in active viewport",
                QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
    }
  }

  /** 
   *  Finds the center of the viewport and centers the same point for the linked
   *  cubes
   * 
   * @param p 
   * 
   */
  void FindTool::findPoint (QPoint p) {
    CubeViewport *d = cubeViewport();
    p_dialog->setCursor(Qt::WaitCursor);

    if (d == NULL) return;

    // Check to make sure that all the linked images have a UniversalGroundMap  
    if (cubeViewport()->isLinked()) {
      std::vector<int> error;
      Isis::iString errorMsg = "The following cubes are not projected and do not "
                              "have a camera model: \n";
      for (int i=0; i<(int)cubeViewportList()->size(); i++) {
        d = (*(cubeViewportList()))[i];
        
        if ((d->isLinked()) && (d->universalGroundMap() == NULL)) {
          errorMsg += "     " + d->cube()->Filename() + "\n";
          error.push_back(i);
        }
      }
      if (!error.empty()) {
        errorMsg += "Do you want this automatically corrected?";
        int reply = QMessageBox::warning(p_dialog, "Error",errorMsg.c_str(),
                                         QMessageBox::Yes,QMessageBox::No,
                                         QMessageBox::NoButton);
        if (reply == QMessageBox::Yes) {
          for (int i=0; i<(int)error.size(); i++) {
            d = (*(cubeViewportList()))[error[i]];
            d->setLinked(false);
          }
        }
        else {
          //p_dialog->hide();
          p_dialog->setCursor(Qt::ArrowCursor);
          return;
        }
      }
    }

    // Get the sample line position to report
    double sample,line;
    d->viewportToCube(p.x(),p.y(),sample,line);

    // If we are outside of the cube, do nothing
    if(!d->universalGroundMap()->SetImage(sample, line)){
      QMessageBox::warning(p_dialog, "Error",
                "Specified Line/Sample position is not in active viewport",
                QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
     return;
    }

    if  (d->universalGroundMap()->SetImage(sample,line)) {
      d->setScale(d->scale(),sample,line);
      p_paint = true;

      int x = 0;
      int y = 0;
      d->cubeToViewport(sample, line, x, y);
      p_point.setX(x);
      p_point.setY(y);
      d->viewport()->repaint();

      if (p_tabWidget->tabText(p_tabWidget->currentIndex()) == "Image") {
        if (d->isLinked()) {
          for (int i=0; i<(int)cubeViewportList()->size(); i++) {
            d = (*(cubeViewportList()))[i];
            if (d == cubeViewport()) continue;
            d->setScale(d->scale(),sample,line);
            x = 0;
            y = 0;
            d->cubeToViewport(sample, line, x, y);
            p_point.setX(x);
            p_point.setY(y);
            d->viewport()->repaint();
          }
        }
      }
      else {
        if (d->isLinked()) {
          // Get the lat/lon and scale values from the universalGroundMap
          double lat = d->universalGroundMap()->UniversalLatitude();
          double lon = d->universalGroundMap()->UniversalLongitude();
          double fixedScale = d->universalGroundMap()->Resolution() * d->scale(); 
  
          for (int i=0; i<(int)cubeViewportList()->size(); i++) {
            d = (*(cubeViewportList()))[i];
            if (d == cubeViewport()) continue;
            if (d->isLinked()) {
  
              // Make sure the lat/lon exists in the image & set it to the center
            if (d->universalGroundMap()->SetUniversalGround(lat,lon) &&
                d->universalGroundMap()->Sample() > 0 && d->universalGroundMap()->Sample() <= d->cubeSamples() &&
                d->universalGroundMap()->Line() > 0 && d->universalGroundMap()->Line() <= d->cubeLines()) {
  
                sample = d->universalGroundMap()->Sample();
                line = d->universalGroundMap()->Line();
                double scale = d->scale();
  
                // If sync scale is checked, recalculate the scale
                if (p_syncScale->isChecked()) {
                  scale = fixedScale / d->universalGroundMap()->Resolution();
                }
                d->setScale(scale,sample,line);
                d->cubeToViewport(sample, line, x, y);
                p_point.setX(x);
                p_point.setY(y);
                d->viewport()->repaint();
              }
              else {
                QApplication::beep();
              }
            } // end if d->isLinked
          } //end for loop
        }
        p_paint = false;
      } 
    }

    p_dialog->setCursor(Qt::ArrowCursor);
  }

  /** 
   *  Centers the lat/lon value clicked on in all linked windows
   * 
   * @param p 
   * @param s 
   */
  void FindTool::mouseButtonRelease(QPoint p, Qt::MouseButton s) {
    /*if (s == Qt::MidButton) {
      if (cubeViewport()->universalGroundMap() == NULL) {
        QApplication::beep();
      }
      else {
        //findPoint(p);
        //mouseMove(p);
      }
    }*/
    p_pressed = false;
  }

  /**
   * Called when a mouse button is pressed
   * 
   * @param p 
   * @param s 
   */
  void FindTool::mouseButtonPress(QPoint p, Qt::MouseButton s) {
    if(cubeViewport()->universalGroundMap() == NULL) return;
    p_pressed = true;
    p_point = p;
    mouseMove(p);
  }

  /** 
   * This is the 'findPoint(QPoint p)' of the mouse actions. 
   *  
   * A red dot is drawn on all linked viewports when the user 
   * clicks the left mouse button.  The red dot follows the cursor 
   * if the user holds down the left mouse button. 
   * In order to draw the red dot in the same location (lat,lon) 
   * on the linked cvps as the user is clicking on in the active 
   * cvp, we must convert the active cvp's x,y to lat lon.  Then 
   * use that lat,lon in the linked cvps to convert to line,sample 
   * and finally back to the viewport x,y. 
   * 
   * @param p
   */
  void FindTool::mouseMove(QPoint p) {
    CubeViewport *d = cubeViewport();
    int x,y;
    if(p_pressed) {
        double sample, line;
        p_point = p;
        d->viewportToCube(p_point.x(),p_point.y(),sample,line);
        p_paint = true;
        d->setScale(d->scale(), sample, line);

        d->cubeToViewport(sample,line, x, y);
        p_point.setX((int)x);
        p_point.setY((int)y);
        d->viewport()->repaint();


        //////???????jw
        double lat=0, lon=0;
        if (d->universalGroundMap() != NULL) {
          if (d->universalGroundMap()->SetImage(sample,line)) {
            lat = d->universalGroundMap()->UniversalLatitude();
            lon = d->universalGroundMap()->UniversalLongitude();
            p_lat = QString::number(lat);
            p_lon = QString::number(lon);
            p_samp = QString::number(sample);
            p_line = QString::number(line);
            p_imageTab->p_sampLineEdit->setText(p_samp);
            p_imageTab->p_lineLineEdit->setText(p_line);
            p_groundTab->p_latLineEdit->setText(p_lat);
            p_groundTab->p_lonLineEdit->setText(p_lon);

          }
        }


        /* now convert this lat lon to viewport coordinates for the paintViewport
           method to use.*/
        if(d->isLinked()) {
          /*convert the line,sample to lat,lon*/
          //double lat=0, lon=0;
          double fixedScale = d->universalGroundMap()->Resolution() * d->scale();

          if (d->universalGroundMap() != NULL) {
            if (d->universalGroundMap()->SetImage(sample,line)) {
              lat = d->universalGroundMap()->UniversalLatitude();
              lon = d->universalGroundMap()->UniversalLongitude();
            }
          }

          for (int i=0; i < (int)cubeViewportList()->size(); i++) {
            d = (*(cubeViewportList()))[i];
            if(d == cubeViewport()) continue;
            if(d->isLinked()) { 
              if(d->universalGroundMap() != NULL) {
                /*convert the lat,lon to line,sample of the linked cvp */
                if (d->universalGroundMap()->SetUniversalGround(lat,lon) &&
                  d->universalGroundMap()->Sample() > 0 && d->universalGroundMap()->Sample() <= d->cubeSamples() &&
                  d->universalGroundMap()->Line() > 0 && d->universalGroundMap()->Line() <= d->cubeLines()) {
                  sample = d->universalGroundMap()->Sample();
                  line = d->universalGroundMap()->Line();

                  double scale = d->scale();

                  // If sync scale is checked, recalculate the scale
                  if (p_syncScale->isChecked()) {
                     scale = fixedScale / d->universalGroundMap()->Resolution();
                  }
                  //This line moves the selected point of the companion image to the
                  //center of the viewport.
                  d->setScale(scale,sample,line);
    
                  /*convert the line,sample to viewport x,y*/
                  d->cubeToViewport(sample,line, x,y);
                  p_point.setX((int)x);
                  p_point.setY((int)y);
                  d->viewport()->repaint();
                }
                else {
                  p_point.setX(0);
                  p_point.setY(0);
                  d->viewport()->repaint();
                }
              }
            }
          }// end for cubeViewportList 
        }
       p_paint = false;
    } //end if p_pressed
  }

  /**
   * This method paints the viewport
   * 
   * @param vp 
   * @param painter 
   */
  void FindTool::paintViewport(CubeViewport *vp, QPainter *painter) {
    if(!p_paint || vp->universalGroundMap() == NULL)return;
    if(vp == cubeViewport() || vp->isLinked()) {
      painter->setPen(p_pen);  
  
      if((p_point.x() > 0) && (p_point.y() > 0)) {
        painter->drawRoundRect(p_point.x(), p_point.y(), 4, 4);
        //p_point.setX(0);
        //p_point.setY(0);
      }
    }
  }


  /**
   * 
   * 
   */
  void FindTool::clearPoint() {
     p_paint = false;
     p_point.setX(0);
     p_point.setY(0);
     CubeViewport *cvp;
    for (int i=0; i<(int)cubeViewportList()->size(); i++) {
      cvp = (*(cubeViewportList()))[i];
      cvp->viewport()->repaint();
    }
  }


  /**
   * Links all cubes that have camera models or are map projections
   */ 
  void FindTool::linkValid() {
    CubeViewport *d;
    for (int i=0; i<(int)cubeViewportList()->size(); i++) {
      d = (*(cubeViewportList()))[i];
      if (d->universalGroundMap() != NULL) {
        d->setLinked(true);
      }
      else {
        d->setLinked(false);
      }
    }
  }
}

