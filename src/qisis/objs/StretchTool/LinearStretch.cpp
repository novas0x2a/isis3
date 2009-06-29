#include "LinearStretch.h"

#include <QLabel>
#include <QGridLayout>

namespace Qisis {
  /**
   * LinearStretch Constructor. StretchTool can not be null. 
   *  
   * Initializes the stretch, QSliders, and sets the name of 
   * this QStretch to be 'Linear' 
   * 
   * @param stretchTool 
   */
  LinearStretch::LinearStretch(const StretchTool *stretchTool) : QStretch(stretchTool, "Linear") {
    p_stretch.AddPair(0.0, 0.0);
    p_stretch.AddPair(255.0, 255.0);

    p_scale = 1.0;
    p_minSlider = new QSlider(Qt::Horizontal);
    p_minSlider->setRange(0, 255);
    p_minSlider->setTickPosition(QSlider::TicksBelow);

    p_maxSlider = new QSlider(Qt::Horizontal);
    p_maxSlider->setRange(0, 255);
    p_maxSlider->setTickPosition(QSlider::TicksBelow);
    p_maxSlider->setValue(255);
  }

  /**
   * Creates a new LinearStretch with this LinearStretch's parameters 
   * and returns it.
   * 
   * 
   * @return LinearStretch* 
   */
  LinearStretch *LinearStretch::clone() {
    LinearStretch *stretch = new LinearStretch(p_stretchTool);
    stretch->setMinMax(p_min, p_max);
    stretch->p_stretch = this->p_stretch;
    stretch->p_minSlider->setValue(p_minSlider->value());
    stretch->p_maxSlider->setValue(p_maxSlider->value());
    return stretch;
  }

  /**
   * Returns the QGroupBox holding parameters.
   * 
   * 
   * @return QGroupBox* 
   */
  QGroupBox *LinearStretch::getParameters() {
    //If the parameters box hasn't been created yet, create it
    if(p_parametersBox == NULL) {
      p_parametersBox = new QGroupBox("Linear Parameters");

      connect(p_minSlider, SIGNAL(valueChanged(int)), this, SLOT(calculateStretch()));
      connect(p_maxSlider, SIGNAL(valueChanged(int)), this, SLOT(calculateStretch()));

      QLabel *minLabel = new QLabel("Min");
      QLabel *maxLabel = new QLabel("Max");

      QGridLayout *layout = new QGridLayout();
      layout->addWidget(minLabel, 0, 0);
      layout->addWidget(p_minSlider, 0, 1);
      layout->addWidget(maxLabel, 1, 0);
      layout->addWidget(p_maxSlider, 1, 1);
      p_parametersBox->setLayout(layout);
    }

    return p_parametersBox;
  }

  /**
   * Sets the min/max and updates the scale.
   * 
   * @param min 
   * @param max 
   */
  void LinearStretch::setMinMax(double min, double max) {
    QStretch::setMinMax(min, max);
    p_scale = 255.0/(max-min);
    calculateStretch();
  }

  /**
   * This method is called when the min/max has changed and will update
   * the stretch pairs accordingly.
   * 
   */
  void LinearStretch::calculateStretch() {
    p_stretch.ClearPairs();

    //If the min value is less than the max, add it first
    if(p_minSlider->value() < p_maxSlider->value()) {
      p_stretch.AddPair(p_min + (p_minSlider->value()/p_scale), 0.0);
      p_stretch.AddPair(p_min + (p_maxSlider->value()/p_scale), 255.0);
    }
    //If the max value is less than the min, add it first
    else if(p_minSlider->value() > p_maxSlider->value()) {
      p_stretch.AddPair(p_min + (p_maxSlider->value()/p_scale), 255.0);
      p_stretch.AddPair(p_min + (p_minSlider->value()/p_scale), 0.0);
    }
    //If the min is equal to the max, make sure the inputs are not the same
    else {
      p_stretch.AddPair(p_min + (p_minSlider->value()/p_scale), 0.0);
      p_stretch.AddPair(p_min + (p_maxSlider->value()/p_scale) + 0.1, 255.0);
    }

    emit(update());
  }
}
