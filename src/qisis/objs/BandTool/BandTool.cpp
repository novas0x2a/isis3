#include <QMenuBar>
#include <QPixmap>
#include <QToolBar>
#include <QToolButton>
#include <QComboBox>

#include "BandTool.h"
#include "MainWindow.h"
#include "CubeViewport.h"


namespace Qisis {

  /**
   * BandTool constructor.
   * 
   * 
   * @param parent 
   */
  BandTool::BandTool (QWidget *parent) : Qisis::Tool(parent) {
  }

  /** 
   * 
   * 
   * @param pad
   * 
   * @return QAction*
   */
  QAction *BandTool::toolPadAction(ToolPad *pad) {
    QAction *action = new QAction(pad);
    action->setIcon(QPixmap(toolIconDir()+"/rgb.png"));
    action->setToolTip("Band Selection (B)");
    action->setShortcut(Qt::Key_B);
    QString text  =
      "<b>Function:</b>  Change the view of the cube from gray scale to RGB.\
      <p><b>Shortcut:</b>  B</p> ";
    action->setWhatsThis(text);
    return action;
  }

  /** 
   * 
   * 
   * @param active
   * 
   * @return QWidget*
   */
  QWidget *BandTool::createToolBarWidget (QStackedWidget *active) {
    QWidget*hbox = new QWidget(active);

    p_rgbButton = new QRadioButton(hbox);
    p_blackwhiteButton = new QRadioButton(hbox);
    QIcon colorIcon;
    QIcon grayIcon;
    colorIcon.addPixmap(toolIconDir()+"/rgb.png",QIcon::Normal,QIcon::On);
    grayIcon.addPixmap(toolIconDir()+"/gray.png",QIcon::Normal,QIcon::Off);
    p_rgbButton->setIcon(colorIcon);
    p_rgbButton->setText("RGB");
    p_blackwhiteButton->setIcon(grayIcon);
    p_blackwhiteButton->setText("Gray");
    p_rgbButton->setCheckable(true);
    p_rgbButton->setIconSize(QSize(22,22));
    p_blackwhiteButton->setIconSize(QSize(22,22));
    p_rgbButton->setToolTip("Change to RGB");
    p_blackwhiteButton->setToolTip("Change to grayscale");
    QString text =
      "<b>Function:</b> Toggle the active viewport between color or \
      grayscale display of the cube.  Color display is only possible if \
      the cube has two or more bands";
    p_rgbButton->setWhatsThis(text);
    p_blackwhiteButton->setWhatsThis(text);

    p_comboBox = new QComboBox(hbox);
    p_comboBox->setEditable(false);
    p_comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    p_comboBox->addItem("Wavelength");
    p_comboBox->setToolTip("Select BandBin keyword");
    text =
      "<b>Function:</b> The default option \"Wavelength\" \
      simply shows the current band displayed in the viewport. However, \
      the labels of many cubes contain the BandBin group. \
      Keywords in this group describe the bands in a meaningful way, \
      such as WaveLength, Filter, Temperature, iTime, etc. \
      Selecting an alternative BandBin keyword will cause those values \
      to show in the spin boxes to the right.";
    p_comboBox->setWhatsThis(text);

    p_stack = new QStackedWidget(hbox);

    QWidget *stuff1 = new QWidget(p_stack);
    p_graySpin = new QSpinBox(stuff1);
    p_graySpin->setToolTip("Change gray band");
    p_stack->addWidget(stuff1);

    QWidget *stuff2 = new QWidget(p_stack);
    p_redSpin = new QSpinBox(stuff2);
    p_redSpin->setToolTip("Change red band");
    p_grnSpin = new QSpinBox(stuff2);
    p_grnSpin->setToolTip("Change green band");
    p_bluSpin = new QSpinBox(stuff2);
    p_bluSpin->setToolTip("Change blue band");
    p_stack->addWidget(stuff2);

    p_stack2 = new QStackedWidget(hbox);
    QWidget *grayWidget = new QWidget(p_stack2);
    p_grayDisplay = new QLabel(grayWidget);
    p_grayDisplay->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    p_stack2->addWidget(grayWidget);

    QWidget *colorWidget = new QWidget(p_stack2);
    p_redDisplay = new QLabel(colorWidget);
    p_greenDisplay = new QLabel(colorWidget);
    p_blueDisplay = new QLabel(colorWidget);
    p_redDisplay->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    p_greenDisplay->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    p_blueDisplay->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    p_stack2->addWidget(colorWidget);
 

    QHBoxLayout *displayLayout = new QHBoxLayout(grayWidget);
    displayLayout->addWidget(p_grayDisplay);
    displayLayout->addStretch(1);
    grayWidget->setLayout(displayLayout);

    displayLayout = new QHBoxLayout(colorWidget);
    displayLayout->addWidget(p_redDisplay);
    displayLayout->addWidget(p_greenDisplay);
    displayLayout->addWidget(p_blueDisplay);
    colorWidget->setLayout(displayLayout);

    QHBoxLayout *layout = new QHBoxLayout(stuff1);
    layout->setMargin(0);
    layout->addWidget(p_graySpin);
    layout->addStretch(1);
    stuff1->setLayout(layout);

    layout = new QHBoxLayout(stuff2);
    layout->setMargin(0);
    layout->addWidget(p_redSpin);
    layout->addWidget(p_grnSpin);
    layout->addWidget(p_bluSpin);
    
    stuff2->setLayout(layout);

    p_stack->setCurrentIndex(0);
    p_stack2->setCurrentIndex(0);
    p_rgbButton->setChecked(false);
    p_blackwhiteButton->setChecked(true);

    QFrame *vertLine = new QFrame();
    vertLine->setFrameShape(QFrame::VLine);
    vertLine->setFrameShadow(QFrame::Sunken);

    layout = new QHBoxLayout(hbox);
    layout->setMargin(0);
    layout->addWidget(p_rgbButton);
    layout->addWidget(p_blackwhiteButton);
    layout->addWidget(p_stack);
    layout->addWidget(vertLine);
    layout->addWidget(p_comboBox);
    layout->addWidget(p_stack2);
    layout->addStretch(1);
    hbox->setLayout(layout);

    return hbox;
  }


