#include "SawtoothStretch.h"

#include <QLabel>
#include <QBoxLayout>

namespace Qisis {
  /**
   * SawtoothStretch Constructor. StretchTool can not be null. 
   *  
   * Initializes the stretch, QSliders, and sets the name of 
   * this QStretch to be 'Sawtooth' 
   * 
   * @param stretchTool 
   */
  SawtoothStretch::SawtoothStretch(const StretchTool *stretchTool) : QStretch(stretchTool, "Sawtooth") {
    p_start = 0;
    p_sawtooths = 1;

    p_startSlider = new QSlider();
    p_startSlider->setRange(1, 254);
    p_startSlider->setOrientation(Qt::Horizontal);
    p_startSlider->setTickPosition(QSlider::TicksBelow);
    p_sawtoothsSlider = new QSlider();
    p_sawtoothsSlider->setOrientation(Qt::Horizontal);
    p_sawtoothsSlider->setTickPosition(QSlider::TicksBelow);
    p_sawtoothsSlider->setTickInterval(1);
    p_sawtoothsSlider->setRange(1, 10);
  }

  /**
   * Creates a new SawtoothStretch with this SawtoothStretch's parameters and 
   * returns it. 
   * 
   * 
   * @return BinaryStretch* 
   */
  SawtoothStretch *SawtoothStretch::clone() {
    SawtoothStretch *stretch = new SawtoothStretch(p_stretchTool);
    stretch->setMinMax(p_min, p_max);
    stretch->p_stretch = this->p_stretch;
    stretch->p_startSlider->setValue(p_startSlider->value());
    stretch->p_sawtoothsSlider->setValue(p_sawtoothsSlider->value());
    stretch->p_start = this->p_start;
    stretch->p_sawtooths = this->p_sawtooths;
    return stretch;
  }

  /**
   * Returns the QGroupBox holding parameters.
   * 
   * 
   * @return QGroupBox* 
   */
  QGroupBox *SawtoothStretch::getParameters() {
    //If the parameters box hasn't been created yet, create it
    if(p_parametersBox == NULL) {
      p_parametersBox = new QGroupBox("Sawtooth Parameters");
  
      QLabel *startLabel = new QLabel("Start-Point");
      QLabel *sawtoothsLabel = new QLabel("Number of Sawtooths");

      connect(p_startSlider, SIGNAL(valueChanged(int)), this, SLOT(changeStart(int)));
      connect(p_sawtoothsSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSawtooths(int)));
  
      QVBoxLayout *layout = new QVBoxLayout();
      layout->addWidget(startLabel);
      layout->addWidget(p_startSlider);
      layout->addWidget(sawtoothsLabel);
      layout->addWidget(p_sawtoothsSlider);
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
  void SawtoothStretch::setMinMax(double min, double max) {
    QStretch::setMinMax(min, max);
    p_scale = 255.0/(max-min);
    changeStart(p_startSlider->value());
  }

  /**
   * Changes the starting point to the scaled value and computes the sawtooth 
   * stretch. 
   * 
   * @param value 
   */
  void SawtoothStretch::changeStart(int value) {
    p_start = p_min + (value/p_scale);
    computeSawtooths();
  }

  /**
   * Changes the number of sawtooths to the value and computers the sawtooth 
   * stretch. 
   * 
   * @param value 
   */
  void SawtoothStretch::changeSawtooths(int value) {
    p_sawtooths = value;
    computeSawtooths();
  }

  /**
   * Computes the sawtooth stretch from the starting point and number of
   * sawtooths.
   * 
   */
  void SawtoothStretch::computeSawtooths() {
    p_stretch.ClearPairs();

    //If the number of sawtooths is zero, update and return.
    if(p_sawtooths == 0) {
      emit(update());
      return;
    }

    //The interval of one sawtooth (start to end)
    double interval = (p_max - p_min) / (double)p_sawtooths;

    //The list of ordered pairs
    QList<QPair<double, double> > pairsList;

    //The slope of the sawtooths
    double slope = 255.0/(interval/2.0);
    //The offset of the first sawtooth
    double offset = p_start-(interval/2.0);
    //The alternating output pair value
    double value = 255.0;

    /**
     * To compute the sawtooths from min to max we begin at the start point and work
     * backwards to the minimum, prepending new pairs, then begin again at the start
     * point and work towards the maximum, appending new pairs.
     * 
     * The min/max output pairs are a special case since a sawtooth's endpoints may
     * not lie on the min/max, so the slope of the sawtooth is calculated to ensure
     * only pairs in the min-max range are added.
     */


    //While the offset is greater than the minimum, add alternating 0/255 pairs
    while(offset > p_min) {
      pairsList.prepend(qMakePair(offset, value));
      offset -= interval/2.0;
      //Alternate output pair value
      value = value == 0.0 ? 255.0 : 0.0;
    }

    //At the minimum point we need to calculate the output based on the slope 
    //of the sawtooths
    if(value == 0.0) {
      value = (p_min - offset) * slope;
    }
    else {
      value = 255.0 - (p_min - offset) * slope;
    }
    
    //Add the minimum value pair
    pairsList.prepend(qMakePair(p_min, value));

    //Add the starting point
    pairsList.append(qMakePair(p_start, 0.0));

    offset = p_start+(interval/2.0); 
    value = 255.0;
    //While the offset is less than the maximum, add alternating 0/255 pairs
    while(offset < p_max) {
      pairsList.append(qMakePair(offset, value));
      offset += interval/2.0;
      value = value == 0.0 ? 255.0 : 0.0;
    }

    //At the maximum point we need to calculate the output based on the slope 
    //of the sawtooths
    if(value == 0.0) {
      value = (offset - p_max) * slope;
    }
    else {
      value = 255.0 - (offset - p_max) * slope;
    }

    //Add the maximum value pair
    pairsList.append(qMakePair(p_max, value));

    //Now add all of the ordered pairs to the stretch
    for(int i = 0; i < pairsList.size(); i++) {
      p_stretch.AddPair(pairsList[i].first, pairsList[i].second);
    }

    emit(update());
  }
}
