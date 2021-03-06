#include <QGridLayout>
#include <QMessageBox>
#include "QnetPointGoodnessFilter.h"
#include "QnetNavTool.h"
#include "ControlNet.h"
#include "SerialNumberList.h"
#include "qnet.h"

using namespace Qisis::Qnet;

namespace Qisis {
  /**
   * Contructor for the Point Goodness of Fit filter.  It creates 
   * the Goodness of Fit filter window found in the navtool 
   * 
   * @param parent The parent widget for the point type
   *               filter
   */
  QnetPointGoodnessFilter::QnetPointGoodnessFilter (QWidget *parent) : QnetFilter(parent) {
    // Create the components for the filter window
    p_lessThanCB = new QCheckBox("Less than ");
    p_maxValueEdit = new QLineEdit();
    p_greaterThanCB = new QCheckBox("Greater than ");
    p_minValueEdit = new QLineEdit();
    QLabel *pad = new QLabel();

    connect(p_lessThanCB,SIGNAL(clicked()),this,SLOT(clearEdit()));
    connect(p_greaterThanCB,SIGNAL(clicked()),this,SLOT(clearEdit()));

    // Create the layout and add the components to it
    QGridLayout *gridLayout = new QGridLayout();
    //gridLayout->addWidget(label,0,0,1,2);
    gridLayout->addWidget(p_lessThanCB,1,0,1,2);
    gridLayout->addWidget(p_maxValueEdit,2,0);
    gridLayout->addWidget(p_greaterThanCB,3,0,1,2);
    gridLayout->addWidget(p_minValueEdit,4,0);
    gridLayout->addWidget(pad,5,0);
    gridLayout->setRowStretch(5,50);
    this->setLayout(gridLayout);
  }

  /**
   * @brief Method overwrites parent method.
   * This method keeps all points that contain at least one
   * measure whose Goodness of Fit is within the range specified 
   * by the user. 
   *  
   * @internal
   *   @history 2008-11-26 Jeannie Walldren - Original Version
   *   @history 2009-01-08 Jeannie Walldren - Modified to remove
   *                          new filter points from the existing
   *                          filtered list. Previously, a new
   *                          filtered list was created from the
   *                          entire control net each time.
   */
  void QnetPointGoodnessFilter::filter() {
    // Make sure there is a control net loaded
    if (g_controlNetwork == NULL) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No points to filter");
      return;
    }

    // Make sure the user entered a value to use in the filtering
    if (p_lessThanCB->isChecked() && p_maxValueEdit->text() == "") {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Maximum Goodness of Fit value must be entered");
      return;
    }
    if (p_greaterThanCB->isChecked() && p_minValueEdit->text() == "") {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Minimum Goodness of Fit value must be entered");
      return;
    }

    // Get the user entered filtering value
    double maxValue = p_maxValueEdit->text().toDouble();
    double minValue = p_minValueEdit->text().toDouble();

    // Loop through each value of the filtered points list 
    // Loop in reverse order since removal list of elements affects index number
    for (int i = g_filteredPoints.size()-1; i >= 0; i--) {
      Isis::ControlPoint cp = (*g_controlNetwork)[g_filteredPoints[i]];
      int numMeasOutsideRange = 0;
      // Loop through each measure of the point at this index of the control net
      for (int j = 0; j < cp.Size(); j++) {

        if (cp[j].GoodnessOfFit() == Isis::Null) {
          numMeasOutsideRange++;
        }
        else if (p_lessThanCB->isChecked() && p_greaterThanCB->isChecked()) {
          if (cp[j].GoodnessOfFit() < maxValue && 
              cp[j].GoodnessOfFit() > minValue) break;
          else numMeasOutsideRange++;
        }
        else if (p_lessThanCB->isChecked()) { 
          if (cp[j].GoodnessOfFit() < maxValue) break;
          else numMeasOutsideRange++;
        }
        else if (p_greaterThanCB->isChecked()) {
          if (cp[j].GoodnessOfFit() > minValue) break;
          else numMeasOutsideRange++;
        }
      }
      // if no measures are within the range, remove from filter list
      if (cp.Size() == numMeasOutsideRange) {
        g_filteredPoints.removeAt(i);
      }
    }

    // Tell the navtool that a list has been filtered and it needs to update
    emit filteredListModified();
    return;
  }
  /**
   * @brief Method overwrites parent method.
   * Clear line edit if the corresponding Check Box is "unchecked"
   */
  void QnetPointGoodnessFilter::clearEdit() {

    if (!p_lessThanCB->isChecked()) p_maxValueEdit->clear();
    if (!p_greaterThanCB->isChecked()) p_minValueEdit->clear();
  }
}
