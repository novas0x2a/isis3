#ifndef QnetPlotTool_h
#define QnetPlotTool_h

#include "geos/geom/MultiPolygon.h"

#include "Tool.h"
#include "Projection.h"

namespace Qisis {
  class QnetPlotTool : public Tool {
    Q_OBJECT

    signals:
      void selectionUpdate();

    public:
      QnetPlotTool (QWidget *parent);
      void paintViewport (CubeViewport *cvp,QPainter *painter);
      bool eventFilter(QObject *o, QEvent *e);

    public slots:
      void updateSelected();
      void paintAllViewports (std::string pointId);

    private slots:
      void drawPolygons();
      void drawPoints();

    private:
      void createPlotDialog(QWidget *parent);

      void drawPolygon (geos::geom::MultiPolygon *poly, CubeViewport *vp, QColor color);
      void drawActiveOverlaps ();

      void drawAllOverlaps (CubeViewport *vp,QPainter *painter);
      void drawActiveOverlaps (CubeViewport *vp,QPainter *painter);
      void drawAllMeasurments (CubeViewport *vp,QPainter *painter);
      void drawActiveMeasurments (CubeViewport *vp,QPainter *painter);
      void drawAllFootprints(CubeViewport *vp,QPainter *painter);
      void drawActiveFootprints (CubeViewport *vp,QPainter *painter);

      void plotPolygon (geos::geom::MultiPolygon *poly, QColor color);
      void plotAllOverlaps ();
      void plotActiveOverlaps ();
      void plotAllMeasurments ();
      void plotActiveMeasurments ();
      void plotAllFootprints ();
      void plotActiveFootprints ();

      int p_activePolygon;
      QDialog *p_plotDialog;
      QWidget *p_plotArea;
      Isis::Projection *p_plotProjection;
      std::string p_editPointId;
  };
};

#endif
