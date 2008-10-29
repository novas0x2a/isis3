#ifndef PlotToolWindow_h
#define PlotToolWindow_h
#include "PlotWindow.h"

namespace Qisis {
  class PlotToolWindow : public Qisis::PlotWindow {
    Q_OBJECT

    public:
      
      PlotToolWindow(QString title,QWidget *parent);
      bool bandMarkersVisible();
      void setViewport(CubeViewport *cvp);
      void setPlotType(QString plotType);
      bool p_markersVisible;
      
    public slots:
      void drawBandMarkers();
      void setBandMarkersVisible(bool visible);
      void showHideLines();
      void fillTable();
      void fillTableSpectral();
      void fillTableSpatial();
      void setStdDev(std::vector<double> stdDevArray);
   
    private:
      void setAutoScaleOption(bool autoScale = false);
      QwtPlotMarker *p_grayBandLine;
      QwtPlotMarker *p_redBandLine;
      QwtPlotMarker *p_greenBandLine;
      QwtPlotMarker *p_blueBandLine;
      CubeViewport  *p_cvp;
      QString p_plotType;
      Isis::PvlKeyword p_wavelengths;
      bool p_autoScale;
      std::vector<double> p_stdDevArray;

  };
};

#endif

