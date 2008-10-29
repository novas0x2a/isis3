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

#include "QnetFileTool.h"
#include "Application.h"
#include "Filename.h"
#include "MainWindow.h"
#include "CubeViewport.h"
#include "Workspace.h"
#include "ImageOverlap.h"
#include "FindImageOverlaps.h"
#include "qnet.h"

using namespace Qisis::Qnet;
using namespace std;

namespace Qisis {
  // Constructor
  QnetFileTool::QnetFileTool (QWidget *parent) : Qisis::FileTool(parent) {
    openAction()->setToolTip("Open network");
    QString whatsThis =
      "<b>Function:</b> Open a <i>control network</i> \
       <p><b>Shortcut:</b>  Ctrl+O\n</p>";
    openAction()->setWhatsThis(whatsThis);

    saveAction()->setToolTip("Save network");
    whatsThis =
      "<b>Function:</b> Save the current <i>control network</i>";
    saveAction()->setWhatsThis(whatsThis);
    saveAction()->setEnabled(true);

    p_saveNet = false;
  }

  /**
   *  Open a list of cubes
   * 
   * @author  ???? Elizabeth Ribelin
   * 
   * @internal
   * @history  2007-06-07 Tracie Sucharski - Allow new network to be opened,
   *                         prompt to save old network.
   * 
   */
  void QnetFileTool::open () {

    //  If network already opened, prompt for saving
    if (g_serialNumberList != NULL && p_saveNet) {
      //  If control net has been changed , prompt for user to save
      int resp = QMessageBox::warning((QWidget*)parent(),"Qnet",
        "The control network files has been modified.\n"
        "Do you want to save your changes?",
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No,
        QMessageBox::Cancel | QMessageBox::Escape);
      if (resp == QMessageBox::Yes) {
        saveAs();
      }
      p_saveNet = false;
    }

    QString filter = "List of cubes (*.lis *.lst *.list);;";
    filter += "Text file (*.txt);;";
    filter += "All (*)";
    QString list = QFileDialog::getOpenFileName((QWidget*)parent(),
                                                "Select a list of cubes",
                                                ".",
                                                filter);
    if (list.isEmpty()) return;

    // Find directory and save for use in file dialog for net file
    Isis::Filename file(list.toStdString());
    QString dir = file.absolutePath();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    // Use the list to get serial numbers and polygons
    try {
      if (g_serialNumberList != NULL) {
        delete g_serialNumberList;
        g_serialNumberList = NULL;
      }
      g_serialNumberList = new Isis::SerialNumberList(list.toStdString());

      if (g_imageOverlap != NULL) {
        delete g_imageOverlap;
        g_imageOverlap = NULL;
      }

#if 0
      //  TODO;  Print error if polyinit hasn't been run
      try {
        g_imageOverlap = new Isis::FindImageOverlaps(*g_serialNumberList);

      } catch ( Isis::iException &e ) {
        QString message = "Error calculating image overlaps. ";
        message += "Polygon plots will not be usable, but you can still ";
        message += "pick points.\n\n";
        string errors = e.Errors();
        message += errors.c_str();
        e.Clear();
        QMessageBox::information((QWidget *)parent(),"Error",message);
        g_imageOverlap = NULL;
      }
#endif

      if (g_controlNetwork != NULL) {
        delete g_controlNetwork;
        g_controlNetwork = NULL;
      }
    }
    catch (Isis::iException &e) {
      QString message = "Error processing cube list.  \n";
      string errors = e.Errors();
      message += errors.c_str();
      e.Clear ();
      QMessageBox::information((QWidget *)parent(),"Error",message);
      QApplication::restoreOverrideCursor();
      return;
    }

    QApplication::restoreOverrideCursor();
    filter = "Control net (*.net);;";
    filter += "Text file (*.txt);;";
    filter += "All (*)";
    QString net = QFileDialog::getOpenFileName((QWidget*)parent(),
                                               "Select a control network",
                                               dir,
                                               filter);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (net.isEmpty()) {
      g_controlNetwork = new Isis::ControlNet();
      g_controlNetwork->SetType(Isis::ControlNet::ImageToGround);
      g_controlNetwork->SetUserName(Isis::Application::UserName());
    }
    else {
      try {
        g_controlNetwork = new Isis::ControlNet(net.toStdString());
      }
      catch (Isis::iException &e) {
        QString message = "Invalid control network.  \n";
        string errors = e.Errors();
        message += errors.c_str();
        e.Clear ();
        QMessageBox::information((QWidget *)parent(),"Error",message);
        QApplication::restoreOverrideCursor();
        return;
      }
    }

    //  Initialize cameras for control net
    g_controlNetwork->SetImages (*g_serialNumberList);

    emit serialNumberListUpdated();
    emit controlNetworkUpdated();
    QApplication::restoreOverrideCursor();
    return;
  }

  // Exit the program
  void QnetFileTool::exit() {
    //  If control net has been changed , prompt for user to save
    if (p_saveNet) {
      int resp = QMessageBox::warning((QWidget*)parent(),"TieTool",
        "The control network files has been modified.\n"
        "Do you want to save your changes?",
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No,
        QMessageBox::Cancel | QMessageBox::Escape);
      if (resp == QMessageBox::Yes) {
        saveAs();
      }
      if (resp == QMessageBox::Cancel) {
        return;
      }
    }
    qApp->quit();
  }

  void QnetFileTool::saveAs() {
    QString filter = "Control net (*.net);;";
    filter += "Text file (*.txt);;";
    filter += "All (*)";
    QString fn=QFileDialog::getSaveFileName((QWidget*)parent(),
                                            "Choose filename to save under",
                                            ".", filter);
    if ( !fn.isEmpty() ) {
      try {
        g_controlNetwork->Write(fn.toStdString());
      } 
      catch (Isis::iException &e) {
        QString message = "Error saving control network.  \n";
        string errors = e.Errors();
        message += errors.c_str();
        e.Clear ();
        QMessageBox::information((QWidget *)parent(),"Error",message);
        return;
      }
    }
    else {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Saving Aborted");
    }
    p_saveNet = false;
  } 

  void QnetFileTool::loadImage(const QString &serialNumber) {
    if (g_serialNumberList->HasSerialNumber(serialNumber.toStdString())) {
      string tempFilename = g_serialNumberList->Filename(serialNumber.toStdString());
      QString filename = tempFilename.c_str();
      emit fileSelected(filename);
    }
    else {
      // TODO:  Handle error?
    }
  }

  void QnetFileTool::loadOverlap(Isis::ImageOverlap *overlap) {
    for (int i=0; i<overlap->Size(); i++) {
      loadImage((*overlap)[i].c_str());
    }
  }

  void QnetFileTool::loadPoint(Isis::ControlPoint *point) {
    for (int i=0; i<point->Size(); i++) {
      string cubeSN = (*point)[i].CubeSerialNumber();
      loadImage(cubeSN.c_str());
    }
  }

  void QnetFileTool::setSaveNet() {
    p_saveNet = true;
  }


}
