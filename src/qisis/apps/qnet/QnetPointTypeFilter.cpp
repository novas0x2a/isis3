#include <QGridLayout>
#include <QMessageBox>
#include "QnetPointTypeFilter.h"
#include "QnetNavTool.h"
#include "ControlNet.h"
#include "SerialNumberList.h"
#include "qnet.h"

using namespace Qisis::Qnet;

namespace Qisis {
  /**
   * Contructor for the Point Type filter.  It creates the
   * Type filter window found in the navtool
   * 
   * @param parent The parent widget for the point type
   *               filter
   */
  QnetPointTypeFilter::QnetPointTypeFilter (QWidget *parent) : QnetFilter(parent) {
    // Create the components for the filter window
    p_ground = new QRadioButton("Ground");
    p_ground->setChecked(true);
    p_ignore = new QRadioButton("Ignored");
    p_held = new QRadioButton("Held");
    QLabel *pad = new QLabel();
    
    // Create the layout and add the components to it
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(p_ground,0,0,1,4);
    gridLayout->addWidget(p_ignore,1,0,1,4);
    gridLayout->addWidget(p_held,2,0,1,4);
    gridLayout->addWidget(pad,3,0);
    gridLayout->setRowStretch(3,50);
    this->setLayout(gridLayout);
  }

  /**
   * Filters a list of points for points that are of the selected
   * type or in the given range. The filtered list will appear in
   * the navtools point list display. 
   * @internal 
   *   @history 2008-11-26 Jeannie Walldren - Fixed comment. 
   *   @history 2009-01-08 Jeannie Walldren - Modified to remove
   *                          new filter points from the existing
   *                          filtered list. Previously, a new
   *                          filtered list was created from the
   *                          entire control net each time.
   */
  void QnetPointTypeFilter::filter() {
    // Make sure there is a control net loaded
    if (g_controlNetwork == NULL) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No points to filter");
      return;
    }

    // Loop through each value of the filtered points list checking 
    // the types of each control point 
    // Loop in reverse order since removal list of elements affects index number
    for (int i = g_filteredPoints.size()-1; i >= 0; i--) {
      Isis::ControlPoint cp = (*g_controlNetwork)[g_filteredPoints[i]];
      if (p_ground->isChecked()) {
        if (cp.Type() == 0) continue;
        else g_filteredPoints.removeAt(i);
      }
      else if (p_ignore->isChecked()) {
        if (cp.Ignore()) continue;
        else g_filteredPoints.removeAt(i);
      }
      else if (p_held->isChecked()) {
        if (cp.Held()) continue;
        else g_filteredPoints.removeAt(i);
      }
    }

    // Tell the navtool that a list has been filtered and it needs to update
    emit filteredListModified();
    return;
  }
}
