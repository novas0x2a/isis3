#include "BinaryStretch.h"

#include <QLabel>
#include <QBoxLayout>

namespace Qisis {
  /**
   * BinaryStretch Constructor. StretchTool can not be null. 
   *  
   * Initializes the stretch, QSliders, and sets the name of 
   * this QStretch to be 'Binary' 
   * 
   * @param stretchTool 
   */
  BinaryStretch::BinaryStretch(const StretchTool *stretchTool) : QStretch(stretchTool, "Binary") {
    p_width = 0.0;
    p_startSlider = new QSlider();
    p_startSlider->setOrientation(Qt::Horizontal);
    p_startSlider->setTickPosition(QSlider::TicksBelow);
    p_startSlider->setRange(0, 254);
    p_widthSlider = new QSlider();
    p_widthSlider->setOrientation(Qt::Horizontal);
    p_widthSlider->setTickPosition(QSlider::TicksBelow);
    p_widthSlider->setRange(1, 255);
  }

  /**
   * Creates a new BinaryStretch with this BinaryStretch's parameters 
   * and returns it.
   * 
   * 
   * @return BinaryStretch* 
   */
  BinaryStretch *BinaryStretch::clone() {
    BinaryStretch *stretch = new BinaryStretch(p_stretchTool);
    stretch->setMinMax(p_min, p_max);
    stretch->p_stretch = this->p_stretch;
    stretch->p_startSlider->setValue(p_startSlider->value());
    stretch->p_widthSlider->setValue(p_widthSlider->value());
    stretch->p_start = this->p_start;
    stretch->p_width = this->p_width;
    return stretch;
  }

  /**
   * Returns the QGroupBox holding parameters.
   * 
   * 
   * @return QGroupBox* 
   */
  QGroupBox *BinaryStretch::getParameters() {
    //If the parameters box hasn't been created yet, create it
    if(p_parametersBox == NULL) {
      p_parametersBox = new QGroupBox("Binary Parameters");
  
      QLabel *startLabel = new QLabel("Start-Point");
      QLabel *widthLabel = new QLabel("Width");
  
      connect(p_startSlider, SIGNAL(valueChanged(int)), this, SLOT(changeStart(int)));
      connect(p_widthSlider, SIGNAL(valueChanged(int)), this, SLOT(changeWidth(int)));
  
      QVBoxLayout *layout = new QVBoxLayout();
      layout->addWidget(startLabel);
      layout->addWidget(p_startSlider);
      layout->addWidget(widthLabel);
      layout->addWidget(p_widthSlider);
      layout->addStretch(1);
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
  void BinaryStretch::setMinMax(double min, double max) {
    QStretch::setMinMax(min, max);
    p_scale = 255.0/(max-min);
    changeStart(p_startSlider->value());
    changeWidth(p_widthSlider->value());
  }

  /**
   * Changes the starting point to the scaled value and computes the binary 
   * stretch. 
   * 
   * @param value 
   */
  void BinaryStretch::changeStart(int value) {
    p_start = p_min + (value/p_scale);
    computeBinary();
  }

  /**
   * Changes the width to the scaled value and computers the binary stretch.
   * 
   * @param value 
   */
  void BinaryStretch::changeWidth(int value) {
    p_width = (value/p_scale);
    computeBinary();
  }

  /**
   * Computes the binary stretch from the starting point and width.
   * 
   */
  void BinaryStretch::computeBinary() {
    p_stretch.ClearPairs();

    //If the width is zero update and return.
    if(p_width == 0) {
      emit(update());
      return;
    }

    double stepSize = 0.0001;
    if(p_start > 0) stepSize = p_start/10000;
 
    //The end point of the window
    double end = p_start+p_width+stepSize;
    if(end > p_max) end = p_max;

    //If the starting point is greater than the minimum add the minimum first
    if(p_start > p_min) {
      p_stretch.AddPair(p_min, 0.0);
    }
    
    //Make sure values before the window are set to 0
    p_stretch.AddPair(p_start, 0.0);

    //The start of the window
    p_stretch.AddPair(p_start+stepSize, 255.0);

    //The end of the window
    p_stretch.AddPair(end-stepSize, 255.0);

    //If the end is less than the maximum, make sure values after the window are set to 0
    if(end < p_max) {
      p_stretch.AddPair(end, 0.0);
      p_stretch.AddPair(p_max, 0.0);
    }

    emit(update());
  }
}
