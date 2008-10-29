#include <QGridLayout>
#include <QMessageBox>
#include "QnetPolygonPointsFilter.h"
#include "QnetNavTool.h"
#include "ControlNet.h"
#include "SerialNumberList.h"
#include "FindImageOverlaps.h"
#include "PolygonTools.h"

#include "geos/geom/Point.h"
#include "geos/geom/MultiPolygon.h"
#include "geos/geom/Coordinate.h"
#include "geos/geom/GeometryFactory.h"

#include "qnet.h"

using namespace Qisis::Qnet;
using namespace std;

namespace Qisis {
  /**
   * Contructor for the Polygon Points filter.  It creates the Points filter
   * window found in the navtool
   * 
   * @param parent The parent widget for the polygon points filter
   */
  QnetPolygonPointsFilter::QnetPolygonPointsFilter (QWidget *parent) : QnetFilter(parent) {
    // Create the labels and buttons for the filter window
    QLabel *label = new QLabel("Filter by number of points in overlap");
    p_lessThanRB = new QRadioButton("Less than (undercontrolled)");
    p_greaterThanRB = new QRadioButton("Greater than (overcontrolled)");
    p_lessThanRB->setChecked(true);
    p_pointSpin = new QSpinBox();
    p_pointSpin->setSuffix(" points");
    QLabel *pad = new QLabel();

    // Create the layout and add the objects to the window
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(label,0,0,1,2);
    gridLayout->addWidget(p_lessThanRB,1,0,1,2);
    gridLayout->addWidget(p_greaterThanRB,2,0,1,2);
    gridLayout->addWidget(p_pointSpin,3,0,1,2);
    gridLayout->addWidget(pad,5,0);
    gridLayout->setRowStretch(5,50);
    this->setLayout(gridLayout);
  }

  /**
   * Filters a list of overlaps for overlaps that have less than or greater than
   * the entered number of points.  The filtered list will appear in the
   * navtools overlap list display.
   */
  void QnetPolygonPointsFilter::filter() {
    // Make sure there are overlaps to filter
    if (g_imageOverlap == NULL) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No overlaps to filter");
      return;
    }

    // Get the number of points in the overlap to filter for
    int num = p_pointSpin->value();

    // Loop through each overlap in the list
    for (int i=0; i<g_imageOverlap->Size(); i++) {
      int count = 0;
      geos::geom::MultiPolygon *poly = Isis::PolygonTools::CopyMultiPolygon(
                                   (*g_imageOverlap)[i]->Polygon());

      // Loop through each point checking to see if it is in the overlap
      for (int j=0; j<g_controlNetwork->Size(); j++) {
        Isis::ControlPoint p = (*g_controlNetwork)[j];
        geos::geom::GeometryFactory *gFact = new geos::geom::GeometryFactory();
        const geos::geom::Coordinate *coord = 
          new geos::geom::Coordinate(p.UniversalLatitude(),p.UniversalLongitude());
        geos::geom::Point *pt = gFact->createPoint(*coord);

        // If the point is in the overlap, increment the counter
        if (poly->contains(pt)) count++;     
      }

      // Check to see if the overlap is within the filtered range and add it to
      // the filtered overlap list if it is
      if (p_lessThanRB->isChecked()) {
        if (count < num) {
          g_filteredOverlaps.push_back(i);
        }
      }
      else if (p_greaterThanRB->isChecked()) {   
        if (count > num) {
          g_filteredOverlaps.push_back(i);
        }
      }
    }
    // Tell the navtool that a list has been filtered and it need to update it
    emit filteredListModified();
    return;
  }
}
