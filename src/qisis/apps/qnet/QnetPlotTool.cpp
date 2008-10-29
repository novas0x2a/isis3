#include <QApplication>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QPushButton>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QGridLayout>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPainter>

#include "QnetPlotTool.h"
#include "MainWindow.h"
#include "CubeViewport.h"
#include "Workspace.h"
#include "ImageOverlap.h"
#include "ProjectionFactory.h"
#include "SerialNumber.h"
#include "PolygonTools.h"

#include "geos/geom/CoordinateSequence.h"
#include "geos/geom/MultiPolygon.h"

#include "qnet.h"

using namespace Isis;
using namespace Qisis::Qnet;
using namespace std;

namespace Qisis {

  // Constructor
  QnetPlotTool::QnetPlotTool (QWidget *parent) : Qisis::Tool(parent) {
    p_activePolygon = -1;
    p_editPointId = "";
//    createPlotDialog(parent);
  }


  // Take care of drawing things on a viewPort.
  // This is overiding the parents paintViewport member.
  void QnetPlotTool::paintViewport (CubeViewport *vp,QPainter *painter) {

    if (g_activeOverlaps.size() > 0) drawActiveOverlaps (vp,painter);

    drawAllMeasurments (vp,painter);

  }


  // Take care of drawing things on all viewPorts.
  // Calling update will cause the Tool class to call all registered tools 
  void QnetPlotTool::paintAllViewports (std::string pointId) {

    p_editPointId = pointId;
    CubeViewport *vp;
    for (int i=0; i<(int)cubeViewportList()->size(); i++) {
      vp = (*(cubeViewportList()))[i];

//       cout<<"before paintViewport vp= "<<i<<endl;
//       paintViewport(vp,painter);
//       cout<<"after paintViewport vp= "<<i<<endl;
      vp->update();
    }

  }



  // Draw the active overlaps on all the viewPorts
  void QnetPlotTool::drawActiveOverlaps() {

    // Look at each active overlap area
    for (int ovIndex=0; ovIndex<g_activeOverlaps.size(); ovIndex++) {
      const ImageOverlap *overlap = (*g_imageOverlap)[ovIndex];

      // Look at each viewPort
      for (int i=0; i<(int)cubeViewportList()->size(); i++) {
        CubeViewport *d = (*(cubeViewportList()))[i];

        // If the SN from the viewPort matches one of SNs in this overlap, draw it
        for (int sn=0; sn<overlap->Size(); sn++ ) {
          
        }

        Isis::Camera *cam = d->camera();
        geos::geom::MultiPolygon *poly = PolygonTools::CopyMultiPolygon(
            (*g_imageOverlap)[p_activePolygon]->Polygon());
        geos::geom::CoordinateSequence *seq = poly->getCoordinates();
        if (seq->getSize() == 0) break;
        double lon = seq->getAt(0).x;
        double lat = seq->getAt(0).y;
        //std::cout << "good = " << cam->SetUniversalGround(lat,lon) << std::endl;
        double samp = cam->Sample();
        double line = cam->Line();
        int sx,sy;
        d->cubeToViewport(samp,line,sx,sy);
        QPainter painter(d->viewport());
        painter.setPen(QColor(255,0,0));
        d->viewport()->setAttribute(Qt::WA_PaintOutsidePaintEvent,true);
        for (unsigned int p=1; p<seq->getSize(); p++) {
          lon = seq->getAt(p).x;
          lat = seq->getAt(p).y;
          //std::cout << lat << " " << lon << std::endl;
          //std::cout <<cam->SetUniversalGround(lat,lon) << " ";
          samp = cam->Sample();
          line = cam->Line();
          int ex,ey;
          d->cubeToViewport(samp,line,ex,ey);
          //  std::cout << sx << " " << sy << " " << ex << " " << ey << std::endl;
          painter.drawLine(sx,sy,ex,ey);
          
          sx = ex;
          sy = ey;
        }
        d->viewport()->setAttribute(Qt::WA_PaintOutsidePaintEvent,false);
        //painter.end();
      }
    }
  }


  void QnetPlotTool::drawPoints() {
  }

  void QnetPlotTool::drawPolygons() {
  }


