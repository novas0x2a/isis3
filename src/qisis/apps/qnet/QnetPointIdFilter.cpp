#include <QGridLayout>
#include <QMessageBox>
#include <QRegExp>
#include "QnetPointIdFilter.h"
#include "QnetNavTool.h"
#include "ControlNet.h"
#include "SerialNumberList.h"
#include "FindImageOverlaps.h"

#include "qnet.h"

using namespace Qisis::Qnet;
using namespace std;

namespace Qisis {
  /**
   * Contructor for the Cube Image filter.  It creates the Cube Name filter window
   * found in the navtool
   * 
   * @param parent The parent widget for the
   *               cube points filter
   */
  QnetPointIdFilter::QnetPointIdFilter (QWidget *parent) : QnetFilter(parent) {
    // Create the components for the filter window
    QLabel *label = new QLabel("Filter by cube name (Regular Expressions");
    p_pointIdEdit = new QLineEdit();

    // Create the layout and add the components to it
    QVBoxLayout *vertLayout = new QVBoxLayout();
    vertLayout->addWidget(label);
    vertLayout->addWidget(p_pointIdEdit);
    vertLayout->addStretch();
    this->setLayout(vertLayout);
  }

  /**
   * Filters a list of images looking for cube names using the regular expression
   * entered.  The filtered list will appear in the navtools cube list display.
   */
  void QnetPointIdFilter::filter() {

        // Make sure there is a control net loaded
    if (g_controlNetwork == NULL) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No points to filter");
      return;
    }

    // Make sure the user has entered a regular expression for filtering
    QRegExp rx(p_pointIdEdit->text());
    rx.setPatternSyntax(QRegExp::Wildcard);
    if (rx.isEmpty()) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Enter search string");
      return;
    }


    // Loop through the control net checking the types of each control measure 
    // for each of the control points
    for (int i=0; i<g_controlNetwork->Size(); i++) {

      string cNetId = (*g_controlNetwork)[i].Id();
      if (rx.indexIn(QString(cNetId.c_str())) != -1) {
        g_filteredPoints.push_back(i);
      }
    }

    // Tell the navtool a list has been filtered and it needs to update
    emit filteredListModified();
    return;
  }
}
