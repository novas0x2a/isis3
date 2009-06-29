#ifndef HistogramWidget_h
#define HistogramWidget_h

#include "Cube.h"
#include "Stretch.h"
#include "Histogram.h"

#include <QWidget>
#include <QStackedWidget>
#include <QSlider>
#include <QLineEdit>
#include <QDoubleValidator>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>

#include "HistogramItem.h"


namespace Qisis {
/**
 * @brief Histogram widget used by AdvancedStretchTool 
 *  
 *  
 * The HistogramWidget displays a given histogram and stretch in a graph and 
 * contains inputs for changing the min/max of the histogram.  
 *
 * @ingroup Visualization Tools 
 *  
 * @author 2009-05-01 Noah Hilt 
 *
 */
  class HistogramWidget : public QWidget {
    Q_OBJECT

  public:
    HistogramWidget(const QString title, const QColor histColor=Qt::gray, const QColor stretchColor=Qt::darkGray);

    void setAbsoluteMinMax(const double min, const double max);
    void setMinMax(const double min, const double max);
    void setHistogram(Isis::Histogram hist);
    void setStretch(Isis::Stretch stretch);

    void clearStretch();

  signals:
    /**
     * Signal to emit when the min/max has changed
     * 
     * @param min 
     * @param max 
     */
    void rangeChanged(const double min, const double max);

  private slots:
    void minChanged(const int min);
    void maxChanged(const int max);
    void minEditChanged();
    void maxEditChanged();

  private:
    QSlider *p_minSlider; //!< QSlider to change the minimum of the histogram
    QSlider *p_maxSlider; //!< QSlider to change the maximum of the histogram

    QDoubleValidator *p_validator; //!< Validates the min/max edit fields to make sure they are doubles
    QLineEdit *p_minEdit; //!< Text field to set the minimum of the histogram
    QLineEdit *p_maxEdit; //!< Text field to set the maximum of the histogram

    QwtPlot *p_plot; //!< The plot to graph the histogram and stretch curve

    HistogramItem *p_histCurve; //!< The histogram curve
    QwtPlotCurve *p_stretchCurve; //!< The stretch curve

    double p_min; //!< The minimum value the histogram's minimum can be set to
    double p_max; //!<  The maximum value the histogram's maximum can be set to
    double p_scale; //!< The scale to convert between the QSlider values and min/max values
  };
};

#endif