  void QnetPlotTool::createPlotDialog(QWidget *parent) {
    // Make the plot area
    p_plotArea = new QWidget();
    p_plotArea->setMinimumSize(400,400);

    // Create the
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(p_plotArea,0,0);

    // Layout everything in the dialog
    p_plotDialog = new QDialog(parent);
    p_plotDialog->setWindowTitle("Network Plot");
    p_plotDialog->setLayout(gridLayout);
    p_plotDialog->setShown(true);

    p_plotArea->installEventFilter(this);

    Isis::PvlGroup mapping("Mapping");
    mapping += Isis::PvlKeyword("ProjectionName","Sinusoidal");
    mapping += Isis::PvlKeyword("LatitudeType","Planetocentric");
    mapping += Isis::PvlKeyword("LongitudeDirection","PositiveEast");
    mapping += Isis::PvlKeyword("EquatorialRadius",3396190.0);
    mapping += Isis::PvlKeyword("PolarRadius",3376200.0);
    mapping += Isis::PvlKeyword("LongitudeDomain",360);
    mapping += Isis::PvlKeyword("MinimumLatitude",18.0);
    mapping += Isis::PvlKeyword("MaximumLatitude",38.0);
    mapping += Isis::PvlKeyword("MinimumLongitude",10.0);
    mapping += Isis::PvlKeyword("MaximumLongitude",24.0);
    mapping += Isis::PvlKeyword("CenterLongitude",17.0);

    Isis::Pvl pvl;
    pvl.AddGroup(mapping);
    p_plotProjection = Isis::ProjectionFactory::Create(pvl);
  }


  // Handel events on the plot window
  bool QnetPlotTool::eventFilter(QObject *o, QEvent *e) {
    if (e->type() == QEvent::Paint) {
      if (g_imageOverlap == NULL) return TRUE;

      //std::cout << "Paint plot area" << std::endl;
      QWidget *w = (QWidget *) o;
      QPainter p(w);
      p.fillRect(0,0,w->width(),w->height(),QColor(0,0,0));
      p.setPen(QColor(255,0,0));

      double pxmin,pxmax,pymin,pymax;
      //std::cout << "XYRange = " << p_plotProjection->XYRange(pxmin,pxmax,pymin,pymax) << std::endl;
      p_plotProjection->XYRange(pxmin,pxmax,pymin,pymax);
      int minSize = w->width() < w->height() ? w->width() : w->height();
      //std::cout << "minSize = " << minSize << std::endl;

      // Draw all the overlaps
      for (int i=0; i<g_imageOverlap->Size(); i++) {
        //std::cout << "paint overlap " << i << endl;
        geos::geom::MultiPolygon *poly = PolygonTools::CopyMultiPolygon((*g_imageOverlap)[i]->Polygon());
        geos::geom::CoordinateSequence *seq = poly->getCoordinates();

        if (seq->getSize() == 0) break;
        double lon = seq->getAt(0).x;
        double lat = seq->getAt(0).y;
        p_plotProjection->SetUniversalGround(lat,lon);
        double px = p_plotProjection->WorldX();
        double py = p_plotProjection->WorldY();

        int sx,sy;
        sx = (int)(double(minSize) / double(pxmax - pxmin) * (px - pxmin) + 0);
        sy = (int)(double(minSize) / double(pymin - pymax) * (py - pymax) + 0);
        for (unsigned int j=1; j<seq->getSize(); j++) {
          //std::cout << sx << " " << sy << std::endl;
          lon = seq->getAt(j).x;
          lat = seq->getAt(j).y;
          p_plotProjection->SetUniversalGround(lat,lon);
          double px = p_plotProjection->XCoord();
          double py = p_plotProjection->YCoord();
          int ex,ey;
          ex = (int)(double(minSize) / double(pxmax - pxmin) * (px - pxmin) + 0);
          ey = (int)(double(minSize) / double(pymin - pymax) * (py - pymax) + 0);
          p.drawLine(sx,sy,ex,ey);
          sx = ex;
          sy = ey;
        }
      }

      // Draw the active overlaps on the plot window
      for (int overlap=0; overlap<g_activeOverlaps.size(); overlap++) {
        int row = g_activeOverlaps[overlap];
        p.setPen(QColor(0,255,0));
        if (row != -1) {
          //std::cout << "paint active overlap " << row << endl;
          geos::geom::MultiPolygon *poly = PolygonTools::CopyMultiPolygon((*g_imageOverlap)[row]->Polygon());
          geos::geom::CoordinateSequence *seq = poly->getCoordinates();
          if (seq->getSize() == 0) break;
          double lon = seq->getAt(0).x;
          double lat = seq->getAt(0).y;
          p_plotProjection->SetUniversalGround(lat,lon);
          double px = p_plotProjection->WorldX();
          double py = p_plotProjection->WorldY();

          int sx,sy;
          sx = (int) (double (minSize) / double (pxmax - pxmin) *
               (px - pxmin) + 0);
          sy = (int) (double (minSize) / double (pymin - pymax) *
               (py - pymax) + 0);
          for (unsigned int j=1; j<seq->getSize(); j++) {
            //std::cout << sx << " " << sy << std::endl;
            lon = seq->getAt(j).x;
            lat = seq->getAt(j).y;
            p_plotProjection->SetUniversalGround(lat,lon);
            double px = p_plotProjection->XCoord();
            double py = p_plotProjection->YCoord();
            int ex,ey;
            ex = (int) (double (minSize) / double (pxmax - pxmin) *
                 (px - pxmin) + 0);
            ey = (int) (double (minSize) / double (pymin - pymax) *
                 (py - pymax) + 0);
            p.drawLine(sx,sy,ex,ey);
            sx = ex;
            sy = ey;
          }
        }
      }
      //p.end();
    }
    return TRUE;
  }


