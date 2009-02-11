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

#include "QtieFileTool.h"
#include "Application.h"
#include "Filename.h"
#include "MainWindow.h"
#include "CubeViewport.h"
#include "Workspace.h"
#include "SerialNumberList.h"

using namespace Qisis;

namespace Qisis {
  // Constructor
  QtieFileTool::QtieFileTool (QWidget *parent) : Qisis::FileTool(parent) {
    openAction()->setToolTip("Open images");
    QString whatsThis =
      "<b>Function:</b> Open a <i>images</i> \
       <p><b>Shortcut:</b>  Ctrl+O\n</p>";
    openAction()->setWhatsThis(whatsThis);


  }

  /**
   *  Open base image and image to be adjusted
   * 
   * @author 2007-08-13 Tracie Sucharski
   * 
   * @internal
   * 
   */
  void QtieFileTool::open () {


    QString filter = "Isis Cubes (*.cub);;";
    filter += "Detached labels (*.lbl);;";
    filter += "All (*)";
    QString file = QFileDialog::getOpenFileName((QWidget*)parent(),
                                                "Select basemap cube",
                                                ".",
                                                filter);
    if (file.isEmpty()) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    // Find directory and save for use in file dialog for match cube
    Isis::Filename baseFile(file.toStdString());

    QString dir = baseFile.absolutePath();

    //  Make sure base is projected
    Isis::Cube *baseCube = new Isis::Cube;
    baseCube->Open(file.toStdString());
    try {
      baseCube->Projection();
    } catch (Isis::iException &e) {
      QString message = "Base must be projected";
      QMessageBox::critical((QWidget *)parent(),"Error",message);
      return;
    }
    baseCube->Close();

    emit fileSelected(file);
    baseCube = (*(cubeViewportList()))[0]->cube();
    QApplication::restoreOverrideCursor();

    // Get match cube
    file = QFileDialog::getOpenFileName((QWidget*)parent(),
                                        "Select cube to tie to base",
                                        dir,
                                        filter);
    if (file.isEmpty()) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    //Make sure match is not projected
    Isis::Cube *matchCube = new Isis::Cube;
    matchCube->Open(file.toStdString());
    if (matchCube->Camera()->HasProjection()) {
      QString message = "The match cube cannot be projected.";
      QMessageBox::critical((QWidget *)parent(),"Error",message);
      return;
    }
    matchCube->Close();

    emit fileSelected(file);
    matchCube = (*(cubeViewportList()))[1]->cube();
    emit cubesOpened(*baseCube,*matchCube);
    QApplication::restoreOverrideCursor();

    return;
  }





#if 0
  // Exit the program
............................................

void QtieFileTool::exit() {
    qApp->quit();
  }
#endif
}
