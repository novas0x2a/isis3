#include <QApplication>
#include <QToolBar>
#include "iException.h"
#include "ViewportMainWindow.h"
#include "QnetFileTool.h"
#include "QnetPlotTool.h"
#include "QnetNavTool.h"
#include "BandTool.h"
#include "ZoomTool.h"
#include "PanTool.h"
#include "StretchTool.h"
#include "FindTool.h"
#include "WindowTool.h"
#include "AdvancedTrackTool.h"
#include "HelpTool.h"
#include "TieTool.h"
#include "RubberBandTool.h"

#define IN_QNET
#include "qnet.h"

int main (int argc, char *argv[]) {
  Qisis::Qnet::g_controlNetwork = NULL;
  Qisis::Qnet::g_serialNumberList = NULL;
  Qisis::Qnet::g_imageOverlap = NULL;

  try {
    QApplication *app = new QApplication(argc,argv);
    QApplication::setApplicationName("qnet");
    app->setStyle("windows");

    Qisis::ViewportMainWindow *vw = new Qisis::ViewportMainWindow("qnet");

    Qisis::Tool *rubberBandTool = Qisis::RubberBandTool::getInstance(vw);
    rubberBandTool->addTo(vw);

    Qisis::QnetFileTool *ftool = new Qisis::QnetFileTool(vw);
    ftool->Tool::addTo(vw);
    vw->permanentToolBar()->addSeparator();

    Qisis::QnetNavTool *ntool = new Qisis::QnetNavTool(vw);
    ntool->Tool::addTo(vw);

    Qisis::QnetPlotTool *pltool = new Qisis::QnetPlotTool(vw);
    pltool->Tool::addTo(vw);

    // The fileTool needs to know when the navTool wants to load images
    QObject::connect(ntool,SIGNAL(loadImage(const QString &)),
                     ftool,SLOT(loadImage(const QString &)));
    // The fileTool needs to know when to load the images associated with an
    // overlap polygon
    QObject::connect(ntool,SIGNAL(loadOverlap(Isis::ImageOverlap *)),
                     ftool,SLOT(loadOverlap(Isis::ImageOverlap *)));
    // The fileTool needs to know when to load the images associated with a
    // point
    QObject::connect(ntool,SIGNAL(loadPoint(Isis::ControlPoint *)),
                     ftool,SLOT(loadPoint(Isis::ControlPoint *)));

    // The navTool needs to know when the file tool has changed the serialNumberList
    QObject::connect(ftool,SIGNAL(serialNumberListUpdated()),
                     ntool,SLOT(updateList()));
    // The navTool needs to know when the plotTool has modified the currently
    // active images, overlaps or points lists
    //QObject::connect(pltool,SIGNAL(selectionUpdate()),
    //                 ntool,SLOT(updateSelected()));
    // The plotTool need to know when the navTool has modified the currently
    // active image, overlaps or points lists
    //QObject::connect(ntool,SIGNAL(selectionUpdate()),
    //                 pltool,SLOT(updateSelected()));

    Qisis::Tool *btool = new Qisis::BandTool(vw);
    btool->addTo(vw);

    Qisis::Tool *ztool = new Qisis::ZoomTool(vw);
    ztool->addTo(vw);
    vw->getMenu("&View")->addSeparator();

    Qisis::Tool *ptool = new Qisis::PanTool(vw);
    ptool->addTo(vw);
    vw->getMenu("&View")->addSeparator();

    Qisis::Tool *stool = new Qisis::StretchTool(vw);
    stool->addTo(vw);

    Qisis::Tool *findTool = new Qisis::FindTool(vw);
    findTool->addTo(vw);

    Qisis::Tool *ttool = new Qisis::AdvancedTrackTool(vw);
    ttool->addTo(vw);

    Qisis::Tool *wtool = new Qisis::WindowTool(vw);
    wtool->addTo(vw);

    vw->permanentToolBar()->addSeparator();
    Qisis::Tool *htool = new Qisis::HelpTool(vw);
    htool->addTo(vw);


    Qisis::Tool *tieTool = new Qisis::TieTool(vw);
    tieTool->addTo(vw);
    tieTool->activate(true);

    QObject::connect(ntool,SIGNAL(modifyPoint(Isis::ControlPoint *)),
                     tieTool,SLOT(modifyPoint(Isis::ControlPoint *))); 

    //  The plotTool needs to know when the TieTool has modified a point and
    //  needs to redraw viewports.  TODO:?? redraw all viewports for now,
    //  this might need to be smartened up to only draw viewports point is
    //  on if it gets too slow.
    QObject::connect(tieTool,SIGNAL(editPointChanged(std::string)),
                     pltool,SLOT(paintAllViewports(std::string)));
    //  The FileTool needs to now if the control network has changed (delete/
    //  edit/create point) so that user can be prompted to save net
    QObject::connect(tieTool,SIGNAL(netChanged()),
                     ftool,SLOT(setSaveNet()));

    QObject::connect(tieTool,SIGNAL(tieToolSave()),ftool,SLOT(saveAs()));

    vw->show();

    return app->exec();
  }
  catch (Isis::iException &e) {
    e.Report();
  }
}

