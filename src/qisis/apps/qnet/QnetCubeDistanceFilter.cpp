#include <QGridLayout>
#include <QMessageBox>
#include "QnetCubeDistanceFilter.h"
#include "QnetNavTool.h"
#include "ControlNet.h"
#include "SerialNumberList.h"
#include "ImageOverlap.h"
#include <vector>
#include <cmath>

#include "qnet.h"

using namespace Qisis::Qnet;

namespace Qisis {
 /**
  * Contructor for the Cube Distance filter.  It
  * creates the Distance filter window found in
  * the navtool
  * 
  * @param parent The parent widget for the cube
  *               distance filter
  */ 
  QnetCubeDistanceFilter::QnetCubeDistanceFilter (QWidget *parent) : QnetFilter(parent) {
    // Create components for the filter window
    QLabel *label = new QLabel("Filter by distance between points in cube");
    QLabel *lessThan = new QLabel("Less than");
    p_distEdit = new QLineEdit();
    QLabel *units = new QLabel ("pixels");
    QLabel *pad = new QLabel();

    // Create the layout and add the components to it
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(label,0,0,1,2);
    gridLayout->addWidget(lessThan,1,0);
    gridLayout->addWidget(p_distEdit,1,1);
    gridLayout->addWidget(units,1,2);
    gridLayout->addWidget(pad,2,0);
    gridLayout->setRowStretch(2,50);
    this->setLayout(gridLayout);
  }

  /**
   * Filters a list of images for images that have points that are less than
   * the user entered distance from other points in the image. The filtered
   * list will appear in the navtools images list display.
   */
  void QnetCubeDistanceFilter::filter() {
    // Make sure we have a list of files to filter
    if ( g_serialNumberList == NULL ) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No cubes to filter");
      return;
    }

    // Make sure the user entered a value for filtering
    int num = -1;
    if ( p_distEdit->text() == "" ) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Distance value must be entered");
      return;
    }

    // Get the user entered value for filtering
    num = p_distEdit->text().toInt();

    // Set up a table to link a list of measures to the image they are in
    std::vector< std::vector<Isis::ControlMeasure> > table;

    // Loop through each image
    for ( int i=0; i<g_serialNumberList->Size(); i++ ) {
      std::vector<Isis::ControlMeasure> measures;

      // Loop through every control point
      for ( int c=0; c<g_controlNetwork->Size(); c++ ) {
        Isis::ControlPoint cp = (*g_controlNetwork)[c];

        // Check each control measure to see if it is for the current image
        for ( int cm=0; cm<cp.Size(); cm++ ) {
          if ( (cp[cm].CubeSerialNumber()) == 
               ((*g_serialNumberList).SerialNumber(i)) ) {
            measures.push_back(cp[cm]);
          }
        }
      }
      // Add the list of measures in each of the images
      table.push_back(measures);
    }

    // Loop through each image again now that we know what control measures are 
    // in them
    for (int i=0; i<g_serialNumberList->Size(); i++) {
      // Set the bestDist to infinity so it will be replaced
      double bestDist = DBL_MAX * 2.;

      // Store the vector of measures for easy access
      std::vector<Isis::ControlMeasure> temp = table[i];

      // Loop through each measure getting the distance from it to each of the
      // rest of the measures
      for (int j=0; j<(int)temp.size(); j++) {
        for (int k=(j+1); k<(int)temp.size(); k++) {
          // Calculate the distance between the two points
          double deltaSamp = temp[j].Sample() - temp[k].Sample();
          double deltaLine = temp[j].Line() - temp[k].Line();
          double dist = sqrt((deltaSamp * deltaSamp) + (deltaLine * deltaLine));

          // If the new distance is less than the current lowest, replace it
          if ( dist < bestDist ) bestDist = dist;
        }
      }
      // If the smallest distance is less than the input number, add the image 
      // to the filtered list of images
      if (bestDist < num) g_filteredImages.push_back(i);
    }
    // Tell the navtool a list has been filtered and it needs to update
    emit filteredListModified();
    return;
  }
}
