#include "HistogramToolWindow.h"

namespace Qisis {
  /**
   * Constructor, creates a new HistogramToolWindow
   * 
   * @param title 
   * @param parent 
   */
  HistogramToolWindow::HistogramToolWindow(QString title,QWidget *parent) : PlotWindow(title, parent) {
    p_plot->enableAxis(QwtPlot::yRight);
    QwtText *leftLabel = new QwtText("Frequency",
                                     QwtText::PlainText);
    leftLabel->setColor(Qt::red);
    QFont font = leftLabel->font();
    font.setPointSize(13);
    font.setBold(true);
    leftLabel->setFont(font);
    p_plot->setAxisTitle(QwtPlot::yLeft, *leftLabel);

    QwtText *rtLabel = new QwtText("Percentage",
                                   QwtText::PlainText);
    rtLabel->setColor(Qt::blue);
    font = rtLabel->font();
    font.setPointSize(13);
    font.setBold(true);
    rtLabel->setFont(font);
    p_plot->setAxisTitle(QwtPlot::yRight,*rtLabel);    

    setAxisLabel(QwtPlot::xBottom,"Pixel Value (DN)");

    setScale(QwtPlot::yRight,0,100);

    setPlotBackground(Qt::white);
  }


  /**
   * This method adds a HistogramToolCurve to the window, sets the symbols to be 
   * hidden, and the curve to be visible. 
   * 
   * @param pc 
   */
  void HistogramToolWindow::add(HistogramToolCurve *pc){
    pc->setSymbolVisible(false);
    PlotWindow::add(pc);
    pc->setVisible(true);
    p_plot->replot();
  }

  /** 
   * This class needs to know which viewport the user is looking 
   * at so it can appropriately draw in the band lines. 
   * 
   * @param cvp
   */
  void HistogramToolWindow::setViewport(CubeViewport *cvp){
    if(cvp == NULL) return;
    p_cvp = cvp;
  }
}
