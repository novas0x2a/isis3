#include <QGridLayout>
#include <QMessageBox>
#include "QnetPointMeasureFilter.h"
#include "QnetNavTool.h"
#include "ControlMeasure.h"
#include "ControlNet.h"
#include "SerialNumberList.h"

#include "qnet.h"

using namespace Qisis::Qnet;

namespace Qisis {
  /**
   * Contructor for the Point Measure filter.  It creates the
   * Measure filter window found in the navtool
   * 
   * @param parent The parent widget for the point measure
   *               filter
   */
  QnetPointMeasureFilter::QnetPointMeasureFilter (QWidget *parent) : QnetFilter(parent) {
    // Create the components for the filter window
    p_unmeasured = new QCheckBox("Unmeasured");
    p_manual = new QCheckBox("Manual");
    p_estimated = new QCheckBox("Estimated");
    p_automatic = new QCheckBox("Auto-Registered");
    p_validatedManual = new QCheckBox("Manual/Validated");
    p_validatedAutomatic = new QCheckBox("Auto-Registered/Validated");

    // Create the layout and add the components to it
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(p_unmeasured);
    layout->addWidget(p_manual);
    layout->addWidget(p_estimated);
    layout->addWidget(p_automatic);
    layout->addWidget(p_validatedManual);
    layout->addWidget(p_validatedAutomatic);
    this->setLayout(layout);
  }

  /**
   * Filters a list of points for points that have at least one measure
   * of the selected type(s). The filtered list will appear in the
   * navtools point list display. 
   *  
   * @internal
   *   @history 2009-01-08 Jeannie Walldren - Modified to remove
   *                          new filter points from the existing
   *                          filtered list. Previously, a new
   *                          filtered list was created from the
   *                          entire control net each time.
   */
  void QnetPointMeasureFilter::filter() {
    // Make sure there is a control net loaded to filter
    if (g_controlNetwork == NULL) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No points to filter");
      return;
    }

    // Make sure they selected at least one type to filter for
    if (!((p_unmeasured->isChecked()) || (p_manual->isChecked()) || 
          (p_estimated->isChecked()) || (p_automatic->isChecked()) ||
          (p_validatedManual->isChecked()) || 
          (p_validatedAutomatic->isChecked()))) {
      QMessageBox::information((QWidget *)parent(),"Error",
                        "You must select at least one measure type to filter");
      return;
    }

    // Loop through each value of the filtered points list 
    // checking the types of each control measure for each 
    // of the control points.  If no measures match, we remove
    // it from the filtered list
    // Loop in reverse order since removal list of elements affects index number
    for (int i = g_filteredPoints.size()-1; i >= 0; i--) {
      Isis::ControlPoint cp = (*g_controlNetwork)[g_filteredPoints[i]];
      int numMeasNotMatching = 0;
      for (int j=0; j<cp.Size(); j++) {
        // if the point contains a measure that matches a checked type, 
        // keep this point in the list and go on to the next point
        if ((p_unmeasured->isChecked()) && 
            (cp[j].Type() == Isis::ControlMeasure::Unmeasured)) {
          break;  
        }
        else if ((p_manual->isChecked()) && 
                 (cp[j].Type() == Isis::ControlMeasure::Manual)) {
          break;
        }
        else if ((p_estimated->isChecked()) && 
                 (cp[j].Type() == Isis::ControlMeasure::Estimated)) {
          break;
        }
        else if ((p_automatic->isChecked()) && 
                 (cp[j].Type() == Isis::ControlMeasure::Automatic)) {
          break;
        }
        else if ((p_validatedManual->isChecked()) && 
                 (cp[j].Type() == Isis::ControlMeasure::ValidatedManual)) {
          break;
        }
        else if ((p_validatedAutomatic->isChecked()) && 
                 (cp[j].Type() == Isis::ControlMeasure::ValidatedAutomatic)) {
          break;
        }
        // if this measure doesn't match any of the checked values, increment
        else numMeasNotMatching++;
      }
      // if no measures match the checked values, 
      // remove this point from the filter list
      if (cp.Size() == numMeasNotMatching) {
        g_filteredPoints.removeAt(i);
      }
    }

    // Tell the navtool that a list has been filtered and it needs to update
    emit filteredListModified();
    return;
  }
}
