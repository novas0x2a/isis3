#ifndef BinaryStretch_h
#define BinaryStretch_h

#include "QStretch.h"

#include <QSlider>

namespace Qisis {
  /**
   * @brief Binary stretch to map a window of values to 255
   *  
   *  
   * This QStretch allows the user to map a window of values to 255 and everything
   * outside of the window to 0. The window's size and location can be manipulated 
   * using this QStretch's parameters. 
   *  
   * 
   *
   * @ingroup Visualization Tools 
   *
   * @author 2009-05-01 Noah hilt 
   * @history 2009-06-04 Stacy Alley 
   * added a variable in the ComputeBinary() method:  stepSize. 
   * This was required because previously this number was hard 
   * coded to be 0.001.  In cubes with very small DN #'s this 
   * would cause an error to be thrown.  The step size needed to 
   * be based on DN values from each cube. 
   * 
   * 
   */

  class BinaryStretch : public QStretch {
    Q_OBJECT

  public:
    BinaryStretch(const StretchTool *stretchTool);
    BinaryStretch *clone();
    QGroupBox *getParameters();
    void setMinMax(double min, double max);
    void computeBinary();

  protected slots:
    void changeStart(int value);
    void changeWidth(int value);

  private:
    double p_scale; //!< The scale to convert between the QSlider values and min/max values
    double p_start; //!< The starting point of the window
    double p_width; //!< The width of the window
    QSlider *p_startSlider; //!< QSlider to move the starting point of the window
    QSlider *p_widthSlider; //!< QSlider to change the width of the window
  };
};

#endif

