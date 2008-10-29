#ifndef TieTool_h
#define TieTool_h

#include <QAction>
#include <QPushButton>
#include <QToolButton>
#include <QRadioButton>
#include <QListWidget>
#include <QMainWindow>
#include <QCheckBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QDial>
#include <QLCDNumber>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QString>


#include "Tool.h"
#include "SerialNumberList.h"
#include "ImageOverlap.h"
#include "ControlNet.h"
#include "ControlPoint.h"
#include "ControlMeasure.h"
#include "ChipViewport.h"
#include "Pvl.h"
#include "AutoRegFactory.h"
#include "QnetGroundPointDialog.h"

namespace Qisis {
  class TieTool : public Qisis::Tool {
    Q_OBJECT

    public:
      TieTool (QWidget *parent);
//      void addTo (QMenu *menu);

    signals:
      void updateLeftView(double sample,double line);
      void updateRightView(double sample,double line);
      void editPointChanged(std::string pointId);
      void netChanged();
      void tieToolSave();

    public slots:
      void modifyPoint(Isis::ControlPoint *point);
      void deletePoint(Isis::ControlPoint *point);

    protected:
      QAction *toolPadAction(ToolPad *pad);
      QWidget *createToolBarWidget (QStackedWidget *parent);
      bool eventFilter(QObject *o,QEvent *e);

    protected slots:
      void mouseButtonRelease(QPoint p, Qt::MouseButton s);

    private slots:
  //    void createPoint();
      void deletePoint();

      void selectLeftChip(int index);
      void selectRightChip(int index);
      void setRotation(int rotation);
      void setNoGeom();
      void setGeom();
      void setRotate();
      void findPoint();
      void registerPoint();
      void savePoint();
      void updateLeftMeasureInfo (int index);
      void updateRightMeasureInfo (int index);
      void updateLeftPositionLabel (double);
      void updateRightGeom();
      void updateRightPositionLabel (double);
      void setIgnorePoint (bool ignore);
      void setHoldPoint (bool hold);

      void setGroundPoint (bool ground);
      void newGroundPoint (Isis::ControlPoint &point);

      void setIgnoreLeftMeasure (bool ignore);
      void setIgnoreRightMeasure (bool ignore);

      void blinkStart();
      void blinkStop();
      void changeBlinkTime(double interval);
      void updateBlink();

      void save();
      void openTemplateFile();
      void addMeasure();
      //void setInterestOp();

    private:
      void createTieDialog(QWidget *parent);
      QMainWindow *p_tieDialog;
      void createMenus();

      QLabel *p_ptIdValue;
      QCheckBox *p_ignorePoint;
      QCheckBox *p_holdPoint;
      QCheckBox *p_groundPoint;
      QLabel *p_leftMeasureType;
      QLabel *p_leftSampError;
      QLabel *p_leftLineError;
      QLabel *p_rightMeasureType;
      QLabel *p_rightSampError;
      QLabel *p_rightLineError;
      QLabel *p_leftZoomFactor;
      QLabel *p_rightZoomFactor;
      QLabel *p_leftSampLinePosition;
      QLabel *p_rightSampLinePosition;
      QLabel *p_leftLatLonPosition;
      QLabel *p_rightLatLonPosition;
      QCheckBox *p_ignoreLeftMeasure;
      QCheckBox *p_ignoreRightMeasure;
      QRadioButton *p_nogeom;
      QRadioButton *p_geom;
      QToolButton *p_rightZoomIn;
      QToolButton *p_rightZoomOut;
      QToolButton *p_rightZoom1;
      

      bool p_timerOn;
      QTimer *p_timer;
      std::vector<ChipViewport *> p_blinkList;
      unsigned char p_blinkIndex;

      QComboBox *p_leftCombo;
      QComboBox *p_rightCombo;

      QDial *p_dial;
      QLCDNumber *p_dialNumber;
      QDoubleSpinBox *p_blinkTimeBox;

      QPushButton *p_autoReg;
      QWidget *p_autoRegExtension;
      QLabel *p_oldPosition;
      QLabel *p_goodFit;
      bool   p_autoRegShown;

      void createPoint(double samp,double line);

      void loadPoint();

      QAction *p_createPoint;
      QAction *p_modifyPoint;
      QAction *p_deletePoint;

//      void loadCubes(Isis::ControlPoint &pt);

      ChipViewport *p_leftView;
      ChipViewport *p_rightView;

      Isis::Chip *p_leftChip;
      Isis::Chip *p_rightChip;
      Isis::Cube *p_leftCube;
      Isis::Cube *p_rightCube;

      Isis::ControlPoint *p_controlPoint;

      std::string p_leftFile;
      Isis::ControlMeasure *p_leftMeasure;
      Isis::ControlMeasure *p_rightMeasure;
      std::vector<std::string> p_pointFiles;

      Isis::AutoReg *p_autoRegFact;

      int p_rotation;
      bool p_geomIt;

      QnetGroundPointDialog *p_groundPointDialog;
  };
};

#endif
