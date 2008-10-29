#include <QGridLayout>
#include <QMessageBox>
#include "QnetPointDistanceFilter.h"
#include "QnetNavTool.h"
#include "ControlNet.h"
#include "SerialNumberList.h"
#include "Camera.h"

#include "qnet.h"

using namespace Qisis::Qnet;

namespace Qisis {
  /**
   * Contructor for the Point Distance filter.  It creates the
   * Distance filter window found in the navtool
   * 
   * @param parent The parent widget for the point distance
   *               filter
   */
  QnetPointDistanceFilter::QnetPointDistanceFilter (QWidget *parent) : QnetFilter(parent) {
    // Create the labels and widgets to be added to the main window
    QLabel *label = new QLabel("Filter using distance between points");
    QLabel *lessThan = new QLabel("Less than");
    p_lineEdit = new QLineEdit();
    QLabel *meters = new QLabel ("meters");
    QLabel *pad = new QLabel();

    // Create the layout and add the widgets to the window
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(label,0,0,1,2);
    gridLayout->addWidget(lessThan,1,0);
    gridLayout->addWidget(p_lineEdit,1,1);
    gridLayout->addWidget(meters,1,2);
    gridLayout->addWidget(pad,2,0);
    gridLayout->setRowStretch(2,50);
    this->setLayout(gridLayout);
  }

  /**
   * Filters a list of points for points that are less than the user entered
   * distance from another point in the control net.  The filtered list will
   * appear in the navtools point list display.
   */
  void QnetPointDistanceFilter::filter() {
    // Make sure we have a control network to filter through
    if (g_controlNetwork == NULL) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No points to filter");
      return;
    }

    // Make sure the user entered a filtering value
    int num = -1;
    if (p_lineEdit->text() == "") {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Distance value must be entered");
      return;
    }
    // Get the user entered value for filtering
    num = p_lineEdit->text().toInt();

    // Loop through each control point
    for (int i=0; i<g_controlNetwork->Size(); i++) {
      // Set the minDist to some huge number so the value is sure to be replaced
      double minDist = 1e10;

      // Get necessary info from the control point for later use
      double rad = (*g_controlNetwork)[i].Radius();
      double lat1 = (*g_controlNetwork)[i].UniversalLatitude();
      double lon1 = (*g_controlNetwork)[i].UniversalLongitude();

      // Skip this point if the lat/lon values were not set in the control net
      if ((lat1 == Isis::Null) || (lon1 == Isis::Null)) {
        continue;
      }

      // Loop through each other point comparing it to the initial point we have
      for (int j=0; j<g_controlNetwork->Size(); j++) {
        if (i == j) continue;
        double lat2 = (*g_controlNetwork)[j].UniversalLatitude();
        double lon2 = (*g_controlNetwork)[j].UniversalLongitude();

        // Skip this point if the lat/lon values were not set in the control net
        if ((lat2 == Isis::Null) || (lon2 == Isis::Null)) {
          continue;
        }

        // Get the distance from the camera class
        double d = Isis::Camera::Distance(lat1,lon1,lat2,lon2,rad);

        // Check to see if the new distance is less than the current minDist
        if (d < minDist) minDist = d;
      }

      // If the minimum distance is less than the number we are filtering for, 
      // add the current control point index to the filtered points list
      if (minDist < num) {
        g_filteredPoints.push_back(i);
      }
    }
    // Tell the nav tool that a list has been filtered and needs to be updated
    emit filteredListModified();
    return;
  }
}
