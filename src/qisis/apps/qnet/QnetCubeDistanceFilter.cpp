#include <vector>
#include <cmath>
#include <QGridLayout>
#include <QMessageBox>
#include "QnetCubeDistanceFilter.h"
#include "QnetNavTool.h"
#include "ControlNet.h"
#include "SerialNumberList.h"
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
  * @internal 
  *   @history 2008-12-03 Jeannie Walldren - Added radio button
  *                          so user may choose to filter
  *                          distance in pixels or meters.
  */ 
  QnetCubeDistanceFilter::QnetCubeDistanceFilter (QWidget *parent) : QnetFilter(parent) {
    // Create components for the filter window
    QLabel *label = new QLabel("Filter by distance between points in cube");
    QLabel *lessThan = new QLabel("Contains points within ");
    p_lineEdit = new QLineEdit();
    p_pixels = new QRadioButton("pixels");
    p_meters = new QRadioButton("meters");
    p_pixels->setChecked(true);
    QLabel *pad = new QLabel();

    // create layout for radio buttons
    QVBoxLayout *units = new QVBoxLayout();
    units->addWidget(p_pixels);
    units->addWidget(p_meters);

    // Create the layout and add the components to it
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(label,0,0,1,2);
    gridLayout->addWidget(lessThan,1,0);
    gridLayout->addWidget(p_lineEdit,1,1);
    gridLayout->addLayout(units,1,2);
    gridLayout->addWidget(pad,2,0);
    gridLayout->setRowStretch(2,50);
    this->setLayout(gridLayout);
  }

  /**
   * Filters a list of images for images that have points that are less than
   * the user entered distance from other points in the image. The filtered
   * list will appear in the navtools images list display. 
   * @internal 
   *   @history 2008-12-03 Jeannie Walldren - Reorganized method
   *                          so it is working properly, to
   *                          increase efficiency (by breaking out
   *                          of loops when possible) and to allow
   *                          filtering for a distance in meters
   *                          or pixels.  Renamed variables for
   *                          clarity.
   *   @history 2009-01-08 Jeannie Walldren - Modified to replace
   *                          existing filtered list with a subset
   *                          of that list. Previously, a new
   *                          filtered list was created from the
   *                          entire serial number list each time.
   */
  void QnetCubeDistanceFilter::filter() {
    // Make sure we have a list of files to filter
    if ( g_serialNumberList == NULL ) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No cubes to filter");
      return;
    }

    // Make sure the user entered a filtering value
    if ( p_lineEdit->text() == "" ) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Distance value must be entered");
      return;
    }

    // Get the user entered value for filtering
    int userEntered = p_lineEdit->text().toInt();

    // create temporary QList to contain new filtered images
    QList <int> temp;
    // Loop through each image in the currently filtered list
    // Loop in reverse order for consistency with other filters
    for (int i = g_filteredImages.size()-1; i >= 0; i--) {

      // Loop through every control point in the control net
      for (int cp1 = 0; cp1 < g_controlNetwork->Size(); cp1++) {
        Isis::ControlPoint controlPt1 = (*g_controlNetwork)[cp1];
        Isis::ControlMeasure controlMeas1;
        // Loop through each control measure of this point until we find this image
        for (int cm1 = 0; cm1 < controlPt1.Size(); cm1++) {
          // Check control measure to see if this point is in the current image
          if ( controlPt1[cm1].CubeSerialNumber() == g_serialNumberList->SerialNumber(g_filteredImages[i]) )  {
            // this measure matches the image, set controlMeas1 variable and break out of loop
            controlMeas1 = controlPt1[cm1];
            break;
          }
          // this measure does not match this cube, keep looking, continue to next measure
          else continue;
        }  // if no matching measure was found, controlMeas1 defaults to sample,line = 0
    
        // If the measure for this image has no line/sample values, 
        // we can not measure distance, so continue to the next control point, cp1
        if(controlMeas1.Sample() == 0 && controlMeas1.Line() == 0) continue;
        // if the user chooses distance in meters, create camera to find lat/lon for this measure
        double rad=0, lat1=0, lon1=0;
        if (p_meters->isChecked()) {
          Isis::Camera *cam1;
          cam1  = g_controlNetwork->Camera(g_filteredImages[i]);
          // try to set image using sample/line values
          if (cam1->SetImage(controlMeas1.Sample(),controlMeas1.Line())) {
            rad = cam1->LocalRadius();
            lat1 = cam1->UniversalLatitude();
            lon1 = cam1->UniversalLongitude();
          }
          else { 
            // if SetImage fails for this measure, don't calculate the distance from this point
            // continue to next control point, cp1
            continue;
          }
        }
        // Loop through the remaining control points to compute distances
        for (int cp2 = (cp1 + 1); cp2 < g_controlNetwork->Size(); cp2++) {
          Isis::ControlPoint controlPt2 = (*g_controlNetwork)[cp2];
          Isis::ControlMeasure controlMeas2;
          // Loop through each measure of the second point until we find this image
          for (int cm2 = 0; cm2 < controlPt2.Size(); cm2++) {
            // Check to see if this point is in the image
            if ( controlPt2[cm2].CubeSerialNumber() == g_serialNumberList->SerialNumber(g_filteredImages[i]) ) {
              // this measure matches the image, set controlMeas2 variable and break out of loop
              controlMeas2 = controlPt2[cm2];
              break;
            }
            // this measure does not match this cube, keep looking, continue to next measure
            else continue;
          }  // if no matching measure was found, controlMeas1 defaults to sample,line = 0
    
          // If the measure has no samp/line values, continue to next point, cp2
          if(controlMeas2.Sample() == 0 && controlMeas2.Line() == 0) continue;
          
          // Now determine distance based on the units chosen
          double dist = 0;
          if (p_pixels->isChecked()) {
            double deltaSamp = controlMeas1.Sample() - controlMeas2.Sample();
            double deltaLine = controlMeas1.Line() - controlMeas2.Line();
            // use the distance formula for cartesian coordinates
            dist = sqrt((deltaSamp * deltaSamp) + (deltaLine * deltaLine));
          }
          else { // meters is checked
            // create camera to find lat/lon for this measure
            double lat2=0, lon2=0;
            Isis::Camera *cam2;
            cam2 = g_controlNetwork->Camera(g_filteredImages[i]);
            // try to set image using sample/line values
            if (cam2->SetImage(controlMeas2.Sample(),controlMeas2.Line())) {
              lat2 = cam2->UniversalLatitude();
              lon2 = cam2->UniversalLongitude();
            }
            else { 
              // if SetImage fails, don't calculate the distance to this point
              // continue to the next control point, cp2
              continue;
            }
            // Calculate the distance between the two points
            // Get the distance from the camera class
            dist = Isis::Camera::Distance(lat1,lon1,lat2,lon2,rad);
          }

          if (dist == 0) {
            // distance not calculated, continue to next cp2
            continue;
          }
    
          // If the distance is less than the value entered by the user,
          // keep the cube in the list
          if ( dist < userEntered ) {
            if (!temp.contains(g_filteredImages[i])) {
              // this image is added to the filtered list
              temp.push_back(g_filteredImages[i]);
            }
          }
          if (temp.contains(g_filteredImages[i])) {
            // g_filteredImages[i] is already added to the filtered list, 
            // no point in checking other distances,
            // break out of cp2 loop and continue to next cp1
            break;
          }
        } // this ends cp2 loop

        if (temp.contains(g_filteredImages[i])) {
           // g_filteredImages[i] is already added to the filtered list, 
           // no point in checking other distances,
           // break out of cp1 loop and continue to next value of i
          break;
        }    
      } // this ends cp1 loop
    } // this ends i loop

    // Sort QList of filtered points before displaying list to user
    qSort(temp.begin(), temp.end());
    // replace existing filter list with this one
    g_filteredImages = temp;

    // Tell the nav tool that a list has been filtered and it needs to updated
    emit filteredListModified();
    return;
  }
}








