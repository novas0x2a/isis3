#ifndef QnetTool_h
#define QnetTool_h

#include <QAction>
#include <QPushButton>
#include <QString>
#include <QToolButton>

#include "AutoRegFactory.h"
#include "ChipViewport.h"
#include "ControlMeasure.h"
#include "ControlNet.h"
#include "ControlPoint.h"
#include "ControlPointEdit.h"
#include "Pvl.h"
#include "QnetHoldPointDialog.h"
#include "SerialNumberList.h"
#include "Tool.h"

namespace Qisis {
  /**
   * @brief Qnet tool operations
   *
   * @ingroup Visualization Tools
   *
   *
   * @internal
   *   @history 2008-11-24 Jeannie Walldren - Changed name from TieTool.
   *                          Replace references to PointEdit class with
   *                          ControlPointEdit. Added "Goodness of Fit" to right
   *                          and left measure info.
   *   @history 2008-11-26  Jeannie Walldren - Added "Number of Measures" to
   *                          QnetTool point information. Defined updateNet() so
   *                          that the QnetTool window's title bar contains the
   *                          name of the control net file. Created
   *                          ignoreChanged() signal, modified pointSaved() and
   *                          createQnetTool() so message box appears if users are
   *                          saving an "Ignore" point and asks whether they would
   *                          like to set Ignore=false.
   *   @history 2008-12-09 Tracie Sucharski - Cleaned up some
   *                          signal/slot connections between QnetTool and
   *                          QnetNavTool for deleting or adding ControlPoints.
   *   @history 2008-12-09 Tracie Sucharski - Add new public slot
   *                          refresh to handle the ignorePoints and deletePoints
   *                          from the QnetNavTool.
   *   @history 2008-12-10 Jeannie Walldren - Added slot methods
   *                          viewTemplateFile() and setTemplateFile() to allow
   *                          user to view, edit or choose a new template file.
   *                          Added "What's this?" descriptions to actions.
   *   @history 2008-12-15 Jeannie Walldren - Some QMessageBox
   *                          warnings had strings tacked on to the list of
   *                          errors.  These strings were changed to iExceptions
   *                          and added to the error stack to conform with Isis
   *                          standards.
   *   @history 2008-12-15 Jeannie Walldren - Created newHoldPoint() method.
   *                          Replaced references to QnetGroundPointDialog with
   *                          QnetHoldPointDialog. Disabled ground point check box
   *                          so user may see whether the point is ground but may
   *                          not change this.  Thus setGroundPoint() and
   *                          newGroundPoint() methods still exist but are not
   *                          currently called.
   *   @history 2008-12-30 Jeannie Walldren - Modified to set measures in
   *                          viewports to Ignore=False if when saving, the user
   *                          chooses to set a point's Ignore=False. Replaced
   *                          references to ignoreChanged() with
   *                          ignorePointChanged(). Added signals
   *                          ignoreLeftChanged() and ignoreRightChanged().
   *   @history 2008-12-31 Jeannie Walldren - Added question box to pointSaved()
   *                          method to ask user whether the reference measure
   *                          should be replaced with the measure in the left
   *                          viewport. Added documentation.
   *   @history 2009-03-09 Jeannie Walldren - Modified createPoint() method
   *                          to clear the error stack after
   *                          displaying a QMessageBox to the user
   *   @history 2009-03-17 Tracie Sucharski - Added the ability to save the
   *                          registration chips to the Options menu.
   */
  class QnetTool : public Qisis::Tool {
    Q_OBJECT

    public:
      QnetTool (QWidget *parent);
//      void addTo (QMenu *menu);
      void paintViewport (CubeViewport *cvp,QPainter *painter);

    signals:
      void qnetToolSave();
      void refreshNavList();
      void editPointChanged(std::string pointId);
      void netChanged();
      void ignorePointChanged();
      void ignoreLeftChanged();
      void ignoreRightChanged();

    public slots:
      void updateList();
      void updateNet(QString cNetFilename);
      void createPoint(double lat,double lon);
      void modifyPoint(Isis::ControlPoint *point);
      void deletePoint(Isis::ControlPoint *point);
      void refresh();

    protected:
      QAction *toolPadAction(ToolPad *pad);
      bool eventFilter(QObject *o,QEvent *e);

    protected slots:
      void mouseButtonRelease(QPoint p, Qt::MouseButton s);

    private slots:
      void paintAllViewports (std::string pointId);
      void saveNet();
      void addMeasure();
      void setIgnorePoint (bool ignore);
      void setHoldPoint (bool hold);
      void newHoldPoint (Isis::ControlPoint &point);
      void setGroundPoint (bool ground);
      void newGroundPoint (Isis::ControlPoint &point);
      void setIgnoreLeftMeasure (bool ignore);
      void setIgnoreRightMeasure (bool ignore);

      void selectLeftMeasure (int index);
      void selectRightMeasure (int index);
      void updateLeftMeasureInfo ();
      void updateRightMeasureInfo ();

      void pointSaved ();

      void setTemplateFile();
      void viewTemplateFile();
      void saveChips();
      //void setInterestOp();

    private:
      void createQnetTool(QWidget *parent);
      QMainWindow *p_qnetTool;
      void createMenus();

      void loadPoint();
      void drawAllMeasurments (CubeViewport *vp,QPainter *painter);


      std::vector<std::string> findPointFiles(double lat,double lon);

      QAction *p_createPoint;
      QAction *p_modifyPoint;
      QAction *p_deletePoint;

      QMainWindow *p_mw;
      ControlPointEdit *p_pointEditor;

      QLabel *p_ptIdValue;
      QLabel *p_numMeasures;
      QCheckBox *p_ignorePoint;
      QCheckBox *p_holdPoint;
      QCheckBox *p_groundPoint;
      QLabel *p_leftMeasureType;
      QLabel *p_leftSampError;
      QLabel *p_leftLineError;
      QLabel *p_leftGoodness;
      QLabel *p_rightMeasureType;
      QLabel *p_rightSampError;
      QLabel *p_rightLineError;
      QLabel *p_rightGoodness;
      QCheckBox *p_ignoreLeftMeasure;
      QCheckBox *p_ignoreRightMeasure;

      QComboBox *p_leftCombo;
      QComboBox *p_rightCombo;

      Isis::ControlPoint *p_controlPoint;

      std::vector<std::string> p_pointFiles;

      std::string p_leftFile;
      Isis::ControlMeasure *p_leftMeasure;
      Isis::ControlMeasure *p_rightMeasure;
      Isis::Cube *p_leftCube;
      Isis::Cube *p_rightCube;

      QnetHoldPointDialog *p_holdPointDialog;
  };
};

#endif
