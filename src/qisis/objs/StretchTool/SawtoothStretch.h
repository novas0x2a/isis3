#ifndef SawtoothStretch_h
#define SawtoothStretch_h

#include "QStretch.h"

#include <QSlider>

namespace Qisis {
  /**
   * @brief Sawtooth stretch to map values across a number of sawtooths 
   *  
   *  
   * This QStretch allows the user to map values across a number of sawtooths that 
   * are manipulated by the user. The sawtooths can range in number and position 
   * depending on the parameters. 
   *  
   *
   * @ingroup Visualization Tools 
   *
   * @author 2009-05-01 Noah hilt
   * 
   * 
   */

  class SawtoothStretch : public QStretch {
    Q_OBJECT

  public:
    SawtoothStretch(const StretchTool *stretchTool);
    SawtoothStretch *clone();
    QGroupBox *getParameters();
    void setMinMax(double min, double max);
    void computeSawtooths();

  protected slots:
    void changeStart(int value);
    void changeSawtooths(int value);

  private:
    double p_scale; //!< The scale to convert between the QSlider values and min/max values
    double p_start; //!< The starting point of the sawtooths
    int p_sawtooths; //!< The number of sawtooths 
    QSlider *p_startSlider; //!< QSlider to move the starting point of the sawtooths
    QSlider *p_sawtoothsSlider; //!< QSlider to change the number of sawtooths
  };
};

#endif