  void QnetPlotTool::updateSelected () {
    p_plotArea->repaint();
  }


  // Draw all overlaps that fall on this viewPort
  void QnetPlotTool::drawAllOverlaps (CubeViewport *vp,QPainter *painter) {
  }


  // Draw the active overlaps on a viewport
  // TODO: make this work for multipolygons
  // TODO: make this work for mosaics (use UniversalGroundMap instead of camera)
  void QnetPlotTool::drawActiveOverlaps (CubeViewport *vp,QPainter *painter) {


    for (int index=0; index<g_activeOverlaps.size(); index++) {
      int overlap = g_activeOverlaps[index];
      Isis::Camera *cam = vp->camera();
      geos::geom::MultiPolygon *poly = PolygonTools::CopyMultiPolygon((*g_imageOverlap)[overlap]->Polygon());
      geos::geom::CoordinateSequence *seq = poly->getCoordinates();
      if (seq->getSize() == 0) break;
      double lon = seq->getAt(0).x;
      double lat = seq->getAt(0).y;
      cam->SetUniversalGround(lat,lon);
      double samp = cam->Sample();
      double line = cam->Line();
      int sx,sy;
      vp->cubeToViewport(samp,line,sx,sy);
      painter->setPen(QColor(255,0,0));
      for (unsigned int p=1; p<seq->getSize(); p++) {
        lon = seq->getAt(p).x;
        lat = seq->getAt(p).y;
        cam->SetUniversalGround(lat,lon);
        samp = cam->Sample();
        line = cam->Line();
        int ex,ey;
        vp->cubeToViewport(samp,line,ex,ey);
        painter->drawLine(sx,sy,ex,ey);
        sx = ex;
        sy = ey;
      }
    }
  }


  // Draw all measurments which are on this viewPort
  void QnetPlotTool::drawAllMeasurments (CubeViewport *vp,QPainter *painter) {
    // Without a controlnetwork there are no points
    if (g_controlNetwork == 0) return;

    // Don't show the measurments on cubes not in the serial number list
    // TODO: Should we show them anyway
    // TODO: Should we add the SN to the viewPort
    std::string serialNumber = SerialNumber::Compose(*vp->cube());
    if (!g_serialNumberList->HasSerialNumber(serialNumber)) return;

    // Draw the measurments on the viewport
    for (int i=0; i<g_controlNetwork->Size(); i++) {
      Isis::ControlPoint &p = (*g_controlNetwork)[i];
      if (p.Id() == p_editPointId) {
        painter->setPen(QColor(200,0,0));
      }
      else if (p.Ignore()) {
        painter->setPen(QColor(200,200,0));
      }
      else {
        painter->setPen(QColor(0,200,0));
      }
      for (int j=0; j<p.Size(); j++) {
        if (p[j].CubeSerialNumber() == serialNumber) {
          double samp = p[j].Sample();
          double line = p[j].Line();
          int x,y;
          vp->cubeToViewport(samp,line,x,y);
          painter->drawLine(x-5,y,x+5,y);
          painter->drawLine(x,y-5,x,y+5);
        }
      }
    }
  }


  // Draw the acitve measurments that fall on this viewPort
  void QnetPlotTool::drawActiveMeasurments (CubeViewport *vp,QPainter *painter) {
  }


  // Draw all footprints that fall on this viewPort
  void QnetPlotTool::drawAllFootprints (CubeViewport *vp,QPainter *painter) {
  }


  // Draw the active footprints that fall on this viewPort
  void QnetPlotTool::drawActiveFootprints (CubeViewport *vp,QPainter *painter) {
  }




  // Draw all overlaps on the plot
  void QnetPlotTool::plotAllOverlaps () {
  }


  // Draw the active overlaps on the plot
  void QnetPlotTool::plotActiveOverlaps () {
  }


  // Draw all measurments on the plot
  void QnetPlotTool::plotAllMeasurments () {
  }


  // Draw the active measurments on the plot
  void QnetPlotTool::plotActiveMeasurments () {
  }


  // Draw all footprints on the plot
  void QnetPlotTool::plotAllFootprints () {
  }


  // Draw the active footprints on the plot
  void QnetPlotTool::plotActiveFootprints () {
  }
}