  /** 
   * This method sets the p_lineEditValueList to the proper values
   * according to what the user has selected in the p_comboBox.
   * These are the values shown in the gray boxes.
   */
  void BandTool::setList(){
    if (p_pvl.FindObject("IsisCube").HasGroup("BandBin") && 
        p_comboBox->count() > 0) {

      Isis::PvlGroup &bandBin = p_pvl.FindObject("IsisCube")
                                   .FindGroup("BandBin");
      p_comboBox->setVisible(true);
      p_grayDisplay->setVisible(true);
      p_redDisplay->setVisible(true);
      p_greenDisplay->setVisible(true);
      p_blueDisplay->setVisible(true);

      for (int i=0; i<bandBin.Keywords(); i++) {
        if (bandBin[i].Name() == p_comboBox->currentText().toStdString()) {
          p_lineEditValueList.clear();
          for (int j=0; j<bandBin[i].Size(); j++) {
            p_lineEditValueList.push_back(QString(bandBin[i][j].c_str()));
          }
        }
      }
    } else {
      for (int i=1; i<=p_bands; i++) {
        p_lineEditValueList.push_back(QString::number(i));
      }
      p_comboBox->setVisible(false);
      p_grayDisplay->setVisible(false);
      p_redDisplay->setVisible(false);
      p_greenDisplay->setVisible(false);
      p_blueDisplay->setVisible(false);
    }
  }


  /** 
   * This method fills the p_comboBox with the keywords from the 
   * band bin group of the currently selected cube.  If there are
   * values for 'Center', that is choosen as the default. 
   * 
   * @param pvl
   */
  void BandTool::setBandBin (Isis::Pvl &pvl) {

    // Get the number of bands and setup the spin box
    Isis::PvlGroup &dim = pvl.FindObject("IsisCube")
                             .FindObject("Core")
                             .FindGroup("Dimensions");
    p_pvl = pvl;
    p_bands = dim["Bands"];

    p_graySpin->setValue(1);
    p_graySpin->adjustSize();
    p_graySpin->setMinimum(1);
    p_graySpin->setMaximum(p_bands);

    p_redSpin->setValue(1);
    p_redSpin->setMinimum(1);
    p_redSpin->setMaximum(p_bands);

    p_bluSpin->setValue(1);
    p_bluSpin->setMinimum(1);
    p_bluSpin->setMaximum(p_bands);

    p_grnSpin->setValue(1);
    p_grnSpin->setMinimum(1);
    p_grnSpin->setMaximum(p_bands);

    p_comboBox->clear();
    if (pvl.FindObject("IsisCube").HasGroup("BandBin")) {
      Isis::PvlGroup &bandBin = pvl.FindObject("IsisCube")
                                .FindGroup("BandBin");
      for (int i=0; i<bandBin.Keywords(); i++) {
        //only add band bin keywords have a size that equals the number of bands 
        if(bandBin[i].Size() == p_bands){
          std::string bandBinName = bandBin[i].Name();
          p_comboBox->addItem(QString(bandBinName.c_str()));
        }

      }

      if (p_comboBox->findText("Center") > 0) {
        p_comboBox->setCurrentIndex(p_comboBox->findText("Center"));
      }
    }
    setList();
  }


