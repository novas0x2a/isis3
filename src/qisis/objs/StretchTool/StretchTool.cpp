#include <iostream>
#include <QMenuBar>
#include <QPixmap>
#include <QToolBar>
#include <QValidator>
#include <QHBoxLayout>

#include "StretchTool.h"
#include "MainWindow.h"
#include "CubeViewport.h"
#include "Workspace.h"

namespace Qisis {
  /**
   * StretchTool constructor
   * 
   * 
   * @param parent 
   */
  StretchTool::StretchTool (QWidget *parent) : Tool::Tool(parent) {
    p_stretchGlobal = new QAction(parent);
    p_stretchGlobal->setShortcut(Qt::CTRL+Qt::Key_G);
    p_stretchGlobal->setText("Global Stretch");
    connect(p_stretchGlobal,SIGNAL(activated()),this,SLOT(stretchGlobal()));

    p_stretchRegional = new QAction(parent);
    p_stretchRegional->setShortcut(Qt::CTRL+Qt::Key_R);
    p_stretchRegional->setText("Regional Stretch");
    connect(p_stretchRegional,SIGNAL(activated()),this,SLOT(stretchRegional()));

    
  }


  /**
   * Adds the stretch action to the toolpad.
   * 
   * 
   * @param pad 
   * 
   * @return QAction* 
   */
  QAction *StretchTool::toolPadAction(ToolPad *pad) {

    QAction *action = new QAction(pad);
    action->setIcon(QPixmap(toolIconDir()+"/stretch_global.png"));
    action->setToolTip("Stretch (S)");
    action->setShortcut(Qt::Key_S);
    QString text  =
      "<b>Function:</b>  Change the stretch range of the cube.\
      <p><b>Shortcut:</b>  S</p> ";
    action->setWhatsThis(text);
    return action;
  }


  /**
   * Adds the stretch action to the given menu.
   * 
   * 
   * @param menu 
   */
  void StretchTool::addTo (QMenu *menu) {
    menu->addAction(p_stretchGlobal);
    menu->addAction(p_stretchRegional);
  }


