#ifndef ScatterPlotTool_h
#define ScatterPlotTool_h

#include "Tool.h"


namespace Qisis {
  /**                                                                       
   * @brief Scatter Plot Tool
   *                                                                                                                                            
   * @author Stacy Alley
   */

  class ScatterPlotWindow;

  class ScatterPlotTool : public Qisis::Tool {
    Q_OBJECT

    public:
      ScatterPlotTool (QWidget *parent);
      void setActionChecked(bool checked);

      typedef std::vector<Qisis::CubeViewport *> CubeViewportList;


      /**
       * Returns the list of cube viewports currently open in Qview.
       * 
       * 
       * @return CubeViewportList* 
       */
      inline CubeViewportList *getCubeViewportList() const { return cubeViewportList(); };

    protected:
      void createWindow();
      QAction *toolPadAction(ToolPad *pad);


      /**
       * Returns this tool's action
       * 
       * 
       * @return QAction* 
       */
      QAction * toolAction() { return p_action; };
      
    private:
        ScatterPlotWindow *p_mainWindow; //!<The window the scatter plot is displayed in.
        QAction *p_action; //!< The tool's action
        QWidget *p_parent;//!< The tool's parent.
      

  };
};




#endif

