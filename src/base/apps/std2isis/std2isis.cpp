#include "Isis.h"
#include "ProcessByLine.h"

#include "SpecialPixel.h"
#include "UserInterface.h"
#include <QImage>

using namespace std;
using namespace Isis;
QImage *qimage;
int line = 0;
//Initialize values to make special pixels invalid
double null_min = DBL_MAX;
double null_max = DBL_MIN;
double hrs_min = DBL_MAX;
double hrs_max = DBL_MIN;
double lrs_min = DBL_MAX;
double lrs_max = DBL_MIN;

void toCube (Buffer &out);
double TestSpecial(const double pixel);

void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();
  ProcessByLine p;


  // Set special pixel ranges
  if (ui.GetBoolean("SETNULLRANGE")) {
    null_min = ui.GetDouble("NULLMIN");
    null_max = ui.GetDouble("NULLMAX");
  }
  if (ui.GetBoolean("SETHRSRANGE")) {
    hrs_min = ui.GetDouble("HRSMIN");
    hrs_max = ui.GetDouble("HRSMAX");
  }
  if (ui.GetBoolean("SETLRSRANGE")) {
    lrs_min = ui.GetDouble("LRSMIN");
    lrs_max = ui.GetDouble("LRSMAX");
  }

  qimage = new QImage(iString(ui.GetFilename("FROM")));

  // qimage is NULL on failure
  if(qimage->isNull()) {
    delete qimage;
    qimage = NULL;
    throw iException::Message(iException::User, 
     "The file [" + ui.GetFilename("FROM") + "] does not contain a recognized image format.",
                              _FILEINFO_);;
  }

  p.SetOutputCube("TO", qimage->width(), qimage->height());

  p.StartProcess(toCube);
  p.EndProcess();

  line = 0;
  delete qimage;
  qimage = NULL;
}

void toCube (Buffer &out) {
  for(int sample = 0; sample < out.SampleDimension(); sample ++) {
    double pixel = qGray(qimage->pixel(sample, line));
    out[sample] = TestSpecial(pixel);
  }

  line ++;
}

/** 
  * Tests the pixel. If it is valid it will return the dn value,
  * otherwise it will return the Isis special pixel value that
  * corresponds to it
  * 
  * @param pixel The double precision value that represents a
  *              pixel.
  * @return double  The double precision value representing the
  *         pixel will return as a valid dn or changed to an isis
  *         special pixel.
  */
  double TestSpecial(const double pixel){
    if (pixel <= null_max && pixel >= null_min){
      return Isis::NULL8;
    } else if (pixel <= hrs_max && pixel >= hrs_min){
      return Isis::HIGH_REPR_SAT8;
    } else if (pixel <= lrs_max && pixel >= lrs_min){
      return Isis::LOW_REPR_SAT8;
    } else {
      return pixel;
    }
  }