  /**
   * Creates the widget to add to the tool bar.
   * 
   * 
   * @param parent 
   * 
   * @return QWidget* 
   */
  QWidget *StretchTool::createToolBarWidget (QStackedWidget *parent) {

    QWidget *hbox = new QWidget(parent);

    QToolButton *butt = new QToolButton(hbox);
    butt->setAutoRaise(true);
    butt->setIconSize(QSize(22,22));
    butt->setIcon(QPixmap(toolIconDir()+"/regional_stretch-2.png"));
    butt->setToolTip("Stretch");
    QString text  =
      "<b>Function:</b> Automatically compute min/max stretch using viewed \
      pixels in the band(s) of the active viewport.  That is, only pixels \
      that are visible in the viewport are used. \
      If the viewport is in RGB color all three bands will be stretched. \
      <p><b>Shortcut:</b>  Ctrl+R</p> \
      <p><b>Mouse:</b>  Left click \
      <p><b>Hint:</b>  Left click and drag for a local stretch.  Uses only \
      pixels in the red marquee</p>";
    butt->setWhatsThis(text);
    connect(butt,SIGNAL(clicked()),this,SLOT(stretchRegional()));
    p_stretchRegionalButton = butt;

    p_stretchBandComboBox = new QComboBox (hbox);
    p_stretchBandComboBox->setEditable(false);
    p_stretchBandComboBox->addItem("Red Band");
    p_stretchBandComboBox->addItem("Green Band");
    p_stretchBandComboBox->addItem("Blue Band");
//    p_stretchBandComboBox->addItem("All Bands");
    p_stretchBandComboBox->setToolTip("Select Color");
    text =
      "<b>Function:</b> Selecting the color will allow the appropriate \
      min/max to be seen and/or edited in text fields to the right.";
//      The All option implies the same min/max will be applied
//      to all three colors (RGB) if either text field is edited";
    p_stretchBandComboBox->setWhatsThis(text);
    p_stretchBandComboBox->setCurrentIndex(0);
    p_stretchBand = Red;
    connect(p_stretchBandComboBox,SIGNAL(activated(int)),this,SLOT(selectBandMinMax(int)));

    QDoubleValidator *dval = new QDoubleValidator(hbox);
    p_stretchMinEdit = new QLineEdit (hbox);
    p_stretchMinEdit->setValidator(dval);
    p_stretchMinEdit->setToolTip("Minimum");
    text =
      "<b>Function:</b> Shows the current minimum pixel value.  Pixel values \
      below minimum are shown as black.  Pixel values above the maximum \
      are shown as white or the highest intensity of red/green/blue \
      if in color. Pixel values between the minimum and maximum are stretched \
      linearly between black and white (or color component). \
      <p><b>Hint:</b>  You can manually edit the minimum but it must be \
      less than the maximum.";
    p_stretchMinEdit->setWhatsThis(text);
    p_stretchMinEdit->setMaximumWidth(100);
    connect(p_stretchMinEdit,SIGNAL(returnPressed()),this,SLOT(changeStretch()));

    p_stretchMaxEdit = new QLineEdit (hbox);
    p_stretchMaxEdit->setValidator(dval);
    p_stretchMaxEdit->setToolTip("Maximum");
    text =
      "<b>Function:</b> Shows the current maximum pixel value.  Pixel values \
      below minimum are shown as black.  Pixel values above the maximum \
      are shown as white or the highest intensity of red/green/blue \
      if in color. Pixel values between the minimum and maximum are stretched \
      linearly between black and white (or color component). \
      <p><b>Hint:</b>  You can manually edit the maximum but it must be \
      greater than the minimum";
    p_stretchMaxEdit->setWhatsThis(text);
    p_stretchMaxEdit->setMaximumWidth(100);
    connect(p_stretchMaxEdit,SIGNAL(returnPressed()),this,SLOT(changeStretch()));

    /*create the two menus that drop down from the buttons*/
    QMenu *copyMenu = new QMenu();
    QMenu *globalMenu = new QMenu(); 

    p_copyBands = new QAction(parent);
    p_copyBands->setText("to All Bands");    
    connect(p_copyBands,SIGNAL(triggered(bool)),this,SLOT(setStretchBands()));
    
    QAction *copyAll = new QAction(parent);
    copyAll->setIcon(QPixmap(toolIconDir()+"/copy_stretch.png"));
    copyAll->setText("to All Viewports");   
    connect(copyAll,SIGNAL(triggered(bool)),this,SLOT(setStretchAllViewports()));

    copyMenu->addAction(copyAll);
    copyMenu->addAction(p_copyBands);

    p_copyButton = new QToolButton();
    p_copyButton->setAutoRaise(true);
    p_copyButton->setIconSize(QSize(22,22));
    p_copyButton->setIcon(QPixmap(toolIconDir()+"/copy_stretch.png"));
    p_copyButton->setPopupMode(QToolButton::MenuButtonPopup);    
    p_copyButton->setMenu(copyMenu);
    p_copyButton->setDefaultAction(copyAll);
    p_copyButton->setToolTip("Copy");
    text  =
      "<b>Function:</b> Copy the current stretch to all the \
      active viewports. Or use the drop down menu to copy the current stretch \
      to all the  bands in the active viewport. \
      <p><b>Hint:</b>  Can reset the stretch to an automaticaly computed \
      stretch by using the 'Reset' stretch button option. </p>";
    p_copyButton->setWhatsThis(text);

    QAction *currentView = new QAction(parent);
    currentView->setText("Active Viewport");
    currentView->setIcon(QPixmap(toolIconDir()+"/global_stretch.png"));
    globalMenu->addAction(currentView);
    connect(currentView,SIGNAL(triggered(bool)),this,SLOT(stretchGlobal()));

    QAction *globalAll = new QAction(parent);
    globalAll->setText("All Viewports");
    globalMenu->addAction(globalAll);
    connect(globalAll,SIGNAL(triggered(bool)),this,SLOT(stretchGlobalAllViewports()));

    QAction *globalBands = new QAction(parent);
    globalBands->setText("All Bands");
    globalMenu->addAction(globalBands);
    connect(globalBands,SIGNAL(triggered(bool)),this,SLOT(stretchGlobalAllBands()));

    p_globalButton = new QToolButton(); //basically acts as a 'reset'
    p_globalButton->setAutoRaise(true);
    p_globalButton->setIconSize(QSize(22,22));
    p_globalButton->setPopupMode(QToolButton::MenuButtonPopup);
    p_globalButton->setMenu(globalMenu);
    p_globalButton->setDefaultAction(currentView);
    p_globalButton->setToolTip("Reset");
    text  =
      "<b>Function:</b> Reset the stretch to be automatically computed \
      using the statisics from the entire image. Use the drop down menu \
      to reset the stretch for all the bands in the active viewport or \
      to reset the stretch for all the viewports. </p>";
    p_globalButton->setWhatsThis(text);
    
    QHBoxLayout *layout = new QHBoxLayout(hbox);
    layout->setMargin(0);
    layout->addWidget(p_copyButton);
    layout->addWidget(p_globalButton);
    layout->addWidget(p_stretchRegionalButton);
    layout->addWidget(p_stretchBandComboBox);
    layout->addWidget(p_stretchMinEdit);
    layout->addWidget(p_stretchMaxEdit);
    layout->addStretch(1);
    hbox->setLayout(layout);
    return hbox;
  }


