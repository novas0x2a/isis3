#include <sstream>
#include <QMessageBox>

#include "Workspace.h"
#include "iString.h"
#include "CubeViewport.h"
#include "CubeAttribute.h"


namespace Qisis {
  
  Workspace::Workspace (QWidget *parent) :
           QWorkspace (parent) {
    connect(this,SIGNAL(windowActivated(QWidget *)),
            this,SLOT(activateViewport(QWidget *)));
    p_lastWindow = NULL;
  };

  void Workspace::activateViewport (QWidget *w) {
    p_cubeViewportList.clear();
    populateList();
    emit cubeViewportActivated((Qisis::CubeViewport *) w);
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
      // delete cube; ***Removed this line since this is being taken care of in addCubeViewport(cube) using cvp->close()
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
    Qisis::CubeViewport *cvp = new Qisis::CubeViewport(cube,this);
    addWindow(cvp);
    cvp->show();
    //  Initially, Load entire image
    double scale = cvp->fitScale();
    cvp->setScale(scale, cvp->cubeSamples()/2.0, cvp->cubeLines()/2.0);
    try{
      cvp->showCube();
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
    populateList();
    emit cubeViewportAdded (cvp);
    return cvp;
  }

  void Workspace::populateList() {
    p_cubeViewportList.clear();
    QWidgetList list = windowList(CreationOrder);
    QWidgetList::const_iterator it = list.begin();
    while (it != list.end()) {
      if ((*it)->isVisible()) {
        p_cubeViewportList.push_back((CubeViewport *) *it);
      }
      ++it;
    }
  }


  void Workspace::addBrowseView(QString cubename) {
    int x_pos = -10000;
    int y_pos = -10000;
    /* Close the last browse window if necessary.  */
    if (p_lastWindow != NULL) {
      populateList();
      for (unsigned int i=0; i<p_cubeViewportList.size(); i++) {
        if (p_lastWindow == p_cubeViewportList[i]) {
          x_pos = p_lastWindow->parentWidget()->x();
          y_pos = p_lastWindow->parentWidget()->y();
          setActiveWindow(p_lastWindow);
          closeActiveWindow();
          break;
        }
      }
      p_lastWindow = NULL;
    }

    unsigned int listSize = p_cubeViewportList.size();
    addCubeViewport(cubename);
    if (listSize != p_cubeViewportList.size()) {
      p_lastWindow = p_cubeViewportList[p_cubeViewportList.size()-1];
      //p_lastWindow->move(x_pos, y_pos);
      
    }

  }

}
