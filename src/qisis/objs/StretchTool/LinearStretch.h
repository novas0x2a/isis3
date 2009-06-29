#ifndef LinearStretch_h
#define LinearStretch_h

#include "QStretch.h"

#include <QSlider>

namespace Qisis {
  /**
   * @brief Linear stretch that consists of two stretch pairs
   *  
   * This QStretch contains parameters for changing the min/max 
   * of a linear stretch. A linear stretch contains two pairs with 
   * outputs of 0.0 and 255.0 respectively. The inputs are determined 
   * by the parameters and will create a linear stretch from min to max. 
   * 
   *
   * @ingroup Visualization Tools 
   *
   * @author 2009-05-01 Noah hilt
   * 
   * 
   */

  class LinearStretch : public QStretch {
    Q_OBJECT

  public:
    LinearStretch(const StretchTool *stretchTool);
    LinearStretch *clone();
    QGroupBox *getParameters();
    void setMinMax(double min, double max);

  protected slots:
    void calculateStretch();

  private:
    double p_scale; //!< The scale to convert between the QSlider values and min/max values
    QSlider *p_minSlider; //!< QSlider to change the minimum of the stretch
    QSlider *p_maxSlider; //!< QSlider to change the maximum of the stretch
  };
};

#endif