  /**
   * Does a global stretch for the active viewport.
   * 
   */
  void StretchTool::stretchGlobal() {
    CubeViewport *vp = cubeViewport();
    if (vp == NULL) return;
    if (vp->isGray()) {
      vp->setStretchInfo(vp->grayBand(), false, 0, 1);
    } else {
      vp->setStretchInfo(vp->redBand(), false, 0, 1);
      vp->setStretchInfo(vp->greenBand(), false, 0, 1);
      vp->setStretchInfo(vp->blueBand(), false, 0, 1);

    }
    vp->autoStretch();
    updateTool();
  }


  /** 
   * Does a global stretch for all the viewports.
   * 
   */
  void StretchTool::stretchGlobalAllViewports(){
    for( int i = 0; i < (int)cubeViewportList()->size(); i++) {
      if ((*(cubeViewportList()))[i]->isGray()) {
        (*(cubeViewportList()))[i]->setStretchInfo((*(cubeViewportList()))[i]->grayBand(), false, 0, 1);
      } else {
        (*(cubeViewportList()))[i]->setStretchInfo((*(cubeViewportList()))[i]->redBand(), false, 0, 1);
        (*(cubeViewportList()))[i]->setStretchInfo((*(cubeViewportList()))[i]->greenBand(), false, 0, 1);
        (*(cubeViewportList()))[i]->setStretchInfo((*(cubeViewportList()))[i]->blueBand(), false, 0, 1);
      }
      (*(cubeViewportList()))[i]->autoStretch();
    }
    updateTool();
  }


  /** 
   * Does a globabl stretch for all the bands in the active
   * viewport.
   * 
   */
  void StretchTool::stretchGlobalAllBands() {
    CubeViewport *vp = cubeViewport();
    if (vp == NULL) return;
    for(int b = 1; b <= vp->cubeBands(); b++) {
      vp->setStretchInfo(b, false, 0, 1);
    }
    vp->autoStretch();
    updateTool();
  }


  /**
   * Does a regional stretch for the active viewport.
   * 
   */
  void StretchTool::stretchRegional() {
    CubeViewport *vp = cubeViewport();
    if (vp == NULL) return;

    double ssamp,esamp,sline,eline;
    vp->viewportToCube(0,0,ssamp,sline);
    vp->viewportToCube(vp->viewport()->width()-1,vp->viewport()->height()-1,
                       esamp,eline);
    vp->autoStretch((int)ssamp,(int)esamp,(int)sline,(int)eline);
    updateTool();
  }


  /**
   * Updates the stretch tool.
   * 
   */
  void StretchTool::updateTool() {

    CubeViewport *vp = cubeViewport();
    if (vp == NULL) return;

    if(vp->isColor()) {
      p_copyBands->setEnabled(false);
    } else {
      p_copyBands->setEnabled(true);
    }

    // Deal with gray viewports
    double min=0,max=0;
    if (vp->isGray()) {
      p_stretchBandComboBox->setShown(false);
      Isis::Stretch str = vp->grayStretch();
      min = str.Input(0);
      max = str.Input(1);
    }

    // Deal with color viewports
    else {
      p_stretchBandComboBox->setShown(true);

      if (p_stretchBand == Red) {
        Isis::Stretch str = vp->redStretch();
        min = str.Input(0);
        max = str.Input(1);
      }
      if (p_stretchBand == Green) {
        Isis::Stretch str = vp->greenStretch();
        min = str.Input(0);
        max = str.Input(1);
      }
      if (p_stretchBand == Blue) {
        Isis::Stretch str = vp->blueStretch();
        min = str.Input(0);
        max = str.Input(1);
      }
    }

    QString strMin;
    strMin.setNum(min);

    p_stretchMinEdit->setText(strMin);

    QString strMax;
    strMax.setNum(max);

    p_stretchMaxEdit->setText(strMax);

    if (vp->getStretchFlag(vp->grayBand())) {
      changeStretch();
    }

  }


  /**
   * Allows the user to select the band to do the stretch on.
   * 
   * 
   * @param index 
   */
  void StretchTool::selectBandMinMax (int index) {
    if (index == 0) p_stretchBand = Red;
    if (index == 1) p_stretchBand = Green;
    if (index == 2) p_stretchBand = Blue;
//    if (index == 3) p_stretchBand = All;
    updateTool();
  }


