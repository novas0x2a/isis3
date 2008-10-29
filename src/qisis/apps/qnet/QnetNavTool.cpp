#include <QApplication>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QPushButton>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QGridLayout>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QTabWidget>

#include "PolygonTools.h"

#include "QnetNavTool.h"
#include "QnetPolygonImagesFilter.h"
#include "QnetPolygonSizeFilter.h"
#include "QnetPolygonPointsFilter.h"
#include "QnetCubeNameFilter.h"
#include "QnetCubePointsFilter.h"
#include "QnetCubeDistanceFilter.h"
#include "QnetPointIdFilter.h"
#include "QnetPointErrorFilter.h"
#include "QnetPointTypeFilter.h"
#include "QnetPointRangeFilter.h"
#include "QnetPointDistanceFilter.h"
#include "QnetPointMeasureFilter.h"
#include "QnetPointImagesFilter.h"
#include "MainWindow.h"
#include "CubeViewport.h"
#include "Workspace.h"
#include "ImageOverlap.h"
#include "TieTool.h"
#include "QnetPlotTool.h"

#include "qnet.h"
using namespace Qisis::Qnet;
using namespace std;

namespace Qisis {
  /**
   * Consructs the Navigation Tool window
   * 
   * @param parent The parent widget for the navigation tool
   */
  QnetNavTool::QnetNavTool (QWidget *parent) : Qisis::Tool(parent) {
    createNavigationDialog(parent);
    g_activePoints.clear();
    g_activeOverlaps.clear();
    g_activeImages.clear();
  }

  /**
   * Creates and shows the dialog box for the navigation tool
   * 
   * @param parent The parent widget for the navigation dialopg
   */
  void QnetNavTool::createNavigationDialog(QWidget *parent) {
    // Create the combo box selector
    p_listCombo = new QComboBox ();
    p_listCombo->addItem("Points");
//    p_listCombo->addItem("Overlaps");
    p_listCombo->addItem("Cubes");
    p_listBox = new QListWidget();
    p_listBox->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect (p_listBox,SIGNAL(itemSelectionChanged()),this,SLOT(selectionChanged()));

    // Create the filter area
    QLabel *filterLabel = new QLabel ("Filters");
    filterLabel->setAlignment(Qt::AlignHCenter);
    p_filterStack = new QStackedWidget ();
                           
    connect(p_listCombo,SIGNAL(activated(int)),
            p_filterStack,SLOT(setCurrentIndex(int)));
    connect(p_listCombo,SIGNAL(activated(int)),
            this,SLOT(updateList()));
    connect(p_listCombo,SIGNAL(activated(int)),this,SLOT(enableTie()));

    // Create action options
    QWidget *actionArea = new QWidget();
    QPushButton *load = new QPushButton("View Cube(s)",actionArea);
    load->setToolTip("Open Selected Images");
    load->setWhatsThis("<b>Function: </b> Opens all selected images, or images \
                        that are associated with the given point or overlap.  \
                        <p><b>Hint: </b> You can select more than one item in \
                        the list by using the shift or control key.</p>");
    connect(load,SIGNAL(clicked()),this,SLOT(load()));
    p_tie = new QPushButton("Modify Point",actionArea);
    p_tie->setToolTip("Modify Selected Point");
    p_tie->setWhatsThis("<b>Function: </b> Opens the tie tool to modify the \
                         selected point from the list.  This option is only \
                         available when the nav tool is in point mode");
    connect(p_tie,SIGNAL(clicked()),this,SLOT(tie()));
    p_filter = new QPushButton("Filter",actionArea);
    p_filter->setToolTip("Filter Current List");
    p_filter->setWhatsThis("<b>Function: </b> Filters the current list by user \
                            specifications made in the selected filter. \
                            <p><b>Note: </b> Any filter options selected in a \
                            filter that is not showing will be ignored.</p>"); 
    connect(p_filter,SIGNAL(clicked()),this,SLOT(filter()));
    QPushButton *reset = new QPushButton("Show All",actionArea);
    reset->setToolTip("Reset the Current List to show all the values in the list");
    reset->setWhatsThis("<b>Function: </b> Resets the list of points, \
                         overlaps, or images to the complete initial list.  \
                         Any filtering that has been done will be lost.");
    connect(reset,SIGNAL(clicked()),this,SLOT(updateList()));
    connect(reset,SIGNAL(clicked()),this,SLOT(resetFilter()));

    // Set up layout
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(load);
    layout->addWidget(p_tie);
    layout->addWidget(p_filter);
    layout->addWidget(reset);

    // Create filter stacked widgets
    createFilters();
    p_filterStack->adjustSize();
  
    // Set up the main window
    p_navDialog = new QDialog(parent);
    p_navDialog->setWindowTitle("Control Network Navigator");

    // Layout everything in the dialog
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(p_listCombo,0,0);
    gridLayout->addWidget(filterLabel,0,1);
    gridLayout->addWidget(p_listBox,1,0);
    gridLayout->addWidget(p_filterStack,1,1);
    gridLayout->addLayout(layout,2,0,1,2);
    p_navDialog->setLayout(gridLayout);

    // View the dialog
    p_navDialog->setShown(true);
  }

