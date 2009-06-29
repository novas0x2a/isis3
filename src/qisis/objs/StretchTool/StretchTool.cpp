#include <QMenuBar>
#include <QPixmap>
#include <QToolBar>
#include <QValidator>
#include <QHBoxLayout>

#include "StretchTool.h"
#include "MainWindow.h"
#include "CubeViewport.h"
#include "Workspace.h"

#include "Statistics.h"
#include "Histogram.h"
#include "Brick.h"

namespace Qisis {
  /**
   * StretchTool constructor
   * 
   * 
   * @param parent 
   */
  StretchTool::StretchTool (QWidget *parent) : Tool::Tool(parent) {
    //Create the advanced dialog's components and layout
    p_dialog = new QDialog(parent);

    p_currentStretch = NULL;

    p_grayHistogram = new HistogramWidget("Visible Histogram - Gray Band");
    p_redHistogram = new HistogramWidget("Visible Histogram - Red Band", Qt::red, Qt::darkRed);
    p_greenHistogram = new HistogramWidget("Visible Histogram - Green Band", Qt::green, Qt::darkGreen);
    p_blueHistogram = new HistogramWidget("Visible Histogram - Blue Band", Qt::blue, Qt::darkBlue);
    connect(p_grayHistogram, SIGNAL(rangeChanged(double, double)), 
            this, SLOT(grayRangeChanged(double, double)));
    connect(p_redHistogram, SIGNAL(rangeChanged(double, double)), 
            this, SLOT(redRangeChanged(double, double)));
    connect(p_greenHistogram, SIGNAL(rangeChanged(double, double)), 
            this, SLOT(greenRangeChanged(double, double)));
    connect(p_blueHistogram, SIGNAL(rangeChanged(double, double)), 
            this, SLOT(blueRangeChanged(double, double)));

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(p_redHistogram);
    hLayout->addWidget(p_greenHistogram);
    hLayout->addWidget(p_blueHistogram);

    QWidget *histColor = new QWidget();
    histColor->setLayout(hLayout);

    p_histogramWidget = new QStackedWidget();
    p_histogramWidget->addWidget(p_grayHistogram);
    p_histogramWidget->addWidget(histColor);

    p_stretchBox = new QComboBox();

    connect(p_stretchBox, SIGNAL(currentIndexChanged(int)), this, SLOT(stretchIndexChanged(int)));

    p_pairsTable = new QTableWidget(0, 2);

    QStringList labels;
    labels << "Input" << "Output";
    p_pairsTable->setHorizontalHeaderLabels(labels);
    p_pairsTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    p_pairsTable->verticalHeader()->hide();
    p_pairsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    p_pairsTable->setSelectionMode(QAbstractItemView::NoSelection);

    p_rgbBox = new QGroupBox("Band");
    p_redButton = new QRadioButton("R");
    p_greenButton = new QRadioButton("G");
    p_blueButton = new QRadioButton("B");

    p_redButton->setChecked(true);

    connect(p_redButton, SIGNAL(toggled(bool)), this, SLOT(setRedStretch(bool)));
    connect(p_greenButton, SIGNAL(toggled(bool)), this, SLOT(setGreenStretch(bool)));
    connect(p_blueButton, SIGNAL(toggled(bool)), this, SLOT(setBlueStretch(bool)));

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(p_redButton);
    hbox->addWidget(p_greenButton);
    hbox->addWidget(p_blueButton);
    hbox->addStretch(1);
    p_rgbBox->setLayout(hbox);

    p_stackedWidget = new QStackedWidget();

    p_saveButton = new QPushButton("Save Pairs");
    p_saveButton->setEnabled(false);

    QPushButton *saveAsButton = new QPushButton("Save Pairs As...");
    connect(p_saveButton, SIGNAL(clicked()), this, SLOT(savePairs()));
    connect(saveAsButton, SIGNAL(clicked()), this, SLOT(savePairsAs()));

    QHBoxLayout *saveButtonsLayout = new QHBoxLayout();
    saveButtonsLayout->addWidget(p_saveButton);
    saveButtonsLayout->addWidget(saveAsButton);

    p_flashButton = new QPushButton("Flash");
    QPushButton *hiddenButton = new QPushButton();
    hiddenButton->setVisible(false);
    hiddenButton->setDefault(true);
    connect(p_flashButton, SIGNAL(pressed()), this, SLOT(stretchGlobal()));
    connect(p_flashButton, SIGNAL(released()), this, SLOT(flash()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(hiddenButton);
    buttonsLayout->addWidget(p_flashButton);

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->addWidget(p_histogramWidget, 0, 0, 1, 2);
    mainLayout->addWidget(p_stretchBox, 2, 0);
    mainLayout->addWidget(p_rgbBox, 2, 1);
    mainLayout->addWidget(p_pairsTable, 3, 0);
    mainLayout->addWidget(p_stackedWidget, 3, 1, 2, 1);
    mainLayout->addLayout(saveButtonsLayout, 4, 0);
    mainLayout->addLayout(buttonsLayout, 5, 1);

    p_dialog->setLayout(mainLayout);

    p_dialog->setWindowTitle("Stretch");

    addStretch(new LinearStretch(this));
    addStretch(new SawtoothStretch(this));
    addStretch(new BinaryStretch(this));
    addStretch(new ManualStretch(this));

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
    connect(p_stretchBandComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(selectBandMinMax(int)));

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

    QPushButton *advancedButton = new QPushButton("Advanced");
    connect(advancedButton, SIGNAL(clicked()), this, SLOT(showAdvancedDialog()));

    QHBoxLayout *layout = new QHBoxLayout(hbox);
    layout->setMargin(0);
    layout->addWidget(p_copyButton);
    layout->addWidget(p_globalButton);
    layout->addWidget(p_stretchRegionalButton);
    layout->addWidget(p_stretchBandComboBox);
    layout->addWidget(p_stretchMinEdit);
    layout->addWidget(p_stretchMaxEdit);
    layout->addWidget(advancedButton);
    layout->addStretch(1);
    hbox->setLayout(layout);
    return hbox;
  }

  /**
   * Updates the stretch tool.
   * 
   */
  void StretchTool::updateTool() {
    CubeViewport *cvp = cubeViewport();

    if (cvp == NULL) {
      //If the current viewport is NULL and the advanced dialog is visible, hide it
      if(p_dialog->isVisible()) {
        p_dialog->hide();
      }
      return;
    }

    //If the viewport map does not contain this viewport yet we need to create a new
    //CubeInfo for each band of the viewport and insert the list of CubeInfos into the 
    //viewport map, also we need to connect a signal to tell when the viewport was closed 
    //so we can remove it from the viewport map properly.
    if(!p_viewportMap.contains(cvp)) {
      QList<StretchTool::CubeInfo *> infoList;
      
      //Create a new CubeInfo for each band of the viewport
      for(int i = 0; i < cvp->cubeBands(); i++) {
        CubeInfo *cubeInfo = new CubeInfo();
        cubeInfo->stretch = new LinearStretch(this);
        cubeInfo->hist = NULL;
        infoList.append(cubeInfo);
      }

      p_viewportMap[cvp] = infoList;

      connect(cvp, SIGNAL(destroyed(QObject *)),
              this, SLOT(removeViewport(QObject *)));
    }

    //If the viewport is in gray mode
    if (cvp->isGray()) {
      p_copyBands->setEnabled(true);
      p_stretchBandComboBox->setShown(false);

      //If the current band's current stretch does not exist, do a global stretch
      if(p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch.Pairs() == 0) {
        stretchGlobal();
      }
    }
    //Otherwise it is in color mode
    else {
      p_copyBands->setEnabled(false);
      p_stretchBandComboBox->setShown(true);

      //If the red green or blue band's current stretch does not exist, do a global stretch
      if(p_viewportMap[cvp][cvp->redBand()-1]->currentStretch.Pairs() == 0 || 
         p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch.Pairs() == 0 ||
         p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch.Pairs() == 0) {
        stretchGlobal();
      }
    }

    //If the advanced dialog is visible
    if(p_dialog->isVisible()) {
      //Set the title of the dialog to the viewport's cube's name
      QString windowTitle = cvp->windowTitle();
      windowTitle.truncate(cvp->windowTitle().lastIndexOf('@'));
      windowTitle.prepend("Stretch - ");

      p_dialog->setWindowTitle(windowTitle);

      //If the viewport is in gray mode
      if(cvp->isGray()) {
        p_rgbBox->hide();
        p_histogramWidget->setCurrentIndex(0);

        CubeInfo *info = p_viewportMap[cvp][cvp->grayBand()-1];

        //Set the gray histogram to the current band's info
        p_grayHistogram->setAbsoluteMinMax(info->min, info->max);
        p_grayHistogram->setMinMax(info->hist->Minimum(), info->hist->Maximum());
        p_grayHistogram->setHistogram(*info->hist); 
        p_grayHistogram->clearStretch();

        //Set the current stretch to the gray band's stretch
        p_stretchBox->setCurrentIndex(p_stretchBox->findText(info->stretch->name()));
        setStretch(info->stretch);

      }
      //Otherwise it is in color mode
      else {
        p_rgbBox->show();
        p_histogramWidget->setCurrentIndex(1);

        CubeInfo *redInfo = p_viewportMap[cvp][cvp->redBand()-1]; 
        CubeInfo *greenInfo = p_viewportMap[cvp][cvp->greenBand()-1];
        CubeInfo *blueInfo = p_viewportMap[cvp][cvp->blueBand()-1];

        //Set the red histogram to the red band's info
        p_redHistogram->setAbsoluteMinMax(redInfo->min, redInfo->max);
        p_redHistogram->setMinMax(redInfo->hist->Minimum(), redInfo->hist->Maximum());
        p_redHistogram->setHistogram(*redInfo->hist); 
        p_redHistogram->setStretch(redInfo->currentStretch);

        //If the stretch band is red set the current stretch to the red band's stretch
        if(p_stretchBand == Red) {
          p_stretchBox->setCurrentIndex(p_stretchBox->findText(redInfo->stretch->name()));
          setStretch(redInfo->stretch);
        }

        //Set the green histogram to the green band's info
        p_greenHistogram->setAbsoluteMinMax(greenInfo->min, greenInfo->max);
        p_greenHistogram->setMinMax(greenInfo->hist->Minimum(), greenInfo->hist->Maximum());
        p_greenHistogram->setHistogram(*greenInfo->hist); 
        p_greenHistogram->setStretch(greenInfo->currentStretch);

        //If the stretch band is green set the current stretch to the green band's stretch
        if(p_stretchBand == Green) {
          p_stretchBox->setCurrentIndex(p_stretchBox->findText(greenInfo->stretch->name()));
          setStretch(greenInfo->stretch);
        }

        //Set the blue histogram to the blue band's info
        p_blueHistogram->setAbsoluteMinMax(blueInfo->min, blueInfo->max);
        p_blueHistogram->setMinMax(blueInfo->hist->Minimum(), blueInfo->hist->Maximum());
        p_blueHistogram->setHistogram(*blueInfo->hist); 
        p_blueHistogram->setStretch(blueInfo->currentStretch);

        //If the stretch band is blue set the current stretch to the blue band's stretch
        if(p_stretchBand == Blue) {
          p_stretchBox->setCurrentIndex(p_stretchBox->findText(blueInfo->stretch->name()));
          setStretch(blueInfo->stretch);
        }
      }
    }
    //Otherwise set the stretch min/max text fields
    else {
      stretchChanged();
    }
  }

  /**
   * This method is called when the stretch has changed and sets the min/max
   * text fields to the correct values.
   * 
   */
  void StretchTool::stretchChanged() {
    CubeViewport *cvp = cubeViewport();
    if (cvp == NULL) return;

    double min=0, max=0;
    //If the viewport is in gray mode
    if (cvp->isGray()) {
      //Get the min/max from the current stretch
      min = p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch.Input(0);
      max = p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch.Input(p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch.Pairs()-1);

      //Stretch the viewport with the current stretch
      cvp->stretchGray(p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch);
    }
    
    //Otherwise it is in color mode
    else {
      //Get the min/max from the current stretch
      if (p_stretchBand == Red) {
        min = p_viewportMap[cvp][cvp->redBand()-1]->currentStretch.Input(0);
        max = p_viewportMap[cvp][cvp->redBand()-1]->currentStretch.Input(1);
      }
      if (p_stretchBand == Green) {
        min = p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch.Input(0);
        max = p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch.Input(1);
      }
      if (p_stretchBand == Blue) {
        min = p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch.Input(0);
        max = p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch.Input(1);
      }

      //Stretch the viewport with the current RGB stretch
      cvp->stretchRGB(p_viewportMap[cvp][cvp->redBand()-1]->currentStretch,
                     p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch,
                     p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch);
    }

    //Set the min/max text fields
    QString strMin;
    strMin.setNum(min);
    p_stretchMinEdit->setText(strMin);

    QString strMax;
    strMax.setNum(max);
    p_stretchMaxEdit->setText(strMax);
  }

  /**
   * This methods shows and updates the advanced dialog.
   * 
   */
  void StretchTool::showAdvancedDialog() {
    if(p_dialog->isVisible()) return;
    p_dialog->show();
    updateTool();
  }

  /**
   * This method is called when a viewport is closed and will properly remove and 
   * delete any viewport information in the viewport map. 
   * 
   * @param cvp 
   */
  void StretchTool::removeViewport (QObject *cvp) {
    //If the viewport map contains the viewport
    if (p_viewportMap.contains((CubeViewport *) cvp)) {
      QList<StretchTool::CubeInfo *> list = p_viewportMap.value((CubeViewport *) cvp);
      //Remove and delete all CubeInfos in the list
      while(!list.isEmpty()) {
        StretchTool::CubeInfo *info = list.takeFirst();
        //If the info is not NULL
        if(info) {
          //If the info's stretch is the current stretch, make sure it is disconnected
          if(info->stretch == p_currentStretch) {
            disconnect(p_currentStretch, SIGNAL(update()), this, SLOT(updateStretch()));
            p_currentStretch->disconnectTable(p_pairsTable);
      
            p_stackedWidget->removeWidget(p_currentStretch->getParameters());
            p_currentStretch = NULL;
          }
          //Delete the stretch
          delete info->stretch;

          //If the info's histogram is not NULL delete it
          if(info->hist) delete info->hist;

          //Delete the cube info
          delete info;
        }
      }

      //Remove the viewport from the map
      p_viewportMap.remove((CubeViewport *) cvp);
    } 
  }

  /**
   * Does a global stretch for the active viewport.
   * 
   */
  void StretchTool::stretchGlobal() {
    CubeViewport *cvp = cubeViewport();
    if (cvp == NULL) return;
    //The viewport is in gray mode
    if (cvp->isGray()) {
      //If the global stretch does not exist for the gray band
      if(p_viewportMap[cvp][cvp->grayBand()-1]->globalStretch.Pairs() == 0) {
        //If the viewport buffer contains the entire cube, stretch the entire viewport
        if(cvp->grayBuffer()->hasEntireCube()) {
          QRect rect(0, 0, cvp->viewport()->width(), cvp->viewport()->height());
          stretchBuffer(rect);
        }
        //Otherwise we need to read from the cube
        else {
          stretchEntireCube(cvp);
        }

        //Set the global stretch to the current stretch
        p_viewportMap[cvp][cvp->grayBand()-1]->globalStretch = p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch;
      }

      //Set the current stretch to the global stretch
      p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch = p_viewportMap[cvp][cvp->grayBand()-1]->globalStretch;

      //Stretch the viewport with the gray band's global stretch
      cvp->stretchGray(p_viewportMap[cvp][cvp->grayBand()-1]->globalStretch);

    } 
    //Otherwise the viewport is in color mode
    else {
      Isis::Stretch redStretch = cvp->redStretch();
      Isis::Stretch greenStretch = cvp->greenStretch();
      Isis::Stretch blueStretch = cvp->blueStretch();

      //If the global stretch does not exist for the red, green or blue band
      if(p_viewportMap[cvp][cvp->redBand()-1]->globalStretch.Pairs() == 0 ||
         p_viewportMap[cvp][cvp->greenBand()-1]->globalStretch.Pairs() == 0 ||
         p_viewportMap[cvp][cvp->blueBand()-1]->globalStretch.Pairs() == 0) {
        //If the viewport buffer contains the entire cube, stretch the entire viewport
        if(cvp->redBuffer()->hasEntireCube()) {
          QRect rect(0, 0, cvp->viewport()->width(), cvp->viewport()->height());
          stretchBuffer(rect);
        }
        //Otherwise we need to read from the cube
        else {
          stretchEntireCube(cvp);
        }

        //Set the global stretch to the current stretch
        p_viewportMap[cvp][cvp->redBand()-1]->globalStretch = p_viewportMap[cvp][cvp->redBand()-1]->currentStretch;
        p_viewportMap[cvp][cvp->greenBand()-1]->globalStretch = p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch;
        p_viewportMap[cvp][cvp->blueBand()-1]->globalStretch = p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch;
      }

      //Set the current stretch to the global stretch
      p_viewportMap[cvp][cvp->redBand()-1]->currentStretch = p_viewportMap[cvp][cvp->redBand()-1]->globalStretch;
      p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch = p_viewportMap[cvp][cvp->greenBand()-1]->globalStretch;
      p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch = p_viewportMap[cvp][cvp->blueBand()-1]->globalStretch;

      //Stretch the viewport with the red, green and blue band's global stretch
      cvp->stretchRGB(p_viewportMap[cvp][cvp->redBand()-1]->globalStretch,
                      p_viewportMap[cvp][cvp->greenBand()-1]->globalStretch,
                      p_viewportMap[cvp][cvp->blueBand()-1]->globalStretch);
    }

    //Update the stretch
    stretchChanged();
  }


  /** 
   * Does a global stretch for all the viewports.
   * 
   */
  void StretchTool::stretchGlobalAllViewports(){
    for( int i = 0; i < (int)cubeViewportList()->size(); i++) {
      CubeViewport *cvp = cubeViewportList()->at(i);
      //The viewport is in gray mode
      if (cvp->isGray()) {
        //If the global stretch does not exist for the gray band
        if(p_viewportMap[cvp][cvp->grayBand()-1]->globalStretch.Pairs() == 0) {
          //If the viewport buffer contains the entire cube, stretch the entire viewport
          if(cvp->grayBuffer()->hasEntireCube()) {
            QRect rect(0, 0, cvp->viewport()->width(), cvp->viewport()->height());
            stretchBuffer(rect);

            //Set the global stretch to the current stretch
            p_viewportMap[cvp][cvp->grayBand()-1]->globalStretch = p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch;
          }
          //Otherwise we need to read from the cube
          else {
            stretchEntireCube(cvp);
          }
        }
  
        //Set the current stretch to the global stretch
        p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch = p_viewportMap[cvp][cvp->grayBand()-1]->globalStretch;
  
        //Stretch the viewport with the gray band's global stretch
        cvp->stretchGray(p_viewportMap[cvp][cvp->grayBand()-1]->globalStretch);
      } 
      //Otherwise the viewport is in color mode
      else {
        Isis::Stretch redStretch = cvp->redStretch();
        Isis::Stretch greenStretch = cvp->greenStretch();
        Isis::Stretch blueStretch = cvp->blueStretch();
  
        //If the global stretch does not exist for the red, green or blue band
        if(p_viewportMap[cvp][cvp->redBand()-1]->globalStretch.Pairs() == 0 ||
           p_viewportMap[cvp][cvp->greenBand()-1]->globalStretch.Pairs() == 0 ||
           p_viewportMap[cvp][cvp->blueBand()-1]->globalStretch.Pairs() == 0) {
          //If the viewport buffer contains the entire cube, stretch the entire viewport
          if(cvp->redBuffer()->hasEntireCube()) {
            QRect rect(0, 0, cvp->viewport()->width(), cvp->viewport()->height());
            stretchBuffer(rect);

            //Set the global stretch to the current stretch
            p_viewportMap[cvp][cvp->redBand()-1]->globalStretch = p_viewportMap[cvp][cvp->redBand()-1]->currentStretch;
            p_viewportMap[cvp][cvp->greenBand()-1]->globalStretch = p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch;
            p_viewportMap[cvp][cvp->blueBand()-1]->globalStretch = p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch;
          }
          //Otherwise we need to read from the cube
          else {
            stretchEntireCube(cvp);
          }
        }
  
        //Set the current stretch to the global stretch
        p_viewportMap[cvp][cvp->redBand()-1]->currentStretch = p_viewportMap[cvp][cvp->redBand()-1]->globalStretch;
        p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch = p_viewportMap[cvp][cvp->greenBand()-1]->globalStretch;
        p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch = p_viewportMap[cvp][cvp->blueBand()-1]->globalStretch;
  
        //Stretch the viewport with the red, green and blue band's global stretch
        cvp->stretchRGB(p_viewportMap[cvp][cvp->redBand()-1]->globalStretch,
                        p_viewportMap[cvp][cvp->greenBand()-1]->globalStretch,
                        p_viewportMap[cvp][cvp->blueBand()-1]->globalStretch);
      }
    }

    //Update the stretch
    stretchChanged();
  }


  /** 
   * Does a globabl stretch for all the bands in the active
   * viewport.
   * 
   */
  void StretchTool::stretchGlobalAllBands() {
    CubeViewport *cvp = cubeViewport();
    if (cvp == NULL) return;
    for(int b = 0; b < cvp->cubeBands(); b++) {
      if(p_viewportMap[cvp][b]) {
        //Set the current stretch to the global stretch
        p_viewportMap[cvp][b]->currentStretch = p_viewportMap[cvp][b]->globalStretch;
      }
    }

    //Update the stretch
    stretchChanged();
  }


  /**
   * Does a regional stretch for the active viewport.
   * 
   */
  void StretchTool::stretchRegional() {
    CubeViewport *cvp = cubeViewport();
    if (cvp == NULL) return;

    //Stretch across the entire viewport
    QRect rect(0, 0, cvp->viewport()->width(), cvp->viewport()->height());
    stretchBuffer(rect);

    //Update the stretch
    stretchChanged();
  }

  /**
   * Allows the user to select the band to do the stretch on.
   * 
   * 
   * @param index 
   */
  void StretchTool::selectBandMinMax (int index) {
    if(index == p_stretchBand) return;

    if (index == 0) {
      p_stretchBand = Red;
      p_redButton->setChecked(true);
    }
    if (index == 1) {
      p_stretchBand = Green;
      p_greenButton->setChecked(true);
    }
    if (index == 2) {
      p_stretchBand = Blue;
      p_blueButton->setChecked(true);
    }

    updateTool();
  }

  /**
   * This method is called when the min/max text fields are changed.
   * 
   */
  void StretchTool::changeStretch () {
    CubeViewport *cvp = cubeViewport();
    if (cvp == NULL) return;

    // Make sure the user didn't enter bad min/max and if so fix it
    double min = p_stretchMinEdit->text().toDouble();
    double max = p_stretchMaxEdit->text().toDouble();
    if (min >= max || p_stretchMinEdit->text() == "" || p_stretchMaxEdit->text() == "") {
      updateTool();
      return;
    }

    //The viewport is in gray mode
    if (cvp->isGray()) {
      Isis::Stretch stretch = cvp->grayStretch();
      stretch.ClearPairs();
      stretch.AddPair(min,0.0);
      stretch.AddPair(max,255.0);
      p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch = stretch;
      
      cvp->stretchGray(stretch);
    }
    //Otherwise the viewport is in color mode
    else {
      Isis::Stretch redStretch = cvp->redStretch();
      Isis::Stretch greenStretch = cvp->greenStretch();
      Isis::Stretch blueStretch = cvp->blueStretch();

      if (p_stretchBand == Red) {
        redStretch.ClearPairs();
        redStretch.AddPair(min,0.0);
        redStretch.AddPair(max,255.0);

        p_viewportMap[cvp][cvp->redBand()-1]->currentStretch = redStretch;
      }
      if (p_stretchBand == Green) {
        greenStretch.ClearPairs();
        greenStretch.AddPair(min,0.0);
        greenStretch.AddPair(max,255.0);

        p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch = greenStretch;
      }
      if (p_stretchBand == Blue) {
        blueStretch.ClearPairs();
        blueStretch.AddPair(min,0.0);
        blueStretch.AddPair(max,255.0);

        p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch = blueStretch;
      }

      cvp->stretchRGB(redStretch,greenStretch,blueStretch);
    }
  }

  /** 
   * This method is called when the RubberBandTool is complete. It
   * will get a rectangle from the RubberBandTool and stretch
   * accordingly.
   * 
   */
  void StretchTool::rubberBandComplete() {
    CubeViewport *cvp = cubeViewport();
    if (cvp == NULL) return;
    if(!RubberBandTool::isValid()) return;

    QRect r = RubberBandTool::rectangle(); 
    //Return if the width or height is zero
    if(r.width() == 0 || r.height() == 0) return;

    stretchBuffer(r);

    //Update the stretch
    stretchChanged();  
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
   * Sets the stretch for all the bands in the active viewport to
   * the current stretch
   * 
   */
  void StretchTool::setStretchBands() {
    CubeViewport *cvp = cubeViewport();
    if(cvp == NULL) return;

    double min = p_stretchMinEdit->text().toDouble();
    double max = p_stretchMaxEdit->text().toDouble();

    for(int b = 0; b < cvp->cubeBands(); b++) {
       if(p_viewportMap[cvp][b]) {
        Isis::Stretch stretch = cvp->grayStretch();
        stretch.ClearPairs();
        stretch.AddPair(min,0.0);
        stretch.AddPair(max,255.0);
        p_viewportMap[cvp][b]->currentStretch = stretch;
      }     
    }
  }


  /** 
   * Sets the stretch for all the viewports to the current
   * stretch in the active viewport.
   * 
   */
  void StretchTool::setStretchAllViewports(){

    for ( int i = 0; i < (int)cubeViewportList()->size(); i++) {
      CubeViewport *cvp = cubeViewportList()->at(i);
      double min = p_stretchMinEdit->text().toDouble();
      double max = p_stretchMaxEdit->text().toDouble();

      //The viewport is in gray mode
      if (cvp->isGray()) {
        Isis::Stretch stretch = cvp->grayStretch();
        stretch.ClearPairs();
        stretch.AddPair(min,0.0);
        stretch.AddPair(max,255.0);
        p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch = stretch;

        cvp->stretchGray(stretch);
      }
      //Otherwise the viewport is in color mode
      else {
        Isis::Stretch redStretch = cvp->redStretch();
        Isis::Stretch greenStretch = cvp->greenStretch();
        Isis::Stretch blueStretch = cvp->blueStretch();
  
        if (p_stretchBand == Red) {
          redStretch.ClearPairs();
          redStretch.AddPair(min,0.0);
          redStretch.AddPair(max,255.0);
  
          p_viewportMap[cvp][cvp->redBand()-1]->currentStretch = redStretch;
        }
        if (p_stretchBand == Green) {
          greenStretch.ClearPairs();
          greenStretch.AddPair(min,0.0);
          greenStretch.AddPair(max,255.0);
  
          p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch = greenStretch;
        }
        if (p_stretchBand == Blue) {
          blueStretch.ClearPairs();
          blueStretch.AddPair(min,0.0);
          blueStretch.AddPair(max,255.0);
  
          p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch = blueStretch;
        }
  
        cvp->stretchRGB(redStretch,greenStretch,blueStretch);
      }
    }
  }


  /**
   * This method computes the stretch over a region using the viewport buffer.
   * 
   * @param rect 
   */
  void StretchTool::stretchBuffer(QRect rect) {
    CubeViewport *cvp = cubeViewport();
    if(cvp == NULL) return;

    //If the viewport is in gray mode
    if(cvp->isGray()) {
      //Get the statistics and histogram from the region
      Isis::Statistics stats = calculateStatisticsFromBuffer(cvp->grayBuffer(), rect);
      Isis::Histogram hist = calculateHistogramFromBuffer(cvp->grayBuffer(), rect, stats.BestMinimum(), stats.BestMaximum());

      Isis::Stretch stretch = cvp->grayStretch();
      stretch.ClearPairs();
      if (fabs(hist.Percent(0.5) - hist.Percent(99.5)) > DBL_EPSILON) {
        stretch.AddPair(hist.Percent(0.5), 0.0);
        stretch.AddPair(hist.Percent(99.5), 255.0);
      }
      else {
        stretch.AddPair(-DBL_MAX, 0.0);
        stretch.AddPair(DBL_MAX, 255.0);
      }

      CubeInfo *info = p_viewportMap[cvp][cvp->grayBand()-1];
      info->currentStretch = stretch;

      //If the info's histogram is null, create it - this is assuming that this will be the histogram for the entire band
      if(!info->hist) {
        info->min = stats.Minimum();
        info->max = stats.Maximum();
        info->hist = new Isis::Histogram(calculateHistogramFromBuffer(cvp->grayBuffer(), rect, hist.Percent(0.5), hist.Percent(99.5)));
      }

      cvp->stretchGray(stretch);
    }
    //Otherwise the viewport is in color mode
    else {
      //Get the statistics and histogram from the region for each band
      Isis::Statistics redStats = calculateStatisticsFromBuffer(cvp->redBuffer(), rect);
      Isis::Statistics greenStats = calculateStatisticsFromBuffer(cvp->greenBuffer(), rect);
      Isis::Statistics blueStats = calculateStatisticsFromBuffer(cvp->blueBuffer(), rect);

      Isis::Histogram redHist = calculateHistogramFromBuffer(cvp->redBuffer(), rect, redStats.BestMinimum(), redStats.BestMaximum());
      Isis::Histogram greenHist = calculateHistogramFromBuffer(cvp->greenBuffer(), rect, greenStats.BestMinimum(), greenStats.BestMaximum());
      Isis::Histogram blueHist = calculateHistogramFromBuffer(cvp->blueBuffer(), rect, blueStats.BestMinimum(), blueStats.BestMaximum());

      Isis::Stretch redStretch = cvp->redStretch();
      Isis::Stretch greenStretch = cvp->greenStretch();
      Isis::Stretch blueStretch = cvp->blueStretch();

      redStretch.ClearPairs();
      if (fabs(redHist.Percent(0.5) - redHist.Percent(99.5)) > DBL_EPSILON) {
        redStretch.AddPair(redHist.Percent(0.5), 0.0);
        redStretch.AddPair(redHist.Percent(99.5), 255.0);
      }
      else {
        redStretch.AddPair(-DBL_MAX, 0.0);
        redStretch.AddPair(DBL_MAX, 255.0);
      }

      greenStretch.ClearPairs();
      if (fabs(greenHist.Percent(0.5) - greenHist.Percent(99.5)) > DBL_EPSILON) {
        greenStretch.AddPair(greenHist.Percent(0.5), 0.0);
        greenStretch.AddPair(greenHist.Percent(99.5), 255.0);
      }
      else {
        greenStretch.AddPair(-DBL_MAX, 0.0);
        greenStretch.AddPair(DBL_MAX, 255.0);
      }

      blueStretch.ClearPairs();
      if (fabs(blueHist.Percent(0.5) - blueHist.Percent(99.5)) > DBL_EPSILON) {
        blueStretch.AddPair(blueHist.Percent(0.5), 0.0);
        blueStretch.AddPair(blueHist.Percent(99.5), 255.0);
      }
      else {
        blueStretch.AddPair(-DBL_MAX, 0.0);
        blueStretch.AddPair(DBL_MAX, 255.0);
      }

      CubeInfo *redInfo = p_viewportMap[cvp][cvp->redBand()-1];
      redInfo->currentStretch = redStretch;

      //If the red info's histogram is null, create it - this is assuming that this will be the histogram for the entire band
      if(!redInfo->hist) {
        redInfo->min = redStats.Minimum();
        redInfo->max = redStats.Maximum();
        redInfo->hist = new Isis::Histogram(calculateHistogramFromBuffer(cvp->redBuffer(), rect, redHist.Percent(0.5), redHist.Percent(99.5)));
      }

      CubeInfo *greenInfo = p_viewportMap[cvp][cvp->greenBand()-1];
      greenInfo->currentStretch = greenStretch;

      //If the green info's histogram is null, create it - this is assuming that this will be the histogram for the entire band
      if(!greenInfo->hist) {
        greenInfo->min = greenStats.Minimum();
        greenInfo->max = greenStats.Maximum();
        greenInfo->hist = new Isis::Histogram(calculateHistogramFromBuffer(cvp->greenBuffer(), rect, greenHist.Percent(0.5), greenHist.Percent(99.5)));
      }      

      CubeInfo *blueInfo = p_viewportMap[cvp][cvp->blueBand()-1];
      blueInfo->currentStretch = blueStretch;

      //If the blue info's histogram is null, create it - this is assuming that this will be the histogram for the entire band
      if(!blueInfo->hist) {
        blueInfo->min = blueStats.Minimum();
        blueInfo->max = blueStats.Maximum();
        blueInfo->hist = new Isis::Histogram(calculateHistogramFromBuffer(cvp->blueBuffer(), rect, blueHist.Percent(0.5), blueHist.Percent(99.5)));
      }

      cvp->stretchRGB(redStretch, greenStretch, blueStretch);
    }
  }

  /**
   * This method computes the stretch over the entire cube.
   * 
   * @param cvp 
   */
  void StretchTool::stretchEntireCube(CubeViewport *cvp) {
    //If the viewport is in gray mode
    if(cvp->isGray()) {
      //Get the statistics and histogram from the entire cube
      Isis::Statistics stats = calculateStatisticsFromCube(cvp->cube(), cvp->grayBand());
      Isis::Histogram hist = calculateHistogramFromCube(cvp->cube(), cvp->grayBand(), stats.BestMinimum(), stats.BestMaximum());

      Isis::Stretch stretch = cvp->grayStretch();
      stretch.ClearPairs();
      if (fabs(hist.Percent(0.5) - hist.Percent(99.5)) > DBL_EPSILON) {
        stretch.AddPair(hist.Percent(0.5), 0.0);
        stretch.AddPair(hist.Percent(99.5), 255.0);
      }
      else {
        stretch.AddPair(-DBL_MAX, 0.0);
        stretch.AddPair(DBL_MAX, 255.0);
      }

      CubeInfo *info = p_viewportMap[cvp][cvp->grayBand()-1];
      info->currentStretch = stretch;

      //If the info's histogram is null, create it - this is assuming that this will be the histogram for the entire band
      if(!info->hist) {
        info->min = stats.Minimum();
        info->max = stats.Maximum();
        info->hist = new Isis::Histogram(hist);
      }

      cvp->stretchGray(stretch);
    }
    //Otherwise the viewport is in color mode
    else {
      //Get the statistics and histogram from the entire cube for each band
      Isis::Statistics redStats = calculateStatisticsFromCube(cvp->cube(), cvp->redBand());
      Isis::Statistics greenStats = calculateStatisticsFromCube(cvp->cube(), cvp->greenBand());
      Isis::Statistics blueStats = calculateStatisticsFromCube(cvp->cube(), cvp->blueBand());

      Isis::Histogram redHist = calculateHistogramFromCube(cvp->cube(), cvp->redBand(), redStats.BestMinimum(), redStats.BestMaximum());
      Isis::Histogram greenHist = calculateHistogramFromCube(cvp->cube(), cvp->greenBand(), greenStats.BestMinimum(), greenStats.BestMaximum());
      Isis::Histogram blueHist = calculateHistogramFromCube(cvp->cube(), cvp->blueBand(), blueStats.BestMinimum(), blueStats.BestMaximum());

      Isis::Stretch redStretch = cvp->redStretch();
      Isis::Stretch greenStretch = cvp->greenStretch();
      Isis::Stretch blueStretch = cvp->blueStretch();

      redStretch.ClearPairs();
      if (fabs(redHist.Percent(0.5) - redHist.Percent(99.5)) > DBL_EPSILON) {
        redStretch.AddPair(redHist.Percent(0.5), 0.0);
        redStretch.AddPair(redHist.Percent(99.5), 255.0);
      }
      else {
        redStretch.AddPair(-DBL_MAX, 0.0);
        redStretch.AddPair(DBL_MAX, 255.0);
      }

      greenStretch.ClearPairs();
      if (fabs(greenHist.Percent(0.5) - greenHist.Percent(99.5)) > DBL_EPSILON) {
        greenStretch.AddPair(greenHist.Percent(0.5), 0.0);
        greenStretch.AddPair(greenHist.Percent(99.5), 255.0);
      }
      else {
        greenStretch.AddPair(-DBL_MAX, 0.0);
        greenStretch.AddPair(DBL_MAX, 255.0);
      }

      blueStretch.ClearPairs();
      if (fabs(blueHist.Percent(0.5) - blueHist.Percent(99.5)) > DBL_EPSILON) {
        blueStretch.AddPair(blueHist.Percent(0.5), 0.0);
        blueStretch.AddPair(blueHist.Percent(99.5), 255.0);
      }
      else {
        blueStretch.AddPair(-DBL_MAX, 0.0);
        blueStretch.AddPair(DBL_MAX, 255.0);
      }

      CubeInfo *redInfo = p_viewportMap[cvp][cvp->redBand()-1];
      redInfo->currentStretch = redStretch;

      //If the red info's histogram is null, create it - this is assuming that this will be the histogram for the entire band
      if(!redInfo->hist) {
        redInfo->min = redStats.Minimum();
        redInfo->max = redStats.Maximum();
        redInfo->hist = new Isis::Histogram(redHist);
      }

      CubeInfo *greenInfo = p_viewportMap[cvp][cvp->greenBand()-1];
      greenInfo->currentStretch = greenStretch;

      //If the green info's histogram is null, create it - this is assuming that this will be the histogram for the entire band
      if(!greenInfo->hist) {
        greenInfo->min = greenStats.Minimum();
        greenInfo->max = greenStats.Maximum();
        greenInfo->hist = new Isis::Histogram(greenHist);
      }      

      CubeInfo *blueInfo = p_viewportMap[cvp][cvp->blueBand()-1];
      blueInfo->currentStretch = blueStretch;

      //If the blue info's histogram is null, create it - this is assuming that this will be the histogram for the entire band
      if(!blueInfo->hist) {
        blueInfo->min = blueStats.Minimum();
        blueInfo->max = blueStats.Maximum();
        blueInfo->hist = new Isis::Histogram(blueHist);
      }

      cvp->stretchRGB(redStretch, greenStretch, blueStretch);
    }
  }

  /**
   * This method will calculate and return the statistics for a given cube and 
   * band. 
   * 
   * @param cube 
   * @param band 
   * 
   * @return Isis::Statistics 
   */
  Isis::Statistics StretchTool::calculateStatisticsFromCube(Isis::Cube *cube, int band) {
    Isis::Statistics stats;
    Isis::Brick brick(cube->Samples(), 1, 1,cube->PixelType());

    for (int line = 0; line < cube->Lines(); line++) {
      brick.SetBasePosition(0, line, band);
      cube->Read(brick);
      stats.AddData(brick.DoubleBuffer(), cube->Samples());
    }

    return stats;
  }

  /**
   * This method will calculate and return the statistics for a given region and 
   * viewport buffer. 
   * 
   * @param buffer 
   * @param rect 
   * 
   * @return Isis::Statistics 
   */
  Isis::Statistics StretchTool::calculateStatisticsFromBuffer(ViewportBuffer *buffer, QRect rect) {
    QRect dataArea = QRect(buffer->bufferXYRect().intersected( rect ) );
    Isis::Statistics stats;

    for(int y = dataArea.top(); !dataArea.isNull() && y <= dataArea.bottom(); y++) {
      const std::vector<double> &line = buffer->getLine(y - buffer->bufferXYRect().top()); 
      stats.AddData(&line.front() + (dataArea.left() - buffer->bufferXYRect().left()), dataArea.width()); 
    }

    return stats;
  }

  /**
   * This method will calculate and return the histogram for a given cube and 
   * band. 
   * 
   * @param cube 
   * @param band 
   * @param min 
   * @param max 
   * 
   * @return Isis::Histogram 
   */
  Isis::Histogram StretchTool::calculateHistogramFromCube(Isis::Cube *cube, int band, double min, double max) {
    Isis::Histogram hist(min, max);
    Isis::Brick brick(cube->Samples(), 1, 1,cube->PixelType());

    for (int line = 0; line < cube->Lines(); line++) {
      brick.SetBasePosition(0, line, band);
      cube->Read(brick);
      hist.AddData(brick.DoubleBuffer(), cube->Samples());
    }

    return hist;
  }

  /**
   * This method will calculate and return the histogram for a given region and 
   * viewport buffer. 
   * 
   * @param buffer 
   * @param rect 
   * @param min 
   * @param max 
   * 
   * @return Isis::Histogram 
   */
  Isis::Histogram StretchTool::calculateHistogramFromBuffer(ViewportBuffer *buffer, QRect rect, double min, double max) {
    QRect dataArea = QRect(buffer->bufferXYRect().intersected( rect ) );
    Isis::Histogram hist(min, max);

    for(int y = dataArea.top(); !dataArea.isNull() && y <= dataArea.bottom(); y++) {
      const std::vector<double> &line = buffer->getLine(y - buffer->bufferXYRect().top()); 
      hist.AddData(&line.front() + (dataArea.left() - buffer->bufferXYRect().left()), dataArea.width()); 
    }

    return hist;
  }


  /**
   * This method will add a new stretch to the stretch list and stretch combo box.
   * 
   * @param stretch 
   */
  void StretchTool::addStretch(QStretch *stretch) {
    p_stretchList.append(stretch);
    p_stretchBox->addItem(stretch->name());
  }

  /**
   * This method is called when the stretch combo box has changed.
   * 
   * @param index 
   */
  void StretchTool::stretchIndexChanged(int index) {
    //If the index is invalid return
    if(index < 0 || index >= p_stretchList.count()) return;

    CubeViewport *cvp = cubeViewport();
    if(cvp != NULL) {
      CubeInfo *info = NULL;

      //If the viewport is in gray mode
      if(cvp->isGray()) {
        info = p_viewportMap[cvp][cvp->grayBand()-1];
      }
      //Otherwise the viewport is in color mode
      else {
        if(p_stretchBand == Red) {
          info = p_viewportMap[cvp][cvp->redBand()-1];
        }
        else if(p_stretchBand == Green) {
          info = p_viewportMap[cvp][cvp->greenBand()-1];
        }
        else if(p_stretchBand == Blue) {
          info = p_viewportMap[cvp][cvp->blueBand()-1];
        }
      }

      //If the cube info exists 
      if(info) {
        QStretch *currentStretch = info->stretch;

        //If the info's stretch is not the same type as the selected index, set it to the selected index
        if(currentStretch->name() != p_stretchList[index]->name()) info->stretch = p_stretchList[index]->clone();

        //Set the stretch to the selected index
        setStretch(info->stretch);

        int index = p_stretchBox->findText(currentStretch->name());
        delete p_stretchList[index];
        p_stretchList[index] = currentStretch->clone();

        //If the info's stretch is not the same type as the selected index, delete it
        if(currentStretch->name() != p_stretchList[index]->name()) delete currentStretch;

        return;
      }
    }

    //Set the stretch to the selected index
    setStretch(p_stretchList[index]);
  }

  /**
   * This method sets the current stretch and disconnects the previous current 
   * stretch if necessary. 
   * 
   * @param stretch 
   */
  void StretchTool::setStretch(QStretch *stretch) {
    //If the current stretch is not NULL, disconnect it
    if(p_currentStretch) {
      disconnect(p_currentStretch, SIGNAL(update()), this, SLOT(updateStretch()));
      p_currentStretch->disconnectTable(p_pairsTable);

      p_stackedWidget->removeWidget(p_currentStretch->getParameters());
    }

    p_currentStretch = stretch;

    //Connect the current stretch
    connect(p_currentStretch, SIGNAL(update()), this, SLOT(updateStretch()));
    p_currentStretch->connectTable(p_pairsTable);

    p_stackedWidget->addWidget(p_currentStretch->getParameters());
    p_stackedWidget->setCurrentWidget(p_currentStretch->getParameters());

    CubeViewport *cvp = cubeViewport();
    if(cvp) {
      CubeInfo *info = NULL;

      if(cvp->isGray()) {
        info = p_viewportMap[cvp][cvp->grayBand()-1];
      }
      else {
        if(p_stretchBand == Red) {
          info = p_viewportMap[cvp][cvp->redBand()-1];
        }
        else if(p_stretchBand == Green) {
          info = p_viewportMap[cvp][cvp->greenBand()-1];
        }
        else if(p_stretchBand == Blue) {
          info = p_viewportMap[cvp][cvp->blueBand()-1];
        }
      }

      //If the cube info is not NULL, set its min/max to the info's min/max
      if(info) p_currentStretch->setMinMax(info->hist->Minimum(), info->hist->Maximum());
    }
  }

  /**
   * This method is called when the current stretch is updated.
   * 
   */
  void StretchTool::updateStretch() {
    Isis::Stretch stretch = p_currentStretch->stretch();
    CubeViewport *cvp = cubeViewport();
    if(cvp != NULL) {
      CubeInfo *info = NULL;

      //If the viewport is in gray mode
      if(cvp->isGray()) {
        info = p_viewportMap[cvp][cvp->grayBand()-1];
        //Update the gray histogram's stretch curve
        p_grayHistogram->setStretch(stretch);
      }
      else {
        //Otherwise the viewport is in color mode
        if(p_stretchBand == Red) {
          info = p_viewportMap[cvp][cvp->redBand()-1];
          //Update the red histogram's stretch curve
          p_redHistogram->setStretch(stretch);

          //If the green band is the same as the red band, 
          //update the green histogram's stretch curve
          if(cvp->greenBand() == cvp->redBand()) {
            p_greenHistogram->setStretch(stretch);
          }

          //If the blue band is the same as the red band, 
          //update the blue histogram's stretch curve
          if(cvp->blueBand() == cvp->redBand()) {
            p_blueHistogram->setStretch(stretch);
          }
        }
        else if(p_stretchBand == Green) {
          //Update the green histogram's stretch curve
          info = p_viewportMap[cvp][cvp->greenBand()-1];
          p_greenHistogram->setStretch(stretch);

          //If the red band is the same as the green band, 
          //update the red histogram's stretch curve
          if(cvp->redBand() == cvp->greenBand()) {
            p_redHistogram->setStretch(stretch);
          }

          //If the blue band is the same as the green band, 
          //update the blue histogram's stretch curve
          if(cvp->blueBand() == cvp->greenBand()) {
            p_blueHistogram->setStretch(stretch);
          }
        }
        else if(p_stretchBand == Blue) {
          //Update the blue histogram's stretch curve
          info = p_viewportMap[cvp][cvp->blueBand()-1];
          p_blueHistogram->setStretch(stretch);

          //If the red band is the same as the blue band, 
          //update the red histogram's stretch curve
          if(cvp->redBand() == cvp->blueBand()) {
            p_redHistogram->setStretch(stretch);
          }

          //If the green band is the same as the blue band, 
          //update the green histogram's stretch curve
          if(cvp->greenBand() == cvp->blueBand()) {
            p_greenHistogram->setStretch(stretch);
          }
        }
      }

      info->currentStretch = stretch;
    }

    //Update the table with the current stretch's stretch pairs
    p_pairsTable->setRowCount(stretch.Pairs());

    for(int i = 0; i < stretch.Pairs(); i++) {
      QTableWidgetItem *inputItem = new QTableWidgetItem(QString("%1").arg(stretch.Input(i)));
      inputItem->setTextAlignment(Qt::AlignCenter);
      QTableWidgetItem *outputItem = new QTableWidgetItem(QString("%1").arg(stretch.Output(i)));
      outputItem->setTextAlignment(Qt::AlignCenter);

      p_pairsTable->setItem(i, 0, inputItem);
      p_pairsTable->setItem(i, 1, outputItem);
    }

    //Update the stretch
    stretchChanged();
  }

  /**
   * This method is called when the gray histogram's range has changed.
   * 
   * @param min 
   * @param max 
   */
  void StretchTool::grayRangeChanged(const double min, const double max) {
    CubeViewport *cvp = cubeViewport();
    if(cvp != NULL) {
      CubeInfo *info = p_viewportMap[cvp][cvp->grayBand()-1];
      if(info->hist) delete info->hist;

      //Calculate a new histogram with the given range
      QRect rect(0, 0, cvp->viewport()->width(), cvp->viewport()->height());
      info->hist = new Isis::Histogram(calculateHistogramFromBuffer(cvp->grayBuffer(), rect, min, max));

      p_grayHistogram->setHistogram(*info->hist); 

      info->stretch->setMinMax(min, max);
    }
  }

  /**
   * This method is called when the red histogram's range has changed.
   * 
   * @param min 
   * @param max 
   */
  void StretchTool::redRangeChanged(const double min, const double max) {
    CubeViewport *cvp = cubeViewport();
    if(cvp != NULL) {
      CubeInfo *info = p_viewportMap[cvp][cvp->redBand()-1];
      if(info->hist) delete info->hist;

      //Calculate a new histogram with the given range
      QRect rect(0, 0, cvp->viewport()->width(), cvp->viewport()->height());
      info->hist = new Isis::Histogram(calculateHistogramFromBuffer(cvp->redBuffer(), rect, min, max));

      p_redHistogram->setHistogram(*info->hist); 

      info->stretch->setMinMax(min, max);

      if(p_stretchBand != Red) {
        p_redHistogram->setStretch(info->stretch->stretch());
        info->currentStretch = info->stretch->stretch();
        stretchChanged();
      }

      //If the green band is the same as the red band we need to update it's histogram/stretch
      if(cvp->greenBand() == cvp->redBand()) {
        p_greenHistogram->setMinMax(min, max);
        p_greenHistogram->setHistogram(*info->hist);
        p_greenHistogram->setStretch(info->stretch->stretch());
      }

      //If the blue band is the same as the red band we need to update it's histogram/stretch
      if(cvp->blueBand() == cvp->redBand()) {
        p_blueHistogram->setMinMax(min, max);
        p_blueHistogram->setHistogram(*info->hist);
        p_blueHistogram->setStretch(info->stretch->stretch());
      }
    }
  }

  /**
   * This method is called when the green histogram's range has changed.
   * 
   * @param min 
   * @param max 
   */
  void StretchTool::greenRangeChanged(const double min, const double max) {
    CubeViewport *cvp = cubeViewport();
    if(cvp != NULL) {
      CubeInfo *info = p_viewportMap[cvp][cvp->greenBand()-1];
      if(info->hist) delete info->hist;

      //Calculate a new histogram with the given range
      QRect rect(0, 0, cvp->viewport()->width(), cvp->viewport()->height());
      info->hist = new Isis::Histogram(calculateHistogramFromBuffer(cvp->greenBuffer(), rect, min, max));

      p_greenHistogram->setHistogram(*info->hist); 

      info->stretch->setMinMax(min, max);

      if(p_stretchBand != Green) {
        p_greenHistogram->setStretch(info->stretch->stretch());
        info->currentStretch = info->stretch->stretch();
        stretchChanged();
      }

      //If the green band is the same as the red band we need to update it's histogram/stretch
      if(cvp->redBand() == cvp->greenBand()) {
        p_redHistogram->setMinMax(min, max);
        p_redHistogram->setHistogram(*info->hist);
        p_redHistogram->setStretch(info->stretch->stretch());
      }

      //If the blue band is the same as the red band we need to update it's histogram/stretch
      if(cvp->blueBand() == cvp->greenBand()) {
        p_blueHistogram->setMinMax(min, max);
        p_blueHistogram->setHistogram(*info->hist);
        p_blueHistogram->setStretch(info->stretch->stretch());
      }
    }
  }

  /**
   * This method is called when the blue histogram's range has changed.
   * 
   * @param min 
   * @param max 
   */
  void StretchTool::blueRangeChanged(const double min, const double max) {
    CubeViewport *cvp = cubeViewport();
    if(cvp != NULL) {
      CubeInfo *info = p_viewportMap[cvp][cvp->blueBand()-1];
      if(info->hist) delete info->hist;

      //Calculate a new histogram with the given range
      QRect rect(0, 0, cvp->viewport()->width(), cvp->viewport()->height());
      info->hist = new Isis::Histogram(calculateHistogramFromBuffer(cvp->blueBuffer(), rect, min, max));

      p_blueHistogram->setHistogram(*info->hist); 

      info->stretch->setMinMax(min, max);

      if(p_stretchBand != Blue) {
        p_blueHistogram->setStretch(info->stretch->stretch());
        info->currentStretch = info->stretch->stretch();
        stretchChanged();
      }

      //If the green band is the same as the red band we need to update it's histogram/stretch
      if(cvp->redBand() == cvp->blueBand()) {
        p_redHistogram->setMinMax(min, max);
        p_redHistogram->setHistogram(*info->hist);
        p_redHistogram->setStretch(info->stretch->stretch());
      }

      //If the blue band is the same as the red band we need to update it's histogram/stretch
      if(cvp->greenBand() == cvp->blueBand()) {
        p_greenHistogram->setMinMax(min, max);
        p_greenHistogram->setHistogram(*info->hist);
        p_greenHistogram->setStretch(info->stretch->stretch());
      }
    }
  }

  /**
   * This method is called when the red radio button is toggled.
   * 
   * @param toggled 
   */
  void StretchTool::setRedStretch(bool toggled) {
    if(!toggled) return;

    //Set the current stretch band to red
    p_stretchBandComboBox->setCurrentIndex(Red);
  }

  /**
   * This method is called when the green radio button is toggled.
   * 
   * @param toggled 
   */
  void StretchTool::setGreenStretch(bool toggled) {
    if(!toggled) return;

    //Set the current stretch band to green
    p_stretchBandComboBox->setCurrentIndex(Green);
  }

  /**
   * This method is called when the blue radio button is toggled.
   * 
   * @param toggled 
   */
  void StretchTool::setBlueStretch(bool toggled) {
    if(!toggled) return;

    //Set the current stretch band to blue
    p_stretchBandComboBox->setCurrentIndex(Blue);
  }

  /**
   * This method is called when the flash button is pressed on the advanced dialog
   * and sets the viewport's stretch to the global stretch.
   * 
   */
  void StretchTool::flash() {
    CubeViewport *cvp = cubeViewport();
    if(cvp != NULL) {
      if(cvp->isGray()) {
        p_viewportMap[cvp][cvp->grayBand()-1]->currentStretch = p_viewportMap[cvp][cvp->grayBand()-1]->stretch->stretch();
      }
      else {
        p_viewportMap[cvp][cvp->redBand()-1]->currentStretch = p_viewportMap[cvp][cvp->redBand()-1]->stretch->stretch();
        p_viewportMap[cvp][cvp->greenBand()-1]->currentStretch = p_viewportMap[cvp][cvp->greenBand()-1]->stretch->stretch();
        p_viewportMap[cvp][cvp->blueBand()-1]->currentStretch = p_viewportMap[cvp][cvp->blueBand()-1]->stretch->stretch();
      }

      //Update the stretch
      stretchChanged();
    }
  }

  /**
   * This method will return a DN value corresponding to the percentage from the 
   * current band's histogram. 
   * 
   * @param percent 
   * 
   * @return double 
   */
  double StretchTool::getHistogramDN(double percent) const {
    if(percent < 0.0) percent = 0.0;
    if(percent > 100.0) percent = 100.0;

    CubeViewport *cvp = cubeViewport();
    if(cvp != NULL) {
      CubeInfo *info = NULL;

      if(cvp->isGray()) {
        info = p_viewportMap[cvp][cvp->grayBand()-1];
      }
      else {
        if(p_stretchBand == Red) {
          info = p_viewportMap[cvp][cvp->redBand()-1];
        }
        else if(p_stretchBand == Green) {
          info = p_viewportMap[cvp][cvp->greenBand()-1];
        }
        else if(p_stretchBand == Blue) {
          info = p_viewportMap[cvp][cvp->blueBand()-1];
        }
      }

      return info->hist->Percent(percent);
    }

    //If the viewport is NULL return 0
    return 0.0;
  }

  /**
   * This method will return a percent value corresponding to the DN in the 
   * current band's histogram. 
   * 
   * @param dn 
   * 
   * @return double 
   */
  double StretchTool::getHistogramPercent(double dn) const {
    CubeViewport *cvp = cubeViewport();
    if(cvp != NULL) {
      Isis::Histogram *hist = NULL;

      if(cvp->isGray()) {
        hist = p_viewportMap[cvp][cvp->grayBand()-1]->hist;
      }
      else {
        if(p_stretchBand == Red) {
          hist = p_viewportMap[cvp][cvp->redBand()-1]->hist;
        }
        else if(p_stretchBand == Green) {
          hist = p_viewportMap[cvp][cvp->greenBand()-1]->hist;
        }
        else if(p_stretchBand == Blue) {
          hist = p_viewportMap[cvp][cvp->blueBand()-1]->hist;
        }
      }


      Isis::BigInt currentPixels = 0;
      double currentPercent;
  
      for (int i = 0; i < hist->Bins(); i++) {
        currentPixels += hist->BinCount(i);
        currentPercent = (double) currentPixels / (double) hist->ValidPixels() * 100.0;

        if (hist->BinMiddle(i) >= dn) {
          return currentPercent;
        }
      }
  
      return 100.0;
    }

    //If the viewport is NULL return 0
    return 0.0;
  }

  /**
   * This method will save the current stretch's stretch pairs to the specified
   * file name.
   * 
   */
  void StretchTool::savePairsAs() {
    QString fn = QFileDialog::getSaveFileName((QWidget*)parent(),
                                            "Choose filename to save under",
                                            ".",
                                            "Text Files (*.txt)");
    QString filename;

    //Make sure the filename is valid
    if (!fn.isEmpty()) {
      if(!fn.endsWith(".txt")) {
        filename = fn + ".txt";
      } else {
        filename = fn;
      }
    }
    //The user cancelled, or the filename is empty
    else {
      return;
    }

    p_currentFile.setFileName( filename );

    p_saveButton->setEnabled(true);
    savePairs();
  }

  /** 
   * This method will save the current stretch's stretch pairs to the current
   * file name if it exists.
   * 
   */
  void StretchTool::savePairs () {
    if(p_currentFile.fileName().isEmpty()) return;

    bool success = p_currentFile.open(QIODevice::WriteOnly);
    if(!success) {
      QMessageBox::critical((QWidget *)parent(),
                   "Error", "Cannot open file, please check permissions");
      p_currentFile.setFileName("");
      p_saveButton->setDisabled(true);
      return;
    }

    QString currentText;
    QTextStream stream( &p_currentFile );

    Isis::Stretch stretch = p_currentStretch->stretch();

    //Add the pairs to the file
    stream << stretch.Text().c_str() << endl;

    p_currentFile.close();
  }
}