  /**
   * Sets up stretch parameters.
   * 
   */
  void StretchTool::changeStretch () {
    CubeViewport *vp = cubeViewport();
    if (vp == NULL) return;

    // Make sure they user didn't enter bad min/max and if so fix it
    double min = p_stretchMinEdit->text().toDouble();
    double max = p_stretchMaxEdit->text().toDouble();
    if (min >= max || p_stretchMinEdit->text() == "" || p_stretchMaxEdit->text() == "") {
      updateTool();
      return;
    }

    // Set up the stretch string
    QString str = QString("%1:0.0 %2:255.0").arg(min).arg(max);

    if (vp->isGray()) {
      vp->setStretchInfo(vp->grayBand(), true, min, max);

      Isis::Stretch stretch = vp->grayStretch();
      stretch.ClearPairs();
      stretch.AddPair(min,0.0);
      stretch.AddPair(max,255.0);
      
      vp->stretchGray(str);


    }
    if (vp->isColor()) {
      std::string redStretch = vp->redStretch().Text();
      std::string greenStretch = vp->greenStretch().Text();
      std::string blueStretch = vp->blueStretch().Text();
      QString redstr = redStretch.c_str();
      QString grnstr = greenStretch.c_str();
      QString blustr = blueStretch.c_str();

      if (p_stretchBand == Red) {
        redstr = str;
      }
      if (p_stretchBand == Green) {
        grnstr = str;
      }
      if (p_stretchBand == Blue) {
        blustr = str;
      }

      vp->stretchRGB(redstr,grnstr,blustr);
    }
  }


  /** 
   * This method is called when the RubberBandTool is complete. It
   * will get a rectangle from the RubberBandTool and stretch
   * accordingly.
   * 
   */
  void StretchTool::rubberBandComplete() {
    CubeViewport *vp = cubeViewport();
    if (vp == NULL) return;
    if(!RubberBandTool::isValid()) return;

    QRect r = RubberBandTool::rectangle();
    if ((r.width() <= 4) || (r.height() <= 4)) {
      //Just do nothing.
      //stretchRegional();
    }
    else {
      double ssamp,esamp,sline,eline;
      vp->viewportToCube(r.left(),r.top(),ssamp,sline);
      vp->viewportToCube(r.right(),r.bottom(),esamp,eline);
      vp->autoStretch((int)ssamp,(int)esamp,(int)sline,(int)eline);
      updateTool();  
    }
  }


  /** 
   * This method will call a global stretch if the right mouse 
   * button is released. 
   * 
   * @param start
   * @param s
   */
  void StretchTool::mouseButtonRelease (QPoint start, Qt::MouseButton s) {
    if (s == Qt::RightButton) {
      stretchGlobal();
      // Resets the RubberBandTool on screen.
      enableRubberBandTool();
    }
  }


  /** 
   * This method enables the RubberBandTool.
   * 
   */
  void StretchTool::enableRubberBandTool() {
    RubberBandTool::enable(RubberBandTool::Rectangle);
  }


  /** 
   * Sets the stretch for all the bands in the acitve viewport to
   * the current stretch
   * 
   */
  void StretchTool::setStretchBands(){
    CubeViewport *cvp = cubeViewport();
    for(int b = 1; b <= cvp->cubeBands(); b++) {
      cvp->setStretchInfo(b, true, p_stretchMinEdit->text().toDouble(), 
                          p_stretchMaxEdit->text().toDouble());
    }
  }


  /** 
   * Sets the stretch for all the viewports to the current
   * stretch in the active viewport.
   * 
   */
  void StretchTool::setStretchAllViewports(){

    for ( int i = 0; i < (int)cubeViewportList()->size(); i++) {
      int b = (*(cubeViewportList()))[i]->grayBand();
      double min = p_stretchMinEdit->text().toDouble();
      double max = p_stretchMaxEdit->text().toDouble();
      (*(cubeViewportList()))[i]->setStretchInfo(b, true, min, max); 

      QString str = QString::number(min) + ":0.0 " +
                    QString::number(max) + ":255.0";

      if ((*(cubeViewportList()))[i]->isGray()) {

        (*(cubeViewportList()))[i]->stretchGray(str);

      } else {
        std::string redStretch = (*(cubeViewportList()))[i]->redStretch().Text();
        std::string greenStretch = (*(cubeViewportList()))[i]->greenStretch().Text();
        std::string blueStretch = (*(cubeViewportList()))[i]->blueStretch().Text();
        QString redstr = redStretch.c_str();
        QString grnstr = greenStretch.c_str();
        QString blustr = blueStretch.c_str();

        if (p_stretchBand == Red) {
          redstr = str;
        }
        if (p_stretchBand == Green) {
          grnstr = str;
        }
        if (p_stretchBand == Blue) {
          blustr = str;
        }

        (*(cubeViewportList()))[i]->stretchRGB(redstr,grnstr,blustr);

      }
    }
  }
  
}