  /**
   * Sets up the tabbed widgets for the different types of filters available
   * 
   * @internal
   * @history  2007-06-05 Tracie Sucharski - Added enumerators for the filter
   *                         indices to make it easier to re-order filters.
   *                         Also, re-ordered the filters to put commonly used
   *                         first. Comment out overlap/polygon code temporarily.
   */
  void QnetNavTool::createFilters() {
    // Set up the point filters
    QTabWidget *pointFilters = new QTabWidget();

    QWidget *errorFilter = new QnetPointErrorFilter();
    connect(errorFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    pointFilters->insertTab(Errors,errorFilter,"Error");
    pointFilters->setTabToolTip(Errors,"Filter Points by Error");
    pointFilters->setTabWhatsThis(Errors,
                                    "<b>Function: </b> Filter points list by \
                                     the bundle adjust error value at each  \
                                     point.  You can filter for points that \
                                     have an error greater than, or less than \
                                     the entered value.");

    QWidget *ptIdFilter = new QnetPointIdFilter();
    connect(ptIdFilter,SIGNAL(filteredListModified()),
            this,SLOT(filterList()));
    pointFilters->insertTab(Id,ptIdFilter,"Point ID");
    pointFilters->setTabToolTip(Id,"Filter Points by PointID");

    QWidget *ptImageFilter = new QnetPointImagesFilter();
    connect(ptImageFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    pointFilters->insertTab(NumberImages,ptImageFilter,"Images");
    pointFilters->setTabToolTip(NumberImages,"Filter Points by Number of Images");
    pointFilters->setTabWhatsThis(NumberImages,"<b>Function: </b> Filter points list \
                                     by the number of images that are in  \
                                     each point. You can filter for         \
                                     points that have more than the given   \
                                     number of images, or less than the \
                                     given number of images.  Points with   \
                                     the exact number of images specified \
                                     will not be included in the filtered \
                                     list.");
    QWidget *typeFilter = new QnetPointTypeFilter();
    connect(typeFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    pointFilters->insertTab(Type,typeFilter,"Type");
    pointFilters->setTabToolTip(Type,"Filter Points by Type");
    pointFilters->setTabWhatsThis(Type,"<b>Function: </b> Filter points list by \
                                     the type of each control point");
    QWidget *rangeFilter = new QnetPointRangeFilter();
    connect(rangeFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    pointFilters->insertTab(LatLonRange,rangeFilter,"Range");
    pointFilters->setTabToolTip(LatLonRange,"Filter Points by Range");
    pointFilters->setTabWhatsThis(LatLonRange,"<b>Function: </b> Filters out points \
                                     that are within a user set range lat/lon \
                                     range.");
    QWidget *ptDistFilter = new QnetPointDistanceFilter();
    connect(ptDistFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    pointFilters->insertTab(Distance,ptDistFilter,"Distance");
    pointFilters->setTabToolTip(Distance,"Filter Points by Distance");
    pointFilters->setTabWhatsThis(Distance,
                                    "<b>Function: </b> Filter points list by \
                                     the distance from each point to the point \
                                     closest to it.");
    QWidget *measureFilter = new QnetPointMeasureFilter();
    connect(measureFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    pointFilters->insertTab(MeasureType,measureFilter,"Measure");
    pointFilters->setTabToolTip(MeasureType,"Filter Points by Measure Type");
    pointFilters->setTabWhatsThis(MeasureType,
                                    "<b>Function: </b> Filter points list by \
                                     the type of measures they contain. If one \
                                     or more measure from a point is found to \
                                     match a selected measure type, the point \
                                     will be left in the filtered list.  More \
                                     than one measure type can be selected");
#if 0
    // Set up the polygon filters
    QTabWidget *polygonFilters = new QTabWidget();
    QWidget *sizeFilter = new QnetPolygonSizeFilter();
    connect(sizeFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    polygonFilters->addTab(sizeFilter,"Size");
    polygonFilters->setTabToolTip(0,"Filter Overlaps by Area");
 /*   polygonFilters->setTabWhatsThis(0,"<b>Function: </b> Filter overlaps list
                                       \ by the area of the overlap (in        \
                                        meters). You can filter for overlaps   \
                                       that are greater than, or less than the \
                                       given area.");                         */
    QWidget *polyPtsFilter = new QnetPolygonPointsFilter();
    connect(polyPtsFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    polygonFilters->addTab(polyPtsFilter,"Points");
    polygonFilters->setTabToolTip(1,"Filter Overlaps by Number of Points");
  /*  polygonFilters->setTabWhatsThis(1,"<b>Function: </b> Filter overlaps list \
                                       by the number of points that are in each \
                                       overlap. You can filter for overlaps \
                                       that have more a given number of points, \
                                       or less than a given number of points.  \
                                       Overlaps with the exact number of points \
                                       specified will not be included in the \
                                       filtered list.");   */
    QWidget *polyImageFilter = new QnetPolygonImagesFilter();
    connect(polyImageFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    polygonFilters->addTab(polyImageFilter,"Images");
    polygonFilters->setTabToolTip(2,"Filter Overlaps by Number of Images");
 /*   polygonFilters->setTabWhatsThis(2,"<b>Function: </b> Filter overlaps list
                                       \ by the number of images that are in  \
                                       each overlap. You can filter for \
                                       overlaps that have more than the given \
                                       number of images, or less than the \
                                       given number of images.  Overlaps with \
                                       the exact number of images specified \
                                       will not be included in the filtered \
                                       list.");     */
#endif
    // Set up the cube filters
    QTabWidget *cubeFilters = new QTabWidget();

    QWidget *cubeNameFilter = new QnetCubeNameFilter();
    connect(cubeNameFilter,SIGNAL(filteredListModified()),
            this,SLOT(filterList()));
    cubeFilters->insertTab(Name,cubeNameFilter,"Cube Name");
    cubeFilters->setTabToolTip(Name,"Filter Images by Cube Name");

    QWidget *cubePtsFilter = new QnetCubePointsFilter();
    connect(cubePtsFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    cubeFilters->insertTab(NumberPoints,cubePtsFilter,"Points");
    cubeFilters->setTabToolTip(NumberPoints,"Filter Images by Number of Points");
    cubeFilters->setTabWhatsThis(NumberPoints,
                                   "<b>Function: </b> Filter images list by \
                                    the number of points that are in each \
                                    image. You can filter for images that have \
                                    more than the given number of points, or \
                                    less than the given number of point.  \
                                    Images with the exact number of points \
                                    specified will not be included in the \
                                    filtered list.");
    QWidget *cubeDistFilter = new QnetCubeDistanceFilter();
    connect(cubeDistFilter,SIGNAL(filteredListModified()),this,SLOT(filterList()));
    cubeFilters->insertTab(PointDistance,cubeDistFilter,"Distance");
    cubeFilters->setTabToolTip(PointDistance,"Filter Images by Distance");
    cubeFilters->setTabWhatsThis(PointDistance,
                                   "<b>Function: </b> Filter images list by \
                                    the shortest distance between two points \
                                    in the image.");
    
    // Add widgets to the filter stack
    p_filterStack->addWidget(pointFilters);
//    p_filterStack->addWidget(polygonFilters);
    p_filterStack->addWidget(cubeFilters);
  }

  /**
   * Updates the list box with whatever is in the global lists
   * 
   * @internal
   * @history  2007-06-05 Tracie Sucharski - Use enumerators to test which
   *                         filter is chosen.  Comment overlap/polygon code
   *                         temporarily.
   */
  void QnetNavTool::updateList() {
    // Dont do anything if there are no cubes loaded
    if (g_serialNumberList == NULL) return;

    // Clear the old list and filtered lists and update with the entire list
    p_listBox->setCurrentRow(-1);
    p_listBox->clear();
    g_filteredPoints.clear();
//    g_filteredOverlaps.clear();
    g_filteredImages.clear();

    // We are dealing with points so output the point numbers
    if (p_listCombo->currentIndex() == Points) {
      for (int i=0; i<g_controlNetwork->Size(); i++) {
        string cNetId = (*g_controlNetwork)[i].Id();
        QString itemString = cNetId.c_str();
        p_listBox->insertItem(i,itemString);
        int images = (*g_controlNetwork)[i].Size();
        p_listBox->item(i)->setToolTip(QString::number(images)+" image(s) in point");
      }
    }
    // We are dealing with overlaps so output the overlap number
#if 0
    else if (p_listCombo->currentIndex() == 1) {
      for (int i=0; i<g_imageOverlap->Size(); i++) {
        QString itemString = "Overlap " + QString::number(i);
        p_listBox->insertItem(i,itemString);
        int images = (*g_imageOverlap)[i]->Size();
        p_listBox->item(i)->setToolTip(QString::number(images)+" image(s) in overlap");
      }
    }
#endif
    // We are dealing with images so output the cube names
    else if (p_listCombo->currentIndex() == Cubes) {
      for (int i=0; i<g_serialNumberList->Size(); i++) {
        Isis::Filename filename = Isis::Filename(g_serialNumberList->Filename(i));
        string tempFilename = filename.Name();
        p_listBox->insertItem(i,tempFilename.c_str());
      }
    }
  }

  /**
   * Resets the visible filter to the default vaues
   */
  void QnetNavTool::resetFilter() {

  }

  /**
   * Updates the list with a new list from one of the filters
   * 
   * @internal
   * @history  2007-06-05 Tracie Sucharski - Use enumerators for the filter
   *                         indices.  Comment out overlap/polygon code temporarily.
   * 
   */
  void QnetNavTool::filterList() {
    // Dont do anything if there are no cubes loaded
    if (g_serialNumberList == NULL) return;

    // Clears the old list and puts the filtered list in its place
    p_listBox->setCurrentRow(-1);
    p_listBox->clear();

    // We are dealing with points so output the point numbers
    if (p_listCombo->currentIndex() == Points) {
      for (int i=0; i<g_filteredPoints.size(); i++) {
        string cNetId = (*g_controlNetwork)[g_filteredPoints[i]].Id();
        QString itemString = cNetId.c_str();
        p_listBox->insertItem(i,itemString);
        int images = (*g_controlNetwork)[g_filteredPoints[i]].Size();
        p_listBox->item(i)->setToolTip(QString::number(images)+" image(s) in point");
      }
    }
#if 0
    // We are dealing with overlaps so write out the overlap numbers
    else if (p_listCombo->currentIndex() == 1) {
      for (int i=0; i<g_filteredOverlaps.size(); i++) {
        QString itemString = "Overlap " + QString::number(g_filteredOverlaps[i]);
        p_listBox->insertItem(i,itemString);
        int images = (*g_imageOverlap)[g_filteredOverlaps[i]]->Size();
        p_listBox->item(i)->setToolTip(QString::number(images)+" image(s) in overlap");
      }
    }
#endif
    // We are dealing with images so write out the cube names
    else if (p_listCombo->currentIndex() == Cubes) {
      for (int i=0; i<g_filteredImages.size(); i++) {
        Isis::Filename filename = Isis::Filename(g_serialNumberList->Filename(g_filteredImages[i]));
        string tempFilename = filename.Name();
        p_listBox->insertItem(i,tempFilename.c_str());
      }
    }
  }

  /**
   * Tells the filetool to load an image, point, or overlap
   * 
   * @internal
   * @history  2007-06-05 Tracie Sucharski - Use enumerators for the filter
   *                         indices.  Comment out overlap/polygon code temporarily.
   * 
   */
  void QnetNavTool::load() {
    // Dont do anything if no cubes are loaded
    if (g_serialNumberList == NULL) return;

    int index = p_listBox->currentRow();
    if (index < 0) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No file selected to load");
      return;
    }

    // TODO: How do we handle them opening 2 overlaps or points that have the 
    //       cube in them, open 2 of the same file or only open 1???
    QList<QListWidgetItem*> selected = p_listBox->selectedItems();
    for (int i=0; i<selected.size(); i++) {
      int index = p_listBox->row(selected[i]);
      // Tell the filetool to load all images for the given point
      if (p_listCombo->currentIndex() == Points) {
        if (g_filteredPoints.size() == 0)  {
          emit loadPoint(&((*g_controlNetwork)[index]));
        }
        else {
          emit loadPoint(&((*g_controlNetwork)[g_filteredPoints[index]]));
        }
      }
#if 0
      // Tell the filetool to load all images for the given overlap
      else if (p_listCombo->currentIndex() == 1) {
        if (g_filteredOverlaps.size() == 0) {
          emit loadOverlap((*g_imageOverlap)[index]);
        }
        else {
          emit loadOverlap((*g_imageOverlap)[g_filteredOverlaps[index]]);
        }
      }
#endif
      // Tell the filetool to load the given image
      else if (p_listCombo->currentIndex() == Cubes) {
        if (g_filteredImages.size() == 0) {
          string serialNumberList = (*g_serialNumberList).SerialNumber(index);
          QString sn = serialNumberList.c_str();
          emit loadImage(sn);
        }
        else {
          string serialNumberList = (*g_serialNumberList).SerialNumber(g_filteredImages[index]);
          QString sn = serialNumberList.c_str();
          emit loadImage(sn);
        }
      }
    }
  }

  /**
   * Calls the tie tool for the given control point
   */
  void QnetNavTool::tie() {
    int index = p_listBox->currentRow();
    if (index < 0) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No point selected to modify");
      return;
    }

    QList<QListWidgetItem*> selected = p_listBox->selectedItems();
    if (selected.size() > 1) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Only one point can be modified at a time");
      return;
    }
    index = p_listBox->row(selected[0]);
    if (g_filteredPoints.size() == 0)  {
      emit modifyPoint(&((*g_controlNetwork)[index]));
    }
    else {
      emit modifyPoint(&((*g_controlNetwork)[g_filteredPoints[index]]));
    }
  }

  /**
   * Figures out what type of widget the filter was selected for and calls the
   * filter method for that filter class
   * 
   * @internal
   * @history  2007-06-05 Tracie Sucharski - Use enumerators for the filter
   *                         indices.  Comment out overlap/polygon code temporarily.
   * 
   */
  void QnetNavTool::filter() {
    p_filter->setEnabled(false);

    // Clear out any old filtered lists
    g_filteredPoints.clear();
//    g_filteredOverlaps.clear();
    g_filteredImages.clear();

    // Get the top widget from the stack
    QTabWidget *tab = (QTabWidget*)(p_filterStack->currentWidget());

    // We're dealing with points
    if (p_listCombo->currentIndex() == Points) {
      PointFilterIndex pointIndex = (PointFilterIndex) tab->currentIndex();
      // We have an errors filter
      if (pointIndex == Errors) {
        QnetPointErrorFilter *widget = 
          (QnetPointErrorFilter*)(tab->currentWidget());
        widget->filter();
      }
      // We have a point id filter
      else if (pointIndex == Id) {
        QnetPointIdFilter *widget = 
          (QnetPointIdFilter*)(tab->currentWidget());
        widget->filter();
      }
      // We have a number of images filter
      else if (pointIndex == NumberImages) {
        QnetPointImagesFilter *widget = 
          (QnetPointImagesFilter*)(tab->currentWidget());
        widget->filter();
      }
      // We have a point type filter
      else if (pointIndex == Type) {
        QnetPointTypeFilter *widget = 
          (QnetPointTypeFilter*)(tab->currentWidget());
        widget->filter();
      }
      // We have a lat/lon range filter
      else if (pointIndex == LatLonRange) {
        QnetPointRangeFilter *widget = 
          (QnetPointRangeFilter*)(tab->currentWidget());
        widget->filter();
      }
      // We have a distance filter
      else if (pointIndex == Distance) {
        QnetPointDistanceFilter *widget = 
          (QnetPointDistanceFilter*)(tab->currentWidget());
        widget->filter();
      }
      // We have a measure filter
      else if (pointIndex == MeasureType) {
        QnetPointMeasureFilter *widget = 
          (QnetPointMeasureFilter*)(tab->currentWidget());
        widget->filter();
      }
    }
#if 0
    // We're dealing with overlaps
    else if (p_listCombo->currentIndex() == 1) {
      // We have a size filter
      if (index == 0) {       
        QnetPolygonSizeFilter *widget = 
          (QnetPolygonSizeFilter*)(tab->currentWidget());
        widget->filter();
      }
      // We have a point filter
      else if (index == 1) {       
        QnetPolygonPointsFilter *widget = 
          (QnetPolygonPointsFilter*)(tab->currentWidget());
        widget->filter();
      }
      // We have an image filter
      else if (index == 2) {       
        QnetPolygonImagesFilter *widget = 
          (QnetPolygonImagesFilter*)(tab->currentWidget());
        widget->filter();
      }     
    }
#endif

    // We're dealing with cubes
    else if (p_listCombo->currentIndex() == Cubes) {
      CubeFilterIndex cubeIndex = (CubeFilterIndex) tab->currentIndex();
      // We have a cube name filter
      if (cubeIndex == Name) {       
        QnetCubeNameFilter *widget = 
          (QnetCubeNameFilter*)(tab->currentWidget());
        widget->filter();
      }
      // We have a point filter
      else if (cubeIndex == NumberPoints) {       
        QnetCubePointsFilter *widget = 
          (QnetCubePointsFilter*)(tab->currentWidget());
        widget->filter();
      }
      // We have a distance filter
      else if (cubeIndex == PointDistance) {       
        QnetCubeDistanceFilter *widget = 
          (QnetCubeDistanceFilter*)(tab->currentWidget());
        widget->filter();
      }
    }
    p_filter->setEnabled(true);
  }

  /**
  * @brief Slot called when the comboBox selecting Points or Cubes filtering
  *        is changed.
  * 
  * @internal
  * @history  2007-06-05 Tracie Sucharski - Use enumerators for the filter
  *                         indices.  Comment out overlap/polygon code temporarily.
  * 
  */
  void QnetNavTool::selectionChanged() {
    // Clear out the old selections
    g_activePoints.clear();
    g_activeOverlaps.clear();
    g_activeImages.clear();

    // If nothing is selected, just return
    if (p_listBox->currentRow() == -1) return;

    // We have at least one item selected
    QList<QListWidgetItem*> selected = p_listBox->selectedItems();
    for (int i=0; i<selected.size(); i++) {
      int index = p_listBox->row(selected[i]);

      // We are dealing with points
      if (p_listCombo->currentIndex() == Points) {
        // If the list is not filtered, add the list box current row
        if (g_filteredPoints.size() == 0) {
          g_activePoints.push_back(index);
        }
        // Otherwise, get the real index to add from the filtered list
        else {
          g_activePoints.push_back(g_filteredPoints[index]);
        }
      }
#if 0
      // We are dealing with overlaps
      else if (p_listCombo->currentIndex() == 1) {
        // If the list is not filtered, add the list box current row
        if (g_filteredOverlaps.size() == 0) {
          g_activeOverlaps.push_back(index);
        }
        // Otherwise, get the real index to add from the filtered list
        else {
          g_activeOverlaps.push_back(g_filteredOverlaps[index]);
        }
      }
#endif      
      // We are dealing with images
      else if (p_listCombo->currentIndex() == Cubes) {
        // If the list is not filtered, add the list box current row
        if (g_filteredImages.size() == 0) {
          g_activeImages.push_back(index);
        }
        // Otherwise, get the real index to add from the filtered list
        else {
          g_activeImages.push_back(g_filteredImages[index]);
        }
      }
      
    }
    // Tell the plottool the selection has changed and it needs to update
    emit selectionUpdate();
  }

  // Updates the selected items in the list with modifications from the plottool
  void QnetNavTool::updateSelected() {
    //TODO: Modify to allow the selection of multiple items??? - DONE? Needs to be tested
    if (g_activePoints.size() > 0) {
      p_listCombo->setCurrentIndex(Points);
      for (int i=0; i<g_activePoints.size(); i++) {
        p_listBox->setCurrentRow(g_activePoints[i]);
      }
    }
#if 0
    else if (g_activeOverlaps.size() > 0) {
      p_listCombo->setCurrentIndex(1);
      for (int i=0; i<g_activeOverlaps.size(); i++) {
        p_listBox->setCurrentRow(g_activeOverlaps[i]);
      }
    }
#endif
    else if (g_activeImages.size() > 0) {
      p_listCombo->setCurrentIndex(Cubes);
      for (int i=0; i<g_activeImages.size(); i++) {
        p_listBox->setCurrentRow(g_activeImages[i]);
      }
    }
  }

  // Only enables the tie button if we are dealing with points
  void QnetNavTool::enableTie() {
    if (p_listCombo->currentIndex() == Points) p_tie->setEnabled(true);
    else p_tie->setEnabled(false);
  }
}

