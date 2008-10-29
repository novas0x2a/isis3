#include <QGridLayout>
#include <QMessageBox>
#include <QRegExp>
#include "QnetCubeNameFilter.h"
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
  QnetCubeNameFilter::QnetCubeNameFilter (QWidget *parent) : QnetFilter(parent) {
    // Create the components for the filter window
    QLabel *label = new QLabel("Filter by cube name (Regular Expressions");
    p_cubeNameEdit = new QLineEdit();

    // Create the layout and add the components to it
    QVBoxLayout *vertLayout = new QVBoxLayout();
    vertLayout->addWidget(label);
    vertLayout->addWidget(p_cubeNameEdit);
    vertLayout->addStretch();
    this->setLayout(vertLayout);
  }

  /**
   * Filters a list of images looking for cube names using the regular expression
   * entered.  The filtered list will appear in the navtools cube list display.
   */
  void QnetCubeNameFilter::filter() {
    // Make sure we have a list of images to filter
    if (g_serialNumberList == NULL) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No cubes to filter");
      return;
    }

    // Make sure the user has entered a regular expression for filtering
    QRegExp rx(p_cubeNameEdit->text());
    rx.setPatternSyntax(QRegExp::Wildcard);
    if (rx.isEmpty()) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Enter search string");
      return;
    }


    // Loop through each image in the list
    for (int i=0; i<g_serialNumberList->Size(); i++) {
      string tempFilename = g_serialNumberList->Filename(i);
      if (rx.indexIn(QString(tempFilename.c_str())) != -1) {
        g_filteredImages.push_back(i);
      }

    }
    // Tell the navtool a list has been filtered and it needs to update
    emit filteredListModified();
    return;

  }
}
