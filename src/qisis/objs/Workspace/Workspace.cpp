#include <sstream>
#include <QMessageBox>

#include "Workspace.h"
#include "iString.h"
#include "CubeViewport.h"
#include "CubeAttribute.h"


namespace Qisis {

  /**
   * Workspace constructor
   * 
   * @param parent 
   */
  Workspace::Workspace (QWidget *parent) : QMdiArea (parent) {
    connect(this,SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this,SLOT(activateViewport(QMdiSubWindow *)));
    setActivationOrder(ActivationHistoryOrder);
  };

  /**
   * This gets called when a window is activated or the workspace loses focus.
   * 
   * @param w 
   */
  void Workspace::activateViewport (QMdiSubWindow *w) {
    if(w) {
      emit cubeViewportActivated((Qisis::CubeViewport *) w->widget());
    }
    //Check if there is no current window (on close)
    else if(!currentSubWindow()) {
      emit cubeViewportActivated((Qisis::CubeViewport *)NULL);
    }
  }

  /**
   * Repopulates the list of CubeViewports and returns a pointer to this list.
   * Ownership is not given to the caller.
   * 
   * @return std::vector<CubeViewport*>* 
   */
  std::vector<CubeViewport *> *Workspace::cubeViewportList() {
    p_cubeViewportList.clear();
    for(int i = 0; i < subWindowList().size(); i++) {
      p_cubeViewportList.push_back((CubeViewport *)subWindowList()[i]->widget());
    }

    return &p_cubeViewportList;
  }

  /**
   * Add a cubeViewport to the workspace, open the cube.
   * 
   * @param cubename[in]  (QString)  cube name
   * 
   * @internal
   *  @history 2006-06-12 Tracie Sucharski,  Clear errors after catching when
   *                           attempting to open cube.
   *  @history 2007-02-13 Tracie Sucharski,  Open cube read, not read/write.
   *                          Opening read/write was done to accomodate the
   *                          EditTool. EditTool will now reopen the cube
   *                          read/write.
   *  @history 2008-05-27 Noah Hilt, When opening a cube now if a
   *           user specifies extra arguments to the cube name,
   *           the cube will be opened using a CubeAttributeInput
   *           specifically for the number and index of bands to
   *           be opened. Additionally, if a cube is opened with 3
   *           bands it will be opened in RGB mode with red,
   *           green, and blue set to the 3 bands respectively.
   *  @history 2008-12-04 Jeannie Walldren - Removed "delete cube"
   *           since this was causing a segfault and this
   *           deallocation is already taking place in
   *           addCubeViewport(cube).
   *  
   */
  void Workspace::addCubeViewport(QString cubename) {
    Isis::Cube *cube = new Isis::Cube;
    try {
      //Read in the CubeAttribueInput from the cube name
      Isis::CubeAttributeInput inAtt(cubename.toStdString());
      std::vector<std::string> bands = inAtt.Bands();
      //Set the virtual bands to the bands specified by the input
      cube->SetVirtualBands(bands);
      cube->Open(cubename.toStdString());
      CubeViewport *cvp = addCubeViewport(cube);
      // Check for RGB format (#R,#G,#B)
      if(bands.size() == 3) {
        Isis::iString st = Isis::iString(bands.at(0));
        int index_red = st.ToInteger();
        st = Isis::iString(bands.at(1));
        int index_green = st.ToInteger();
        st = Isis::iString(bands.at(2));
        int index_blue = st.ToInteger();
        cvp->viewRGB(index_red, index_green, index_blue);
      }
    }
    catch (...) {
      //delete cube; ***Removed this line since this is being taken care of in addCubeViewport(cube) using cvp->close()
      QMessageBox::information( this, "Error","Unable to open cube " + cubename);
    }
  }

  /**
   * Add a cubeViewport to the workspace.
   * 
   * @param cube[in]  (Isis::Cube *)  cube information
   * 
   * @internal
   *
   * @history  2007-04-13  Tracie Sucharski - Load entire image instead of 
   *                           fitting smallest dimension.
   * @history  2008-05-27  Noah Hilt - Now returns a CubeViewport 
   *           in order for the addCubeViewport(QString cubename)
   *           method to modify the CubeViewport.
   * @history 2008-08-20 Stacy Alley - Changed the setScale call 
   *          to match the zoomTool's fit in view
   * @history 2008-12-04 Jeannie Walldren - Added try/catch to 
   *          close the CubeViewport if showCube() is not
   *          successful.
   */
  CubeViewport* Workspace::addCubeViewport(Isis::Cube *cube) {
    Qisis::CubeViewport *cvp = NULL;

    QMdiSubWindow *window = new QMdiSubWindow;
    window->setOption(QMdiSubWindow::RubberBandResize, true);
    window->setOption(QMdiSubWindow::RubberBandMove, true);

    try {
      cvp = new Qisis::CubeViewport(cube, this);
      window->setWidget(cvp);
      window->setAttribute(Qt::WA_DeleteOnClose);
      addSubWindow(window);
      cvp->show();
    }
    catch (Isis::iException &e){
      // close CubeViewport window
      cvp->close();
      // add a new message to the caught exception and throw
      throw e.Message(Isis::iException::Programmer, 
                                      "Exception caught when attempting to show cube " 
                                      + cube->Filename(), 
                                      _FILEINFO_);
    }

    emit cubeViewportAdded (cvp);
    return cvp;
  }

  void Workspace::addBrowseView(QString cubename) {
    /* Close the last browse window if necessary.  */
    if (subWindowList().size() > 0) {
      removeSubWindow(subWindowList()[0]);
    }

    addCubeViewport(cubename);
  }
}
