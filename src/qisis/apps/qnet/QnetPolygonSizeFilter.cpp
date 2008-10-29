#include <QGridLayout>
#include <QMessageBox>
#include "QnetPolygonSizeFilter.h"
#include "QnetNavTool.h"
#include "ControlNet.h"
#include "SerialNumberList.h"
#include "FindImageOverlaps.h"
#include "PolygonTools.h"

#include "qnet.h"

using namespace Qisis::Qnet;

namespace Qisis {
  /**
   * Contructor for the Polygon Size filter.  It creates
   * the Type filter window found in the navtool
   * 
   * @param parent The parent widget for the polygon size
   *               filter
   */
  QnetPolygonSizeFilter::QnetPolygonSizeFilter (QWidget *parent) : QnetFilter(parent) {
    // Create the components for the widget
    QLabel *label = new QLabel("Filter using area of overlaps");
    p_lessThanRB = new QRadioButton("Less than (undercontrolled)");
    p_greaterThanRB = new QRadioButton("Greater than (overcontrolled)");
    p_lineEdit = new QLineEdit();
    QLabel *kms = new QLabel ("kilometers");
    QLabel *pad = new QLabel();
    p_lessThanRB->setChecked(true);

    // Add the components to the main widget and format them
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(label,0,0,1,2);
    gridLayout->addWidget(p_lessThanRB,1,0,1,2);
    gridLayout->addWidget(p_greaterThanRB,2,0,1,2);
    gridLayout->addWidget(p_lineEdit,3,0);
    gridLayout->addWidget(kms,3,1);
    gridLayout->addWidget(pad,4,0);
    gridLayout->setRowStretch(4,50);
    this->setLayout(gridLayout);
  }

  /**
   * Filters the overlaps for areas that are less than or greater than the
   * user entered value. The filtered list will appear in the navtool window
   */
  void QnetPolygonSizeFilter::filter() {
    // Make sure we have overlaps to filter
    if (g_imageOverlap == NULL) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No overlaps to filter");
      return;
    }

    int num = -1;
    // Make sure they entered a size value
    if (p_lineEdit->text() == "") {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Polygon size value must be entered");
      return;
    }
    // Get the user entered filter value
    num = p_lineEdit->text().toInt();

    // Loop through the overlaps comparing their size with the entered value
    for (int i=0; i<g_imageOverlap->Size(); i++) {
      const Isis::ImageOverlap *ov = (*g_imageOverlap)[i];
//      Isis::PolygonTools poly;
//      poly.SetLonLatPolygon(ov->Polygon());
      Isis::Cube cube;
      cube.Open(g_serialNumberList->Filename((*ov)[0]));
      Isis::Projection *proj;
      try {
        proj = cube.Projection();
      }
      catch (Isis::iException e) {
        Isis::PvlGroup mapping("Mapping");
        mapping = Isis::Projection::TargetRadii(cube.Camera()->Target());
        mapping += Isis::PvlKeyword("ProjectionName","Sinusoidal");
        mapping += Isis::PvlKeyword("LatitudeType","Planetocentric");
        mapping += Isis::PvlKeyword("LongitudeDirection","PositiveEast");
        mapping += Isis::PvlKeyword("LongitudeDomain",360);
        mapping += Isis::PvlKeyword("CenterLongitude",17.0);

        Isis::Pvl pvl;
        pvl.AddGroup(mapping);
        proj = Isis::ProjectionFactory::Create(pvl);
      }
//      poly.SetProjection(*proj);
      geos::geom::MultiPolygon *poly = Isis::PolygonTools::XYFromLonLat(*(ov->Polygon()), proj);
      double area = poly->getArea() / 1000000.0 ; //convert to km
      delete poly;
      std::cout << area << std::endl;
      if (p_lessThanRB->isChecked()) {
        if (area < num) {
          g_filteredOverlaps.push_back(i);
        }
      }
      else if (p_greaterThanRB->isChecked()) {
        if (area > num) {
          g_filteredOverlaps.push_back(i);
        }
      }
    }

    // Tell the navtool that a list has been filtered and it needs to update
    emit filteredListModified();
    return;
  }
}