  /** 
   * This method is connected to the qspinboxes.  When the user
   * selects a new band, the viewport needs to be updated and the
   * values display next to the p_comboBox also need to be
   * updated.
   * 
   */
  void BandTool::changeView () {
    CubeViewport *v = cubeViewport();

    if (v == NULL) return;

    if (p_rgbButton->isChecked()) {
      p_stack->setCurrentIndex(1);
      p_stack2->setCurrentIndex(1);
      if (v->isGray() ||
          p_redSpin->value() != v->redBand() ||
          p_grnSpin->value() != v->greenBand() ||
          p_bluSpin->value() != v->blueBand()) {
          v->viewRGB(p_redSpin->value(),p_grnSpin->value(),p_bluSpin->value());
      }
    }
    else {
      p_stack->setCurrentIndex(0);
      p_stack2->setCurrentIndex(0);
      if (v->isColor() || p_graySpin->value() != v->grayBand()) {
        v->viewGray(p_graySpin->value());
      }
    }

    setDisplay();

  }


  /** 
   * This method updates the values displayed in the gray boxes.
   * Called from changeView.
   */
  void BandTool::setDisplay(){
    //Gray
    if(p_graySpin->value()-1 < p_lineEditValueList.size()) {
      p_grayDisplay->setText
       (p_lineEditValueList[p_graySpin->value()-1]);
    } else {
      p_grayDisplay->setText("N/A");
    }
    p_grayDisplay->adjustSize();

    //Red
    if(p_redSpin->value()-1 < p_lineEditValueList.size()){
       p_redDisplay->setText
         (p_lineEditValueList[p_redSpin->value()-1]);
    } else {
      p_redDisplay->setText("N/A");
    }
     p_redDisplay->adjustSize();

     //Green
     if(p_grnSpin->value()-1 < p_lineEditValueList.size()){ 
       p_greenDisplay->setText
         (p_lineEditValueList[p_grnSpin->value()-1]);
     } else{
       p_greenDisplay->setText("N/A");
     }
     p_greenDisplay->adjustSize();

     //Blue
     if(p_bluSpin->value()-1 < p_lineEditValueList.size()){
       p_blueDisplay->setText
         (p_lineEditValueList[p_bluSpin->value()-1]);
     } else {
       p_blueDisplay->setText("N/A");
     }
     p_blueDisplay->adjustSize();

  }


  /** 
   * updates the band tool
   * 
   */
  void BandTool::updateTool() {

    disconnect (p_comboBox,0,0,0);
    disconnect (p_graySpin,0,0,0);
    disconnect (p_redSpin,0,0,0);
    disconnect (p_grnSpin,0,0,0);
    disconnect (p_bluSpin,0,0,0);
    disconnect(p_rgbButton,0,0,0);

    CubeViewport *cvp = cubeViewport();
    if (cvp != NULL) {
    
      setBandBin(*cvp->cube()->Label());

      if (cvp->isGray()) {

        p_rgbButton->setChecked(false);
        p_blackwhiteButton->setChecked(true);
        p_stack->setCurrentIndex(0);
        p_stack2->setCurrentIndex(0);

      }
      else {
        p_rgbButton->setChecked(true);
        p_blackwhiteButton->setChecked(false);
        p_stack->setCurrentIndex(1);
        p_stack2->setCurrentIndex(1);
      }

      p_graySpin->setValue(cvp->grayBand());
      p_graySpin->updateGeometry();

      p_redSpin->setValue(cvp->redBand());
      p_redSpin->updateGeometry();

      p_grnSpin->setValue(cvp->greenBand());
      p_grnSpin->updateGeometry();

      p_bluSpin->setValue(cvp->blueBand());
      p_bluSpin->updateGeometry();

      changeView();

      connect(p_comboBox,SIGNAL(activated(int)),this,SLOT(setList()));
      connect(p_comboBox,SIGNAL(activated(int)),this,SLOT(setDisplay()));

      connect(p_graySpin,SIGNAL(valueChanged(int)),this,SLOT(changeView()));
      connect(p_redSpin,SIGNAL(valueChanged(int)),this,SLOT(changeView()));
      connect(p_grnSpin,SIGNAL(valueChanged(int)),this,SLOT(changeView()));
      connect(p_bluSpin,SIGNAL(valueChanged(int)),this,SLOT(changeView()));

      connect(p_rgbButton,SIGNAL(toggled(bool)),this,SLOT(changeView()));
    }
  }


}
