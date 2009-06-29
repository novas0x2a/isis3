#include "HistogramWidget.h"

#include <QHBoxLayout>
#include <QLayout>
#include <QLabel>

#include <qwt_symbol.h>
#include <qwt_interval_data.h>
#include <qwt_double_interval.h>
#include <qwt_scale_div.h>

namespace Qisis {
  /**
   * HistogramWidget constructor. Initializes all of the widgets and sets the plot 
   * title, histogram curve's color and stretch curve's color. 
   * 
   * @param title 
   * @param histColor 
   * @param stretchColor 
   */
  HistogramWidget::HistogramWidget(const QString title, const QColor histColor, const QColor stretchColor) : QWidget() {
    p_plot = new QwtPlot(QwtText(title));
    p_plot->setCanvasBackground(Qt::white);
    p_plot->enableAxis(QwtPlot::yRight);
    p_plot->setAxisScale(QwtPlot::xBottom, 0, 255);
    p_plot->setAxisLabelRotation(QwtPlot::xBottom, 45);
    p_plot->setAxisScale(QwtPlot::yRight, 0, 255);

    p_histCurve = new HistogramItem();
    p_histCurve->setColor(histColor);

    p_stretchCurve = new QwtPlotCurve();
    p_stretchCurve->setYAxis(QwtPlot::yRight);
    p_stretchCurve->setPen(QPen(QBrush(stretchColor), 2, Qt::DashLine));
    p_stretchCurve->setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(stretchColor), QPen(stretchColor), QSize(5, 5)));

    p_histCurve->attach(p_plot);
    p_stretchCurve->attach(p_plot);

    QLabel *minLabel = new QLabel("Min");
    QLabel *maxLabel = new QLabel("Max");

    p_minSlider = new QSlider(Qt::Horizontal);
    p_minSlider->setRange(0, 100);
    p_minSlider->setSingleStep(1);
    p_minSlider->setTickInterval(5);
    p_minSlider->setTickPosition(QSlider::TicksBelow);
    p_minSlider->setMinimumWidth(150);
    p_maxSlider = new QSlider(Qt::Horizontal);
    p_maxSlider->setRange(0, 100);
    p_maxSlider->setValue(100);
    p_maxSlider->setSingleStep(1);
    p_maxSlider->setTickInterval(5);
    p_maxSlider->setTickPosition(QSlider::TicksBelow);
    p_maxSlider->setMinimumWidth(150);

    connect(p_minSlider, SIGNAL(valueChanged(int)), this, SLOT(minChanged(int)));
    connect(p_maxSlider, SIGNAL(valueChanged(int)), this, SLOT(maxChanged(int)));

    p_minEdit = new QLineEdit();
    p_minEdit->setMaximumWidth(100);

    p_maxEdit = new QLineEdit();
    p_maxEdit->setMaximumWidth(100);

    p_validator = new QDoubleValidator(p_minEdit);
    p_minEdit->setValidator(p_validator);
    p_maxEdit->setValidator(p_validator);

    connect(p_minEdit, SIGNAL(editingFinished()), this, SLOT(minEditChanged()));
    connect(p_maxEdit, SIGNAL(editingFinished()), this, SLOT(maxEditChanged())); 

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setColumnStretch(0, 1);
    mainLayout->addWidget(p_plot, 0, 0, 1, 5);
    mainLayout->addWidget(minLabel, 1, 1);
    mainLayout->addWidget(p_minSlider, 1, 2);
    mainLayout->addWidget(p_minEdit, 1, 3);
    mainLayout->addWidget(maxLabel, 2, 1);
    mainLayout->addWidget(p_maxSlider, 2, 2);
    mainLayout->addWidget(p_maxEdit, 2, 3);
    mainLayout->setColumnStretch(4, 1);
    this->setLayout(mainLayout);
  }

  /**
   * Sets the absolute min/max that the histogram's min/max can go to and updates 
   * the scale. 
   * 
   * @param min 
   * @param max 
   */
  void HistogramWidget::setAbsoluteMinMax(double min, double max) {
      p_min = min;
      p_max = max;
      p_scale = (max - min)/100.0;
  }

  /**
   * Sets the min/max sliders and text fields values to min/max.
   * 
   * @param min 
   * @param max 
   */
  void HistogramWidget::setMinMax(double min, double max) {
    int imin, imax;
    //Disconnect the sliders so they don't trigger an update
    disconnect(p_minSlider, SIGNAL(valueChanged(int)), this, SLOT(minChanged(int)));
    disconnect(p_maxSlider, SIGNAL(valueChanged(int)), this, SLOT(maxChanged(int)));

    //Get the min/max in the slider's scale
    imin = (int)((min - p_min) / p_scale);
    imax = (int)((max - p_min) / p_scale);

    p_minSlider->setValue(imin);
    p_maxSlider->setValue(imax);
    p_minEdit->setText(QString("%1").arg(min));
    p_maxEdit->setText(QString("%1").arg(max));

    //Reconnect the sliders
    connect(p_minSlider, SIGNAL(valueChanged(int)), this, SLOT(minChanged(int)));
    connect(p_maxSlider, SIGNAL(valueChanged(int)), this, SLOT(maxChanged(int)));
  }

  /**
   * Creates a histogram curve from the given histogram and plots it.
   * 
   * @param hist 
   */
  void HistogramWidget::setHistogram(Isis::Histogram hist) { 
    std::vector<double> xarray,yarray;
    for (int i=0; i<hist.Bins(); i++) {
      if (hist.BinCount(i) > 0) {
        xarray.push_back(hist.BinMiddle(i));
        yarray.push_back(hist.BinCount(i));
      }
    }

    //These are all variables needed in the following for loop.
    //----------------------------------------------
    QwtArray<QwtDoubleInterval> intervals(xarray.size());
    QwtValueList majorTicks;
    QwtArray<double> values(yarray.size());
    double maxYValue = DBL_MIN;
    double minYValue = DBL_MAX;
    // --------------------------------------------- 

    for(unsigned int y = 0; y < yarray.size(); y++) {

      intervals[y] = QwtDoubleInterval(xarray[y], xarray[y] + hist.BinSize());
     
      majorTicks.push_back(xarray[y]);
      majorTicks.push_back(xarray[y]+ hist.BinSize());

      values[y] = yarray[y];  
      if(values[y] > maxYValue) maxYValue = values[y]; 
      if(values[y] < minYValue) minYValue = values[y];
    }
    
    QwtScaleDiv scaleDiv;
    scaleDiv.setTicks(QwtScaleDiv::MajorTick, majorTicks);

    p_histCurve->setData(QwtIntervalData(intervals, values));
    p_plot->setAxisScale(QwtPlot::xBottom, hist.Minimum(), hist.Maximum());
  }

  /**
   * Creates a stretch curbe from the given stretch and plots it.
   * 
   * @param stretch 
   */
  void HistogramWidget::setStretch(Isis::Stretch stretch) {
    std::vector<double> xarray,yarray;
    for (int i = 0; i < stretch.Pairs(); i++) {
      xarray.push_back(stretch.Input(i));
      yarray.push_back(stretch.Output(i));
    }

    p_stretchCurve->setData(&xarray[0],&yarray[0],xarray.size());
    p_plot->replot();
  }
  
  /**
   * Clears the stretch curve from the plot.
   * 
   */
  void HistogramWidget::clearStretch() {
    p_stretchCurve->setData(NULL, NULL, 0);
    p_plot->replot();
  }

  /**
   * When the min slider has changed, calculate the new minimum and emit a signal 
   * that it has changed. 
   * 
   * @param min 
   */
  void HistogramWidget::minChanged(const int min) {
    //If min is greater than the current maximum, set min to be just below the current maximum and return
    if(min >= p_maxSlider->value()) {
      disconnect(p_minSlider, SIGNAL(valueChanged(int)), this, SLOT(minChanged(int)));
      p_minSlider->setValue(p_maxSlider->value() - 1);
      connect(p_minSlider, SIGNAL(valueChanged(int)), this, SLOT(minChanged(int)));
      return;
    }

    //Calculate the min and max values
    double dmin = p_min + (min * p_scale);
    double dmax = p_min + (p_maxSlider->value() * p_scale);

    //Update the min text field with the new min
    p_minEdit->setText(QString("%1").arg(dmin));

    //Emit a signal that the min has changed
    emit rangeChanged(dmin, dmax);
  }

  /**
   * When the max slider has changed, calculate the new maximum and emit a signal 
   * that it has changed. 
   * 
   * @param max 
   */
  void HistogramWidget::maxChanged(const int max) {
    //If max is less than the current minimum, set max to be just above the current minimum and return
    if(max <= p_minSlider->value()) {
      disconnect(p_maxSlider, SIGNAL(valueChanged(int)), this, SLOT(maxChanged(int)));
      p_maxSlider->setValue(p_minSlider->value() + 1);
      connect(p_maxSlider, SIGNAL(valueChanged(int)), this, SLOT(maxChanged(int)));
      return;
    }

    //Calculate the min and max values
    double dmin = p_min + (p_minSlider->value() * p_scale);
    double dmax = p_min + (max * p_scale);

    //Update the max text field with the new max
    p_maxEdit->setText(QString("%1").arg(dmax));

    //Emit a signal that the max has changed
    emit rangeChanged(dmin, dmax);
  }

  /**
   * When the min text field has changed, validate the new minimum and emit a
   * signal that it has changed if valid.
   * 
   */
  void HistogramWidget::minEditChanged() {
    double min = p_minEdit->text().toDouble();
    double max = p_maxEdit->text().toDouble();

    //If the min entered is less than the absolute minimum of the histogram
    //set it to the absolute minimum and update the min text field
    if(min < p_min) {
      min = p_min;
      p_minEdit->setText(QString("%1").arg(min));
    }

    //If min is greater than or equal to max, return
    if(min >= max) {
      return;
    }

    //Calculate the min in the slider scale and set the min slider to the new minimum
    int imin = (int)((min - p_min) / p_scale);
    disconnect(p_minSlider, SIGNAL(valueChanged(int)), this, SLOT(minChanged(int)));
    p_minSlider->setValue(imin);
    connect(p_minSlider, SIGNAL(valueChanged(int)), this, SLOT(minChanged(int)));

    //Emit a signal that the min has changed
    emit rangeChanged(min, max);
  }

  /**
   * When the max text field has changed, validate the new maximum and emit a
   * signal that it has changed if valid.
   * 
   */
  void HistogramWidget::maxEditChanged() {
    double min = p_minEdit->text().toDouble();
    double max = p_maxEdit->text().toDouble();

    //If the max entered is greater than the absolute maximum of the histogram
    //set it to the absolute maximum and update the max text field
    if(max > p_max) {
      max = p_max;
      p_maxEdit->setText(QString("%1").arg(max));
    }

    //If max is less than or equal to min, return
    if(max <= min) {
      return;
    }

    //Calculate the max in the slider scale and set the max slider to the new maximum
    int imax = (int)((max - p_min) / p_scale);
    disconnect(p_maxSlider, SIGNAL(valueChanged(int)), this, SLOT(maxChanged(int)));
    p_maxSlider->setValue(imax);
    connect(p_maxSlider, SIGNAL(valueChanged(int)), this, SLOT(maxChanged(int)));

    //Emit a signal that the max has changed
    emit rangeChanged(min, max);
  }
}
