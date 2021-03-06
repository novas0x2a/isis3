#include <QApplication>
#include <QToolBar>
#include "iException.h"
#include "Filename.h"
#include "ViewportMainWindow.h"
#include "QnetFileTool.h"
#include "QnetNavTool.h"
#include "BandTool.h"
#include "ZoomTool.h"
#include "PanTool.h"
#include "StretchTool.h"
#include "FindTool.h"
#include "WindowTool.h"
#include "AdvancedTrackTool.h"
#include "HelpTool.h"
#include "QnetTool.h"
#include "RubberBandTool.h"

#define IN_QNET
#include "qnet.h"

int main (int argc, char *argv[]) {
  Qisis::Qnet::g_controlNetwork = NULL;
  Qisis::Qnet::g_serialNumberList = NULL;

  try {
    QApplication *app = new QApplication(argc,argv);
    QApplication::setApplicationName("qnet");
    app->setStyle("windows");

    // Add the Qt plugin directory to the library path
    Isis::Filename qtpluginpath ("$ISISROOT/3rdParty/plugins");
    QCoreApplication::addLibraryPath(qtpluginpath.Expanded().c_str());

    Qisis::Qnet::g_vpMainWindow = new Qisis::ViewportMainWindow("qnet");

    Qisis::Tool *rubberBandTool = Qisis::RubberBandTool::getInstance(Qisis::Qnet::g_vpMainWindow);
    rubberBandTool->addTo(Qisis::Qnet::g_vpMainWindow);

    Qisis::QnetFileTool *ftool = new Qisis::QnetFileTool(Qisis::Qnet::g_vpMainWindow);
    ftool->Tool::addTo(Qisis::Qnet::g_vpMainWindow);
    Qisis::Qnet::g_vpMainWindow->permanentToolBar()->addSeparator();

    Qisis::QnetNavTool *ntool = new Qisis::QnetNavTool(Qisis::Qnet::g_vpMainWindow);
    ntool->Tool::addTo(Qisis::Qnet::g_vpMainWindow);

    // The fileTool needs to know when the navTool wants to load images
    QObject::connect(ntool,SIGNAL(loadImage(const QString &)),
                     ftool,SLOT(loadImage(const QString &)));
    // The fileTool needs to know when to load the images associated with a
    // point
    QObject::connect(ntool,SIGNAL(loadPoint(Isis::ControlPoint *)),
                     ftool,SLOT(loadPoint(Isis::ControlPoint *)));

    // The navTool needs to know when the file tool has changed the serialNumberList
    QObject::connect(ftool,SIGNAL(serialNumberListUpdated()),
                     ntool,SLOT(resetList()));

    // The navTool cube name filter needs to know when the file tool 
    // has changed the serialNumberList in order to refresh the list
    // Jeannie Walldren 2009-01-26
    QObject::connect(ftool,SIGNAL(serialNumberListUpdated()),
                     ntool,SLOT(resetCubeList()));

    Qisis::Tool *btool = new Qisis::BandTool(Qisis::Qnet::g_vpMainWindow);
    btool->addTo(Qisis::Qnet::g_vpMainWindow);

    Qisis::Tool *ztool = new Qisis::ZoomTool(Qisis::Qnet::g_vpMainWindow);
    ztool->addTo(Qisis::Qnet::g_vpMainWindow);
    Qisis::Qnet::g_vpMainWindow->getMenu("&View")->addSeparator();

    Qisis::Tool *ptool = new Qisis::PanTool(Qisis::Qnet::g_vpMainWindow);
    ptool->addTo(Qisis::Qnet::g_vpMainWindow);
    Qisis::Qnet::g_vpMainWindow->getMenu("&View")->addSeparator();

    Qisis::Tool *stool = new Qisis::StretchTool(Qisis::Qnet::g_vpMainWindow);
    stool->addTo(Qisis::Qnet::g_vpMainWindow);

    Qisis::Tool *findTool = new Qisis::FindTool(Qisis::Qnet::g_vpMainWindow);
    findTool->addTo(Qisis::Qnet::g_vpMainWindow);

    Qisis::Tool *ttool = new Qisis::AdvancedTrackTool(Qisis::Qnet::g_vpMainWindow);
    ttool->addTo(Qisis::Qnet::g_vpMainWindow);

    Qisis::Tool *wtool = new Qisis::WindowTool(Qisis::Qnet::g_vpMainWindow);
    wtool->addTo(Qisis::Qnet::g_vpMainWindow);

    Qisis::Qnet::g_vpMainWindow->permanentToolBar()->addSeparator();
    Qisis::Tool *htool = new Qisis::HelpTool(Qisis::Qnet::g_vpMainWindow);
    htool->addTo(Qisis::Qnet::g_vpMainWindow);

    Qisis::Tool *qnetTool = new Qisis::QnetTool(Qisis::Qnet::g_vpMainWindow);
    qnetTool->addTo(Qisis::Qnet::g_vpMainWindow);
    qnetTool->activate(true);
    QObject::connect(ntool,SIGNAL(modifyPoint(Isis::ControlPoint *)),
                     qnetTool,SLOT(modifyPoint(Isis::ControlPoint *))); 
    QObject::connect(ntool,SIGNAL(ignoredPoints()),qnetTool,SLOT(refresh()));
    QObject::connect(ntool,SIGNAL(deletedPoints()),qnetTool,SLOT(refresh()));
    
    // The QnetTool needs to know when the file tool has changed the
    // serialNumberList and the controlnetwork.
//  QObject::connect(ftool,SIGNAL(serialNumberListUpdated()),
//                   qnetTool,SLOT(updateList()));

    // the next command was uncommented to allows the QnetTool to display the name 
    // of the control network file and update when a new control network is selected 
    // 2008-11-26 Jeannie Walldren
    QObject::connect(ftool,SIGNAL(controlNetworkUpdated(QString)),   
                     qnetTool,SLOT(updateNet(QString)));             
    QObject::connect(qnetTool,SIGNAL(editPointChanged(std::string)),
                     ntool,SLOT(updateEditPoint(std::string)));
    QObject::connect(qnetTool,SIGNAL(refreshNavList()),
                     ntool,SLOT(refreshList()));
    //  The FileTool needs to now if the control network has changed (delete/
    //  edit/create/ignore point) so that user can be prompted to save net
    QObject::connect(qnetTool,SIGNAL(netChanged()),ftool,SLOT(setSaveNet()));
    QObject::connect(ntool,SIGNAL(netChanged()),ftool,SLOT(setSaveNet()));

    QObject::connect(qnetTool,SIGNAL(qnetToolSave()),ftool,SLOT(saveAs()));


    // Connect the viewport's close signal to the file tool's exit method
    // Added 2008-12-04 by Jeannie Walldren
    QObject::connect(Qisis::Qnet::g_vpMainWindow , 
                     SIGNAL(closeWindow()), ftool, SLOT(exit()));
    //-----------------------------------------------------------------

    Qisis::Qnet::g_vpMainWindow->show();

    return app->exec();
  }
  catch (Isis::iException &e) {
    e.Report();
  }
}

