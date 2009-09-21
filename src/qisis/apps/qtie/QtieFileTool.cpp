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
    
    saveAction()->setEnabled(false);

  }

  /**
   *  Open base image and image to be adjusted
   * 
   * @author 2007-08-13 Tracie Sucharski
   * 
   * @internal 
   * @history 2009-06-10  Tracie Sucharski - Added error checking and looping 
   *                            when opening files.  Allow new files to be
   *                            opened.
   * 
   */
  void QtieFileTool::open () {


    //  If we've already opened files, clear before starting over
    if (cubeViewportList()->size() > 0) {
      closeAll();
      emit newFiles();
    }


    QString baseFile,matchFile;
    QString dir;
    QString filter = "Isis Cubes (*.cub);;";
    filter += "Detached labels (*.lbl);;";
    filter += "All (*)";

    Isis::Cube *baseCube = new Isis::Cube;
    while (!baseCube->IsOpen()) {
      baseFile = QFileDialog::getOpenFileName((QWidget*)parent(),
                                                  "Select basemap cube (projected)",
                                                  ".",filter);
      if (baseFile.isEmpty()) {
        delete baseCube;
        return;
      }
  
      // Find directory and save for use in file dialog for match cube
      Isis::Filename fname(baseFile.toStdString());
      dir = fname.absolutePath();
  
      //  Make sure base is projected
      try {
        baseCube->Open(baseFile.toStdString());
        try {
          baseCube->Projection();
        } catch (Isis::iException &e) {
          baseCube->Close();
          QString message = "Base must be projected";
          QMessageBox::critical((QWidget *)parent(),"Error",message);
          baseCube->Close();
        }
      } catch (Isis::iException &e) {
        QString message = "Unable to open base cube";
        QMessageBox::critical((QWidget *)parent(),"Error",message);
      }
    }
    baseCube->Close();


    Isis::Cube *matchCube = new Isis::Cube;

    while (!matchCube->IsOpen()) {
      // Get match cube
      matchFile = QFileDialog::getOpenFileName((QWidget*)parent(),
                                          "Select cube to tie to base",
                                          dir,filter);
      if (matchFile.isEmpty()) {
        delete baseCube;
        delete matchCube;
        (*(cubeViewportList()))[0]->close();
        return;
      }
  
      //Make sure match is not projected
      try {
        matchCube->Open(matchFile.toStdString());
        try {
  
          if (matchCube->HasGroup("Mapping")) {
            QString message = "The match cube cannot be a projected cube.";
            QMessageBox::critical((QWidget *)parent(),"Error",message);
            matchCube->Close();
            continue;
          }
        } catch (Isis::iException &e) {
          QString message = "Error reading match cube labels.";
          QMessageBox::critical((QWidget *)parent(),"Error",message);
          matchCube->Close();
        }
      } catch (Isis::iException &e) {
        QString message = "Unable to open match cube";
        QMessageBox::critical((QWidget *)parent(),"Error",message);
      }
    }
    matchCube->Close();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    emit fileSelected(baseFile);
    baseCube = (*(cubeViewportList()))[0]->cube();
    QApplication::restoreOverrideCursor();


    QApplication::setOverrideCursor(Qt::WaitCursor);
    emit fileSelected(matchFile);
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
