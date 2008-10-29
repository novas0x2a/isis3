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

  FindTool::~FindTool(){
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

    QPushButton *clearButton = new QPushButton("Clear Dot");
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearPoint()));

    QPushButton* cancelButton = new QPushButton ("Cancel");
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(clearPoint()));
    connect(cancelButton, SIGNAL(clicked()), p_dialog, SLOT(hide()));

    // Put the buttons in a horizontal orientation
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->addWidget(okButton);
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

      // Update Status Text Field
      if (cubeViewport()->universalGroundMap() == NULL) {
        p_status->setText("None");
        p_findPoint->setEnabled(false);
        p_findPtButton->setEnabled(false);
        p_syncScale->setEnabled(false);
        if (cubeViewportList()->size() > 1) {
          p_linkButton->setEnabled(true);
        }
        else {
          p_linkButton->setEnabled(false);
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
  void FindTool::okAction(){

    if (p_tabWidget->tabText(p_tabWidget->currentIndex()) == "Ground") {
      getUserGroundPoint();
    } else if (p_tabWidget->tabText(p_tabWidget->currentIndex())== "Image") {
      getUserImagePoint();
    }
  }

  /** 
   *  Gets the lat/lon position the user entered from the dialog box
   *  
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
    double lon = Isis::iString(p_groundTab->p_lonLineEdit->text().toStdString()).ToDouble();

    if (d->universalGroundMap()->SetUniversalGround(lat,lon)) {
      //p_dialog->hide();
      double sample = d->universalGroundMap()->Sample();
      double line = d->universalGroundMap()->Line();
      int x,y;
      d->cubeToViewport(sample,line,x,y);
      emit findPoint(QPoint(x,y));          
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
   */
  void FindTool::getUserImagePoint() {
    CubeViewport *d = cubeViewport();
    if (d == NULL) return;

   int line = p_imageTab->p_lineLineEdit->text().toInt();
   int sample = p_imageTab->p_sampLineEdit->text().toInt();
   if(sample > d->cubeSamples() || sample < 1) {
     p_imageTab->p_sampLineEdit->setText("1");
     QMessageBox::warning(p_dialog, "Error",
                "Specified Sample is not in active viewport",
                QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);

   }

   if(line > d->cubeLines() || line < 1) {
     p_imageTab->p_lineLineEdit->setText("1");
     QMessageBox::warning(p_dialog, "Error",
                "Specified Line is not in active viewport",
                QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);

   }

   int x,y;
   d->cubeToViewport(sample,line,x,y);
   emit findPoint(QPoint(x,y));
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
          p_dialog->hide();
          return;
        }
      }
    }

    // Get the sample line position to report
    d = cubeViewport();
    double sample,line;
    d->viewportToCube(p.x(),p.y(),sample,line);

    // If we are outside of the cube, do nothing
    if ((sample < 0.5) || (line < 0.5) || 
        (sample > d->cubeSamples()+0.5) || (line > d->cubeLines()+0.5)) {
     return;
    }

    if  (d->universalGroundMap()->SetImage(sample,line)) {
      d->setScale(d->scale(),sample,line);

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
            if (d->universalGroundMap()->SetUniversalGround(lat,lon)) {

              sample = d->universalGroundMap()->Sample();
              line = d->universalGroundMap()->Line();
              double scale = d->scale();

              // If sync scale is checked, recalculate the scale
              if (p_syncScale->isChecked()) {
                 scale = fixedScale / d->universalGroundMap()->Resolution();
              }
              d->setScale(scale,sample,line);
            }
            else {
              QApplication::beep();
            }
          }
        }
      }
    }
    else {
      QApplication::beep();
    }

    int x = 0;
    int y = 0;
    d->cubeToViewport(sample, line, x, y);
    p_point.setX(x);
    p_point.setY(y);
    d->viewport()->repaint();
  }

  /** 
   *  Centers the lat/lon value clicked on in all linked windows
   * 
   * @param p 
   * @param s 
   */
  void FindTool::mouseButtonRelease(QPoint p, Qt::MouseButton s) {
    if (s == Qt::MidButton) {
      if (cubeViewport()->universalGroundMap() == NULL) {
        QApplication::beep();
      }
      else {
        //emit findPoint(p);
        mouseMove(p);
      }
    }
    p_released = true;
    p_pressed = false;
  }

  /**
   * Called when a mouse button is pressed
   * 
   * @param p 
   * @param s 
   */
  void FindTool::mouseButtonPress(QPoint p, Qt::MouseButton s) {
    p_released = false;
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
    CubeViewport *d;
    double sample, line, lat=0, lon=0;
    Isis::UniversalGroundMap *groundMap;
    int x,y;
    if(p_pressed) {
      if(!p_released) {
        p_point = p;

        /*convert the viewport x,y to line,sample */
        cubeViewport()->viewportToCube(p_point.x(),p_point.y(),sample,line);

        /*convert the line,sample to lat,lon*/
        if (cubeViewport()->camera() != NULL) {
          if (cubeViewport()->camera()->SetImage(sample,line)) {
            lat = cubeViewport()->camera()->UniversalLatitude();
            lon = cubeViewport()->camera()->UniversalLongitude();
          }
        }

        /* now convert this lat lon to viewport coordinates for the paintViewport
           method to use.*/

        for (int i=0; i < (int)cubeViewportList()->size(); i++) {
          d = (*(cubeViewportList()))[i];
          //if((d->isLinked()) && (d != cubeViewport())) {
          if(d->isLinked()) {
            groundMap = new Isis::UniversalGroundMap(*d->cube());
            /*convert the lat,lon to line,sample of the linked cvp */
            if(groundMap->SetUniversalGround(lat, lon)){
              sample = groundMap->Sample();
              line = groundMap->Line();

              double scale = d->scale();
              //This line moves the selected point of the companion image to the
              //center of the viewport.
              if(d != cubeViewport()) d->setScale(scale,sample,line);

              /*convert the line,sample to viewport x,y*/
              d->cubeToViewport(sample,line, x,y);
              p_point.setX((int)x);
              p_point.setY((int)y);
              d->viewport()->repaint();
              clearPoint();
            }
            delete groundMap;

          }
        }// end for cubeViewportList

      } // end if !p_released      
    } //end if p_pressed
  }

  /**
   * This method paints the viewport
   * 
   * @param vp 
   * @param painter 
   */
  void FindTool::paintViewport(CubeViewport *vp, QPainter *painter) {
    if(vp != cubeViewport() && p_dialog->isVisible()) return; 
    if(!vp->isLinked() && !p_dialog->isVisible())return;

    painter->setPen(p_pen);  

    if((p_point.x() > 0) && (p_point.y() > 0)) {
      painter->drawRoundRect(p_point.x(), p_point.y(), 4, 4);
      //p_point.setX(0);
      //p_point.setY(0);
    }
    
  }


  /**
   * 
   * 
   */
  void FindTool::clearPoint(){
     p_point.setX(0);
     p_point.setY(0);
     cubeViewport()->viewport()->repaint();
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

