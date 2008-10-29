#ifndef HistogramToolWindow_h
#define HistogramToolWindow_h

#include "PlotWindow.h"
#include "HistogramToolCurve.h"
#include "Histogram.h"

namespace Qisis {
  class HistogramToolWindow : public Qisis::PlotWindow {
    Q_OBJECT

    public:     
      HistogramToolWindow(QString title,QWidget *parent);
      void add(HistogramToolCurve *pc);
      void setViewport(CubeViewport *cvp);
      
    public slots:

   
    private:
      CubeViewport  *p_cvp; //!< The current viewport
  };
};

#endif
